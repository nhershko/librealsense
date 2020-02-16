#pragma once

#include "icompression.h"
#include <zlib.h>

class GzipCompression :public ICompression 
{
    public: 
        int compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf);
        int decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
        GzipCompression(rs2::video_stream_profile &stream);
    private:
        z_stream strm;
        int windowsBits, GZIP_ENCODING;
};
