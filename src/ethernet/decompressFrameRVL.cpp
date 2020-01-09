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
#include "decompressFrameRVL.h"

int decompressFrameRVL::decodeVLE()
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

void decompressFrameRVL::decompressDepthFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf) 
{
	short* uncompressedBuf2 = (short*)uncompressedBuf;
	pBuffer = (int*)buffer + 1;
	nibblesWritten = 0;
	short current, previous = 0;
	unsigned int compressedSize;
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
	printf("finish decompression, full size: %lu , compressed size %d \n",size, compressedSize);

	//statistic:
	fullSizeSum += size;
	compressedSizeSum += compressedSize;
	float zipRatio = fullSizeSum/(float)compressedSizeSum;
	frameCounter++;
	printf("rvl zip ratio is: %0.2f , frameCounter: %d\n", zipRatio, frameCounter);
}