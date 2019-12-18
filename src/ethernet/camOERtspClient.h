#ifndef _CAM_OE_RTSP_CLIENT_H
#define _CAM_OE_RTSP_CLIENT_H

#include "Profile.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "StreamClientState.h"
#include "IcamOERtsp.h"

#include <vector>

class camOERTSPClient: public RTSPClient, IcamOERtsp
{
public:
    static IcamOERtsp* getRtspClient(char const* rtspURL,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0);
    void sendDescribe();
    void initFunc();


// IcamOERtsp functions
  virtual std::vector<Profile> queryProfiles();
  virtual int addProfile(Profile);
  virtual void start();
  virtual void stop();
  virtual void close();

protected:
    

// TODO: deside about protection level
public:
  StreamClientState scs;
  std::vector<Profile> supportedProfiles;

  //IcamOErtsp moshe = factory(typeof(camOERTSPClient));

private:
    //void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
    camOERTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
    virtual ~camOERTSPClient();
};
#endif // _CAM_OE_RTSP_CLIENT_H

