#include <stdio.h>
#include <math.h>
#include <windows.h>

#include "LZSSCompress.h"

///////////////////////////////////////////////////////////////////////////////
// SetupWindowSize()
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Compress::SetupCompressionLevel(UINT nCompressionLevel)
{

	switch (nCompressionLevel)
	{
		case 0:
			m_nHashChainLimit	= 1;
			break;
		case 1:
			m_nHashChainLimit	= 4;
			break;
		case 2:
			m_nHashChainLimit	= 16;
			break;
		case 3:
			m_nHashChainLimit	= 64;
			break;
		case 4:
			m_nHashChainLimit	= 256;
			break;
		default:
			m_nHashChainLimit	= 1;	// Cannot be 0.  1 = 2 hash entries
			break;
	}

} // SetupCompressionLevel()


///////////////////////////////////////////////////////////////////////////////
// Compress()
///////////////////////////////////////////////////////////////////////////////

int HS_LZSS_Compress::Compress(UCHAR *bData, UCHAR *bCompressedData, ULONG nDataSize, ULONG *nCompressedSize)
{
	int	nRes;

	// Set up initial values
	m_nDataStreamPos		= 0;				// We are at the start of the input data
	m_nCompressedStreamPos	= 0;				// We are at the start of the compressed data
	m_nCompressedLong		= 0;				// Compressed stream temporary 32bit value
	m_nCompressedBitsUsed	= 0;				// Number of bits used in temporary value
	m_bCompressedData		= bCompressedData;	// Pointer to our output buffer
	m_bData					= bData;			// Pointer to our input buffer
	m_nDataSize				= nDataSize;		// Store the size of our input data
	m_nCompressedSize		= 0;				// No compressed data yet


	// If the input file is too small then there is a chance of
	// buffer overrun, so just abort
	if (m_nDataSize < HS_LZSS_MINDATASIZE)
		return HS_LZSS_E_BADCOMPRESS;

	// Uncompressed size (4 bytes)
	CompressedStreamWriteBits( (m_nDataSize >> 16) & 0x0000ffff, 16);
	CompressedStreamWriteBits( m_nDataSize & 0x0000ffff, 16);

	// Initialize our hash table
	HashTableInit();

	//
	// Jump to our main compression function
	//
	nRes = CompressLoop();
	if (nRes != HS_LZSS_E_OK)
    {
    	// Free up our hash table
    	HashTableFree();
		return nRes;							// Return error code
    }


	// Write out 2 bytes (16 bits) of nothing to help avoid overruns when decoding
	// Lazy....I know.
	CompressedStreamWriteBits(0, 16);

	// Flush the compressed bitstream (pad with zeros if required)
	CompressedStreamWriteBitsFlush();

	// Free up our hash table
	HashTableFree();

	// Return the compressed data size
	*nCompressedSize = m_nCompressedSize;

	return HS_LZSS_E_OK;						// Return with success message

} // Compress()


///////////////////////////////////////////////////////////////////////////////
// CompressLoop()
//
///////////////////////////////////////////////////////////////////////////////

int HS_LZSS_Compress::CompressLoop(void)
{
	UINT	nOffset1, nLen1;					// n Offset values
	ULONG	nMaxData;							// The largest size we want for compressed data
	UINT	nHash;
	UINT	nWPos;
	ULONG	nIncrement;


	nMaxData = m_nDataSize - 16;				// Abort if compressed data is this big

	// Loop around until there is no more data
	while (m_nDataStreamPos < m_nDataSize)
	{
		// If at any point, we reach within 16 bytes of the end of our output buffer, abort!
		// See the Compress() function for details of what is output after we have finished
		if ( m_nCompressedSize >= nMaxData )
			return HS_LZSS_E_BADCOMPRESS;


		// Check for a match at the current position
		FindMatches(&nOffset1, &nLen1);			// Search for matches for current position
	
		// Did we get a match?
		if (nLen1 > 0)
		{
			CompressedStreamWriteBits(HS_LZSS_MATCH, 1);
			CompressedStreamWriteBits(nOffset1 - HS_LZSS_MINMATCHLEN, HS_LZSS_WINBITS);
			CompressedStreamWriteMatchLen(nLen1);	// Range is auto adjusted
			nIncrement = nLen1;					// Move forwards matched len
		}
		else
		{
			// No matches, just store the literal byte
			CompressedStreamWriteBits(HS_LZSS_LITERAL, 1);
			CompressedStreamWriteBits(m_bData[m_nDataStreamPos], 8);
			nIncrement = 1;						// Move forward 1 literal
		}

		// We have skipped forwards either 1 byte or xxx bytes (if matched) we must now
		// add entries in the hash table for all the entries we've skipped
		while (nIncrement > 0)
		{
			// Get the start of the window offset (to help remove old hash entries)
			if (m_nDataStreamPos < HS_LZSS_WINLEN)
				nWPos = 0;
			else
				nWPos = m_nDataStreamPos - HS_LZSS_WINLEN;

			nHash =((40543*((((m_bData[m_nDataStreamPos]<<4)^m_bData[m_nDataStreamPos+1])<<4)^m_bData[m_nDataStreamPos+2]))>>4) & 0xFFF;
			HashTableAdd(nHash, m_nDataStreamPos, nWPos);	

			m_nDataStreamPos++;
			nIncrement--;
		}  // End while

	} // End while

	return HS_LZSS_E_OK;						// Return with success message

} // CompressLoop()


