#pragma once

#include <iostream>
#include <cstdint>
#include <cstring>
#include "compressFrameJpeg.h"
#include <stdio.h>
#include "jpeglib.h"
#include <time.h>

#define MAX_INPUT_COMPONENT 3
#define MAX_WIDTH 1280

compressFrameJpeg::compressFrameJpeg() 
{
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	rowBuffer = new unsigned char[MAX_INPUT_COMPONENT * MAX_WIDTH];
}
compressFrameJpeg::~compressFrameJpeg() 
{
	jpeg_destroy_compress(&cinfo);
}

int compressFrameJpeg::compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf, int width, int height, rs2_format format)
{	
	uint64_t compressedSize = 0;
	unsigned char * data;
#ifdef COMPRESSION_STATISTICS
	t1 = clock();
#endif

	jpeg_mem_dest(&cinfo, &data, &compressedSize);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_YCbCr;
	jpeg_set_defaults(&cinfo);
	jpeg_start_compress(&cinfo, TRUE);
	long unsigned int row_stride = cinfo.image_width * cinfo.input_components;

	while (cinfo.next_scanline < cinfo.image_height) {
		//TODO: add to RGB format compress 
		//row_pointer[0] = & buffer[cinfo.next_scanline * row_stride];
    	//(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
		//Convert from YUYV to YUV
        for (int i = 0; i < cinfo.image_width; i += 2) { //input strides by 4 bytes, output strides by 6 (2 pixels)
            rowBuffer[i*3] = buffer[i*2 + 0]; // Y (unique to this pixel)
            rowBuffer[i*3 + 1] = buffer[i*2 + 1]; // U (shared between pixels)
            rowBuffer[i*3 + 2] = buffer[i*2 + 3]; // V (shared between pixels)
            rowBuffer[i*3 + 3] = buffer[i*2 + 2]; // Y (unique to this pixel)
            rowBuffer[i*3 + 4] = buffer[i*2 + 1]; // U (shared between pixels)
            rowBuffer[i*3 + 5] = buffer[i*2 + 3]; // V (shared between pixels)
        }
		row_pointer[0] = rowBuffer; 
    	buffer += cinfo.image_width * 2; 
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	
	memcpy(compressedBuf + 4, data, compressedSize);
	memcpy(compressedBuf, &compressedSize, sizeof(unsigned int));
#ifdef COMPRESSION_STATISTICS	
	t2 = clock();
	int diff = t2 - t1;
	diffSum += diff;
	printf("compress took %d clicks (%f miliseconds).\n",diff,((float)diff)/1000);
	printf("compress time measurement is: %0.2f , frameCounter: %d\n", ((float)diffSum/frameCounter)/1000, frameCounter);
	frameCounter++;
#endif
	printf("finish color compression with jpeg, full size: %lu , compressed size %u \n", size, compressedSize);
	return compressedSize;
}
