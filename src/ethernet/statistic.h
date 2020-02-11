#ifndef _CLIENT_COMMON_H
#define _CLIENT_COMMON_H
#pragma once
#include "time.h"
#include <chrono>
#include <map>

class stream_statistic {
    public:
        double avgProcessingTime = 0, avgGettingTime = 0;
        std::chrono::system_clock::time_point clockBegin, clockEnd, prevClockBegin;
        std::chrono::duration<double> processingTime, getFrameDiffTime;
        int frameCounter = 0;

};

class statistic
{
    public:
       static std::map<int, stream_statistic*> statisticStreams;
};
#endif