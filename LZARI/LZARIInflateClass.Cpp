#ifndef _LZARIInflate_Cpp
#define _LZARIInflate_Cpp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>     
#include <Stdio.H>         
#include <Stdlib.H>      
#include <String.H>      

#include "LZARIClass.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LZARI::Inflate(char *inSrcBuffer, char *outTarBuffer, unsigned long inSrcSize,  unsigned long *outTarSize)
{
    int i = 0;
    int j = 0;
    int r = 0;
    int c = 0;

    DWORD Current_Size = 0;

    InitSingle();

    CompressSource = inSrcBuffer;
    CompressTarget = outTarBuffer;
    SourceSize = inSrcSize;

    StartDecode();

    r = SizeOfRingBuffer - MatchLengthLimit;

    while( Current_Size < SourceSize )
    {
        c = DecodeChar();
        if (c < 256)
        {
            CompressTarget[TargetPos] = c;
            TargetPos++;

            TextBuffer[r++] = c;
            r &= (SizeOfRingBuffer - 1);
            Current_Size++;
        }
        else{
            i = (r - DecodePosition() - 1) & (SizeOfRingBuffer - 1);
            j = c - 255 + THRESHOLD;

            int k = 0;
            while(k < j)
            {
                c = TextBuffer[(i + k) & (SizeOfRingBuffer - 1)];

                CompressTarget[TargetPos] = c;
                TargetPos++;

                TextBuffer[r++] = c;
                r &= (SizeOfRingBuffer - 1);
                Current_Size++;
                k++;
            }
        }
    }

    *outTarSize = TargetPos;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

