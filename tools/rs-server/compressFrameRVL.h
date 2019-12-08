#pragma once

#include "IcompressFrame.h"

class compressFrameRVL :public IcompressFrame 
{
    public: 
        int compressFrame(unsigned char* buffer, int size, unsigned char* compressedBuf);
    private:
        int EncodeVLE(int value);
        int *pBuffer, word, nibblesWritten;
};
