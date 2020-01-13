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
// Implementation

#include "RsServerMediaSession.h"


////////// ServerMediaSession //////////

RsServerMediaSession* RsServerMediaSession
::createNew(UsageEnvironment& env, RsSensor& sensor,
	    char const* streamName, char const* info,
	    char const* description, Boolean isSSM, char const* miscSDPLines) {
  return new RsServerMediaSession(env, sensor, streamName, info, description,
				isSSM, miscSDPLines);
}


static char const* const libNameStr = "LIVE555 Streaming Media v";
char const* const libVersionStr = LIVEMEDIA_LIBRARY_VERSION_STRING;

RsServerMediaSession::RsServerMediaSession(UsageEnvironment& env,
               RsSensor& sensor,
				       char const* streamName,
				       char const* info,
				       char const* description,
				       Boolean isSSM, char const* miscSDPLines)
  : ServerMediaSession(env,streamName,info,description,isSSM,miscSDPLines),rsSensor(sensor) {
    envir() << "RsServerMediaSession constructor \n";
}

RsServerMediaSession::~RsServerMediaSession() {
  envir() << "RsServerMediaSession destructor \n";
  //TODO:: to check if i need to delete rsSensor
}


int RsServerMediaSession::openRsCamera( std::unordered_map<long long int, rs2::frame_queue> &streamProfiles)
{
    envir() << "openRsCamera  \n";
    int status = rsSensor.open(streamProfiles);
    if (status == EXIT_SUCCESS)
    {
      return rsSensor.start(streamProfiles);
    }
    else
    {
      return status;
    }    
}

void RsServerMediaSession::closeRsCamera()
{
    envir() << "closeRsCamera  \n";
    rsSensor.getRsSensor().stop();
    rsSensor.getRsSensor().close();
}


RsSensor& RsServerMediaSession::getRsSensor()
{
  return rsSensor;
}