///////////////////////////////////////////////////////////////////////////////
// CompressedStreamWriteBits()
//
// Will write a number of bits (variable) into the compressed data stream
//
// When there are no more bits to send, you should call the function with the
// parameters 0, 0 to make sure that any left over bits are flushed into the
// compressed stream
//
// Note 16 bits is the maximum allowed value for this function!!!!!
// ================================================================
// This equates to a maximum window size of 65535
//
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Compress::CompressedStreamWriteBits(UINT nValue, UINT nNumBits)
{
	while (nNumBits > 0)
	{
		nNumBits--;

		// Make room for another bit (shift left once)
		m_nCompressedLong = m_nCompressedLong << 1;

		// Merge (OR) our value into the temporary long
		m_nCompressedLong = m_nCompressedLong | ((nValue >> nNumBits) & 0x00000001);

		// Update how many bits we are using (add 1)
		m_nCompressedBitsUsed++;

		// Now check if we have filled our temporary long with bits (32bits)
		if (m_nCompressedBitsUsed == 32)
		{
			// We now need to dump the highest 16 bits into our compressed
			// stream.  Highest order 8 bits first
			m_bCompressedData[m_nCompressedStreamPos++] = (UCHAR)(m_nCompressedLong >> 24);
			m_bCompressedData[m_nCompressedStreamPos++] = (UCHAR)(m_nCompressedLong >> 16);

			// We have just stored 2 bytes of data, increase our size count
			m_nCompressedSize = m_nCompressedSize + 2;

			// We've now written out 16 bits so make more room (16 bits more room :) )
			m_nCompressedBitsUsed = m_nCompressedBitsUsed - 16;
		}

	} // End while


} // CompressedStreamWriteBits()


///////////////////////////////////////////////////////////////////////////////
// CompressedStreamWriteBitsFlush()
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Compress::CompressedStreamWriteBitsFlush(void)
{
	// We have been asked to finish, flush remaining by using up remaining
	// unused bits with zeros

	// Bits remaining = 32 minus bits used
	m_nCompressedLong = m_nCompressedLong << (32 - m_nCompressedBitsUsed);

	// Now write out all 32 bits
	m_bCompressedData[m_nCompressedStreamPos++] = (UCHAR)(m_nCompressedLong >> 24);
	m_bCompressedData[m_nCompressedStreamPos++] = (UCHAR)(m_nCompressedLong >> 16);
	m_bCompressedData[m_nCompressedStreamPos++] = (UCHAR)(m_nCompressedLong >> 8);
	m_bCompressedData[m_nCompressedStreamPos++] = (UCHAR)(m_nCompressedLong);

	// We have just stored 4 bytes of data, increase our size count
	m_nCompressedSize = m_nCompressedSize + 4;

	// All done
	m_nCompressedBitsUsed = 0;

} // CompressedStreamWriteBitsFlush()


