#pragma once

#include "icompression.h"

class RvlCompression :public ICompression 
{
    public: 
        int compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf);
        int decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
        RvlCompression(rs2::video_stream_profile &stream);
    private:
        int EncodeVLE(int value);
        int decodeVLE();
        int *pBuffer, word, nibblesWritten;
};
