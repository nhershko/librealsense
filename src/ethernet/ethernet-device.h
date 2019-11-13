// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2018 Intel Corporation. All Rights Reserved.
#pragma once

#include "core/streaming.h"
#include "device.h"
#include "context.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/hpp/rs_internal.hpp>
#include <librealsense2/hpp/rs_sensor.hpp>

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

			/**
			 * CTOR
			 * \param[in]  address       ip adress + port of remote device server in the following format. IP:PORT. E.G.: 1.1.1.1:8080
			 */
		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		ethernet_device();

		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		ethernet_device(std::string ipadress);
			
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

	private:

		void create_sensors();

		void read_frame();

		void thread_main();

		rs2_software_video_frame& get_frame();

		std::string pipe_name = "\\\\.\\pipe\\DepthStreamSink";

		//std::string pipe_name = "\\\\.\\pipe\\DepthStreamSink";
		

			int frame_number = 0;
			std::chrono::high_resolution_clock::time_point last;
			
			software_sensor* remote_device_sensors;
			
			std::vector<uint8_t> pixels;
			std::thread t;
			
			const int W = 640;
			const int H = 480;
			const int BPP = 2;

#if defined(_WIN32)
		HANDLE hPipe;
#else
		int fifo;
		int fd;
#endif
			std::mutex mtx;
			rs2_stream_profile* depth_stream;
			rs2_software_video_frame depth_frame;
			rs2_sensor* depth_sensor;
			//volatile bool is_active = false;
			
			rs2_device* dev; // Create software-only device
	};
}