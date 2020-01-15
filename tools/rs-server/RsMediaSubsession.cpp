/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2019 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a file.
// Implementation
#include "RsMediaSubsession.h"
#include "RsRawVideoRTPSink.h"
//#include <librealsense2/h/rs_sensor.h>

#define CAPACITY 100

RsMediaSubsession *RsMediaSubsession::createNew(UsageEnvironment &env, rs2::video_stream_profile &video_stream_profile /*, rs2::frame_queue &queue*/)
{
  return new RsMediaSubsession(env, video_stream_profile /*, queue*/);
}

RsMediaSubsession ::RsMediaSubsession(UsageEnvironment &env, rs2::video_stream_profile &video_stream_profile /*, rs2::frame_queue &queue*/)
    : OnDemandServerMediaSubsession(env, false), videoStreamProfile(video_stream_profile) /*,frameQueue(queue)*/
{
  envir() << "RsMediaSubsession constructor" <<this << "\n";
  frameQueue = rs2::frame_queue(CAPACITY, true);
}

RsMediaSubsession::~RsMediaSubsession() {
  envir() << "RsMediaSubsession destructor" <<this << "\n";
    //TODO:: free the queue
}

rs2::frame_queue &RsMediaSubsession::get_frame_queue()
{
  return frameQueue;
}

rs2::video_stream_profile RsMediaSubsession::get_stream_profile()
{
  return videoStreamProfile;
}

FramedSource *RsMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned &estBitrate)
{
  estBitrate = 5000; // kbps, estimate //TODO:: to calculate the right value
  return RsDeviceSource::createNew(envir(), videoStreamProfile, frameQueue);
}

RTPSink *RsMediaSubsession ::createNewRTPSink(Groupsock *rtpGroupsock,
                                              unsigned char rtpPayloadTypeIfDynamic,
                                              FramedSource * /*inputSource*/)
{
  switch (videoStreamProfile.format())
  {
  case RS2_FORMAT_RGB8:
  {
    pixelSize = 3;
    return RsRawVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, videoStreamProfile.height(), videoStreamProfile.width(), 8, "RGB");
  }
  case RS2_FORMAT_BGR8:
  {
    pixelSize = 3;
    return RsRawVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, videoStreamProfile.height(), videoStreamProfile.width(), 8, "BGR");
  }
  case RS2_FORMAT_RGBA8:
  {
    pixelSize = 3;
    return RsRawVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, videoStreamProfile.height(), videoStreamProfile.width(), 8, "RGBA");
  }
  case RS2_FORMAT_BGRA8:
  {
    pixelSize = 3;
    return RsRawVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, videoStreamProfile.height(), videoStreamProfile.width(), 8, "BGRA");
  }
  case RS2_FORMAT_Z16:
  case RS2_FORMAT_Y16:
  case RS2_FORMAT_RAW16:
  case RS2_FORMAT_YUYV:
  {
    pixelSize = 2;
    return RsRawVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, videoStreamProfile.height(), videoStreamProfile.width(), 8, "YCbCr-4:2:2");
  }
  default:
    pixelSize = 0;
    break;
  }
  return NULL;
}
