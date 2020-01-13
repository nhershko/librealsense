

#include "RsRawVideoRTPSink.h"

RsRawVideoRTPSink*
  RsRawVideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
        // The following headers provide the 'configuration' information, for the SDP description:
        unsigned height, unsigned width, unsigned depth,
        char const* sampling, char const* colorimetry)
        {
            return new RsRawVideoRTPSink(env, RTPgs, rtpPayloadFormat,
                 height, width, depth, sampling);

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
                  char const* sampling, char const* colorimetry)
  : RawVideoRTPSink(env, RTPgs, rtpPayloadFormat,
                 height, width, depth, sampling, colorimetry) {
    env << "RawVideoRTPSink constructor\n";

}

void RsRawVideoRTPSink::stopPlaying()
{
    //fCurFragmentationOffset = 0;
    RawVideoRTPSink::stopPlaying();

       
}