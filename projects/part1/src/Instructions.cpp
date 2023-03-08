#include <string.h>
#include <stdlib.h>
#include <bitset>
#include <iostream>

#include "Instructions.h"
#include "Tables.h"

char getbits( char x, int p, int n )
{
    return (x >> (p + 1 - n)) & ~(~0 << n);
}

std::string registerLookup( char reg, char w )
{
    int regIndex = reg + (8 * w);
    if( regIndex < (sizeof( registerLookupTable ) / sizeof( char )) )
        return registerLookupTable[regIndex];

    return "invalid register";
}

Instruction GetInstruction( char op )
{
    if( getbits( op, 4, 1 ) )
        return instructionTable[0];

    char opCode = getbits( op, 7, 6 );

    Instruction* opPtr = instructionTable;
    int tblSize = sizeof( instructionTable ) / sizeof( Instruction );

    while( opPtr < instructionTable + tblSize - 1 )
    {
        if( opPtr->opCode == opCode )
            break;

        ++opPtr;
    }

    return *(opPtr);
}

enum mode
{
    mem_mode,
    byte_mode,
    word_mode,
    reg_mode,
    mode_count
};
Operation* Mov_To_From( char*& instrPtr )
{
    Operation* op = (Operation*)calloc( 1, sizeof( Operation ) );
    op->opName = "mov";

    char hi = *instrPtr;
    char d = getbits( hi, 1, 1 );
    char w = getbits( hi, 0, 1 );
    // std::cout << std::bitset<8>( *instrPtr ) << std::endl;

    ++instrPtr;
    char lo = *instrPtr;
    // std::cout << std::bitset<8>( *instrPtr ) << std::endl;

    char mod = getbits( lo, 7, 2 );
    char reg = getbits( lo, 5, 3 );
    char rm = getbits( lo, 2, 3 );

    switch( mod )
    {
        case byte_mode:
        {
            if( d == 0 )
                op->dest = "[" + effectiveAddressTable[rm] + "]";
            else
                op->dest = registerLookup( reg, w );

            if( d == 1 )
                op->src = "[" + effectiveAddressTable[rm] + " + " + std::to_string( *(++instrPtr) ) + "]";
            else
                op->src = registerLookup( reg, w );

            break;
        }
        case word_mode:
        {
            lo = *(++instrPtr);

            if( d == 0 )
                op->dest = "[" + effectiveAddressTable[rm] + "]";
            else
                op->dest = registerLookup( reg, w );

            if( d == 1 )
                op->src = "[" + effectiveAddressTable[rm] + " + " + std::to_string( (char)*(++instrPtr) << 8 | (unsigned char)lo ) + "]";
            else
                op->src = registerLookup( reg, w );

            break;
        }
        case reg_mode:
        {
            op->dest = d ? registerLookup( reg, w ) : registerLookup( rm, w );

            op->src = d ? registerLookup( rm, w ) : registerLookup( reg, w );
            break;
        }
        case mem_mode:
        {
            if( d == 0 )
                op->dest = "[" + effectiveAddressTable[rm] + "]";
            else
                op->dest = registerLookup( reg, w );

            if( d == 1 )
                op->src = "[" + effectiveAddressTable[rm] + "]";
            else
                op->src = registerLookup( reg, w );

            break;
        }
        default:
            // "mode not implemented";
        break;
    }

    return op;
}

Operation* Mov_Immediate( char*& instrPtr )
{
    Operation* op = (Operation*)calloc( 1, sizeof( Operation ) );
    op->opName = "mov";

    char hi = *instrPtr;
    char w = getbits( hi, 3, 1 );
    char reg = getbits( hi, 2, 3 );
    // std::cout << std::bitset<8>( *instrPtr ) << std::endl;

    op->dest = registerLookup( reg, w );

    char lo = *(++instrPtr);
    // std::cout << std::bitset<8>( *instrPtr ) << std::endl;

    if( w )
    {
        op->src = std::to_string( (char)*(++instrPtr) << 8 | (unsigned char)lo );
    }
    else
    {
        op->src = std::to_string( lo );
    }

    return op;
}