///////////////////////////////////////////////////////////////////////////////
// CompressedStreamWriteMatchLen()
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Compress::CompressedStreamWriteMatchLen(UINT nLen)
{
	// Write out the match length using the convention shown in the LZP
	// article by Charles Bloom

	// First adjust the range from 0-295, to 1-296, we will never
	// have a match of 0 so it would be a waste
	nLen = nLen - HS_LZSS_MINMATCHLEN;

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

	if (nLen >= 3)
	{
		CompressedStreamWriteBits(0xffff, 2);
	
		if (nLen >= 10)
		{
			CompressedStreamWriteBits(0xffff, 3);

			if (nLen >= 41)
			{
				CompressedStreamWriteBits(0xffff, 5);

				if (nLen >= 296)
				{
					nLen = nLen - 296;
					CompressedStreamWriteBits(0xffff, 8);

					while (nLen >= 255)
					{
						nLen = nLen - 255;
						CompressedStreamWriteBits(0xffff, 8);
					}

					CompressedStreamWriteBits(nLen, 8);	
				}
				else
				{
					nLen = nLen - 41;
					CompressedStreamWriteBits(nLen, 8);	// 41-295
				}
			}
			else
			{
				nLen = nLen - 10;
				CompressedStreamWriteBits(nLen, 5);
			}
		}
		else
		{
			nLen = nLen - 3;
			CompressedStreamWriteBits(nLen, 3);
		}
	}
	else
		CompressedStreamWriteBits(nLen, 2);


} // CompressedStreamWriteMatchLen()


///////////////////////////////////////////////////////////////////////////////
// FindMatches()
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Compress::FindMatches(/*ULONG nInitialDataPos,*/ UINT *nOffset, UINT *nLen)
{
	ULONG	nTempWPos, nWPos, nDPos;	// Temp Window and Data position markers
	ULONG	nTempLen;					// Temp vars 
	ULONG	nBestOffset, nBestLen;		// Stores the best match so far
	struct LZSS_Hash *lpTempHash, *lpTempPrev, *lpTempNext;
	UINT	nHash;

	// Reset all variables
	nBestOffset = 0;
	nBestLen	= 0;

	// Get our window start position, if the window would take us beyond
	// the start of the file, just use 0
	if (m_nDataStreamPos < HS_LZSS_WINLEN)
		nWPos = 0;
	else
		nWPos = m_nDataStreamPos - HS_LZSS_WINLEN;

	// Generate a hash of the next three chars
	nHash =((40543*((((m_bData[m_nDataStreamPos]<<4)^m_bData[m_nDataStreamPos+1])<<4)^m_bData[m_nDataStreamPos+2]))>>4) & 0xFFF;

	// Main loop
	lpTempPrev = NULL;
	lpTempHash = m_HashTable[nHash];			// Get our match from the hash table

	while (lpTempHash != NULL)
	{
		nTempWPos	= lpTempHash->nPos;			// The position of the supposed match
		lpTempNext	= lpTempHash->lpNext;		// Next hash

		// Is this entry too old to be used for a match?  It will get automatically erased
		// When the compressloop adds this hash to the table, so we won't bother deleting it
		// now.
		if (nTempWPos >= nWPos)
		{
			nDPos		= m_nDataStreamPos;
			nTempLen	= 0;
		
			while ( (m_bData[nTempWPos] == m_bData[nDPos]) && (nTempWPos < m_nDataStreamPos) &&
					nDPos < m_nDataSize ) 
			{
				nTempLen++;	
				nTempWPos++; 
				nDPos++;
			} 

			// See if this match was better than previous match
			if (nTempLen > nBestLen)
			{
				nBestLen	= nTempLen;
				nBestOffset = m_nDataStreamPos - lpTempHash->nPos;
			}

		}// End if

		lpTempPrev = lpTempHash;
		lpTempHash = lpTempNext;

	} // End while


	// Setup our return values of bestoffset and bestlen
	if (nBestLen < HS_LZSS_MINMATCHLEN)
		nBestLen = 0;
	*nOffset	= nBestOffset;					// Return value
	*nLen		= nBestLen;						// Return value

} // FindMatches()


///////////////////////////////////////////////////////////////////////////////
// HashTableInit()
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Compress::HashTableInit(void)
{
	int i;
	
	// Clear the hash table
	for (i=0; i<HS_LZSS_HASHTABLE_SIZE; i++)
		m_HashTable[i] = NULL;

	// Clear the hash malloc cache 
	for (i=0; i<HS_LZSS_MALLOC_CACHE; i++)
		m_HashAlloc[i] = NULL;

	m_nHashAllocNumEntries = 0;					// Alloc buffer is not full
}


///////////////////////////////////////////////////////////////////////////////
// HashTableFree()
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Compress::HashTableFree(void)
{
	int i;
	struct LZSS_Hash *lpTempNext, *lpTempHash;

	// Free the hash table
	for (i=0; i<HS_LZSS_HASHTABLE_SIZE; i++)
	{
		lpTempHash = m_HashTable[i];
		while (lpTempHash != NULL)
		{
			// Go through the chain freeing each entry
			lpTempNext = lpTempHash->lpNext;
			HashEntryFree(lpTempHash);
			lpTempHash = lpTempNext;
		}
		free(lpTempHash);
	}

	// Free any alloc buffer entries not taken care of above
	for (i=0; i<HS_LZSS_MALLOC_CACHE; i++)
	{
		if (m_HashAlloc[i] != NULL)
			free(m_HashAlloc[i]);
	}

} // HashTableFree()


