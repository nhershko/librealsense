

#include "RsRawVideoRTPSink.h"

RsRawVideoRTPSink*
  RsRawVideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
        // The following headers provide the 'configuration' information, for the SDP description:
        unsigned height, unsigned width, unsigned depth,
         unsigned int format, char const* format_str, char const* sampling, char const* colorimetry)
        {
            return new RsRawVideoRTPSink(env, RTPgs, rtpPayloadFormat,
                 height, width, depth, format, format_str, sampling, colorimetry);

        }


  
/*RawVideoRTPSink* RawVideoRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
        unsigned height, unsigned width, unsigned depth,
        char const* sampling, char const* colorimetry) {
  return new RawVideoRTPSink(env, RTPgs,
                             rtpPayloadFormat,
                             height, width, depth,
                             sampling, colorimetry);
}*/

RsRawVideoRTPSink
::RsRawVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
                  unsigned height, unsigned width, unsigned depth,
                  unsigned int format, char const* format_str, char const* sampling, char const* colorimetry)
  : RawVideoRTPSink(env, RTPgs, rtpPayloadFormat,
                 height, width, depth, sampling, colorimetry) {
    env << "RsRawVideoRTPSink constructor\n";
      // Then use this 'config' string to construct our "a=fmtp:" SDP line:
    unsigned fmtpSDPLineMaxSize = 200;// 200 => more than enough space
    fFmtpSDPLine = new char[fmtpSDPLineMaxSize];
    sprintf(fFmtpSDPLine, "a=fmtp:%d sampling=%s;width=%u;height=%u;depth=%u;rs_format=%u;format_str=%s;colorimetry=%s\r\n",
        rtpPayloadType(), sampling, width, height, depth, format, format_str, colorimetry);

    // Set parameters
    fSampling = strDup(sampling);
    fColorimetry = strDup(colorimetry);
    fFormat = strDup(format_str);
}

/*RsRawVideoRTPSink :: ~RsRawVideoRTPSink()
{
    envir() << "RsRawVideoRTPSink destructor " <<this <<"\n";
}*/

void RsRawVideoRTPSink::stopPlaying()
{
    //fCurFragmentationOffset = 0;
    RawVideoRTPSink::stopPlaying();       
}

char const* RsRawVideoRTPSink::auxSDPLine() {
  return fFmtpSDPLine;
}
