// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2018 Intel Corporation. All Rights Reserved.
#pragma once

#include "core/streaming.h"
#include "device.h"
#include "context.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/hpp/rs_internal.hpp>
#include <librealsense2/hpp/rs_sensor.hpp>


//RTSP_CLIENT
#include "common/camOE_stream_profile.hh"

#include "rtsp_client/environment.h"
#include "rtsp_client/rtspconnectionclient.h"
#include "rtsp_client/sdpclient.h"
#include "rtsp_client/mkvclient.h"
#include "rtsp_client/callbacks.h"



#if defined(_WIN32)
  #include <windows.h> 
  #include <stdio.h>
  #include <tchar.h>
  #include <strsafe.h>
#else
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/ioctl.h>
  #include <fcntl.h>
  #include<signal.h>
#endif

namespace rs2
{
	class software_sensor;
	class software_device_info;
	
	void ethernet_device_deleter(void* p) {

	}

	class ethernet_device : public rs2::software_device
	{
		public: 

		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		ethernet_device();

		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		ethernet_device(std::string ip_address);
			
		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		virtual ~ethernet_device();
		
		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		void start();

		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		void stop();

		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		rs2_intrinsics get_intrinsics();
		
		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		virtual std::vector<sensor> query_sensors() const override;

		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		rs2_device* get_device();

		void add_frame_to_queue(int type,Frame* frame);

		

	private:

		rs2_video_stream rtsp_stream_to_rs_video_stream(camOE_stream_profile rtsp_stream);

		void incomming_server_frames_handler();
		
		void inject_frames_to_sw_device();

		rs2_software_video_frame& get_frame();


		std::queue<Frame*> depth_frames;
		std::queue<Frame*> color_frames;

		unsigned int frame_queue_max_size = 30;
		
		int frame_number = 0;
		std::chrono::high_resolution_clock::time_point last;

		std::string ip_address;
			
		std::thread t,t2;

		std::mutex mtx,mtx2;
		
		// TODO: modify dynamically 
		rs2_sensor* sensors[2];
		rs2_stream_profile* profiles[2];

		rs2_device* dev; // Create software-only device
		Environment* env;
		rs2_software_video_frame last_frame[2];
		std::vector<uint8_t> pixels_buff[2];
};

	class RS_RTSPFrameCallback: public RTSPCallback
			{
				public:

				int id;

				RS_RTSPFrameCallback(ethernet_device* ethernet_device, const std::string & output) : RTSPCallback(output)
				{
					dev = ethernet_device;
				}

				bool onData(const char* id, unsigned char* buffer, ssize_t size, struct timeval presentationTime) override
				{
					std::cout << "CB_ID " << this->id << "sink id " << id << " " << size << " ts:" << presentationTime.tv_sec << "." << presentationTime.tv_usec << std::endl;
					dev->add_frame_to_queue(this->id,new Frame((char*)buffer,size,presentationTime));
					return true;
				}

				bool onData(char sink_id, const char* id, unsigned char* buffer, ssize_t size, struct timeval presentationTime) override
				{
					//std::cout << "CB_ID " << this->id << "sink id " << std::to_string(sink_id) << " " << size << " ts:" << presentationTime.tv_sec << "." << presentationTime.tv_usec << std::endl;
					if("96"==std::to_string(sink_id))
						dev->add_frame_to_queue(0,new Frame((char*)buffer,size,presentationTime));
					else if ("97"==std::to_string(sink_id))
						dev->add_frame_to_queue(1,new Frame((char*)buffer,size,presentationTime));
					return true;
				}

				private:
					rs2::ethernet_device* dev;
			};
}