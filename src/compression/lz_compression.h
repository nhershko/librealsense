#pragma once

#include "icompression.h"
#include <lz4.h>

class LZCompression :public ICompression 
{
    public: 
        int compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf);
        int decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
        LZCompression(int width, int height, rs2_format format);
    private:
};
