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
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**********/
// Copyright (c) 1996-2019, Live Networks, Inc.  All rights reserved
// A test program that reads a H.265 Elementary Stream video file
// and streams it using RTP
// main program
//
// NOTE: For this application to work, the H.265 Elementary Stream video file *must* contain
// VPS, SPS and PPS NAL units, ideally at or near the start of the file.
// These VPS, SPS and PPS NAL units are used to specify 'configuration' information that is set in
// the output stream's SDP description (by the RTSP server that is built in to this application).
// Note also that - unlike some other "*Streamer" demo applications - the resulting stream can be
// received only using a RTSP client (such as "openRTSP")

#include <iostream>

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <signal.h>
#include "RsSource.hh"
#include "RsMediaSubsession.h"
#include "RsDevice.hh"
#include "RsRTSPServer.hh"
#include "RsServerMediaSession.h"

UsageEnvironment *env;
rs2::device selected_device;
RTSPServer *rtspServer;
RsDevice device;
std::vector<RsSensor> sensors;

void sigint_handler(int sig);

int main(int argc, char **argv)
{
  signal(SIGINT, sigint_handler);

  // Begin by setting up our usage environment:
  TaskScheduler *scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  rtspServer = RsRTSPServer::createNew(*env, 8554);
  if (rtspServer == NULL)
  {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  sensors = device.getSensors();
  int sensorIndex = 0; //TODO::to remove
  for (auto sensor : sensors)
  {
    RsServerMediaSession *sms;
    if (sensorIndex == 0) //TODO: to move to generic exposure when host is ready
    {
      sms = RsServerMediaSession::createNew(*env, sensor, "depth" /*sensor.get_sensor_name().data()*/, "",
                                            "Session streamed by \"realsense streamer\"",
                                            True);
    }
    else if (sensorIndex == 1)
    {
      sms = RsServerMediaSession::createNew(*env, sensor, "color" /*sensor.get_sensor_name().data()*/, "",
                                            "Session streamed by \"realsense streamer\"",
                                            True);
    }
    int index = 0;
    for (auto stream_profile : sensor.getStreamProfiles())
    {
      rs2::video_stream_profile stream = stream_profile.second;
      //TODO: expose all streams when host is ready
      /*if ((sensorIndex == 1 || sensorIndex == 0) //don't expose IMU streams
          && (stream.format() == RS2_FORMAT_Z16 || stream.format() == RS2_FORMAT_Y16 || stream.format() == RS2_FORMAT_RAW16 || stream.format() == RS2_FORMAT_YUYV))
      {
        sms->addSubsession(RsMediaSubsession::createNew(*env,  stream));
      }*/
      if (sensorIndex == 0 && stream.width() == 640 && stream.height() == 480 && stream.format() == RS2_FORMAT_Z16 && stream.fps() == 30)
      {
        sms->addSubsession(RsMediaSubsession::createNew(*env, stream));
      }
      if (sensorIndex==0 && stream.width()==640 && stream.height() == 480 && stream.format()== RS2_FORMAT_RGB8 && stream.fps() == 30)
      {

        *env << "\n\n\n\nstream added\n";        
         sms->addSubsession(RsMediaSubsession::createNew(*env,  stream));
      }
      if (sensorIndex == 1 && stream.width() == 640 && stream.height() == 480 && stream.format() == RS2_FORMAT_YUYV && stream.fps() == 30)
      {
        sms->addSubsession(RsMediaSubsession::createNew(*env, stream));
      }
      index++;
    }

    rtspServer->addServerMediaSession(sms);
    char *url = rtspServer->rtspURL(sms);
    *env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
    sensorIndex++;
  }

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

void sigint_handler(int sig)
{
  Medium::close(rtspServer);
  exit(sig);
}
