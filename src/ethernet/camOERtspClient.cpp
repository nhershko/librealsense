#include "camOERtspClient.h"
#include "camOESink.h"

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#include <iostream>
#include <thread>
#include <condition_variable>
#include <vector>
#include <string>

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"
#define REQUEST_STREAMING_OVER_TCP 0 // TODO - uderstand this

int camOERTSPClient::stream_counter = 0;

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

// TODO: should we have seperate mutex for each command?
std::condition_variable cv;
std::mutex command_mtx;
bool cammand_done = false;

// Forward function definitions:

// RTSP 'response handlers':
//void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
//void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
//void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
//void continueAfterTEARDOWN(RTSPClient* rtspClient, int resultCode, char* resultString);
//void continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString);

//void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
//void subsessionByeHandler(void* clientData, char const* reason);

std::vector<rtp_rs_video_stream> camOERTSPClient::queryStreams()
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
    cv.wait(lck, []{return cammand_done;}); 
    // for the next command
    cammand_done = false; 

    this->envir() << "in sendDescribe After wait\n";   
    return this->supportedProfiles;
}

int camOERTSPClient::addStream(rtp_rs_video_stream stream, rtp_callback* callback_obj)
{
  this->envir()  << "looking for sub session \n";;
  MediaSubsession* subsession = this->subsessionMap.find(stream.rtp_uid)->second;
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
        cv.wait(lck, []{return cammand_done;}); 
        // for the next command
        cammand_done = false;
        
        if (this->commandResultCode == 0)
        {
          // TODO: change size according to BPP
          //
          subsession->sink = camOESink::createNew(this->envir(), *subsession, stream.video_stream.width*stream.video_stream.height*2, this->url());
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
  cv.wait(lck, []{return cammand_done;}); 
  // for the next command
  cammand_done = false;
  return this->commandResultCode;
}

int camOERTSPClient::stop(rtp_rs_video_stream stream)
{
  MediaSubsession* subsession = this->subsessionMap.find(stream.rtp_uid)->second;
  unsigned res = this->sendPauseCommand(*subsession, this->continueAfterPAUSE);
  // wait for continueAfterPAUSE to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck, []{return cammand_done;}); 
  // for the next command
  cammand_done = false; 
  return this->commandResultCode;
}

int camOERTSPClient::stop()
{
  unsigned res = this->sendPauseCommand(*this->scs.session, this->continueAfterPAUSE);
  // wait for continueAfterPAUSE to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck, []{return cammand_done;}); 
  // for the next command
  cammand_done = false;
  return this->commandResultCode;
}


int camOERTSPClient::close()
{
  unsigned res = this->sendTeardownCommand(*this->scs.session, this->continueAfterTEARDOWN);
  // wait for continueAfterTEARDOWN to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck, []{return cammand_done;}); 
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
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((camOERTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << "Failed to get a SDP description: " << resultString << "\n";
      delete[] resultString;
      break;
    }

    char* const sdpDescription = resultString;
    env << "Got a SDP description:\n" << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    scs.iter = new MediaSubsessionIterator(*scs.session);
    scs.subsession = scs.iter->next();
    while (scs.subsession != NULL) {
      // Get more data from the SDP string 
      const char* strWidthVal = scs.subsession->attrVal_str("width");
      const char* strHeightVal = scs.subsession->attrVal_str("height");
      const char* strFormatVal = scs.subsession->attrVal_str("format");
      const char* strUidVal = scs.subsession->attrVal_str("uid");
      const char* strFpsVal = scs.subsession->attrVal_str("fps");
      const char* strIndexVal = scs.subsession->attrVal_str("index");
      const char* strStreamTypeVal = scs.subsession->attrVal_str("stream_type");

      int width = strWidthVal != "" ? std::stoi(strWidthVal) : 0;
      int height = strHeightVal != "" ? std::stoi(strHeightVal) : 0;
      int format = strFormatVal != "" ? std::stoi(strFormatVal) : 0;
      int uid = strUidVal != "" ? std::stoi(strUidVal) : 0;
      int fps = strFpsVal != "" ? std::stoi(strFpsVal) : 0;
      int index = strIndexVal != "" ? std::stoi(strIndexVal) : 0;
      int stream_type = strStreamTypeVal != "" ? std::stoi(strStreamTypeVal): 0;
      rtp_rs_video_stream rtpVideoStream;
      rtpVideoStream.rtp_uid = camOERTSPClient::stream_counter++;
      rtpVideoStream.video_stream.width = width;
      rtpVideoStream.video_stream.height = height;
      rtpVideoStream.video_stream.uid = uid; //camOERTSPClient::stream_counter++;
      rtpVideoStream.video_stream.fmt = static_cast<rs2_format>(format);
      rtpVideoStream.video_stream.fps = fps;
      rtpVideoStream.video_stream.index = index;
      rtpVideoStream.video_stream.type = static_cast<rs2_stream>(stream_type);
    
      std::string url_str = rtspClient->url();
      // Remove last "/"
      url_str = url_str.substr(0, url_str.size()-1);
      std::size_t stream_name_index = url_str.find_last_of("/") + 1;
      std::string stream_name = url_str.substr(stream_name_index, url_str.size());
      /*if (stream_name.compare("depth") == 0)
      {
        rtpVideoStream.video_stream.type = RS2_STREAM_DEPTH;
        //nhershko: hard coded 
        if(videoStream.fmt == RS2_FORMAT_RGB8)
          videoStream.type = RS2_STREAM_INFRARED;
        else
            videoStream.fmt = RS2_FORMAT_Z16;
      }
      else if((stream_name.compare("color") == 0))
      {
        rtpVideoStream.video_stream.type = RS2_STREAM_COLOR;
        //nhershko: hard coded 
       // videoStream.fmt = RS2_FORMAT_YUYV;
      }*/

      //nhershko: hard coded fixes
      rtpVideoStream.video_stream.bpp=2;      

      // TODO: update width and height in subsession?
      ((camOERTSPClient*)rtspClient)->subsessionMap.insert(std::pair<int, MediaSubsession*>(rtpVideoStream.rtp_uid, scs.subsession));
      ((camOERTSPClient*)rtspClient)->supportedProfiles.push_back(rtpVideoStream);
      scs.subsession = scs.iter->next();
      // TODO: when to delete p?
    }

  } while (0);

  {
    std::lock_guard<std::mutex> lck(command_mtx);
    cammand_done = true;
  }
  cv.notify_one();

  // An unrecoverable error occurred with this stream.
  // TODO: 
  //shutdownStream(rtspClient);
}

void camOERTSPClient::continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((camOERTSPClient*)rtspClient)->scs; // alias
  env << "continueAfterSETUP " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  {
    std::lock_guard<std::mutex> lck(command_mtx);
    cammand_done = true;
  }
  cv.notify_one();
}

void camOERTSPClient::continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  env << "continueAfterPLAY " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  {
    std::lock_guard<std::mutex> lck(command_mtx);
    cammand_done = true;
  }
  cv.notify_one();

}

void camOERTSPClient::continueAfterTEARDOWN(RTSPClient* rtspClient, int resultCode, char* resultString)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  env << "continueAfterTEARDOWN " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  // In order to start the next UID from 0
  camOERTSPClient::stream_counter = 0;
  {
    std::lock_guard<std::mutex> lck(command_mtx);
    cammand_done = true;
  }
  cv.notify_one();
}

void camOERTSPClient::continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  env << "continueAfterPAUSE " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  {
    std::lock_guard<std::mutex> lck(command_mtx);
    cammand_done = true;
  }
  cv.notify_one();
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
