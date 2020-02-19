#ifndef _RS_SENSOR_HH
#define _RS_SENSOR_HH

#include <librealsense2/rs.hpp>
#include <unordered_map>
#include <chrono>
#include "compression/icompression.h"

class RsSensor
{
public:
	RsSensor(rs2::sensor sensor, rs2::device device);
	int open(std::unordered_map<long long int, rs2::frame_queue> &stream_profiles_queues);
	int start(std::unordered_map<long long int, rs2::frame_queue> &stream_profiles_queues);
	rs2::sensor &getRsSensor() { return m_sensor; }
	std::unordered_map<long long int, rs2::video_stream_profile> getStreamProfiles() { return m_stream_profiles; }
	long long int getStreamProfileKey(rs2::stream_profile profile);
	std::string get_sensor_name();
	static int getStreamProfileBpp(rs2_format format);
	rs2::device getDevice() { return m_device; }

private:
	rs2::sensor m_sensor;
	std::unordered_map<long long int, rs2::video_stream_profile> m_stream_profiles;
	std::unordered_map<long long int, ICompression *> m_iCompress;
	rs2::device m_device;
	std::unordered_map<long long int,std::chrono::high_resolution_clock::time_point> prevSample;
};

#endif
