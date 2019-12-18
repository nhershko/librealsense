#pragma once

#include "IcompressFrame.h"

typedef enum zipMethod {
    gzip,
    rvl,
} zipMethod;

class compressFrameFactory
{
    public: 
        static IcompressFrame* create(zipMethod  zipMeth);
};
