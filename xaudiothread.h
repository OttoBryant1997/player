#ifndef XAUDIOTHREAD_H
#define XAUDIOTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <xdecode.h>
#include <xaudioplay.h>
#include <xresample.h>
#include <avthread.h>
struct AVCodecParameters;
/**
 * @brief The XAudioThread class
 * 音频线程类,内含音频播放7，音频解码7,音频转换对象(重采样),以及线程行为控制的接口
 * 通过push输入avpacket即可
 */
class XAudioThread : public AVThread
{
    Q_OBJECT
public:
    explicit XAudioThread(QObject *parent = nullptr);
    virtual ~XAudioThread();
    virtual void run() override;
    /**
     * @brief Open
     * @param params 无论成功与否都会回收
     * @return
     */
    virtual bool Open(AVCodecParameters* params,int sampleRate,int channels);
    /**
     * @brief Push 将音视频包放入播放pkt队列
     * @param pkt
     */
    virtual void Push(AVPacket *pkt) override;
    /**
     * @brief Pop 从pkt队列获得一包数据，并把头部丢掉
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
    /**
     * @brief setAudioTimeBase 设置音频时间基数
     * @param timebase
     */
    virtual void setAudioTimeBase(double timebase);
    /**
     * @brief mPts 正在播放的音频的pts
     */
    qint64 mPts = 0;
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
     * @brief setVolume
     * @param volume 0.0 - 1.0
     */
    virtual void setVolume(double volume);
    /**
     * @brief setPlaySpeed 0.1 - 3.0
     * @param volume
     */
    virtual void setPlaySpeed(double speed);
signals:
protected:
    double mAudioTimeBase = 0;
    XAudioPlay* mAudioPlayer = nullptr;
    XResample* mResample = nullptr;
    QMutex aMutex;
    double mVolume = 1.0;
    double mPlaySpeed = 1.0;
};

#endif // XAUDIOTHREAD_H
