#ifndef _I_CAMOE_RTSP_H
#define _I_CAMOE_RTSP_H

#include "camOESink.h"
#include <librealsense2/hpp/rs_internal.hpp>
#include "rtp_callback.hh"
#include <vector>

// TODO: Change this to a class that inherits from lrs class
struct rtp_rs_video_stream
{
  rs2_video_stream video_stream;
  int rtp_uid;
};

class IcamOERtsp
{
    public:
        virtual std::vector<rtp_rs_video_stream> queryStreams() = 0;
        virtual int addStream(rtp_rs_video_stream stream, rtp_callback* frameCallBack) = 0;
        virtual int start() = 0;
        virtual int stop(rtp_rs_video_stream stream) = 0;
        virtual int stop() = 0;
        virtual int close() = 0;
        
};

#endif // _I_CAMOE_RTSP_H


