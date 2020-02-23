#pragma once
#include <librealsense2/rs.hpp>

class ICompression {
   public: 
        virtual int compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf) = 0;
        virtual int decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf) = 0;
   protected:
        int m_width, m_height, m_bpp;
        rs2_format m_format;
        int decompframeCounter = 0, compframeCounter = 0;

};
