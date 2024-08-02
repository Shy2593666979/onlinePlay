
# QT实现在线流媒体播放平台
## 简介
Qt是一种流行的C++开发框架，它提供了用于构建图形用户界面（GUI）和跨平台应用程序的工具和库。Qt具有广泛的应用范围，可以用于开发桌面应用程序、移动应用程序和嵌入式系统等。Qt具有丰富的多媒体功能，包括音频和视频处理能力。

在使用Qt进行在线流媒体开发时，我们可以结合FFmpeg和QMediaPlayer来实现强大的音视频处理和播放功能。首先，可以使用FFmpeg解码音视频文件或流，提取音频和视频数据。然后，可以使用QMediaPlayer加载音频和视频数据，并通过其播放控制接口实现音视频的同步播放。此外，可以使用Qt的图形界面开发能力，创建用户友好的界面，显示视频内容，并提供交互控制。

## 开发视频
FFmpeg是一个开源的多媒体框架，它提供了一套用于处理音视频的工具和库。FFmpeg支持广泛的音视频格式和编解码器，能够对音视频进行解码、编码、转换和处理等操作。在在线流媒体应用中，FFmpeg通常用于处理音视频文件或流，提取其中的音频和视频数据，进行解码和编码，以及应用特效和滤镜等。
### ffmpeg下载
如果你的qt版本是32位，下载的FFmpeg是64位，则可能识别不了函数
在这里我提供了64位和32位的FFmpeg下载链接

32位链接： [https://pan.baidu.com/s/1wBAv6yYYa_9n64wzmHdO2w](https://pan.baidu.com/s/1wBAv6yYYa_9n64wzmHdO2w)
提取码：0703

64位链接： [https://pan.baidu.com/s/1aEHWpbTQkhVA30KtfviYjA](https://pan.baidu.com/s/1aEHWpbTQkhVA30KtfviYjA)
提取码：0703


  
因为需要里面需要打开的文件位置，所以打开时需要更改位置
![在这里插入图片描述](https://img-blog.csdnimg.cn/3926d2a5fec843fa9b28a6e37ed397b9.png)

## 添加功能
我们目前只是完成了打开视频和音频的功能，还需要完善一下功能：
- 打开文件夹
- 播放暂停
- 播放下一首歌曲
- 开启倍速模式
- 开启调节音量模式


## 效果展示
![在这里插入图片描述](https://img-blog.csdnimg.cn/6d92701ffaff434e8a634ca9d54f9d01.png)

![在这里插入图片描述](https://img-blog.csdnimg.cn/80d3bddad04343d6b630a2572bad29f2.png)