///////////////////////////////////////////////////////////////////////////////
// HashTableAdd()
// Adds a hash entry of "nPos" at index "nHash"
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Compress::HashTableAdd(UINT nHash, ULONG nPos, ULONG nTooOldPos)
{
	UINT				nTempUINT;
	struct LZSS_Hash	*lpTempNext, *lpTempHash, *lpOldestHash, *lpTempPrev;
	ULONG				nOldestPos;
	ULONG				nTempPos;

	lpTempHash = m_HashTable[nHash];			// Get our first entry at this hash index

	// First "prune" any entries that are too old
	lpTempPrev = NULL;
	while (lpTempHash != NULL)
	{
		lpTempNext	= lpTempHash->lpNext;
		nTempPos	= lpTempHash->nPos;

		// If this entry is too old, remove it from the list
		if (nTempPos < nTooOldPos)
		{
			// Remove it!
			if (lpTempPrev != NULL)
				lpTempPrev->lpNext = lpTempNext;
			else
				m_HashTable[nHash] = lpTempNext;

			HashEntryFree(lpTempHash);
		}
		else
			lpTempPrev = lpTempHash;

		lpTempHash = lpTempNext;

	} // End while


	lpTempHash = m_HashTable[nHash];			// Get our first entry at this hash index

	// Check if there are any entries for this hash (we may have just deleted the base
	// entry in the code above!
	if (lpTempHash == NULL)
	{
		// Initial entry
		lpTempHash			= HashEntryAlloc();
		lpTempHash->lpNext	= NULL;
		lpTempHash->nPos	= nPos;
		m_HashTable[nHash]	= lpTempHash;
		return;
	}


	// Now traverse the chain until we find an empty entry or 
	// exceed the size limit
	lpTempHash = m_HashTable[nHash];			// Get our first entry at this hash index

	nOldestPos = nPos;							// At first, oldest is current!
	nTempUINT = m_nHashChainLimit;
	while (nTempUINT > 0)
	{
		nTempUINT--;

		nTempPos = lpTempHash->nPos;

		// Update our oldest hash in case we need to replace
		if (nTempPos < nOldestPos)
		{	
			nOldestPos = nTempPos;
			lpOldestHash = lpTempHash;
		}

		lpTempNext = lpTempHash->lpNext;
		if (lpTempNext == NULL)
		{
			// Found an empty entry
			lpTempNext			= HashEntryAlloc();
			lpTempHash->lpNext	= lpTempNext;
			lpTempNext->lpNext	= NULL;
			lpTempNext->nPos	= nPos;
			return;
		}

		lpTempHash = lpTempNext;

	} // End while

	//If we are here, then we ran out of room in the chain, if so, replace the oldest entry
	lpOldestHash->nPos = nPos;

} // HashTableAdd()


///////////////////////////////////////////////////////////////////////////////
// HashEntryAlloc()
// malloc() is really really slow when allocating and freeing lots of tiny
// 4bytes chunks of memory.  This function acts as a buffer to speed things along
///////////////////////////////////////////////////////////////////////////////

struct LZSS_Hash* HS_LZSS_Compress::HashEntryAlloc(void)
{
	struct LZSS_Hash *lpHash;

	if (m_nHashAllocNumEntries == 0)
	{
		// No unused entries, do it the slow way
		return (struct LZSS_Hash*)malloc(sizeof(LZSS_Hash));
	}

	// Use the last entry in the list
	m_nHashAllocNumEntries--;
	lpHash = m_HashAlloc[m_nHashAllocNumEntries];
	m_HashAlloc[m_nHashAllocNumEntries] = NULL;

	return lpHash;

} // HashMalloc


///////////////////////////////////////////////////////////////////////////////
// HashEntryFree()
///////////////////////////////////////////////////////////////////////////////

void HS_LZSS_Compress::HashEntryFree(struct LZSS_Hash *lpHash)
{

	// If there is no room to buffer this memory release just free it :(
	if (m_nHashAllocNumEntries == HS_LZSS_MALLOC_CACHE)
	{
		free(lpHash);
		return;
	}

	// Enter this entry into our buffer
	m_HashAlloc[m_nHashAllocNumEntries] = lpHash;
	m_nHashAllocNumEntries++;


} // HashEntryFree()
