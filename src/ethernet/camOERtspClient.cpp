#include "camOERtspClient.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#include <iostream>
#include <thread>
#include <condition_variable>
#include <vector>
#include <string>

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"
#define REQUEST_STREAMING_OVER_TCP 0 // TODO - uderstand this

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
bool describe_done = false;

// Forward function definitions:

// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterTEARDOWN(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString);

std::vector<rs2_video_stream> camOERTSPClient::queryStreams()
{
  // TODO - handle in a function
    unsigned res = this->sendDescribeCommand(continueAfterDESCRIBE);
    if (res == 0)
    {
      // An error occurred (continueAfterDESCRIBE was already called)
      // TODO: return error code
      return this->supportedProfiles;
    }
    this->envir() << "in sendDescribe after sending command\n";
    // wait for continueAfterDESCRIBE to finish
    std::unique_lock<std::mutex> lck(command_mtx);
    cv.wait(lck); 

    this->envir() << "in sendDescribe After wait\n";   
    return this->supportedProfiles;
}
int camOERTSPClient::addStream(rs2_video_stream stream)
{
  MediaSubsession* subsession = this->subsessionMap.find(stream.uid)->second;


  if (subsession != NULL) {
     if (!subsession->initiate()) {
       this->envir() << "Failed to initiate the subsession \n";
      
      } else {
      this->envir()  << "Initiated the subsession \n";;

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      unsigned res = this->sendSetupCommand(*subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP); 
      if (res == 0)
      {
        // An error occurred (continueAfterSETUP was already called)
        return this->commandResultCode;
      } 
      // wait for continueAfterSETUP to finish
      std::unique_lock<std::mutex> lck(command_mtx);
      cv.wait(lck); 
      return this->commandResultCode; 
      }
  }
  // TODO: return error - setup failed
  return -1;
}
int camOERTSPClient::start()
{
  unsigned res = this->sendPlayCommand(*this->scs.session, continueAfterPLAY);
  if (res == 0)
  {
    // An error occurred (continueAfterPLAY was already called)
    return this->commandResultCode;
  }
  // wait for continueAfterPLAY to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck); 
  return this->commandResultCode;
}
int camOERTSPClient::stop(rs2_video_stream stream)
{
  MediaSubsession* subsession = this->subsessionMap.find(stream.uid)->second;
  unsigned res = this->sendPauseCommand(*subsession, continueAfterPAUSE);
  if (res == 0)
  {
    // An error occurred (continueAfterPAUSE was already called)
    return this->commandResultCode;
  }
  // wait for continueAfterPAUSE to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck); 
  return this->commandResultCode;
}

int camOERTSPClient::stop()
{
  unsigned res = this->sendPauseCommand(*this->scs.session, continueAfterPAUSE);
  if (res == 0)
  {
    // An error occurred (continueAfterPAUSE was already called)
    return this->commandResultCode;
  }
  // wait for continueAfterPAUSE to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck); 
  return this->commandResultCode;
}


int camOERTSPClient::close()
{
  unsigned res = this->sendTeardownCommand(*this->scs.session, continueAfterTEARDOWN);
  if (res == 0)
  {
    // An error occurred (continueAfterTEARDOWN was already called)
    return this->commandResultCode;
  }
  // wait for continueAfterTEARDOWN to finish
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.wait(lck); 
  return this->commandResultCode;
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
}

// TODO: Error handling
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
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

  int stream_counter = 0;
  scs.iter = new MediaSubsessionIterator(*scs.session);
  scs.subsession = scs.iter->next();
  while (scs.subsession != NULL) {
    // Get more data from the SDP string 
    const char* strWidthVal = scs.subsession->attrVal_str("width");
    const char* strHeightVal = scs.subsession->attrVal_str("height");
    int width = strWidthVal != NULL ? std::stoi(strWidthVal) : 0;
    int height = strHeightVal != NULL ? std::stoi(strHeightVal) : 0;
    rs2_video_stream videoStream;
    videoStream.width = width;
    videoStream.height = height;
    videoStream.uid = stream_counter;
   
    std::string url_str = rtspClient->url();
    // Remove last "/"
    url_str = url_str.substr(0, url_str.size()-1);
    std::size_t stream_name_index = url_str.find_last_of("/") + 1;
    std::string stream_name = url_str.substr(stream_name_index, url_str.size());
    if (stream_name.compare("depth") == 0)
    {
      videoStream.type = RS2_STREAM_DEPTH;
    }
    else if((stream_name.compare("color") == 0))
    {
      videoStream.type = RS2_STREAM_COLOR;
    }

    // TODO: update width and height in subsession?
    ((camOERTSPClient*)rtspClient)->subsessionMap.insert(std::pair<int, MediaSubsession*>(videoStream.uid, scs.subsession));
    stream_counter++;
    ((camOERTSPClient*)rtspClient)->supportedProfiles.push_back(videoStream);
    scs.subsession = scs.iter->next();
    // TODO: when to delete p?
  }

    std::unique_lock<std::mutex> lck(command_mtx);
    cv.notify_one();

    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  // TODO: 
  //shutdownStream(rtspClient);
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  env << "continueAfterSETUP " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.notify_one();
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  env << "continueAfterPLAY " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.notify_one();

}

void continueAfterTEARDOWN(RTSPClient* rtspClient, int resultCode, char* resultString)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  env << "continueAfterTEARDOWN " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.notify_one();
}

void continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  env << "continueAfterPAUSE " << resultCode << " " << resultString <<"\n";
  ((camOERTSPClient*)rtspClient)->commandResultCode = resultCode;
  std::unique_lock<std::mutex> lck(command_mtx);
  cv.notify_one();
}