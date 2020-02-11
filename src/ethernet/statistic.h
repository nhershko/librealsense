#ifndef _CLIENT_COMMON_H
#define _CLIENT_COMMON_H
#pragma once
#include "time.h"
#include <chrono>

class stream_statistic {

};

class statistic
{
    public:
        static std::chrono::system_clock::time_point depthClockBegin, depthClockEnd, colorClockBegin, colorClockEnd, prevDepthClockBegin, prevColorClockBegin;
        static int depthframeCounter, colorframeCounter;
        static std::chrono::duration<double> depthProcessingTime, colorProcessingTime, depthGetFrameDiffTime, colorGetFrameDiffTime ;
        static float avgDepthProcessingTime, avgColorProcessingTime, avgDepthGettingTime;
};
#endif