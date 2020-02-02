#include "RsSimpleRTPSink.h"
#include <iostream>
#include <string>
#include <sstream>

RsSimpleRTPSink *
RsSimpleRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs,
                           unsigned char rtpPayloadFormat,
                           unsigned rtpTimestampFrequency,
                           char const *sdpMediaTypeString,
                           char const *rtpPayloadFormatName,
                           rs2::video_stream_profile &video_stream,
                           unsigned numChannels,
                           Boolean allowMultipleFramesPerPacket,
                           Boolean doNormalMBitRule)
{
  return new RsSimpleRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, sdpMediaTypeString, rtpPayloadFormatName,
                             video_stream, numChannels, allowMultipleFramesPerPacket, doNormalMBitRule);
}

// TODO Michal: oveload with other types if needed
std::string getSdpLineFOrField(const char* name, int val)
{
  std::ostringstream oss;
  oss << name << "=" << val << ";";
  return oss.str();
}

std::string getSdpLineForVideoStream(rs2::video_stream_profile &video_stream)
{
  std::string str;
  str.append(getSdpLineFOrField("width", video_stream.width()));
  str.append(getSdpLineFOrField("height", video_stream.height()));
  str.append(getSdpLineFOrField("format", video_stream.format()));
  str.append(getSdpLineFOrField("uid", video_stream.unique_id()));
  str.append(getSdpLineFOrField("fps", video_stream.fps()));
  str.append(getSdpLineFOrField("stream_index", video_stream.stream_index()));
  str.append(getSdpLineFOrField("stream_type", video_stream.stream_type()));
  //str.append(getSdpLineFOrField("is_compressed", 0));
  return str;
}


RsSimpleRTPSink ::RsSimpleRTPSink(UsageEnvironment &env, Groupsock *RTPgs,
                                  unsigned char rtpPayloadFormat,
                                  unsigned rtpTimestampFrequency,
                                  char const *sdpMediaTypeString,
                                  char const *rtpPayloadFormatName,
                                  rs2::video_stream_profile &video_stream,
                                  unsigned numChannels,
                                  Boolean allowMultipleFramesPerPacket,
                                  Boolean doNormalMBitRule)
    : SimpleRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, sdpMediaTypeString, rtpPayloadFormatName,
                    numChannels, allowMultipleFramesPerPacket, doNormalMBitRule)
{
  env << "RsRawVideoRTPSink constructor\n";
  // Then use this 'config' string to construct our "a=fmtp:" SDP line:
  unsigned fmtpSDPLineMaxSize = 200; // 200 => more than enough space
  fFmtpSDPLine = new char[fmtpSDPLineMaxSize];
  std::string sdpStr =  getSdpLineForVideoStream(video_stream);
  sprintf(fFmtpSDPLine, "a=fmtp:%d;%s\r\n",
          rtpPayloadType(),
          sdpStr.c_str());
}

char const *RsSimpleRTPSink::auxSDPLine()
{
  return fFmtpSDPLine;
}