#ifndef _CAMERA_HH
#define _CAMERA_HH

#include <librealsense2/rs.hpp>
#include <map>


class RsSensor
{
public:
	RsSensor(rs2::sensor sensor);
	int open(std::map<int, rs2::frame_queue> &stream_profiles_queues);
	int start(std::map<int, rs2::frame_queue> &stream_profiles_queues);
	rs2::sensor &getRsSensor() { return m_sensor; }
	std::vector<rs2::video_stream_profile> getStreamProfiles() { return m_stream_profiles; }
	int getStreamProfileIndex(rs2::stream_profile profile);  
	int getStreamProfileIndex(rs2_stream type, rs2_format format, int fps, int width, int height);
	std::string get_sensor_name();

private:
	rs2::sensor m_sensor;
	std::vector<rs2::video_stream_profile> m_stream_profiles;
};

class RsCamera
{
public:
	RsCamera();
	~RsCamera();
	std::vector<RsSensor> &getSensors() { return m_sensors; }

private:
	rs2::device m_dev;
	std::vector<RsSensor> m_sensors;
};

#endif
