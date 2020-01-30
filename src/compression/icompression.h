#pragma once
#include <librealsense2/rs.hpp>

class ICompression {
   public: 
        virtual int compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf) = 0;
        virtual void decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf) = 0;
   protected:
        int m_width, m_height;
        rs2_format m_format;
        int decompframeCounter = 0, compframeCounter = 0;
#ifdef COMPRESSION_STATISTICS
        clock_t tCompBegin, tCompEnd, tDecompBegin, tDecompEnd;
        float compTimeDiff = 0, decompTimeDiff = 0;
        long long  fullSizeSum = 0, compressedSizeSum = 0;
#endif

};
