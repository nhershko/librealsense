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
// (This data structure is used for media *receivers* - i.e., clients.
//  For media streamers, use "ServerMediaSession" instead.)
// C++ header

/* NOTE: To support receiving your own custom RTP payload format, you must first define a new
   subclass of "MultiFramedRTPSource" (or "BasicUDPSource") that implements it.
   Then define your own subclass of "MediaSession" and "MediaSubsession", as follows:
   - In your subclass of "MediaSession" (named, for example, "myMediaSession"):
       - Define and implement your own static member function
           static myMediaSession* createNew(UsageEnvironment& env, char const* sdpDescription);
	 and call this - instead of "MediaSession::createNew()" - in your application,
	 when you create a new "MediaSession" object.
       - Reimplement the "createNewMediaSubsession()" virtual function, as follows:
           MediaSubsession* myMediaSession::createNewMediaSubsession() { return new myMediaSubsession(*this); }
   - In your subclass of "MediaSubsession" (named, for example, "myMediaSubsession"):
       - Reimplement the "createSourceObjects()" virtual function, perhaps similar to this:
           Boolean myMediaSubsession::createSourceObjects(int useSpecialRTPoffset) {
	     if (strcmp(fCodecName, "X-MY-RTP-PAYLOAD-FORMAT") == 0) {
	       // This subsession uses our custom RTP payload format:
	       fReadSource = fRTPSource = myRTPPayloadFormatRTPSource::createNew( <parameters> );
	       return True;
	     } else {
	       // This subsession uses some other RTP payload format - perhaps one that we already implement:
	       return ::createSourceObjects(useSpecialRTPoffset);
	     }
	   }  
*/

#ifndef _RS_MEDIA_SESSION_HH
#define _RS_MEDIA_SESSION_HH

#include "MediaSession.hh"

class RsMediaSubsession; // forward

class RsMediaSession: public MediaSession {
public:
  static RsMediaSession* createNew(UsageEnvironment& env,
				 char const* sdpDescription);
 
protected:
  RsMediaSession(UsageEnvironment& env);
      // called only by createNew();
  virtual ~RsMediaSession();

  virtual MediaSubsession* createNewMediaSubsession();

  friend class RsMediaSubsessionIterator;
};


class RsMediaSubsessionIterator {
public:
  RsMediaSubsessionIterator(RsMediaSession const& session);
  virtual ~RsMediaSubsessionIterator();

  RsMediaSubsession* next(); // NULL if none
  void reset();

private:
  RsMediaSession const& fOurSession;
  RsMediaSubsession* fNextPtr;
};


class RsMediaSubsession: public MediaSubsession {
protected:
  friend class RsMediaSession;
  friend class RsMediaSubsessionIterator;
  RsMediaSubsession(RsMediaSession& parent);
  virtual ~RsMediaSubsession();
  virtual Boolean createSourceObjects(int useSpecialRTPoffset);
    // create "fRTPSource" and "fReadSource" member objects, after we've been initialized via SDP
};

#endif
