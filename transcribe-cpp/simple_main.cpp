#include <aws/core/Aws.h>
#include <aws/transcribestreaming/TranscribeStreamingServiceClient.h>
#include <aws/transcribestreaming/model/StartStreamTranscriptionRequest.h>
#include <aws/transcribestreaming/model/StartStreamTranscriptionHandler.h>
#include <aws/transcribestreaming/model/AudioStream.h>
#include <aws/transcribestreaming/model/AudioEvent.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/threading/Semaphore.h>
#include <portaudio.h>
#include <iostream>
#include <vector>
#include <atomic>
#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <string>
#include "transcribe_helper.h"

using namespace Aws::TranscribeStreamingService;
using namespace Aws::TranscribeStreamingService::Model;

std::atomic<bool> running{false};
std::unique_ptr<TranscribeStreamingServiceClient> client;
std::queue<std::vector<int16_t>> audioQueue;
std::mutex audioMutex;
std::shared_ptr<AudioStream> transcribeStream;
std::atomic<bool> needNewSession{true};
std::mutex sessionMutex;

int audioCallback(const void* inputBuffer, void* outputBuffer,
                 unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
                 PaStreamCallbackFlags statusFlags, void* userData) {
    
    if (!running || !inputBuffer) return paContinue;
    
    const auto* input = static_cast<const int16_t*>(inputBuffer);
    
    // Queue audio data for processing
    std::lock_guard<std::mutex> lock(audioMutex);
    std::vector<int16_t> audioData(input, input + framesPerBuffer);
    audioQueue.push(audioData);
    
    return paContinue;
}

