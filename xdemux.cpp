#include "xdemux.h"

XDemux::XDemux(QObject *parent)
    : QObject(parent)
{
    static bool isFirst = true;
    static QMutex initMutex;
    initMutex.lock();
    if(isFirst)
    {
        av_register_all();
        avformat_network_init();
        isFirst = false;
    }
    initMutex.unlock();
    isRunning = true;
}

XDemux::~XDemux()
{
    qDebug() <<  "~XDemux()" ;
    isRunning = false;
}
/**
 * @brief Open
 * @param url
 * @return true on success,else false
 */
bool XDemux::Open(const char *url)
{
    int errcode = 0;
    close();
    AVDictionary* opt = nullptr;
    /** rtsp流以TCP协议打开 */
    av_dict_set(&opt,"rtsp_transport","tcp",0);
    /** 网络延时时间 */
    av_dict_set(&opt,"max_delay","500",0);



    mMtx.lock();
    qDebug() <<  "mMtx.lock();" ;
    errcode = avformat_open_input(&mAvFormat,url,
                                  0,//自动选择格式
                                  &opt);
    if(errcode!=0)
    {
        mMtx.unlock();
        char* errBuf = new char[1024];
        av_strerror(errcode,errBuf,1024);
        qDebug() <<  "Open " << url << " failed:" << errBuf;
        delete[] errBuf;
        return false;
    }
    qDebug() << "Open " << url << " success.";

    errcode = avformat_find_stream_info(mAvFormat,0);
    if(errcode < 0)
    {
        mMtx.unlock();
        char* errBuf = new char[1024];
        av_strerror(errcode,errBuf,1024);
        qDebug() << "avformat_find_stream_info() failed:" << errBuf ;
        delete[] errBuf;
        return false;
    }//直播流
    qDebug() << "avformat_find_stream_info() success." ;

    /** 打印流信息*/
    mTotalDurationMs = mAvFormat->duration / AV_TIME_BASE * 1000;//ms

    qDebug() << "File duration:"
         << mTotalDurationMs/3600 << "h"
         << (mTotalDurationMs%3600)/60 << "min"
         <<((mTotalDurationMs%3600)%60)%60 << "s";

    av_dump_format(mAvFormat,
                   0,//不重要
                   url,//不重要
                   0);//解码用0，编码用1


    //音频流
    qDebug() << "＝＝＝＝＝＝＝＝＝＝＝Audio　info＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝";
    mAudioIndex =
    av_find_best_stream(mAvFormat,
                        AVMEDIA_TYPE_AUDIO,//想要的流索引的类型
                        -1,//自动选择
                        -1,//多路音视频才给值
                        nullptr,//因为解封装和解码要降耦合，所以不传
                        0);//保留字段
    AVStream* as = mAvFormat->streams[mAudioIndex];
    mSampleRate = as->codecpar->sample_rate;
    mChannels = as->codecpar->channels;
    mAudioTimeBase = r2d(as->time_base)*(double)1000;

    qDebug() << "streams[" << mAudioIndex << "]"
         << " is Audio." ;
    qDebug() << "\t sampleRate = " << as->codecpar->sample_rate ;
    qDebug() << "\t format = " << (AVSampleFormat)as->codecpar->format ;
    qDebug() << "\t channels = " << as->codecpar->channels ;
    qDebug() << "\t codec_id = " << as->codecpar->codec_id ;
    qDebug() << "\t frame_size = "<< as->codecpar->frame_size ;

    //视频流
    qDebug() << "＝＝＝＝＝＝＝＝＝＝＝Vedio　info＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝";
    mVideoIndex =
    av_find_best_stream(mAvFormat,
                        AVMEDIA_TYPE_VIDEO,//想要的流索引的类型
                        -1,//自动选择
                        -1,//多路音视频才给值
                        nullptr,//因为解封装和解码要降耦合，所以不传
                        0);//保留字段
    as = mAvFormat->streams[mVideoIndex];
    mWidth = as->codecpar->width;
    mHeight = as->codecpar->height;
    mVideoTimeBase = r2d(as->time_base)*(double)1000;

    qDebug() << "streams[" << mVideoIndex << "]"
         << " is Vedio." ;
    qDebug() << "\t width = " << as->codecpar->width<<
            ",height = " <<as->codecpar->height;
    qDebug() << "\t codec_id = "<<as->codecpar->codec_id;
    qDebug() << "\t fps = " <<r2d(as->avg_frame_rate) ;
    mMtx.unlock();
    return true;
}

