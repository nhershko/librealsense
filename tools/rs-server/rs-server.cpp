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
#include "RsCamera.h"
#include "RsRTSPServer.hh"
#include "RsServerMediaSession.h"

int w1 = 640;//1280;
int h1 = 480;//720;
int w2 = 640;//1280;
int h2 = 480;//720;
UsageEnvironment *env;
rs2::device selected_device;
RsDeviceSource *devSource1;
RsDeviceSource *devSource2;
RawVideoRTPSink *videoSink1;
RawVideoRTPSink *videoSink2;
RTSPServer *rtspServer;
RsCamera cam;
std::vector<RsSensor> sensors;
std::map<int, rs2::frame_queue> depth_queues;

void play(); // forward
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

  sensors = cam.getSensors();
  int sensorIndex =0;//TODO::to remove
  for (auto sensor:sensors)
  {    
    RsServerMediaSession *sms = RsServerMediaSession::createNew(*env,sensor, sensor.get_sensor_name().data(), "",
                                                          "Session streamed by \"realsense streamer\"",
                                                          True);
    int index = 0;
    for (auto stream:sensor.getStreamProfiles())
    {
      if (sensorIndex==0 && stream.width()==640 && stream.height() == 480 && stream.format()== RS2_FORMAT_Z16 && stream.fps() == 30)
      {
        //depth_queues[index] = rs2::frame_queue(CAPACITY, true);
        sms->addSubsession(RsMediaSubsession::createNew(*env,  stream/*,depth_queues[index]*/));
      }
      else  if (sensorIndex==0 && stream.width()==640 && stream.height() == 480 && stream.format()== RS2_FORMAT_RGB8 && stream.fps() == 30)
      {
         sms->addSubsession(RsMediaSubsession::createNew(*env,  stream/*,depth_queues[index]*/));
      }
      else if (sensorIndex==1 && stream.width()==640 && stream.height() == 480 && stream.format()== RS2_FORMAT_YUYV && stream.fps() == 30)
      {
        //depth_queues[index] = rs2::frame_queue(CAPACITY, true);
        sms->addSubsession(RsMediaSubsession::createNew(*env,  stream/*, depth_queues[index]*/));
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
