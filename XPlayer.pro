QT       += core gui opengl multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += $$PWD\include
#message($$INCLUDEPATH)


opt = $$find(QMAKESPEC,"win64")
isEmpty(opt)
{
message("Using 32-bit libs")
}
!isEmpty(opt)
{
}
win32
{
    LIBS += -L$$PWD\lib\win32 \
    -lavcodec -lavformat -lavutil -lswscale -lswresample
}


SOURCES += \
    avthread.cpp \
    commontools.cpp \
    main.cpp \
    widget.cpp \
    xaudioplay.cpp \
    xaudiothread.cpp \
    xdecode.cpp \
    xdemux.cpp \
    xdemuxthread.cpp \
    xresample.cpp \
    xvideothread.cpp \
    xvideowidget.cpp

HEADERS += \
    IVideoCall.h \
    avthread.h \
    commontools.h \
    widget.h \
    xaudioplay.h \
    xaudiothread.h \
    xdecode.h \
    xdemux.h \
    xdemuxthread.h \
    xresample.h \
    xvideothread.h \
    xvideowidget.h

FORMS += \
    widget.ui



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
