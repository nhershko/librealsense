#include "camOERtspClient.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#include "Profile.h"

#include <iostream>
#include <thread>
#include <condition_variable>
#include <vector>
#include <string>

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"
#define REQUEST_STREAMING_OVER_TCP 0 // TODO - uderstand this

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

// // TODO: move to seperate files
// class StreamClientState {
// public:
//   StreamClientState();
//   virtual ~StreamClientState();

// public:
//   MediaSubsessionIterator* iter;
//   MediaSession* session;
//   MediaSubsession* subsession;
//   TaskToken streamTimerTask;
//   double duration;
// };

// IcamOERtsp* rtspClient;

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

// the rtsp client can be deleted from the function
 //typedef void (*DESCRIBECallback) (StreamClientState* scs, RTSPClient* rtspClient);
 //typedef void (*SETUPCallback) (int status);

 //DESCRIBECallback appDescribeCallback;
 //SETUPCallback appSetupCallback;


//TaskScheduler* scheduler;
//UsageEnvironment* env;

std::condition_variable cv;
std::mutex describe_mtx;
bool describe_done = false;

// Forward function definitions:

// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
void setupSubsession(MediaSubsession* subsession, RTSPClient* rtspClient);

// TODO: should be in client-app
/*void describeCb(StreamClientState* scs, RTSPClient* rtspClient)
{
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

  *env << "describeCb " << "subsession: " << scs->session->hasSubsessions() << "\n"; 

  // Should get the subsession from the client-app
  MediaSubsession* subsessionToSend = ((camOERTSPClient*)rtspClient)->scs.iter->next();
  *env << "Subsession width: " << subsessionToSend->videoWidth() << "\n";
  *env << "Subsession port: " << subsessionToSend->serverPortNum << "\n";
  setupSubsession(subsessionToSend, rtspClient);
}*/

/*void setupCb(int status)
{
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

  *env << "setupCb " << "status: " << status << "\n"; 
}*/

void camOERTSPClient::sendDescribe()
{
    //camOERTSPClient* camOERtspClient = (camOERTSPClient*)rtspClient;
    this->sendDescribeCommand(continueAfterDESCRIBE);
    this->envir() << "in sendDescribe after sending command\n";
    // wait for continueAfterDESCRIBE to finish
    std::unique_lock<std::mutex> lck(describe_mtx);
    cv.wait(lck); 
   // while(describe_done== false)
   // {sleep(1);}
    // describe_done = false; 

    this->envir() << "in sendDescribe After wait\n"; 
}

