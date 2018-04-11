#ifndef __HS_LZSS_COMPRESS_H
#define __HS_LZSS_COMPRESS_H

// STANDALONE CLASS
//
// This is a memory-to-memory implementation of the LZSS compression
// algorithm.  See the "lzss.txt" file for a hopefully useful explanation
// of how LZSS works.
//
// Note, this is a "as simple to follow as possible" piece of code, there are many
// more efficient and quicker (and cryptic) methods but this was done as a learning
// exercise.
//

// Usage
// =====
// Call "Compress()" with two UCHAR buffers, one for the data to compress and one for
// the compressed output, the size of the input biffer and the required compression level.  
// >> The output buffer should be of the same size as the input.<<
// If the compressed data would have overun the output buffer, or the data is too small
// to compress the function will abort and return the code HS_LZSS_E_BADCOMPRESS.  If the
// function is successful it will return HS_LZSS_E_OK and the size of the new compressed data
// in "nCompressedSize".
//
// The compression level parameter (0-4) gives the following results:
// Compression Level  Window Bits  Match Bits  Hash Limit
// -----------------  -----------  ----------  ----------
// Fast       (0)     13           4           1               
// Normal     (1)     13           4           4
// Slow       (2)     13           4           16
// Very slow  (3)     13           4           64
// Ultra slow (4)     13           4           256
//

//
// The start of the compression stream will contain "other" items apart from data
// LZSS					4 bytes
// Uncompressed Size	4 bytes (1 ULONG)
// ...
// Compressed data
// ...
//
#define HS_LZSS_BUF_SZ           524288  // 1/2 MB Buffer.
#define HS_LZSS_EXTRA_SZ         HS_LZSS_BUF_SZ

// Error codes
#define	HS_LZSS_E_OK			0				// OK
#define	HS_LZSS_E_BADCOMPRESS	1				// Compressed file would be bigger than source!
#define HS_LZSS_E_NOTLZSS		2				// Not a valid LZSS data stream

// Stream flags
#define HS_LZSS_LITERAL			0				// Just output the literal byte
#define HS_LZSS_MATCH			1				// Output a (offset, len) match pair

#define HS_LZSS_MINDATASIZE		32				// Arbitrary miniumum data size that we should attempt to compress

// Window sizing related stuff
#define HS_LZSS_MINMATCHLEN		3
#define HS_LZSS_WINBITS			14				// 0 - 16383
#define HS_LZSS_WINLEN			16383 + HS_LZSS_MINMATCHLEN

// Define our hash and hash table structure
#define HS_LZSS_HASHTABLE_SIZE	4096			// Our hash function gives values 0-4095
#define HS_LZSS_MALLOC_CACHE	4096			// How many malloc() calls we can buffer
												// Memory used is 4096 x 4 = 16KB
typedef struct LZSS_Hash
{
	ULONG	nPos;								// Position in data stream
	struct 	LZSS_Hash *lpNext;					// Next entry in linked list

} _LZSS_Hash;

class HS_LZSS_Compress
{
public:
	// Functions
	int	Compress(UCHAR *bData, UCHAR *bCompressedData, ULONG nDataSize, ULONG *nCompressedSize);
	void SetupCompressionLevel(UINT nCompressionLevel); // Init the window and len sizes

private:
	// Variables
	ULONG	m_nDataStreamPos;					// Current position in the data stream
	ULONG	m_nCompressedStreamPos;				// Curent position in the compressed stream
	UCHAR	*m_bData;
	UCHAR	*m_bCompressedData;

	ULONG	m_nDataSize;						// The size of our uncompressed data
	ULONG	m_nCompressedSize;					// The size of our compressed data

	// Temporary variables used for the bit operations
	ULONG	m_nCompressedLong;					// Compressed stream temporary 32bit value
	int		m_nCompressedBitsUsed;				// Number of bits used in temporary value

	// Hash table related variables
	UINT	m_nHashChainLimit;					// The max length of each hash chain
	struct	LZSS_Hash *m_HashTable[HS_LZSS_HASHTABLE_SIZE];	// Hash table
	struct	LZSS_Hash *m_HashAlloc[HS_LZSS_MALLOC_CACHE];	// malloc buffer table
	UINT	m_nHashAllocNumEntries;		

	// Functions
	int		CompressLoop(void);					// The main compression loop
	void	FindMatches(UINT *nOffset, UINT *nLen);		// Searches for pattern matches

	// Bit operation functions
	void	CompressedStreamWriteBits(UINT nValue, UINT nNumBits);
	void	CompressedStreamWriteBitsFlush(void);
	void	CompressedStreamWriteMatchLen(UINT nLen);

	// Hash table functions
	void	HashTableInit(void);				// Make hash table ready for first use
	void	HashTableFree(void);				// Free all hash entries
	void	HashTableAdd(UINT nHash, ULONG nPos, ULONG nTooOldPos);	// Add an entry to the table
	struct LZSS_Hash* HashEntryAlloc(void);		// malloc cache
	void	HashEntryFree(struct LZSS_Hash*);	// free cache
};
#endif
