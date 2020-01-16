#ifndef CAM_OE_SINK_H
#define CAM_OE_SINK_H

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include <functional>
#include "rtp_callback.hh"


class camOESink : public MediaSink {
public:
  static camOESink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession,
                  int bufferSize, // identifies the kind of data that's being received
			      char const* streamId = NULL); // identifies the stream itself (optional)
    
    void set_callback(rtp_callback* callback);

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
  
  rtp_callback* m_rtp_callback;
};

#endif // CAM_OE_SINK_H
