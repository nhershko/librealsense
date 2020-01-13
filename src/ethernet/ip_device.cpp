#include "ip_device.hh"
#include <librealsense2/rs.hpp>

// michalpr - remove this. this is a workaround until callback branch will be merged
void ipDeviceFrameCallBack(u_int8_t* buf, unsigned int size, struct timeval presentationTime)
{
    std::cout << "myFrameCallBack. size = " << size << " time (sec) = " << presentationTime.tv_sec << "\n";
}

ip_device::~ip_device()
{
    is_device_alive = false;
    sw_device_status_check.join();
    std::cout << "destroy ip_device\n";
}

ip_device::ip_device(std::string ip_address, rs2::software_device sw_device)
{
    this->ip_address = ip_address;
    this->sw_dev = sw_device;
    this->is_device_alive = true;

    rtsp_clients[rs2_stream::RS2_STREAM_DEPTH-1] = camOERTSPClient::getRtspClient(std::string("rtsp://" + ip_address + ":8554/depth").c_str(),"ethernet_device");
	((camOERTSPClient*)rtsp_clients[rs2_stream::RS2_STREAM_DEPTH-1])->initFunc();
    rtsp_clients[rs2_stream::RS2_STREAM_COLOR-1] = camOERTSPClient::getRtspClient(std::string("rtsp://" + ip_address + ":8554/color").c_str(),"ethernet_device");
	((camOERTSPClient*)rtsp_clients[rs2_stream::RS2_STREAM_COLOR-1])->initFunc();
    
    //init device data
    init_device_data();

    //poll sw device streaming state
    this->sw_device_status_check = std::thread(&ip_device::polling_state_loop,this);
}

std::vector<rs2_video_stream> ip_device::query_server_streams()
{
    std::cout << "Querry Sensors\n";
	std::vector<rs2_video_stream> streams;

	int stream_uid=0;
	for (size_t i = 0; i < SENSORS_NUMBER; i++)
	{
		if (rtsp_clients[i]==NULL)
			continue;
		std::vector<rs2_video_stream> sensor_streams = rtsp_clients[i]->queryStreams();
		for (size_t j = 0; j < sensor_streams.size() ; j++)
		{
		sensor_streams[j].uid = stream_uid;
		streams.insert(streams.end(),sensor_streams[j]);
		stream_uid++;
        }
	}
    return streams;
}

bool ip_device::init_device_data()
{
    //TODO: getting streams per sensor
    auto streams = query_server_streams();

    for (size_t i = 0; i < streams.size(); i++)
	{
		rs2_video_stream st = streams[i];//rtsp_stream_to_rs_video_stream(streams.front());

        std::string sensor_name;
        if (st.type==RS2_STREAM_DEPTH)
			sensor_name = "Depth (Remote)";
        else 
            sensor_name = "Color (Remote)";
			
        rs2::software_sensor tmp_sensor= sw_dev.add_sensor(sensor_name);

        rs2::stream_profile profile = tmp_sensor.add_video_stream(st,i==0);

		last_frame[i].bpp = st.bpp;
		last_frame[i].profile = profile;
		last_frame[i].stride = st.bpp * st.width;
		pixels_buff[i].resize(last_frame[i].stride * st.height, 0);
		last_frame[i].pixels = pixels_buff[i].data();
		last_frame[i].deleter = {};//&ethernet_device_deleter;
		
		//quese array is 0 based so setting type -1 as address
		//inject_threads[i] = std::thread(&rs2::ethernet_device::pull_from_queue,this,available_streams[i].type-1);
	}
    
    return true;
}

void ip_device::polling_state_loop()
{
        while(this->is_device_alive)
        {
            //TODO: consider using sensor id as vector id (indexer)
            std::vector<rs2::sensor> sensors = this->sw_dev.query_sensors();
            for (size_t i = 0; i < sensors.size(); i++)
            {
                auto current_active_streams = sensors[i].get_active_streams();
                if (active_stream_per_sensor[i] != current_active_streams.size())
                {
                    std::cout<<"sensor: " << i << " active streams has changed.\n";
                    update_sensor_stream(i,current_active_streams);
                    active_stream_per_sensor[i] = current_active_streams.size();
                }
                else
                {

                }

            }
            
            usleep(1000);
            //check if the active streams vector was changed 
            //if added stream:
            // 1. add to active streams 
            // 2. 
        }

}

void ip_device::update_sensor_stream(int sensor_index,std::vector<rs2::stream_profile> updated_streams)
{
    for (size_t i = 0; i < updated_streams.size(); i++)
    {
        
        rs2::video_stream_profile vst(updated_streams[i]);
        rs2_video_stream st;
        st.fps = vst.fps();
        st.fmt = vst.format();
        st.type = vst.stream_type();
        st.width = vst.width();
        st.height = vst.height();

        std::cout << "Starting new stream" << st.type << "  " << st.fps << "\n" ;
        
        rtsp_clients[sensor_index]->addStream(st, &ipDeviceFrameCallBack);
        rtsp_clients[sensor_index]->start();
    }
}

rs2::software_device ip_device::create_ip_device(std::string ip_address)
{
    // create sw device
    rs2::software_device sw_dev = rs2::software_device();
    // create IP instance
    ip_device* ip_dev = new ip_device(ip_address, sw_dev);
    // set client destruction functioun
    ip_dev->sw_dev.set_destruction_callback([ip_dev]{delete ip_dev;});
    // register device info to sw device
    ip_dev->sw_dev.register_info(rs2_camera_info::RS2_CAMERA_INFO_SERIAL_NUMBER,"12345678");
    // return sw device 
    return sw_dev;
}


    //3. get sensors and streams
    /*
    std::vector<rs2::software_sensor> sensors;
    //rs2::software_sensor sensors[2] = {NULL};
    sensors.insert(sensors.end(),sw_dev.add_sensor("bla"));
    //sensors[0] = rs2_software_device_add_sensor(sw_dev,("Depth (Remote)"));
    
    rs2_video_stream st = {RS2_STREAM_DEPTH,0,0,640,480,30,2,RS2_FORMAT_Z16,
            { 640, 480,(float)640 / 2, (float)480 / 2,(float)640 / 2, (float)480 / 2,RS2_DISTORTION_BROWN_CONRADY ,{ 0,0,0,0,0 } }};
    sensors[0].add_video_stream(st);


    //4. for each sensor:
    //  4.1. create and add sw sensor to sw device
    //  4.2. add streams per sensor
    //5. register tear_down function to sw_device dtor
    */