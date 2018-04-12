///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ZLibEncapsulation_CPP
#define _ZLibEncapsulation_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "../../NSWFL/NSWFL.h"

#include "ZLibEncapsulation.H"

using namespace NSWFL::Memory;

#ifdef _USE_GLOBAL_MEMPOOL
extern MemoryPool *pMem; //pMem must be defined and initalized elsewhere.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GZipInflateFile(const char *sSourceName, const char *sTargetName)
{
	char *sInBuf = NULL;
	char *sOutBuf = NULL;

	FILE *fSource = NULL;
	FILE *hTarget = NULL;

	int iResult = Z_OK;

	unsigned int iHave = 0;

    z_stream zStream;

	memset(&zStream, 0, sizeof(zStream));

	if((iResult = inflateInit(&zStream)) != Z_OK)
	{
        return ZLIB_U_ERROR_COMPRESSION;
	}

	if(fopen_s(&fSource, sSourceName, "rb") != 0)
	{
	    deflateEnd(&zStream);
		return ZLIB_U_ERROR_FILE_READ;
	}

	if(fopen_s(&hTarget, sTargetName, "wb") != 0)
	{
	    deflateEnd(&zStream);
		fclose(fSource);
		return ZLIB_U_ERROR_FILE_WRITE;
	}
	
	#ifdef _USE_GLOBAL_MEMPOOL
	if((sInBuf = (char *) pMem->Allocate(ZLIB_CHUNK_SIZE + 1, 1)) == NULL)
	#else
	if((sInBuf = (char *) calloc(ZLIB_CHUNK_SIZE + 1, 1)) == NULL)
	#endif
	{
	    deflateEnd(&zStream);
		fclose(fSource);
		fclose(hTarget);
		return ZLIB_U_ERROR_OUT_OF_MEMORY;
	}

	#ifdef _USE_GLOBAL_MEMPOOL
    if((sOutBuf = (char *) pMem->Allocate(ZLIB_CHUNK_SIZE + 1, 1)) == NULL)
	#else
    if((sOutBuf = (char *) calloc(ZLIB_CHUNK_SIZE + 1, 1)) == NULL)
	#endif
	{
	    deflateEnd(&zStream);
		#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(sInBuf);
		#else
		free(sInBuf);
		#endif
		fclose(fSource);
		fclose(hTarget);
		return ZLIB_U_ERROR_OUT_OF_MEMORY;
	}

    do {
        zStream.avail_in = (uInt) fread(sInBuf, 1, ZLIB_CHUNK_SIZE, fSource);

        if (zStream.avail_in == 0)
		{
            break;
		}

        zStream.next_in = (Bytef *) sInBuf;

        do {
            zStream.avail_out = ZLIB_CHUNK_SIZE;
            zStream.next_out = (Bytef *) sOutBuf;

            if((iResult = inflate(&zStream, Z_NO_FLUSH)) != Z_OK)
			{
				if(iResult != Z_STREAM_END)
				{
					break;
				}
			}

			iHave = (ZLIB_CHUNK_SIZE - zStream.avail_out);

			if (fwrite(sOutBuf, 1, iHave, hTarget) != iHave || ferror(hTarget))
			{
                iResult = Z_ERRNO;
				break;
            }

			if(iResult != Z_OK)
			{
				break;
			}
        } while (zStream.avail_out == 0);
    } while (iResult != Z_STREAM_END);

    inflateEnd(&zStream);

	#ifdef _USE_GLOBAL_MEMPOOL
	pMem->Free(sInBuf);
    pMem->Free(sOutBuf);
	#else
	free(sInBuf);
    free(sOutBuf);
	#endif

	fclose(fSource);
	fclose(hTarget);

	if(iResult == Z_OK)
	{
		return ZLIB_U_ERROR_OK;
	}
	else{
		return ZLIB_U_ERROR_COMPRESSION;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GZipDeflateFile(const char *sSourceName, const char *sTargetName, int iLevel)
{
	char *sInBuf = NULL;
	char *sOutBuf = NULL;

	FILE *fSource = NULL;
	FILE *hTarget = NULL;

	int iFlush = 0;
	int iResult = Z_OK;

	unsigned int iHave = 0;

	z_stream zStream;

	memset(&zStream, 0, sizeof(zStream));

	if((iResult = deflateInit(&zStream, iLevel)) != Z_OK)
	{
        return ZLIB_U_ERROR_COMPRESSION;
	}

	if(fopen_s(&fSource, sSourceName, "rb") != 0)
	{
	    deflateEnd(&zStream);
		return ZLIB_U_ERROR_FILE_READ;
	}

	if(fopen_s(&hTarget, sTargetName, "wb") != 0)
	{
	    deflateEnd(&zStream);
		fclose(fSource);
		return ZLIB_U_ERROR_FILE_WRITE;
	}
	
	#ifdef _USE_GLOBAL_MEMPOOL
	if((sInBuf = (char *) pMem->Allocate(ZLIB_CHUNK_SIZE + 1, 1)) == NULL)
	#else
	if((sInBuf = (char *) calloc(ZLIB_CHUNK_SIZE + 1, 1)) == NULL)
	#endif
	{
	    deflateEnd(&zStream);
		fclose(fSource);
		fclose(hTarget);
		return ZLIB_U_ERROR_OUT_OF_MEMORY;
	}

	#ifdef _USE_GLOBAL_MEMPOOL
    if((sOutBuf = (char *) pMem->Allocate(ZLIB_CHUNK_SIZE + 1, 1)) == NULL)
	#else
    if((sOutBuf = (char *) calloc(ZLIB_CHUNK_SIZE + 1, 1)) == NULL)
	#endif
	{
	    deflateEnd(&zStream);
		#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(sInBuf);
		#else
		free(sInBuf);
		#endif
		fclose(fSource);
		fclose(hTarget);
		return ZLIB_U_ERROR_OUT_OF_MEMORY;
	}

    do {
        zStream.avail_in = (uInt) fread(sInBuf, 1, ZLIB_CHUNK_SIZE, fSource);

		iFlush = feof(fSource) ? Z_FINISH : Z_NO_FLUSH;
        zStream.next_in = (Bytef *)sInBuf;

        do {
            zStream.avail_out = ZLIB_CHUNK_SIZE;
            zStream.next_out = (Bytef *) sOutBuf;

			iResult = deflate(&zStream, iFlush);

			iHave = (ZLIB_CHUNK_SIZE - zStream.avail_out);
            if(fwrite(sOutBuf, 1, iHave, hTarget) != iHave || ferror(hTarget))
			{
                iResult = Z_ERRNO;
				break;
            }
        } while (zStream.avail_out == 0);

		if(iResult != Z_OK)
		{
			break;
		}
	} while (iFlush != Z_FINISH);

    deflateEnd(&zStream);

	#ifdef _USE_GLOBAL_MEMPOOL
	pMem->Free(sInBuf);
    pMem->Free(sOutBuf);
	#else
	free(sInBuf);
    free(sOutBuf);
	#endif
	
	fclose(fSource);
	fclose(hTarget);

	if(iResult == Z_OK)
	{
		return ZLIB_U_ERROR_OK;
	}
	else{
		return ZLIB_U_ERROR_COMPRESSION;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
