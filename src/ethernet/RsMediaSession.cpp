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
// Implementation

#include "liveMedia.hh"
#include "Locale.hh"
#include "GroupsockHelper.hh"
#include "RsMediaSession.hh"
#include <ctype.h>

////////// RsMediaSession //////////

RsMediaSession *RsMediaSession::createNew(UsageEnvironment &env,
                                          char const *sdpDescription)
{
  RsMediaSession *newSession = new RsMediaSession(env);
  if (newSession != NULL)
  {
    if (!newSession->initializeWithSDP(sdpDescription))
    {
      delete newSession;
      return NULL;
    }
  }
  return newSession;
}

RsMediaSession::RsMediaSession(UsageEnvironment &env)
    : MediaSession(env)
{
}

RsMediaSession::~RsMediaSession()
{
}

MediaSubsession *RsMediaSession::createNewMediaSubsession()
{
  // default implementation:
  return new RsMediaSubsession(*this);
}

////////// RsMediaSubsessionIterator //////////

RsMediaSubsessionIterator::RsMediaSubsessionIterator(RsMediaSession const &session)
    : fOurSession(session)
{
  reset();
}

RsMediaSubsessionIterator::~RsMediaSubsessionIterator()
{
}

RsMediaSubsession *RsMediaSubsessionIterator::next()
{
  RsMediaSubsession *result = fNextPtr;

  if (fNextPtr != NULL)
    fNextPtr = (RsMediaSubsession *)(fNextPtr->fNext);

  return result;
}

void RsMediaSubsessionIterator::reset()
{
  fNextPtr = (RsMediaSubsession *)(fOurSession.fSubsessionsHead);
}

////////// MediaSubsession //////////

RsMediaSubsession::RsMediaSubsession(RsMediaSession &parent)
    : MediaSubsession(parent)
{
}

RsMediaSubsession::~RsMediaSubsession()
{
}

Boolean RsMediaSubsession::createSourceObjects(int useSpecialRTPoffset)
{
  if (strcmp(fCodecName, "Y") == 0)
  {
    // This subsession uses our custom RTP payload format:
    char *mimeType = new char[strlen(mediumName()) + strlen(codecName()) + 2];
    sprintf(mimeType, "%s/%s", mediumName(), codecName());
    fReadSource = fRTPSource = SimpleRTPSource::createNew(env(), fRTPSocket, fRTPPayloadFormat,
                                                          fRTPTimestampFrequency, mimeType);
    delete[] mimeType;
    return True;
  }
  else
  {
    // This subsession uses some other RTP payload format - perhaps one that we already implement:
    return MediaSubsession::createSourceObjects(useSpecialRTPoffset);
  }
}
