#ifndef _CAM_OE_RTSP_WRAPPER_H
#define _CAM_OE_RTSP_WRAPPER_H

#include <librealsense2/hpp/rs_internal.hpp>
#include <vector>

class IcamOERtsp
{
    public:
        virtual std::vector<rs2_video_stream> queryProfiles() = 0;
        virtual int addProfile(rs2_video_stream) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void close() = 0;
};

#endif // _CAM_OE_RTSP_WRAPPER_H


