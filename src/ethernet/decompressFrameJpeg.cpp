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
clock_t t1, t2;
float diffsum = 0;
int frameCounter = 0;

decompressFrameJpeg::~decompressFrameJpeg() 
{
	jpeg_destroy_decompress(&cinfo);
}

decompressFrameJpeg::decompressFrameJpeg()
{
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	//destBuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, MAX_INPUT_COMPONENT*MAX_WIDTH, 1);

}

void  decompressFrameJpeg::decompressColorFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf) 
{	
	t1 = clock();
	unsigned char * ptr =  uncompressedBuf;
	unsigned char* data = buffer + sizeof(unsigned int);
	uint64_t compressedSize = 0;
	uint res;
	memcpy(&compressedSize, buffer, sizeof(unsigned int));
	jpeg_mem_src(&cinfo, data , compressedSize);
	memcpy(&res, buffer+sizeof(unsigned int), sizeof(unsigned int));
	if (res != 3774863615) { //workaround for bad frames = E0FF D8FF - the First 4 bytes jpeg standards  
		printf("skip frame\n");
		return;
	}
	int retVal =  jpeg_read_header(&cinfo, TRUE);
	if (!retVal) {
		return;
	}
	cinfo.out_color_space = JCS_YCbCr;
	int r =  jpeg_start_decompress(&cinfo);
	if (!r) {
		printf("error\n");
		return;
	}
	uint64_t row_stride = cinfo.output_width * cinfo.output_components;
	destBuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
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
	t2 = clock() - t1;
	diffsum += t2;
	//printf ("It took me %d clicks (%f miliseconds).\n",t2,((float)t2)/1000);
	//printf("decompress time measurement is: %0.2f , frameCounter: %d\n", ((float)diffsum/frameCounter)/1000, frameCounter);
	frameCounter++;
	printf("finish color decompression with jpeg, full size: %lu , compressed size %u \n", size, compressedSize);
}
