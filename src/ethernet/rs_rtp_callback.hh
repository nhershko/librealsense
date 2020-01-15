
#pragma once

#include <time.h>
#include <queue>

#include <memory>
#include <mutex>

#include "rtp_stream.hh"

class rs_rtp_callback : public rtp_callback
{

private:

    const int QUEUE_MAX_SIZE = 10;

    std::shared_ptr<rs_rtp_stream> m_rtp_stream;
    
    int m_stream_uid;
    std::shared_ptr<std::queue<Tmp_Frame*>> m_frames_queue;
    std::shared_ptr<std::mutex> m_queue_lock;
    int arrive_frames_counter;

public:
    
    rs_rtp_callback(int stream_uid, std::queue<Tmp_Frame*>* frames_queue_ptr, std::mutex* queue_lock)
    :m_stream_uid(stream_uid),
        m_frames_queue(frames_queue_ptr),
            m_queue_lock(queue_lock)
    {        
    }

    rs_rtp_callback(std::shared_ptr<rs_rtp_stream> rtp_stream)
    {        
        m_rtp_stream = rtp_stream;
    }

    ~rs_rtp_callback();

    void on_frame(unsigned char*buffer,ssize_t size, struct timeval presentationTime);

    int arrived_frames(){   return arrive_frames_counter;   }
};

