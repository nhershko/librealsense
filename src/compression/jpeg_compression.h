#pragma once

#include "icompression.h"
#include "jpeglib.h"
#include <time.h>

class JpegCompression :public ICompression 
{
    public: 
        int compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf);
        int decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
        JpegCompression(rs2::video_stream_profile &stream);
        ~JpegCompression();
    private:
        void convertYUYVtoYUV(unsigned char** buffer);
        void convertYUVtoYUYV(unsigned char** uncompressBuff);

        struct jpeg_error_mgr jerr;
	    struct jpeg_compress_struct cinfo;
        struct jpeg_decompress_struct dinfo;
        unsigned char *rowBuffer;
        JSAMPROW row_pointer[1];
        JSAMPARRAY destBuffer;
};
