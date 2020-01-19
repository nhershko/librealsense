#pragma once

#include <sstream>
#include <iomanip>
#include "IdecompressFrame.h"
#include "jpeglib.h"

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
};
