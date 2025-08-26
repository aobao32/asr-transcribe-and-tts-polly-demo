import boto3
import pyaudio

def count_words(text):
    """计算英文单词数量"""
    return len(text.split())

def text_to_speech(text):
    """调用AWS Polly生成语音并播放"""
    # 初始化 boto3 客户端 - 法兰克福区域
    polly_client = boto3.client('polly', region_name='us-west-2')
    
    # 调用 AWS Polly 生成语音
    response = polly_client.synthesize_speech(
        VoiceId='Amy',  # 英国英语声音
        OutputFormat='pcm',  # PCM格式
        TextType='text',  # 普通文本
        Text=text,
        SampleRate='16000',  # 采样率16000
        Engine='generative'  # 生成式引擎
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

def main():
    print("Amazon Polly 文字转语音程序")
    print("按 Ctrl+C 退出程序")
    print("-" * 40)
    
    while True:
        try:
            # 提示用户输入文字
            text = input("\n请输入要转换为语音的英文文字（不超过200个单词）: ").strip()
            
            if not text:
                print("输入不能为空，请重新输入。")
                continue
            
            # 检查单词数量
            word_count = count_words(text)
            if word_count > 200:
                print(f"输入过长！当前单词数: {word_count}，请输入不超过200个英文单词。")
                continue
            
            print(f"正在生成语音... (单词数: {word_count})")
            
            # 生成并播放语音
            text_to_speech(text)
            
            print("播放完成！")
            
        except KeyboardInterrupt:
            print("\n\n程序已退出。")
            break
        except Exception as e:
            print(f"发生错误: {e}")
            print("请重试。")

if __name__ == "__main__":
    main()