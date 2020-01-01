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
//#include <librealsense2/h/rs_sensor.h>


RsMediaSubsession* RsMediaSubsession::createNew(UsageEnvironment& env,  RSDeviceParameters p, rs2::device& selected_device, rs2_format f)
{
  return new RsMediaSubsession(env,p,selected_device,f);
}

RsMediaSubsession
::RsMediaSubsession(UsageEnvironment& env, RSDeviceParameters p, rs2::device& selected_device, rs2_format f)//rs2::stream_profile* stream)
  : OnDemandServerMediaSubsession(env, false),params(p),format(f)
{
  device = selected_device;
}

RsMediaSubsession::~RsMediaSubsession() {

}

FramedSource* RsMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = 5000; // kbps, estimate //TODO:: to calculate the right valu
  
  return RsDeviceSource::createNew(envir(), params, device); 
}

RTPSink* RsMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
  envir() << "RsMediaSubsession createNewRTPSink \n";
  //RawVideoRTPSink *videoSink;
  switch (format)
  {            
  case  RS2_FORMAT_RGB8: 
  {
      pixelSize = 3;
       return  RawVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, params.h, params.w, 8, "RGB");
       
      // break;           
  }           
  case  RS2_FORMAT_BGR8: 
  {
    pixelSize = 3;
       return RawVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, params.h, params.w, 8, "BGR");
       
       //break;           
  }
  case  RS2_FORMAT_RGBA8:  
  {
    pixelSize = 3;
       return RawVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, params.h, params.w, 8, "RGBA");
       
       //break;           
  }         
  case  RS2_FORMAT_BGRA8: 
  {
    pixelSize = 3;
       return RawVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, params.h, params.w, 8, "BGRA");
       //break;           
  }       
  case  RS2_FORMAT_Z16:   
  case  RS2_FORMAT_Y16:             
  case  RS2_FORMAT_RAW16:          
  case  RS2_FORMAT_YUYV:  
  {
      pixelSize = 2;
       return RawVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, params.h, params.w, 8, "YCbCr-4:2:2");
      // break;           
  }
  default:
     pixelSize = 0;
     break;
  }    

  return NULL;
}

