#pragma once

#include <iostream>
#include <cstdint>
#include <cstring>
#include "jpeg_compression.h"
#include <stdio.h>
#include "jpeglib.h"
#include <time.h>
#include <ipDevice_Common/statistic.h>

#define MAX_INPUT_COMPONENT 3

JpegCompression::JpegCompression(int width, int height, rs2_format format)
{
	cinfo.err = jpeg_std_error(&jerr);
	dinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_create_decompress(&dinfo);
	rowBuffer = new unsigned char[MAX_INPUT_COMPONENT * width];
	destBuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, MAX_INPUT_COMPONENT * width, 1);//TODO: use  bpp instead of MAX_INPUT_COMPONENT
	m_format = format;
	m_width = width;
	m_height = height;
	cinfo.input_components = MAX_INPUT_COMPONENT; //TODO:change to bpp
	if (m_format == RS2_FORMAT_YUYV) {
		cinfo.in_color_space = JCS_YCbCr;
		cinfo.input_components = 3; //yuyv is 2 bpp, we need to change to yuv that is 3 bpp
	} else if(RS2_FORMAT_RGB8){
		cinfo.in_color_space = JCS_RGB;
	}else {
		printf("unsupport format on jpeg compression");
		return;
	}
	jpeg_set_defaults(&cinfo);
}

JpegCompression::~JpegCompression() 
{
	jpeg_destroy_decompress(&dinfo);
	jpeg_destroy_compress(&cinfo);
}

void JpegCompression::convertYUYVtoYUV(unsigned char** buffer)
{
	for (int i = 0; i < cinfo.image_width; i += 2) { //input strides by 4 bytes, output strides by 6 (2 pixels)
        rowBuffer[i*3] = (*buffer)[i*2 + 0]; // Y (unique to this pixel)
        rowBuffer[i*3 + 1] = (*buffer)[i*2 + 1]; // U (shared between pixels)
        rowBuffer[i*3 + 2] = (*buffer)[i*2 + 3]; // V (shared between pixels)
        rowBuffer[i*3 + 3] = (*buffer)[i*2 + 2]; // Y (unique to this pixel)
        rowBuffer[i*3 + 4] = (*buffer)[i*2 + 1]; // U (shared between pixels)
        rowBuffer[i*3 + 5] = (*buffer)[i*2 + 3]; // V (shared between pixels)
    }
	row_pointer[0] = rowBuffer;
	(*buffer) += cinfo.image_width * 2; 
}

void JpegCompression::convertYUVtoYUYV(unsigned char** uncompressBuff) 
{
	for (int i = 0; i < dinfo.output_width ; i += 2) {
		(*uncompressBuff)[i*2] = destBuffer[0][i*3]; // Y (unique to this pixel)
        (*uncompressBuff)[i*2 + 1] = destBuffer[0][i*3 + 1]; // U (shared between pixels)
        (*uncompressBuff)[i*2 + 2] = destBuffer[0][i*3 + 3]; // Y (unique to this pixel)
        (*uncompressBuff)[i*2 + 3] = destBuffer[0][i*3 + 2]; // V (shared between pixels)
    }
	(*uncompressBuff)+= dinfo.output_width *2;
}

int JpegCompression::compressBuffer(unsigned char* buffer, int size, unsigned char* compressedBuf)
{	
	long unsigned int compressedSize = 0;
	unsigned char * data;
#ifdef STATISTICS
	statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_COLOR]->compressionBegin = std::chrono::system_clock::now();
