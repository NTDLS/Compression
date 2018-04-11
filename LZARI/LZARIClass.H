#ifndef _LZARICompression_H
#define _LZARICompression_H
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Size Of Ring Buffer
#define SizeOfRingBuffer 4096

// Upper Limit For MatchLength
#define MatchLengthLimit 128

// Encode String Into Position And Length If MatchLength Is Greater Than This
#define THRESHOLD         2

// Index For Root Of Binary Search Trees
#define NIL SizeOfRingBuffer

// Character Code = 0, 1, ..., N_CHAR - 1
#define N_CHAR    (256 - THRESHOLD + MatchLengthLimit)

class LZARI{
    public:
        bool Deflate(char *inSrcBuffer, char *outTarBuffer, unsigned long inSrcSize, unsigned long *outTarSize);
        bool Inflate(char *inSrcBuffer, char *outTarBuffer, unsigned long inSrcSize, unsigned long *outTarSize);
        void Initialize(void);

    private:
        char *CompressSource;
        char *CompressTarget;

        unsigned long SourcePos;
        unsigned long TargetPos;

        unsigned long SourceSize;
        unsigned long TargetSize;

        // Ring Buffer Of Size SizeOfRingBuffer, With Extra MatchLengthLimit-1
        // Bytes To Facilitate String Comparison
        unsigned char  TextBuffer[SizeOfRingBuffer + MatchLengthLimit - 1];

        // MatchLength Of Longest Match. Set By The InsertNode() Procedure.
        int MatchLength;

        // Left Child. Set By The InsertNode() Procedure.
        int LChild[SizeOfRingBuffer + 1];

        // Right Child. Set By The InsertNode() Procedure.
        int RChild[SizeOfRingBuffer + 257];

        // Parent. Set By The InsertNode() Procedure.
        int Parent[SizeOfRingBuffer + 1];

        // Used And Set By PutBit() Procedure
        unsigned int PutBitBuffer;
        unsigned int PutBitMask;

        // Used And Set By GetBit() Procedure
        unsigned int GetBitBuffer;
        unsigned int GetBitMask;

        // Set By The InsertNode() Procedure.
        int MatchPosition;

        int Low;
        int High;
        int Value;

        // Counts For Magnifying Low And High Around Q2
        int Shifts;

        int Character2Symbol[N_CHAR + 1];
        int Symbol2Character[N_CHAR + 1];
        int xCypCharacter2Symbol[N_CHAR + 1];
        int xCypSymbol2Character[N_CHAR + 1];

        // Frequency For Symbols
        unsigned int SymbolsFrequency[N_CHAR + 1];
        unsigned int xCypSymbolsFrequency[N_CHAR + 1];

        // Cumulative Frequency For Symbols
        unsigned int CumulativeSymbols[N_CHAR + 1];
        unsigned int xCypCumulativeSymbols[N_CHAR + 1];

        // Cumulative Frequency For Positions
        unsigned int CumulativePosition[SizeOfRingBuffer + 1];
        unsigned int xCypCumulativePosition[SizeOfRingBuffer + 1];

        //-------------------------------------------------------

        int BinarySearchPos(unsigned int x);
        int BinarySearchSym(unsigned int x);
        int DecodeChar(void);
        int DecodePosition(void);
        int GetBit(void);

        void DeleteNode(int p);
        void EncodeChar(int ch);
        void EncodeEnd(void);
        void EncodePosition(int position);
        void FlushBitBuffer(void);
        void InitSingle(void);
        void InitTree(void);
        void InsertNode(int r);
        void Output(int bit);
        void PutBit(int bit);
        void StartDecode(void);
        void StartModel(void);
        void UpdateModel(int sym);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//********* Arithmetic Compression *********

#define M    15

// Q1 ( = 2 To The M) Must Be Sufficiently Large, But Not So Large As The Unsigned Long 4 * Q1 * (Q1 - 1) OverFlows.

#define Q1    (1UL << M)
#define Q2    (2 * Q1)
#define Q3    (3 * Q1)
#define Q4    (4 * Q1)
#define MAX_CUM (Q1 - 1)

// Character Code = 0, 1, ..., N_CHAR - 1
#define N_CHAR    (256 - THRESHOLD + MatchLengthLimit)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

