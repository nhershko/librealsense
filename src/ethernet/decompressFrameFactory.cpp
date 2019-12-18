#pragma once

#include <iostream>
#include <cstdint>
#include <cstring>
#include <zlib.h>
#include "decompressFrameRVL.h"
#include "decompressFrameGzip.h"
#include "decompressFrameFactory.h"

IdecompressFrame* decompressFrameFactory::create(zipMethod  zipMeth)
{
	switch(zipMeth)
	{
		case zipMethod::gzip:
			return new decompressFrameGzip();
			break;
		case zipMethod::rvl:
			return new decompressFrameRVL();
			break;
		default:
			printf("unknown zip method\n");
			return nullptr;
	}
}