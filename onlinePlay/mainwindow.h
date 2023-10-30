#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <simplevideoplayer.h>
#include <audioplayer.h>
#include <QDebug>
#include <Qpainter>
#include <QMediaPlayer>
#include <QMediaService>
#include <synchapi.h>
#include <QMediaRecorder>
#include <QPushButton>
#include <QFileDialog>


extern "C"   //ffmpeg是采用c语言实现的 c++工程种 导入c语言
{
    //avcodec:编解码(最重要的库)
    #include <libavcodec/avcodec.h>
    //avformat:封装格式处理
    #include <libavformat/avformat.h>
    //swscale:视频像素数据格式转换
    #include <libswscale/swscale.h>
    //avdevice:各种设备的输入输出
    #include <libavdevice/avdevice.h>
    //avutil:工具库（大部分库都需要这个库的支持）
    #include <libavutil/avutil.h>
}


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    //void startPlay();

protected:
    void paintEvent(QPaintEvent *event);
private:
    Ui::MainWindow *ui;
    SimpleVideoPlayer *_simpleVp;
    audioPlayer * _simpleAp;
    QMediaPlayer *player;
    QImage _Qimg;

signals:
     void speedChanged(QString speed);
private slots:
    void sigGetVideoFrame(QImage img);
    void startPlay();
    void changeAudio(int volume);
    void openFileDialog();
    void pauseAudio();
    void nextVideo();
    void previousVideo();
    void onSpeedChanged();
};

#endif // MAINWINDOW_H
