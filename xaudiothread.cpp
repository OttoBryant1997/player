#include "xaudiothread.h"

XAudioThread::XAudioThread(QObject *parent) : AVThread(parent)
{
    mDecoder = new XDecode(CodecType::audio);
    mAudioPlayer = XAudioPlay::getInstance();
    mResample = new XResample();
}

XAudioThread::~XAudioThread()
{
    qDebug() << "XAudioThread::~XAudioThread()";

}

void XAudioThread::run()
{
    qDebug() << "enter XAudioThread::run()";
//    QThread::setPriority(QThread::HighestPriority);
    unsigned char* pcmData =
            new unsigned char[1024*256];
    while(!mThreadShouldExit)
    {
        aMutex.lock();
        if(mIsPause)
        {//暂停阻塞
            aMutex.unlock();
            yieldCurrentThread();
            continue;
        }
        bool ret = mDecoder->Send(Pop());
        if(!ret)
        {
            aMutex.unlock();
            msleep(1);
            continue;
        }
        //一次send多次recv
        while (!mThreadShouldExit)
        {
            AVFrame* frame = mDecoder->Recv();
            if(!frame)break;

            //队列尾部的时间长度减掉队列中的时间长度获得队列头部也就是正在播放的音频的时间
            mPts = mDecoder->mPts - mAudioPlayer->getWaitngTimeMs();
//            qDebug() <<  "mDecoder->mPts  = " << mDecoder->mPts
//                      << mAudioPlayer->getWaitngTimeMs() ;
            //重采样
            int size = mResample->Resample(frame,pcmData);

//            qDebug() <<  "1 ";
            //播放音频
            while (!mThreadShouldExit)
            {
//                qDebug() <<  "2 ";
                if(size <= 0)break;
                //缓冲不足,等待
                if(mAudioPlayer->FreeBytes() < size )
                {
                    msleep(1);
//                    qDebug() <<  "3 " <<mAudioPlayer->FreeBytes() - size << mIsPause;
                    continue;
                }
                else if (mIsPause) {//丢掉一帧音频,进入暂停阻塞
                    break;
                }
                else
                {
                    mAudioPlayer->Write(pcmData,size);
                    break;
                }
            }
        }
        aMutex.unlock();
    }
    delete[] pcmData;
}

bool XAudioThread::Open(AVCodecParameters *params,int sampleRate,int channels)
{
    if(!params)return false;
    Clear();
    aMutex.lock();
    mPts = 0;
    mAudioTimeBase = 0;

    bool ret = true;
    if(!mResample->Open(params,false))
    {
        ret = false;
        qDebug() << "mResample->Open(params,false) false";
    }
    mAudioPlayer->mSampleRate = sampleRate;
    mAudioPlayer->mChannels = channels;
    if(!mAudioPlayer->Open())
    {
        ret = false;
        qDebug() << "mAudioPlayer->Open(); false";
    }
    if(!mDecoder->Open(params))
    {//在decoder里面会释放params
        ret = false;
    }
    aMutex.unlock();
    return ret;
}

void XAudioThread::Push(AVPacket *pkt)
{
//    qDebug() << "XAudioThread::Push()";
    if(!pkt)return;
    while(!mThreadShouldExit)
    {//阻塞
        aMutex.lock();
        if(mPkts.size() < mMaxList)
        {
//            qDebug() << "XAudioThread::Push()";
            mPkts.push_back(pkt);
            aMutex.unlock();
            break;
        }
//        qDebug() << "XAudioThread::Push() wait";
        aMutex.unlock();
        msleep(1);
    }
}

AVPacket *XAudioThread::Pop()
{
//    qDebug() << "XAudioThread::Pop()";
    if(mPkts.isEmpty())
    {
        return nullptr;
    }
    AVPacket * pkt = mPkts.takeFirst();
    return pkt;
}

void XAudioThread::Clear()
{
    aMutex.lock();
    mDecoder->Clear();
    while(!mPkts.isEmpty())
    {
        AVPacket* pkt = mPkts.takeFirst();
        av_packet_free(&pkt);
    }
    if(mAudioPlayer)mAudioPlayer->Clear();
    aMutex.unlock();
}

void XAudioThread::Close()
{
    Clear();

    qDebug() <<"mDemux->close();";
    mThreadShouldExit = true;
    wait();

    mDecoder->Close();
//线程safe的函数不必要再加锁
    aMutex.lock();
    if(mDecoder)delete mDecoder;
    mDecoder = nullptr;
    aMutex.unlock();
    mAudioPlayer->Close();
    mResample->Close();
    aMutex.lock();
    delete mResample;
    aMutex.unlock();
}


void XAudioThread::setAudioTimeBase(double timebase)
{
    mAudioTimeBase = timebase;
}

void XAudioThread::setPause(bool isPause)
{
    aMutex.lock();
    mIsPause = isPause;
    if(mAudioPlayer)mAudioPlayer->SetPause(isPause);
    aMutex.unlock();
}

XDecode *XAudioThread::getDecoder()
{
    return mDecoder;
}

void XAudioThread::setVolume(double volume)
{
    aMutex.lock();
    if(mAudioPlayer)mAudioPlayer->setVolume(volume);
    aMutex.unlock();
}

void XAudioThread::setPlaySpeed(double speed)
{
    aMutex.lock();
    if(mAudioPlayer)mAudioPlayer->setPlaySpeed(speed);
    aMutex.unlock();
}
