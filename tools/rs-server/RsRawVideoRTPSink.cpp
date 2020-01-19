

#include "RsRawVideoRTPSink.h"

RsRawVideoRTPSink*
  RsRawVideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
        // The following headers provide the 'configuration' information, for the SDP description:
        unsigned depth,
        rs2::video_stream_profile& video_stream, char const* sampling, char const* colorimetry)
        {
            return new RsRawVideoRTPSink(env, RTPgs, rtpPayloadFormat,
                 depth, video_stream, sampling, colorimetry);
        }

RsRawVideoRTPSink
::RsRawVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
                  unsigned depth,
                  rs2::video_stream_profile& video_stream, char const* sampling, char const* colorimetry)
  : RawVideoRTPSink(env, RTPgs, rtpPayloadFormat,
                 video_stream.height(), video_stream.width(), depth, sampling, colorimetry) {
    env << "RsRawVideoRTPSink constructor\n";
      // Then use this 'config' string to construct our "a=fmtp:" SDP line:
    unsigned fmtpSDPLineMaxSize = 200;// 200 => more than enough space
    fFmtpSDPLine = new char[fmtpSDPLineMaxSize];
    sprintf(fFmtpSDPLine, "a=fmtp:%d sampling=%s;depth=%u;width=%d;height=%d;format=%d;uid=%d;fps=%u;index=%d;colorimetry=%s\r\n",
        rtpPayloadType(), sampling, depth, 
        video_stream.width(), video_stream.height(), video_stream.format(), video_stream.unique_id(), 
        video_stream.fps(), video_stream.stream_index(), colorimetry);

    // Set parameters
    fSampling = strDup(sampling);
    fColorimetry = strDup(colorimetry);
}

void RsRawVideoRTPSink::stopPlaying()
{
    //fCurFragmentationOffset = 0;
    RawVideoRTPSink::stopPlaying();       
}

char const* RsRawVideoRTPSink::auxSDPLine() {
  return fFmtpSDPLine;
}