AVPacket *XDemux::Read()
{
    mMtx.lock();
    if(!mAvFormat)
    {
        mMtx.unlock();
        return nullptr;
    }
    AVPacket* pkt = av_packet_alloc();
    //读一帧并分配数据空间
    int errCode = av_read_frame(mAvFormat,pkt);
    if(errCode < 0)
    {
        mMtx.unlock();
        av_packet_free(&pkt);
        return nullptr;
    }
    //pts dts 转化为毫秒
//    qDebug() << "Read success";
    pkt->pts = pkt->pts * r2d(mAvFormat->streams[pkt->stream_index]->time_base) * 1000 ;
    pkt->dts = pkt->dts * r2d(mAvFormat->streams[pkt->stream_index]->time_base) * 1000 ;
    mMtx.unlock();
    return pkt;
}

void XDemux::Clear()
{
    mMtx.lock();
    if(mAvFormat)avformat_flush(mAvFormat);
    mMtx.unlock();
}

AVCodecParameters *XDemux::CopyVParams()
{
    mMtx.lock();
    if(!mAvFormat)
    {
        mMtx.unlock();
        return nullptr;
    }
    AVCodecParameters* vparams = avcodec_parameters_alloc();
    avcodec_parameters_copy(vparams,mAvFormat->streams[mVideoIndex]->codecpar);
    mMtx.unlock();
    return vparams;
}

AVCodecParameters *XDemux::CopyAParams()
{
    mMtx.lock();
    if(!mAvFormat)
    {
        mMtx.unlock();
        return nullptr;
    }
    AVCodecParameters* aparams = avcodec_parameters_alloc();
    avcodec_parameters_copy(aparams,mAvFormat->streams[mAudioIndex]->codecpar);
    mMtx.unlock();
    return aparams;
}

bool XDemux::Seek(double pos)
{
    if(pos>1.0)pos=1.0;
    if(pos<0.0)pos=0.0;
    mMtx.lock();
    if(!mAvFormat)
    {
        mMtx.unlock();
        return false;
    }
    //置空读写缓冲
    avformat_flush(mAvFormat);

    double streamTimeBase = r2d(mAvFormat->streams[mVideoIndex]->time_base);
    if(streamTimeBase <= 0)streamTimeBase = (double)1000 / (double)AV_TIME_BASE;
    //基于某些视频没有给对流时长,则采用文件时长
    qint64 seekPos = mAvFormat->streams[mVideoIndex]->duration * pos;
    if(seekPos < 0)seekPos = mAvFormat->duration*streamTimeBase*pos;

    int errCode = av_seek_frame(mAvFormat,mVideoIndex,seekPos,
                  AVSEEK_FLAG_BACKWARD|AVSEEK_FLAG_FRAME);
    mMtx.unlock();
    if(errCode < 0 )return false;
    qDebug() << "XDemux::Seek(double pos) success,pos =" << pos;
    return true;
}



void XDemux::close()
{
    mMtx.lock();
    if(!mAvFormat)
    {
        mMtx.unlock();
        return;
    }
    avformat_close_input(&mAvFormat);
    avformat_network_deinit();
    mTotalDurationMs = 0;
    mAudioTimeBase = 0;
    mVideoTimeBase = 0;
    mMtx.unlock();
}

bool XDemux::isAudio(AVPacket *pkt)
{
    if(!pkt)return false;
    return pkt->stream_index == mAudioIndex;
}

bool XDemux::isVideo(AVPacket *pkt)
{
    if(!pkt)return false;
    return pkt->stream_index == mVideoIndex;
}

double XDemux::getAudioTimeBase()
{
    return mAudioTimeBase;
}

double XDemux::getVideoTimeBase()
{
    return mVideoTimeBase;
}

qint64 XDemux::getTotalDuration()
{
    return mTotalDurationMs;
}

double XDemux::r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num/(double)r.den;
}
