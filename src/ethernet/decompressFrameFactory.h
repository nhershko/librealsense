#pragma once

#include "IdecompressFrame.h"

typedef enum zipMethod {
    gzip,
    rvl,
} zipMethod;

class decompressFrameFactory
{
    public: 
        static IdecompressFrame* create(zipMethod  zipMeth);
};
