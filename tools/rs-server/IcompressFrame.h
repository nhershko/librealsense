#pragma once

class IcompressFrame {
   public: 
        virtual int compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf) = 0;
        virtual int compressDepthFrame(unsigned char* buffer, int size, unsigned char* compressedBuf) = 0;
};
