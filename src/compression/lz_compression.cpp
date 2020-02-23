#pragma once

#include <iostream>
#include <cstring>
#include "lz_compression.h"
#include <ipDevice_Common/statistic.h>

 LZCompression::LZCompression(int width, int height, rs2_format format, int bpp)
 {
	 	m_format = format;
		m_width = width;
		m_height = height;
		m_bpp = bpp;
 }

int LZCompression::compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf)
{
#ifdef STATISTICS
	statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH]->compressionBegin = std::chrono::system_clock::now();
#endif
    const int max_dst_size = LZ4_compressBound(size);
    const int compressedSize = LZ4_compress_default((const char *)buffer, (char*)compressedBuf+sizeof(int), size, max_dst_size);
    if (compressedSize <= 0) {
		printf("error: 0 or negative result from LZ4_compress_default() indicates a failure trying to compress the data. ");
		return -1;
	}
	if (compframeCounter++%50 == 0) {
		printf("finish lz depth compression, size: %lu, compressed size %u, frameNum: %d \n",size, compressedSize, compframeCounter);
	}
#ifdef STATISTICS
	stream_statistic * st  = statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH];
	st->compressionFrameCounter++;
	std::chrono::system_clock::time_point compressionEnd = std::chrono::system_clock::now();
	st->compressionTime = compressionEnd - st->compressionBegin;
    st->avgCompressionTime += st->compressionTime.count();
    printf("STATISTICS: streamType: %d, lz4 compress time: %0.2fm, average: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_DEPTH, st->compressionTime*1000, 
            (st->avgCompressionTime*1000)/st->compressionFrameCounter,st->compressionFrameCounter);
	st->decompressedSizeSum = size;
	st->compressedSizeSum = compressedSize;
	printf("STATISTICS: streamType: %d, lz4 ratio: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_DEPTH,st->decompressedSizeSum/(float)st->compressedSizeSum, st->compressionFrameCounter);
#endif
	memcpy(compressedBuf, &compressedSize , sizeof(compressedSize));
	return compressedSize;
}


int  LZCompression::decompressBuffer(unsigned char* buffer, int compressedSize, unsigned char* uncompressedBuf) 
{	
#ifdef STATISTICS
	statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH]->decompressionBegin = std::chrono::system_clock::now();
#endif
    const int decompressed_size = LZ4_decompress_fast((const char *)buffer, (char *)uncompressedBuf, m_width* m_height * m_bpp);
    if (decompressed_size < 0) {
		printf("error: negative result from LZ4_decompress_safe indicates a failure trying to decompress the data\n");
		return -1;
	}
	int original_size = m_width* m_height * m_bpp;
    // if (decompressed_size != original_size); 
    // 	printf("Decompressed data is different from original!, decompressed_size: %d original size: %d \n",decompressed_size,  m_width* m_height * m_bpp );
	if (decompframeCounter++%50 == 0) {
		printf("finish lz depth decompression, size: %lu, compressed size %u, frameNum: %d \n", decompressed_size, compressedSize, decompframeCounter);
	}
#ifdef STATISTICS
	stream_statistic * st  = statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH];
	st->decompressionFrameCounter++;
	std::chrono::system_clock::time_point decompressionEnd = std::chrono::system_clock::now();
	st->decompressionTime = decompressionEnd - st->decompressionBegin;
    st->avgDecompressionTime += st->decompressionTime.count();
    printf("STATISTICS: streamType: %d, lz4 decompress time: %0.2fm, average: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_DEPTH, st->decompressionTime*1000, 
            (st->avgDecompressionTime*1000)/st->decompressionFrameCounter,st->decompressionFrameCounter);
#endif
	return decompressed_size;
}
