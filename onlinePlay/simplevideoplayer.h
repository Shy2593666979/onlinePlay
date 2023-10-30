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
