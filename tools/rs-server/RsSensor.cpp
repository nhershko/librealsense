#include <iostream>
#include <math.h>
#include "RsDevice.hh"

RsSensor::RsSensor(rs2::sensor sensor)
{
	m_sensor = sensor;
	for (rs2::stream_profile stream_profile : m_sensor.get_stream_profiles())
	{
		if (stream_profile.is<rs2::video_stream_profile>())
		{
			//make a map with all the sensor's stream profiles
			m_stream_profiles.emplace(getStreamProfileKey(stream_profile), stream_profile.as<rs2::video_stream_profile>());
		}
	}
}

int RsSensor::open(std::unordered_map<long long int, rs2::frame_queue> &stream_profiles_queues)
{
	std::vector<rs2::stream_profile> requested_stream_profiles;
	for (auto stream_profile : stream_profiles_queues)
	{
		//make a vector of all requested stream profiles
		long long int stream_profile_key = stream_profile.first;
		requested_stream_profiles.push_back(m_stream_profiles.at(stream_profile_key));
	}
	try
	{
		m_sensor.open(requested_stream_profiles);
	}
	catch (const std::exception &e)
    {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int RsSensor::start(std::unordered_map<long long int, rs2::frame_queue> &stream_profiles_queues)
{
	auto callback = [&](const rs2::frame &frame) {
		long long int profile_key = getStreamProfileKey(frame.get_profile());
		//check if profile exists in map:

		if (stream_profiles_queues.find(profile_key) != stream_profiles_queues.end())
		{
			//push frame to its queue
			stream_profiles_queues[profile_key].enqueue(frame);
		}
	};

	m_sensor.start(callback);

	return EXIT_SUCCESS;
}

long long int RsSensor::getStreamProfileKey(rs2::stream_profile profile)
{
	long long int key;
	key = profile.stream_type() * pow(10, 12) + profile.format() * pow(10, 10) + profile.fps() * pow(10, 8);
	if (profile.is<rs2::video_stream_profile>())
	{
		rs2::video_stream_profile video_stream_profile = profile.as<rs2::video_stream_profile>();
		key += video_stream_profile.width() * pow(10, 4) + video_stream_profile.height();
	}
	return key;
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

int RsSensor::getStreamProfileBpp(rs2_format format)
{
	int bpp = 0;
	switch (format)
	{            
		case  RS2_FORMAT_RGB8: 
		{
			bpp = 3;
			break;
		}           
		case  RS2_FORMAT_BGR8: 
		{
			bpp = 3;
			break;
		}
		case  RS2_FORMAT_RGBA8:  
		{
			bpp = 3;
			break;
		}         
		case  RS2_FORMAT_BGRA8: 
		{
			bpp = 3;
			break;
		}       
		case  RS2_FORMAT_Z16:   
		case  RS2_FORMAT_Y16: 
		case  RS2_FORMAT_Y8:             
		case  RS2_FORMAT_RAW16:          
		case  RS2_FORMAT_YUYV:  
		case  RS2_FORMAT_UYVY:
		{
			bpp = 2;
			break;
		}
		default:
			bpp = 0;
			break;
	}   
	return bpp;
}
