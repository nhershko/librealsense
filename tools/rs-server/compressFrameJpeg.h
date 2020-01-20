#pragma once

#include "IcompressFrame.h"
#include "jpeglib.h"
#include <time.h>

class compressFrameJpeg :public IcompressFrame 
{
    public: 
        int compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf, int width, int height, rs2_format format);
        int compressDepthFrame(unsigned char* buffer, int size, unsigned char* compressedBuf){};
        compressFrameJpeg();
        ~compressFrameJpeg();
    private:
        struct jpeg_error_mgr jerr;
	    struct jpeg_compress_struct cinfo;
        unsigned char *rowBuffer;
        JSAMPROW row_pointer[1];
#ifdef COMPRESSION_STATISTICS
        clock_t t1, t2;
        float diffSum = 0;
        int frameCounter = 0;
#endif

};
