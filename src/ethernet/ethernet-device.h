// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2018 Intel Corporation. All Rights Reserved.
#pragma once

#include "core/streaming.h"
#include "device.h"
#include "context.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/hpp/rs_internal.hpp>

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
		software_sensor* query_ethernet_device_sensors();

		#ifdef _WIN32
		__declspec(dllexport)
		#endif
		rs2_device* ethernet_device::get_device();

	private:

		void create_sensors();

		void read_frame();

		void ethernet_device::thread_main();

		rs2_software_video_frame& get_frame();

		std::string pipe_name = "\\\\.\\pipe\\DepthStreamSink";

		/*
		rs2_intrinsics get_intrinsics()
			{
				rs2_intrinsics intrinsics = { W, H,
					(float)W / 2, (float)H / 2,
					(float)W / 2, (float)H / 2,
					RS2_DISTORTION_BROWN_CONRADY ,{ 0,0,0,0,0 } };

				return intrinsics;
			}

			HANDLE create_named_pipe(std::string stream_name) {
				HANDLE hPipe;

				hPipe = CreateNamedPipeA(
					pipe_name.c_str(),
					PIPE_ACCESS_DUPLEX,
					PIPE_TYPE_BYTE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
					1,
					1024 * 1024 * 32,
					1024 * 1024 * 32,
					NMPWAIT_USE_DEFAULT_WAIT,
					NULL);

				return hPipe;
			}



			rs2_software_video_frame& get_frame() {
				std::lock_guard<std::mutex> lck(mtx);
				return depth_frame;
			}

			rs2_device* get_device() {
				return dev;
			}

			//void connect_ethernet_device();
		*
		/
		private:

			void create_sensors() {
				rs2_intrinsics depth_intrinsics = get_intrinsics();
				depth_sensor = rs2_software_device_add_sensor(dev, "Depth (Remote)", NULL);
				rs2_video_stream st = { RS2_STREAM_DEPTH, 0, 1, W,
										H, 30, BPP,
										RS2_FORMAT_Z16, depth_intrinsics };
				depth_stream = rs2_software_sensor_add_video_stream(depth_sensor, st, NULL);
				depth_frame.profile = depth_stream;
			}
			
			void ethernet_device::thread_main() {
				while (is_active) {
					read_frame();
				}
			}

			void read_frame()
			{
				std::lock_guard<std::mutex> lck(mtx);
				DWORD bytesRead = 0;
				if (ReadFile(hPipe, depth_frame.pixels, W * H * BPP, &bytesRead, NULL)) {
					std::cout << " Read: " << bytesRead << " Bytes from buffer!" << std::endl;
				}
				else
					std::cerr << "Cannot read from buffer!" << std::endl;

				using namespace std::chrono;
				auto now = system_clock::now();
				depth_frame.timestamp = time_point_cast<milliseconds>(now).time_since_epoch().count();
				rs2_software_sensor_on_video_frame(depth_sensor, depth_frame, NULL);

				depth_frame.frame_number++;
			}

			*/

			int frame_number = 0;
			std::chrono::high_resolution_clock::time_point last;
			
			software_sensor* remote_device_sensors;
			
			std::vector<uint8_t> pixels;
			std::thread t;
			
			const int W = 640;
			const int H = 480;
			const int BPP = 2;

			HANDLE hPipe;
			std::mutex mtx;
			rs2_stream_profile* depth_stream;
			rs2_software_video_frame depth_frame;
			rs2_sensor* depth_sensor;
			//volatile bool is_active = false;

			rs2_device* dev; // Create software-only device
	};
}