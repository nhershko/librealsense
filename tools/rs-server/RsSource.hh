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

// The following class can be used to define specific encoder parameters
class RSDeviceParameters : public DeviceParameters
{
public:
  RSDeviceParameters() //:DeviceParameters()
  {
  }
  RSDeviceParameters(int width, int height, int bitPerPixel, int streamId, int sensorId)
  {
    w = width;
    h = height;
    bpp = bitPerPixel;
    streamID = streamId;
    sensorID = sensorId;
  }
  RSDeviceParameters(RSDeviceParameters &params) //:DeviceParameters()
  {
    w = params.w;
    h = params.h;
    bpp = params.bpp;
    streamID = params.streamID;
    sensorID = params.sensorID;
  }

  //private:
  int w, h, bpp, streamID, sensorID;
};

class RsDeviceSource : public FramedSource
{
public:
  static RsDeviceSource *createNew(UsageEnvironment &env, RSDeviceParameters deviceParams, rs2::device selectedDevice);

public:
  RSDeviceParameters getParams() { return fParams; };

protected:
  RsDeviceSource(UsageEnvironment &env, RSDeviceParameters deviceParams, rs2::device selectedDevice);
  virtual ~RsDeviceSource();

private:
  virtual void doGetNextFrame();
  //virtual void doStopGettingFrames(); // optional
  void frameCallback(rs2::frame frame);

private:
  static void deliverRSFrame0(void *clientData);
  void deliverRSFrame();

private:
  RSDeviceParameters fParams;
  rs2::frame frame;
  rs2::device selected_device;
  rs2::sensor selected_sensor;
  std::mutex m;
  std::condition_variable cv;
  bool isWaitingFrame;
};

#endif
