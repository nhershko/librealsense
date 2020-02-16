#pragma once
#include <librealsense2/rs.hpp>
#include <librealsense2/hpp/rs_frame.hpp>

class ICompression {
   public: 
        virtual int compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf) = 0;
        virtual int decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf) = 0;
   protected:
        int decompframeCounter = 0, compframeCounter = 0;
        rs2::video_stream_profile *m_stream;

};
