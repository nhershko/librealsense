#include "rtp_callback.hh"
#include <iostream>

void rtp_callback::on_frame_callback(unsigned char*buffer,ssize_t size, struct timeval presentationTime)
{
    if(QUEUE_MAX_SIZE > m_frames_queue.get()->size())
	{
		m_queue_lock->lock();
        std::cout <<"\n\n\Got frame\n\n\n";
		this->m_frames_queue->push(new Tmp_Frame((char*)buffer,size,presentationTime));
        arrive_frames_counter++;
        m_queue_lock->unlock();
	}
	else
	{
		//std::cout<< "queue is full. dropping frame of type " << type << std::endl;
	}
    return;
}

rtp_callback::~rtp_callback()
{

}
