﻿/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include <thread>


#include "ReadVideoFileThread.h"

#if defined(WIN32)
#include <WinSock2.h>
#include <Windows.h>
#include <direct.h>
#include <io.h> //C (Windows)    access
#else
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

void Sleep(long mSeconds)
{
    usleep(mSeconds * 1000);
}

#endif


ReadVideoFileThread::ReadVideoFileThread()
{
    mVideoCallBack = nullptr;

#ifdef USE_FFMPEG_VIDEO_PARSE
    mFFmpegVideoParsing = new FFmpegVideoParsing();
#else
    mNaluParsing  = new NALUParsing();
#endif

    mVideoDecoder = new VideoDecoder();
}

ReadVideoFileThread::~ReadVideoFileThread()
{

}

void ReadVideoFileThread::startRead(char* filePath, AVCodecID id)
{
    strcpy(mFileName, filePath);

    //启动新的线程实现读取视频文件
    std::thread([=]
    {
        for (int i=0;i<10000;i++) //测试稳定性

        this->run(id);

    }).detach();
}

void ReadVideoFileThread::run(AVCodecID id)
{
    bool openDecorderSucceed = mVideoDecoder->openDecoder(id);

    char *fileName = mFileName;
    FILE *fp = fopen(fileName, "rb");
    if (fp == nullptr)
    {
        fprintf(stderr, "H264 file not exist! \n");
        return;
    }

    int frameNum = 0; //当前播放的帧序号

#ifdef USE_FFMPEG_VIDEO_PARSE

    auto getVideoFrameFunc = [&](uint8_t *buffer, const int &size, void *param)
    {
        uint8_t *bufferYUV = nullptr;
        int width  = 0;
        int height = 0;
fprintf(stderr, "%s %d %d %d\n", __FUNCTION__, buffer, size, openDecorderSucceed);
        mVideoDecoder->decode(buffer, size, bufferYUV, width, height);

        if (bufferYUV != nullptr)
        {
            int frameRate = mVideoDecoder->getFrameRate(); //获取帧率

            /// h264裸数据不包含时间戳信息  因此只能根据帧率做同步
            /// 需要成功解码一帧后 才能获取到帧率
            /// 为0说明还没获取到 则直接显示
            if (frameRate != 0)
            {
                Sleep(1000 / frameRate);
            }

            //然后传给主线程显示
            doDisplayVideo(bufferYUV, width, height, ++frameNum);
        }
    };

    mFFmpegVideoParsing->start(getVideoFrameFunc, nullptr);

    while(!feof(fp))
    {
        char buf[10240] = {0};
        int size = fread(buf, 1, 1024, fp);//从h264文件读1024个字节 (模拟从网络收到h264流)
        int nCount = mFFmpegVideoParsing->inputVideoBuffer((const uint8_t*)buf, size);
    }

    fclose(fp);

    mFFmpegVideoParsing->stop();

#else
    if (id == AV_CODEC_ID_H264)
    {
        mNaluParsing->setVideoType(T_NALU_H264);
    }
    else
    {
        mNaluParsing->setVideoType(T_NALU_H265);
    }
    while(!feof(fp))
    {
        char buf[10240];
        int size = fread(buf, 1, 1024, fp);//从h264文件读1024个字节 (模拟从网络收到h264流)
        int nCount = mNaluParsing->inputH264Data((uchar*)buf,size);

        while(1)
        {
            //从前面读到的数据中获取一个nalu
            T_NALU* nalu = mNaluParsing->getNextFrame();
            if (nalu == nullptr) break;

            uint8_t *bufferYUV = nullptr;
            int width;
            int height;

            if (nalu->type == T_NALU_H264)
            {
                mVideoDecoder->decode(nalu->nalu.h264Nalu.buf, nalu->nalu.h264Nalu.len, bufferYUV, width, height);
            }
            else if (nalu->type == T_NALU_H265)
            {
                mVideoDecoder->decode(nalu->nalu.h265Nalu.buf, nalu->nalu.h265Nalu.len, bufferYUV, width, height);
            }

            if (bufferYUV != nullptr)
            {
                int frameRate = mVideoDecoder->getFrameRate(); //获取帧率

                /// h264裸数据不包含时间戳信息  因此只能根据帧率做同步
                /// 需要成功解码一帧后 才能获取到帧率
                /// 为0说明还没获取到 则直接显示
                if (frameRate != 0)
                {
                    Sleep(1000 / frameRate);
                }

                //然后传给主线程显示
                doDisplayVideo(bufferYUV, width, height, ++frameNum);
            }

            NALUParsing::FreeNALU(nalu);
        }
    }
#endif

    mVideoDecoder->closeDecoder();
}

///显示视频数据，此函数不宜做耗时操作，否则会影响播放的流畅性。
void ReadVideoFileThread::doDisplayVideo(const uint8_t *yuv420Buffer, const int &width, const int &height, const int &frameNum)
{
//    fprintf(stderr, "%s \n", __FUNCTION__);
    if (mVideoCallBack != nullptr)
    {
        VideoFramePtr videoFrame = std::make_shared<VideoFrame>();

        VideoFrame * ptr = videoFrame.get();

        ptr->initBuffer(width, height);
        ptr->setYUVbuf(yuv420Buffer);

        mVideoCallBack->onDisplayVideo(videoFrame, frameNum);
    }
}
