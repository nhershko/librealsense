#pragma once

#include <librealsense2/rs.hpp>
#include "camOERtspClient.h"
#include "software-device.h"

struct Tmp_Frame
{
	Tmp_Frame(char* buffer, int size,  struct timeval timestamp) : m_buffer(buffer), m_size(size), m_timestamp(timestamp) {};
	Tmp_Frame(const Tmp_Frame&);
	Tmp_Frame& operator=(const Tmp_Frame&);
	~Tmp_Frame()  { delete [] m_buffer; };
	
	char* m_buffer;
	unsigned int m_size;
	struct timeval m_timestamp;
};

class rs_rtp_stream
{
    public:

        rs_rtp_stream(rs2_video_stream rs_stream, rs2::stream_profile rs_profile)
        {
            std::cout << "\t@@@ initiate frame buffer for stream ID: " << rs_stream.uid <<std::endl;
            frame_data_buff.bpp = rs_stream.bpp;
		    frame_data_buff.profile = rs_profile;
		    frame_data_buff.stride = rs_stream.bpp * rs_stream.width;
		    pixels_buff.resize(frame_data_buff.stride * rs_stream.height, 0);
            frame_data_buff.pixels = pixels_buff.data();
    		frame_data_buff.deleter = this->frame_deleter;

            m_rs_stream = rs_stream;

        }

        rs2_stream stream_type()
        {
            return m_rs_stream.type;                   
        }

        void insert_frame(Tmp_Frame* new_raw_frame)
        {
            this->stream_lock.lock();
                frames_queue.push(new_raw_frame);
            this->stream_lock.unlock();

        }

        Tmp_Frame* extract_frame()
        {
            this->stream_lock.lock();
            Tmp_Frame* frame = frames_queue.front();
			frames_queue.pop();
            this->stream_lock.unlock();
            return frame;
        }

        void reset_queue()
        {
            while(!frames_queue.empty())
            {
                frames_queue.pop();
            }
            std::cout << "done clean frames queue: " << m_rs_stream.uid << std::endl;
        }

        int queue_size()
        {
            int size;
            this->stream_lock.lock();
            size = frames_queue.size();
            this->stream_lock.unlock();
            return size;
        }

        
        bool is_enabled;

        rs2_video_stream m_rs_stream;

        rs2_software_video_frame frame_data_buff;

    private:

        static void frame_deleter(void* p) {}

        std::mutex stream_lock;

        std::queue<Tmp_Frame*> frames_queue;
        // frame data buffers
        
        // pixels data 
        std::vector<uint8_t> pixels_buff;
        
        //nhershko consider remove the below profile
        rs2::stream_profile profile;
};
