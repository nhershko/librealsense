#include "camOESink.h"
#include "stdio.h"
#include <string>

camOESink* camOESink::createNew(UsageEnvironment& env, MediaSubsession& subsession, int bufferSize, char const* streamId) {
  return new camOESink(env, subsession, bufferSize, streamId);
}

camOESink::camOESink(UsageEnvironment& env, MediaSubsession& subsession, int bufferSize, char const* streamId)
  : MediaSink(env),
    fSubsession(subsession) {
  fStreamId = strDup(streamId);
  fBufferSize = bufferSize;
  fReceiveBuffer = new u_int8_t[bufferSize];
  std::string url_str = fStreamId;
  // Remove last "/"
  url_str = url_str.substr(0, url_str.size()-1);
  std::size_t stream_name_index = url_str.find_last_of("/") + 1;
  std::string stream_name = url_str.substr(stream_name_index, url_str.size());
  if (stream_name.compare("depth") == 0)
  {
    fp = fopen("file_depth.bin", "ab");
  }
  else if((stream_name.compare("color") == 0))
  {
    fp = fopen("file_rgb.bin", "ab");
  }
}

camOESink::~camOESink() {
  delete[] fReceiveBuffer;
  delete[] fStreamId;
  fclose(fp);
}

void camOESink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  camOESink* sink = (camOESink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void camOESink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
  // We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
  if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
  envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
  if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
  char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
  sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
  envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
  if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
    envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
  }
#ifdef DEBUG_PRINT_NPT
  envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
  envir() << "\n";
#endif
  envir() << "********* frame ************\n";
  fwrite(fReceiveBuffer, frameSize, 1, fp);
  // Then continue, to request the next frame of data:
  continuePlaying();
}


Boolean camOESink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(fReceiveBuffer, fBufferSize,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  return True;
}

