#include "RsSimpleRTPSink.h"

RsSimpleRTPSink*
  RsSimpleRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
	    unsigned char rtpPayloadFormat,
	    unsigned rtpTimestampFrequency,
	    char const* sdpMediaTypeString,
	    char const* rtpPayloadFormatName,
            rs2::video_stream_profile& video_stream, 
	    unsigned numChannels ,
	    Boolean allowMultipleFramesPerPacket ,
	    Boolean doNormalMBitRule )
        {
            return new RsSimpleRTPSink(env, RTPgs, rtpPayloadFormat,rtpTimestampFrequency,sdpMediaTypeString,rtpPayloadFormatName,
                 video_stream, numChannels, allowMultipleFramesPerPacket,doNormalMBitRule);

        }

RsSimpleRTPSink
::RsSimpleRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
	    unsigned char rtpPayloadFormat,
	    unsigned rtpTimestampFrequency,
	    char const* sdpMediaTypeString,
	    char const* rtpPayloadFormatName,
            rs2::video_stream_profile& video_stream,
	    unsigned numChannels ,
	    Boolean allowMultipleFramesPerPacket ,
	    Boolean doNormalMBitRule )
  :SimpleRTPSink(env, RTPgs, rtpPayloadFormat,rtpTimestampFrequency,sdpMediaTypeString,rtpPayloadFormatName,
                 numChannels, allowMultipleFramesPerPacket,doNormalMBitRule) {
    env << "RsRawVideoRTPSink constructor\n";
      // Then use this 'config' string to construct our "a=fmtp:" SDP line:
    unsigned fmtpSDPLineMaxSize = 200;// 200 => more than enough space
    fFmtpSDPLine = new char[fmtpSDPLineMaxSize];
    sprintf(fFmtpSDPLine, "a=fmtp:%d sampling=%s;depth=%u;width=%d;height=%d;format=%d;uid=%d;fps=%u;index=%d;stream_type=%d;colorimetry=%s\r\n",
        rtpPayloadType(), "0", "0", 
        video_stream.width(), video_stream.height(), video_stream.format(), video_stream.unique_id(), 
        video_stream.fps(), video_stream.stream_index(), video_stream.stream_type(), "0");
}

void RsSimpleRTPSink::stopPlaying()
{
    SimpleRTPSink::stopPlaying();       
}

char const* RsSimpleRTPSink::auxSDPLine() {
  return fFmtpSDPLine;
}
