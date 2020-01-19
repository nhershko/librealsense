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

#ifndef _RS_SERVER_MEDIA_SUBSESSION_HH
#define _RS_SERVER_MEDIA_SUBSESSION_HH

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

#include "RsSource.hh"

class RsServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
   static RsServerMediaSubsession *createNew(UsageEnvironment &env, rs2::video_stream_profile &video_stream_profile /*, rs2::frame_queue &queue*/);
   rs2::frame_queue &get_frame_queue();
   rs2::video_stream_profile get_stream_profile();

protected:                                                                                                                  // we're a virtual base class
   RsServerMediaSubsession(UsageEnvironment &env, rs2::video_stream_profile &video_stream_profile /*, rs2::frame_queue &queue*/); //rs2::stream_profile* stream);
   virtual ~RsServerMediaSubsession();
   virtual FramedSource *createNewStreamSource(unsigned clientSessionId,
                                               unsigned &estBitrate);
   virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                                     unsigned char rtpPayloadTypeIfDynamic,
                                     FramedSource *inputSource);

private:
   rs2::video_stream_profile videoStreamProfile;
   rs2::frame_queue frameQueue;
   int pixelSize;
};
#endif //_RS_SERVER_MEDIA_SUBSESSION_HH
