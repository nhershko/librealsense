#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <functional>
#include <list> 
#include <iostream>
#include <iomanip>
#include <cassert>
#include "IdecompressFrame.h"

class decompressFrameJpeg :public IdecompressFrame 
{
        public: 
                void decompressColorFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
                void decompressDepthFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf){};

};
