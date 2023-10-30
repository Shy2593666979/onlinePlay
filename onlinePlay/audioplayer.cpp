#include "audioplayer.h"

audioPlayer::audioPlayer()
{

}

audioPlayer::~audioPlayer()
{

}
void audioPlayer::play()
{
    start();
}
void audioPlayer::run()
{
    //AVFormatContext *pAVFormatCtx = avformat_alloc_context();
    if (_audioPath.isNull())
    {
        return;
    }

    char *file = _audioPath.toUtf8().data();


    //描述多媒体文件的构成及其基本信息
    AVFormatContext *pAVFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pAVFormatCtx, file, NULL, NULL) != 0)
    {
        std::cout<<"open Audiofile fail "<<std::endl;
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

    int audioStream = -1;
    for (int i = 0; i < pAVFormatCtx->nb_streams; ++i) {
        if (pAVFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = i;
            break;
        }
    }
    if (audioStream == -1) {
        qDebug() << "Audio stream not found";
        avformat_close_input(&pAVFormatCtx);
        return ;
        // Depending on your requirements, you might want to return or continue processing the video stream
    }

    AVCodecContext *pAudioCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pAudioCodecCtx, pAVFormatCtx->streams[audioStream]->codecpar);

    const AVCodec *pAudioCodec = avcodec_find_decoder(pAVFormatCtx->streams[audioStream]->codecpar->codec_id);
    if (!pAudioCodec) {
        qDebug() << "Audio codec not found";
        return;
    }
    avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL);

    /*
    QAudioFormat audioFormat;
    audioFormat.setSampleRate(pAudioCodecCtx->sample_rate);
    audioFormat.setChannelCount(pAudioCodecCtx->channels);
    audioFormat.setSampleSize(16);  // Assuming decoded to S16 (you might need to adjust based on your format)
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::SignedInt);

    QAudioOutput *audioOutput = new QAudioOutput(audioFormat);
    QIODevice *audioDevice = audioOutput->start();
    AVPacket packet;
    while(av_read_frame(pAVFormatCtx, &packet) >= 0)
    {
        if (packet.stream_index == audioStream) {
            int ret = avcodec_send_packet(codec_ctx, &packet);
                    if (ret < 0) {
                        fprintf(stderr, "Error sending a packet for decoding\n");
                        break;
                    }
                    while (ret >= 0) {
                        AVFrame* frame = av_frame_alloc();
                        ret = avcodec_receive_frame(codec_ctx, frame);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                            av_frame_free(&frame);
                            break;
                        } else if (ret < 0) {
                            fprintf(stderr, "Error during decoding\n");
                            av_frame_free(&frame);
                            break;
                        }
                        int samples = av_samples_get_buffer_size(NULL, codec_ctx->channels, frame->nb_samples, codec_ctx->sample_fmt, 0);
                        if (samples <= 0) {
                            av_frame_free(&frame);
                            continue;
                        }
                        memcpy(buffer[0], frame->data[0], samples);
                        if (codec_ctx->channels > 1) {
                            memcpy(buffer[1], frame->data[1], samples / 2);
                        }
                        av_audio_fifo_write(fifo, (void**)buffer, frame->nb_samples);
                        av_frame_free(&frame);
                    }
            }
            av_frame_free(&audioFrame);
        }
        av_packet_unref(&packet);
    }*/
    // 初始化音频缓冲区和FIFO
    int frame_size = pAudioCodecCtx->frame_size;
    AVAudioFifo* fifo = av_audio_fifo_alloc(pAudioCodecCtx->sample_fmt, pAudioCodecCtx->channels, frame_size);
    if (fifo == NULL) {
        fprintf(stderr, "Failed to allocate audio FIFO\n");
        avcodec_free_context(&pAudioCodecCtx);
        avformat_close_input(&pAVFormatCtx);
        return ;
    }
    uint8_t** buffer = NULL;
    if (av_samples_alloc_array_and_samples(&buffer, NULL, pAudioCodecCtx->channels, frame_size, pAudioCodecCtx->sample_fmt, 0) < 0) {
        fprintf(stderr, "Failed to allocate audio buffer\n");
        av_audio_fifo_free(fifo);
        avcodec_free_context(&pAudioCodecCtx);
        avformat_close_input(&pAVFormatCtx);
        return ;
    }

    // 解码音频流
    AVPacket packet;
    av_init_packet(&packet);
    while (av_read_frame(pAVFormatCtx, &packet) == 0) {
        if (packet.stream_index == audioStream) {
            int ret = avcodec_send_packet(pAudioCodecCtx, &packet);
            if (ret < 0) {
                fprintf(stderr, "Error sending a packet for decoding\n");
                break;
            }
            while (ret >= 0) {
                AVFrame* frame = av_frame_alloc();
                ret = avcodec_receive_frame(pAudioCodecCtx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    av_frame_free(&frame);
                    break;
                } else if (ret < 0) {
                    fprintf(stderr, "Error during decoding\n");
                    av_frame_free(&frame);
                    break;
                }
                int samples = av_samples_get_buffer_size(NULL, pAudioCodecCtx->channels, frame->nb_samples, pAudioCodecCtx->sample_fmt, 0);
                if (samples <= 0) {
                    av_frame_free(&frame);
                    continue;
                }
                memcpy(buffer[0], frame->data[0], samples);
                if (pAudioCodecCtx->channels > 1) {
                    memcpy(buffer[1], frame->data[1], samples / 2);
                }
                av_audio_fifo_write(fifo, (void**)buffer, frame->nb_samples);
                av_frame_free(&frame);
            }
        }
        av_packet_unref(&packet);
    }
    av_packet_unref(&packet);

    // 确定音频格式
    QAudioFormat format;
    format.setSampleRate(pAudioCodecCtx->sample_rate);
    format.setChannelCount(pAudioCodecCtx->channels);
    format.setSampleSize(16);  // 你可能需要根据你的音频数据进行调整
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioOutput* audioOutput = new QAudioOutput(format);
    QIODevice* device = audioOutput->start();

    // 从FIFO中读取音频数据并播放
    while (av_audio_fifo_size(fifo) > 0) {
        int frame_size = std::min(av_audio_fifo_size(fifo), pAudioCodecCtx->frame_size);
        av_audio_fifo_read(fifo, (void**)buffer, frame_size);
        int data_size = av_samples_get_buffer_size(NULL, pAudioCodecCtx->channels, frame_size, pAudioCodecCtx->sample_fmt, 0);
        device->write((const char*)buffer[0], data_size);
    }


    avcodec_free_context(&pAudioCodecCtx);
    avformat_close_input(&pAVFormatCtx);
    av_freep(&buffer[0]);
    av_freep(&buffer);
    av_audio_fifo_free(fifo);
    audioOutput->stop();
    delete audioOutput;

}
