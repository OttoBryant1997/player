#ifndef XRESAMPLE_H
#define XRESAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include<libavcodec/avcodec.h>
#include<libswresample/swresample.h>

#ifdef __cplusplus
}
#endif

#include <QObject>
#include <QMutex>
#include <QDebug>
/**
 * @brief The XResample class 音频转换对象,将音频帧转换为可播放的pcm数据
 */
class XResample : public QObject
{
    Q_OBJECT
public:
    explicit XResample(QObject *parent = nullptr);
    virtual ~XResample();
    /**
     * @brief Open 配置音频重采样上下文，输出参数和输入参数一致，除了采样格式变成s16
     * @param param 配置音频重采样上下文需要的参数,会被释放
     * @return true on success,false on failed.s
     */
    virtual bool Open(AVCodecParameters* param,bool isClearParam = false);
    /**
     * @brief Close 回收音频重采样上下文
     */
    virtual void Close();
    /**
     * @brief Resample 音频重采样,无论成败释放in_data
     * @param audioFrame   输入一帧音频数据
     * @param pcmData  输出pcm数据
     * @return 返回数据大小
     */
    virtual int Resample(AVFrame* audioFrame,unsigned char* pcmData);
signals:
protected:
    QMutex mAudioDecMtx;
    SwrContext* mSwrCtx = nullptr;
    AVSampleFormat mOutAudioFMT = AVSampleFormat::AV_SAMPLE_FMT_S16;
};

#endif // XRESAMPLE_H
