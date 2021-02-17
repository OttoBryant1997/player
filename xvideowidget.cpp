#include "XVideoWidget.h"

#include <QDebug>
#include <QTimer>
#include <QFileDialog>

#ifdef __cplusplus
extern "C"
{
#endif

#include<libavutil/frame.h>

#ifdef __cplusplus
}
#endif

//顶点shader
const char* vsCode__ = GET_STR(
    attribute vec4 windowPos;
    highp attribute vec2 texturePosY;
    highp attribute vec2 texturePosU;
    highp attribute vec2 texturePosV;
    highp varying vec2 textureOutPosY;
    highp varying vec2 textureOutPosU;
    highp varying vec2 textureOutPosV;
    void main(void)
    {
        gl_Position = windowPos;
        textureOutPosY = texturePosY;
        textureOutPosU = texturePosU;
        textureOutPosV = texturePosV;
    }
);


//片元shader
const char* fsCode420p__ = GET_STR(
    highp varying vec2 textureOutPosY;
    highp varying vec2 textureOutPosU;
    highp varying vec2 textureOutPosV;
    uniform sampler2D texYUV;

    void main(void)
    {
        vec3 yuv;
        vec3 rgb;
        yuv.x = texture2D(texYUV, textureOutPosY).r;
        yuv.y = texture2D(texYUV, textureOutPosU).r - 0.5;
        yuv.z = texture2D(texYUV, textureOutPosV).r - 0.5;
        rgb = mat3(1.0, 1.0, 1.0,
            0.0, -0.39465, 2.03211,
            1.13983, -0.58060, 0.0) * yuv;
        gl_FragColor = vec4(rgb, 1.0);
    }
);




XVideoWidget::XVideoWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
}

XVideoWidget::~XVideoWidget()
{
//    delete mYUVdata[0];
}

//初始化opengl
void XVideoWidget::initializeGL()
{
    mutex.lock();
    qDebug() << "initializeGL";

    //初始化opengl （QOpenGLFunctions继承）函数
    initializeOpenGLFunctions();

    //program加载shader（顶点和片元）脚本
    //顶点shader
    qDebug() << mProgram.addShaderFromSourceCode(QGLShader::Vertex, vsCode__);
    //片元（像素）
    qDebug()<<mProgram.addShaderFromSourceCode(QGLShader::Fragment, fsCode420p__);


    //设置窗口顶点坐标的变量
    mProgram.bindAttributeLocation("windowPos",IDWINDOWPOS);

    //设置材质坐标
    mProgram.bindAttributeLocation("texturePosY",IDTEXPOSY);
    mProgram.bindAttributeLocation("texturePosU",IDTEXPOSU);
    mProgram.bindAttributeLocation("texturePosV",IDTEXPOSV);



    //编译shader
    qDebug() << "program.link() = " << mProgram.link();

    qDebug() << "program.bind() = " << mProgram.bind();

    //传递顶点和材质坐标
    //顶点0点在中间，总长为2 材质0点在左下角，总长1
    static const GLfloat Pos[] = {//z
        -1.0f,1.0f,         0.0f, -1.0 ,            0.0f,-1.0/3.0f,        0.5f,-1.0/3.0f,
        1.0f,1.0f,          1.0f, -1.0,             0.5f,-1.0/3.0f,        1.0f,-1.0/3.0f,
        -1.0f,-1.0f,        0.0f, -1.0/3.0f,         0.0f,0.0,              0.5f,0.0,
        1.0f,-1.0f,         1.0f, -1.0/3.0f,         0.5f,0.0,              1.0f,0.0,
    };

    //顶点
    glVertexAttribPointer(IDWINDOWPOS, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), Pos);
    glEnableVertexAttribArray(IDWINDOWPOS);

    //材质
    glVertexAttribPointer(IDTEXPOSY, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), Pos + 2);
    glEnableVertexAttribArray(IDTEXPOSY);
    glVertexAttribPointer(IDTEXPOSU, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), Pos + 4);
    glEnableVertexAttribArray(IDTEXPOSU);
    glVertexAttribPointer(IDTEXPOSV, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), Pos + 6);
    glEnableVertexAttribArray(IDTEXPOSV);

    //从shader获取材质
    mGPUtex = mProgram.uniformLocation("texYUV");
    mutex.unlock();
}

//刷新显示
void XVideoWidget::paintGL()
{
    mutex.lock();
    updateTex();
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    mutex.unlock();
}


// 窗口尺寸变化
void XVideoWidget::resizeGL(int mWidth, int mHeight)
{
//    qDebug() << "resizeGL "<<mWidth<<":"<<mHeight;
//    qDebug() << "max tex = " << GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS;
}



