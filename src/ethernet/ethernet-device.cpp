#include "ethernet-device.h"

int frame_number = 0;
std::chrono::high_resolution_clock::time_point last;

//software_sensor* remote_device_sensors;

std::vector<uint8_t> pixels;
std::thread t;

const int W = 640;
const int H = 480;
const int BPP = 2;

HANDLE hPipe;
std::mutex mtx;
rs2_stream_profile* depth_stream;
rs2_software_video_frame depth_frame;
rs2_sensor* depth_sensor;
volatile bool is_active = false;

void bla()
{

}

HANDLE create_named_pipe(std::string stream_name) {
	HANDLE hPipe;

	hPipe = CreateNamedPipeA(
		stream_name.c_str(),
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
		1,
		1024 * 1024 * 32,
		1024 * 1024 * 32,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);

	return hPipe;
}

rs2::ethernet_device::~ethernet_device()
{
	CloseHandle(hPipe);
	rs2_delete_device(dev);
}

static int counter = 0;

rs2::ethernet_device::ethernet_device()
{
	depth_frame.bpp = BPP;
	depth_frame.stride = BPP * W;
	pixels.resize(depth_frame.stride * H, 0);
	depth_frame.pixels = pixels.data();
	dev = rs2_create_software_device(NULL);
	depth_frame.deleter = &ethernet_device_deleter;
	create_sensors();
}

void rs2::ethernet_device::start() {
	if (is_active)
		return;
	is_active = true;
	hPipe = create_named_pipe(pipe_name);
	t = std::thread(&rs2::ethernet_device::thread_main, this);
}
void rs2::ethernet_device::stop() {
	if (!is_active)
		return;
	is_active = false;
	t.join();
	CloseHandle(hPipe);
}

std::vector<rs2::sensor> rs2::ethernet_device::ethernet_device::query_sensors()
{
	std::cout << "Mock ethernet device querry";
	std::vector<rs2::sensor> sensors;
	//TODO: get device sensors via network
	rs2::sensor mock_sensor;
	sensors.push_back(mock_sensor);
	return sensors;
}

#pragma region POC


std::string pipe_name = "\\\\.\\pipe\\DepthStreamSink";

rs2_intrinsics rs2::ethernet_device::get_intrinsics()
{
	rs2_intrinsics intrinsics = { W, H,
		(float)W / 2, (float)H / 2,
		(float)W / 2, (float)H / 2,
		RS2_DISTORTION_BROWN_CONRADY ,{ 0,0,0,0,0 } };

	return intrinsics;
}

rs2_software_video_frame& rs2::ethernet_device::get_frame() {
	std::lock_guard<std::mutex> lck(mtx);
	return depth_frame;
}

rs2_device* rs2::ethernet_device::get_device() {
	return dev;
}

void rs2::ethernet_device::create_sensors() {

	rs2_intrinsics depth_intrinsics = get_intrinsics();
	depth_sensor = rs2_software_device_add_sensor(dev, "Depth (Remote)", NULL);
	rs2_video_stream st = { RS2_STREAM_DEPTH, 0, 1, W,
							H, 30, BPP,
							RS2_FORMAT_Z16, depth_intrinsics };
	depth_stream = rs2_software_sensor_add_video_stream(depth_sensor, st, NULL);
	depth_frame.profile = depth_stream;
}

void rs2::ethernet_device::read_frame()
{
	std::lock_guard<std::mutex> lck(mtx);
	DWORD bytesRead = 0;
	if (ReadFile(hPipe, depth_frame.pixels, W * H * BPP, &bytesRead, NULL)) {
		std::cout << " Read: " << bytesRead << " Bytes from buffer!" << std::endl;
	}
	else
		std::cerr << "Cannot read from buffer!" << std::endl;

	using namespace std::chrono;
	auto now = system_clock::now();
	depth_frame.timestamp = time_point_cast<milliseconds>(now).time_since_epoch().count();
	rs2_software_sensor_on_video_frame(depth_sensor, depth_frame, NULL);

	depth_frame.frame_number++;
}

void rs2::ethernet_device::thread_main() {
	while (is_active) {
		read_frame();
	}
}

#pragma endregion


