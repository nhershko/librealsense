#include <iostream>
#include <mutex>
#include <thread>

#include "RsCamera.h"

RsSensor::RsSensor(rs2::sensor sensor)
{
	//std::cerr << "RsSensor constructor" << std::endl;
	m_sensor = sensor;
	for (rs2::stream_profile stream_profile : m_sensor.get_stream_profiles())
	{
		if (stream_profile.is<rs2::video_stream_profile>())
		{
			m_stream_profiles.push_back(stream_profile.as<rs2::video_stream_profile>());
		}
	}
}

RsSensor::~RsSensor()
{
	//std::cerr << "RsSensor destructor" << std::endl;
}

int RsSensor::open(std::map<int, rs2::frame_queue> &stream_profiles_queues)
{
	std::vector<rs2::stream_profile> stream_profiles;
	for (auto stream_profile : stream_profiles_queues)
	{
		//make a vector of all requested stream profiles
		stream_profiles.push_back(m_stream_profiles[stream_profile.first]);
	}
	try
	{
		m_sensor.open(stream_profiles);
	}
	catch (...)
	{
		std::cerr << "unsupported combination of streams" << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int RsSensor::start(std::map<int, rs2::frame_queue> &stream_profiles_queues)
{
	auto callback = [&](const rs2::frame &frame) {
		int profile_indx = getStreamProfileIndex(frame.get_profile());
		//check if profile exists in map:
		if (profile_indx != -1 && stream_profiles_queues.find(profile_indx) != stream_profiles_queues.end())
		{
			//push frame to its queue
			stream_profiles_queues[profile_indx].enqueue(frame);
		}
	};

	m_sensor.start(callback);

	return EXIT_SUCCESS;
}

int RsSensor::getStreamProfileIndex(rs2::stream_profile profile)
{
	if (profile.is<rs2::video_stream_profile>())
	{
		rs2::video_stream_profile video_stream_profile = profile.as<rs2::video_stream_profile>();
		return getStreamProfileIndex(video_stream_profile.stream_type(), video_stream_profile.format(), video_stream_profile.fps(), video_stream_profile.width(), video_stream_profile.height());
	}
	else
	{
		return -1; //only video_stream_profile is supported
	}
	
}
int RsSensor::getStreamProfileIndex(rs2_stream type, rs2_format format, int fps, int width, int height)
{
	int index = 0;
	for (rs2::video_stream_profile video_stream_profile : m_stream_profiles)
	{
			if (video_stream_profile.stream_type() == type &&
				video_stream_profile.format() == format &&
				video_stream_profile.width() == width &&
				video_stream_profile.height() == height &&
				video_stream_profile.fps() == fps)
			{
				return index;
			}
		index++;
	}
	return -1;
}

std::string RsSensor::get_sensor_name()
{
	if (m_sensor.supports(RS2_CAMERA_INFO_NAME))
	{
		return m_sensor.get_info(RS2_CAMERA_INFO_NAME);
	}
	else
	{
		return "Unknown Sensor";
	}
}

RsCamera::RsCamera()
{
	//get RS device
	std::cerr << "RsCamera constructor" << std::endl;
	// The context represents the current platform with respect to connected devices
	rs2::context ctx;
	rs2::device_list devices = ctx.query_devices();
	rs2::device dev;
	if (devices.size() == 0)
	{
		std::cerr << "No device connected, please connect a RealSense device" << std::endl;
		rs2::device_hub device_hub(ctx);
		m_dev = device_hub.wait_for_device();
	}
	else
	{
		m_dev = devices[0]; // Only one device is supported
	}

	//get RS sensors
	for (auto &sensor : m_dev.query_sensors())
	{
		m_sensors.push_back(RsSensor(sensor));
	}
}

RsCamera::~RsCamera()
{
	std::cerr << "RsCamera destructor" << std::endl;
}
