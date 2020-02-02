#pragma once

#include <iostream>
#include <cstdint>
#include <cstring>
#include <zlib.h>
#include "rvl_compression.h"

 RvlCompression::RvlCompression(int width, int height, rs2_format format)
 {
	 	m_format = format;
		m_width = width;
		m_height = height;
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
	int bpp = 2;
	short * buffer2 = (short *)buffer;
	int * pHead =  pBuffer = (int*)compressedBuf + 1;
	nibblesWritten = 0;
	short *end = buffer2 + size/bpp;
	short previous = 0;
#ifdef COMPRESSION_STATISTICS
	tCompBegin = clock();
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
	if (compframeCounter%50 == 0) {
		printf("finish rvl depth compression, size: %lu, compressed size %u, frameNum: %d \n", size, compressedSize, compframeCounter);
	}
	memcpy(compressedBuf, &compressedSize, sizeof(unsigned int));
#ifdef COMPRESSION_STATISTICS	
	tCompEnd = clock();
	int diff = tCompEnd - tCompBegin;
	compTimeDiff += diff;
	if (compframeCounter%50 == 0) {
		printf("rvl compress time measurement is: %0.2f, average: %0.2f, frameCounter: %d\n",((float)diff)/1000, ((float)compTimeDiff/compframeCounter)/1000, compframeCounter);
	}
	compframeCounter++;
#endif
	return compressedSize;
}


void RvlCompression::decompressBuffer(unsigned char* buffer, int size, unsigned char* uncompressedBuf) 
{
	short* uncompressedBuf2 = (short*)uncompressedBuf;
	pBuffer = (int*)buffer + 1;
	nibblesWritten = 0;
	short current, previous = 0;
	unsigned int compressedSize;
#ifdef COMPRESSION_STATISTICS
	tDecompBegin = clock();
#endif
	memcpy(&compressedSize, buffer, sizeof(unsigned int));
	int numPixelsToDecode = size/2;
	while (numPixelsToDecode)
	{
		int zeros = decodeVLE();
		numPixelsToDecode -= zeros;
		for (; zeros; zeros--)
			*uncompressedBuf2++ = 0;
		int nonzeros = decodeVLE();
		numPixelsToDecode -= nonzeros;
		for (; nonzeros; nonzeros--)
		{
			int positive = decodeVLE();
			int delta = (positive >> 1) ^ -(positive & 1);
			current = previous + delta;
			*uncompressedBuf2++ = current;
			previous = current;
		}
	}
	if (decompframeCounter%50 == 0) {
		printf("finish rvl depth compression, size: %lu, compressed size %u, frameNum: %d \n", size, compressedSize, decompframeCounter);
	}
#ifdef COMPRESSION_STATISTICS
	tDecompEnd = clock();
	decompTimeDiff += tDecompEnd - tDecompBegin;

	fullSizeSum += size;
	compressedSizeSum += compressedSize;
	decompframeCounter++;

	if (decompframeCounter%50 == 0) {
		printf("rvl decompress zip ratio is: %0.2f , frameCounter: %d\n", fullSizeSum/(float)compressedSizeSum, decompframeCounter);
		printf("rvl decompress time measurement is: %0.2f, average: %0.2f, frameCounter: %d\n",((float)(tDecompEnd - tDecompBegin))/1000, ((float)decompTimeDiff/decompframeCounter)/1000, decompframeCounter);
	}
#endif
}
