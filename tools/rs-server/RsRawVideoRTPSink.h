#ifndef _RS_RAW_VIDEO_RTP_SINK_HH
#define _RS_RAW_VIDEO_RTP_SINK_HH

#include <librealsense2/hpp/rs_internal.hpp>

#include "RawVideoRTPSink.hh"

class RsRawVideoRTPSink : public RawVideoRTPSink
{
public:
  static RsRawVideoRTPSink*
  createNew(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
        // The following headers provide the 'configuration' information, for the SDP description:
        unsigned depth,
        rs2::video_stream_profile& video_stream, char const* sampling, char const* colorimetry = "BT709-2" );

protected:
  RsRawVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
                  u_int8_t rtpPayloadFormat,
                  unsigned depth,
                  rs2::video_stream_profile& video_stream, char const* sampling, char const* colorimetry= "BT709-2" );



protected: // redefined virtual functions:
  virtual void stopPlaying();

private:
  char* fFmtpSDPLine;
  char* fSampling;
  unsigned fWidth;
  unsigned fHeight;
  unsigned fDepth;
  char* fFormat;
  char* fColorimetry;
  virtual char const* auxSDPLine(); // for the "a=fmtp:" SDP line
  
};

#endif //_RS_RAW_VIDEO_RTP_SINK_HH
