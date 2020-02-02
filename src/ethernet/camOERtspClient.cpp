#include "camOERtspClient.h"
#include "camOESink.h"

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#include <iostream>
#include <thread>
//#include <condition_variable>
#include <vector>
#include <string>
#include <math.h>

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"
#define REQUEST_STREAMING_OVER_TCP 0 // TODO - uderstand this

//int camOERTSPClient::stream_counter = 0;

long long int camOERTSPClient::getStreamProfileUniqueKey(rs2_video_stream profile)
{
	long long int key;
	key = profile.type * pow(10, 12) + profile.fmt * pow(10, 10) + profile.fps * pow(10, 8);
	//if (profile.is<rs2::video_stream_profile>())
	{
		//rs2::video_stream_profile video_stream_profile = profile.as<rs2::video_stream_profile>();
		key += profile.width * pow(10, 4) + profile.height;
	}
	return key;
}

IcamOERtsp* camOERTSPClient::getRtspClient(char const* rtspURL,
	char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler); 
  return (IcamOERtsp*) new camOERTSPClient(*env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, applicationName, tunnelOverHTTPPortNum);
  //return rtspClient;
}

camOERTSPClient::camOERTSPClient(UsageEnvironment& env, char const* rtspURL,
			     int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) {
}
camOERTSPClient::~camOERTSPClient() {
}

std::vector<rs2_video_stream> camOERTSPClient::queryStreams()
{
  // TODO - handle in a function
    unsigned res = this->sendDescribeCommand(this->continueAfterDESCRIBE);
    if (res == 0)
    {
      // An error occurred (continueAfterDESCRIBE was already called)
      // TODO: return error code
      return this->supportedProfiles;
    }
    this->envir() << "in sendDescribe after sending command\n";
    // wait for continueAfterDESCRIBE to finish
    std::unique_lock<std::mutex> lck(command_mtx);
    cv.wait(lck, [this]{return cammand_done;}); 
    // for the next command
    cammand_done = false; 

    this->envir() << "in sendDescribe After wait\n";   
    return this->supportedProfiles;
}

int camOERTSPClient::addStream(rs2_video_stream stream, rtp_callback* callback_obj)
{
  this->envir()  << "looking for sub session \n";
  long long uniqueKey = getStreamProfileUniqueKey(stream);
  RsMediaSubsession* subsession = this->subsessionMap.find(uniqueKey)->second;
  this->envir()  << "find sub session " << subsession  << "\n";;
  if (subsession != NULL) {
    this->envir()  << " initiate subsession"  << "\n";;
     if (!subsession->initiate()) {
        this->envir() << "Failed to initiate the subsession \n";
        
        } else {
        this->envir()  << "Initiated the subsession \n";;
        
        // Continue setting up this subsession, by sending a RTSP "SETUP" command:
        unsigned res = this->sendSetupCommand(*subsession, this->continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP); 
        // wait for continueAfterSETUP to finish
        std::unique_lock<std::mutex> lck(command_mtx);
        cv.wait(lck, [this]{return cammand_done;}); 
        // for the next command
        cammand_done = false;
        
        if (this->commandResultCode == 0)
        {
          // TODO: change size according to BPP
          //
          subsession->sink = camOESink::createNew(this->envir(), *subsession, stream.width*stream.height*2, this->url());
        // perhaps use your own custom "MediaSink" subclass instead
          if (subsession->sink == NULL) {
            this->envir() << "Failed to create a data sink for the subsession: " << this->envir().getResultMsg() << "\n";
            // TODO: define error
            this->commandResultCode = -1;
            return this->commandResultCode;
          }

        this->envir()  << "Created a data sink for the subsession\n";
        subsession->miscPtr = this; // a hack to let subsession handler functions get the "RTSPClient" from the subsession 
        ((camOESink*)(subsession->sink))->set_callback(callback_obj);
        subsession->sink->startPlaying(*(subsession->readSource()),
                  subsessionAfterPlaying, subsession);
        // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
        if (subsession->rtcpInstance() != NULL) {
          subsession->rtcpInstance()->setByeWithReasonHandler(subsessionByeHandler, subsession);
          }
        }
        return this->commandResultCode;
      }
  }
  // TODO: return error - setup failed
  return -1;
}
int camOERTSPClient::start()
{
  unsigned res = this->sendPlayCommand(*this->scs.session, this->continueAfterPLAY);
  // wait for continueAfterPLAY to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck, [this]{return cammand_done;}); 
  // for the next command
  cammand_done = false;
  return this->commandResultCode;
}

int camOERTSPClient::stop(rs2_video_stream stream)
{
  // TODO: uniqueu key
  MediaSubsession* subsession = this->subsessionMap.find(stream.uid)->second;
  unsigned res = this->sendPauseCommand(*subsession, this->continueAfterPAUSE);
  // wait for continueAfterPAUSE to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck, [this]{return cammand_done;}); 
  // for the next command
  cammand_done = false; 
  return this->commandResultCode;
}

int camOERTSPClient::stop()
{
  unsigned res = this->sendPauseCommand(*this->scs.session, this->continueAfterPAUSE);
  // wait for continueAfterPAUSE to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck, [this]{return cammand_done;}); 
  // for the next command
  cammand_done = false;
  return this->commandResultCode;
}


int camOERTSPClient::close()
{
  unsigned res = this->sendTeardownCommand(*this->scs.session, this->continueAfterTEARDOWN);
  // wait for continueAfterTEARDOWN to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck, [this]{return cammand_done;}); 
  // for the next command
  cammand_done = false;
  is_connected = false;
  
  int res_code = this->commandResultCode;

  // delete the rtsp instance
  this->envir() <<  "Closing the stream.\n";
  //Medium::close(this);

  return res_code;
}

