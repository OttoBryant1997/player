#ifndef XAUDIOPLAY_H
#define XAUDIOPLAY_H

#include <QObject>
#include <QDebug>
/**
 * @brief The XAudioPlay class 工厂类
 */
class XAudioPlay : public QObject
{
    Q_OBJECT
public:
    explicit XAudioPlay(QObject *parent = nullptr);
    /**
     * @brief getInstance 可以改造成传参方式，获得不同的音频播放控件
     * @return
     */
    static XAudioPlay *getInstance();
    /**
     * @brief Open 打开音频播放7
     * @return
     */
    virtual bool Open() = 0;
    /**
     * @brief Close 关闭音频播放7
     */
    virtual void Close() = 0;
    /**
     * @brief FreeBytes 音频播放队列剩余可用空间
     * @return
     */
    virtual int FreeBytes() = 0;
    /**
     * @brief Write 向音频播放队列写入pcm数据
     * @param data
     * @param dataSize
     * @return
     */
    virtual bool Write(const unsigned char* data,int dataSize) = 0;
    /**
     * @brief getWaitngTimeMs 获得音频队列中的时间长度
     * @return
     */
    virtual qint64 getWaitngTimeMs() = 0;
    /**
     * @brief Clear 置空播放队列
     */
    virtual void Clear() = 0;
    /**
     * @brief SetPause
     * @param isPause
     */
    virtual void SetPause(bool isPause) = 0;
    /**
     * @brief setVolume
     * @param volume 0.0 - 1.0
     */
    virtual void setVolume(double volume) = 0;
    /**
     * @brief setPlaySpeed
     * @param speed
     */
    virtual void setPlaySpeed(double speed) = 0;
signals:

public:
    int mSampleRate = 44100;
    int mSampleSizeBits = 16;
    int mChannels = 2;
};

#endif // XAUDIOPLAY_H
