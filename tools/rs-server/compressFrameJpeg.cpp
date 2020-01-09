#pragma once

#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <zlib.h>
#include "compressFrameJpeg.h"
#include <stdio.h>
#include "jpeglib.h"
#include <vector>

int image_counter = 0;
unsigned char  data [614400];

int compressFrameJpeg::compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf)
{	

	image_counter++;
	char filename [20];
	snprintf (filename, 20, "raw_yuyv_%02d.yuv", image_counter);
	FILE * f = fopen(filename, "wb");
	fwrite(buffer, size, 1,f);
	fclose(f);
	struct jpeg_error_mgr jerr;
	struct jpeg_compress_struct cinfo;
	JSAMPROW row_pointer[1];
	int row_stride;
	//data = compressedBuf + sizeof(unsigned int);
	//printf("%p,%p\n",compressedBuf,data);
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	uint64_t compressedSize = 0;
	//unsigned char * ptr = data;
	unsigned char * data;
	jpeg_mem_dest(&cinfo, &data, &compressedSize);

	cinfo.image_width = 640;
	cinfo.image_height = 480;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_YCbCr;

	jpeg_set_defaults(&cinfo);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = cinfo.image_width * cinfo.input_components;

	unsigned char *tmprowbuf = new unsigned char[cinfo.image_width * 3]; 
	row_pointer[0] = tmprowbuf;
	while (cinfo.next_scanline < cinfo.image_height) {
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
	// std::vector<uint8_t> tmprowbuf(cinfo.image_width * cinfo.input_components);
	// row_pointer[0] = &tmprowbuf[0];
	// while (cinfo.next_scanline < cinfo.image_height) {
	// 	// row_pointer[0] = & buffer[cinfo.next_scanline * row_stride];
	// 	unsigned i, j;
    //     unsigned offset = cinfo.next_scanline * cinfo.image_width * 2; //offset to the correct row
    //     for (i = 0, j = 0; i < cinfo.image_width * 2; i += 4, j += 6) { //input strides by 4 bytes, output strides by 6 (2 pixels)
    //         tmprowbuf[j + 0] = buffer[offset + i + 0]; // Y (unique to this pixel)
    //         tmprowbuf[j + 1] = buffer[offset + i + 1]; // U (shared between pixels)
    //         tmprowbuf[j + 2] = buffer[offset + i + 3]; // V (shared between pixels)
    //         tmprowbuf[j + 3] = buffer[offset + i + 2]; // Y (unique to this pixel)
    //         tmprowbuf[j + 4] = buffer[offset + i + 1]; // U (shared between pixels)
    //         tmprowbuf[j + 5] = buffer[offset + i + 3]; // V (shared between pixels)
    //     }
	// 	jpeg_write_scanlines(&cinfo, row_pointer, 1);
	// }
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	printf("finish color compression, full size: %lu , compressed size %u \n", size, compressedSize);
	
	memcpy(compressedBuf + 4, data, compressedSize);
	memcpy(compressedBuf, &compressedSize, sizeof(unsigned int));
	// image_counter++;
	// char filename [20];
	// snprintf (filename, 20, "image_yuv_%02d", image_counter);
	// FILE * f = fopen(filename, "wb");
	// fwrite(data, compressedSize + sizeof(unsigned int), 1,f);
	// fclose(f);
	return compressedSize;
}

