#ifndef _RS_SIMPLE_RTP_SINK_HH
#define _RS_SIMPLE_RTP_SINK_HH

#include <librealsense2/hpp/rs_internal.hpp>
#include "SimpleRTPSink.hh"

class RsSimpleRTPSink : public SimpleRTPSink
{
public:
	static RsSimpleRTPSink *createNew(UsageEnvironment &env, Groupsock *RTPgs,
									  unsigned char rtpPayloadFormat,
									  unsigned rtpTimestampFrequency,
									  char const *sdpMediaTypeString,
									  char const *rtpPayloadFormatName,
									  rs2::video_stream_profile &video_stream,
									  unsigned numChannels = 1,
									  Boolean allowMultipleFramesPerPacket = True,
									  Boolean doNormalMBitRule = True);

protected:
	RsSimpleRTPSink(UsageEnvironment &env, Groupsock *RTPgs,
					unsigned char rtpPayloadFormat,
					unsigned rtpTimestampFrequency,
					char const *sdpMediaTypeString,
					char const *rtpPayloadFormatName,
					rs2::video_stream_profile &video_stream,
					unsigned numChannels = 1,
					Boolean allowMultipleFramesPerPacket = True,
					Boolean doNormalMBitRule = True);

private:
	char *fFmtpSDPLine;
	virtual char const *auxSDPLine(); // for the "a=fmtp:" SDP line
};

#endif //_RS_SIMPLE_RTP_SINK_HH
