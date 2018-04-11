#ifndef _LZARIDeflate_Cpp
#define _LZARIDeflate_Cpp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>     
#include <Stdio.H>         
#include <Stdlib.H>      
#include <String.H>      

#include "LZARIClass.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LZARI::Deflate(char *inSrcBuffer, char *outTarBuffer, unsigned long inSrcSize, unsigned long *outTarSize)
{
    int i    = 1;
    int c    = 0;
    int r    = 0;
    int s    = 0;
    int Last_MatchLength = 0;
    int len  = 0;

    InitSingle();

    CompressSource = inSrcBuffer;
    CompressTarget = outTarBuffer;
    SourceSize = inSrcSize;

    InitTree();

    r = SizeOfRingBuffer - MatchLengthLimit;

    while( (len < MatchLengthLimit) && (SourcePos < SourceSize) )
    {
        c = CompressSource[SourcePos];
        SourcePos++;
        TextBuffer[r + len] = c;
        len++;
    }

    while ( i <= MatchLengthLimit)
    {
        InsertNode(r - i);
        i++;
    }

    InsertNode(r);

    do
    {
        if (MatchLength > len)
            MatchLength = len;

        if (MatchLength <= THRESHOLD)
        {
            MatchLength = 1;
            EncodeChar(TextBuffer[r]);
        }
        else{
            EncodeChar(255 - THRESHOLD + MatchLength);
            EncodePosition(MatchPosition - 1);
        }

        Last_MatchLength = MatchLength;

        i = 0;

        while( (i < Last_MatchLength) && (SourcePos < SourceSize))
        {
            c = CompressSource[SourcePos];
            SourcePos++;
            DeleteNode(s);
            TextBuffer[s] = c;
            if (s < MatchLengthLimit - 1) TextBuffer[s + SizeOfRingBuffer] = c;
            s = (s + 1) & (SizeOfRingBuffer - 1);
            r = (r + 1) & (SizeOfRingBuffer - 1);
            InsertNode(r);
            i++;
        }

        while (i++ < Last_MatchLength)
        {
            DeleteNode(s);
            s = (s + 1) & (SizeOfRingBuffer - 1);
            r = (r + 1) & (SizeOfRingBuffer - 1);
            if (--len) InsertNode(r);
        }

    }
    while(len > 0); // End Of Do Loop

    EncodeEnd();

    *outTarSize = TargetPos;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

