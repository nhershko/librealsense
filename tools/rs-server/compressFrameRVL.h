#pragma once

#include "IcompressFrame.h"

class compressFrameRVL :public IcompressFrame 
{
    public: 
        int compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf, int width, int height){};
        int compressDepthFrame(unsigned char* buffer, int size, unsigned char* compressedBuf);
    private:
        int EncodeVLE(int value);
        int *pBuffer, word, nibblesWritten;
};
