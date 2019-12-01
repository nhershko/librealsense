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
	return depth_frame;
}

rs2_device* rs2::ethernet_device::get_device() {
	return dev;
}

void rs2::ethernet_device::add_frame_to_queue(int type, Frame* raw_frame)
{
	if(QUEUE_MAX_SIZE>this->depth_frames.size())
	{
		std::mutex m_mtx;
		const std::lock_guard<std::mutex> lock(m_mtx);
		if(type==0)
			this->depth_frames.push(raw_frame);
		else 
		{
			this->color_frames.push(raw_frame);
		}
		frame_number++;
	}
	else
	{
		std::cout<< "queue is full. dropping frame" << std::endl;
	}
}

void rs2::ethernet_device::inject_frames_to_sw_device()
{
	rs2_intrinsics depth_intrinsics = get_intrinsics();
	depth_sensor = rs2_software_device_add_sensor(dev, "Depth (Remote)", NULL);
	
	rs2_video_stream st = { RS2_STREAM_DEPTH, 0, 1, W,
							H, 30, BPP,
							RS2_FORMAT_Z16, depth_intrinsics };
	depth_stream = rs2_software_sensor_add_video_stream(depth_sensor, st, NULL);
	
	depth_frame.bpp = BPP;
	depth_frame.profile = depth_stream;
	depth_frame.stride = BPP * W;
	pixels.resize(depth_frame.stride * H, 0);
	depth_frame.pixels = pixels.data();
	depth_frame.deleter = &ethernet_device_deleter;
  
	//color

	rs2_intrinsics color_intrinsics = { W,H,
            0, 0,
            0, 0,
            RS2_DISTORTION_BROWN_CONRADY ,{ 0,0,0,0,0 } };

	int color_bpp = 3;

	rs2_video_stream st2 = { RS2_STREAM_COLOR, 0, 1, W,
							H, 30, BPP,
							RS2_FORMAT_YUYV, color_intrinsics };

	//software_sensor color_sensor = this->add_sensor("Color (Remote)");
	//color_sensor.add_video_stream(st2);
	
	color_sensor = rs2_software_device_add_sensor(dev, "Color (Remote)", NULL);
	color_stream = rs2_software_sensor_add_video_stream(color_sensor, st2, NULL);
	
	color_frame.bpp = BPP;
	color_frame.profile = color_stream;
	color_frame.stride = BPP * W;
	pixels.resize(color_frame.stride * H, 0);
	color_frame.pixels = pixels.data();
	color_frame.deleter = &ethernet_device_deleter;

	while (is_active)
	{
			if (depth_frames.empty()) {
				//do nothing 
			} else {				
				const std::lock_guard<std::mutex> lock(mtx);
				Frame* frame = depth_frames.front();
				depth_frames.pop();
				memcpy(depth_frame.pixels, frame->m_buffer, frame->m_size);
				// delete frame;
				depth_frame.timestamp = frame->m_timestamp.tv_sec;
				depth_frame.frame_number++;
				rs2_software_sensor_on_video_frame(depth_sensor, depth_frame, NULL);
			}

			if (color_frames.empty()) {
				;//do nothing 
			} else {				
				const std::lock_guard<std::mutex> lock(mtx2);
				Frame* frame = color_frames.front();
				color_frames.pop();
				memcpy(color_frame.pixels, frame->m_buffer, frame->m_size);
				// delete frame;
				color_frame.timestamp = frame->m_timestamp.tv_sec;
				color_frame.frame_number++;
				//color_sensor.on_video_frame(color_frame);
				rs2_software_sensor_on_video_frame(color_sensor, color_frame, NULL);
			}
	}
		
}

void rs2::ethernet_device::incomming_server_frames_handler()
{
	char stop = 0;
	Environment env(stop);

	// default value
	int  timeout = 10;
	int rtptransport = RTSPConnection::RTPUDPUNICAST;
	int  logLevel = 255;
	std::string output;
	std::string url = "rtsp://" + ip_address + "/unicast"; //"rtsp://10.12.144.74:8554/unicast";
	RS_RTSPFrameCallback rs_cb(this, output);
	rs_cb.id=0;
	RS_RTSPFrameCallback rs_cb_color(this, output);
	rs_cb_color.id=1;

	RTSPConnection rtsp_client = RTSPConnection(env, &rs_cb, url.c_str(), timeout, rtptransport);
	RTSPConnection rtsp_client_color = RTSPConnection(env, &rs_cb_color, (url+"2").c_str(), timeout, rtptransport);

	signal(SIGINT, sig_handler);
	std::cout << "Start mainloop" << std::endl;
	env.mainloop();

}

void rs2::ethernet_device::start() {
	if (is_active)
		return;
	is_active = true;

	t = std::thread(&rs2::ethernet_device::incomming_server_frames_handler,this);
	t2 = std::thread(&rs2::ethernet_device::inject_frames_to_sw_device,this);
}
void rs2::ethernet_device::stop() {
	if (!is_active)
		return;
	is_active = false;
	t.join();
	t2.join();
}
