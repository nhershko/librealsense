#include <iostream>
#include "RsCamera.hh"

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
