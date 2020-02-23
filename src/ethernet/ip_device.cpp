#include "ip_device.hh"

#include <ipDevice_Common/statistic.h>
#include <list>

#include <chrono>
#include <thread>

 char* sensors_str[] = {"depth","color"};

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

    remote_sensors[sensor_index]->rtsp_client = camOERTSPClient::getRtspClient(std::string("rtsp://" + ip_address + ":8554/"+ sensors_str[sensor_index]).c_str(),"ethernet_device");
	
    ((camOERTSPClient*)remote_sensors[sensor_index]->rtsp_client)->initFunc(&rs_rtp_stream::get_memory_pool());
    ((camOERTSPClient*)remote_sensors[sensor_index]->rtsp_client)->queryStreams();
    
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

    if (remote_sensors[sensor_id]->rtsp_client==NULL)
			return streams;
    
    //workaround 
    if(remote_sensors[sensor_id]->rtsp_client==nullptr)
            recover_rtsp_client(sensor_id);
    
    streams = remote_sensors[sensor_id]->rtsp_client->queryStreams();

    std::cout <<"\t@@@ got " << streams.size() <<std::endl;
    
    return streams;
}

bool ip_device::init_device_data()
{
    std::string url,sensor_name="";
    for (int sensor_id = 0; sensor_id < NUM_OF_SENSORS; sensor_id++)
    {

        url = std::string("rtsp://" + ip_address + ":8554/"+sensors_str[sensor_id]);
        sensor_name = sensors_str[sensor_id] + std::string(" (Remote)");

        remote_sensors[sensor_id] = new ip_sensor();

        remote_sensors[sensor_id]->rtsp_client = camOERTSPClient::getRtspClient(url.c_str(),"ip_device_device");
	    ((camOERTSPClient*)remote_sensors[sensor_id]->rtsp_client)->initFunc(&rs_rtp_stream::get_memory_pool());

        std::cout << "\t@@@ adding new sensor of type id: " << sensor_id << std::endl;

        rs2::software_sensor tmp_sensor = sw_dev.add_sensor(sensor_name);

        remote_sensors[sensor_id]->sw_sensor = new rs2::software_sensor(tmp_sensor);
        
        //TODO: DEMO_WW8 get option per sensor
        //auto sensor_option = query_option(sensor_id);
        /*
        for opt in sensor option
            sensor[ind].add_option(opt,opt_parameters)
        */
        //hard_coded 
        if (sensor_id==0)
        {
            remote_sensors[sensor_id]->sw_sensor->add_read_only_option(RS2_OPTION_DEPTH_UNITS, 0.001);
            remote_sensors[sensor_id]->sw_sensor->add_read_only_option(RS2_OPTION_STEREO_BASELINE,0);

            //options.push_front({rs2_option::RS2_OPTION_EXPOSURE,{0,33,14,1}});
            //options.push_back({rs2_option::RS2_OPTION_EXPOSURE,{0,33,14,1},true});
            //options.push_back({rs2_option::RS2_OPTION_EXPOSURE,{0,33,14,1},true});
            //options.push_back({rs2_option::RS2_OPTION_GAIN,{0,100,50,1},true});
            
            remote_sensors[sensor_id]->sw_sensor->add_option(rs2_option::RS2_OPTION_EXPOSURE,{0,33,14,1});
            remote_sensors[sensor_id]->sw_sensor->add_option(rs2_option::RS2_OPTION_GAIN,{0,250,70,1});

            //for (my_control opt : options) 
            //        sensors[sensor_id]->add_option(opt.option,opt.range);
        }

        std::cout << "\t@@@ sensor options number is " << remote_sensors[sensor_id]->sw_sensor->get_supported_options().size() << " streams per sensor " << sensor_id << std::endl;

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
            streams_collection[stream_key] = std::make_shared<rs_rtp_stream>(st,remote_sensors[sensor_id]->sw_sensor->add_video_stream(st,stream_index==0));
            memPool = &rs_rtp_stream::get_memory_pool();
            std::cout << "\t@@@ added stream [uid:hash] ["  << st.uid<<":"<< stream_key <<"] of type: " << streams_collection[stream_key].get()->stream_type() << std::endl;
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
            bool enabled;
            //for eahc sensor check the size of active streams
            for (size_t i = 0; i < sensors.size(); i++)
            {
                //poll start/stop events
                auto current_active_streams = sensors[i].get_active_streams();
                if(current_active_streams.size()>0)
                    enabled=true;
                else
                    enabled=false;
                
                if (remote_sensors[i]->is_enabled != enabled)
                {
                    std::cout<<"\t@@@ sensor: " << i << " active streams has changed.\n\n\n";
                    update_sensor_state(i,current_active_streams);
                    remote_sensors[i]->is_enabled = enabled;
                }
                else
                {
                    //std::cout<<"sensor: " << i << " have not changed.\n";
                }
                //poll control change events
                auto sensor_supported_option = sensors[i].get_supported_options();
                for (rs2_option opt : sensor_supported_option)
                    if(remote_sensors[i]->sensors_option[opt] != (float)sensors[i].get_option(opt))
                    {
                        //TODO: get from map once to reduce logarithmic complexity 
                        remote_sensors[i]->sensors_option[opt] = (float)sensors[i].get_option(opt);
                        std::cout<<"option: " << opt << " has changed to:  " << remote_sensors[i]->sensors_option[opt] << std::endl;
                        //TODO: DEMO_WW8 rtspclient - set (opt,val)
                    }
                    else
                    {
                        //std::cout<<"option: " << opt << " was not changed! " <<std::endl;
                    }
            }
			std::this_thread::sleep_for(std::chrono::microseconds(POLLING_SW_DEVICE_STATE_INTERVAL));
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
    retVal.index = sp.stream_index();
    
    return retVal;
}

void ip_device::update_sensor_state(int sensor_index,std::vector<rs2::stream_profile> updated_streams)
{
    //check if need to close all
    if(updated_streams.size()==0)
    {
        
        std::cout <<"\t@@@ removing all streams for sensor index: " << sensor_index <<std::endl;
        remote_sensors[sensor_index]->rtsp_client->stop();
        remote_sensors[sensor_index]->rtsp_client->close();
        remote_sensors[sensor_index]->rtsp_client=nullptr;

        //for (long long int key : streams_uid_per_sensor[sensor_index]) 
        for (long long int key : remote_sensors[sensor_index]->active_streams_keys) 
        {
            if (streams_collection[key].get()->is_enabled==false)
                continue;
            std::cout << "\t@@@ stopping stream [uid:key] " << streams_collection[key].get()->m_rs_stream.uid <<":"<<key<< "]" <<std::endl;
            streams_collection[key].get()->is_enabled=false;
            if (inject_frames_thread[key].joinable()) inject_frames_thread[key].join();
        }
        remote_sensors[sensor_index]->active_streams_keys.clear();
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
        if(remote_sensors[sensor_index]->rtsp_client==nullptr)
        {
            recover_rtsp_client(sensor_index);
        }

        rtp_callbacks[requested_stream_key] = new rs_rtp_callback(streams_collection[requested_stream_key]);
        
        remote_sensors[sensor_index]->rtsp_client->addStream(streams_collection[requested_stream_key].get()->m_rs_stream ,rtp_callbacks[requested_stream_key]);
        std::cout << "\t@@@ initiate new thread for stream: " << vst.unique_id() << "\n";    
        inject_frames_thread[requested_stream_key] = std::thread(&ip_device::inject_frames_loop,this,streams_collection[requested_stream_key]);
        //streams_uid_per_sensor[sensor_index].push_front(requested_stream_key);
        remote_sensors[sensor_index]->active_streams_keys.push_front(requested_stream_key);
    }

    remote_sensors[sensor_index]->rtsp_client->start();
    std::cout << "stream started for sensor index: " << sensor_index << "  \n";
}

rs2::software_device ip_device::create_ip_device(const char* ip_address)
{
	std::string addr(ip_address);

    // create sw device
    rs2::software_device sw_dev = rs2::software_device();
    // create IP instance
    ip_device* ip_dev = new ip_device(addr, sw_dev);
    // set client destruction functioun
    ip_dev->sw_dev.set_destruction_callback([ip_dev]{delete ip_dev;});
    // register device info to sw device
    device_data data = ip_dev->remote_sensors[0]->rtsp_client->getDeviceData();
    ip_dev->sw_dev.update_info(RS2_CAMERA_INFO_NAME, data.name + "\n IP Device");
    ip_dev->sw_dev.register_info(rs2_camera_info::RS2_CAMERA_INFO_IP_ADDRESS, addr);
    ip_dev->sw_dev.register_info(rs2_camera_info::RS2_CAMERA_INFO_SERIAL_NUMBER, data.serial_num);
    ip_dev->sw_dev.register_info(rs2_camera_info::RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR, data.usb_type);
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

    rtp_stream.get()->frame_data_buff.frame_number = 0;
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
            rtp_stream.get()->frame_data_buff.pixels = frame->m_buffer;
            //rtp_stream.get()->frame_data_buff.timestamp = (frame->m_timestamp.tv_sec*1000)+(frame->m_timestamp.tv_usec/1000); // convert to milliseconds
	    rtp_stream.get()->frame_data_buff.timestamp = frame->m_metadata->timestamp;
            rtp_stream.get()->frame_data_buff.frame_number++;
            // TODO Michal: change this to HW time once we pass the metadata
            //rtp_stream.get()->frame_data_buff.domain = RS2_TIMESTAMP_DOMAIN_SYSTEM_TIME;
            rtp_stream.get()->frame_data_buff.domain = frame->m_metadata->timestamp_domain;

            remote_sensors[sensor_id]->sw_sensor->set_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP,rtp_stream.get()->frame_data_buff.timestamp);
            remote_sensors[sensor_id]->sw_sensor->set_metadata(RS2_FRAME_METADATA_ACTUAL_FPS,rtp_stream.get()->m_rs_stream.fps);
            remote_sensors[sensor_id]->sw_sensor->set_metadata(RS2_FRAME_METADATA_FRAME_COUNTER,rtp_stream.get()->frame_data_buff.frame_number);
            remote_sensors[sensor_id]->sw_sensor->set_metadata(RS2_FRAME_METADATA_FRAME_EMITTER_MODE,1);

            //nhershko todo: set it at actuqal arrivial time
            remote_sensors[sensor_id]->sw_sensor->set_metadata(RS2_FRAME_METADATA_TIME_OF_ARRIVAL,
                std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count());
#ifdef STATISTICS
            stream_statistic * st  = statistic::getStatisticStreams()[ rtp_stream.get()->stream_type()];
            std::chrono::system_clock::time_point clockEnd = std::chrono::system_clock::now();
            st->processingTime = clockEnd - st->clockBeginVec.front();
            st->clockBeginVec.pop();
            st->avgProcessingTime +=st->processingTime.count();
            printf("STATISTICS: streamType: %d, processing time: %0.2fm, average: %0.2fm, counter: %d\n",type,st->processingTime*1000, (st->avgProcessingTime*1000)/st->frameCounter,st->frameCounter);
#endif
            remote_sensors[sensor_id]->sw_sensor->on_video_frame(rtp_stream.get()->frame_data_buff);
            //std::cout<<"\t@@@ added frame from type " << type << " with uid " << rtp_stream.get()->m_rs_stream.uid << " time stamp: " << (double)rtp_stream.get()->frame_data_buff.frame_number <<" profile: " << rtp_stream.get()->frame_data_buff.profile->profile->get_stream_type() << "   \n";
        }
	}

    rtp_stream.get()->reset_queue();
    std::cout<<"polling data at stream index " << rtp_stream.get()->m_rs_stream.uid <<" is done\n";

}
