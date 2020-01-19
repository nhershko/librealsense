#pragma once

#include "IcompressFrame.h"
#include <zlib.h>

class compressFrameGzip :public IcompressFrame 
{
    public: 
        int compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf, int width, int height, rs2_format format);
        int compressDepthFrame(unsigned char* buffer, int size, unsigned char* compressedBuf);
    private:
        z_stream strm;
};
