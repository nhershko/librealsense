#ifndef _CAM_OE_RTSP_CLIENT_H
#define _CAM_OE_RTSP_CLIENT_H

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "StreamClientState.h"
#include "IcamOERtsp.h"

#include <librealsense2/hpp/rs_internal.hpp>

#include <vector>
#include <map>

class camOERTSPClient: public RTSPClient, IcamOERtsp
{
public:
    static IcamOERtsp* getRtspClient(char const* rtspURL,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0);
    void describe();
    void setup(rs2_video_stream stream);
    void initFunc();



// IcamOERtsp functions
  virtual std::vector<rs2_video_stream> queryStreams();
  virtual int addStream(rs2_video_stream stream);
  virtual void start();
  virtual void stop();
  virtual void close();

protected:
  
// TODO: deside about protection level
public:
  StreamClientState scs;
  std::vector<rs2_video_stream> supportedProfiles;
  std::map<int, MediaSubsession*> subsessionMap;

private:
    camOERTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
    virtual ~camOERTSPClient();
};
#endif // _CAM_OE_RTSP_CLIENT_H

