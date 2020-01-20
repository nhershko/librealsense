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


decompressFrameJpeg::~decompressFrameJpeg() 
{
	jpeg_destroy_decompress(&cinfo);
}

decompressFrameJpeg::decompressFrameJpeg()
{
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	//TODO:: check if we can alocate this buffer here 
	//destBuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, MAX_INPUT_COMPONENT*MAX_WIDTH, 1);

}

void  decompressFrameJpeg::decompressColorFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf) 
{	
	unsigned char * ptr =  uncompressedBuf;
	unsigned char* data = buffer + sizeof(unsigned int);
	uint64_t compressedSize = 0;
	uint jpegHeader, res;
#ifdef COMPRESSION_STATISTICS
	t1 = clock();
#endif

	memcpy(&compressedSize, buffer, sizeof(unsigned int));
	jpeg_mem_src(&cinfo, data , compressedSize);
	memcpy(&jpegHeader, buffer+sizeof(unsigned int), sizeof(unsigned int));
	//check header integrity if = E0FF D8FF - the First 4 bytes jpeg standards.(workaround for bad frames). 
	if (jpegHeader != 3774863615) {  
		printf("skip frame\n");
		return;
	}
	res =  jpeg_read_header(&cinfo, TRUE);
	if (!res) {
		return;
	}
	cinfo.out_color_space = JCS_YCbCr;
	res =  jpeg_start_decompress(&cinfo);
	if (!res) {
		printf("error\n");
		return;
	}
	uint64_t row_stride = cinfo.output_width * cinfo.output_components;
	destBuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	while (cinfo.output_scanline < cinfo.output_height) {
		(void) jpeg_read_scanlines(&cinfo, destBuffer, 1);
		//convert from YUV to YUYV
        for (int i = 0; i < cinfo.output_width ; i += 2) {
			ptr[i*2] = destBuffer[0][i*3]; // Y (unique to this pixel)
            ptr[i*2 + 1] = destBuffer[0][i*3 + 1]; // U (shared between pixels)
            ptr[i*2 + 2] = destBuffer[0][i*3 + 3]; // Y (unique to this pixel)
            ptr[i*2 + 3] = destBuffer[0][i*3 + 2]; // V (shared between pixels)
        }
		ptr+= cinfo.output_width *2;
	}
//TODO: workaround for bad frame
if (compressedSize > size){
	compressedSize=0;
} 
	(void) jpeg_finish_decompress(&cinfo);
#ifdef COMPRESSION_STATISTICS	
	t2 = clock();
	int diff = t2 - t1;
	diffSum += diff;
	printf ("It took me %d clicks (%f miliseconds).\n",diff,((float)diff)/1000);
	printf("decompress time measurement is: %0.2f , frameCounter: %d\n", ((float)diffSum/frameCounter)/1000, frameCounter);
	frameCounter++;

	fullSizeSum += size;
	compressedSizeSum += compressedSize;
	float zipRatio = fullSizeSum/(float)compressedSizeSum;
	frameCounter++;
	printf("gzip zip ratio is: %0.2f , frameCounter: %d %d %d\n", zipRatio, frameCounter, fullSizeSum,compressedSizeSum );
#endif
	printf("finish color decompression with jpeg, full size: %lu , compressed size %u \n", size, compressedSize);
}
