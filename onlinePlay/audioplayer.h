#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QThread>
#include <QMainWindow>
#include <QAudioFormat>
#include <QAudioOutput>
#include <iostream>
#include <QDebug>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/pixfmt.h"
    #include "libswscale/swscale.h"
    #include <libavutil/imgutils.h>
    #include <libavutil/audio_fifo.h>
}

class audioPlayer : public QThread
{
    Q_OBJECT

public:
    explicit audioPlayer();
    ~audioPlayer();

    void setAudio(const QString &audioPath)
    {
        this->_audioPath = audioPath;
    }

    inline void play();

signals:
    void sigCreateAudioFrame(QImage image);

protected:
    void run();

private:
    QString _audioPath;
};

#endif // AUDIOPLAYER_H
