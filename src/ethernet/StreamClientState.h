#ifndef _STREM_CLIENT_STATE_H
#define _STREM_CLIENT_STATE_H

#include "liveMedia.hh"

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class StreamClientState {
public:
  StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL) {
}
virtual ~StreamClientState() {
  // TODO: understand this
  /*delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);*/
  }

public:
  MediaSubsessionIterator* iter;
  MediaSession* session;
  MediaSubsession* subsession;
};

#endif // _STREM_CLIENT_STATE_H