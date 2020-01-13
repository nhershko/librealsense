#include "ip_device.hh"
#include <librealsense2/rs.hpp>

void ip_device_deleter(void* p) 
{

}

//WA for stop
void recover_rtsp_client(int sensor_index, IcamOERtsp* rtsp_clients, std::string url)
{
    std::cout <<"do second init for rtsp client\n";
    
    if(sensor_index==0)
        rtsp_clients = camOERTSPClient::getRtspClient(std::string("rtsp://" + url + ":8554/depth").c_str(),"ethernet_device");
    else 
        rtsp_clients = camOERTSPClient::getRtspClient(std::string("rtsp://" + url + ":8554/color").c_str(),"ethernet_device");
    
    ((camOERTSPClient*)rtsp_clients)->initFunc();
    rtsp_clients->queryStreams();
    std::cout <<"done\n";
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

    #ifdef COMPRESSION
	idecomress = decompressFrameFactory::create(zipMethod::gzip);
    #endif

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
        //temporary nhershko workaround for start after stop
        if(!((camOERTSPClient*)rtsp_clients[i])->isConnected())
        {
            recover_rtsp_client(i,rtsp_clients[i],ip_address);
        }
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
			
        rs2::software_sensor tmp_sensor = sw_dev.add_sensor(sensor_name);
        sensors[st.type-1] = new rs2::software_sensor(tmp_sensor);

        std::cout << "create profile uid: " << st.uid << std::endl;
        profiles[st.type-1] = sensors[st.type-1]->add_video_stream(st,i==0);
        std::cout << "create profile at: " << st.type-1 << std::endl;

		last_frame[st.type-1].bpp = st.bpp;
		last_frame[st.type-1].profile = profiles[st.type-1];
		last_frame[st.type-1].stride = st.bpp * st.width;
		pixels_buff[st.type-1].resize(last_frame[st.type-1].stride * st.height, 0);
		last_frame[st.type-1].pixels = pixels_buff[st.type-1].data();
		last_frame[st.type-1].deleter = ip_device_deleter;
		
        std::cout << "initiate frame buffer for stream ID: " << st.type-1 <<std::endl;
	}
    
    return true;
}

void ip_device::polling_state_loop()
{
        while(this->is_device_alive)
        {
            //TODO: consider using sensor id as vector id (indexer)
            std::vector<rs2::sensor> sensors = this->sw_dev.query_sensors();
            //for eahc sensor check the size of active streams
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
                    //std::cout<<"sensor: " << i << " have not changed.\n";
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
    //check if need to close all
    if(updated_streams.size()==0)
    {
        rtsp_clients[sensor_index]->stop();
        rtsp_clients[sensor_index]->close();
        injected_thread_active[sensor_index]=false;
        inject_frames_thread[sensor_index].join();

        return;
    }

    for (size_t i = 0; i < updated_streams.size(); i++)
    {
        rs2::video_stream_profile vst(updated_streams[i]);
        rs2_video_stream st;
        st.fps = vst.fps();
        st.fmt = vst.format();
        st.type = vst.stream_type();
        st.width = vst.width();
        st.height = vst.height();

        //temporary nhershko workaround for start after stop
        if(!((camOERTSPClient*)rtsp_clients[sensor_index])->isConnected())
        {
            recover_rtsp_client(sensor_index,rtsp_clients[sensor_index],ip_address);
        }

        rtp_callbacks[sensor_index] = 
            new rtp_callback(st.uid,&frame_queues[sensor_index],&queue_locks[sensor_index]);
        st.uid=sensor_index;
        rtsp_clients[sensor_index]->addStream(st,rtp_callbacks[sensor_index]);
    }
    rtsp_clients[sensor_index]->start();
    std::cout << "stream started for sensor index: " << sensor_index << "  \n" ;

    inject_frames_thread[sensor_index] = std::thread(&ip_device::inject_frames_loop,this,sensor_index);
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

void ip_device::inject_frames_loop(int stream_index)
{
    injected_thread_active[stream_index]=true;
    while (injected_thread_active[stream_index])
    {
        if (this->frame_queues[stream_index].empty()) 
        {
	        //std::cout<<"queue is empty\n";
		} 
        else 
        {				
            queue_locks[stream_index].lock();
            Tmp_Frame* frame = frame_queues[stream_index].front();
			frame_queues[stream_index].pop();
            queue_locks[stream_index].unlock();

#ifdef COMPRESSION			
			if (stream_index==0) {
				// depth
				idecomress->decompressDepthFrame((unsigned char *)frame->m_buffer, frame->m_size, (unsigned char*)(last_frame[0].pixels));
			} else {
				// other -> color
#endif
				memcpy(last_frame[stream_index].pixels, frame->m_buffer, frame->m_size);
#ifdef COMPRESSION
			}
#endif
			last_frame[stream_index].timestamp = frame->m_timestamp.tv_sec;
			last_frame[stream_index].frame_number++;
            sensors[stream_index]->on_video_frame(last_frame[stream_index]);
            //std::cout<<"added frame from type " << stream_index << "to sensor ptr " << sensors[stream_index] << " \n";
		}
	}
    clean_frames_queue(stream_index);
	std::cout<<"polling data at stream index " << stream_index <<" is done\n";
}

void ip_device::clean_frames_queue(int index)
{
    while(!frame_queues[index].empty())
	{
		frame_queues[index].pop();
	}
    std::cout << "done clean frames queue: " << index << std::endl;
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