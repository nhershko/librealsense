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
	
	unsigned int compressedSize;
	int windowsBits = 15;
	int GZIP_ENCODING = 16;
	memcpy(&compressedSize, buffer,sizeof(unsigned int));
	assert(compressedSize < size);
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = (Bytef *)buffer + sizeof(unsigned int);
	strm.avail_in =  size;
	strm.next_out = (Bytef *)uncompressedBuf;
	strm.avail_out = size;
	int z_result = inflateInit2(&strm, windowsBits | GZIP_ENCODING);
	z_result = inflate(&strm, Z_FINISH);
	//assert(z_result != Z_STREAM_ERROR );
	//assert(z_result != Z_BUF_ERROR);
	//assert(z_result == Z_STREAM_END);
	inflateEnd(&strm);
	printf("finish decompression, full size: %lu , compressed size %u \n", strm.total_out, compressedSize);	

	//statistic:
	fullSizeSum += size;
	compressedSizeSum += compressedSize;
	float zipRatio = fullSizeSum/(float)compressedSizeSum;
	frameCounter++;
	printf("gzip zip ratio is: %0.2f , frameCounter: %d\n", zipRatio, frameCounter);
}