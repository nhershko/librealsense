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
#include "compressFrameFactory.h"
#include <map>

RsDeviceSource *
RsDeviceSource::createNew(UsageEnvironment &env, rs2::video_stream_profile &video_stream_profile, rs2::frame_queue &queue)
{
  return new RsDeviceSource(env, video_stream_profile, queue);
}

RsDeviceSource::RsDeviceSource(UsageEnvironment &env, rs2::video_stream_profile &video_stream_profile, rs2::frame_queue &queue): FramedSource(env)
{
  //envir() << "RsDeviceSource constructor\n";
  frames_queue = &queue;
  stream_profile = &video_stream_profile;
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
  rs2::frame frame=frames_queue->wait_for_frame(); //todo: check if it copies the frame
  frame.keep();

  deliverRSFrame(&frame);
}

void RsDeviceSource::deliverRSFrame(rs2::frame *frame)
{
#ifdef COMPRESSION
  IcompressFrame* iCompressColor =  compressFrameFactory::create(zipMethod::gzip);
  IcompressFrame* iCompressDepth =  compressFrameFactory::create(zipMethod::gzip);
#endif
  if (!isCurrentlyAwaitingData())
  {
    envir() << "isCurrentlyAwaitingData returned false" << "\n";
    return; // we're not ready for the data yet
  }

  unsigned newFrameSize = frame->get_data_size();

  if (newFrameSize > fMaxSize)
  {
    fFrameSize = fMaxSize;
    fNumTruncatedBytes = newFrameSize - fMaxSize;
  }
  else
  {
    fFrameSize = newFrameSize;
  }
  gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
#ifdef COMPRESSION
   if(stream_profile->stream_type() == RS2_STREAM_DEPTH) 
   {
      iCompressDepth->compressDepthFrame((unsigned char*)frame->get_data(), fFrameSize, fTo);
   } else if(stream_profile->stream_type() == RS2_STREAM_COLOR) 
   {
      iCompressColor->compressColorFrame((unsigned char*)frame->get_data(), fFrameSize, fTo);
   } else {
#endif
    //envir() << "got new frame: frame size is " << fFrameSize <<  "stream type is is " << stream_profile->stream_type() << "stream resolution is" <<  stream_profile->width() << "," << stream_profile->height() << "\n";
    memmove(fTo, frame->get_data(), fFrameSize);
    //envir() << "after memove frame \n";
#ifdef COMPRESSION
   }
#endif
  // After delivering the data, inform the reader that it is now available:
  FramedSource::afterGetting(this);
}

