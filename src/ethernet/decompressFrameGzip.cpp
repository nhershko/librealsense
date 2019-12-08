#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <functional>
#include <thread>
#include <string>
#include <list> 
#include <iostream>
#include <iomanip>
#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <zlib.h>
#include "decompressFrameGzip.h"

void  decompressFrameGzip::decompressFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf) 
{
	z_stream strm;
	unsigned int compressedSize;
	memcpy(&compressedSize, buffer,sizeof(unsigned int));
	//printf("all buff size %d, compressed size %u \n", size, compressedSize );			
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	int windowsBits = 15;
	int GZIP_ENCODING = 16;
	strm.next_in = (Bytef *)buffer + sizeof(unsigned int);
	strm.avail_in =  size;
	strm.next_out = (Bytef *)uncompressedBuf;
	strm.avail_out = size;
	int z_result = inflateInit2(&strm, windowsBits | GZIP_ENCODING);
	z_result = inflate(&strm, Z_FINISH);
	// assert(z_result != Z_STREAM_ERROR && z_result != Z_BUF_ERROR && z_result == Z_STREAM_END);//fix condition 
	if (z_result == Z_STREAM_ERROR ) printf("ERROR: stream error\n");
	if (z_result == Z_BUF_ERROR )    printf("ERROR: buffer error\n");
	if (z_result != Z_STREAM_END )   printf("ERROR: stream end\n");
	// assert(z_result != Z_STREAM_ERROR );
	// assert(z_result != Z_BUF_ERROR);
	// assert(z_result == Z_STREAM_END);
	inflateEnd(&strm);
	printf("finish decompression, full size: %lu , compressed size %u \n", strm.total_out, compressedSize);	
	fullSizeSum += size;
	compressedSizeSum += compressedSize;
	float zipRatio = fullSizeSum/(float)compressedSizeSum;
	frameCounter++;
	printf("zip ratio is: %0.2f , frameCounter: %d\n", zipRatio, frameCounter);
}