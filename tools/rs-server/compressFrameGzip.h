#pragma once

#include "IcompressFrame.h"

class compressFrameGzip :public IcompressFrame 
{
    public: 
        int compressFrame(unsigned char* buffer, int size, unsigned char* compressedBuf);
};
