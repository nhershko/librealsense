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
	strm.next_out = (Bytef *)compressedBuf + sizeof(unsigned int);
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
	deflateEnd(&strm);
	unsigned int compressedSize = strm.total_out;
	memcpy(compressedBuf, &compressedSize, sizeof(unsigned int));
	if (compframeCounter%50 == 0) {
		printf("finish depth compression with gzip, full size: %lu , compressed size %u, frame counter: \n", size, compressedSize, compframeCounter);
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

void  GzipCompression::decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf) 
{	
	unsigned int compressedSize;
#ifdef COMPRESSION_STATISTICS
	tDecompBegin = clock();
#endif
	memcpy(&compressedSize, buffer,sizeof(unsigned int));
	if(compressedSize > size){
		printf("error: corrupted frame\n");
		return;
	}
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = (Bytef *)buffer + sizeof(unsigned int);
	strm.avail_in =  size;
	strm.next_out = (Bytef *)uncompressedBuf;
	strm.avail_out = size;
	int z_result = inflateInit2(&strm, windowsBits | GZIP_ENCODING);
	z_result = inflate(&strm, Z_FINISH);
	if(z_result != Z_STREAM_END) {
		printf("error: decompress frame with gzip failed\n");
		return;
	}
	inflateEnd(&strm);
	if (decompframeCounter%50 == 0) {
		printf("finish color decompression with gzip, full size: %lu , compressed size %u, frame counter: \n", strm.total_out, compressedSize, decompframeCounter);
	}
#ifdef COMPRESSION_STATISTICS
	tDecompEnd = clock();
	decompTimeDiff += tDecompEnd - tDecompBegin;

	fullSizeSum += size;
	compressedSizeSum += compressedSize;
	decompframeCounter++;
	if (decompframeCounter%50 == 0) {
		printf("gzip decompress zip ratio is: %0.2f , frameCounter: %d\n", fullSizeSum/(float)compressedSizeSum, decompframeCounter);
		printf("gzip decompress time measurement is: %0.2f, average: %0.2f, frameCounter: %d\n",((float)(tDecompEnd - tDecompBegin))/1000, ((float)decompTimeDiff/decompframeCounter)/1000, decompframeCounter);
	}
#endif
}