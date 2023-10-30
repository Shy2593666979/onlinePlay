
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

### SimpleVideoPlayer.h
```c
#ifndef SIMPLEVIDEOPLAYER_H
#define SIMPLEVIDEOPLAYER_H

#include <QThread>
#include <QMainWindow>
#include <QImage>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/pixfmt.h"
    #include "libswscale/swscale.h"
    #include <libavutil/imgutils.h>
}

class SimpleVideoPlayer : public QThread
{
    Q_OBJECT

public:
    explicit SimpleVideoPlayer();
    ~SimpleVideoPlayer();

    void setVideo(const QString &videoPath)
    {
        this->_videoPath = videoPath;
    }

    inline void play();

    enum class PlayerState {
        Playing,
        Paused,
        Stopped
    };

    void togglePlayPause();
    void changeState();
    void setPlaybackSpeed(QString speed);
signals:
    void sigCreateVideoFrame(QImage image);

protected:
    void run();

private:
    QString _videoPath;
    PlayerState _state = PlayerState::Playing;
    QString speedVideo = "1x";
};

#endif // SIMPLEVIDEOPLAYER_H

```
### SimpleVideoPlayer.cpp
```c
#include "SimpleVideoPlayer.h"
#include <iostream>
#include <QDebug>



SimpleVideoPlayer::SimpleVideoPlayer()
{

}

SimpleVideoPlayer::~SimpleVideoPlayer()
{

}

void SimpleVideoPlayer::play()
{
    start();
}

void SimpleVideoPlayer::togglePlayPause()
{
    if (_state == PlayerState::Playing) {
            _state = PlayerState::Paused;
     } else {
            _state = PlayerState::Playing;
     }
}

void SimpleVideoPlayer::changeState()
{
    if(_state == PlayerState::Playing || _state == PlayerState::Paused)
    {
        _state = PlayerState::Stopped;
    }
}

void SimpleVideoPlayer::setPlaybackSpeed(QString speed)
{
    speedVideo = speed;
}

void SimpleVideoPlayer::run()
{
    _state = PlayerState::Playing;
    if (_videoPath.isNull())
    {
        return;
    }

    char *file1 = _videoPath.toUtf8().data();

    //编解码器的注册，最新版本不需要调用
    //av_register_all();

    //描述多媒体文件的构成及其基本信息
    AVFormatContext *pAVFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pAVFormatCtx, file1, NULL, NULL) != 0)
    {
        std::cout<<"open file fail"<<std::endl;
        avformat_free_context(pAVFormatCtx);
        return;
    }

    //读取一部分视音频数据并且获得一些相关的信息
    if (avformat_find_stream_info(pAVFormatCtx, NULL) < 0)
    {
        std::cout<<"avformat find stream fail"<<std::endl;
        avformat_close_input(&pAVFormatCtx);
        return;
    }

    int iVideoIndex = -1;

    for (uint32_t i = 0; i < pAVFormatCtx->nb_streams; ++i)
    {
        //视频流
        if (pAVFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            iVideoIndex = i;
            break;
        }
    }

    if (iVideoIndex == -1)
    {
        std::cout<<"video stream not find"<<std::endl;
        avformat_close_input(&pAVFormatCtx);
        return;
    }

    //获取视频流的编解码器上下文的数据结构
    AVCodecContext *pAVCodecCtx = avcodec_alloc_context3(NULL);
    if (!pAVCodecCtx) {
        fprintf(stderr, "Could not allocate codec context\n");
        exit(1);
    }

    if (avcodec_parameters_to_context(pAVCodecCtx, pAVFormatCtx->streams[iVideoIndex]->codecpar) < 0) {
        fprintf(stderr, "Could not copy codec parameters\n");
        exit(1);
    }

    //编解码器信息的结构体
    const AVCodec *pAVCodec = avcodec_find_decoder(pAVCodecCtx->codec_id);

    if (pAVCodec == NULL)
    {
        std::cout<<"not find decoder"<<std::endl;
        return;
    }

    //初始化一个视音频编解码器
    if (avcodec_open2(pAVCodecCtx, pAVCodec, NULL) < 0)
    {
        std::cout<<"avcodec_open2 fail"<<std::endl;
        return;
    }

    //AVFrame 存放从AVPacket中解码出来的原始数据
    AVFrame *pAVFrame = av_frame_alloc();
    AVFrame *pAVFrameRGB = av_frame_alloc();

    //用于视频图像的转换,将源数据转换为RGB32的目标数据
    SwsContext *pSwsCtx = sws_getContext(pAVCodecCtx->width, pAVCodecCtx->height, pAVCodecCtx->pix_fmt,
                                         pAVCodecCtx->width, pAVCodecCtx->height, AV_PIX_FMT_RGB32,
                                         SWS_BICUBIC, NULL, NULL, NULL);
    int iNumBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, pAVCodecCtx->width, pAVCodecCtx->height,1);

    uint8_t *pRgbBuffer = (uint8_t *)(av_malloc(iNumBytes * sizeof(uint8_t)));
    //为已经分配的空间的结构体AVPicture挂上一段用于保存数据的空间
    av_image_fill_arrays(pAVFrameRGB->data, pAVFrameRGB->linesize, pRgbBuffer, AV_PIX_FMT_RGB32, pAVCodecCtx->width, pAVCodecCtx->height,1);
    //AVpacket 用来存放解码之前的数据
    AVPacket packet;
    av_new_packet(&packet, pAVCodecCtx->width * pAVCodecCtx->height);

    //读取码流中视频帧
    while (av_read_frame(pAVFormatCtx, &packet) >= 0)
    {
        if (_state == PlayerState::Stopped) {
            qDebug()<<"当前音乐已终止";
            break;
        }

        if (_state == PlayerState::Paused) {
            QThread::msleep(600); // Sleep for 600ms 线程休息
            continue;
        }

        if (packet.stream_index != iVideoIndex)
        {
            av_packet_unref(&packet);
            continue;
        }

        // 发送数据包到解码器
        if (avcodec_send_packet(pAVCodecCtx, &packet) < 0)
        {
            std::cout << "Error sending packet for decoding." << std::endl;
            av_packet_unref(&packet);
            continue;
        }

        // 从解码器接收解码后的帧
        while (avcodec_receive_frame(pAVCodecCtx, pAVFrame) == 0)
        {
            // 转换像素格式
            sws_scale(pSwsCtx, (uint8_t const * const *)pAVFrame->data, pAVFrame->linesize, 0, pAVCodecCtx->height, pAVFrameRGB->data, pAVFrameRGB->linesize);

            // 构造QImage
            QImage img(pRgbBuffer, pAVCodecCtx->width, pAVCodecCtx->height, QImage::Format_RGB32);

            // 绘制QImage
            emit sigCreateVideoFrame(img);
        }

        av_packet_unref(&packet);
        double fps = av_q2d(pAVFormatCtx->streams[iVideoIndex]->r_frame_rate);
        int delay = 900.0 / fps;
        if(speedVideo == "2x"){
            msleep(delay / 2);
        }
        else if(speedVideo == "0.5x"){
            msleep(delay * 2);
        }
        else{
            msleep(delay);
        }
        //msleep(15);
    }

    //资源回收
    /*av_free(pAVFrame);
    av_free(pAVFrameRGB);
    sws_freeContext(pSwsCtx);
    avcodec_close(pAVCodecCtx);
    avformat_close_input(&pAVFormatCtx);*/
    av_frame_free(&pAVFrame);
    av_frame_free(&pAVFrameRGB);
    sws_freeContext(pSwsCtx);
    avcodec_free_context(&pAVCodecCtx);
    avformat_close_input(&pAVFormatCtx);

}


```
## 开发音频
QMediaPlayer是Qt自带的多媒体播放器类，它提供了简单易用的接口，用于播放音频和视频文件。QMediaPlayer支持多种音视频格式，并且可以通过网络流式传输音视频内容。它具有播放、暂停、停止、跳转和调整音量等常见的播放控制功能。此外，QMediaPlayer还提供了信号和槽机制，可用于捕获播放状态的变化和处理用户交互事件。
```c
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("在线播放平台");

    //comboBox 数据居中
    ui->comboBox->setStyleSheet("QComboBox QAbstractItemView { text-align: center; }");
    /*QMediaPlayer * m_player;
    m_player = new QMediaPlayer(this);
    QString strPlayer = "D:/QT.study/onlinePlay/qiFengLe";
    m_player->setMedia(QUrl::fromLocalFile(strPlayer));
    m_player->setVolume(50);
    m_player->play();*/

    //QPushButton *openFileButton = new QPushButton("打开文件", this);
    //setCentralWidget(openFileButton);

    connect(ui->openFileButton, &QPushButton::clicked, this, &MainWindow::openFileDialog);


    _simpleVp = new SimpleVideoPlayer();
    // 创建媒体播放器
    player = new QMediaPlayer;
    //_simpleAp = new audioPlayer();

    connect(_simpleVp,SIGNAL(sigCreateVideoFrame(QImage)),this,SLOT(sigGetVideoFrame(QImage)));
    connect(ui->Slider,&QSlider::valueChanged,this,&MainWindow::changeAudio);
    connect(ui->startBtn,&QPushButton::clicked,this,&MainWindow::startPlay);
    connect(ui->pauseButton , &QPushButton::clicked, this->_simpleVp, &SimpleVideoPlayer::togglePlayPause);

    connect(ui->boxBS, &QComboBox::currentTextChanged, this, &MainWindow::onSpeedChanged);


    //下一首歌曲
    connect(ui->nextButton,&QPushButton::clicked,this->_simpleVp,&SimpleVideoPlayer::changeState);
    connect(ui->nextButton,&QPushButton::clicked,this,&MainWindow::nextVideo);

    //上一首歌曲
    connect(ui->previousBtn,&QPushButton::clicked,this->_simpleVp,&SimpleVideoPlayer::changeState);
    connect(ui->previousBtn,&QPushButton::clicked,this,&MainWindow::previousVideo);

    //暂停播放
    connect(ui->pauseButton,&QPushButton::clicked,this,&MainWindow::pauseAudio);

    //增加MV曲目
    ui->comboBox->addItem("起风了----------买辣椒也用券");
    ui->comboBox->addItem("悬溺------------葛东琪");
    ui->comboBox->addItem("平凡之路--------朴树");
    ui->comboBox->addItem("把回忆拼好给你---------王贰浪");
    ui->comboBox->addItem("带我去找夜生活---------告五人");

    //增加倍速
    ui->boxBS->addItem("1x");
    ui->boxBS->addItem("0.5x");
    ui->boxBS->addItem("2x");

    connect(this, &MainWindow::speedChanged, _simpleVp, &SimpleVideoPlayer::setPlaybackSpeed);
    //视频文件路径
    videoStr["起风了----------买辣椒也用券"] = "D:/QT.music/video/qifengle.mkv";
    videoStr["平凡之路--------朴树"] = "D:/QT.music/video/pingfanzhilu.mkv";
    videoStr["带我去找夜生活---------告五人"] = "D:/QT.music/video/yeshenghuo.mkv";
    videoStr["把回忆拼好给你---------王贰浪"] = "D:/QT.music/video/huiyi.mkv";
    videoStr["悬溺------------葛东琪"] = "D:/QT.music/video/xuanni.mkv";

    //音乐文件路径
    audioStr["起风了----------买辣椒也用券"] = "D:/QT.music/audio/qifengle1.mp3";
    audioStr["平凡之路--------朴树"] = "D:/QT.music/audio/pingfanzhilu.mp3";
    audioStr["带我去找夜生活---------告五人"] = "D:/QT.music/audio/yeshenghuo.mp3";
    audioStr["把回忆拼好给你---------王贰浪"] = "D:/QT.music/audio/huiyi.mp3";
    audioStr["悬溺------------葛东琪"] = "D:/QT.music/audio/xuanni.mp3";

    QIcon icon;
    icon.addFile(tr(":/qrs/4.png"));
    ui->pauseButton->setIconSize(QSize(120,40));
    ui->pauseButton->setIcon(icon);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete _simpleVp;
    delete player;
}

void MainWindow::startPlay()
{
    //获得当前选择歌曲
    QString Str = ui->comboBox->currentText();
    qDebug()<<Str;

    // 加载音乐文件
    QString currentAudio = audioStr[Str];
   // player->setMedia(QUrl::fromLocalFile(currentAudio));

    //0~100音量范围,默认是100
    ui->Slider->setValue(50);
    player->setVolume(50);

    // 加载视频文件
    QString currentVideo = videoStr[Str];
    //_simpleVp->setVideo(currentVideo);

    /*
    _simpleAp->setAudio("D:/QT.study/onlinePlay/qifengle1.mp3");
    开始播放，QThread类的成员函数
    _simpleAp->start();
    */

    //开始播放
    player->play();
    _simpleVp->start();


}
```
因为需要里面需要打开的文件位置，所以打开时需要更改位置
![在这里插入图片描述](https://img-blog.csdnimg.cn/3926d2a5fec843fa9b28a6e37ed397b9.png)

## 添加功能
我们目前只是完成了打开视频和音频的功能，还需要完善一下功能：
- 打开文件夹
- 播放暂停
- 播放下一首歌曲
- 开启倍速模式
- 开启调节音量模式


### 打开文件夹
```c
void MainWindow::openFileDialog()
{
    //QString filePath = QFileDialog::getOpenFileName(this, "Open Song", "", "Audio Files (*.mp3 *.wav);;All Files (*)");
    QString filePath = QFileDialog::getOpenFileName(this, "Open Video", "", "Video Files (*.mp4 *.avi *.mkv *.mov);;All Files (*)");
    ui->openFilePath->setText(filePath);
    if (!filePath.isEmpty())
    {
        // 这里处理用户选择的歌曲，例如将其传递给播放器等。
        qDebug() << "Selected song:" << filePath;
        _simpleVp->setVideo(filePath);
        QString str;
        for(auto it = videoStr.begin();it != videoStr.end();it++)
        {
            if(it->second == filePath)
            {
                str = it->first;
                break;
            }
        }
        player->setMedia(QUrl::fromLocalFile(audioStr[str]));
    }
}
```
### 播放暂停
```c
void MainWindow::pauseAudio()
{
    QIcon icon;

    if (player->state() == QMediaPlayer::PlayingState) {
        icon.addFile(tr(":/qrs/3.png"));
        player->pause();
        //ui->pauseButton->setText("Play");
        ui->pauseButton->setIconSize(QSize(120,40));
        ui->pauseButton->setStyleSheet("background-color: transparent;");
        ui->pauseButton->setFlat(true);
        ui->pauseButton->setIcon(icon);
    } else {
        icon.addFile(tr(":/qrs/4.png"));
        player->play();
        //ui->pauseButton->setText("Pause");

        ui->pauseButton->setIconSize(QSize(120,40));
        ui->pauseButton->setStyleSheet("background-color: transparent;");
        ui->pauseButton->setFlat(true);
        ui->pauseButton->setIcon(icon);
    }
}

```
### 播放上下一首
```c
void MainWindow::nextVideo() //播放下一首歌曲
{
    int idx = 0;
    QString fileStr = ui->openFilePath->text();
    for(auto it = videoStr.begin();it != videoStr.end();it++)
    {
        if(it->second == fileStr)
        {
            break;
        }
        idx++;
    }
    idx = (idx + 1) % 5;
    int idx1 = idx;
    QString videoFile;                 //= (videoStr.begin() + idx)->second;
    QString audioFile;                 //= (audioStr.begin() + idx)->second;
    for(auto it = videoStr.begin();it != videoStr.end();it++)
    {
        if(idx == 0)
        {
            videoFile = it->second;
            break;
        }
        idx--;
    }
    for(auto it = audioStr.begin();it != audioStr.end();it++)
    {
        if(idx1 == 0)
        {
            audioFile = it->second;
            break;
        }
        idx1--;
    }
    ui->openFilePath->setText(videoFile);
    _simpleVp->setVideo(videoFile);
    player->setMedia(QUrl::fromLocalFile(audioFile));
    _simpleVp->start();
    player->play();
}

//实现上一首歌曲功能
void MainWindow::previousVideo()
{
    int idx = 0;
    QString fileStr = ui->openFilePath->text();
    for(auto it = videoStr.begin();it != videoStr.end();it++)
    {
        if(it->second == fileStr)
        {
            break;
        }
        idx++;
    }
    idx = (idx - 1 + 5) % 5;
    int idx1 = idx;
    QString videoFile;                 //= (videoStr.begin() + idx)->second;
    QString audioFile;                 //= (audioStr.begin() + idx)->second;
    for(auto it = videoStr.begin();it != videoStr.end();it++)
    {
        if(idx == 0)
        {
            videoFile = it->second;
            break;
        }
        idx--;
    }
    for(auto it = audioStr.begin();it != audioStr.end();it++)
    {
        if(idx1 == 0)
        {
            audioFile = it->second;
            break;
        }
        idx1--;
    }
    ui->openFilePath->setText(videoFile);
    _simpleVp->setVideo(videoFile);
    player->setMedia(QUrl::fromLocalFile(audioFile));
    _simpleVp->start();
    player->play();
}
```
### 选择倍速
```c
void MainWindow::onSpeedChanged() //选择不同倍速时，会给视频和音频发射信号
{
    QString SpeedStr = ui->boxBS->currentText();
    if(SpeedStr == "2x"){
        player->setPlaybackRate(2.0);
    }
    else if(SpeedStr == "0.5x")
    {
        player->setPlaybackRate(0.5);
    }
    else {
        player->setPlaybackRate(1.0);
    }
    emit speedChanged(SpeedStr);
}
```
## 效果展示
![在这里插入图片描述](https://img-blog.csdnimg.cn/d6a914a40ec0418f8fb6fb2c26abe826.png)
![在这里插入图片描述](https://img-blog.csdnimg.cn/6d92701ffaff434e8a634ca9d54f9d01.png)

![在这里插入图片描述](https://img-blog.csdnimg.cn/80d3bddad04343d6b630a2572bad29f2.png)


