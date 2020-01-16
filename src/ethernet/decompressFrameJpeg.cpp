#pragma once

#include <iomanip>
#include <cstdint>
#include <cstring>
#include "decompressFrameJpeg.h"
#include <stdio.h>
#include "jpeglib.h"

	//TODO: add this code for RGB format
	// (void) jpeg_read_scanlines(&cinfo, destBuffer, 1);
	// for (int i = 0; i < cinfo.output_width h; i ++) {
	// 	ptr[i] = destBuffer[0][i];
    //     ptr[i+ 1] = destBuffer[0][i +1];
    //     ptr[i+ 2] = destBuffer[0][i +2];
	// }
	// ptr += cinfo.output_width  * cinfo.output_component;
	//(void) jpeg_read_scanlines(&cinfo, destBuffer, 1);
	//memcpy(ptr,destBuffer[0], 640*3);
	//ptr+= 640*3;

void  decompressFrameJpeg::decompressColorFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf) 
{	
	struct jpeg_error_mgr jerr;
	struct jpeg_decompress_struct cinfo;
	unsigned char * ptr =  uncompressedBuf;
	uint64_t compressedSize = 0;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	memcpy(&compressedSize, buffer, sizeof(unsigned int));
	unsigned char* data = buffer + sizeof(unsigned int);
	jpeg_mem_src(&cinfo, data , compressedSize);
	(void) jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_YCbCr;
	(void) jpeg_start_decompress(&cinfo);

	uint64_t row_stride = cinfo.output_width * cinfo.output_components;
	JSAMPARRAY destBuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	while (cinfo.output_scanline < cinfo.output_height) {
		(void) jpeg_read_scanlines(&cinfo, destBuffer, 1);
        for (int i = 0; i < cinfo.output_width ; i += 2) { //input strides by 4 bytes, output strides by 6 (2 pixels)
			ptr[i*2] = destBuffer[0][i*3]; // Y (unique to this pixel)
            ptr[i*2 + 1] = destBuffer[0][i*3 + 1]; // U (shared between pixels)
            ptr[i*2 + 2] = destBuffer[0][i*3 + 3]; // Y (unique to this pixel)
            ptr[i*2 + 3] = destBuffer[0][i*3 + 2]; // V (shared between pixels)
        }
		ptr+= cinfo.output_width *2;
	}
  
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	printf("finish color decompression with jpeg, full size: %lu , compressed size %u \n", size, compressedSize);
}
