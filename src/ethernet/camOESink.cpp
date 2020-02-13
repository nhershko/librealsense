#include "camOESink.h"
#include "stdio.h"
#include <string>


#define WRITE_FRAMES_TO_FILE 0

camOESink* camOESink::createNew(UsageEnvironment& env, MediaSubsession& subsession, rs2_video_stream stream, memory_pool* memPool,char const* streamId) {
  return new camOESink(env, subsession,stream ,memPool, streamId);
}

camOESink::camOESink(UsageEnvironment& env, MediaSubsession& subsession,rs2_video_stream stream, memory_pool* mem_pool, char const* streamId)
  : MediaSink(env),memPool(mem_pool),fSubsession(subsession) 
  {
  fstream = stream;
  fStreamId = strDup(streamId);
  fBufferSize = stream.width*stream.height*stream.bpp + sizeof(rs_frame_header);
  fReceiveBuffer = nullptr;
  fto = nullptr;
  std::string url_str = fStreamId;
  // Remove last "/"
  url_str = url_str.substr(0, url_str.size()-1);
  std::size_t stream_name_index = url_str.find_last_of("/") + 1;
  std::string stream_name = url_str.substr(stream_name_index, url_str.size());

  /*if (stream_name.compare("depth") == 0)
  {
    fp = fopen("file_depth.bin", "ab");
  }
  else if((stream_name.compare("color") == 0))
  {
    fp = fopen("file_rgb.bin", "ab");
  }*/
#ifdef COMPRESSION
  iCompress = CompressionFactory::getObject(fstream.width, fstream.height, fstream.fmt, fstream.type);
#endif
  envir() << "create new sink";

}

camOESink::~camOESink() {
  if (fReceiveBuffer!=nullptr)
  {
    memPool->returnMem(fReceiveBuffer);
  }
  delete[] fStreamId;
  //fclose(fp);
}


void camOESink::afterGettingUid0Frame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  
  camOESink* sink = (camOESink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void camOESink::afterGettingUid1Frame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  
  camOESink* sink = (camOESink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);

}
                
void camOESink::afterGettingUid2Frame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  
  camOESink* sink = (camOESink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void camOESink::afterGettingUid3Frame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  
  camOESink* sink = (camOESink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 0

void camOESink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
  // We've just received a frame of data.  (Optionally) print out information about it:
/*
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
*/
  rs_over_ethernet_data_header *header = (rs_over_ethernet_data_header *)fReceiveBuffer;
  if (header->size == frameSize - sizeof(rs_over_ethernet_data_header))
  {
    if (this->m_rtp_callback != NULL)
    {
#ifdef COMPRESSION
      fto =  memPool->getNextMem();
      if (fto == nullptr)
      {
        return;
      }
      int decompressedSize = iCompress->decompressBuffer(fReceiveBuffer+sizeof(rs_frame_header), header->size-sizeof(rs_frame_metadata), fto+sizeof(rs_frame_header));
      // copy metadata
      memcpy(fto+sizeof(rs_over_ethernet_data_header), fReceiveBuffer+sizeof(rs_over_ethernet_data_header), sizeof(rs_frame_metadata));
      this->m_rtp_callback->on_frame((u_int8_t*)fto+sizeof(rs_over_ethernet_data_header), decompressedSize + sizeof(rs_frame_metadata), presentationTime);//todo: change to bpp
      memPool->returnMem(fReceiveBuffer);
#else
    this->m_rtp_callback->on_frame(fReceiveBuffer+sizeof(rs_over_ethernet_data_header), header->size, presentationTime);
#endif     
    }
    else
    {
      // TODO: error, no call back
      memPool->returnMem(fReceiveBuffer);
      envir() << "Frame call back is NULL\n";
    }
  }
  else
  {
    memPool->returnMem(fReceiveBuffer);
    envir() << fStreamId <<":corrupted frame!!!: data size is "<<header->size<<" frame size is "<< frameSize <<"\n";
    //printf("%p:corrupted frame!!!: data size is %d frame size is %d \n",fStreamId,header->size,frameSize);
  }
  fReceiveBuffer = nullptr;
  //fwrite(fReceiveBuffer, frameSize, 1, fp);
  // Then continue, to request the next frame of data
  continuePlaying();
}


Boolean camOESink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fReceiveBuffer = memPool->getNextMem();
  if (fReceiveBuffer == nullptr)
  {
    return false;
  }

  if(fstream.uid == 0)
  {
  fSource->getNextFrame(fReceiveBuffer, fBufferSize,
                        afterGettingUid0Frame, this,
                        onSourceClosure, this);
  }
  else if(fstream.uid == 1)
  {
  fSource->getNextFrame(fReceiveBuffer, fBufferSize,
                        afterGettingUid1Frame, this,
                        onSourceClosure, this);
  }
  else if(fstream.uid == 2)
  {
  fSource->getNextFrame(fReceiveBuffer, fBufferSize,
                        afterGettingUid2Frame, this,
                        onSourceClosure, this);
  }
  else if(fstream.uid == 3)
  {
  fSource->getNextFrame(fReceiveBuffer, fBufferSize,
                        afterGettingUid3Frame, this,
                        onSourceClosure, this);
  }
  else
  {
    return false;
  }
  return True;
}

void camOESink::set_callback(rtp_callback* callback)
{
  this->m_rtp_callback = callback;
}

