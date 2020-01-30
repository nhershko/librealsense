#ifndef CAM_OE_SINK_H
#define CAM_OE_SINK_H

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "rtp_callback.hh"
#include <librealsense2/hpp/rs_internal.hpp>
#include <compression/compression_factory.h>

class camOESink : public MediaSink {
public:
  static camOESink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession,
                  rs2_video_stream stream, // identifies the kind of data that's being received
			      char const* streamId = NULL); // identifies the stream itself (optional)
    
    void set_callback(rtp_callback* callback);

private:
  camOESink(UsageEnvironment& env, MediaSubsession& subsession, rs2_video_stream stream, char const* streamId);
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
  u_int8_t* fto;
  int fBufferSize;
  MediaSubsession& fSubsession;
  char* fStreamId;
  FILE* fp;
  
  rtp_callback* m_rtp_callback;
  rs2_video_stream fstream; 
  ICompression *iCompress;
};

#endif // CAM_OE_SINK_H
