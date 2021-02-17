#include "xvideothread.h"

XVideoThread::XVideoThread(QObject *parent) : AVThread(parent)
{
    mDecoder = new XDecode(CodecType::video);
}

XVideoThread::~XVideoThread()
{

}

void XVideoThread::run()
{
    qDebug() << "enter XVideoThread::run()";
//    QThread::setPriority(QThread::NormalPriority);
    while(!mThreadShouldExit)
    {
        vMutex.lock();

        if(mIsPause)
        {
            vMutex.unlock();
            yieldCurrentThread();
            continue;
        }
        //音视频同步
        if(mSyncPts > 0 && mSyncPts < mDecoder->mPts)
        {
//            qDebug() << "Syncing"<<mSyncPts<<mDecoder->mPts;//生效
            vMutex.unlock();
            msleep(5);
            continue;
        }
        bool ret = mDecoder->Send(Pop());
        if(!ret)
        {
            vMutex.unlock();
            msleep(1);
            continue;
        }
        //一次send多次recv
        while (!mThreadShouldExit)
        {
            AVFrame* frame = mDecoder->Recv();
            if(!frame)break;
            //播放视频,这里不方便直接调用videowidget的repaint函数,因此开一个接口类
            if (mVideoCall)
            {
                mVideoCall->Repaint(frame);
            }
            break;
        }
        vMutex.unlock();
    }
}

bool XVideoThread::Open(AVCodecParameters *params,int w,int h,IVideoCall* callback)
{
    Clear();
    vMutex.lock();
    mSyncPts = 0;
    mVideoTimeBase = 0;
    if(!params)return false;
    if(!callback)return false;

    mVideoCall = callback;
    //初始化显示窗口
    mVideoCall->Init(w,h);
    vMutex.unlock();
    //打开解码7

    bool ret = true;
    if(!mDecoder->Open(params))
    {//在decoder里面会释放params
        ret = false;
    }
    return ret;
}



void XVideoThread::setSyncTimeLine(qint64 audioPts)
{
    vMutex.lock();
    mSyncPts = audioPts;
    vMutex.unlock();
}

void XVideoThread::setVideoTimeBase(qint64 timebase)
{
    mVideoTimeBase = timebase;
}

void XVideoThread::Push(AVPacket *pkt)
{
    if(!pkt)return;
    while(!mThreadShouldExit)
    {//阻塞
        vMutex.lock();
        if(mPkts.size() < mMaxList)
        {
            mPkts.push_back(pkt);
            vMutex.unlock();
            break;
        }
        vMutex.unlock();
        msleep(1);
    }
}

AVPacket *XVideoThread::Pop()
{
//    qDebug() << "vMutex.lock();";//调用线程自己的线程safe的函数一定要在锁外或者用可重入锁
    if(mPkts.isEmpty())
    {
        return nullptr;
    }
    AVPacket * pkt = mPkts.takeFirst();
    return pkt;
}

void XVideoThread::Clear()
{
    vMutex.lock();
    mDecoder->Clear();
    while(!mPkts.isEmpty())
    {
        AVPacket* pkt = mPkts.takeFirst();
        av_packet_free(&pkt);
    }
    vMutex.unlock();
}

void XVideoThread::Close()
{
    Clear();

    mThreadShouldExit = true;
    wait();

    mDecoder->Close();

    vMutex.lock();
    if(mDecoder)delete mDecoder;
    mDecoder = nullptr;
    vMutex.unlock();
}
void XVideoThread::setPause(bool isPause)
{
    vMutex.lock();
    mIsPause = isPause;
    vMutex.unlock();
}

XDecode *XVideoThread::getDecoder()
{
    return mDecoder;
}

IVideoCall* XVideoThread::getVideoHandler()
{
    return mVideoCall;
}

