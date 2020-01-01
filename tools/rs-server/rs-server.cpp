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

int w1 = 1280;//1280;
int h1 = 720;//720;
int w2 = 640;//1280;
int h2 = 480;//720;
UsageEnvironment *env;
rs2::device selected_device;
RsDeviceSource *devSource1;
RsDeviceSource *devSource2;
RawVideoRTPSink *videoSink1;
RawVideoRTPSink *videoSink2;
RTSPServer *rtspServer;

void play(); // forward
void sigint_handler(int sig);

int main(int argc, char **argv)
{
  signal(SIGINT,sigint_handler);

  // Begin by setting up our usage environment:
  TaskScheduler *scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  rtspServer = RTSPServer::createNew(*env, 8554);
  if (rtspServer == NULL)
  {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  rs2::context ctx;

  // Using the context we can get all connected devices in a device list
  rs2::device_list devices = ctx.query_devices();
  std::string name; 
  

  if (devices.size() == 0)
  {
    rs2::device_hub device_hub(ctx);
    selected_device = device_hub.wait_for_device();
  }
  else
  {
    *env << "Found " << devices.size() << " cameras\n";
    selected_device = devices[0];
  }
  


  ServerMediaSession *sms = ServerMediaSession::createNew(*env, "unicast", "",
                                                          "Session streamed by \"testH265VideoStreamer\"",
                                                          True);
  RSDeviceParameters params1(w1, h1, 2, 0, 30);
  RSDeviceParameters params2(w2, h2, 2, 1, 30);
  sms->addSubsession(RsMediaSubsession::createNew(*env,  params1, selected_device, RS2_FORMAT_Z16));
  sms->addSubsession(RsMediaSubsession::createNew(*env,  params2, selected_device, RS2_FORMAT_YUYV));
  rtspServer->addServerMediaSession(sms);

  char *url = rtspServer->rtspURL(sms);
  *env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;

  // Start the streaming:
  *env << "Beginning streaming...\n";

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

/*
void play()
{
  rs2::context ctx;

  // Using the context we can get all connected devices in a device list
  rs2::device_list devices = ctx.query_devices();
  std::string name; 
  

  if (devices.size() == 0)
  {
    rs2::device_hub device_hub(ctx);
    selected_device = device_hub.wait_for_device();
  }
  else
  {
    *env << "Found " << devices.size() << " cameras\n";
    selected_device = devices[0];
  }
  
  RSDeviceParameters params1(w1, h1, 2, 0, 30);
  RSDeviceParameters params2(w2, h2, 2, 1, 30);
  devSource1 = RsDeviceSource::createNew(*env, params1, selected_device); 
  if (devSource1 == NULL)
  {
    *env << "Unable to read from device source\n";
    exit(1);
  }
  videoSink1->startPlaying(*devSource1, afterPlaying1, videoSink1);

  devSource2 = RsDeviceSource::createNew(*env, params2, selected_device); 
  if (devSource2 == NULL)
  {
    *env << "Unable to read from device source\n";
    exit(1);
  }
  videoSink2->startPlaying(*devSource2, afterPlaying2, videoSink2);
}*/



void sigint_handler(int sig)
{
  Medium::close(rtspServer);
  exit(sig);
}
