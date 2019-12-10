#include "ethernet-device.h"

int frame_number = 0;
std::chrono::high_resolution_clock::time_point last;

std::vector<uint8_t> pixels;
std::thread t;

const int W = 640;
const int H = 480;
const int BPP = 2;

std::mutex mtx;
rs2_stream_profile* depth_stream;
volatile bool is_active = false;

char stop_flag = 0;
void sig_handler(int signo)
{
	if (signo == SIGINT) {
		printf("received SIGINT\n");
		stop_flag = 1;
	}
}

rs2::ethernet_device::ethernet_device()
{
	dev = rs2_create_software_device(NULL);
	
}

rs2::ethernet_device::ethernet_device(std::string url) : ethernet_device()
{
	ip_address = url;
}

rs2::ethernet_device::~ethernet_device()
{
	rs2_delete_device(dev);
}

std::vector<rs2::sensor> rs2::ethernet_device::ethernet_device::query_sensors() const
{
	std::cout << "Mock ethernet device querry";
	
	/*
	TODO: use rtsp client
	std::vector<stream_profile> rtsp_profiles = rtsp_client.query_streams();
	std::vector<rs2_video_stream> ethernet_devie_streams = rtsp_stream_to_rs_video_stream(rtsp_profiles);
	*/

	std::vector<rs2::sensor> sensors;
	//TODO: get device sensors via network
	rs2::sensor mock_sensor;
	sensors.push_back(mock_sensor);
	return sensors;
}


rs2_intrinsics rs2::ethernet_device::get_intrinsics()
{
	rs2_intrinsics intrinsics = { W, H,
		(float)W / 2, (float)H / 2,
		(float)W / 2, (float)H / 2,
		RS2_DISTORTION_BROWN_CONRADY ,{ 0,0,0,0,0 } };

	return intrinsics;
}

rs2_software_video_frame& rs2::ethernet_device::get_frame() {
	return last_frame[0];
}

rs2_device* rs2::ethernet_device::get_device() {
	return dev;
}

void rs2::ethernet_device::add_frame_to_queue(int type, Frame* raw_frame)
{
	if(QUEUE_MAX_SIZE>this->frame_queues[type].size())
	{
		std::mutex m_mtx;
		const std::lock_guard<std::mutex> lock(m_mtx);
		//TODO: map between type and stream queue
		this->frame_queues[type].push(raw_frame);
		frame_number++;
	}
	else
	{
		std::cout<< "queue is full. dropping frame of type " << type << std::endl;
	}
}


rs2_video_stream rs2::ethernet_device::rtsp_stream_to_rs_video_stream(camOE_stream_profile profile)
{
	static int streams_count = 0;

	rs2_stream type;
	rs2_format format;
	rs2_intrinsics intrinsics;
	int BPP = 2;

	if (profile.stream_type()==0)
	{
		intrinsics =  get_intrinsics();
		type = RS2_STREAM_DEPTH;
		format = RS2_FORMAT_Z16;
	}
	else
	{
		intrinsics =  { W, H ,0, 0, 0, 0, RS2_DISTORTION_BROWN_CONRADY, { 0,0,0,0,0 } };
		type = RS2_STREAM_COLOR;
		format = RS2_FORMAT_YUYV;
	}
	
	rs2_video_stream st = {type,0,streams_count,profile.width(),profile.hight(),profile.fps(),BPP,format,intrinsics};
	streams_count++;

	return st;
}

void rs2::ethernet_device::inject_frames_to_sw_device()
{
	//todo: replace with input from rtsp client
	std::queue<camOE_stream_profile> streams;
	streams.push(camOE_stream_profile(stream_type_id::STREAM_DEPTH,{640,480},30));
	streams.push(camOE_stream_profile(stream_type_id::STREAM_COLOR,{640,480},30));

	inject_threads = new std::thread[streams.size()];

	for (size_t i = 0; i <= streams.size(); i++)
	{
		rs2_video_stream st = rtsp_stream_to_rs_video_stream(streams.front());
		
		if (streams.front().stream_type()==0)
			sensors[i] = rs2_software_device_add_sensor(dev, "Depth (Remote)", NULL);
		else 
			sensors[i] = rs2_software_device_add_sensor(dev, "Color (Remote)", NULL);

		profiles[i] = rs2_software_sensor_add_video_stream(sensors[i], st, NULL);

		last_frame[i].bpp = st.bpp;
		last_frame[i].profile = profiles[i];
		last_frame[i].stride = st.bpp * st.width;
		pixels_buff[i].resize(last_frame[i].stride * st.height, 0);
		last_frame[i].pixels = pixels_buff[i].data();
		last_frame[i].deleter = &ethernet_device_deleter;
		

		inject_threads[i] = std::thread(&rs2::ethernet_device::pull_from_queue,this,streams.front().stream_type());

		streams.pop();
	}
}

void rs2::ethernet_device::pull_from_queue(int stream_index)
{
	while(is_active)
	{
		if (frame_queues[stream_index].empty()) {
			/*no data at quque*/;
		} else {				
			const std::lock_guard<std::mutex> lock(mtx);
			Frame* frame = frame_queues[stream_index].front();
			frame_queues[stream_index].pop();
			memcpy(last_frame[stream_index].pixels, frame->m_buffer, frame->m_size);
			// delete frame;
			last_frame[stream_index].timestamp = frame->m_timestamp.tv_sec;
			last_frame[stream_index].frame_number++;
			rs2_software_sensor_on_video_frame(sensors[stream_index], last_frame[stream_index], NULL);
		}
	}
	std::cout<<"pulling data at stream index " << stream_index <<" is done\n";
}

void rs2::ethernet_device::incomming_server_frames_handler()
{
	Environment env(stop_flag);

	// default value
	int  timeout = 10;
	int rtptransport = RTSPConnection::RTPUDPUNICAST;
	int  logLevel = 255;
	std::string output;
	std::string url = "rtsp://" + ip_address + "/unicast"; //"rtsp://10.12.144.74:8554/unicast";
	RS_RTSPFrameCallback rs_cb(this, output);
	rs_cb.id=0;
	//RS_RTSPFrameCallback rs_cb_color(this, output);
	//rs_cb_color.id=1;

	RTSPConnection rtsp_client = RTSPConnection(env, &rs_cb, url.c_str(), timeout, rtptransport);
	//RTSPConnection rtsp_client_color = RTSPConnection(env, &rs_cb_color, (url+"2").c_str(), timeout, rtptransport);

	signal(SIGINT, sig_handler);
	std::cout << "Start mainloop" << std::endl;
	env.mainloop();

}

void rs2::ethernet_device::start() {
	if (is_active)
		return;
	is_active = true;

	t = std::thread(&rs2::ethernet_device::incomming_server_frames_handler,this);
	inject_frames_to_sw_device();
}
void rs2::ethernet_device::stop() {
	if (!is_active)
		return;
	is_active = false;
	t.join();
}
