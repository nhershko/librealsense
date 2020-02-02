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
// NOTE: Sections of this code labeled "%%% TO BE WRITTEN %%%" are incomplete, and need to be written by the programmer
// (depending on the features of the particular device).
// Implementation

#include "RsSource.hh"
#include <GroupsockHelper.hh>
#include <librealsense2/h/rs_sensor.h>
#include "BasicUsageEnvironment.hh"
#include "RsCommon.hh"
#include <cassert>
#include <compression/compression_factory.h>

    RsDeviceSource *
    RsDeviceSource::createNew(UsageEnvironment &env, rs2::video_stream_profile &video_stream_profile, rs2::frame_queue &queue)
{
  return new RsDeviceSource(env, video_stream_profile, queue);
}

RsDeviceSource::RsDeviceSource(UsageEnvironment &env, rs2::video_stream_profile &video_stream_profile, rs2::frame_queue &queue) : FramedSource(env)
{
  envir() << "RsDeviceSource constructor " << this << "\n";
  frames_queue = &queue;
  stream_profile = &video_stream_profile;
#ifdef COMPRESSION
  if (video_stream_profile.stream_type() == RS2_STREAM_COLOR || video_stream_profile.stream_type() == RS2_STREAM_INFRARED)
  {
    iCompress = CompressionFactory::create(zipMethod::Jpeg, video_stream_profile.width(), video_stream_profile.height(), video_stream_profile.format());
  }
  else if (video_stream_profile.stream_type() == RS2_STREAM_DEPTH)
  {
    iCompress = CompressionFactory::create(zipMethod::gzip, video_stream_profile.width(), video_stream_profile.height(), video_stream_profile.format());
  }
  else
  {
    envir() << "error: unsupported compression for this stream type\n";
  }
#endif
}

RsDeviceSource::~RsDeviceSource()
{
  envir() << "RsDeviceSource destructor " << this << "\n";
}

void RsDeviceSource::doGetNextFrame()
{
  // This function is called (by our 'downstream' object) when it asks for new data.
  // Note: If, for some reason, the source device stops being readable (e.g., it gets closed), then you do the following:
  if (0)
  { // the source stops being readable
    handleClosure();

    return;
  }
  // If a new frame of data is immediately available to be delivered, then do this now:
  rs2::frame frame;
  try
  {
    frame = frames_queue->wait_for_frame(); //todo: check if it copies the frame
    frame.keep();
    deliverRSFrame(&frame);
  }
  catch (const std::exception &e)
  {
    envir() << "RsDeviceSource: " << e.what() << '\n';
  }
}

void RsDeviceSource::deliverRSFrame(rs2::frame *frame)
{
  if (!isCurrentlyAwaitingData())
  {
    envir() << "isCurrentlyAwaitingData returned false"
            << "\n";
    return; // we're not ready for the data yet
  }

  unsigned newFrameSize = frame->get_data_size();

  gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
  rs_over_ethernet_data_header header;
#ifdef COMPRESSION
  fFrameSize = iCompress->compressBuffer((unsigned char *)frame->get_data(), frame->get_data_size(), fTo + sizeof(header));
#else
  fFrameSize = frame->get_data_size();
  memmove(fTo + sizeof(header), frame->get_data(), fFrameSize);
#endif

  header.size = fFrameSize;
  fFrameSize += sizeof(header);
  memmove(fTo, &header, sizeof(header));
  assert(fMaxSize > fFrameSize); //TODO: to remove on release
  // After delivering the data, inform the reader that it is now available:
  FramedSource::afterGetting(this);
}
