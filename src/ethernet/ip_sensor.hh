/*
this class hold ip sensor data.
mainly: 
1. sw sensor
2. relative streams
3. state -> on/off
*/

#include <librealsense2/rs.hpp>
#include "software-device.h"
#include <list>

class ip_sensor
{
private:
    
    rs2::software_sensor* sw_sensor;

    std::list<long long int> active_streams_keys;

    bool enabled;

public:
    ip_sensor(/* args */);
    ~ip_sensor();
};

ip_sensor::ip_sensor(/* args */)
{
}

ip_sensor::~ip_sensor()
{
}
