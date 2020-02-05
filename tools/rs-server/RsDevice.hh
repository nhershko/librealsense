#ifndef _RS_CAMERA_HH
#define _RS_CAMERA_HH

#include <librealsense2/rs.hpp>
#include "RsSensor.hh"

class RsDevice
{
public:
	RsDevice();
	~RsDevice();
	std::vector<RsSensor> &getSensors() { return m_sensors; }
	rs2::device getRs2Device() { return m_device; }
private:
	rs2::device m_device;
	std::vector<RsSensor> m_sensors;
};

#endif
