#ifndef _RS_CAMERA_HH
#define _RS_CAMERA_HH

#include <librealsense2/rs.hpp>
#include "RsSensor.hh"


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
