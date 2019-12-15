#pragma once

#include <iostream>
#include <cstdint>
#include <cstring>
#include <zlib.h>
#include "compressFrameRVL.h"
#include "compressFrameGzip.h"
#include "compressFrameFactory.h"

IcompressFrame* compressFrameFactory::create(zipMethod  zipMeth)
{
	switch(zipMeth)
	{
		case zipMethod::gzip:
			return new compressFrameGzip();
			break;
		case zipMethod::rvl:
			return new compressFrameRVL();
			break;
		default:
			printf("unknown zip method\n");
			return nullptr;
	}
}