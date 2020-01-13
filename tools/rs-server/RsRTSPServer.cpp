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
    envir() << "RsRTSPClientConnection  constructor" << this <<"\n";
  }

RsRTSPServer::RsRTSPClientConnection::~RsRTSPClientConnection()
{
  envir() << "RsRTSPClientConnection  destructor" << this <<"\n";
}


////////// RsRTSPServer::RsRTSPClientSession implementation //////////

RsRTSPServer::RsRTSPClientSession
::RsRTSPClientSession(RTSPServer& ourServer, u_int32_t sessionId)
  :RTSPClientSession(ourServer, sessionId){
}

RsRTSPServer::RsRTSPClientSession::~RsRTSPClientSession(){}

void RsRTSPServer::RsRTSPClientSession::handleCmd_TEARDOWN(RTSPClientConnection* ourClientConnection,
				ServerMediaSubsession* subsession)
        {
            envir() << "TEARDOWN \n";
            RTSPServer::RTSPClientSession::handleCmd_TEARDOWN(ourClientConnection,subsession);
        }

void RsRTSPServer::RsRTSPClientSession::handleCmd_PLAY(RTSPClientConnection* ourClientConnection,
				ServerMediaSubsession* subsession, char const* fullRequestStr)
        {
            envir() << "PLAY \n";
            openRsCamera();
            RTSPServer::RTSPClientSession::handleCmd_PLAY(ourClientConnection,subsession,fullRequestStr);
        }

void RsRTSPServer::RsRTSPClientSession::handleCmd_PAUSE(RTSPClientConnection* ourClientConnection,
				 ServerMediaSubsession* subsession)
         {
            envir() << "PAUSE \n";
            RTSPServer::RTSPClientSession::handleCmd_PAUSE(ourClientConnection,subsession);
            closeRsCamera();
         }

int RsRTSPServer::RsRTSPClientSession::openRsCamera()
{
    for (int i = 0; i < fNumStreamStates; ++i) 
    {
      if (fStreamStates[i].subsession != NULL) 
      {
          long long int profile_key = ((RsServerMediaSession*)fOurServerMediaSession)->getRsSensor().getStreamProfileKey(((RsMediaSubsession*)(fStreamStates[i].subsession))->get_stream_profile());
          streamProfiles[profile_key] = ((RsMediaSubsession*)(fStreamStates[i].subsession))->get_frame_queue();
          rs2::frame f;
          while (streamProfiles[profile_key].poll_for_frame(&f));
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
          long long int profile_key = ((RsServerMediaSession*)fOurServerMediaSession)->getRsSensor().getStreamProfileKey(((RsMediaSubsession*)(fStreamStates[i].subsession))->get_stream_profile());
          rs2::frame f;
          while (streamProfiles[profile_key].poll_for_frame(&f));
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
