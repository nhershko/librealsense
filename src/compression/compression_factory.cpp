#pragma once

#include "rvl_compression.h"
#include "gzip_compression.h"
#include "jpeg_compression.h"
#include "compression_factory.h"

ICompression* CompressionFactory::create(zipMethod  zipMeth, int width, int height, rs2_format format)
{
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
		default:
			printf("unknown zip method\n");
			return nullptr;
	}
}