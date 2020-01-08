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
// A RTSP server
// Implementation

#include "RTSPServer.hh"
#include "RTSPCommon.hh"
#include "RTSPRegisterSender.hh"
#include "Base64.hh"
#include <GroupsockHelper.hh>
#include "RsRTSPServer.hh"
#include "RsServerMediaSession.h"
#include "RsMediaSubsession.h"

////////// RTSPServer implementation //////////

RsRTSPServer*
RsRTSPServer::createNew(UsageEnvironment& env, Port ourPort,
		      UserAuthenticationDatabase* authDatabase,
		      unsigned reclamationSeconds) {
  int ourSocket = setUpOurSocket(env, ourPort);
  if (ourSocket == -1) return NULL;
  
  return new RsRTSPServer(env, ourSocket, ourPort, authDatabase, reclamationSeconds);
}


RsRTSPServer::RsRTSPServer(UsageEnvironment& env,
		       int ourSocket, Port ourPort,
		       UserAuthenticationDatabase* authDatabase,
		       unsigned reclamationSeconds)
  : RTSPServer(env, ourSocket, ourPort, authDatabase, reclamationSeconds) {
}

RsRTSPServer::~RsRTSPServer(){}


////////// RTSPServer::RTSPClientConnection implementation //////////

RsRTSPServer::RsRTSPClientConnection
::RsRTSPClientConnection(RTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr)
  :RTSPClientConnection(ourServer, clientSocket, clientAddr)
  {
    //envir() << "RsRTSPClientConnection  constructor" << this <<"\n";
  }

RsRTSPServer::RsRTSPClientConnection::~RsRTSPClientConnection()
{
  //envir() << "RsRTSPClientConnection  destructor" << this <<"\n";
}


////////// RsRTSPServer::RsRTSPClientSession implementation //////////

RsRTSPServer::RsRTSPClientSession
::RsRTSPClientSession(RTSPServer& ourServer, u_int32_t sessionId)
  :RTSPClientSession(ourServer, sessionId){
}

RsRTSPServer::RsRTSPClientSession::~RsRTSPClientSession(){}


