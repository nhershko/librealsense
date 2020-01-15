#pragma once

class IcompressFrame {
   public: 
        virtual int compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf, int width, int height) = 0;
        virtual int compressDepthFrame(unsigned char* buffer, int size, unsigned char* compressedBuf) = 0;
};
