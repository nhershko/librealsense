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
int client_image_counter = 0;

unsigned char  data [614400];
void decompressColorFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf);

int compressFrameJpeg::compressColorFrame(unsigned char* buffer, int size, unsigned char* compressedBuf)
{	
	// unsigned char *tmprowbuf = new unsigned char[640*480*3]; 
    //     for (int i = 0; i < 640*480; i += 2) { //input strides by 4 bytes, output strides by 6 (2 pixels)
    //         tmprowbuf[i*3] = buffer[i*2 + 0]; // Y (unique to this pixel)
    //         tmprowbuf[i*3 + 1] = buffer[i*2 + 1]; // U (shared between pixels)
    //         tmprowbuf[i*3 + 2] = buffer[i*2 + 3]; // V (shared between pixels)
    //         tmprowbuf[i*3 + 3] = buffer[i*2 + 2]; // Y (unique to this pixel)
    //         tmprowbuf[i*3 + 4] = buffer[i*2 + 1]; // U (shared between pixels)
    //         tmprowbuf[i*3 + 5] = buffer[i*2 + 3]; // V (shared between pixels)
    //     }

	//  unsigned char *tmprowbuf2 = new unsigned char[640*480*2]; 
    // 	for (int i = 0; i < 640*480; i += 2) { //input strides by 4 bytes, output strides by 6 (2 pixels)
	// 	tmprowbuf2[i*2] = tmprowbuf[i*3]; // Y (unique to this pixel)
    // 	tmprowbuf2[i*2 + 1] = tmprowbuf[i*3 + 1]; // U (shared between pixels)
    //     tmprowbuf2[i*2 + 2] = tmprowbuf[i*3 + 3]; // Y (unique to this pixel)
    //     tmprowbuf2[i*2 + 3] = tmprowbuf[i*3 + 2]; // V (shared between pixels)
    // }
	// printf("finish color compression\n");
	// image_counter++;
	// char filename [20];
	// snprintf (filename, 20, "image_1_YUV_%02d.yuv", image_counter);
	// FILE * f = fopen(filename, "wb");
	// fwrite(tmprowbuf, 640*480*3, 1,f);
	// fclose(f);

	// char filename2 [20];
	// snprintf (filename2, 20, "image_1_YUYV_%02d.yuv", image_counter);
	// FILE * f1 = fopen(filename2, "wb");
	// fwrite(tmprowbuf2, size, 1,f1);
	// fclose(f1);
	// return 0;
	char filename2 [20];
	snprintf (filename2, 20, "image_1_YUYV_%02d.yuv", image_counter);
	FILE * f1 = fopen(filename2, "wb");
	fwrite(buffer, size, 1,f1);
	fclose(f1);
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
	//jpeg_set_quality(&cinfo, 100, TRUE /* limit to baseline-JPEG values */);

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
	image_counter++;
	char filename [20];
	snprintf (filename, 20, "jpeg_%02d.jpg", image_counter);
	FILE * f = fopen(filename, "wb");
	fwrite(data, compressedSize + sizeof(unsigned int), 1,f);
	fclose(f);
	unsigned char * newBuff = new unsigned char[640*480*2];
	decompressColorFrame(compressedBuf, size, newBuff);
	return compressedSize;
}

void decompressColorFrame(unsigned char* buffer, int size, unsigned char* uncompressedBuf) 
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
	client_image_counter++;
	char filename1 [20];
	snprintf (filename1, 20, "client_jpeg1_%02d", image_counter);
	FILE * f1 = fopen(filename1, "wb");
	fwrite(data,compressedSize, 1,f1);
	fclose(f1);
	jpeg_mem_src(&cinfo, data , compressedSize);
	(void) jpeg_read_header(&cinfo, TRUE);
	//printf("jpeg_color_space %d\n", cinfo.jpeg_color_space);
	//printf("out_color_space %d\n", cinfo.out_color_space);
	(void) jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	destBuffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	JSAMPROW row_pointer[1];
	 unsigned char *tmprowbuf = new unsigned char[cinfo.image_width * 2]; 
	 row_pointer[0] = tmprowbuf;
	//unsigned char *jdata = new unsigned char[cinfo.image_width * 3];
    //unsigned char * rowptr[1];
	while (cinfo.output_scanline < cinfo.output_height) {
		//(void) jpeg_read_scanlines(&cinfo, destBuffer, 1);
		//memcpy(ptr,destBuffer[0], cinfo.image_width * 3);
		//ptr += cinfo.image_width * 3;

		(void) jpeg_read_scanlines(&cinfo, destBuffer, 1);
        for (int i = 0; i < cinfo.image_width; i += 2) { //input strides by 4 bytes, output strides by 6 (2 pixels)
			tmprowbuf[i*2] = destBuffer[0][i*3 ]; // Y (unique to this pixel)
            tmprowbuf[i*2 + 1] = destBuffer[0][i*3 + 1]; // U (shared between pixels)
            tmprowbuf[i*2 + 2] = destBuffer[0][i*3 + 3]; // Y (unique to this pixel)
            tmprowbuf[i*2 + 3] = destBuffer[0][i*3 + 2]; // V (shared between pixels)
        }
		row_pointer[0] = tmprowbuf; 
		memcpy(ptr, row_pointer[0], cinfo.image_width*2);
		ptr+= cinfo.image_width *2;
		//rowptr[0] = uncompressedBuf +  3 * cinfo.output_width * cinfo.output_scanline; 
		//(void) jpeg_read_scanlines(&cinfo, rowptr, 1);
		//memcpy(ptr,destBuffer[0], 640*3);
		//ptr+= 640*3;
	}
  
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	printf("finish color decompression, full size: %lu , compressed size %u \n", size, compressedSize);

	//image_counter++;
	char filename [20];
	snprintf (filename, 20, "client_YUYV_%02d.yuv", image_counter);
	FILE * f = fopen(filename, "wb");
	fwrite(uncompressedBuf,size, 1,f);
	fclose(f);	
}