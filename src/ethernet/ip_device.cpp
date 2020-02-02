#include "ip_device.hh"
#include <librealsense2/rs.hpp>

void ip_device_deleter(void* p) 
{

}

rs2_intrinsics get_hard_coded_sensor_intrinsics(rs2_video_stream stream)
{
	//todo: get intrinsics from rtsp 
    int W = stream.width;
    int H = stream.height;
	if (stream.type==RS2_STREAM_DEPTH)
		return { W, H,(float)W / 2, (float)H / 2,(float)W / 2, (float)H / 2,RS2_DISTORTION_BROWN_CONRADY ,{ 0,0,0,0,0 } };
	return { W, H ,0, 0, 0, 0, RS2_DISTORTION_BROWN_CONRADY, { 0,0,0,0,0 } };
}

//WA for stop
void ip_device::recover_rtsp_client(int sensor_index)
{
    std::cout <<"\t@@@ do second init for rtsp client\n";

    if(sensor_index==0)
    {
        rtsp_clients[0] = camOERTSPClient::getRtspClient(std::string("rtsp://" + ip_address + ":8554/depth").c_str(),"ethernet_device");
	}
    else if (sensor_index==1)
    {
        rtsp_clients[1] = camOERTSPClient::getRtspClient(std::string("rtsp://" + ip_address + ":8554/color").c_str(),"ethernet_device");
	}
    ((camOERTSPClient*)rtsp_clients[sensor_index])->initFunc();
    ((camOERTSPClient*)rtsp_clients[sensor_index])->queryStreams();
    
    std::cout <<"\t@@@ done\n";
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

    //init device data
    init_device_data();

    //poll sw device streaming state
    this->sw_device_status_check = std::thread(&ip_device::polling_state_loop,this);
}

std::vector<rs2_video_stream> ip_device::query_streams(int sensor_id)
{
    std::cout << "\n\n\nQuerry Sensors\n\n\n";
	std::vector<rs2_video_stream> streams;

    if (rtsp_clients[sensor_id]==NULL)
			return streams;
    
    //workaround 
    if(!((camOERTSPClient*)rtsp_clients[sensor_id])->isConnected())
            recover_rtsp_client(sensor_id);
    
    streams = rtsp_clients[sensor_id]->queryStreams();

    std::cout <<"\t@@@ got " << streams.size() <<std::endl;
    
    return streams;
}

