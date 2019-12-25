#include "camOERtspClient.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

// TODO: change to lrs class
#include "Profile.h"

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

std::condition_variable cv;
std::mutex describe_mtx;
bool describe_done = false;

// Forward function definitions:

// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
void setupSubsession(MediaSubsession* subsession, RTSPClient* rtspClient);

void camOERTSPClient::sendDescribe()
{
    this->sendDescribeCommand(continueAfterDESCRIBE);
    this->envir() << "in sendDescribe after sending command\n";
    // wait for continueAfterDESCRIBE to finish
    std::unique_lock<std::mutex> lck(describe_mtx);
    cv.wait(lck); 

    this->envir() << "in sendDescribe After wait\n"; 
}

std::vector<Profile> camOERTSPClient::queryProfiles()
{
    this->sendDescribe();  
    return this->supportedProfiles;
}
int camOERTSPClient::addProfile(Profile)
{

}
void camOERTSPClient::start()
{

}
void camOERTSPClient::stop()
{

}
void camOERTSPClient::close()
{

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

  scs.iter = new MediaSubsessionIterator(*scs.session);
  scs.subsession = scs.iter->next();
  while (scs.subsession != NULL) {
    const char* strWidthVal = scs.subsession->attrVal_str("width");
    const char* strHeightVal = scs.subsession->attrVal_str("height");
    int width = strWidthVal != NULL ? std::stoi(strWidthVal) : 0;
    int height = strHeightVal != NULL ? std::stoi(strHeightVal) : 0;
    Profile* p = new Profile(height,width);
    ((camOERTSPClient*)rtspClient)->supportedProfiles.push_back(*p);
    scs.subsession = scs.iter->next();
    // TODO: when to delete p?
  }

    std::unique_lock<std::mutex> lck(describe_mtx);
    cv.notify_one();

    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  // TODO: 
  //shutdownStream(rtspClient);
}

void setupSubsession(MediaSubsession* subsession, RTSPClient* rtspClient)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  env <<  "Setting up subsession:  " << subsession->codecName() <<  "\n"; 
  rtspClient->sendSetupCommand(*subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  env << "continueAfterSETUP" << "\n";
}