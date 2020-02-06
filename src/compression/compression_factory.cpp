#pragma once

#include "rvl_compression.h"
#include "gzip_compression.h"
#include "jpeg_compression.h"
#include "lz_compression.h"
#include "compression_factory.h"

ICompression* CompressionFactory::getObject(int width, int height, rs2_format format, rs2_stream stream_type)
{ 	
	zipMethod zipMeth;
	if(stream_type == RS2_STREAM_COLOR || stream_type == RS2_STREAM_INFRARED) {
		zipMeth = zipMethod::Jpeg;
   	} else if(stream_type == RS2_STREAM_DEPTH ) {
        zipMeth = zipMethod::lz;
  	}//todo:set default compression for unknown format 

	switch(zipMeth)
	{
		case zipMethod::gzip:
			return new GzipCompression(width, height, format);
			break;
		case zipMethod::rvl:
			return new RvlCompression(width, height, format);
			break;
		case zipMethod::Jpeg:
			return new JpegCompression(width, height, format);
			break;
		case zipMethod::lz:
			return new LZCompression(width, height, format);
			break;
		default:
			printf("unknown zip method\n");
			return nullptr;
	}
}