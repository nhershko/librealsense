#pragma once

#include <iostream>
#include <cstring>
#include <zlib.h>
#include "gzip_compression.h"

 GzipCompression::GzipCompression(int width, int height, rs2_format format)
 {
	 	m_format = format;
		m_width = width;
		m_height = height;
		windowsBits = 15;
		GZIP_ENCODING = 16;
 }

int GzipCompression::compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf)
{
#ifdef COMPRESSION_STATISTICS
	tCompBegin = clock();
#endif
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = (Bytef *)buffer;
	strm.avail_in =  size;
	strm.next_out = (Bytef *)compressedBuf;
	strm.avail_out = size;
	int z_result = deflateInit2 (&strm, Z_BEST_SPEED/*Z_DEFAULT_COMPRESSION*/, Z_DEFLATED, windowsBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);
	if(z_result != Z_OK) {
		printf("error: init frame compression with gzip failed\n");
		return -1;
	}
	z_result = deflate(&strm, Z_FINISH);
	if(z_result != Z_STREAM_END) {
		printf("error: compress frame with gzip failed\n");
		return -1;
	}
	int compressedSize = strm.total_out;
	deflateEnd(&strm);
	if (compframeCounter%50 == 0) {
		printf("finish gzip depth compression, size: %lu, compressed size %u, frameNum: %d \n",size, compressedSize, compframeCounter);
	}
#ifdef COMPRESSION_STATISTICS	
	tCompEnd = clock();
	compTimeDiff += tCompEnd - tCompBegin;
	compframeCounter++;
	if (compframeCounter%50 == 0) {
		printf("gzip compress time measurement is: %0.2f, average: %0.2f, frameCounter: %d\n",((float)(tCompEnd - tCompBegin))/1000, ((float)compTimeDiff/compframeCounter)/1000, compframeCounter);
	}
#endif
	return compressedSize;
}

void  GzipCompression::decompressBuffer(unsigned char* buffer, int compressedSize, unsigned char* uncompressedBuf) 
{	
#ifdef COMPRESSION_STATISTICS
	tDecompBegin = clock();
#endif
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = (Bytef *)buffer;
	strm.avail_in =  compressedSize;
	strm.next_out = (Bytef *)uncompressedBuf;
	strm.avail_out = m_width*m_height*2; //change to bpp
	int z_result = inflateInit2(&strm, windowsBits | GZIP_ENCODING);
	z_result = inflate(&strm, Z_FINISH);
	if(z_result == Z_STREAM_ERROR || z_result == Z_BUF_ERROR) {
		printf("error: decompress frame with gzip failed\n");
		return;
	}
	inflateEnd(&strm);
	if (decompframeCounter%50 == 0) {
		printf("finish gzip depth decompression, size: %lu, compressed size %u, frameNum: %d \n", strm.total_out, compressedSize, decompframeCounter);
	}
#ifdef COMPRESSION_STATISTICS
	tDecompEnd = clock();
	decompTimeDiff += tDecompEnd - tDecompBegin;

	fullSizeSum += m_width*m_height*2;//change to bpp
	compressedSizeSum += compressedSize;
	decompframeCounter++;
	if (decompframeCounter%50 == 0) {
		printf("gzip decompress zip ratio is: %0.2f , frameCounter: %d\n", fullSizeSum/(float)compressedSizeSum, decompframeCounter);
		printf("gzip decompress time measurement is: %0.2f, average: %0.2f, frameCounter: %d\n",((float)(tDecompEnd - tDecompBegin))/1000, ((float)decompTimeDiff/decompframeCounter)/1000, decompframeCounter);
	}
#endif
}
