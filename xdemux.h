#ifndef XDEMUX_H
#define XDEMUX_H
#include <QObject>
#include <QMutex>
#include <QDebug>


#ifdef __cplusplus
extern "C"
{
#endif

#include<libavformat/avformat.h>


#ifdef __cplusplus
}
#endif


/**
打开文件或者流媒体 rtmp http rtsp
*/
/**
 * @brief The XDemux class 解码上下文的封装对象,提供文件操作和文件信息接口
 */
class XDemux : public QObject
{
    Q_OBJECT
public:
    explicit XDemux(QObject *parent = nullptr);
    virtual ~XDemux();
    /**
     * @brief Open
     * @param url
     * @return true on success,else false
     */
    virtual bool Open(const char* url);
    /**
     * @brief Read 空间需要调用者使用av_packet_free释放
     * @return
     */
    virtual AVPacket* Read();
    /**
     * @brief Clear 置空读写缓冲
     */
    virtual void Clear();
    /**
     * @brief CopyVParams 拷贝一份视频解码参数 avcodec_parameters_free()释放
     * @return AVCodecParameters*
     */
    virtual AVCodecParameters* CopyVParams();
    /**
     * @brief CopyVParams 拷贝一份音频解码参数 avcodec_parameters_free()释放
     * @return AVCodecParameters*
     */
    virtual AVCodecParameters* CopyAParams();
    /**
     * @brief Seek 即文件的seek
     * @param pos 0.0 - 1.0
     * @return true on success else false
     */
    virtual bool Seek(double pos);

    /**
     * @brief close 关闭文件和网络
     */
    virtual void close();
    /**
     * @brief isAudio 判断一个包是不是音频
     * @return
     */
    virtual bool isAudio(AVPacket* pkt);
    /**
     * @brief isAudio 判断一个包是不是视频
     * @return
     */
    virtual bool isVideo(AVPacket* pkt);
    /**
     * @brief getAudioTimeBase 返回音频时间基数，方便进行帧同步，pts*timebase即可
     * @return
     */
    virtual double getAudioTimeBase();
    /**
     * @brief getVideoTimeBase 返回视频时间基数，方便进行帧同步，pts*timebase即可
     * @return
     */
    virtual double getVideoTimeBase();

    /**
     * @brief getTotalDuration 获得本文件的总时长
     * @return
     */
    virtual qint64 getTotalDuration();
signals:
public:
    int mWidth = 0;
    int mHeight = 0;
    bool isRunning = false;
    int mVideoIndex = 0;
    int mAudioIndex = 0;
protected:
    AVFormatContext* mAvFormat = nullptr;
    QMutex mMtx;

    qint64 mpts;
    qint64 mTotalDurationMs = 0;
    int mAudioTimeBase = 0;
    int mVideoTimeBase = 0;
public:
    int mSampleRate = 0;
    int mChannels = 0;
private:
    static double r2d(AVRational r);
};

#endif // XDEMUX_H
