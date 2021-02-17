#include "avthread.h"

AVThread::AVThread(QObject *parent) : QThread(parent)
{

}

AVThread::~AVThread()
{
    mThreadShouldExit = true;
    wait();
    delete mDecoder;
}


