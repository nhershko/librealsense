#include <iostream>
#include <cstring>
#include <zlib.h>
#include "gzip_compression.h"
#include <ipDevice_Common/statistic.h>

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
#ifdef STATISTICS
	statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH]->compressionBegin = std::chrono::system_clock::now();
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
	if (compframeCounter++%50 == 0) {
		printf("finish gzip depth compression, size: %lu, compressed size %u, frameNum: %d \n",size, compressedSize, compframeCounter);
	}
#ifdef STATISTICS
	stream_statistic * st  = statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH];
	st->compressionFrameCounter++;
	std::chrono::system_clock::time_point compressionEnd = std::chrono::system_clock::now();
	st->compressionTime = compressionEnd - st->compressionBegin;
    st->avgCompressionTime += st->compressionTime.count();
    printf("STATISTICS: streamType: %d, gzip compression time: %0.2fm, average: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_DEPTH, st->compressionTime*1000, 
            (st->avgCompressionTime*1000)/st->compressionFrameCounter,st->compressionFrameCounter);
	st->decompressedSizeSum = size;
	st->compressedSizeSum = compressedSize;
	printf("STATISTICS: streamType: %d, gzip ratio: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_DEPTH,st->decompressedSizeSum/(float)st->compressedSizeSum, st->compressionFrameCounter);
#endif
	return strm.total_out;
}

int  GzipCompression::decompressBuffer(unsigned char* buffer, int compressedSize, unsigned char* uncompressedBuf) 
{	
#ifdef STATISTICS
	statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH]->decompressionBegin = std::chrono::system_clock::now();
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
		return -1;
	}
	inflateEnd(&strm);
	if (decompframeCounter++%50 == 0) {
		printf("finish gzip depth decompression, size: %lu, compressed size %u, frameNum: %d \n", strm.total_out, compressedSize, decompframeCounter);
	}
#ifdef STATISTICS
	stream_statistic * st  = statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH];
	st->decompressionFrameCounter++;
	std::chrono::system_clock::time_point decompressionEnd = std::chrono::system_clock::now();
	st->decompressionTime = decompressionEnd - st->decompressionBegin;
    st->avgDecompressionTime += st->decompressionTime.count();
    printf("STATISTICS: streamType: %d, gzip decompression time: %0.2fm, average: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_DEPTH, st->decompressionTime*1000, 
            (st->avgDecompressionTime*1000)/st->decompressionFrameCounter,st->decompressionFrameCounter);
#endif
	return strm.total_out;
}
