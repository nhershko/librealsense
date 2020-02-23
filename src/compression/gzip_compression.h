#pragma once

#include "icompression.h"
#include <zlib.h>

class GzipCompression :public ICompression 
{
    public: 
        int compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf);
        int decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
        GzipCompression(int width, int height, rs2_format format, int bpp);
    private:
        z_stream strm;
        int windowsBits, GZIP_ENCODING;
};