std::vector<Profile> camOERTSPClient::queryProfiles()
{
    //camOERTSPClient* camOERtspClient = (camOERTSPClient*)rtspClient;
    // TODO: move the init func to the getRtspClient func
    //this->initFunc();
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

/*std::vector<Profile> queryProfiles(char* url)
{
  
 rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
  // wait for continueAfterDESCRIBE to finish
  std::unique_lock<std::mutex> lck(mtx);
  cv.wait(lck);

  return rtspClient->supportedProfiles;
}*/

void schedulerThread(camOERTSPClient* rtspClientInstance)
{
    //camOERTSPClient* camOERtspClient = (camOERTSPClient*)rtspClient;
   // TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    //UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
    rtspClientInstance->envir() << "in schedulerThread!\n";
    char eventLoopWatchVariable = 0;
    rtspClientInstance->envir().taskScheduler().doEventLoop(&eventLoopWatchVariable);
}

void camOERTSPClient::initFunc(/*DESCRIBECallback describeCallBack, SETUPCallback setupCallBack*/)
{
  // Begin by setting up our usage environment:
  //scheduler = BasicTaskScheduler::createNew();
  //env = BasicUsageEnvironment::createNew(*scheduler);

  // Create a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
  // to receive (even if more than stream uses the same "rtsp://" URL).
  // const char rtspURL[] = "rtsp://10.12.145.82:8554/testStream";
  // const char appName[] = "camOErtspClient";
  // rtspClient = camOERTSPClient::createNew(rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, appName);
  // if (rtspClient == NULL) {
  //   std::cout << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " /*<< *env.getResultMsg()*/ << "\n";
  //   return;
  // }

  //appDescribeCallback = describeCallBack;
  //appSetupCallback = setupCallBack;
   std::thread thread_scheduler(schedulerThread, this);
   thread_scheduler.detach();
}

void openURL(/*RTSPClient* rtspClient*/) {
  // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
  // to receive (even if more than stream uses the same "rtsp://" URL).
 /* RTSPClient* rtspClient = RTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
  if (rtspClient == NULL) {
    env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
    return;
  }*/

  // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
  // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
  // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
  // rtspClient->sendDescribeCommand(continueAfterDESCRIBE); 
}

// TODO: Error handling
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((camOERTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << "Failed to get a SDP description: " << resultString << "\n";
      //std::cout << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      delete[] resultString;
      break;
    }

    char* const sdpDescription = resultString;
    env << "Got a SDP description:\n" << sdpDescription << "\n";
    // std::cout << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      //std::cout << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      //std::cout << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }


    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.

    // Should get the subsession from the client-app
  /*MediaSubsession* subsessionToSend = ((camOERTSPClient*)rtspClient)->scs.iter->next();
  env << "Subsession width: " << subsessionToSend->videoWidth() << "\n";
  env << "Subsession hieght: " << subsessionToSend->videoWidth() << "\n";*/
  //setupSebsession(subsessionToSend, rtspClient);
  
  
  //appDescribeCallback(&scs, rtspClient);
  
  // signal describe thread

  // convert subsessions to profiles
  // set rtspClient.profiles
  scs.iter = new MediaSubsessionIterator(*scs.session);
scs.subsession = scs.iter->next();
  while (scs.subsession != NULL) {
    // TODO: what is this?
    //if (!scs.subsession->initiate()) {
    const char* strWidthVal = scs.subsession->attrVal_str("width");
    const char* strHeightVal = scs.subsession->attrVal_str("height");
    int width = strWidthVal != NULL ? std::stoi(strWidthVal) : 0;
    int height = strHeightVal != NULL ? std::stoi(strHeightVal) : 0;
    Profile* p1 = new Profile(height,width);
    //Profile* p2 = new Profile(444,555);
    ((camOERTSPClient*)rtspClient)->supportedProfiles.push_back(*p1);
    //((camOERTSPClient*)rtspClient)->supportedProfiles.push_back(*p2);
    scs.subsession = scs.iter->next();
  }

    env << "Notify!!!!!\n";
    std::unique_lock<std::mutex> lck(describe_mtx);
    cv.notify_one();

    //env << "Setting describe_done to true!!!\n";
    //describe_done = true;
    
    //setupNextSubsession(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  // TODO: 
  //shutdownStream(rtspClient);
}

void setupSubsession(MediaSubsession* subsession, RTSPClient* rtspClient)
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  // Continue setting up this subsession, by sending a RTSP "SETUP" command:
  env <<  "Setting up subsession:  " << subsession->codecName() <<  "\n"; 
  rtspClient->sendSetupCommand(*subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  env << "continueAfterSETUP" << "\n";
  //appSetupCallback(resultCode);

}

void describeThread()
{
  //rtspClient->envir() << "in describeThread!\n";
  //rtspClient->sendDescribe();
  //std::unique_lock<std::mutex> lck(mtx);
  //cv.wait(lck, []{return describe_done;});   
  // busy wait
  // while(describe_done== false)
  // {}
  // describe_done = false;
}

#if 0
 char eventLoopWatchVariable = 0;
 int main(int argc, char** argv) {
   // Begin by setting up our usage environment:
   // TaskScheduler* scheduler = BasicTaskScheduler::createNew();
   // UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

   // appDescribeCallback = &describeCb;
   // appSetupCallback = &setupCb;

   // // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
   // // to receive (even if more than stream uses the same "rtsp://" URL).
   // const char rtspURL[] = "rtsp://10.12.145.82:8554/testStream";
   // const char appName[] = "camOEClient";
   // RTSPClient* rtspClient = RTSPClient::createNew(*env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, appName);
   // if (rtspClient == NULL) {
   //   *env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " /*<< *env.getResultMsg()*/ << "\n";
   //   return 1;
   // }
   IcamOERtsp* rtspClient;
   const char rtspURL[] = "rtsp://10.12.145.82:8554/testStream";
   const char appName[] = "camOErtspClient";
   rtspClient = camOERTSPClient::getRtspClient(rtspURL, appName);
   if (rtspClient == NULL) {
     std::cout << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " /*<< *env.getResultMsg()*/ << "\n";
     return 1;
   }

   //initRtspClient(/*describeCb, setupCb*/);
   //((camOERTSPClient*)rtspClient)->initFunc();
   //getProfiles();

   //openURL(/*rtspClient*/);
   //openURL(*env, "camOEClient", "rtsp://10.12.145.82:8554/");
  // }

   //((camOERTSPClient*)rtspClient)->sendDescribe();
   rtspClient->queryProfiles();

 /*
   // Should get the subsession from the client-app
   MediaSubsession* subsessionToSend = ((camOERTSPClient*)rtspClient)->scs.iter->next();
   *env << "Subsession width: " << subsessionToSend->videoWidth() << "\n";
   *env << "Subsession hieght: " << subsessionToSend->videoWidth() << "\n";
   setupSebsession(subsessionToSend, rtspClient);*/

  // All subsequent activity takes place within the event loop:
   //env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
     //std::thread thread_describe(describeThread);
    
     // std::thread thread_scheduler(schedulerThread);
     // thread_scheduler.detach();
     //thread_describe.join();

    // This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.

  return 0;

//   // If you choose to continue the application past this point (i.e., if you comment out the "return 0;" statement above),
//   // and if you don't intend to do anything more with the "TaskScheduler" and "UsageEnvironment" objects,
//   // then you can also reclaim the (small) memory used by these objects by uncommenting the following code:
//   /*
//     env->reclaim(); env = NULL;
//     delete scheduler; scheduler = NULL;
//   */
 }
 #endif