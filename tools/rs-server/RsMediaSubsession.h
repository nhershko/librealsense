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
// C++ header
#include <liveMedia.hh>

#ifndef _RS_MEDIA_SUBSESSION_HH
#define _RS_SUBSESSION_HH

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

#include "RsSource.hh"



class RsMediaSubsession: public OnDemandServerMediaSubsession {
  public:
   static RsMediaSubsession* createNew(UsageEnvironment& env,  RSDeviceParameters p, rs2::device& selected_device, rs2_format f);
protected: // we're a virtual base class
  RsMediaSubsession(UsageEnvironment& env,  RSDeviceParameters p, rs2::device& selected_device, rs2_format f);//rs2::stream_profile* stream);
  virtual ~RsMediaSubsession();
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource);


private:
   RSDeviceParameters params;
   rs2::device device;
   int pixelSize;
   rs2_format format;
};
#endif
