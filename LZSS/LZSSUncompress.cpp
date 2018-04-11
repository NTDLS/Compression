#include <stdio.h>
#include <math.h>
#include <windows.h>

#include "LZSSUncompress.h"

///////////////////////////////////////////////////////////////////////////////
// GetUncompressedSize()
///////////////////////////////////////////////////////////////////////////////

int HS_LZSS_Uncompress::GetUncompressedSize(UCHAR *bCompressedData, ULONG *nUncompressedSize)
{
	UINT nTemp;

	// Set up initial values
	m_nCompressedStreamPos = 0; // We are at the start of the compressed data
	m_nCompressedLong      = 0; // Compressed stream temporary 32bit value
	m_nCompressedBitsUsed  = 0; // Number of bits used in temporary value
	m_bCompressedData      = bCompressedData; // Pointer to our input buffer

	// Uncompressed size (4 bytes)
	CompressedStreamReadBits( &nTemp, 16); 
	*nUncompressedSize = ((ULONG)nTemp) << 16;
	CompressedStreamReadBits( &nTemp, 16);
	*nUncompressedSize = *nUncompressedSize | (ULONG)nTemp;

	return HS_LZSS_E_OK;

} // GetUncompressedSize()


///////////////////////////////////////////////////////////////////////////////
// Uncompress()
//
// This function ASSUMES that our decompression buffer is big enough, i.e.
// The caller should have done GetUncompressedSize() first and allocated 
// a buffer big enough!
//
// Also, the GetUncompressedSize checks that the LZSS alg ID was found
// so we won't bother this time
//
// In other words, don't run this function on random areas of memory, it will
// crash.
//
///////////////////////////////////////////////////////////////////////////////

int HS_LZSS_Uncompress::Uncompress(UCHAR *bCompressedData, UCHAR *bData)
{
	UINT	nTemp;
	UINT	nOffset, nLen;
	ULONG	nTempPos;

	// Set up initial values
	m_nDataStreamPos		= 0;				// We are at the start of the input data
	m_nCompressedStreamPos	= 0;				// We are at the start of the compressed data
	m_nCompressedLong		= 0;				// Compressed stream temporary 32bit value
	m_nCompressedBitsUsed	= 0;				// Number of bits used in temporary value
	m_bCompressedData		= bCompressedData;	// Pointer to our input buffer
	m_bData					= bData;			// Pointer to our output buffer

	// Get the uncompressed size (4 bytes)
	CompressedStreamReadBits( &nTemp, 16); 	m_nDataSize = ((ULONG)nTemp) << 16;
	CompressedStreamReadBits( &nTemp, 16);	m_nDataSize = m_nDataSize | (ULONG)nTemp;

	// Perform decompression until we fill our predicted buffer
	while(m_nDataStreamPos < m_nDataSize)
	{
		// Read in the 1 bit flag
		CompressedStreamReadBits(&nTemp, 1);	

		// Was it a literal byte, or a (offset,len) match pair?
		if (nTemp == HS_LZSS_LITERAL)			// use nTemp == 0
		{
			// Output a literal byte
			CompressedStreamReadBits(&nTemp, 8);
			m_bData[m_nDataStreamPos++] = (UCHAR)nTemp;
		}
		else
		{
			// Read the offset and length
			CompressedStreamReadBits(&nOffset, HS_LZSS_WINBITS);
			nOffset += HS_LZSS_MINMATCHLEN;		// Adjust to our range
			//CompressedStreamReadBits(&nLen, HS_LZSS_MATCHBITS);
			CompressedStreamReadMatchLen(&nLen);
			
			// Adjust the values
			//nLen	+= HS_LZSS_MINMATCHLEN;

			// Write out our match
			nTempPos = m_nDataStreamPos - nOffset;
			while (nLen > 0)
			{
				nLen--;
				m_bData[m_nDataStreamPos++] = m_bData[nTempPos++];	
			}
		}
	}
	
	return HS_LZSS_E_OK;

} // Uncompress()


///////////////////////////////////////////////////////////////////////////////
// CompressedStreamReadBits()
//
// Will read up to 16 bits from the compressed data stream
//
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Uncompress::CompressedStreamReadBits(UINT *nValue, UINT nNumBits)
{

	ULONG	nTemp;

	// Ensure that the high order word of our bit buffer is blank 
	m_nCompressedLong = m_nCompressedLong & 0x000ffff;

	while (nNumBits > 0)
	{
		nNumBits--;

		// Check if we need to refill our decoding bit buffer
		if (m_nCompressedBitsUsed == 0)
		{
			// Fill the low order 16 bits of our long buffer
			nTemp = (ULONG)m_bCompressedData[m_nCompressedStreamPos++];
			m_nCompressedLong = m_nCompressedLong | (nTemp << 8);
			nTemp = (ULONG)m_bCompressedData[m_nCompressedStreamPos++];
			m_nCompressedLong = m_nCompressedLong | nTemp;
			m_nCompressedBitsUsed = 16;
		}

		// Shift the data into the high part of the long
		m_nCompressedLong = m_nCompressedLong << 1;
		m_nCompressedBitsUsed--;
	}

	*nValue = (UINT)(m_nCompressedLong >> 16);

} // CompressedStreamReadBits()


///////////////////////////////////////////////////////////////////////////////
// CompressedStreamReadMatchLen()
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Uncompress::CompressedStreamReadMatchLen(UINT *nLen)
{
	UINT	nTemp;

	// Read in the match length using the convention shown in the LZP
	// article by Charles Bloom

	// Value	Bitstream
	// 0		00 (bin 0)
	// 1		01 (bin 1)
	// 2		10 (bin 2)
	// 3		11 000 (bin 0)					SUBTRACT 3
	// ...
	// 9		11 110 (bin 6)	
	// 10		11 111 00000 (bin 0)			SUBTRACT 10
	// ...
	// 40       11 111 11110
	// 41       11 111 11111 00000000 (bin 0)	SUBTRACT 41
	// 296		11 111 11111 11111111 00000000 (bin 0)	SUBTRACT 296
	// 551		11 111 11111 11111111 00000000 (bin 0)   SUBTRACT 551

	*nLen = 0;									// Starting value

	// Read in first two bits
	CompressedStreamReadBits(&nTemp, 2);	

	if (nTemp == 3)	// Bin 11 = Dec 3
	{
		*nLen = 3;
		
		CompressedStreamReadBits(&nTemp, 3);	// Read next three bits
	
		if (nTemp == 7) // Bin 111 = Dec 7
		{
			*nLen = 10;
			
			CompressedStreamReadBits(&nTemp, 5);	// Read next five bits

			if (nTemp == 31) // Bin 11111 = Dec 31
			{
				*nLen = 41;
				
				CompressedStreamReadBits(&nTemp, 8);	// Read next eight bits

				if (nTemp == 255) // Bin 11111111 = Dec 255
				{
					*nLen = 296;
					CompressedStreamReadBits(&nTemp, 8);	

					while (nTemp == 255)
					{
						*nLen = *nLen + 255;
						CompressedStreamReadBits(&nTemp, 8);	
					}
				}
			}
		}
	}

	*nLen = *nLen + nTemp;	// Final calculation

	// Finally adjust the range from 0-295, to 1-296, we will never
	// have a match of 0 so it would be a waste
	*nLen = *nLen + HS_LZSS_MINMATCHLEN;

} // CompressedStreamReadMatchLen()
