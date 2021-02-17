#include "xdemuxthread.h"

XDemuxThread::XDemuxThread(QObject *parent) : QThread(parent)
{

}

XDemuxThread::~XDemuxThread()
{
    mExit = true;
    wait();
}

bool XDemuxThread::Open(const char *url, IVideoCall *call)
{
    if(url == nullptr || url[0] == '\0')
        return false;
    mutex.lock();
    qDebug() << "mutex.lock();";
    bool ret = mDemux->Open(url);
    mProgressPtsMs = 0;
    mProgressTotalMs = mDemux->getTotalDuration();
    qDebug() << "mDemux->Open(url);";
    if(!ret)
    {
        qDebug() << "mDemux->Open(url) failed.";
        return false;
    }
    //打开视频解码7和视频处理线程
    if(!mVideoThread->Open(mDemux->CopyVParams(),mDemux->mWidth,mDemux->mHeight,call))
    {
        qDebug() << "mVideoThread->Open() failed.";
        ret = false;
    }
    mVideoThread->setVideoTimeBase(mDemux->getVideoTimeBase());
    //打开音频解码7和音频处理线程
    if(!mAudioThread->Open(mDemux->CopyAParams(),mDemux->mSampleRate,mDemux->mChannels))
    {
        qDebug() << "mAudioThread->Open() failed.";
        ret = false;
    }
    mAudioThread->setAudioTimeBase(mDemux->getAudioTimeBase());
    mutex.unlock();
    qDebug() << "XDemuxThread::Open()"<<ret;
    return ret;
}

void XDemuxThread::Start()
{
    mutex.lock();
    if(!mDemux)mDemux = new XDemux();
    if(!mAudioThread)mAudioThread = new XAudioThread();
    if(!mVideoThread)mVideoThread = new XVideoThread();
    // demux线程
    QThread::start();
    //音视频线程
    if(mAudioThread)mAudioThread->start();
    if(mVideoThread)mVideoThread->start();
    mutex.unlock();
}

void XDemuxThread::run()
{
    qDebug() <<"enter XDemuxThread::run()";
    while(!mExit || !mAudioThread || !mVideoThread)
    {
        mutex.lock();
        if(mIsPause)
        {
            mutex.unlock();
            yieldCurrentThread();
            continue;
        }
        if(!mDemux)
        {
            mutex.unlock();
            msleep(5);
            continue;
        }
        //音视频同步
        mVideoThread->setSyncTimeLine(mAudioThread->mPts * mPlaySpeed);
        mProgressPtsMs = mAudioThread->mPts;
        AVPacket* pkt = mDemux->Read();
        if(!pkt)
        {
            mutex.unlock();
            msleep(5);
            continue;
        }
        if(mDemux->isAudio(pkt))
        {
            mAudioThread->Push(pkt);
        }
        else if(mDemux->isVideo(pkt))
        {
            mVideoThread->Push(pkt);
        }
        mutex.unlock();
    }
    qDebug() <<"exit XDemuxThread::run()";
}

void XDemuxThread::Close()
{
    mExit = true;
    wait();
    mutex.lock();
    if(mDemux)mDemux->close();
    if(mAudioThread)mAudioThread->Close();
    if(mVideoThread)mVideoThread->Close();
    delete mDemux;mDemux=nullptr;
    delete mAudioThread;mAudioThread=nullptr;
    delete mVideoThread;mVideoThread=nullptr;
    mutex.unlock();
}

void XDemuxThread::setPause(bool isPause)
{
    mutex.lock();
    this->mIsPause = isPause;
    mAudioThread->setPause(isPause);
    mVideoThread->setPause(isPause);
    mutex.unlock();
}

void XDemuxThread::Clear()
{
    mutex.lock();
    if(mDemux)mDemux->Clear();
    if(mAudioThread)mAudioThread->Clear();
    if(mVideoThread)mVideoThread->Clear();
    mutex.unlock();
}

void XDemuxThread::Seek(double pos)
{
    Clear();
    mutex.lock();
    bool oldPauseStatus = mIsPause;
    mutex.unlock();
    setPause(true);

    mutex.lock();
    mDemux->Seek(pos);
    //实际要显示的位置pts
    qint64 seekPts = pos * mDemux->getTotalDuration();
    refreshFrame(seekPts);
    mutex.unlock();
//    Clear();//会卡顿
    setPause(oldPauseStatus);
}

void XDemuxThread::setPlaySpeed(double speed)
{
    mutex.lock();
    if(speed < 0.1 )speed = 0.1;
    if(speed > 3.0)speed = 3.0;
    mPlaySpeed = speed;
    qDebug() << "mPlaySpeed" << mPlaySpeed;

    mutex.unlock();
}

void XDemuxThread::setVolume(double volume)
{
    mutex.lock();
    if(mAudioThread)mAudioThread->setVolume(volume);
    mutex.unlock();
}

bool XDemuxThread::refreshFrame(double seekPts)
{//不是线程safe的
    while (!mExit)
    {
        AVPacket* pkt = mDemux->Read();
        if(!pkt)break;
        if(pkt->stream_index == mDemux->mVideoIndex)
        {
            if(mVideoThread->getDecoder())
            {
                bool re =  mVideoThread->getDecoder()->Send(pkt);
                if(!re)break;
                while(!mExit)
                {
                    AVFrame * frame = mVideoThread->getDecoder()->Recv();
                    if(!frame)break;
                    if(frame->pts >= seekPts)
                    {
                        if(!mVideoThread->getVideoHandler())
                        {
                            av_frame_free(&frame);
                            return false;
                        }
                        mProgressPtsMs = seekPts;
//                        qDebug() << "in refresh mProgressPtsMs=" << mProgressPtsMs;
                        mVideoThread->getVideoHandler()->Repaint(frame);
                        return true;
                    }
                }
            }
        }
        else
        {
            //是音频数据则用于刷新mpts
            av_packet_free(&pkt);
            continue;
        }
    }
    return true;
}
