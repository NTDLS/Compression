#ifndef _RLECOMPRESSION_H
#define _RLECOMPRESSION_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _MAX_LEN        (0x7f) // Maximum Length For Runs Or Sequences

#define _MAX_RUN_HEADER (0xff) // Bit-7 == 1 : Run Follows
#define _MAX_SEQ_HEADER (0x7f) // Bit-7 == 0 : Unencode Sequence Follows

#define _RUN            (0x80) // Bit-7 == 1 : Run Follows
#define _SEQ            (0x00) // Bit-7 == 0 : Unencoded Sequence Follows

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *RLEDecode(char *sSource, char *sTarget, int iSourceSz, int *iInOutTargetSz, int *iAlloc);
char *RLEEncode(char *sSource, char *sTarget, int iSourceSz, int *iInOutTargetSz, int *iAlloc);
char *RLEResizeBuffer(char *sBuf, int *iBufSz);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
