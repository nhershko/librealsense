#pragma once

#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstring>
#include "compressFrameJpeg.h"
#include <stdio.h>
#include "jpeglib.h"

int image_counter = 0;
int client_image_counter = 0;

int compressFrameJpeg::compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf, int width, int height)
{	
	struct jpeg_error_mgr jerr;
	struct jpeg_compress_struct cinfo;
	JSAMPROW row_pointer[1];
	int row_stride;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	uint64_t compressedSize = 0;
	unsigned char * data;
	int bpp = size/(width*height);
	jpeg_mem_dest(&cinfo, &data, &compressedSize);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_YCbCr;
	jpeg_set_defaults(&cinfo);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = cinfo.image_width * cinfo.input_components;

	unsigned char *tmprowbuf = new unsigned char[cinfo.image_width * cinfo.input_components]; 
	row_pointer[0] = tmprowbuf;
	while (cinfo.next_scanline < cinfo.image_height) {
		//row_pointer[0] = & buffer[cinfo.next_scanline * row_stride];
    	//(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        for (int i = 0; i < cinfo.image_width; i += 2) { //input strides by 4 bytes, output strides by 6 (2 pixels)
            tmprowbuf[i*3] = buffer[i*2 + 0]; // Y (unique to this pixel)
            tmprowbuf[i*3 + 1] = buffer[i*2 + 1]; // U (shared between pixels)
            tmprowbuf[i*3 + 2] = buffer[i*2 + 3]; // V (shared between pixels)
            tmprowbuf[i*3 + 3] = buffer[i*2 + 2]; // Y (unique to this pixel)
            tmprowbuf[i*3 + 4] = buffer[i*2 + 1]; // U (shared between pixels)
            tmprowbuf[i*3 + 5] = buffer[i*2 + 3]; // V (shared between pixels)
        }
		row_pointer[0] = tmprowbuf; 
    	buffer += cinfo.image_width * 2; 
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	printf("finish color compression with jpeg, full size: %lu , compressed size %u \n", size, compressedSize);
	
	memcpy(compressedBuf + 4, data, compressedSize);
	memcpy(compressedBuf, &compressedSize, sizeof(unsigned int));
	return compressedSize;
}
