#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <xdemuxthread.h>
#include <QResizeEvent>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE
#include<xvideowidget.h>
class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    void OpenFile();
    void onPlayOrPauseClicked();
    void onSliderPressed();
    void onSliderReleased();
    void onVolumeChanged(int);//0-99
    void onSpeedChanged(double);//0.1 - 3.0
private:
    XDemuxThread mDemuxThread;
    Ui::Widget *ui;
private:
    //刷新进度条
    void timerEvent(QTimerEvent *e)override;
    //窗口尺寸
    void resizeEvent(QResizeEvent *event) override;
    //双击满屏
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    //设置按钮显示内容 暂停显示play 播放显示pause
    void setPause(bool isPause);

    bool isSliderPressed = false;
//    bool posDebug = false;
};
#endif // WIDGET_H
