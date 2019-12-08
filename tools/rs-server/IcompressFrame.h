#pragma once

class IcompressFrame {
   public: 
        virtual int compressFrame(unsigned char* buffer, int size, unsigned char* compressedBuf) = 0;
};
