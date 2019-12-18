#ifndef _CAM_OE_RTSP_WRAPPER_H
#define _CAM_OE_RTSP_WRAPPER_H

#include "Profile.h"
#include <vector>

class IcamOERtsp
{
    public:
        //camOErtspWrapper(char* url);
        virtual std::vector<Profile> queryProfiles() = 0;
        virtual int addProfile(Profile) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void close() = 0;
    //private:
        //camOERTSPClient rtspClient;
        //BasicTaskScheduler* scheduler;
        //BasicUsageEnvironment* env; 
};

#endif // _CAM_OE_RTSP_WRAPPER_H