void schedulerThread(camOERTSPClient* rtspClientInstance)
{
    rtspClientInstance->envir() << "in schedulerThread!\n";
    char eventLoopWatchVariable = 0;
    rtspClientInstance->envir().taskScheduler().doEventLoop(&eventLoopWatchVariable);
}

void camOERTSPClient::initFunc()
{
   std::thread thread_scheduler(schedulerThread, this);
   thread_scheduler.detach();
   is_connected=true;
}

bool camOERTSPClient::isConnected()
{
  return is_connected;
}

// TODO: Error handling
void camOERTSPClient::continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((camOERTSPClient*)rtspClient)->scs; // alias
  camOERTSPClient* camOeRtspClient = ((camOERTSPClient*)rtspClient); // alias
  do {
    if (resultCode != 0) {
      env << "Failed to get a SDP description: " << resultString << "\n";
      delete[] resultString;
      break;
    }

    char* const sdpDescription = resultString;
    env << "Got a SDP description:\n" << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    scs.session = RsMediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << "Failed to create a RsMediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    scs.iter = new RsMediaSubsessionIterator(*scs.session);
    scs.subsession = scs.iter->next();
    while (scs.subsession != NULL) {
      // Get more data from the SDP string 
      const char* strWidthVal = scs.subsession->attrVal_str("width");
      const char* strHeightVal = scs.subsession->attrVal_str("height");
      const char* strFormatVal = scs.subsession->attrVal_str("format");
      const char* strUidVal = scs.subsession->attrVal_str("uid");
      const char* strFpsVal = scs.subsession->attrVal_str("fps");
      const char* strIndexVal = scs.subsession->attrVal_str("stream_index");
      const char* strStreamTypeVal = scs.subsession->attrVal_str("stream_type");

      int width = strWidthVal != "" ? std::stoi(strWidthVal) : 0;
      int height = strHeightVal != "" ? std::stoi(strHeightVal) : 0;
      int format = strFormatVal != "" ? std::stoi(strFormatVal) : 0;
      int uid = strUidVal != "" ? std::stoi(strUidVal) : 0;
      int fps = strFpsVal != "" ? std::stoi(strFpsVal) : 0;
      int index = strIndexVal != "" ? std::stoi(strIndexVal) : 0;
      int stream_type = strStreamTypeVal != "" ? std::stoi(strStreamTypeVal): 0;
      rs2_video_stream videoStream;
      videoStream.width = width;
      videoStream.height = height;
      videoStream.uid = uid;//camOERTSPClient::stream_counter++;
      videoStream.fmt = static_cast<rs2_format>(format);
      videoStream.fps = fps;
      videoStream.index = index;
      videoStream.type = static_cast<rs2_stream>(stream_type);
    
      std::string url_str = rtspClient->url();
      // Remove last "/"
      url_str = url_str.substr(0, url_str.size()-1);
      std::size_t stream_name_index = url_str.find_last_of("/") + 1;
      std::string stream_name = url_str.substr(stream_name_index, url_str.size());

      //nhershko: hard coded fixes
      videoStream.bpp=2;      

      // TODO: update width and height in subsession?
      long long uniqueKey = getStreamProfileUniqueKey(videoStream);
      camOeRtspClient->subsessionMap.insert(std::pair<int, RsMediaSubsession*>(uniqueKey, scs.subsession));
      camOeRtspClient->supportedProfiles.push_back(videoStream);
      scs.subsession = scs.iter->next();
      // TODO: when to delete p?
    }

  } while (0);

  {
    std::lock_guard<std::mutex> lck(camOeRtspClient->command_mtx);
    camOeRtspClient->cammand_done = true;
  }
  camOeRtspClient->cv.notify_one();

  // An unrecoverable error occurred with this stream.
  // TODO: 
  //shutdownStream(rtspClient);
}

void camOERTSPClient::continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((camOERTSPClient*)rtspClient)->scs; // alias
  camOERTSPClient* camOeRtspClient = ((camOERTSPClient*)rtspClient); // alias
  env << "continueAfterSETUP " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  {
    std::lock_guard<std::mutex> lck(camOeRtspClient->command_mtx);
    camOeRtspClient->cammand_done = true;
  }
  camOeRtspClient->cv.notify_one();
}

void camOERTSPClient::continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  camOERTSPClient* camOeRtspClient = ((camOERTSPClient*)rtspClient); // alias
  env << "continueAfterPLAY " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  {
    std::lock_guard<std::mutex> lck(camOeRtspClient->command_mtx);
    camOeRtspClient->cammand_done = true;
  }
  camOeRtspClient->cv.notify_one();
}

void camOERTSPClient::continueAfterTEARDOWN(RTSPClient* rtspClient, int resultCode, char* resultString)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  camOERTSPClient* camOeRtspClient = ((camOERTSPClient*)rtspClient); // alias
  env << "continueAfterTEARDOWN " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  // In order to start the next UID from 0
  //camOERTSPClient::stream_counter = 0;
  {
    std::lock_guard<std::mutex> lck(camOeRtspClient->command_mtx);
    camOeRtspClient->cammand_done = true;
  }
  camOeRtspClient->cv.notify_one();
}

void camOERTSPClient::continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  camOERTSPClient* camOeRtspClient = ((camOERTSPClient*)rtspClient); // alias
  env << "continueAfterPAUSE " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  {
    std::lock_guard<std::mutex> lck(camOeRtspClient->command_mtx);
    camOeRtspClient->cammand_done = true;
  }
  camOeRtspClient->cv.notify_one();
}

// TODO: implementation
void camOERTSPClient::subsessionAfterPlaying(void* clientData)
{
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);
  rtspClient->envir() << "subsessionAfterPlaying\n";
}
void camOERTSPClient::subsessionByeHandler(void* clientData, char const* reason)
{

}