int stream_id=0;
bool ip_device::init_device_data()
{
    std::string url,sensor_name;
    for (int sensor_id = 0; sensor_id < SENSORS_NUMBER; sensor_id++)
    {
        if(sensor_id==0)
        {
            url = std::string("rtsp://" + ip_address + ":8554/depth");
            sensor_name = "Depth (Remote)";
        }
        else if (sensor_id==1)
        {
            url = std::string("rtsp://" + ip_address + ":8554/color");
            sensor_name = "Color (Remote)";
        }

        rtsp_clients[sensor_id] = camOERTSPClient::getRtspClient(url.c_str(),"ip_device_device");
	    ((camOERTSPClient*)rtsp_clients[sensor_id])->initFunc();

        std::cout << "\t@@@ adding new sensor of type id: " << sensor_id << std::endl;

        rs2::software_sensor tmp_sensor = sw_dev.add_sensor(sensor_name);

        sensors[sensor_id] = new rs2::software_sensor(tmp_sensor);
        
        //hard_coded 
        if (sensor_id==0)
        {
            sensors[sensor_id]->add_read_only_option(RS2_OPTION_DEPTH_UNITS, 0.001);
            sensors[sensor_id]->add_read_only_option(RS2_OPTION_STEREO_BASELINE,0);
        }

        auto streams = query_streams(sensor_id);

        std::cout << "\t@@@ got " << streams.size() << " streams per sensor " << sensor_id << std::endl;

        for (int stream_index = 0; stream_index < streams.size(); stream_index++)
        {
            // just for readable code
            rs2_video_stream st = streams[stream_index];
            
            //todo: remove
            st.intrinsics = get_hard_coded_sensor_intrinsics(st);

            std::cout << "\t@@@ add stream uid: "  << st.uid <<" at sensor: " << sensor_id << std::endl;
            
            //nhershko: check why profile with type 0
            long long int stream_key = camOERTSPClient::getStreamProfileUniqueKey(st);
            streams_collection[stream_key] = std::make_shared<rs_rtp_stream>(st,sensors[sensor_id]->add_video_stream(st,stream_index==0));
            
            std::cout << "\t@@@ added stream [uid:hash] ["  << st.uid<<":"<< stream_key <<"] of type: " << streams_collection[stream_key].get()->stream_type() << std::endl;
            streams_uid_per_sensor[sensor_id].push_front(stream_key);
        }
        std::cout << "\t@@@ done adding streams for sensor ID: " << sensor_id <<std::endl;
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
                    std::cout<<"\t@@@ sensor: " << i << " active streams has changed.\n\n\n";
                    update_sensor_state(i,current_active_streams);
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

rs2_video_stream convert_stream_object(rs2::video_stream_profile sp)
{
    rs2_video_stream retVal;
    retVal.fmt = sp.format();
    retVal.type = sp.stream_type();
    retVal.fps = sp.fps();
    retVal.width = sp.width();
    retVal.height = sp.height();
    
    return retVal;
}

void ip_device::update_sensor_state(int sensor_index,std::vector<rs2::stream_profile> updated_streams)
{
    //check if need to close all
    if(updated_streams.size()==0)
    {
        
        std::cout <<"\t@@@ removing all streams for sensor index: " << sensor_index <<std::endl;
        rtsp_clients[sensor_index]->stop();
        rtsp_clients[sensor_index]->close();

        for (long long int key : streams_uid_per_sensor[sensor_index]) 
        {
            if (streams_collection[key].get()->is_enabled==false)
                continue;
            std::cout << "\t@@@ stopping stream [uid:key] " << streams_collection[key].get()->m_rs_stream.uid <<":"<<key<< "]" <<std::endl;
            streams_collection[key].get()->is_enabled=false;
            inject_frames_thread[key].join();
        }
        return;
    }


    std::cout <<"\t@@@ got new " <<updated_streams.size() << " streams to enable.\n";
    for (size_t i = 0; i < updated_streams.size(); i++)
    {
        rs2::video_stream_profile vst(updated_streams[i]);

        

        long long int requested_stream_key = camOERTSPClient::getStreamProfileUniqueKey(convert_stream_object(vst));

        if(streams_collection.find(requested_stream_key) == streams_collection.end())
        {
            std::cout<<"\n\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
            std::cout<< "\t@@@ stream with uid: " << vst.unique_id() << " was not found! adding new stream" << std::endl;
            std::cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n\n";
            exit(-1);
        }
        
        std::cout<< "\t@@@ starting new stream with [uid:key] [" << vst.unique_id() <<":"<< requested_stream_key << "] of type: " << vst.stream_type()  << std::endl;

        //temporary nhershko workaround for start after stop
        if(!((camOERTSPClient*)rtsp_clients[sensor_index])->isConnected())
        {
            recover_rtsp_client(sensor_index);
        }

        rtp_callbacks[requested_stream_key] = new rs_rtp_callback(streams_collection[requested_stream_key]);
        
        rtsp_clients[sensor_index]->addStream(streams_collection[requested_stream_key].get()->m_rs_stream ,rtp_callbacks[requested_stream_key]);
        
        std::cout << "\t@@@ initiate new thread for stream: " << vst.unique_id() << "\n";    
        inject_frames_thread[requested_stream_key] = std::thread(&ip_device::inject_frames_loop,this,streams_collection[requested_stream_key]);
    }

    rtsp_clients[sensor_index]->start();
    std::cout << "stream started for sensor index: " << sensor_index << "  \n";
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

int stream_type_to_sensor_id(rs2_stream type)
{
    if ( type == RS2_STREAM_INFRARED || type == RS2_STREAM_DEPTH)
        return 0;
    return 1;
}

void ip_device::inject_frames_loop(std::shared_ptr<rs_rtp_stream> rtp_stream)
{
    rtp_stream.get()->is_enabled=true;
    int uid = rtp_stream.get()->m_rs_stream.uid;
    rs2_stream type = rtp_stream.get()->m_rs_stream.type;
    int sensor_id = stream_type_to_sensor_id(type);
    
    while (rtp_stream.get()->is_enabled==true)
    {
        if (rtp_stream.get()->queue_size()==0) 
        {
	        
		} 
        else 
        {				
            Raw_Frame* frame = rtp_stream.get()->extract_frame();
			memcpy(rtp_stream.get()->frame_data_buff.pixels, frame->m_buffer, frame->m_size);
			rtp_stream.get()->frame_data_buff.timestamp = frame->m_timestamp.tv_usec;
			rtp_stream.get()->frame_data_buff.frame_number++;
            
            
            sensors[sensor_id]->set_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP,rtp_stream.get()->frame_data_buff.timestamp);
            sensors[sensor_id]->set_metadata(RS2_FRAME_METADATA_ACTUAL_FPS,rtp_stream.get()->m_rs_stream.fps);
            sensors[sensor_id]->set_metadata(RS2_FRAME_METADATA_FRAME_COUNTER,rtp_stream.get()->frame_data_buff.frame_number);
            sensors[sensor_id]->set_metadata(RS2_FRAME_METADATA_FRAME_EMITTER_MODE,1);

            //nhershko todo: set it at actuqal arrivial time
            sensors[sensor_id]->set_metadata(RS2_FRAME_METADATA_TIME_OF_ARRIVAL,
                std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count());

            sensors[sensor_id]->on_video_frame(rtp_stream.get()->frame_data_buff);
            //std::cout<<"\t@@@ added frame from type " << type << " with uid " << rtp_stream.get()->m_rs_stream.uid << " time stamp: " << (double)rtp_stream.get()->frame_data_buff.frame_number <<" profile: " << rtp_stream.get()->frame_data_buff.profile->profile->get_stream_type() << "   \n";
        }
	}

    rtp_stream.get()->reset_queue();
    std::cout<<"polling data at stream index " << rtp_stream.get()->m_rs_stream.uid <<" is done\n";

}
