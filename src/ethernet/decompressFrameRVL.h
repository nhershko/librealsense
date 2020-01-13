#pragma once

#include <sstream>
#include <functional>
#include <iostream>
#include <iomanip>
#include "IdecompressFrame.h"

class decompressFrameRVL :public IdecompressFrame 
{
    public: 
        void decompressColorFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf) {};
        void decompressDepthFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
    private:
        int decodeVLE();
        int *pBuffer, word, nibblesWritten;
        long long  fullSizeSum = 0, compressedSizeSum = 0, frameCounter = 0; //for ratio statistics
};
