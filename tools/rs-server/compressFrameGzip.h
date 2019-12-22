#pragma once

#include "IcompressFrame.h"
#include <zlib.h>

class compressFrameGzip :public IcompressFrame 
{
    public: 
        int compressFrame(unsigned char* buffer, int size, unsigned char* compressedBuf);
    private:
        z_stream strm;
};
