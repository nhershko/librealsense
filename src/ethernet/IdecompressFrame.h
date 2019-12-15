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

class IdecompressFrame {
   public: 
        virtual void decompressFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf) = 0;
};
