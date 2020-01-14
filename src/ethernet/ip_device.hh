//#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
//#include <librealsense2/hpp/rs_internal.hpp>
#pragma once

#include <librealsense2/rs.hpp>
#include "camOERtspClient.h"
#include "software-device.h"

#include "IdecompressFrame.h"
#include "decompressFrameFactory.h"

#include "rtp_callback.hh"

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

        std::queue<Tmp_Frame*> frame_queues[MAX_ACTIVE_STREAMS];

        std::mutex queue_locks[MAX_ACTIVE_STREAMS];

        IcamOERtsp* rtsp_clients[SENSORS_NUMBER] = {NULL};

        rtp_callback* rtp_callbacks[MAX_ACTIVE_STREAMS];
        
        rs2::software_device sw_dev;

        IdecompressFrame* idecomress;

        std::thread sw_device_status_check;

        std::thread inject_frames_thread[MAX_ACTIVE_STREAMS];

        volatile bool injected_thread_active[MAX_ACTIVE_STREAMS];

        ip_device(std::string ip_address, rs2::software_device sw_device);

        bool init_device_data();

        void polling_state_loop();

        void inject_frames_loop(int stream_index);

        void update_sensor_stream(int sensor_index,std::vector<rs2::stream_profile> updated_streams);

        // sensors
        rs2::software_sensor* sensors[SENSORS_NUMBER];

        // frame data buffers
        rs2_software_video_frame last_frame[MAX_ACTIVE_STREAMS];
        // pixels data 
        std::vector<uint8_t> pixels_buff[MAX_ACTIVE_STREAMS];
        //profiles
        rs2::stream_profile profiles[MAX_ACTIVE_STREAMS];

        //rtp/rtsp protocol

        // => describe 
        std::vector<rs2_video_stream> query_server_streams();

        void add_frame_to_queue(Tmp_Frame* frame);

        void clean_frames_queue(int stream_index);

        void recover_rtsp_client(int sensor_index);

/*
        void tear_down();
        void start();
        void stop();
        void describe();
*/
    };







    //}//namespace