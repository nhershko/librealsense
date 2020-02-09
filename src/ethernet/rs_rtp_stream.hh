#pragma once

#include <librealsense2/rs.hpp>
#include "camOERtspClient.h"
#include "software-device.h"

const int RTP_QUEUE_MAX_SIZE = 30;

struct Raw_Frame
{
	Raw_Frame(char* buffer, int size,  struct timeval timestamp) : m_buffer(buffer), m_size(size), m_timestamp(timestamp) {};
	Raw_Frame(const Raw_Frame&);
	Raw_Frame& operator=(const Raw_Frame&);
	~Raw_Frame()  { delete [] m_buffer; };
	
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

        void insert_frame(Raw_Frame* new_raw_frame)
        {
            if(queue_size()>RTP_QUEUE_MAX_SIZE)
            {
                std::cout << "queue is full. dropping frame for stream id: " << this->m_rs_stream.uid << std::endl;
            }
            else
            {
                this->stream_lock.lock();
                    frames_queue.push(new_raw_frame);
                this->stream_lock.unlock();
            }
        }

        Raw_Frame* extract_frame()
        {
            this->stream_lock.lock();
            Raw_Frame* frame = frames_queue.front();
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

        static memory_pool& get_memory_pool()
        {
            static memory_pool memory_pool_instance = memory_pool();
            return memory_pool_instance;
        }

    private:

        
        static void frame_deleter(void* p) { 
                get_memory_pool().returnMem((unsigned char*)p - sizeof(rs_over_ethernet_data_header));
            }

        std::mutex stream_lock;

        std::queue<Raw_Frame*> frames_queue;
        // frame data buffers
        
        // pixels data 
        std::vector<uint8_t> pixels_buff;
        
        //nhershko consider remove the below profile
        rs2::stream_profile profile;
};
