#pragma once
#include <librealsense2/rs.hpp>
class IcompressFrame {
   public: 
        virtual int compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf, int width, int height, rs2_format format) = 0;
        virtual int compressDepthFrame(unsigned char* buffer, int size, unsigned char* compressedBuf) = 0;
};
