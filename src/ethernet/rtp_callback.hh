#pragma once

#include <time.h>
#include <queue>

#include <memory>
#include <mutex>


class rs_callback
{
    public:
        
        void virtual on_frame(unsigned char*buffer,ssize_t size, struct timeval presentationTime)=0;
        
        //void virtual on_error(int code, std::string message)=0;
        
        //void virtual on_messege(std::string message)=0;
};

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

class rtp_callback : public rs_callback
{

private:

    const int QUEUE_MAX_SIZE = 10;
    
    int m_stream_uid;
    std::shared_ptr<std::queue<Tmp_Frame*>> m_frames_queue;
    std::shared_ptr<std::mutex> m_queue_lock;
    int arrive_frames_counter;

public:
    
    rtp_callback(int stream_uid, std::queue<Tmp_Frame*>* frames_queue_ptr, std::mutex* queue_lock)
    :m_stream_uid(stream_uid),
        m_frames_queue(frames_queue_ptr),
            m_queue_lock(queue_lock)
    {        
    }
    ~rtp_callback();

    void on_frame(unsigned char*buffer,ssize_t size, struct timeval presentationTime);

    int arrived_frames(){   return arrive_frames_counter;   }
};

