#include "xresample.h"

XResample::XResample(QObject *parent) : QObject(parent)
{

}

XResample::~XResample()
{
    if(mSwrCtx)swr_free(&mSwrCtx);
}

bool XResample::Open(AVCodecParameters *param,bool isClearParam)
{
    //音频数据转换,又叫重采样
    int errcode = 0;
    mAudioDecMtx.lock();
    mSwrCtx = swr_alloc_set_opts(mSwrCtx,
                       av_get_default_channel_layout(param->channels),//输出格式
                       mOutAudioFMT,//输出样本格式
                       param->sample_rate,//输出采样率
                       av_get_default_channel_layout(param->channels),//输入格式
                       (AVSampleFormat)param->format,//输入样本格式
                       param->sample_rate,//输入采样率
                       0,nullptr
                       );
    if(isClearParam)avcodec_parameters_free(&param);
    errcode = swr_init(mSwrCtx);
    mAudioDecMtx.unlock();
    if(errcode < 0)
    {
        qDebug() << "Enter avcodec_send_packet err" << endl;
        char* errBuf = new char[1024];
        av_strerror(errcode,errBuf,1024);
        qDebug() << "avcodec_send_packet(pCodecCtxV,pk) failed:" << errBuf << endl;
        delete[] errBuf;
    }
    return true;
}

void XResample::Close()
{
    mAudioDecMtx.lock();
    swr_free(&mSwrCtx);
    mAudioDecMtx.unlock();
}

int XResample::Resample(AVFrame* audioFrame,unsigned char* pcmData)
{
    mAudioDecMtx.lock();
    if(!audioFrame)
    {
        mAudioDecMtx.unlock();
        return -1;
    }
    if(!pcmData)
    {
        av_frame_free(&audioFrame);
        mAudioDecMtx.unlock();
        return -1;
    }
    uint8_t* data[2] = {0};
    data[0] = pcmData;
    //每个样本是s16
    int nbSamplesPerChannel = swr_convert(mSwrCtx,
                data,
                audioFrame->nb_samples,
                (const uint8_t**)audioFrame->data,
                audioFrame->nb_samples);
    mAudioDecMtx.unlock();
    if(nbSamplesPerChannel < 0)
        return nbSamplesPerChannel;
    int outBytes = nbSamplesPerChannel*
            av_get_bytes_per_sample((AVSampleFormat)mOutAudioFMT)*
            audioFrame->channels;
    av_frame_free(&audioFrame);//BUG fix of memory leak
    return outBytes;
}

