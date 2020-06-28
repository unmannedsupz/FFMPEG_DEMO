
win32{

    contains(QT_ARCH, i386) {
        message("32-bit")
        INCLUDEPATH += $$PWD/win32/ffmpeg/include

        LIBS += -L$$PWD/win32/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

    } else {
        message("64-bit")
        INCLUDEPATH += $$PWD/win64/ffmpeg/include

        LIBS += -L$$PWD/win64/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

    }

}

unix{
    contains(QT_ARCH, i386) {
        message("32-bit, 请自行编译32位库!")
    } else {
        message("64-bit")
        INCLUDEPATH += $$PWD/linux/ffmpeg/include

        LIBS += -L$$PWD/linux/ffmpeg/lib  -lavformat  -lavcodec -lavdevice -lavfilter -lavutil -lswresample -lswscale -lpostproc

        LIBS += -lpthread -ldl
    }

#QMAKE_POST_LINK 表示编译后执行内容
#QMAKE_PRE_LINK 表示编译前执行内容

#解压库文件
#QMAKE_PRE_LINK += "cd $$PWD/lib/linux && tar xvzf ffmpeg.tar.gz "
system("cd $$PWD/lib/linux && tar xvzf ffmpeg.tar.gz")

}
