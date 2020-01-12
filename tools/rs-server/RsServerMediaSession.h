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
// A data structure that represents a session that consists of
// potentially multiple (audio and/or video) sub-sessions
// (This data structure is used for media *streamers* - i.e., servers.
//  For media receivers, use "MediaSession" instead.)
// C++ header

#ifndef _RS_SERVER_MEDIA_SESSION_HH
#define _RS_SERVER_MEDIA_SESSION_HH

#include "ServerMediaSession.hh"
#include "RsCamera.hh"

class RsServerMediaSession: public ServerMediaSession {
public:
  static RsServerMediaSession* createNew(UsageEnvironment& env,
               RsSensor& sensor,
				       char const* streamName = NULL,
				       char const* info = NULL,
				       char const* description = NULL,
				       Boolean isSSM = False,
				       char const* miscSDPLines = NULL);
  RsSensor& getRsSensor();
  int openRsCamera(std::unordered_map<long long int, rs2::frame_queue> &streamProfiles);
  void closeRsCamera();




protected:
  RsServerMediaSession(UsageEnvironment& env,RsSensor& sensor, char const* streamName,
		     char const* info, char const* description,
		     Boolean isSSM, char const* miscSDPLines);
  // called only by "createNew()"

  virtual ~RsServerMediaSession();

private: 
  RsSensor rsSensor;
  //std::map<int, rs2::frame_queue> streamProfiles;
};

#endif
