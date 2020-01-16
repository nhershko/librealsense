#ifndef _RS_SENSOR_HH
#define _RS_SENSOR_HH

#include <librealsense2/rs.hpp>
#include <unordered_map>

class RsSensor
{
public:
	RsSensor(rs2::sensor sensor);
	int open(std::unordered_map<long long int, rs2::frame_queue> &stream_profiles_queues);
	int start(std::unordered_map<long long int, rs2::frame_queue> &stream_profiles_queues);
	rs2::sensor &getRsSensor() { return m_sensor; }
	std::unordered_map<long long int, rs2::video_stream_profile> getStreamProfiles() { return m_stream_profiles; }
	long long int getStreamProfileKey(rs2::stream_profile profile);
	std::string get_sensor_name();

private:
	rs2::sensor m_sensor;
	std::unordered_map<long long int, rs2::video_stream_profile> m_stream_profiles;
};

#endif
