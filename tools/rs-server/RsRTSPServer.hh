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
// C++ header

#ifndef _RS_RTSP_SERVER_HH
#define _RS_RTSP_SERVER_HH

#ifndef _RTSP_SERVER_HH
#include "RsRTSPServer.hh"
#endif

#include "RsCamera.hh"
#include <librealsense2/rs.hpp>

class RsRTSPServer: public RTSPServer {
public:
  static RsRTSPServer* createNew(UsageEnvironment& env, Port ourPort = 554,
			       UserAuthenticationDatabase* authDatabase = NULL,
			       unsigned reclamationSeconds = 65);

protected:
  RsRTSPServer(UsageEnvironment& env,
	     int ourSocket, Port ourPort,
	     UserAuthenticationDatabase* authDatabase,
	     unsigned reclamationSeconds);
  virtual ~RsRTSPServer();

public: 
  class RsRTSPClientSession; // forward
  class RsRTSPClientConnection: public RTSPClientConnection {

  protected:
    RsRTSPClientConnection(RTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr);
    virtual ~RsRTSPClientConnection();
     
    friend class RsRTSPServer;
    friend class RsRTSPClientSession;
  };
  // The state of an individual client session (using one or more sequential TCP connections) handled by a RTSP server:
  class RsRTSPClientSession: public RTSPClientSession {
  protected:
    RsRTSPClientSession(RTSPServer& ourServer, u_int32_t sessionId);
    virtual ~RsRTSPClientSession();

    friend class RsRTSPServer;
    friend class RsRTSPClientConnection;

    virtual void handleCmd_withinSession(RTSPClientConnection* ourClientConnection,
					 char const* cmdName,
					 char const* urlPreSuffix, char const* urlSuffix,
					 char const* fullRequestStr);

    int openRsCamera();
    int closeRsCamera();

  protected:
    //RsRTSPServer& fOurRsRTSPServer; // same as ::fOurServer
  private:
    std::unordered_map<long long int, rs2::frame_queue> streamProfiles;
  };

protected: // redefined virtual functions
  // If you subclass "RTSPClientConnection", then you must also redefine this virtual function in order
  // to create new objects of your subclass:
  virtual ClientConnection* createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr);

protected:
  // If you subclass "RTSPClientSession", then you must also redefine this virtual function in order
  // to create new objects of your subclass:
  virtual ClientSession* createNewClientSession(u_int32_t sessionId);

private:
  int openRsCamera(RsSensor sensor,std::unordered_map<long long int, rs2::frame_queue>& streamProfiles);

private:
  friend class RsRTSPClientConnection;
  friend class RsRTSPClientSession;
};

#endif //_RS_RTSP_SERVER_HH
