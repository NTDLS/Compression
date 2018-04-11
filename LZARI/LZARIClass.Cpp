#ifndef _Compression_Cpp
#define _Compression_Cpp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <String.H>

#include "LZARIClass.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::PutBit(int bit)  /* Output One Bit (Bit = 0,1) */
{
    if (bit)
    {
        PutBitBuffer |= PutBitMask;
    }

    if ((PutBitMask >>= 1) == 0)
    {
        CompressTarget[TargetPos] = PutBitBuffer;
        TargetPos++;
        PutBitBuffer = 0;
        PutBitMask = 128;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::FlushBitBuffer(void)  /* Send remaining bits */
{
    int LocalLoop = 0;

    while(LocalLoop < 7)
    {
        PutBit(0);
        LocalLoop++;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LZARI::GetBit(void)    /* Get one bit (0 or 1) */
{
    if ((GetBitMask >>= 1) == 0)
    {
        GetBitBuffer = CompressSource[SourcePos];
        SourcePos++;
        GetBitMask = 128;
    }
    return( (GetBitBuffer & GetBitMask) != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/********** Multiple Binary Trees **********/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::InitTree(void)  /* Initialize Trees */
{
    int LocalLoop;

    /* For LocalLoop = 0 to SizeOfRingBuffer - 1, RChild[LocalLoop] and LChild[LocalLoop]
    will be the right and left children of node LocalLoop.
    These nodes need not be initialized. Also, Parent[LocalLoop] is the
    parent of node LocalLoop. These are initialized to NIL (= SizeOfRingBuffer),
    which stands for 'not used.' For LocalLoop = 0 to 255,
    RChild[SizeOfRingBuffer + LocalLoop + 1] is the root of the tree for strings
    that begin with character LocalLoop.  These are initialized
    to NIL.  Note there are 256 trees. */

    /* Root */
    for (LocalLoop = SizeOfRingBuffer + 1; LocalLoop <= SizeOfRingBuffer + 256; LocalLoop++)
    {
        RChild[LocalLoop] = NIL;
    }

    /* Node */
    for (LocalLoop = 0; LocalLoop < SizeOfRingBuffer; LocalLoop++)
    {
        Parent[LocalLoop] = NIL;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Inserts string of length MatchLengthLimit, TextBuffer[r..r+MatchLengthLimit-1], into one of the
trees (TextBuffer[r]'th tree) and returns the longest-match position
and length via the global variables MatchPosition and MatchLength.
If MatchLength = MatchLengthLimit, then removes the old node in favor of the new
one, because the old one will be deleted sooner.
Note r plays double role, as tree node and position in buffer. */

void LZARI::InsertNode(int r)
{
    int Value_A, Value_B, Cmp, Temp;
    unsigned char *LocalKey;

    Cmp = 1;
    LocalKey = &TextBuffer[r];
    Value_B = SizeOfRingBuffer + 1 + LocalKey[0];
    RChild[r] = LChild[r] = NIL;
    MatchLength = 0;

    while(1)
    {
        if (Cmp >= 0)
        {
            if (RChild[Value_B] != NIL) Value_B = RChild[Value_B];
            else {
                RChild[Value_B] = r;
                Parent[r] = Value_B;
                return;
            }
        }
        else {
            if (LChild[Value_B] != NIL)
            {
                Value_B = LChild[Value_B];
            }
            else {
                LChild[Value_B] = r;
                Parent[r] = Value_B;
                return;
            }
        }

        for (Value_A = 1; Value_A < MatchLengthLimit; Value_A++)
        {
            if((Cmp = LocalKey[Value_A] - TextBuffer[Value_B + Value_A]) != 0)  break;
        }

        if (Value_A > THRESHOLD)
        {
            if (Value_A > MatchLength)
            {
                MatchPosition = (r - Value_B) & (SizeOfRingBuffer - 1);
                if ((MatchLength = Value_A) >= MatchLengthLimit) break;
            }
            else if (Value_A == MatchLength){
                if ((Temp = (r - Value_B) & (SizeOfRingBuffer - 1)) < MatchPosition)
                    MatchPosition = Temp;
            }
        }
    }

    Parent[r] = Parent[Value_B];
    LChild[r] = LChild[Value_B];
    RChild[r] = RChild[Value_B];

    Parent[LChild[Value_B]] = r;
    Parent[RChild[Value_B]] = r;

    if(RChild[Parent[Value_B]] == Value_B)
        RChild[Parent[Value_B]] = r;
    else LChild[Parent[Value_B]] = r;

    Parent[Value_B] = NIL;  /* Remove Value_B */
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::DeleteNode(int p)    /* Delete node p from tree */
{
    int  Value_A;

    if (Parent[p] == NIL)
    {
        return;  /* Not In Tree */
    }

    if (RChild[p] == NIL)
    {
        Value_A = LChild[p];
    }
    else if (LChild[p] == NIL) {
         Value_A = RChild[p];
    }
    else {
        Value_A = LChild[p];
        if (RChild[Value_A] != NIL)
        {
            do{
                Value_A = RChild[Value_A];
            } while (RChild[Value_A] != NIL);

            RChild[Parent[Value_A]] = LChild[Value_A];
            Parent[LChild[Value_A]] = Parent[Value_A];
            LChild[Value_A] = LChild[p];
            Parent[LChild[p]] = Value_A;
        }
        RChild[Value_A] = RChild[p];
        Parent[RChild[p]] = Value_A;
    }

    Parent[Value_A] = Parent[p];

    if (RChild[Parent[p]] == p)
        RChild[Parent[p]] = Value_A;
            else LChild[Parent[p]] = Value_A;

    Parent[p] = NIL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/********** Arithmetic Compression **********/

void LZARI::StartModel(void)  /* Initialize model */
{
    int ch, sym, i;

    CumulativeSymbols[N_CHAR] = 0;
    for(sym = N_CHAR; sym >= 1; sym--)
    {
        ch = sym - 1;
        Character2Symbol[ch] = sym;
        Symbol2Character[sym] = ch;
        SymbolsFrequency[sym] = 1;
        CumulativeSymbols[sym - 1] = CumulativeSymbols[sym] + SymbolsFrequency[sym];
    }

    SymbolsFrequency[0] = 0;  /* sentinel (!= SymbolsFrequency[1]) */
    CumulativePosition[SizeOfRingBuffer] = 0;

    for (i = SizeOfRingBuffer; i >= 1; i--)
    {
        CumulativePosition[i - 1] = CumulativePosition[i] + 10000 / (i + 200);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::UpdateModel(int sym)
{
    int i, c, ch_i, ch_sym;

    if (CumulativeSymbols[0] >= MAX_CUM)
    {
        c = 0;
        for (i = N_CHAR; i > 0; i--)
        {
            CumulativeSymbols[i] = c;
            c += (SymbolsFrequency[i] = (SymbolsFrequency[i] + 1) >> 1);
        }
        CumulativeSymbols[0] = c;
    }

    for (i = sym; SymbolsFrequency[i] == SymbolsFrequency[i - 1]; i--);

    if (i < sym)
    {
        ch_i = Symbol2Character[i];
        ch_sym = Symbol2Character[sym];
        Symbol2Character[i] = ch_sym;
        Symbol2Character[sym] = ch_i;
        Character2Symbol[ch_i] = sym;
        Character2Symbol[ch_sym] = i;
    }

    SymbolsFrequency[i]++;

    while (--i >= 0)
        CumulativeSymbols[i]++;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::Output(int bit)  /* Output 1 Bit, Followed By Its Complements */
{
    PutBit(bit);
    for( ; Shifts > 0; Shifts--)
        PutBit(!bit);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::EncodeChar(int ch)
{
    int  sym;
    unsigned long int  range;

    sym = Character2Symbol[ch];
    range = High - Low;
    High = Low + (range * CumulativeSymbols[sym - 1]) / CumulativeSymbols[0];
    Low +=         (range * CumulativeSymbols[sym    ]) / CumulativeSymbols[0];
    for (;;)
    {
        if (High <= Q2)
            Output(0);
        else if (Low >= Q2)
        {
            Output(1);
            Low -= Q2;
            High -= Q2;
        }
        else if (Low >= Q1 && High <= Q3)
        {
            Shifts++;
            Low -= Q1;
            High -= Q1;
        }
        else break;
        Low += Low;
        High += High;
    }
    UpdateModel(sym);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::EncodePosition(int position)
{
    unsigned long int  range;

    range = High - Low;
    High = Low + (range * CumulativePosition[position     ]) / CumulativePosition[0];
    Low +=         (range * CumulativePosition[position + 1]) / CumulativePosition[0];
    for (;;)
    {
        if (High <= Q2) Output(0);
        else if (Low >= Q2)
        {
            Output(1);
            Low -= Q2;
            High -= Q2;
        }
        else if (Low >= Q1 && High <= Q3)
        {
            Shifts++;
            Low -= Q1;
            High -= Q1;
        }
        else break;
        Low += Low;
        High += High;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::EncodeEnd(void)
{
    Shifts++;
    if (Low < Q1)
        Output(0);
            else Output(1);

    FlushBitBuffer();  /* flush bits remaining in buffer */
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* 1      if x >= CumulativeSymbols[1],
N_CHAR if CumulativeSymbols[N_CHAR] > x,
i such that CumulativeSymbols[i - 1] > x >= CumulativeSymbols[i] otherwise */

int LZARI::BinarySearchSym(unsigned int x)
{
    int i, j, k;

    i = 1;
    j = N_CHAR;
    while (i < j)
    {
        k = (i + j) / 2;
        if (CumulativeSymbols[k] > x)
            i = k + 1;
                else j = k;
    }
    return i;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* 0 if x >= CumulativePosition[1],
SizeOfRingBuffer - 1 if CumulativePosition[SizeOfRingBuffer] > x,
i such that CumulativePosition[i] > x >= CumulativePosition[i + 1] otherwise */

int LZARI::BinarySearchPos(unsigned int x)
{
    int i, j, k;

    i = 1;
    j = SizeOfRingBuffer;
    while (i < j)
    {
        k = (i + j) / 2;
        if (CumulativePosition[k] > x)
            i = k + 1;
                else j = k;
    }
    return i - 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::StartDecode(void)
{
    int i;
    for (i = 0; i < M + 2; i++)
    {
        Value = 2 * Value + GetBit();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LZARI::DecodeChar(void)
{
    int  sym, ch;
    unsigned long int  range;

    range = High - Low;
    sym = BinarySearchSym( (unsigned int) (((Value - Low + 1) * CumulativeSymbols[0] - 1) / range));
    High = Low + (range * CumulativeSymbols[sym - 1]) / CumulativeSymbols[0];
    Low +=         (range * CumulativeSymbols[sym    ]) / CumulativeSymbols[0];

    while(1)
    {
        if (Low >= Q2)
        {
            Value -= Q2;
            Low -= Q2;
            High -= Q2;
        }
        else if (Low >= Q1 && High <= Q3)
        {
            Value -= Q1;
            Low -= Q1;
            High -= Q1;
        }
        else if (High > Q2) break;

        Low += Low;
        High += High;
        Value = 2 * Value + GetBit();
    }

    ch = Symbol2Character[sym];
    UpdateModel(sym);
    return ch;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LZARI::DecodePosition(void)
{
    int position;
    unsigned long int  range;

    range = High - Low;
    position = BinarySearchPos((unsigned int) (((Value - Low + 1) * CumulativePosition[0] - 1) / range));
    High = Low + (range * CumulativePosition[position     ]) / CumulativePosition[0];
    Low +=         (range * CumulativePosition[position + 1]) / CumulativePosition[0];

    while(1)
    {
        if (Low >= Q2)
        {
            Value -= Q2;
            Low -= Q2;
            High -= Q2;
        }
        else if (Low >= Q1 && High <= Q3)
        {
            Value -= Q1;
            Low -= Q1;
            High -= Q1;
        }
        else if (High > Q2) break;

        Low += Low;
        High += High;
        Value = 2 * Value + GetBit();
    }
    return position;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::Initialize(void)
{
    int LocalLoop = 0;

    CompressSource = NULL;
    CompressTarget = NULL;

    SourceSize = 0;
    TargetSize = 0;

    while(LocalLoop != (SizeOfRingBuffer + MatchLengthLimit - 1))
    {
        TextBuffer[LocalLoop] = 0;
        LocalLoop++;
    }
    LocalLoop = 0;

    while(LocalLoop != (SizeOfRingBuffer + 257))
    {
        RChild[LocalLoop] = 0;
        LocalLoop++;
    }
    LocalLoop = 0;

    while(LocalLoop != (SizeOfRingBuffer + 1))
    {
        CumulativePosition[LocalLoop] = 0;
        LChild[LocalLoop] = 0;
        Parent[LocalLoop] = 0;
        LocalLoop++;
    }
    LocalLoop = 0;

    while(LocalLoop < (N_CHAR + 1))
    {
        Symbol2Character[LocalLoop] = 0;
        SymbolsFrequency[LocalLoop] = 0;
        CumulativeSymbols[LocalLoop] = 0;
        Character2Symbol[LocalLoop] = 0;
        LocalLoop++;
    }
    LocalLoop = 0;

    InitSingle();

    StartModel();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LZARI::InitSingle(void)
{
    SourcePos = 0;
    TargetPos = 0;

    GetBitBuffer = 0;
    GetBitMask = 0;
    MatchLength = 0;
    MatchPosition = 0;
    PutBitBuffer = 0;
    PutBitMask = 128;
    Shifts = 0;
    Value = 0;
    High = Q4;
    Low = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

