#pragma once

#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <zlib.h>
#include "compressFrameGzip.h"


int compressFrameGzip::compressDepthFrame(unsigned char* buffer, int size, unsigned char* compressedBuf)
{
	int windowsBits = 15;
	int GZIP_ENCODING = 16;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = (Bytef *)buffer;
	strm.avail_in =  size;
	strm.next_out = (Bytef *)compressedBuf + sizeof(unsigned int);
	strm.avail_out = size;
	int z_result = deflateInit2 (&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,windowsBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);
	z_result = deflate(&strm, Z_FINISH);
	assert(z_result != Z_STREAM_ERROR && z_result != Z_BUF_ERROR);
	assert(z_result == Z_STREAM_END);
	deflateEnd(&strm);
	unsigned int compressedSize = strm.total_out;
	printf("finish depth compression with GZIP, full size: %u, compressed size: %lu\n",size, compressedSize );	
	memcpy(compressedBuf, &compressedSize, sizeof(unsigned int));
	return compressedSize;
}

int compressFrameGzip::compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf, int width, int height, rs2_format format)
{
	compressDepthFrame(buffer, size, compressedBuf);
}
