#ifndef _I_CAMOE_RTSP_H
#define _I_CAMOE_RTSP_H

#include "camOESink.h"
#include <librealsense2/hpp/rs_internal.hpp>
#include "rtp_callback.hh"
#include <vector>
typedef struct device_data
{
    std::string serial_num;
    std::string name;
    std::string usb_type;
} device_data;

class IcamOERtsp
{
    public:
        virtual std::vector<rs2_video_stream> queryStreams() = 0;
        virtual int addStream(rs2_video_stream stream, rtp_callback* frameCallBack) = 0;
        virtual int start() = 0;
        virtual int stop(rs2_video_stream stream) = 0;
        virtual int stop() = 0;
        virtual int close() = 0;
        virtual device_data getDeviceData() = 0;
};

#endif // _I_CAMOE_RTSP_H