int main() {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    
    // Initialize AWS client
    Aws::Client::ClientConfiguration config;
    config.region = "us-west-2";
    client = std::make_unique<TranscribeStreamingServiceClient>(config);
    
    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }
    
    // Setup audio input
    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paInt16;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;
    
    PaStream* stream;
    err = Pa_OpenStream(&stream, &inputParams, nullptr, 16000, 1600, paClipOff, audioCallback, nullptr);
    
    if (err != paNoError) {
        std::cerr << "Failed to open stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }
    
    std::cout << "Starting audio capture from default input device..." << std::endl;
    std::cout << "Speak into your microphone. Press Enter to stop." << std::endl;
    
    running = true;
    Pa_StartStream(stream);
    
    // 创建转录处理器
    StartStreamTranscriptionHandler handler;
    
    // 设置错误回调
    handler.SetOnErrorCallback([](const Aws::Client::AWSError<TranscribeStreamingServiceErrors>& error) {
        std::cerr << "转录错误: " << error.GetMessage() << std::endl;
    });
    
    // 设置转录结果回调 - 这里会接收真正的转录文字
    handler.SetTranscriptEventCallback([](const TranscriptEvent& event) {
        for (const auto& result : event.GetTranscript().GetResults()) {
            if (result.GetIsPartial()) {
                std::cout << "[部分结果] ";
            } else {
                std::cout << "[最终结果] ";
                // 获得最终结果后，标记需要新的会话
                needNewSession = true;
            }
            for (const auto& alternative : result.GetAlternatives()) {
                std::string transcript = alternative.GetTranscript();
                TranscribeHelper::setTranscriptionResult(transcript);
                std::cout << transcript << std::endl;
            }
        }
    });
    
    // 创建转录请求 - 仅支持英文
    StartStreamTranscriptionRequest request;
    request.SetMediaSampleRateHertz(16000);
    request.SetLanguageCode(LanguageCode::en_US);  // 设置为英文
    request.SetMediaEncoding(MediaEncoding::pcm);
    request.SetEventStreamHandler(handler);
    
    std::cout << "Amazon Transcribe 服务启动，请设置MacOS隐私允许程序调用默认音频输入设备。" << std::endl;
    
    // 设置流准备回调
    auto onStreamReady = [&](AudioStream& stream) {
        std::cout << "--- 本程序仅支持英文语音 --- ，开始发送音频..." << std::endl;
        transcribeStream = std::shared_ptr<AudioStream>(&stream, [](AudioStream*){});
    };
    
    // 设置响应回调
    Aws::Utils::Threading::Semaphore signaling(0, 1);
    auto onResponseCallback = [&signaling](const TranscribeStreamingServiceClient*,
                                          const StartStreamTranscriptionRequest&,
                                          const StartStreamTranscriptionOutcome& outcome,
                                          const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) {
        if (!outcome.IsSuccess()) {
            std::cerr << "Streaming流错误: " << outcome.GetError().GetMessage() << std::endl;
        }
        signaling.Release();
    };
    
    // 启动异步转录
    client->StartStreamTranscriptionAsync(request, onStreamReady, onResponseCallback);
    
    // 等待流准备好
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Audio processing thread - send audio to transcription
    std::thread audioThread([&]() {
        while (running) {
            // 检查是否需要新的转录会话
            if (needNewSession.load()) {
                std::lock_guard<std::mutex> lock(sessionMutex);
                if (needNewSession.load()) {
                    // 结束当前会话
                    if (transcribeStream) {
                        transcribeStream->WriteAudioEvent(AudioEvent());
                        transcribeStream->flush();
                        transcribeStream->Close();
                        transcribeStream.reset();
                    }
                    
                    std::cout << "\n--- Start new transcribe session 开始新的转录会话 ---" << std::endl;
                    
                    // 创建新的转录请求
                    StartStreamTranscriptionRequest newRequest;
                    newRequest.SetMediaSampleRateHertz(16000);
                    newRequest.SetLanguageCode(LanguageCode::en_US);
                    newRequest.SetMediaEncoding(MediaEncoding::pcm);
                    newRequest.SetEventStreamHandler(handler);
                    
                    // 启动新的转录会话
                    client->StartStreamTranscriptionAsync(newRequest, onStreamReady, onResponseCallback);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    
                    needNewSession = false;
                }
            }
            
            std::vector<int16_t> audioData;
            {
                std::lock_guard<std::mutex> lock(audioMutex);
                if (!audioQueue.empty()) {
                    audioData = audioQueue.front();
                    audioQueue.pop();
                }
            }
            
            if (!audioData.empty()) {
                // 实时发送音频数据到AWS Transcribe
                if (transcribeStream) {
                    try {
                        // 转换音频数据为字节
                        Aws::Vector<unsigned char> audioBytes;
                        audioBytes.reserve(audioData.size() * 2);
                        for (int16_t sample : audioData) {
                            audioBytes.push_back(sample & 0xFF);
                            audioBytes.push_back((sample >> 8) & 0xFF);
                        }
                        
                        // 创建音频事件并发送
                        AudioEvent audioEvent(std::move(audioBytes));
                        if (!transcribeStream->WriteAudioEvent(audioEvent)) {
                            std::cerr << "发送音频数据失败" << std::endl;
                        }
                        
                    } catch (const std::exception& e) {
                        std::cerr << "音频处理错误: " << e.what() << std::endl;
                    }
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    std::cout << "Sending Audio to Amazon Transcribe Streaming、... 按 Enter 回车键结束程序" << std::endl;
    std::cin.get();
    
    // 发送空的音频事件表示结束
    if (transcribeStream) {
        transcribeStream->WriteAudioEvent(AudioEvent());
        transcribeStream->flush();
        transcribeStream->Close();
    }
    
    running = false;
    
    // Wait for thread to finish
    if (audioThread.joinable()) audioThread.join();
    
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    
    signaling.WaitOne(); // 等待转录完成
    
    Aws::ShutdownAPI(options);
    
    // 返回最终的转录结果
    std::string finalResult = TranscribeHelper::getTranscriptionResult();
    std::cout << "\n最终转录结果: " << finalResult << std::endl;
    
    std::cout << "程序退出" << std::endl;
    return 0;
}