#pragma once

#include <iostream>
#include <cstring>
#include "lz_compression.h"

 LZCompression::LZCompression(int width, int height, rs2_format format)
 {
	 	m_format = format;
		m_width = width;
		m_height = height;
 }

int LZCompression::compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf)
{
#ifdef COMPRESSION_STATISTICS
	tCompBegin = clock();
#endif
    const int max_dst_size = LZ4_compressBound(size);
    const int compressed_data_size = LZ4_compress_default((const char *)buffer, (char*)compressedBuf, size, max_dst_size);
    if (compressed_data_size <= 0) {
		printf("error: 0 or negative result from LZ4_compress_default() indicates a failure trying to compress the data. ");
		return -1;
	}
	if (compframeCounter++%50 == 0) {
		printf("finish lz depth compression, size: %lu, compressed size %u, frameNum: %d \n",size, compressed_data_size, compframeCounter);
	}
#ifdef COMPRESSION_STATISTICS	
	tCompEnd = clock();
	compTimeDiff += tCompEnd - tCompBegin;
	if (compframeCounter%50 == 0) {
		printf("lz compress time measurement is: %0.2f, average: %0.2f, frameCounter: %d\n",((float)(tCompEnd - tCompBegin))/1000, ((float)compTimeDiff/compframeCounter)/1000, compframeCounter);
	}
#endif
	return compressed_data_size;
}


int  LZCompression::decompressBuffer(unsigned char* buffer, int compressedSize, unsigned char* uncompressedBuf) 
{	
#ifdef COMPRESSION_STATISTICS
	tDecompBegin = clock();
#endif
    const int decompressed_size = LZ4_decompress_fast((const char *)buffer, (char *)uncompressedBuf, m_width* m_height * 2); //change to bpp
    if (decompressed_size < 0) {
		printf("error: negative result from LZ4_decompress_safe indicates a failure trying to decompress the data\n");
		return -1;
	}
	int original_size = m_width* m_height * 2; //change to bpp
    // if (decompressed_size != original_size); 
    // 	printf("Decompressed data is different from original!, decompressed_size: %d original size: %d \n",decompressed_size,  m_width* m_height * 2 );
	if (decompframeCounter++%50 == 0) {
		printf("finish lz depth decompression, size: %lu, compressed size %u, frameNum: %d \n", decompressed_size, compressedSize, decompframeCounter);
	}
#ifdef COMPRESSION_STATISTICS
	tDecompEnd = clock();
	decompTimeDiff += tDecompEnd - tDecompBegin;

	fullSizeSum += m_width*m_height*2;//change to bpp
	compressedSizeSum += compressedSize;
	if (decompframeCounter%50 == 0) {
		printf("lz decompress zip ratio is: %0.2f , frameCounter: %d\n", fullSizeSum/(float)compressedSizeSum, decompframeCounter);
		printf("lz decompress time measurement is: %0.2f, average: %0.2f, frameCounter: %d\n",((float)(tDecompEnd - tDecompBegin))/1000, ((float)decompTimeDiff/decompframeCounter)/1000, decompframeCounter);
	}
#endif
	return decompressed_size;
}
