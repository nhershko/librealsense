#include "rtspconnectionclient.h"

struct Frame
{
	Frame(char* buffer, int size, timeval timestamp) : m_buffer(buffer), m_size(size), m_timestamp(timestamp) {};
	Frame(const Frame&);
	Frame& operator=(const Frame&);
	~Frame()  { delete [] m_buffer; };
	
	char* m_buffer;
	unsigned int m_size;
	timeval m_timestamp;
};

class myCB : public RTSPConnection::Callback
{
	myCB(void* on_frame_calllback);
	bool onData(char sink_id, const char* id, unsigned char* buffer, ssize_t size, struct timeval presentationTime) override
		{
			/*
			if(96==sink_id)
				m_dev->add_frame_to_queue(0,new Frame((char*)buffer,size,presentationTime));
			else if (97==sink_id)
				m_dev->add_frame_to_queue(1,new Frame((char*)buffer,size,presentationTime));
			return true;
			*/
		}

};

class RTSPCallback : public RTSPConnection::Callback
{
	private:
		std::map<std::string,std::ofstream> m_ofs;
		std::string m_fileprefix;
		void* m_fn;
		//rs2::ethernet_device* m_dev;
		
	public:
		RTSPCallback(const std::string & output) : m_fileprefix(output)  {}
		RTSPCallback(void* fn, const std::string & output)
		{
			m_fn = fn;
		}
		
		virtual bool    onNewSession(const char* id, const char* media, const char* codec, const char*) {
			std::cout << id << " " << media << "/" <<  codec << std::endl;
			return true;
		}
		
		virtual bool    onData(const char* id, unsigned char* buffer, ssize_t size, struct timeval presentationTime) {
			std::cout << id << " " << size << " ts:" << presentationTime.tv_sec << "." << presentationTime.tv_usec << std::endl;
            std::cout << "[nhershko] got data!" << std::endl;
			auto it = m_ofs.find(id);
			if (it != m_ofs.end()) {
				it->second.write((char*)buffer, size);
			}
			return true;
		}

		bool onData(char sink_id, const char* id, unsigned char* buffer, ssize_t size, struct timeval presentationTime) override
		{
			/*
			if(96==sink_id)
				m_dev->add_frame_to_queue(0,new Frame((char*)buffer,size,presentationTime));
			else if (97==sink_id)
				m_dev->add_frame_to_queue(1,new Frame((char*)buffer,size,presentationTime));
			return true;
			*/
		}
		
		virtual void    onError(RTSPConnection& connection, const char* message) {
			std::cout << "Error:" << message << std::endl;
			connection.start(10);
		}
		
		virtual void    onConnectionTimeout(RTSPConnection& connection) {
			std::cout << "Connection timeout -> retry" << std::endl;
			connection.start();
		}
		
		virtual void    onDataTimeout(RTSPConnection& connection)       {
			std::cout << "Data timeout -> retry" << std::endl;
			connection.start();
		}		
};

class SDPCallback : public SDPClient::Callback
{
	private:
		std::map<std::string,std::ofstream> m_ofs;
		std::string m_fileprefix;
		
	public:
		SDPCallback(const std::string & output) : m_fileprefix(output)  {}
		
		virtual bool    onNewSession(const char* id, const char* media, const char* codec, const char*) {
			if (!m_fileprefix.empty()) {
				auto it = m_ofs.find(id);
				if (it == m_ofs.end()) {
					std::string filename = m_fileprefix + "_" + media + "_" + codec + "_" + id;
					m_ofs[id].open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
				}
			}
			std::cout << id << " " << media << "/" <<  codec << std::endl;
			return true;
		}
				
		virtual bool    onData(const char* id, unsigned char* buffer, ssize_t size, struct timeval presentationTime) {
			std::cout << id << " " << size << " ts:" << presentationTime.tv_sec << "." << presentationTime.tv_usec << std::endl;
			auto it = m_ofs.find(id);
			if (it != m_ofs.end()) {
				it->second.write((char*)buffer, size);
			}
			return true;
		}
		
		virtual void    onError(SDPClient& connection, const char* message) {
			std::cout << "Error:" << message << std::endl;
		}		
};


