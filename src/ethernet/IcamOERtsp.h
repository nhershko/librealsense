#ifndef _I_CAMOE_RTSP_H
#define _I_CAMOE_RTSP_H

#include <librealsense2/hpp/rs_internal.hpp>
#include <vector>

class IcamOERtsp
{
    public:
        virtual std::vector<rs2_video_stream> queryStreams() = 0;
        virtual int addStream(rs2_video_stream) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void close() = 0;
};

#endif // _I_CAMOE_RTSP_H


