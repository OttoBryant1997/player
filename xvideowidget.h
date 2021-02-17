#ifndef XVIDEOWIDGET_H
#define XVIDEOWIDGET_H

//自动加双引号
#define GET_STR(TOKEN) #TOKEN
#define IDWINDOWPOS     0
#define IDTEXPOSY    1
#define IDTEXPOSU    2
#define IDTEXPOSV    3

//准备yuv数据
// ffmpeg -i Toys1080.mp4 -t 10 -s 240x128 -pix_fmt yuv420p  out240x128.yuv

/**
通过继承glfunctions方便直接使用
通过组合QGLShaderProgram方便连接c++和shader
*/

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
#include <QFile>
#include <QTimer>
#include<QMutex>
#include<QOpenGLExtraFunctions>
#include<IVideoCall.h>
struct AVFrame;
/**
 * @brief The XVideoWidget class 用于显示YUV数据的一个视口类,
 * 如果要用QML则将QOpenGLWidget换成 QQuickPaintedItem并按QPainter方式实现
 */
class XVideoWidget : public QOpenGLWidget,
        protected QOpenGLFunctions,
        public IVideoCall
{
    Q_OBJECT
public:
    XVideoWidget(QWidget *parent);
    ~XVideoWidget();
    void Init(int w,int h)override;
    virtual void Repaint(AVFrame* frame)override;
protected:
    //刷新显示
    void paintGL()override;

    //初始化gl
    void initializeGL()override;

    // 窗口尺寸变化
    void resizeGL(int width, int height)override;

private:
    //shader程序
    QGLShaderProgram mProgram;

    GLuint mPBO[2]={0};
    //shader中yuv变量地址
    GLuint mGPUtex = 0;
    //opengl的 texture地址
    GLuint mCPUtex = 0;

    //材质内存空间
    unsigned char* mYUVdata[3] = {0};
    int mWidth = 0;int mHeight = 0;

private:
    void updateTex();
    void createTex();
    QMutex mutex;
    AVFrame* mFrame = nullptr;
private:
    /**
     * @brief checkLinePading 如果需要行对准则改变参数
     */
//    void checkLinePading(int lineSize0);
};

#endif //!XVIDEOWIDGET_H

