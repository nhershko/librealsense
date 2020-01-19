//#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
//#include <librealsense2/hpp/rs_internal.hpp>
#pragma once

#include <librealsense2/rs.hpp>
#include "camOERtspClient.h"
#include "software-device.h"

#include "rtp_stream.hh"

#include "IdecompressFrame.h"
#include "decompressFrameFactory.h"

#include "rs_rtp_callback.hh"

#define MAX_ACTIVE_STREAMS 4

#define SENSORS_NUMBER 2



class ip_device
    {

    public:

        #ifdef _WIN32
            __declspec(dllexport)
        #endif
        static rs2::software_device create_ip_device(std::string ip_address);

        ~ip_device();
        
    private:

        volatile bool is_device_alive;
    
        std::string ip_address;

        std::map<int, int> active_stream_per_sensor;

        std::map<int, std::list<int>> streams_uid_per_sensor;

        std::map<int, std::shared_ptr<rs_rtp_stream>> streams_collection;

        std::map<int, std::thread> inject_frames_thread;

        std::map<int, rs_rtp_callback*> rtp_callbacks;

        //rs_rtp_callback* rtp_callbacks[MAX_ACTIVE_STREAMS];

        IcamOERtsp* rtsp_clients[SENSORS_NUMBER] = {NULL};

        
        
        rs2::software_device sw_dev;

        IdecompressFrame* idecomress;

        std::thread sw_device_status_check;

        ip_device(std::string ip_address, rs2::software_device sw_device);

        bool init_device_data();

        void polling_state_loop();

        void inject_frames_loop(std::shared_ptr<rs_rtp_stream> rtp_stream);

        void update_sensor_state(int sensor_index,std::vector<rs2::stream_profile> updated_streams);

        // sensors
        rs2::software_sensor* sensors[SENSORS_NUMBER];

        //rtp/rtsp protocol

        // => describe 
        std::vector<rtp_rs_video_stream> query_streams(int sensor_id);

        void recover_rtsp_client(int sensor_index);

/*
        void tear_down();
        void start();
        void stop();
        void describe();
*/
    };

    //}//namespace