#include "xaudioplay.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <QMutex>
class EasyAudioPlay:public XAudioPlay
{
public:
    bool Open() override;
    bool Write(const unsigned char* data,int dataSize) override;
    void Close() override;
    int FreeBytes() override;
    qint64 getWaitngTimeMs() override;
    virtual void Clear() override;
    virtual void SetPause(bool isPause) override;
    virtual void setVolume(double volume) override;
    virtual void setPlaySpeed(double speed) override;
    ~EasyAudioPlay();
private:
    QAudioOutput* mOutput;
    QIODevice * mIO;
    QMutex apMutex;
    QAudioFormat mFmt;
    double mPlaySpeed = 1.0;
};

/*------------------------------------------------*/
XAudioPlay::XAudioPlay(QObject *parent) : QObject(parent)
{

}

XAudioPlay *XAudioPlay::getInstance()
{
    static EasyAudioPlay play;
    return &play;
}

bool EasyAudioPlay::Open()
{
    //音频格式信息
    mFmt.setSampleRate(mSampleRate);//*mPlaySpeed);//速度控制
    mFmt.setSampleSize(mSampleSizeBits);//没设置对有杂音
    mFmt.setCodec("audio/pcm");
    mFmt.setChannelCount(mChannels);
    mFmt.setByteOrder(QAudioFormat::LittleEndian);
    mFmt.setSampleType(QAudioFormat::SignedInt);//UnSignedInt改音量又杂音
    //播放输出
    apMutex.lock();
    mOutput = new QAudioOutput(mFmt);
    /** from 0.0 (silence) to 1.0 (full volume),设置高音量效果较好 */
    mOutput->setVolume(1);
    mIO = mOutput->start();//开始播放
    apMutex.unlock();
    if(mIO)return true;
    else return false;
}

bool EasyAudioPlay::Write(const unsigned char *data,int dataSize)
{
    if(!data || dataSize <= 0)
    {
        return false;
    }
    apMutex.lock();
    if(!mOutput || !mIO)
    {
        apMutex.unlock();
        return false;
    }
    int writeSize = mIO->write((const char*)data,dataSize);
    apMutex.unlock();
    if(writeSize != dataSize)
        return false;
    else return true;
}


void EasyAudioPlay::Close()
{
    apMutex.lock();
    if(mIO)
    {
        mIO->close();
        mIO = nullptr;
    }
    if(mOutput)
    {
        mOutput->stop();
        delete mOutput;
        mOutput = nullptr;
    }
    apMutex.unlock();
}

int EasyAudioPlay::FreeBytes()
{
    apMutex.lock();
    if(!mOutput)
    {
        apMutex.unlock();
        return -1;
    }
    int freeBytes = mOutput->bytesFree();
    apMutex.unlock();
    return freeBytes;
}

qint64 EasyAudioPlay::getWaitngTimeMs()
{
    apMutex.lock();
    if(!mOutput)
    {
        apMutex.unlock();
        return 0;
    }
    //音频队列还未播放字节数
    double WaitSize =
    mOutput->bufferSize() - mOutput->bytesFree();
    //每秒播放字节数
    double PlaySizePerSec = mSampleRate*mSampleSizeBits*mChannels / 8;
    qint64 WaitTimeMs = 0;
//    qDebug() <<  "WaitSize" << WaitSize<<mOutput->bufferSize() << mOutput->bytesFree();
    if(PlaySizePerSec <= 0)
    {
        apMutex.unlock();
        return WaitTimeMs;
    }
    WaitTimeMs = WaitSize / PlaySizePerSec * 1000;
    apMutex.unlock();
    return  WaitTimeMs;
}

void EasyAudioPlay::Clear()
{
    apMutex.lock();
    if(mIO)
    {
        mIO->reset();
    }
    apMutex.unlock();
}

void EasyAudioPlay::SetPause(bool isPause)
{
    apMutex.lock();
    if(mOutput)
    {
        if(isPause)
        {
            mOutput->suspend();
        }
        else
        {
            mOutput->resume();
        }
    }
    apMutex.unlock();
}



void EasyAudioPlay::setVolume(double volume)
{
    apMutex.lock();
    if(volume < 0.0)volume = 0.0;
    if(volume > 1.0)volume = 1.0;

    mOutput->setVolume(volume);
    apMutex.unlock();
}

void EasyAudioPlay::setPlaySpeed(double speed)
{
    apMutex.lock();
    if(speed < 0.1)speed = 0.1;
    if(speed > 3.0)speed = 3.0;
    mPlaySpeed = speed;
    mFmt.setSampleRate(mSampleRate*mPlaySpeed);

//    if(mOutput) delete  mOutput;
//    mOutput = new QAudioOutput(mFmt);
    apMutex.unlock();
}



EasyAudioPlay::~EasyAudioPlay()
{
    delete mOutput;
}
