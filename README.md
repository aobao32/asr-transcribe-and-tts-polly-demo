# asr-transcribe-and-tts-polly-demo

本Demo演示ASR功能 (Amazon Transcribe) 本机mic输入，以及TTS（Amazon Polly）语音合成播放。

## 一、Amazon Transcribe 使用

### 1、Python sample code

MacOS系统安装依赖库：

```shell
brew install portaudio
pip3 install pyaudio aiofile asyncio sounddevice amazon-transcribe
```

如果您本机有多个Python版本，请注意确认安装了pip库的Python版本和从开发工具（例如VSCode）运行的Python是同一个版本。否则可能会出现为Python3.12版本安装了pip库，然后VSCode使用Python3.13版本运行代码，就会提示找不到库文件。

#### (1) 从音频文件生成字幕

目录`transcribe-python`下的`input from file.py`是从文件输入，指定一个文件的绝对路径，例如：

```
AUDIO_PATH = "/Users/username/Documents/MyWorkshop/transcribe/python/speech_demo.wav"
```

然后执行这个python即可。

```shell
python3 input_from_file.py
```

#### (2) 实时语音流生成文本

目录`transcribe-python`下的`input_from_mic.py`是从本机mic输入。使用前，需要先进入本机操作系统的音频输入属性界面，从多个输入设备中，选择实际的mic，然后再运行程序。

此外，MacOS等系统的安全机制会询问是否允许Python访问，此时选择允许，即可正常输入语音。如果是Mac电脑，还需要注意在屏幕关闭（俗称屏幕扣着）的时候，机身内置的Mic是永远静音（设备虽然可用但是输入音量也就是电平为0），因此不能扣盖测试。

执行这个python即可。

```shell
python3 input_from_mic.py
```

### 2、Java sample code

安装JDK可使用Amazon Corretto 17，下载目录在本文末尾的文档。此外也可以使用OpenJDK。

MacOS下安装Maven：

```shell
brew install maven
```

下载代码，切换到`pom.xml`文件所在的目录，执行如下程序打包：

```shell
mvn package
```

即可在`pom.xml`所在目录下，生成一个`target`目录，在其中可找到文件名是`ASR-x.x.jar`的文件。

执行如下命令运行Java程序：

```shell
java -jar target/ASR-1.1-SNAPSHOT.jar
```

### 3、注意事项

为保证实时短句的输入性能、降低延迟，建议使用如下参数设置：

- 采样率设置为16KHz，即`samplerate=16000`
- 输入为单声道，即`channels=1`，如果您的设备有多声道，可以现在设备硬件层面通过降噪算法，分离声音，然后输入给Transcribe
- 窗口大小建议为1600，即`blocksize=1600`

由于输入是短句，因此`enable_partial_results_stabilization`和`partial_results_stability=medium`两项参数不需要设置，即不需要传入这两个参数，使用默认值即可获得较好效果。

此外，不建议使用多种语言混合识别场景，短句场景建议识别单一语言，即在传入参数位置指定单一语言。如果您有多语言输入需求，建议结合您的应用程序设计，根据用户Profile配置中选择的当前国家和地区，来规定识别语言。

## 二、Amazon Polly 使用

### 1、语音引擎和语种

Amazon Polly现在提供四种语音引擎，分别是：

- 传统
- 神经网络
- 长篇神经网络
- 生成式

传统引擎和神经网络引擎是旧式的发生引擎，长篇引擎适合生成有声书。最新的引擎是生成式引擎，截止2024年12月，尽在AWS美东1、美西2、法兰克福等部分region可用。具体请参考Polly官方文档。

### 2、Python sample code

安装依赖包环境：

```shell
pip3 install pyaudio boto3
```

在`polly-python`目录下的`demo01.py`是调用Polly的代码例子。在使用前还需要调整本机默认的音频输出设备，例如笔记本内置扬声器、蓝牙耳机、有线耳机等。本代码将从默认输出设备播放。

执行`python3 demo01.py`即可获得音频播放。

### 3、注意事项

在调整Polly生成的音频质量时候，有多种格式可以选择，需要注意的是使用不同引擎，例如传统引擎和神经网络的引擎，输出音频位数有限制。

SSML主要是给传统和神经网络引擎用的，生成式引擎已经自动做了优化。因此，生成式引擎仅支持一部分SSML标签语言，不支持所有的SSML。因此，如果代码提示非法SSML格式，可能原因是使用了当前引擎不支持的SSML标签。

## 三、参考文档

Transcribe streaming python sample code:

[https://github.com/awslabs/amazon-transcribe-streaming-sdk/blob/develop/examples/simple_mic.py]()

Transcribe streaming 官方文档

[https://docs.aws.amazon.com/transcribe/latest/dg/streaming.html]()

Transcribe Java sdk sample from aws-doc

[https://github.com/awsdocs/aws-doc-sdk-examples/blob/b320aeae1a3e650bffc23f9584a26a7ca177cbb2/javav2/example_code/transcribe/src/main/java/com/amazonaws/transcribestreaming/TranscribeStreamingDemoApp.java]()

Amazon Corretto (JDK) 下载：

[https://docs.aws.amazon.com/corretto/latest/corretto-17-ug/downloads-list.html]()

Polly引擎和区域清单：

[https://docs.aws.amazon.com/zh_cn/polly/latest/dg/generative-voices.html]()

SSML标签语言可用引擎清单：

[https://docs.aws.amazon.com/zh_cn/polly/latest/dg/ssml.html]()