void XVideoWidget::Repaint(AVFrame *frame)
{
    if(!frame)return;
    mutex.lock();
    if(mWidth*mHeight==0||!mYUVdata[0]||frame->width!=this->mWidth||frame->height!=this->mHeight )
    {
        av_frame_free(&frame);
        mutex.unlock();
        return;
    }
//    qDebug() << "frame.linesize="<<frame->linesize[0]<<mWidth;
    //frame.linesize > as->codecpar->width;
    if(mWidth == frame->linesize[0])
    {
        memcpy(mYUVdata[0],frame->data[0],mWidth*mHeight);
        memcpy(mYUVdata[1],frame->data[1],mWidth*mHeight/4);
        memcpy(mYUVdata[2],frame->data[2],mWidth*mHeight/4);
    }
    else
    {//行对准
        for(int i=0;i<mHeight;i++)
        {
            memcpy(mYUVdata[0]+i*mWidth,frame->data[0]+i*frame->linesize[0],mWidth);
        }
        for(int i=0;i<mHeight/2;i++)
        {
            memcpy(mYUVdata[1]+i*mWidth/2,frame->data[1]+i*frame->linesize[1],mWidth/2);
            memcpy(mYUVdata[2]+i*mWidth/2,frame->data[2]+i*frame->linesize[2],mWidth/2);
        }
    }
    mutex.unlock();
    av_frame_free(&frame); //bug fix for memory leak
    update();
}

void XVideoWidget::Init(int w, int h)
{
    mutex.lock();
    this->mWidth = w;
    this->mHeight = h;
    if(mYUVdata[0])delete[] mYUVdata[0];mYUVdata[0] =nullptr;
    if(mYUVdata[1])delete[] mYUVdata[1];mYUVdata[1] =nullptr;
    if(mYUVdata[2])delete[] mYUVdata[2];mYUVdata[2] =nullptr;
    if(mCPUtex)glDeleteTextures(1,&mCPUtex);
    createTex();
    mutex.unlock();
}

void XVideoWidget::updateTex()
{
    //
//    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,mPBO[0]);
//    glBufferData(GL_PIXEL_UNPACK_BUFFER,mWidth*mHeight*3/2,0,GL_STREAM_DRAW);
//    //QOpenGLExtraFunctions::glMapBufferRange(GL_PIXEL_UNPACK_BUFFER,GL_WRITE_ONLY);
////    if(0)
////    {
////        unmap
////    }
//    glBindTexture(GL_TEXTURE_2D, mCPUtex); //2层绑定到V材质
//    glCopyTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,0,0,mWidth,mHeight*3/2,0);


    glActiveTexture(GL_TEXTURE0);//0 -> GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS-1
    glBindTexture(GL_TEXTURE_2D, mCPUtex); //2层绑定到V材质


    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    mWidth,mHeight, GL_RED, //一定不用GL_ALPHA
                    GL_UNSIGNED_BYTE, mYUVdata[0]);//修改材质内容(复制内存内容)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0,
                    mHeight, mWidth/2,mHeight/2, GL_RED, //一定不用GL_ALPHA
                    GL_UNSIGNED_BYTE, mYUVdata[1]);//修改材质内容(复制内存内容)
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    mWidth/2, mHeight, mWidth/2,mHeight/2, GL_RED, //一定不用GL_ALPHA
                    GL_UNSIGNED_BYTE, mYUVdata[2]);//修改材质内容(复制内存内容)
    //DMA->PBUFFER
    glUniform1i(mGPUtex, 0);//与shader uni建立关联
}

void XVideoWidget::createTex()
{
    //获得可用显卡贴图ID
    glGenTextures(1,&mCPUtex);
    //GPU绑定现在唯一可用贴图
    glBindTexture(GL_TEXTURE_2D, mCPUtex);
    //放大过滤，线性插值   GL_NEAREST(效率高，但马赛克严重)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //创建材质显存空间
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, mWidth,mHeight*3/2,
                 GL_FALSE, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    //分配材质内存空间
    mYUVdata[0] = new unsigned char[mWidth*mHeight];
    mYUVdata[1] = new unsigned char[mWidth*mHeight/4];
    mYUVdata[2] = new unsigned char[mWidth*mHeight/4];

//    glGenBuffers(2,mPBO);
//    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,mPBO[0]);
//    glBufferData(GL_PIXEL_UNPACK_BUFFER,mWidth*mHeight,0,GL_STREAM_DRAW);

}
