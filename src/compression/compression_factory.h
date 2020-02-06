#pragma once

#include "icompression.h"

typedef enum zipMethod {
    gzip,
    rvl,
    Jpeg,
    lz,
} zipMethod;

class CompressionFactory
{
    public: 
        static ICompression* getObject(int width, int height, rs2_format format, rs2_stream stream_type);
};
