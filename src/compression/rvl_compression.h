#pragma once

#include "icompression.h"

class RvlCompression :public ICompression 
{
    public: 
        int compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf);
        int decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
        RvlCompression(int width, int height, rs2_format format);
    private:
        int EncodeVLE(int value);
        int decodeVLE();
        int *pBuffer, word, nibblesWritten;
};
