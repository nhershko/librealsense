#include "rtspconnectionclient.h"
#include "../ethernet-device.h"

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

class RTSPCallback : public RTSPConnection::Callback
{
	private:
		std::map<std::string,std::ofstream> m_ofs;
		std::string m_fileprefix;
		
	public:
		RTSPCallback(const std::string & output) : m_fileprefix(output)  {}
		
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
            std::cout << "[nhershko] got data!" << std::endl;
			auto it = m_ofs.find(id);
			if (it != m_ofs.end()) {
				it->second.write((char*)buffer, size);
			}
			return true;
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

class MKVCallback : public MKVClient::Callback
{
	private:
		std::map<std::string,std::ofstream> m_ofs;
		std::string m_fileprefix;
		
	public:
		MKVCallback(const std::string & output) : m_fileprefix(output)  {}
		
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
};

char stop = 0;
void sig_handler(int signo)
{
	if (signo == SIGINT) {
		printf("received SIGINT\n");
		stop = 1;
	}
}


//nhershko

