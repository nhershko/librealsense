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

RsDeviceSource *
RsDeviceSource::createNew(UsageEnvironment &env, RSDeviceParameters deviceParams, rs2::device selectedDevice)
{
  return new RsDeviceSource(env, deviceParams, selectedDevice);
}

RsDeviceSource::RsDeviceSource(UsageEnvironment &env, RSDeviceParameters deviceParams, rs2::device selectedDevice)
    : FramedSource(env), fParams(deviceParams)
{

  isWaitingFrame = false;
  selected_device = selectedDevice;

  std::vector<rs2::sensor> sensors = selected_device.query_sensors();
  if (sensors.size() > deviceParams.sensorID)
  {
    auto frameCallback = [&](const rs2::frame &f) {
      frame = f;
      std::lock_guard<std::mutex> lk(m);
      isWaitingFrame = true;
      cv.notify_one();
    };

    selected_sensor = sensors[deviceParams.sensorID];
    std::vector<rs2::stream_profile> stream_profiles = selected_sensor.get_stream_profiles();
    selected_sensor.open(stream_profiles[deviceParams.streamID]);
    selected_sensor.start(frameCallback);
  }
  else
  {
    envir() << "failed to create device \n";
    return;
  }
}

RsDeviceSource::~RsDeviceSource()
{
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
  std::unique_lock<std::mutex> lk(m);
  cv.wait(lk, [&] { return isWaitingFrame == true; });
  deliverRSFrame();
}

void RsDeviceSource::deliverRSFrame()
{
  if (!isCurrentlyAwaitingData())
  {
    envir() << "isCurrentlyAwaitingData returned false"<<fParams.sensorID<<"\n";
    return; // we're not ready for the data yet
  }
  isWaitingFrame = false;
  u_int8_t *newFrameDataStart = (u_int8_t *)frame.get_data();
  //int size = frame.get_data_size();
  //envir() << "frame size is "<<size<<"\n";
  unsigned newFrameSize = fParams.w * fParams.h * fParams.bpp;

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
  memmove(fTo, frame.get_data(), fFrameSize);
  // After delivering the data, inform the reader that it is now available:
  FramedSource::afterGetting(this);
}
