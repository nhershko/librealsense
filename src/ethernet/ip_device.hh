#pragma once

//////////////////////////////////////////////////////////////////////////
#ifdef _WIN32

#define _WINSOCKAPI_ 

#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")

#include <SIGNAL.H>

#endif
//////////////////////////////////////////////////////////////////////////

#include <librealsense2/rs.hpp>
#include "option.h"

#include "camOERtspClient.h"
#include "software-device.h"

#include <list>

#include "rs_rtp_stream.hh"
#include "rs_rtp_callback.hh"
#include "ip_sensor.hh"

#define NUM_OF_SENSORS 2

#define POLLING_SW_DEVICE_STATE_INTERVAL 1000

class ip_device
    {

    public:

        #ifdef _WIN32
            __declspec(dllexport)
        #endif
        static rs2::software_device create_ip_device(const char* ip_address);

        ~ip_device();
        
    private:

        volatile bool is_device_alive;

        memory_pool* memPool;
    
        std::string ip_address;

        ip_sensor* remote_sensors[NUM_OF_SENSORS];

        //todo: consider wrapp all maps to single container 
        std::map<long long int, std::shared_ptr<rs_rtp_stream>> streams_collection;

        std::map<long long int, std::thread> inject_frames_thread;

        std::map<long long int, rs_rtp_callback*> rtp_callbacks;

        rs2::software_device sw_dev;

        std::thread sw_device_status_check;

        ip_device(std::string ip_address, rs2::software_device sw_device);

        bool init_device_data();

        void polling_state_loop();

        void inject_frames_loop(std::shared_ptr<rs_rtp_stream> rtp_stream);

        void update_sensor_state(int sensor_index,std::vector<rs2::stream_profile> updated_streams);

        std::vector<rs2_video_stream> query_streams(int sensor_id);

        void recover_rtsp_client(int sensor_index);
    };