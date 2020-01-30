#pragma once

#include <iostream>
#include <cstdint>
#include <cstring>
#include <zlib.h>
#include "compressFrameRVL.h"

int compressFrameRVL::EncodeVLE(int value)
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

int compressFrameRVL::compressDepthFrame(unsigned char* buffer, int size, unsigned char* compressedBuf) 
{
	short * buffer2 = (short *)buffer;
	int * pHead =  pBuffer = (int*)compressedBuf + 1;
	nibblesWritten = 0;
	short *end = buffer2 + size/2;// size/2 = numPixels
	short previous = 0;
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
	printf("finish compression with RVL, full size: %u, compressed size: %lu\n",size, compressedSize);	
	memcpy(compressedBuf, &compressedSize, sizeof(unsigned int));
	return compressedSize;
}
