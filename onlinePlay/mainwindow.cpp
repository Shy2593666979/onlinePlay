#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <map>
#include <windows.h>
#include <QPushButton>


std::map<QString,QString>videoStr;
std::map<QString,QString>audioStr;
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
void MainWindow::changeAudio(int volume)
{
    player->setVolume(volume);
}
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

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //painter.setBrush(Qt::black);
    painter.drawRect(0, 0, this->width(), this->height());

    if (_Qimg.size().width() <= 0)
        return;

    //比例缩放
    QImage img = _Qimg.scaled(this->size(),Qt::KeepAspectRatio);
    int x = this->width() - img.width();
    int y = this->height() - img.height();

    x /= 2;
    y /= 2;

    //QPoint(x,y)为中心绘制图像
    painter.drawImage(QPoint(x,y),img);
}

void MainWindow::sigGetVideoFrame(QImage img)
{
    _Qimg = img;
    //paintEvent
    update();
}
