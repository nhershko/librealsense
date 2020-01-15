#pragma once

#include "IcompressFrame.h"
#include <zlib.h>

class compressFrameJpeg :public IcompressFrame 
{
    public: 
        int compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf, int width, int height);
        int compressDepthFrame(unsigned char* buffer, int size, unsigned char* compressedBuf){};
};