#endif
	jpeg_mem_dest(&cinfo, &data, &compressedSize);
	cinfo.image_width = m_width;
	cinfo.image_height = m_height;
	uint64_t row_stride = cinfo.image_width * cinfo.input_components;
	jpeg_start_compress(&cinfo, TRUE);
	while (cinfo.next_scanline < cinfo.image_height) {
		if (m_format == RS2_FORMAT_RGB8) {
			row_pointer[0] = & buffer[cinfo.next_scanline * row_stride];
		} else if(m_format == RS2_FORMAT_YUYV){
			convertYUYVtoYUV(&buffer);
		} else {
			printf("unsupport format on jpeg compression");
			return -1;
		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	
	memcpy(compressedBuf, data, compressedSize);
	if (compframeCounter++%50 == 0) {
		printf("finish jpeg color compression, size: %lu, compressed size %u, frameNum: %d \n", size, compressedSize, compframeCounter);
	}
#ifdef STATISTICS
	stream_statistic * st  = statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_COLOR];
	st->compressionFrameCounter++;
	st->compressionEnd = std::chrono::system_clock::now();
	st->compressionTime = st->compressionEnd - st->compressionBegin;
    st->avgCompressionTime += st->compressionTime.count();
    printf("STATISTICS: streamType: %d, jpeg compress time: %0.2fm, average: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_COLOR, st->compressionTime*1000, 
            (st->avgCompressionTime*1000)/st->compressionFrameCounter,st->compressionFrameCounter);
	st->decompressedSizeSum = size;
	st->compressedSizeSum = compressedSize;
	printf("STATISTICS: streamType: %d, jpeg ratio: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_COLOR,st->decompressedSizeSum/(float)st->compressedSizeSum, st->compressionFrameCounter);
#endif
	return compressedSize;
}


int  JpegCompression::decompressBuffer(unsigned char* buffer, int compressedSize, unsigned char* uncompressedBuf) 
{	
	unsigned char * ptr =  uncompressedBuf;
	unsigned char* data = buffer;
	uint jpegHeader, res;
#ifdef STATISTICS
	statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_COLOR]->decompressionBegin = std::chrono::system_clock::now();
#endif
	jpeg_mem_src(&dinfo, data , compressedSize);
	memcpy(&jpegHeader, buffer, sizeof(unsigned int));
	if (jpegHeader != 0xE0FFD8FF) { //check header integrity if = E0FF D8FF - the First 4 bytes jpeg standards. 
		printf("Error: not a jpeg frame, skip frame\n");
		return -1;
	}
	res =  jpeg_read_header(&dinfo, TRUE);
	if (!res) {
		printf("Error: jpeg_read_header failed\n");
		return -1;
	}
	if (m_format == RS2_FORMAT_RGB8) {
		dinfo.out_color_space = JCS_RGB;
	} else if(m_format == RS2_FORMAT_YUYV){
		dinfo.out_color_space = JCS_YCbCr;
	}
	res =  jpeg_start_decompress(&dinfo);
	if (!res) {
		printf("error: jpeg_start_decompress failed \n");
		return -1;
	}
	uint64_t row_stride = dinfo.output_width * dinfo.output_components;
	while (dinfo.output_scanline < dinfo.output_height) {
		(void) jpeg_read_scanlines(&dinfo, destBuffer, 1);
		if (m_format == RS2_FORMAT_RGB8) {
			for (int i = 0; i < dinfo.output_width; i ++) {
		 		ptr[i] = destBuffer[0][i];
		  		ptr[i+ 1] = destBuffer[0][i +1];
		    	ptr[i+ 2] = destBuffer[0][i +2];
		    }
			ptr += dinfo.output_width  * dinfo.output_components;
			//memcpy(ptr,destBuffer[0], row_stride);
			//ptr+= row_stride;
		} else if(m_format == RS2_FORMAT_YUYV){
			convertYUVtoYUYV(&ptr);
		} else {
			printf("unsupport format on jpeg compression");
			return -1;
		}
	}
	(void) jpeg_finish_decompress(&dinfo);
	int uncompressedSize = dinfo.output_width*dinfo.output_height*2;//TODO: change to bpp 
	if (decompframeCounter++%50 == 0) {
		printf("finish jpeg color decompression, size: %lu, compressed size %u, frameNum: %d \n",uncompressedSize, compressedSize, decompframeCounter);
	}

#ifdef STATISTICS
	stream_statistic * st  = statistic::getStatisticStreams()[rs2_stream::RS2_STREAM_COLOR];
	st->decompressionFrameCounter++;
	st->decompressionEnd = std::chrono::system_clock::now();
	st->decompressionTime = st->decompressionEnd - st->decompressionBegin;
    st->avgDecompressionTime += st->decompressionTime.count();
    printf("STATISTICS: streamType: %d, jpeg decompress time: %0.2fm, average: %0.2fm, counter: %d\n",rs2_stream::RS2_STREAM_COLOR, st->decompressionTime*1000, 
            (st->avgDecompressionTime*1000)/st->decompressionFrameCounter,st->decompressionFrameCounter);
#endif
	return uncompressedSize;
}