void RsRTSPServer::RsRTSPClientSession
::handleCmd_withinSession(RTSPServer::RTSPClientConnection* ourClientConnection,
			  char const* cmdName,
			  char const* urlPreSuffix, char const* urlSuffix,
			  char const* fullRequestStr) {
  // This will either be:
  // - a non-aggregated operation, if "urlPreSuffix" is the session (stream)
  //   name and "urlSuffix" is the subsession (track) name, or
  // - an aggregated operation, if "urlSuffix" is the session (stream) name,
  //   or "urlPreSuffix" is the session (stream) name, and "urlSuffix" is empty,
  //   or "urlPreSuffix" and "urlSuffix" are both nonempty, but when concatenated, (with "/") form the session (stream) name.
  // Begin by figuring out which of these it is:
  ServerMediaSubsession* subsession;
  
  if (fOurServerMediaSession == NULL) { // There wasn't a previous SETUP!
    ((RsRTSPClientConnection*)ourClientConnection)->handleCmd_notSupported();
    return;
  } else if (urlSuffix[0] != '\0' && strcmp(fOurServerMediaSession->streamName(), urlPreSuffix) == 0) {
    // Non-aggregated operation.
    // Look up the media subsession whose track id is "urlSuffix":
    ServerMediaSubsessionIterator iter(*fOurServerMediaSession);
    while ((subsession = iter.next()) != NULL) {
      if (strcmp(subsession->trackId(), urlSuffix) == 0) break; // success
    }
    if (subsession == NULL) { // no such track!
      ((RsRTSPClientConnection*)ourClientConnection)->handleCmd_notFound();
      return;
    }
  } else if (strcmp(fOurServerMediaSession->streamName(), urlSuffix) == 0 ||
	     (urlSuffix[0] == '\0' && strcmp(fOurServerMediaSession->streamName(), urlPreSuffix) == 0)) {
    // Aggregated operation
    subsession = NULL;
  } else if (urlPreSuffix[0] != '\0' && urlSuffix[0] != '\0') {
    // Aggregated operation, if <urlPreSuffix>/<urlSuffix> is the session (stream) name:
    unsigned const urlPreSuffixLen = strlen(urlPreSuffix);
    if (strncmp(fOurServerMediaSession->streamName(), urlPreSuffix, urlPreSuffixLen) == 0 &&
	fOurServerMediaSession->streamName()[urlPreSuffixLen] == '/' &&
	strcmp(&(fOurServerMediaSession->streamName())[urlPreSuffixLen+1], urlSuffix) == 0) {
      subsession = NULL;
    } else {
      ((RsRTSPClientConnection*)ourClientConnection)->handleCmd_notFound();
      return;
    }
  } else { // the request doesn't match a known stream and/or track at all!
    ((RsRTSPClientConnection*)ourClientConnection)->handleCmd_notFound();
    return;
  }
  
  if (strcmp(cmdName, "TEARDOWN") == 0) {
    envir() << "TEARDOWN \n";
    handleCmd_TEARDOWN(ourClientConnection, subsession);
  } else if (strcmp(cmdName, "PLAY") == 0) {  
    envir() << "PLAY \n";
    openRsCamera();
    handleCmd_PLAY(ourClientConnection, subsession, fullRequestStr);
  } else if (strcmp(cmdName, "PAUSE") == 0) {
    envir() << "PAUSE \n";
     handleCmd_PAUSE(ourClientConnection, subsession);
     closeRsCamera();
  } else if (strcmp(cmdName, "GET_PARAMETER") == 0) {
    handleCmd_GET_PARAMETER(ourClientConnection, subsession, fullRequestStr);
  } else if (strcmp(cmdName, "SET_PARAMETER") == 0) {
    handleCmd_SET_PARAMETER(ourClientConnection, subsession, fullRequestStr);
  }
}

int RsRTSPServer::RsRTSPClientSession::openRsCamera()
{
    for (int i = 0; i < fNumStreamStates; ++i) 
    {
      if (fStreamStates[i].subsession != NULL) 
      {
          int profile_indx = ((RsServerMediaSession*)fOurServerMediaSession)->getRsSensor().getStreamProfileIndex(((RsMediaSubsession*)(fStreamStates[i].subsession))->get_stream_profile());
          
          streamProfiles[profile_indx] = ((RsMediaSubsession*)(fStreamStates[i].subsession))->get_frame_queue();
          rs2::frame f;
          while (streamProfiles[profile_indx].poll_for_frame(&f))
          {
            envir() <<"dequeue from queue\n";
          }
      }  
    }
    ((RsServerMediaSession*)fOurServerMediaSession)->openRsCamera(streamProfiles);//TODO:: to check if this is indeed RsServerMediaSession
}

int RsRTSPServer::RsRTSPClientSession::closeRsCamera()
{
    ((RsServerMediaSession*)fOurServerMediaSession)->closeRsCamera();//TODO:: to check if this is indeed RsServerMediaSession
    for (int i = 0; i < fNumStreamStates; ++i) 
    {
      if (fStreamStates[i].subsession != NULL) 
      {
          int profile_indx = ((RsServerMediaSession*)fOurServerMediaSession)->getRsSensor().getStreamProfileIndex(((RsMediaSubsession*)(fStreamStates[i].subsession))->get_stream_profile());
          rs2::frame f;
          while (streamProfiles[profile_indx].poll_for_frame(&f))
          {
            envir() <<"dequeue from queue\n";
          }
      }  
    }
}

GenericMediaServer::ClientConnection*
RsRTSPServer::createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr) {
  return new RsRTSPClientConnection(*this, clientSocket, clientAddr);
}

GenericMediaServer::ClientSession*
RsRTSPServer::createNewClientSession(u_int32_t sessionId) {
  return new RsRTSPClientSession(*this, sessionId);
}
