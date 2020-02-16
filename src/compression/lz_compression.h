#pragma once

#include "icompression.h"
#include <lz4.h>

class LZCompression :public ICompression 
{
    public: 
        int compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf);
        int decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
        LZCompression(rs2::video_stream_profile & stream);
    private:

};
