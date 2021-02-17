#ifndef XDEMUXTHREAD_H
#define XDEMUXTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <xdemux.h>
#include <xaudiothread.h>
#include <xvideothread.h>
#include <IVideoCall.h>
/**
 * @brief The XDemuxThread class 文件核心线程,掌管文件操作,音视频操作
 */
class XDemuxThread : public QThread
{
    Q_OBJECT
public:
    explicit XDemuxThread(QObject *parent = nullptr);
    virtual ~XDemuxThread();
    /**
     * @brief Open 音视频处理线程和音视频解码7等对象的创建
     * @param url
     * @param call UI句柄
     * @return
     */
    virtual bool Open(const char* url,IVideoCall* call);
    /**
     * @brief Start 开始音视频线程
     */
    virtual void Start();
    /**
     * @brief run 解封装并调度音视频线程
     */
    void run()override;
    /**
     * @brief Close 关闭所有线程和资源
     */
    virtual void Close();
public:
    qint64 mProgressPtsMs = 0;
    qint64 mProgressTotalMs = 0;
    bool mIsPause = false;
    void setPause(bool isPause);
    /**
     * @brief Clear 刷空任音视频务队列
     */
    void Clear();
    /**
     * @brief Seek 刷空任音视频务队列并seek到指定百分比位置
     * @param pos
     */
    void Seek(double pos);
    /**
     * @brief setPlaySpeed 设置播放速度
     * @param speed
     */
    void setPlaySpeed(double speed);
    /**
     * @brief setVolume 设置音量
     * @param volume
     */
    void setVolume(double volume);
signals:
protected:

    XDemux* mDemux = nullptr;
    XAudioThread* mAudioThread = nullptr;
    XVideoThread* mVideoThread = nullptr;
    QMutex mutex;
    bool mExit = false;
    double mPlaySpeed = 1.0;
    double mVolume = 1.0;
private:
    bool refreshFrame(double seekPts);
};

#endif // XDEMUXTHREAD_H
