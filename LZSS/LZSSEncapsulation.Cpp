///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include <windows.h>

#include "LZSSCompress.H"
#include "LZSSUncompress.H"
#include "LZSSEncapsulation.H"

#ifdef _USE_GLOBAL_MEMPOOL
#include "../../../@Libraries/CMemPool/CMemPool.H"
extern CMemPool *pMem; //gMem must be defined and initalized elsewhere.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LZSSCompressFile(const char *sSource, const char *sTarget, const int iLevel)
{
	HS_LZSS_Compress MyCompress;

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
        return LZSS_ERROR_FILE_READ;
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

    if(hfTarget == INVALID_HANDLE_VALUE)
    {
		CloseHandle(hfSource);
        return LZSS_ERROR_FILE_WRITE;
    }

	memset(&MyCompress, 0, sizeof(MyCompress));

	MyCompress.SetupCompressionLevel(iLevel);

    DWORD dwBytesRead = 0;
    DWORD dwBytesWritten = 0;
    DWORD dwDeflSize = 0;

	int iLZSSRes = 0;

    unsigned char *sSourceBuf = NULL;
    unsigned char *sTargetBuf = NULL;

	#ifdef _USE_GLOBAL_MEMPOOL
    if((sSourceBuf = (unsigned char *) pMem->Allocate(HS_LZSS_BUF_SZ + 1, sizeof(char))) == NULL)
	#else
    if((sSourceBuf = (unsigned char *) calloc(HS_LZSS_BUF_SZ + 1, sizeof(char))) == NULL)
	#endif
	{
		CloseHandle(hfSource);
		CloseHandle(hfTarget);
		return LZSS_ERROR_OUT_OF_MEMORY;
	}

	#ifdef _USE_GLOBAL_MEMPOOL
    if((sTargetBuf = (unsigned char *) pMem->Allocate((HS_LZSS_BUF_SZ + HS_LZSS_EXTRA_SZ) + 1, sizeof(char))) == NULL)
	#else
    if((sTargetBuf = (unsigned char *) calloc((HS_LZSS_BUF_SZ + HS_LZSS_EXTRA_SZ) + 1, sizeof(char))) == NULL)
	#endif
	{
		CloseHandle(hfSource);
	    CloseHandle(hfTarget);
		#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(sSourceBuf);
		#else
		free(sSourceBuf);
		#endif
		return LZSS_ERROR_OUT_OF_MEMORY;
	}

	bool bCompression = false;

    do{
        ReadFile(hfSource, sSourceBuf, sizeof(char) * HS_LZSS_BUF_SZ, &dwBytesRead, NULL);
			
		iLZSSRes = MyCompress.Compress(sSourceBuf, sTargetBuf, dwBytesRead, &dwDeflSize);

		if(iLZSSRes != HS_LZSS_E_OK)
        {
            bCompression = false;
            WriteFile(hfTarget, &bCompression, sizeof(bool), &dwBytesWritten, NULL);
			WriteFile(hfTarget, &dwBytesRead, sizeof(DWORD), &dwBytesWritten, NULL);
			WriteFile(hfTarget, sSourceBuf, sizeof(char) * dwBytesRead, &dwBytesWritten, NULL);
		}
        else{
            bCompression = true;
			WriteFile(hfTarget, &bCompression, sizeof(bool), &dwBytesWritten, NULL);
			WriteFile(hfTarget, &dwDeflSize, sizeof(DWORD), &dwBytesWritten, NULL);
			WriteFile(hfTarget, sTargetBuf, sizeof(char) * dwDeflSize, &dwBytesWritten, NULL);
		}

    } while(dwBytesRead == HS_LZSS_BUF_SZ);

	#ifdef _USE_GLOBAL_MEMPOOL
    pMem->Free(sSourceBuf);
    pMem->Free(sTargetBuf);
	#else
    free(sSourceBuf);
    free(sTargetBuf);
	#endif

    CloseHandle(hfSource);
    CloseHandle(hfTarget);

    return LZSS_ERROR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LZSSUncompressFile(char *sSource, char *sTarget)
{
	HS_LZSS_Uncompress MyUnCompress;

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
        return LZSS_ERROR_FILE_READ;
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

    if(hfTarget == INVALID_HANDLE_VALUE)
    {
		CloseHandle(hfSource);
        return LZSS_ERROR_FILE_WRITE;
    }

	memset(&MyUnCompress, 0, sizeof(MyUnCompress));

    DWORD dwBytesRead = 0;
    DWORD dwBytesWritten = 0;
    DWORD dwDeflSize = 0;
	DWORD dwInflSize = 0;

	int iLZSSRes = 0;

    unsigned char *sSourceBuf = NULL;
    unsigned char *sTargetBuf = NULL;

	#ifdef _USE_GLOBAL_MEMPOOL
    if((sSourceBuf = (unsigned char *) pMem->Allocate(HS_LZSS_BUF_SZ + 1, sizeof(char))) == NULL)
	#else
    if((sSourceBuf = (unsigned char *) calloc(HS_LZSS_BUF_SZ + 1, sizeof(char))) == NULL)
	#endif
	{
		CloseHandle(hfSource);
		CloseHandle(hfTarget);
		return LZSS_ERROR_OUT_OF_MEMORY;
	}

	#ifdef _USE_GLOBAL_MEMPOOL
    if((sTargetBuf = (unsigned char *) pMem->Allocate((HS_LZSS_BUF_SZ + HS_LZSS_EXTRA_SZ) + 1, sizeof(char))) == NULL)
	#else
    if((sTargetBuf = (unsigned char *) calloc((HS_LZSS_BUF_SZ + HS_LZSS_EXTRA_SZ) + 1, sizeof(char))) == NULL)
	#endif
	{
		CloseHandle(hfSource);
	    CloseHandle(hfTarget);
		#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(sSourceBuf);
		#else
		free(sSourceBuf);
		#endif
		return LZSS_ERROR_OUT_OF_MEMORY;
	}
	bool bCompression = false;

    do{
		ReadFile(hfSource, &bCompression, sizeof(bool), &dwBytesRead, NULL);
        ReadFile(hfSource, &dwDeflSize, sizeof(DWORD), &dwBytesRead, NULL);

		ReadFile(hfSource, sSourceBuf, sizeof(char) * dwDeflSize, &dwBytesRead, NULL);
			
        if(bCompression)
		{
			iLZSSRes = MyUnCompress.GetUncompressedSize(sSourceBuf, &dwInflSize);
			MyUnCompress.Uncompress(sSourceBuf, sTargetBuf);

			WriteFile(hfTarget, sTargetBuf, sizeof(char) * dwInflSize, &dwBytesWritten, NULL);
		}
		else WriteFile(hfTarget, sSourceBuf, sizeof(char) * dwBytesRead, &dwBytesWritten, NULL);

    } while(dwInflSize == HS_LZSS_BUF_SZ);

	#ifdef _USE_GLOBAL_MEMPOOL
    pMem->Free(sSourceBuf);
    pMem->Free(sTargetBuf);
	#else
    free(sSourceBuf);
    free(sTargetBuf);
	#endif

    CloseHandle(hfSource);
    CloseHandle(hfTarget);

    return LZSS_ERROR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
