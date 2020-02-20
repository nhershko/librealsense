#pragma once

#include <iostream>
#include <cstdint>
#include <cstring>
#include <zlib.h>
#include "rvl_compression.h"
#include <ipDevice_Common/statistic.h>

 RvlCompression::RvlCompression(int width, int height, rs2_format format, int bpp)
 {
	 	m_format = format;
		m_width = width;
		m_height = height;
		m_bpp = bpp;
 }


int RvlCompression::EncodeVLE(int value)
{
	do {
		int nibble = value & 0x7; // lower 3 bits
		if (value >>= 3) nibble |= 0x8; // more to come
		word <<= 4;
		word |= nibble;
		if (++nibblesWritten == 8) // output word
		{
		*pBuffer++ = word;
		nibblesWritten = 0;
		word = 0;
		}
	} while (value);
}

int RvlCompression::decodeVLE()
{
	unsigned int nibble;
	int value = 0, bits = 29;
	do
	{
		if (!nibblesWritten)
		{
			word = *pBuffer++; // load word
			nibblesWritten = 8;
		}
		nibble = word & 0xf0000000;
		value |= (nibble << 1) >> bits;
		word <<= 4;
		nibblesWritten--;
		bits -= 3;
	} while (nibble & 0x80000000);
	return value;
}

int RvlCompression::compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf) 
{
	short * buffer2 = (short *)buffer;
	int * pHead =  pBuffer = (int*)compressedBuf + 1;
	nibblesWritten = 0;
	short *end = buffer2 + size/m_bpp;
	short previous = 0;
#ifdef STATISTICS
	statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH]->compressionBegin = std::chrono::system_clock::now();
#endif
	while (buffer2 != end)
	{
		int zeros = 0, nonzeros = 0;
		for (; (buffer2 != end) && !*buffer2; buffer2++, zeros++);
		EncodeVLE(zeros);
		for (short* p = buffer2; (p != end) && *p++; nonzeros++);
		EncodeVLE(nonzeros);
		for (int i = 0; i < nonzeros; i++)
		{
			short current = *buffer2++;
			int delta = current - previous;
			int positive = (delta << 1) ^ (delta >> 31);
			EncodeVLE(positive);
			previous = current;
		}
	}
	if (nibblesWritten) // last few values
		*pBuffer++ = word << 4 * (8 - nibblesWritten);
	int compressedSize = int((char*)pBuffer - (char*)pHead);
	if (compframeCounter++%50 == 0) {
		printf("finish rvl depth compression, size: %d, compressed size %u, frameNum: %d \n", size, compressedSize, compframeCounter);
	}
#ifdef STATISTICS
	stream_statistic * st  = statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH];
	st->compressionFrameCounter++;
	std::chrono::system_clock::time_point compressionEnd = std::chrono::system_clock::now();
	st->compressionTime = compressionEnd - st->compressionBegin;
    st->avgCompressionTime += st->compressionTime.count();
    printf("STATISTICS: streamType: %d, rvl compress time: %0.2fm, average: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_DEPTH, st->compressionTime*1000, 
            (st->avgCompressionTime*1000)/st->compressionFrameCounter,st->compressionFrameCounter);
	st->decompressedSizeSum = size;
	st->compressedSizeSum = compressedSize;
	printf("STATISTICS: streamType: %d, rvl ratio: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_DEPTH, st->decompressedSizeSum/(float)st->compressedSizeSum, st->compressionFrameCounter);
#endif
	memcpy(compressedBuf, &compressedSize, sizeof(compressedSize));
	return compressedSize;
}


int RvlCompression::decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf) 
{
	short* currentPtr = (short*)uncompressedBuf;
	pBuffer = (int*)buffer + 1;
	nibblesWritten = 0;
	short current, previous = 0;
	unsigned int compressedSize;
#ifdef STATISTICS
	statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH]->decompressionBegin = std::chrono::system_clock::now();
#endif
	memcpy(&compressedSize, buffer, sizeof(unsigned int));
	int numPixelsToDecode = size/2;
	while (numPixelsToDecode)
	{
		int zeros = decodeVLE();
		numPixelsToDecode -= zeros;
		for (; zeros; zeros--)
			*currentPtr++ = 0;
		int nonzeros = decodeVLE();
		numPixelsToDecode -= nonzeros;
		for (; nonzeros; nonzeros--)
		{
			int positive = decodeVLE();
			int delta = (positive >> 1) ^ -(positive & 1);
			current = previous + delta;
			*currentPtr++ = current;
			previous = current;
		}
	}
	int uncompressedSize = int((char*)currentPtr - (char*)uncompressedBuf);
	if (decompframeCounter++%50 == 0) {
		printf("finish rvl depth compression, size: %lu, compressed size %u, frameNum: %d \n", uncompressedSize, compressedSize, decompframeCounter);
	}
#ifdef STATISTICS
	stream_statistic * st  = statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_DEPTH];
	st->decompressionFrameCounter++;
	std::chrono::system_clock::time_point decompressionEnd = std::chrono::system_clock::now();
	st->decompressionTime = decompressionEnd - st->decompressionBegin;
    st->avgDecompressionTime += st->decompressionTime.count();
    printf("STATISTICS: streamType: %d, rvl decompress time: %0.2fm, average: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_DEPTH, st->decompressionTime*1000, 
            (st->avgDecompressionTime*1000)/st->decompressionFrameCounter,st->decompressionFrameCounter);
#endif
	return  uncompressedSize;
}
