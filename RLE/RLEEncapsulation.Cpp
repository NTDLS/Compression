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
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#include "RLEEncapsulation.H"
#include "RLECompression.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _USE_GLOBAL_MEMPOOL
#include "../../../@Libraries/CMemPool/CMemPool.H"
extern CMemPool *pMem; //gMem must be defined and initalized elsewhere.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RLEEncodeFile(const char *sSource, const char *sTarget)
{
	FILE *fSource = NULL;
	FILE *fTarget = NULL;

    char *sBuffer = NULL;
    char *sEncoded = NULL;

	int iBytesRead = 0;
	int iBufferSz = (1024 * 1024);
	int iAlloc = iBufferSz;
	int iOutSz = iBufferSz;

	if(fopen_s(&fSource, sSource, "rb") != 0)
	{
		return RLE_ERROR_FILE_READ;
	}

	if(fopen_s(&fTarget, sTarget, "wb") != 0)
	{
		fclose(fSource);
		return RLE_ERROR_FILE_WRITE;
	}

#ifdef _USE_GLOBAL_MEMPOOL
	if((sBuffer = (char *) pMem->Allocate(iBufferSz + 1, sizeof(char))) == NULL)
#else
	if((sBuffer = (char *) calloc(iBufferSz + 1, sizeof(char))) == NULL)
#endif
	{
		fclose(fSource);
		fclose(fTarget);
		return RLE_ERROR_OUT_OF_MEMORY;
	}

#ifdef _USE_GLOBAL_MEMPOOL
	if((sEncoded = (char *) pMem->Allocate(iBufferSz + 1, sizeof(char))) == NULL)
#else
	if((sEncoded = (char *) calloc(iBufferSz + 1, sizeof(char))) == NULL)
#endif
	{
		fclose(fSource);
		fclose(fTarget);
		
		#ifdef _USE_GLOBAL_MEMPOOL
			pMem->Free(sBuffer);
		#else
			free(sBuffer);
		#endif

		return RLE_ERROR_OUT_OF_MEMORY;
	}

	do{
		iBytesRead = (int)fread(sBuffer, sizeof(char), iBufferSz, fSource);
		sEncoded = RLEEncode(sBuffer, sEncoded, iBytesRead, &iOutSz, &iAlloc);
		fwrite(sEncoded, sizeof(char), iOutSz, fTarget);
	} while(iBytesRead == iBufferSz);

	fclose(fSource);
	fclose(fTarget);
		
#ifdef _USE_GLOBAL_MEMPOOL
	pMem->Free(sBuffer);
	pMem->Free(sEncoded);
#else
	free(sBuffer);
	free(sEncoded);
#endif

	return RLE_ERROR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RLEDecodeFile(const char *sSource, const char *sTarget)
{
	FILE *fSource = NULL;
	FILE *fTarget = NULL;

    char *sBuffer = NULL;
    char *sEncoded = NULL;

	int iBytesRead = 0;
	int iBufferSz = (1024 * 1024);
	int iAlloc = iBufferSz;
	int iOutSz = iBufferSz;

	if(fopen_s(&fSource, sSource, "rb") != 0)
	{
		return RLE_ERROR_FILE_READ;
	}

	if(fopen_s(&fTarget, sTarget, "wb") != 0)
	{
		fclose(fSource);
		return RLE_ERROR_FILE_WRITE;
	}

#ifdef _USE_GLOBAL_MEMPOOL
	if((sBuffer = (char *) pMem->Allocate(iBufferSz + 1, sizeof(char))) == NULL)
#else
	if((sBuffer = (char *) calloc(iBufferSz + 1, sizeof(char))) == NULL)
#endif
	{
		fclose(fSource);
		fclose(fTarget);
		return RLE_ERROR_OUT_OF_MEMORY;
	}

#ifdef _USE_GLOBAL_MEMPOOL
	if((sEncoded = (char *) pMem->Allocate(iBufferSz + 1, sizeof(char))) == NULL)
#else
	if((sEncoded = (char *) calloc(iBufferSz + 1, sizeof(char))) == NULL)
#endif
	{
		fclose(fSource);
		fclose(fTarget);

		#ifdef _USE_GLOBAL_MEMPOOL
			pMem->Free(sBuffer);
		#else
			free(sBuffer);
		#endif

		return RLE_ERROR_OUT_OF_MEMORY;
	}

	do{
		iBytesRead = (int)fread(sBuffer, sizeof(char), iBufferSz, fSource);
		sEncoded = RLEDecode(sBuffer, sEncoded, iBytesRead, &iOutSz, &iAlloc);
		fwrite(sEncoded, sizeof(char), iOutSz, fTarget);
	} while(iBytesRead == iBufferSz);

	fclose(fSource);
	fclose(fTarget);
		
#ifdef _USE_GLOBAL_MEMPOOL
	pMem->Free(sBuffer);
	pMem->Free(sEncoded);
#else
	free(sBuffer);
	free(sEncoded);
#endif

	return RLE_ERROR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif