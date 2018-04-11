///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>    
#include <Stdlib.H> 
#include <String.H> 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LZARIClass.H"
#include "LZARIEncapsulation.H"

#ifdef _USE_GLOBAL_MEMPOOL
#include "../../CMemPool/CMemPool.H"
extern CMemPool *pMem; //gMem must be defined and initalized elsewhere.
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LZARICompressFile(const char *sSource, const char *sTarget)
{
    HANDLE hfSource = NULL;
    HANDLE hfTarget = NULL;

    hfSource = CreateFile(
        sSource,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if(hfSource == INVALID_HANDLE_VALUE)
    {
        return LZARI_ERROR_FILE_READ;
    }

    hfTarget = CreateFile(
        sTarget,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if(hfSource == INVALID_HANDLE_VALUE)
    {
		CloseHandle(hfSource);
        return LZARI_ERROR_FILE_WRITE;
    }

    LZARI MyLZARI;

    MyLZARI.Initialize();

    DWORD BytesRead = 0;
    DWORD BytesWritted = 0;
    DWORD DeflSize = 0;

    char *SourceBuf = NULL;
    char *TargetBuf = NULL;

	#ifdef _USE_GLOBAL_MEMPOOL
	if((SourceBuf = (char *) pMem->Allocate(LZARI_BUFFER_SIZE + 1, sizeof(char))) == NULL)
	#else
	if((SourceBuf = (char *) calloc(LZARI_BUFFER_SIZE + 1, sizeof(char))) == NULL)
	#endif
	{
	    CloseHandle(hfSource);
	    CloseHandle(hfTarget);
		return LZARI_ERROR_OUT_OF_MEMORY;
	}

	#ifdef _USE_GLOBAL_MEMPOOL
    if((TargetBuf = (char *) pMem->Allocate((LZARI_BUFFER_SIZE + LZARI_EXTRA_SIZE) + 1, sizeof(char))) == NULL)
	#else
    if((TargetBuf = (char *) calloc((LZARI_BUFFER_SIZE + LZARI_EXTRA_SIZE) + 1, sizeof(char))) == NULL)
	#endif
	{
	    CloseHandle(hfSource);
	    CloseHandle(hfTarget);
		#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(SourceBuf);
		#else
		free(SourceBuf);
		#endif
		return LZARI_ERROR_OUT_OF_MEMORY;
	}

    do{
        ReadFile(hfSource, SourceBuf, LZARI_BUFFER_SIZE, &BytesRead, NULL);

        DeflSize = (LZARI_BUFFER_SIZE + LZARI_EXTRA_SIZE);

        MyLZARI.Deflate(SourceBuf, TargetBuf, BytesRead, &DeflSize);
        //printf("%d Bytes to %d Bytes\n", BytesRead, DeflSize);

        WriteFile(hfTarget, &BytesRead, sizeof(DWORD), &BytesWritted, NULL);
        WriteFile(hfTarget, &DeflSize, sizeof(DWORD), &BytesWritted, NULL);
        WriteFile(hfTarget, TargetBuf, DeflSize, &BytesWritted, NULL);
    } while(BytesRead == LZARI_BUFFER_SIZE);

	#ifdef _USE_GLOBAL_MEMPOOL
    pMem->Free(SourceBuf);
    pMem->Free(TargetBuf);
	#else
    free(SourceBuf);
    free(TargetBuf);
	#endif

    CloseHandle(hfSource);
    CloseHandle(hfTarget);

    return LZARI_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LZARIDecompressFile(const char *sSource, const char *sTarget)
{
    HANDLE hfSource = NULL;
    HANDLE hfTarget = NULL;

    hfSource = CreateFile(
        sSource,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if(hfSource == INVALID_HANDLE_VALUE)
    {
        return LZARI_ERROR_FILE_READ;
    }

    hfTarget = CreateFile(
        sTarget,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if(hfSource == INVALID_HANDLE_VALUE)
    {
        CloseHandle(hfSource);
        return LZARI_ERROR_FILE_WRITE;
    }

    LZARI MyLZARI;

    MyLZARI.Initialize();

    char *SourceBuf = NULL;
    char *TargetBuf = NULL;

	#ifdef _USE_GLOBAL_MEMPOOL
    if((SourceBuf = (char *) pMem->Allocate(LZARI_BUFFER_SIZE + 1, sizeof(char))) == NULL)
	#else
    if((SourceBuf = (char *) calloc(LZARI_BUFFER_SIZE + 1, sizeof(char))) == NULL)
	#endif
	{
	    CloseHandle(hfSource);
	    CloseHandle(hfTarget);
		return LZARI_ERROR_OUT_OF_MEMORY;
	}

	#ifdef _USE_GLOBAL_MEMPOOL
	if((TargetBuf = (char *) pMem->Allocate((LZARI_BUFFER_SIZE + LZARI_EXTRA_SIZE) + 1, sizeof(char))) == NULL)
	#else
	if((TargetBuf = (char *) calloc((LZARI_BUFFER_SIZE + LZARI_EXTRA_SIZE) + 1, sizeof(char))) == NULL)
	#endif
	{
	    CloseHandle(hfSource);
	    CloseHandle(hfTarget);
		#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(SourceBuf);
		#else
		free(SourceBuf);
		#endif
		return LZARI_ERROR_OUT_OF_MEMORY;
	}

    DWORD OrigSize = 0;
    DWORD DeflSize = 0;
    DWORD InflSize = 0;
    DWORD BytesRead = 0;
    DWORD BytesWritted = 0;

    do{
        ReadFile(hfSource, &OrigSize, sizeof(DWORD), &BytesRead, NULL);
        ReadFile(hfSource, &DeflSize, sizeof(DWORD), &BytesRead, NULL);
        ReadFile(hfSource, SourceBuf, DeflSize, &BytesRead, NULL);

        InflSize = (LZARI_BUFFER_SIZE + LZARI_EXTRA_SIZE);

        MyLZARI.Inflate(SourceBuf, TargetBuf, OrigSize, &InflSize);
        //printf("%d Bytes to %d Bytes\n", DeflSize, InflSize);

        WriteFile(hfTarget, TargetBuf, InflSize, &BytesWritted, NULL);
    } while(OrigSize == LZARI_BUFFER_SIZE);

	#ifdef _USE_GLOBAL_MEMPOOL
    pMem->Free(SourceBuf);
    pMem->Free(TargetBuf);
	#else
    free(SourceBuf);
    free(TargetBuf);
	#endif

    CloseHandle(hfSource);
    CloseHandle(hfTarget);

    return LZARI_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
