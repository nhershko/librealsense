#pragma once

#include <sstream>
#include <iomanip>
#include "IdecompressFrame.h"
#include "jpeglib.h"
#include <time.h>

class decompressFrameJpeg :public IdecompressFrame 
{
        public: 
                void decompressColorFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
                void decompressDepthFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf){};
                decompressFrameJpeg();
                ~decompressFrameJpeg();
        private:
                struct jpeg_error_mgr jerr;
	        struct jpeg_decompress_struct cinfo;
                JSAMPARRAY destBuffer;
#ifdef COMPRESSION_STATISTICS
                clock_t t1, t2;
                float diffSum = 0;
                int frameCounter = 0;
                long long  fullSizeSum = 0, compressedSizeSum = 0;
#endif
};
