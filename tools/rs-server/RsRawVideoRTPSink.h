#ifndef _RS_RAW_VIDEO_RTP_SINK_HH
#define _RS_RAW_VIDEO_RTP_SINK_HH

#include "RawVideoRTPSink.hh"


class RsRawVideoRTPSink: public RawVideoRTPSink {
public:
  static RsRawVideoRTPSink*
  createNew(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
        // The following headers provide the 'configuration' information, for the SDP description:
        unsigned height, unsigned width, unsigned depth,
        char const* sampling, char const* colorimetry = "BT709-2" );

protected:
  RsRawVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
                  u_int8_t rtpPayloadFormat,
                  unsigned height, unsigned width, unsigned depth,
                  char const* sampling, char const* colorimetry= "BT709-2" );



protected: // redefined virtual functions:
  virtual void stopPlaying();
  
};


#endif //_RS_RAW_VIDEO_RTP_SINK_HH
