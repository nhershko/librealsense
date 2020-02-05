#include "RsSimpleRTPSink.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

RsSimpleRTPSink *
RsSimpleRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs,
                           unsigned char rtpPayloadFormat,
                           unsigned rtpTimestampFrequency,
                           char const *sdpMediaTypeString,
                           char const *rtpPayloadFormatName,
                           rs2::video_stream_profile &video_stream,
                           // TODO Michal: this is a W/A for passing the sensor's metadata
                           RsDevice &device,
                           unsigned numChannels,
                           Boolean allowMultipleFramesPerPacket,
                           Boolean doNormalMBitRule)
{
  return new RsSimpleRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, sdpMediaTypeString, rtpPayloadFormatName,
                             video_stream, device, numChannels, allowMultipleFramesPerPacket, doNormalMBitRule);
}

// TODO Michal: oveload with other types if needed
std::string getSdpLineForField(const char* name, int val)
{
  std::ostringstream oss;
  oss << name << "=" << val << ";";
  return oss.str();
}

std::string getSdpLineForField(const char* name, const char* val)
{
  std::ostringstream oss;
  oss << name << "=" << val << ";";
  return oss.str();
}

std::string getSdpLineForVideoStream(rs2::video_stream_profile &video_stream, RsDevice &device)
{
  std::string str;
  str.append(getSdpLineForField("width", video_stream.width()));
  str.append(getSdpLineForField("height", video_stream.height()));
  str.append(getSdpLineForField("format", video_stream.format()));
  str.append(getSdpLineForField("uid", video_stream.unique_id()));
  str.append(getSdpLineForField("fps", video_stream.fps()));
  str.append(getSdpLineForField("stream_index", video_stream.stream_index()));
  str.append(getSdpLineForField("stream_type", video_stream.stream_type()));
  str.append(getSdpLineForField("bpp", RsSensor::getStreamProfileBpp(video_stream.format())));
  rs2::device rs_device = device.getRs2Device();
  str.append(getSdpLineForField("cam_serial_num", rs_device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)));
  str.append(getSdpLineForField("usb_type", rs_device.get_info(RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR)));
  std::string name = rs_device.get_info(RS2_CAMERA_INFO_NAME);
  // We don't want to sent spaces over SDP
  // TODO Michal: Decide what character to use for replacing spaces
  std::replace(name.begin(), name.end(), ' ', '^');
  str.append(getSdpLineForField("cam_name", name.c_str()));

  return str;
}


RsSimpleRTPSink ::RsSimpleRTPSink(UsageEnvironment &env, Groupsock *RTPgs,
                                  unsigned char rtpPayloadFormat,
                                  unsigned rtpTimestampFrequency,
                                  char const *sdpMediaTypeString,
                                  char const *rtpPayloadFormatName,
                                  rs2::video_stream_profile &video_stream,
                                  RsDevice &device,
                                  unsigned numChannels,
                                  Boolean allowMultipleFramesPerPacket,
                                  Boolean doNormalMBitRule)
    : SimpleRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, sdpMediaTypeString, rtpPayloadFormatName,
                    numChannels, allowMultipleFramesPerPacket, doNormalMBitRule)
{
  env << "RsSimpleVideoRTPSink constructor\n";
  // Then use this 'config' string to construct our "a=fmtp:" SDP line:
  unsigned fmtpSDPLineMaxSize = 200; // 200 => more than enough space
  fFmtpSDPLine = new char[fmtpSDPLineMaxSize];
  std::string sdpStr =  getSdpLineForVideoStream(video_stream, device);
  sprintf(fFmtpSDPLine, "a=fmtp:%d;%s\r\n",
          rtpPayloadType(),
          sdpStr.c_str());
}

char const *RsSimpleRTPSink::auxSDPLine()
{
  return fFmtpSDPLine;
}
