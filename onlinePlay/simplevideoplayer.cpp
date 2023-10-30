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
