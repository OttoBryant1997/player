#ifndef IVIDEOCALL_H
#define IVIDEOCALL_H
struct AVFrame;
/**
 * @brief The IVideoCall class 窗口句柄的抽象类,便于扩展任意窗口类型
 */
class IVideoCall
{
public:
    virtual void Init(int w,int h) = 0;
    virtual void Repaint(AVFrame* frame) = 0;
};

#endif // IVIDEOCALL_H
