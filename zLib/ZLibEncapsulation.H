///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ZLibEncapsulation_H
#define _ZLibEncapsulation_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "zConf.h"
#include "zLib.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ZLIB_U_ERROR_OK				0
#define ZLIB_U_ERROR_FILE_READ		1
#define ZLIB_U_ERROR_FILE_WRITE		2
#define ZLIB_U_ERROR_OUT_OF_MEMORY	3
#define ZLIB_U_ERROR_COMPRESSION	4

#define ZLIB_CHUNK_SIZE			16384

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GZipInflateFile(const char *sSourceName, const char *sTargetName);
int GZipDeflateFile(const char *sSourceName, const char *sTargetName, int iLevel);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
