#include "ip_device.hh"
#include <librealsense2/rs.hpp>

void ip_device_deleter(void* p) 
{

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

    #ifdef COMPRESSION
	idecomress = decompressFrameFactory::create(zipMethod::gzip);
    #endif

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

        auto streams = query_streams(sensor_id);

        std::cout << "\t@@@ got " << streams.size() << " streams per sensor " << sensor_id << std::endl;

        for (int stream_index = 0; stream_index < streams.size(); stream_index++)
        {
            // just for readable code
            rs2_video_stream st = streams[stream_index];

            std::cout << "\t@@@ add stream uid: "  << st.uid <<" at sensor: " << sensor_id << std::endl;
            
            //nhershko: check why profile with type 0
            streams_collection[st.uid] = std::make_shared<rs_rtp_stream>(st,sensors[sensor_id]->add_video_stream(st,stream_index==0));
            
            std::cout << "\t@@@ added stream uid: "  << st.uid <<" of type: " << streams_collection[st.uid].get()->stream_type() << std::endl;
            streams_uid_per_sensor[sensor_id].push_front(st.uid);
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
        
        std::cout <<"\t@@@ removing all streams for sensor index: " << sensor_index <<std::endl;
        rtsp_clients[sensor_index]->stop();
        rtsp_clients[sensor_index]->close();

        for (int uid : streams_uid_per_sensor[sensor_index]) 
        {
            if (streams_collection[uid].get()->is_enabled==false)
                continue;
            std::cout << "\t@@@ stopping stream uid: " << uid <<std::endl;
            streams_collection[uid].get()->is_enabled=false;
            inject_frames_thread[uid].join();
        }
        return;
    }


    std::cout <<"\t@@@ got new " <<updated_streams.size() << " streams to enable.\n";
    for (size_t i = 0; i < updated_streams.size(); i++)
    {
        rs2::video_stream_profile vst(updated_streams[i]);

        if(streams_collection.find(vst.unique_id()) == streams_collection.end())
        {
            std::cout<<"\n\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
            std::cout<< "\t@@@ stream with uid: " << vst.unique_id() << " was not found! adding new stream" << std::endl;
            std::cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n\n";
        }

        //streams_collection[vst.unique_id()].get()->is_enabled=true;
        //vst.unique_id
        /*
        rs2_video_stream st;
        st.fps = vst.fps();
        st.fmt = vst.format();
        st.type = vst.stream_type();
        st.width = vst.width();
        st.height = vst.height();
        st.uid = vst.unique_id();
        */

        std::cout<< "\t@@@ starting new stream with uid: " << vst.unique_id() << " of type: " << vst.stream_type()  << std::endl;

        //temporary nhershko workaround for start after stop
        if(!((camOERTSPClient*)rtsp_clients[sensor_index])->isConnected())
        {
            recover_rtsp_client(sensor_index);
            //st = rtsp_clients[sensor_index]->queryStreams()[0];
        }

        rtp_callbacks[vst.unique_id()] = 
            new rs_rtp_callback(streams_collection[vst.unique_id()]);
        
        rtsp_clients[sensor_index]->addStream(streams_collection[vst.unique_id()].get()->m_rs_stream ,rtp_callbacks[vst.unique_id()]);
        
        std::cout << "\t@@@ initiate new thread for stream: " << vst.unique_id() << "\n";    
        inject_frames_thread[vst.unique_id()] = std::thread(&ip_device::inject_frames_loop,this,streams_collection[vst.unique_id()]);
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

void ip_device::inject_frames_loop(std::shared_ptr<rs_rtp_stream> rtp_stream)
{

    rtp_stream.get()->is_enabled=true;
    int uid = rtp_stream.get()->m_rs_stream.uid;
    rs2_stream type = rtp_stream.get()->m_rs_stream.type;
    
    while (rtp_stream.get()->is_enabled==true)
    {
        if (rtp_stream.get()->queue_size()==0) 
        {
	        
		} 
        else 
        {				
            //std:: cout <<"\t@@@ get frame of stream uid "<< uid << "type: " << type << std::endl;
            Tmp_Frame* frame = rtp_stream.get()->extract_frame();
            //std:: cout <<"\t@@@ got the frame"<<std::endl;

#ifdef COMPRESSION			
            //check if depth image
			if (type==rs2_stream::RS2_STREAM_DEPTH) 
            {
				// depth
                //std:: cout <<"\t@@@ before com the frame"<<std::endl;
				idecomress->decompressDepthFrame((unsigned char *)frame->m_buffer, frame->m_size, (unsigned char*)(rtp_stream.get()->frame_data_buff.pixels));
                //std:: cout <<"\t@@@ after com the frame"<<std::endl;
			} else if(type==rs2_stream::RS2_STREAM_COLOR) {
				// other -> color
#endif
                //std:: cout <<"\t@@@ color frame"<<std::endl;
				memcpy(rtp_stream.get()->frame_data_buff.pixels, frame->m_buffer, frame->m_size);
#ifdef COMPRESSION
			}
            else 
            {
                std::cerr <<" BAD type"<<std::endl;
                exit(-1);
            }
#endif
			rtp_stream.get()->frame_data_buff.timestamp = frame->m_timestamp.tv_sec;
			rtp_stream.get()->frame_data_buff.frame_number++;
            
            sensors[type-1]->on_video_frame(rtp_stream.get()->frame_data_buff);
            //std::cout<<"added frame from type " << uid << "to sensor ptr " << sensors[rtp_stream.get()->stream_type()-1] << " \n";
        }
	}

    rtp_stream.get()->reset_queue();
    std::cout<<"polling data at stream index " << rtp_stream.get()->m_rs_stream.uid <<" is done\n";

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