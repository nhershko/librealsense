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
	JSAMPROW row_pointer[1];
	unsigned char * ptr =  uncompressedBuf;
	uint64_t row_stride, compressedSize = 0;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	memcpy(&compressedSize, buffer, sizeof(unsigned int));
	unsigned char* data = buffer + sizeof(unsigned int);
	jpeg_mem_src(&cinfo, data , compressedSize);
	(void) jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_YCbCr;
	(void) jpeg_start_decompress(&cinfo);

	row_stride = cinfo.output_width * cinfo.output_components;
	destBuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	unsigned char *tmprowbuf = new unsigned char[row_stride]; 
	//row_pointer[0] = tmprowbuf;
	while (cinfo.output_scanline < cinfo.output_height) {
		// (void) jpeg_read_scanlines(&cinfo, destBuffer, 1);
		// for (int i = 0; i < cinfo.output_width h; i ++) {
		// 	tmprowbuf[i] = destBuffer[0][i];
        //     tmprowbuf[i+ 1] = destBuffer[0][i +1];
        //     tmprowbuf[i+ 2] = destBuffer[0][i +2];
		// }
		// row_pointer[0] = tmprowbuf; 
		// memcpy(ptr,destBuffer[0], cinfo.output_width  * cinfo.output_component);
		// ptr += cinfo.output_width  * cinfo.output_component;
		(void) jpeg_read_scanlines(&cinfo, destBuffer, 1);
        for (int i = 0; i < cinfo.output_width ; i += 2) { //input strides by 4 bytes, output strides by 6 (2 pixels)
			tmprowbuf[i*2] = destBuffer[0][i*3]; // Y (unique to this pixel)
            tmprowbuf[i*2 + 1] = destBuffer[0][i*3 + 1]; // U (shared between pixels)
            tmprowbuf[i*2 + 2] = destBuffer[0][i*3 + 3]; // Y (unique to this pixel)
            tmprowbuf[i*2 + 3] = destBuffer[0][i*3 + 2]; // V (shared between pixels)
        }
		row_pointer[0] = tmprowbuf; 
		memcpy(ptr, row_pointer[0], cinfo.output_width*2);
		ptr+= cinfo.output_width *2;
		//rowptr[0] = uncompressedBuf +  3 * cinfo.output_width * cinfo.output_scanline; 
		//(void) jpeg_read_scanlines(&cinfo, rowptr, 1);
		//memcpy(ptr,destBuffer[0], 640*3);
		//ptr+= 640*3;
	}
  
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	printf("finish color decompression with jpeg, full size: %lu , compressed size %u \n", size, compressedSize);
}
