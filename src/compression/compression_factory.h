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
        static ICompression* getObject(rs2::video_stream_profile stream);
};
