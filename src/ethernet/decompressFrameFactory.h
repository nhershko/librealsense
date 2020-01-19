#pragma once

#include "IdecompressFrame.h"

typedef enum zipMethod {
    gzip,
    rvl,
    Jpeg,
} zipMethod;

class decompressFrameFactory
{
    public: 
        static IdecompressFrame* create(zipMethod  zipMeth);
};
