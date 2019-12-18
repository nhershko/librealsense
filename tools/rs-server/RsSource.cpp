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

// unsigned char fbuf[640*480*2] = {0};
// bool first_frame = true;

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

  std::vector<rs2::sensor> sensors = selectedDevice.query_sensors();
  if (sensors.size() > deviceParams.sensorID)
  {
    auto frameCallback = [&](const rs2::frame &f) {
      std::lock_guard<std::mutex> lk(m);
      // envir() << "Received frame with size " << f.get_data_size() << "\n";
      // memmove(fbuf, f.get_data(), 640*480*2 /*f.get_data_size()*/);
      // memmove(fbuf, f.get_data(), f.get_data_size());
      memcpy(fbuf, f.get_data(), f.get_data_size());
      // frame = f;
      isWaitingFrame = true;
      cv.notify_one();
    };

    selected_sensor = sensors[deviceParams.sensorID];
    int streamID;
    std::vector<rs2::stream_profile> stream_profiles = selected_sensor.get_stream_profiles();
  
    streamID = get_stream_id();
    if (streamID == -1)
     {
      envir() << "failed to open stream \n";
      return;
     }  
    envir() << "stream ID is "<<streamID<< "\n";
    selected_sensor.open(stream_profiles[streamID]);
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
/*  
  deliverRSFrame();
}

void RsDeviceSource::deliverRSFrame()
{
*/
  IcompressFrame* iCompress =  compressFrameFactory::create(zipMethod::gzip);
  if (!isCurrentlyAwaitingData())
  {
    envir() << "isCurrentlyAwaitingData returned false"<<fParams.sensorID<<"\n";
    return; // we're not ready for the data yet
  }
  isWaitingFrame = false;
  //u_int8_t *newFrameDataStart = (u_int8_t *)frame.get_data();
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
  //// memmove(fTo, frame.get_data(), fFrameSize);
  //// unsigned char b[640*480*2];
   if(fParams.sensorID == 0) 
   {
#if 0
       unsigned char compressedBuf[fFrameSize];
       // memset(fbuf, 0, fFrameSize);
       unsigned int newSize = iCompress->compressFrame(fbuf, fFrameSize, compressedBuf);
       envir() << "Compression income " << fFrameSize << ", outcome " << newSize << "\n";
       // memmove(fTo, compressedBuf, fFrameSize);
       memmove(fTo, compressedBuf, newSize + sizeof(unsigned int));
#else
       iCompress->compressFrame(fbuf, fFrameSize, fTo);
#endif
   } else {
       memmove(fTo, fbuf, 640*480*2);
   }
  // After delivering the data, inform the reader that it is now available:
  FramedSource::afterGetting(this);
}

int RsDeviceSource::get_stream_id()
    {
        rs2_format f;
        if (fParams.sensorID == 0)
        {
          f = RS2_FORMAT_Z16;
        }
        else if(fParams.sensorID == 1)
        {
          f = RS2_FORMAT_YUYV;
        }
        else
        {
          return -1;
        }
        
        std::vector<rs2::stream_profile> stream_profiles = selected_sensor.get_stream_profiles();
        std::map<std::pair<rs2_stream, int>, int> unique_streams;
        for (auto&& sp : stream_profiles)
        {
            unique_streams[std::make_pair(sp.stream_type(), sp.stream_index())]++;
        }
        for (size_t i = 0; i < unique_streams.size(); i++)
        {
            auto it = unique_streams.begin();
            std::advance(it, i);
        }
        int profile_num = 0;
        for (rs2::stream_profile stream_profile : stream_profiles)
        {
            rs2_stream stream_data_type = stream_profile.stream_type();
            int stream_index = stream_profile.stream_index();
            int unique_stream_id = stream_profile.unique_id(); 
            if (stream_profile.is<rs2::video_stream_profile>()) 
            {     
                rs2::video_stream_profile video_stream_profile = stream_profile.as<rs2::video_stream_profile>();
                if (video_stream_profile.format() == f &&
                    video_stream_profile.width() == fParams.w &&
                    video_stream_profile.height() == fParams.h &&
                    video_stream_profile.fps() == fParams.fps )
                    {
                      return profile_num;
                    }
            }
            profile_num++;
        }
        return -1;
    }

