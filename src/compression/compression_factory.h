#pragma once

#include "icompression.h"

typedef enum zipMethod {
    gzip,
    rvl,
    Jpeg,
} zipMethod;

class CompressionFactory
{
    public: 
        static ICompression* create(zipMethod  zipMeth, int width, int height, rs2_format format);
};
