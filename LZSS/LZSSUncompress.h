#ifndef __HS_LZSS_UNCOMPRESS_H
#define __HS_LZSS_UNCOMPRESS_H

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
// First call the "GetUncompressedSize()" function on your compressed data in order
// to work out how big the uncompressed data is going to be.  If the function returns
// HS_LZSS_E_NOTLZSS then the input data is not valid LZSS compressed data.  Otherwise
// HS_LZSS_E_OK will indicate success.
//
// Call the "Uncompress()" function with two UCHAR buffers, one for the compressed data
// and one for the uncompressed data.  Make sure that the uncompressed buffer is large
// enough (GetUncompressedSize()) as NO overrun checking will be performed.
// If the function returns
// HS_LZSS_E_NOTLZSS then the input data is not valid LZSS compressed data.  Otherwise
// HS_LZSS_E_OK will indicate success.
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

// Window sizing related stuff
#define HS_LZSS_MINMATCHLEN		3
#define HS_LZSS_WINBITS			14				// 0 - 16383
#define HS_LZSS_WINLEN			16383 + HS_LZSS_MINMATCHLEN

class HS_LZSS_Uncompress
{
public:
	// Functions
	int		Uncompress(UCHAR *bCompressedData, UCHAR *bData);
	int		GetUncompressedSize(UCHAR *bCompressedData, ULONG *nUncompressedSize);

private:
	// Variables
	ULONG	m_nDataStreamPos;					// Current position in the data stream
	ULONG	m_nCompressedStreamPos;				// Curent position in the compressed stream
	UCHAR	*m_bData;							// The uncompressed data buffer
	UCHAR	*m_bCompressedData;					// The compressed data buffer

	ULONG	m_nDataSize;						// The size of our uncompressed data
	ULONG	m_nCompressedSize;					// The size of our compressed data

	// Temporary variables used for the bit operations
	ULONG	m_nCompressedLong;					// Compressed stream temporary 32bit value
	int		m_nCompressedBitsUsed;				// Number of bits used in temporary value

	// Functions
	void	SetupWindowSize(UINT nWindowBits, UINT nLenBits);

	// Bit operation functions
	void	CompressedStreamReadBits(UINT *nValue, UINT nNumBits);
	void	CompressedStreamReadMatchLen(UINT *nLen);
};
#endif
