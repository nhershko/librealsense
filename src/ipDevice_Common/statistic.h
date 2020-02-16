#ifndef _CLIENT_COMMON_H
#define _CLIENT_COMMON_H
#pragma once
#include "time.h"
#include <chrono>
#include <map>
#include <queue>

class stream_statistic {
    public:
        std::queue<std::chrono::system_clock::time_point> clockBeginVec;
        std::chrono::system_clock::time_point clockBegin, clockEnd, prevClockBegin, compressionBegin, compressionEnd, decompressionBegin, decompressionEnd;
        std::chrono::duration<double> processingTime, getFrameDiffTime, compressionTime, decompressionTime;
        int frameCounter = 0, compressionFrameCounter = 0, decompressionFrameCounter = 0;
        double avgProcessingTime = 0, avgGettingTime = 0, avgCompressionTime = 0, avgDecompressionTime = 0;
        long long  decompressedSizeSum = 0, compressedSizeSum = 0;
};

class statistic
{
    public:
        static std::map<int, stream_statistic*>& getStatisticStreams()
        { 
            static std::map<int, stream_statistic*> statisticStreams;
            return statisticStreams;
        };
    private:
        
};
#endif