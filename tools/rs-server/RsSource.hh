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
// A template for a MediaSource encapsulating an audio/video input device
//
// NOTE: Sections of this code labeled "%%% TO BE WRITTEN %%%" are incomplete, and needto be written by the programmer
// (depending on the features of the particulardevice).
// C++ header

#ifndef _RS_DEVICE_SOURCE_HH
#define _RS_DEVICE_SOURCE_HH

#ifndef _DEVICE_SOURCE_HH
#include "DeviceSource.hh"
#endif

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

#include <rs.hpp> // Include RealSense Cross Platform API
#include <mutex>
#include <condition_variable>
#include <compression/icompression.h>

class RsDeviceSource : public FramedSource
{
public:
  static RsDeviceSource *createNew(UsageEnvironment &env, rs2::video_stream_profile &video_stream_profile, rs2::frame_queue &queue);
  void handleWaitForFrame();
  static void waitForFrame(RsDeviceSource* deviceSource);
protected:
  RsDeviceSource(UsageEnvironment &env, rs2::video_stream_profile &video_stream_profile, rs2::frame_queue &queue);
  virtual ~RsDeviceSource();
private:
  virtual void doGetNextFrame();
 
  rs2::frame_queue* getFramesQueue(){return frames_queue;};
  //virtual void doStopGettingFrames(); // optional

private:
  void deliverRSFrame(rs2::frame *frame);

private:
  rs2::frame_queue *frames_queue;
  rs2::video_stream_profile *stream_profile;
#ifdef COMPRESSION
  ICompression * iCompress;
#endif
std::chrono::high_resolution_clock::time_point getFrame,gotFrame;
std::chrono::duration<double> networkTimeSpan,waitingTimeSpan,processingTimeSpan;
};

#endif
