#ifndef XVIDEOTHREAD_H
#define XVIDEOTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <xdecode.h>
#include <xaudioplay.h>
#include <xresample.h>
#include <IVideoCall.h>
#include <avthread.h>
/**
 * @brief The XVideoThread class 视频线程
 * 内含视频播放窗口，视频解码7,以及线程行为控制的接口
 * 通过窗口句传递YUV数据
 */
class XVideoThread : public AVThread
{
    Q_OBJECT
public:
    explicit XVideoThread(QObject *parent = nullptr);
    virtual ~XVideoThread();
    virtual void run() override;
    /**
     * @brief Open
     * @param params 无论成功与否都会回收
     * @return
     */
    virtual bool Open(AVCodecParameters* params,int w,int h,IVideoCall* callback);

    /**
     * @brief setSyncTimeLine 以音频的pts作为同步基准
     * @param audioPts
     */
    virtual void setSyncTimeLine(qint64 audioPts);
    /**
     * @brief setVideoTimeBase 设置视频时间基数,frame.pts*timebase即为毫秒级的pts
     * @param timebase
     */
    virtual void setVideoTimeBase(qint64 timebase);
    /**
     * @brief Push 将音视频包放入播放队列
     * @param pkt
     */
    virtual void Push(AVPacket *pkt) override;
    /**
     * @brief Pop 从队列获得一包数据，并把头部丢掉
     * @return
     */
    virtual AVPacket * Pop() override;
    /**
     * @brief Clear 置空解码上下文和解码队列
     */
    virtual void Clear() override;
    /**
     * @brief Close 回收空间停止线程
     */
    virtual void Close() override;

    bool mIsPause = false;
    /**
     * @brief setPause 暂停线程
     * @param isPause
     */
    virtual void setPause(bool isPause)override;
    /**
     * @brief getDecoder 拿到解码7
     * @return
     */
    virtual XDecode * getDecoder() override;
    /**
     * @brief getVideoHandler 获得窗口句柄
     * @return
     */
    IVideoCall* getVideoHandler();
signals:
protected:
    qint64 mVideoTimeBase = 0;
    qint64 mSyncPts = 0;

    IVideoCall* mVideoCall = nullptr;
    QMutex vMutex;
};

#endif // XVIDEOTHREAD_H
