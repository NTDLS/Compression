///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _LZSSEncapsulation_H
#define _LZSSEncapsulation_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LZSS_ERROR_OK				0
#define LZSS_ERROR_FILE_READ		1
#define LZSS_ERROR_FILE_WRITE		2
#define LZSS_ERROR_OUT_OF_MEMORY	3

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LZSSCompressFile(const char *sSource, const char *sTarget, const int iLevel);
int LZSSUncompressFile(char *sSource, char *sTarget);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
