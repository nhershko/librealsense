#ifndef _CAM_OE_RTP_SINK_H
#define _CAM_OE_RTP_SINK_H

#include "RawVideoRTPSink.hh"

class camOERTPSink: public RawVideoRTPSink
{
public:
    static camOERTPSink*
    createNew(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
            // The following headers provide the 'configuration' information, for the SDP description:
            unsigned height, unsigned width, unsigned depth,
            char const* sampling, char const* colorimetry = "BT709-2");

    protected:
    camOERTPSink(UsageEnvironment& env, Groupsock* RTPgs,
                    u_int8_t rtpPayloadFormat,
                    unsigned height, unsigned width, unsigned depth,
                    char const* sampling, char const* colorimetry = "BT709-2");
    // called only by createNew()
    
    virtual ~camOERTPSink();
        
};

#endif // CAM_OE_RTP_SINK_H