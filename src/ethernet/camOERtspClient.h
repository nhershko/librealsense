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
    virtual bool isConnected();



// IcamOERtsp functions
  virtual std::vector<rs2_video_stream> queryStreams();
  virtual int addStream(rs2_video_stream stream, rtp_callback* frameCallBack);
  virtual int start();
  virtual int stop(rs2_video_stream stream);
  virtual int stop();
  virtual int close();

protected:
  
// TODO: deside about protection level
public:
  StreamClientState scs;
  std::vector<rs2_video_stream> supportedProfiles;
  std::map<int, MediaSubsession*> subsessionMap;
  int commandResultCode;
  static void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
  static void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
  static void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
  static void continueAfterTEARDOWN(RTSPClient* rtspClient, int resultCode, char* resultString);
  static void continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString);
  static void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
  static void subsessionByeHandler(void* clientData, char const* reason);


private:
    camOERTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);

    // called only by createNew();
    virtual ~camOERTSPClient();

    bool is_connected;
};
#endif // _CAM_OE_RTSP_CLIENT_H

