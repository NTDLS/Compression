#ifndef _RLECOMPRESSION_CPP
#define _RLECOMPRESSION_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Run Length Encoding (RLE) Compression
// 8-Bit Header Version
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <String.H>

#include "../../../@Libraries/CMemPool/CMemPool.H"

#include "RLECompression.H"

#ifdef _USE_GLOBAL_MEMPOOL
#include "../../../@Libraries/CMemPool/CMemPool.H"
extern CMemPool *pMem; //gMem must be defined and initalized elsewhere.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *RLEResizeBuffer(char *sBuf, int *iBufSz)
{
	#ifdef _USE_GLOBAL_MEMPOOL
		if((sBuf = (char *) pMem->ReAllocate(sBuf, ((int)*iBufSz) + 1024, sizeof(char))) == NULL)
		{
			return NULL;
		}
	#else
		if((sBuf = (char *) realloc(sBuf, ((int)*iBufSz) + 1024)) == NULL)
		{
			return NULL;
		}
	#endif

	*iBufSz = (*iBufSz + 1024);

	return sBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *RLEEncode(char *sSource, char *sTarget, int iSourceSz, int *iInOutTargetSz, int *iAlloc)
{
	int iTargetSz = *iInOutTargetSz;
	//----------------------------------------------------------------

    char cCurrentCharacter             = 0; // Current Character
    char cRun_Character                = 0; // which Char Run Is Of

    register unsigned int iIndex_Var_1 = 0; // Generic Index Variable
    register unsigned int iIndex_Var_2 = 0; // Another Index Variable

    unsigned short iSequence_Length    = 0; // Length Of Non-Run Sequence
    unsigned short iRun_Length         = 0; // Length Of Char Run So Far

    int iRPos = 0;
    int iWPos = 0;

    char Sequence[_MAX_LEN];               // Buffer For Uncompressible Data

    //----------------------------------------------------------------

    while(iRPos < iSourceSz)
    {
        cCurrentCharacter = sSource[iRPos];
        iRPos++;

        // Haven't Got A Sequence Yet
        if (iSequence_Length == 0)
        {
            // Start A New Run
            if (iRun_Length == 0)
            {
                cRun_Character = cCurrentCharacter;
                ++iRun_Length;
                continue;
            }

            // Got Another Char In The Run
            if (cRun_Character == cCurrentCharacter)
            {
                if( ++iRun_Length == _MAX_LEN )
                {
					if(iWPos == iTargetSz)
					{
						sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
					}
                    sTarget[iWPos++] = (char) _MAX_RUN_HEADER;

					if(iWPos == iTargetSz)
					{
						sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
					}
                    sTarget[iWPos++] = (char) cRun_Character;

					iRun_Length = 0;
                    continue;
                }
            }

            // Got A Different Character Than The Run We Were Building
            // So Write Out The Run And Start A New One Of The New Character.
            if (iRun_Length > 2)
            {
				if(iWPos == iTargetSz)
				{
					sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
				}
                sTarget[iWPos++] = (char) (_RUN | iRun_Length);

				if(iWPos == iTargetSz)
				{
					sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
				}
                sTarget[iWPos++] = (char) cRun_Character;

                iRun_Length = 1;
                cRun_Character   = cCurrentCharacter;
                continue;
            }


            // Run Was Only One Or Two Chars, Make A Sequence Out Of It Instead
            for (iIndex_Var_2 = 0; iIndex_Var_2 < iRun_Length; iIndex_Var_2++); // Copy 1 Or 2 Char Run To Sequence[]
            {
                Sequence[iSequence_Length] = cRun_Character;
                ++iSequence_Length;
                if (iSequence_Length == _MAX_LEN) // If Sequence[] Is Full, Write To String
                {
					if(iWPos == iTargetSz)
					{
						sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
					}
                    sTarget[iWPos++] = (char) _MAX_SEQ_HEADER;

                    for (iIndex_Var_1 = 0; iIndex_Var_1 < iSequence_Length; iIndex_Var_1++)
                    {
						if(iWPos == iTargetSz)
						{
							sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
						}
                        sTarget[iWPos++] = (char) Sequence[iIndex_Var_1];
                    }

                    iSequence_Length = 0;
                }
            }

            iRun_Length = 0;
            Sequence[iSequence_Length++] = cCurrentCharacter;

            if (iSequence_Length == _MAX_LEN) // if Sequence[] Is Full, Write To String
            {
				if(iWPos == iTargetSz)
				{
					sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
				}
                sTarget[iWPos++] = (char) _MAX_SEQ_HEADER;

                for (iIndex_Var_1 = 0; iIndex_Var_1 < iSequence_Length; iIndex_Var_1++)
                {
					if(iWPos == iTargetSz)
					{
						sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
					}
                    sTarget[iWPos++] = (char) Sequence[iIndex_Var_1];
                }

                iSequence_Length = 0;
            }
        }
        else{  // A Sequence Exists

            if (iRun_Length != 0) // if A Run Exists
            {
                if (cCurrentCharacter == cRun_Character )  // Add To Run
                {
                    ++iRun_Length;
                    if (iRun_Length == _MAX_LEN)  // If Run Is Full
                    {
                        // Write Sequence That Precedes Run
						if(iWPos == iTargetSz)
						{
							sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
						}
                        sTarget[iWPos++] = (char) (_SEQ | iSequence_Length);

                        for (iIndex_Var_1 = 0; iIndex_Var_1 < iSequence_Length; iIndex_Var_1++)
                        {
							if(iWPos == iTargetSz)
							{
								sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
							}
                            sTarget[iWPos++] = (char) Sequence[iIndex_Var_1];
                        }

                        // Write Run
						if(iWPos == iTargetSz)
						{
							sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
						}
                        sTarget[iWPos++] = (char) (_RUN | iRun_Length);

						if(iWPos == iTargetSz)
						{
							sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
						}
                        sTarget[iWPos++] = (char) cRun_Character;

                        // And Start Out Fresh
                        iSequence_Length = iRun_Length = 0;

                    }  // End Write Full Run With Existing Sequence

                    continue;

                }  // End Add To Run For Sequence Exists

                // We Couldn't Add To The Run, And A Preceding Sequence
                // Exists, So Write The Sequence And The Run, And
                // Try Starting A New Run With The Current Character.
                // Write Sequence That Precedes Run.
                if(iWPos == iTargetSz)
				{
					sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
				}
                sTarget[iWPos++] = (char) (_SEQ | iSequence_Length);

				for (iIndex_Var_1 = 0; iIndex_Var_1 < iSequence_Length; iIndex_Var_1++)
                {
                    if(iWPos == iTargetSz)
					{
						sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
					}

					sTarget[iWPos++] = (char) Sequence[iIndex_Var_1];
                }

                // Write Run
                if(iWPos == iTargetSz)
				{
					sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
				}
                sTarget[iWPos++] = (char) (_RUN | iRun_Length);

                if(iWPos == iTargetSz)
				{
					sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
				}
                sTarget[iWPos++] = (char) cRun_Character;

                // And Start A New Run w/ cCurrentCharacter
                iSequence_Length = 0;
                iRun_Length = 1;
                cRun_Character = cCurrentCharacter;
                continue;

            }    // End Can't Add To Existing Run, And Preceding Sequence Exists


            // No Run Exists, But A Sequences Does.
            // Try To Create A Run By Looking At cCurrentCharacter
            // And The Last Char Of The Sequence.
            // If That Fails, Add The Char To The Sequence.
            // If The Sequence Is Full, Write It To String.
            // (Slightly Non Optimal) We Could Wait One More Char.

            if( Sequence[iSequence_Length - 1] == cCurrentCharacter)
            {
                cRun_Character = cCurrentCharacter;
                iRun_Length = 2;
                --iSequence_Length;
                continue;
            }

            //Couldn't Make A Run, Add Char To sequence. Maybe Next Time Around...
            Sequence[iSequence_Length++] = cCurrentCharacter;
            if (iSequence_Length == _MAX_LEN) // If The Sequence Is Full, Write Out
            {
                if(iWPos == iTargetSz)
				{
					sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
				}
                sTarget[iWPos++] = (char) _MAX_SEQ_HEADER;

                for (iIndex_Var_1 = 0; iIndex_Var_1 < _MAX_LEN; iIndex_Var_1++)
                {
					if(iWPos == iTargetSz)
					{
						sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
					}
                    sTarget[iWPos++] = (char) Sequence[iIndex_Var_1];
                }

                iSequence_Length = 0;

            }

        }  // End Branch On Sequence Exists

    } // Done With Whole String


    //There May Be Data Left That Hasn't Been Written. If So, Write It
    if (iSequence_Length != 0)  // Write Sequence That Precedes Run
    {
        if(iWPos == iTargetSz)
		{
			sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
		}
        sTarget[iWPos++] = (char) (_SEQ | iSequence_Length);

        for (iIndex_Var_1 = 0; iIndex_Var_1 < iSequence_Length; iIndex_Var_1++)
        {
			if(iWPos == iTargetSz)
			{
				sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
			}
            sTarget[iWPos++] = (char) Sequence[iIndex_Var_1];
        }
    }

    //----------------------------------------------------------------

    // Write Run
    if (iRun_Length != 0)
    {
        if(iWPos == iTargetSz)
		{
			sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
		}
        sTarget[iWPos++] = (char) (_RUN | iRun_Length);

		if(iWPos == iTargetSz)
		{
			sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
		}
        sTarget[iWPos++] = (char) cRun_Character;
    }

    //----------------------------------------------------------------

    sTarget[iWPos] = '\0';
	*iInOutTargetSz = iWPos;

	if(iWPos > *iAlloc)
	{
		*iAlloc = iWPos;
	}

	return sTarget;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *RLEDecode(char *sSource, char *sTarget, int iSourceSz, int *iInOutTargetSz, int *iAlloc)
{
    //----------------------------------------------------------------

	int iTargetSz = *iInOutTargetSz;

	register int CurrentByte = 0; // Current Character
    register unsigned short iIndex_Var_1 = 0;
    register unsigned short Length = 0;

    int Packet_Header  = 0;
    int iRPos = 0;
    int iWPos = 0;

    //----------------------------------------------------------------

    while(iRPos < iSourceSz)
    {
        Packet_Header = sSource[iRPos];
        iRPos++;

        Length = _MAX_LEN & Packet_Header;

        // If It's A Run
        if (Packet_Header & _RUN)
        {
            CurrentByte = sSource[iRPos];
            iRPos++;

            for (iIndex_Var_1 = 0; iIndex_Var_1 < Length; iIndex_Var_1++)
            {
				if(iWPos == iTargetSz)
				{
					sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
				}
                sTarget[iWPos++] = (char) CurrentByte;
            }
        }
        else{ // It's A Sequence

            for (iIndex_Var_1 = 0; iIndex_Var_1 < Length; iIndex_Var_1++)
            {
				if(iWPos == iTargetSz)
				{
					sTarget = RLEResizeBuffer(sTarget, &iTargetSz);
				}
                sTarget[iWPos++] = sSource[iRPos++];
            }
        }
    }

    //----------------------------------------------------------------

    sTarget[iWPos] = '\0';
	*iInOutTargetSz = iWPos;

	if(iWPos > *iAlloc)
	{
		*iAlloc = iWPos;
	}

	return sTarget;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
