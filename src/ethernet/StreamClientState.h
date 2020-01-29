#ifndef _STREAM_CLIENT_STATE_H
#define _STREAM_CLIENT_STATE_H

#include "RsMediaSession.hh"

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
  RsMediaSubsessionIterator* iter;
  RsMediaSession* session;
  RsMediaSubsession* subsession;
};

#endif // _STREAM_CLIENT_STATE_H
