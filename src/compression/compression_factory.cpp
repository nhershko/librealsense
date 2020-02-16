#pragma once

#include "rvl_compression.h"
#include "gzip_compression.h"
#include "jpeg_compression.h"
#include "lz_compression.h"
#include "compression_factory.h"

ICompression* CompressionFactory::getObject(rs2::video_stream_profile stream)
{ 	
	zipMethod zipMeth;
	if(stream.stream_type() == RS2_STREAM_COLOR || stream.stream_type() == RS2_STREAM_INFRARED) {
		zipMeth = zipMethod::Jpeg;
   	} else if(stream.stream_type() == RS2_STREAM_DEPTH ) {
        zipMeth = zipMethod::lz;
  	}//todo:set default compression for unknown format 

	switch(zipMeth)
	{
		case zipMethod::gzip:
			return new GzipCompression(stream);
			break;
		case zipMethod::rvl:
			return new RvlCompression(stream);
			break;
		case zipMethod::Jpeg:
			return new JpegCompression(stream);
			break;
		case zipMethod::lz:
			return new LZCompression(stream);
			break;
		default:
			printf("unknown zip method\n");
			return nullptr;
	}
}