#ifndef CAM_OE_SINK_H
#define CAM_OE_SINK_H

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

// TODO: think where to put this declaration 
typedef void (*frame_call_back)(u_int8_t*, unsigned int, struct timeval);

class camOESink : public MediaSink {
public:
  static camOESink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession,
                  int bufferSize, // identifies the kind of data that's being received
			      char const* streamId = NULL); // identifies the stream itself (optional)
    void setFrameCallback(frame_call_back callback);

private:
  camOESink(UsageEnvironment& env, MediaSubsession& subsession, int bufferSize, char const* streamId);
    // called only by "createNew()"
  virtual ~camOESink();

  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
				struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);

private:
  // redefined virtual functions:
  virtual Boolean continuePlaying();

private:
  u_int8_t* fReceiveBuffer;
  int fBufferSize;
  MediaSubsession& fSubsession;
  char* fStreamId;
  FILE* fp;
  frame_call_back fFrameCallBack;
};

#endif // CAM_OE_SINK_H
