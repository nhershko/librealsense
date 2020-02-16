#pragma once

#include <iostream>
#include <cstring>
#include "lz_compression.h"
#include <ipDevice_Common/statistic.h>

 LZCompression::LZCompression(rs2::video_stream_profile &stream)
 {
	m_stream = new rs2::video_stream_profile(stream);
 }

int LZCompression::compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf)
{
#ifdef STATISTICS
	statistic::getStatisticStreams()[m_stream->unique_id()]->compressionBegin = std::chrono::system_clock::now();
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
#ifdef STATISTICS
	stream_statistic * st  = statistic::getStatisticStreams()[m_stream->unique_id()];
	st->compressionFrameCounter++;
	st->compressionEnd = std::chrono::system_clock::now();
	st->compressionTime = st->compressionEnd - st->compressionBegin;
    st->avgCompressionTime += st->compressionTime.count();
    printf("STATISTICS: streamType: %d, lz4 compress time: %0.2fm, average: %0.2fm, counter: %d\n",m_stream->unique_id(), st->compressionTime*1000, 
            (st->avgCompressionTime*1000)/st->compressionFrameCounter,st->compressionFrameCounter);
	st->decompressedSizeSum = size;
	st->compressedSizeSum = compressed_data_size;
	printf("STATISTICS: streamType: %d, lz4 ratio: %0.2fm, counter: %d\n",m_stream->unique_id(),st->decompressedSizeSum/(float)st->compressedSizeSum, st->compressionFrameCounter);
#endif
	return compressed_data_size;
}


int  LZCompression::decompressBuffer(unsigned char* buffer, int compressedSize, unsigned char* uncompressedBuf) 
{	
#ifdef STATISTICS
	statistic::getStatisticStreams()[m_stream->unique_id()]->decompressionBegin = std::chrono::system_clock::now();
#endif
    const int decompressed_size = LZ4_decompress_fast((const char *)buffer, (char *)uncompressedBuf, m_stream->width()* m_stream->height() * 2); //change to bpp
    if (decompressed_size < 0) {
		printf("error: negative result from LZ4_decompress_safe indicates a failure trying to decompress the data\n");
		return -1;
	}
	int original_size = m_stream->width()* m_stream->height() * 2; //change to bpp
    // if (decompressed_size != original_size); 
    // 	printf("Decompressed data is different from original!, decompressed_size: %d original size: %d \n",decompressed_size,  m_width* m_height * 2 );
	if (decompframeCounter++%50 == 0) {
		printf("finish lz depth decompression, size: %lu, compressed size %u, frameNum: %d \n", decompressed_size, compressedSize, decompframeCounter);
	}
#ifdef STATISTICS
	stream_statistic * st  = statistic::getStatisticStreams()[m_stream->unique_id()];
	st->decompressionFrameCounter++;
	st->decompressionEnd = std::chrono::system_clock::now();
	st->decompressionTime = st->decompressionEnd - st->decompressionBegin;
    st->avgDecompressionTime += st->decompressionTime.count();
    printf("STATISTICS: streamType: %d, lz4 decompress time: %0.2fm, average: %0.2fm, counter: %d\n",m_stream->unique_id(), st->decompressionTime*1000, 
            (st->avgDecompressionTime*1000)/st->decompressionFrameCounter,st->decompressionFrameCounter);
#endif
	return decompressed_size;
}
