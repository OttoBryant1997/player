#ifndef XDECODE_H
#define XDECODE_H

#include <QObject>
#include <QMutex>
#include <QDebug>
#include <xdemux.h>
#ifdef __cplusplus
extern "C"
{
#endif

#include<libavcodec/avcodec.h>

#ifdef __cplusplus
}
#endif
enum CodecType{
    audio,
    video
};
/**
 * @brief The XDecode class 解码7类,提供Send输入包数据,Recv拿到音视频帧数据
 * 如果是音频解码7则传入音频解码参数集,视频则提供视频解码参数集
 */
class XDecode : public QObject
{
    Q_OBJECT
public:
    explicit XDecode(enum CodecType type,QObject *parent = nullptr);
    ~XDecode();
    bool isAudio = false;

    /**
     * @brief Open 打开解码7并释放para
     * @param para
     * @return
    */
    virtual bool Open(AVCodecParameters* para);
    /**
     * @brief Close 关闭解码7
     * @return
     */
    virtual void Close();
    /**
     * @brief Clear 置空解码队列
     */
    virtual void Clear();
    /**
     * @brief Send 传入包发送到解码线程，并回收空间
     * @param pkt
     * @return
     */
    virtual bool Send(AVPacket* pkt);
    /**
     * @brief Recv 收帧，一次send可能要多次recv，最后send null,多次recv就可以，每次复制一份，由接受者释放
     * @return
     */
    virtual AVFrame* Recv();

    /** 本解码帧的pts */
    qint64 mPts = 0;


signals:
private:
    AVCodecContext* mCodecCtx = nullptr;
    QMutex mCodecMtx;
private:
};

#endif // XDECODE_H
