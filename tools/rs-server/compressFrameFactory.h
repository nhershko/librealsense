#pragma once

#include "IcompressFrame.h"

typedef enum zipMethod {
    gzip,
    rvl,
    Jpeg,
} zipMethod;

class compressFrameFactory
{
    public: 
        static IcompressFrame* create(zipMethod  zipMeth);
};
