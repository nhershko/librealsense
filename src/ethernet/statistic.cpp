#include "statistic.h"

    int statistic::depthframeCounter = 0;
    int statistic::colorframeCounter = 0;
    std::chrono::duration<double> statistic::depthProcessingTime;
    std::chrono::duration<double> statistic::colorProcessingTime;
    std::chrono::duration<double> statistic::depthGetFrameDiffTime;
    std::chrono::duration<double> statistic::colorGetFrameDiffTime;
    float statistic::avgDepthProcessingTime = 0;
    float statistic::avgColorProcessingTime = 0;
    float statistic::avgDepthGettingTime = 0;
    std::chrono::system_clock::time_point statistic::depthClockBegin;
    std::chrono::system_clock::time_point statistic::depthClockEnd;
    std::chrono::system_clock::time_point statistic::colorClockBegin;
    std::chrono::system_clock::time_point statistic::colorClockEnd;
    std::chrono::system_clock::time_point statistic::prevDepthClockBegin;
    std::chrono::system_clock::time_point statistic::prevColorClockBegin;
