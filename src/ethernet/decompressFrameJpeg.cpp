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
#include "decompressFrameJpeg.h"
#include <stdio.h>
#include "jpeglib.h"
#include <vector>

int image_counter = 0;

void  decompressFrameJpeg::decompressColorFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf) 
{	


	struct jpeg_error_mgr jerr;
	struct jpeg_decompress_struct cinfo;
	JSAMPARRAY destBuffer;
	unsigned char * ptr =  uncompressedBuf;
	int row_stride;	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	uint64_t compressedSize = 0;
	memcpy(&compressedSize, buffer, sizeof(unsigned int));
	unsigned char* data = buffer + sizeof(unsigned int);
	// image_counter++;
	// char filename [20];
	// snprintf (filename, 20, "client_image_1_%02d", image_counter);
	// FILE * f = fopen(filename, "wb");
	// fwrite(data,compressedSize, 1,f);
	// fclose(f);
	jpeg_mem_src(&cinfo, data , compressedSize);
	(void) jpeg_read_header(&cinfo, TRUE);
	printf("jpeg_color_space %d\n", cinfo.jpeg_color_space);
	printf("out_color_space %d\n", cinfo.out_color_space);
	(void) jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	destBuffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	JSAMPROW row_pointer[1];
	// unsigned char *tmprowbuf = new unsigned char[cinfo.image_width * 2]; 
	// row_pointer[0] = tmprowbuf;
	unsigned char *jdata = (unsigned char *)malloc( cinfo.output_width * cinfo.output_components *3 );
    unsigned char * rowptr[1];
	while (cinfo.output_scanline < cinfo.output_height) {
		// (void) jpeg_read_scanlines(&cinfo, destBuffer, 1);
        // for (int i = 0; i < cinfo.image_width; i += 2) { //input strides by 4 bytes, output strides by 6 (2 pixels)
		// 	tmprowbuf[i*2] = destBuffer[0][i*3]; // Y (unique to this pixel)
        //     tmprowbuf[i*2 + 1] = destBuffer[0][i*3 + 4]; // U (shared between pixels)
        //     tmprowbuf[i*2 + 2] = destBuffer[0][i*3 + 3]; // Y (unique to this pixel)
        //     tmprowbuf[i*2 + 3] = 0; // destBuffer[0][i*3 + 5]; // V (shared between pixels)
        // }
		// row_pointer[0] = tmprowbuf; 
		// memcpy(ptr, row_pointer[0], cinfo.image_width*2);
		// ptr+= cinfo.image_width *2;
		rowptr[0] = uncompressedBuf +  3 * cinfo.output_width * cinfo.output_scanline; 
		(void) jpeg_read_scanlines(&cinfo, rowptr, 1);
		//memcpy(ptr,destBuffer[0], 640*3);
		//ptr+= 640*3;
	}
#if 0
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
#endif	
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	printf("finish color compression, full size: %lu , compressed size %u \n", size, compressedSize);

	image_counter++;
	char filename [20];
	snprintf (filename, 20, "client_row_1_%02d.yuv", image_counter);
	FILE * f = fopen(filename, "wb");
	fwrite(uncompressedBuf,size, 1,f);
	fclose(f);	
}