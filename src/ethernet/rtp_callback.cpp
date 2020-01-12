#include "rtp_callback.hh"
#include <iostream>

void rtp_callback::on_frame(unsigned char*buffer,ssize_t size, struct timeval presentationTime)
{
    
    if(QUEUE_MAX_SIZE > m_frames_queue.get()->size())
	{
		m_queue_lock->lock();
        this->m_frames_queue->push(new Tmp_Frame((char*)buffer,size,presentationTime));
        m_queue_lock->unlock();
        arrive_frames_counter++;
	}
	else
	{
		std::cout<< "queue is full. dropping frame of type " << this->m_stream_uid << std::endl;
	}
    return;
}

rtp_callback::~rtp_callback()
{

}
