#ifndef AVTHREAD_H
#define AVTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <xdecode.h>
/**
 * @brief The AVThread class 音视频线程父类
 */
class AVThread :  public QThread
{
    Q_OBJECT
public:
    explicit AVThread(QObject *parent = nullptr);
    virtual ~AVThread();
    /**
     * @brief Push 将音视频包放入播放队列
     * @param pkt
     */
    virtual void Push(AVPacket* pkt) = 0;
    /**
     * @brief Pop 从队列获得一包数据，并把头部丢掉
     * @return
     */
    virtual AVPacket* Pop() = 0;
    /**
     * @brief Clear 置空解码队列 和包队列
     */
    virtual void Clear() = 0;
    /**
     * @brief Close 回收空间停止线程
     */
    virtual void Close() = 0;

    /**
     * @brief setPause 暂停线程
     * @param isPause
     */
    virtual void setPause(bool isPause) = 0;

    /**
     * @brief getDecoder 拿到解码7
     * @return
     */
    virtual XDecode* getDecoder()=0;
//    bool mIsPause = false;
//    void setPause(bool isPause);
protected:
    XDecode* mDecoder = nullptr;
    QList<AVPacket*> mPkts;
    int mMaxList = 100;
    bool mThreadShouldExit = false;
//    QMutex mMutex;//锁不能共用
signals:

};

#endif // AVTHREAD_H
