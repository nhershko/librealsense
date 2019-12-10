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

class decompressFrameGzip :public IdecompressFrame 
{
        public: 
                void decompressFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf);
        private:
                long long  fullSizeSum = 0, compressedSizeSum = 0, frameCounter = 0; //for ratio statistics
                z_stream strm;

};
