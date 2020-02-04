#ifndef _CAM_OE_RTSP_CLIENT_H
#define _CAM_OE_RTSP_CLIENT_H

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "StreamClientState.h"
#include "IcamOERtsp.h"

#include <librealsense2/hpp/rs_internal.hpp>

#include <vector>
#include <map>
#include <condition_variable>

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

    static long long int getStreamProfileUniqueKey(rs2_video_stream profile);
    void setDeviceData(device_data data);

    // IcamOERtsp functions
    virtual std::vector<rs2_video_stream> queryStreams();
    virtual int addStream(rs2_video_stream stream, rtp_callback* frameCallBack);
    virtual int start();
    virtual int stop(rs2_video_stream stream);
    virtual int stop();
    virtual int close();
    virtual device_data getDeviceData() { return fDeviceData; }

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

    StreamClientState scs;
    std::vector<rs2_video_stream> supportedProfiles;
    std::map<int, RsMediaSubsession*> subsessionMap;
    int commandResultCode;
    static int stream_counter;
    // TODO: should we have seperate mutex for each command?
    std::condition_variable cv;
    std::mutex command_mtx;
    bool cammand_done = false;
    // TODO: W/A for stop - should be removed
    bool is_connected;
    device_data fDeviceData;

};
#endif // _CAM_OE_RTSP_CLIENT_H

