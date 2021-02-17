#include "xdecode.h"

XDecode::XDecode(enum CodecType type,QObject *parent)
    : QObject(parent)
{
    isAudio = type== CodecType::audio;
    avcodec_register_all();
}

XDecode::~XDecode()
{

}

bool XDecode::Open(AVCodecParameters *para)
{
    int errcode = 0;
    if(!para)return false;
    Close();
    /** 根据具体流para查找解码7 */
    AVCodec* pSoftCodec = avcodec_find_decoder(para->codec_id);

    if(!pSoftCodec)
    {
        avcodec_parameters_free(&para);
        if(isAudio)
        {
            qDebug() <<  "Can't find SoftCodec for audio stream,codecId = " <<
                    para->codec_id ;
        }
        else
        {
            {
                qDebug() <<  "Can't find SoftCodec for video stream,codecId = " <<
                        para->codec_id ;
            }
        }
        return false;
    }
    if(isAudio)
    {
        qDebug() <<  "Find SoftCodec for audio stream,codecId = " <<
                para->codec_id ;
    }
    else
    {
        {
            qDebug() <<  "Find SoftCodec for video stream,codecId = " <<
                    para->codec_id ;
        }
    }


    /** 支持硬件解码，如"h264_mediacodec" */
//    AVCodec* pHardCodec = avcodec_find_decoder_by_name("h264_mediacodec");
    /** 获得空间存放该解码的上下文 */
    mCodecMtx.lock();
    mCodecCtx = avcodec_alloc_context3(pSoftCodec);
    /** 将具体流需要的解码参数拷贝到解码上下文，方便将解封装线程和解码线程分离 */
    avcodec_parameters_to_context(mCodecCtx,para);
    /** 解码线程数,这里默认配置8线程解码*/
    mCodecCtx->thread_count = 8;
    /** 打开解码7 */
    errcode = avcodec_open2(mCodecCtx,pSoftCodec,0);
    if(errcode < 0)
    {
        avcodec_parameters_free(&para);
        avcodec_free_context(&mCodecCtx);
        mCodecMtx.unlock();
        char* errBuf = new char[1024];
        av_strerror(errcode,errBuf,1024);
        qDebug() <<  "avcodec_open2(mCodecCtx,pSoftCodec,0) failed:" << errBuf ;
        delete[] errBuf;
        return false;
    }
    mCodecMtx.unlock();
    qDebug() <<  "Open vedio decoder success."  ;
    avcodec_parameters_free(&para);
    return true;
}

void XDecode::Close()
{
    mCodecMtx.lock();
    if(mCodecCtx)
    {
        avcodec_close(mCodecCtx);
        avcodec_free_context(&mCodecCtx);
    }
    mPts = 0;
    mCodecMtx.unlock();
}

void XDecode::Clear()
{
    qDebug() << "enter XDecode::Clear() ";
    mCodecMtx.lock();
    //置空解码缓冲
    if(mCodecCtx)avcodec_flush_buffers(mCodecCtx);

    mCodecMtx.unlock();
    qDebug() << "XDecode::Clear() exit";
}

bool XDecode::Send(AVPacket *pkt)
{
    if(!pkt || !pkt->data || pkt->size <= 0)return false;
    mCodecMtx.lock();
    if(!mCodecCtx)
    {
        mCodecMtx.unlock();
        return false;
    }
    int errcode = avcodec_send_packet(mCodecCtx,pkt);
    mCodecMtx.unlock();
    av_packet_unref(pkt);
    av_packet_free(&pkt);//BUG fix of memory leak
    if(errcode < 0)
    {//可以看到视频解码较慢，开始一段时间都没有收到视频帧
        char* errBuf = new char[1024];
        av_strerror(errcode,errBuf,1024);
//        qDebug() << "avcodec_send_packet failed:" <<errBuf;
        delete[] errBuf;
        return false;
    }
    return true;
}

AVFrame *XDecode::Recv()
{
    mCodecMtx.lock();
    if(!mCodecCtx)
    {
        mCodecMtx.unlock();
        qDebug() << "!mCodecCtx.";
        return nullptr;
    }
    AVFrame* frame = av_frame_alloc();
    int errcode = avcodec_receive_frame(mCodecCtx,frame);
    mCodecMtx.unlock();
    if(errcode!=0)
    {
//        qDebug() << "avcodec_receive_frame failed.";
        if(errcode < 0)
        {//可以看到视频解码较慢，开始一段时间都没有收到视频帧
            char* errBuf = new char[1024];
            av_strerror(errcode,errBuf,1024);
//            qDebug() << "Receive frame failed: " <<errBuf ;
            delete[] errBuf;
        }
        av_frame_free(&frame);
        return nullptr;
    }
//    qDebug() << "XDecode::Recv() frame:" << frame;
    mPts = frame->pts;
    return frame;
}

