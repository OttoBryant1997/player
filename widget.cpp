#include "widget.h"
#include "ui_widget.h"
#include<QFileDialog>
#include<QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    mDemuxThread.Start();
    startTimer(40);
}

Widget::~Widget()
{
    delete ui;
    mDemuxThread.Close();
}


void Widget::OpenFile()
{
    QString file = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择视频文件"));
    qDebug() << "file = " <<file;
    if(file.isEmpty())return;
    this->setWindowTitle(file);
    //测试rtmp用
//    mDemuxThread.Open("rtmp://58.200.131.2:1935/livetv/cctv6",ui->video);
//    return ;
    if(!mDemuxThread.Open(file.toLocal8Bit(),ui->video))
    {
        QMessageBox::information(0,"Error","OpenFile failed.");
        return ;
    }
    setPause(mDemuxThread.mIsPause);
}

void Widget::onPlayOrPauseClicked()
{
    mDemuxThread.setPause(!mDemuxThread.mIsPause);
    setPause(mDemuxThread.mIsPause);
}

void Widget::onSliderPressed()
{
    isSliderPressed = true;
}

void Widget::onSliderReleased()
{
    double pos = (double)ui->playPos->value() / ui->playPos->maximum();
    mDemuxThread.Seek(pos);
    isSliderPressed = false;
}

void Widget::onVolumeChanged(int volume)
{
    qDebug() << "volume =" << volume;
    double volumePercent = (double) volume / ui->audioVolume->maximum();
    mDemuxThread.setVolume(volumePercent);
}

void Widget::onSpeedChanged(double playSpeed)
{
//    mDemuxThread.setPlaySpeed(playSpeed);
    qDebug() << "playSpeed =" << playSpeed;
}

void Widget::timerEvent(QTimerEvent *)
{
    if(isSliderPressed)return;
    if(mDemuxThread.mProgressTotalMs > 0)
    {
        double pos = (double)mDemuxThread.mProgressPtsMs / mDemuxThread.mProgressTotalMs;
        ui->playPos->setValue(pos*(ui->playPos->maximum() - ui->playPos->minimum()));
    }

}

void Widget::resizeEvent(QResizeEvent *event)
{
//    qDebug() << "event = " <<event->size();
    ui->video->resize(event->size());
    ui->playPos->move(0,this->height() - 20);
    ui->playPos->resize(this->width(),10);
    ui->playOrPause->move(this->width()/2,this->height()-50);
    ui->openFile->move(0,this->height() - 50);

    ui->playSpeed->move(this->width()-100,this->height()-50);

    ui->audioVolume->move(this->width()-20,20);
    ui->audioVolume->resize(5,this->height() - 50);
}

void Widget::mouseDoubleClickEvent(QMouseEvent *)
{
    if(isFullScreen())
    {
        showNormal();
        ui->openFile->setVisible(true);

    }
    else
    {
        showFullScreen();
        ui->openFile->setVisible(false);
    }
}

void Widget::setPause(bool isPause)
{
    if(isPause)
        ui->playOrPause->setText("Play");
    else
        ui->playOrPause->setText("Pause");
}

