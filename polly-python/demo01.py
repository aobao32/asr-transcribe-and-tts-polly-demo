'''
Voice Engine
https://docs.aws.amazon.com/polly/latest/dg/generative-voices.html

API Guide
https://docs.aws.amazon.com/polly/latest/dg/API_SynthesizeSpeech.html

Python boto3 SDK sample
https://docs.aws.amazon.com/code-library/latest/ug/polly_example_polly_SynthesizeSpeech_section.html
'''

import boto3
import pyaudio

# 初始化 boto3 客户端
polly_client = boto3.client('polly', region_name='us-west-2')  # 根据你的区域选择

def text_to_speech(text):
    # 调用 AWS Polly 生成语音
    response = polly_client.synthesize_speech(
        #VoiceId='Joanna',  # 选择你喜欢的声音-美国英语
        VoiceId='Amy',  # 选择你喜欢的声音-英国英语
        OutputFormat='pcm',  # 输出格式为 PCM
        TextType='ssml',  # 文本类型为普通文本text或者标签语言ssml
        Text=text,
        SampleRate='16000',  # 采样率
        #Engine='neural'  # 使用神经网络引擎
        Engine='generative'  # 使用神经网络引擎
    )

    # 读取音频流
    audio_stream = response['AudioStream'].read()

    # 使用 pyaudio 播放音频
    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paInt16,
                    channels=1,
                    rate=16000,
                    output=True)

    stream.write(audio_stream)

    stream.stop_stream()
    stream.close()
    p.terminate()

if __name__ == "__main__":
    text = '<speak>Amazon Polly is generative text-to-speech (TTS) engine offers the most human-like, emotionally engaged, and adaptive conversational voices available for the use via the Amazon Polly console. <break time="2s"/> The Generative engine is the largest Amazon Polly TTS model to-date. It deploys a billion-parameter transformer that converts raw text into speech codes, followed by a convolution-based decoder that converts these speech codes into waveforms in an incremental, streamable manner. <break time="1s"/> This method shows the widely-reported emergent abilities of Large Language Models (LLMs) when trained on increasing volumes of publicly available and proprietary data comprising a variety of voices, languages, and styles.</speak>'
    text_to_speech(text)