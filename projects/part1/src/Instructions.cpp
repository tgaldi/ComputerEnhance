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
Operation Mov_To_From( char*& instrPtr )
{
    Operation op;
    op.opName = "mov";

    char hi = *instrPtr;
    // std::cout << std::bitset<8>( *instrPtr ) << std::endl;

    char d = getbits( hi, 1, 1 );
    char w = getbits( hi, 0, 1 );

    char lo = *(++instrPtr);
    // std::cout << std::bitset<8>( *instrPtr ) << std::endl;

    char mod = getbits( lo, 7, 2 );
    char reg = getbits( lo, 5, 3 );
    char rm = getbits( lo, 2, 3 );

    switch( mod )
    {
        case byte_mode:
        {
            op.dest = d
                ? registerLookup( reg, w )
                : "[" + effectiveAddressTable[rm] + "]";

            op.src = d
                ? "[" + effectiveAddressTable[rm] + " + " + std::to_string( *(++instrPtr) ) + "]"
                : registerLookup( reg, w );

            break;
        }
        case word_mode:
        {
            lo = *(++instrPtr);

            op.dest = d
                ? registerLookup( reg, w )
                : "[" + effectiveAddressTable[rm] + "]";

            op.src = d
                ? op.src = "[" + effectiveAddressTable[rm] + " + " + std::to_string( (char)*(++instrPtr) << 8 | (unsigned char)lo ) + "]"
                : registerLookup( reg, w );

            break;
        }
        case reg_mode:
        {
            op.dest = d ? registerLookup( reg, w ) : registerLookup( rm, w );

            op.src = d ? registerLookup( rm, w ) : registerLookup( reg, w );
            break;
        }
        case mem_mode:
        {
            op.dest = d
                ? registerLookup( reg, w )
                : "[" + effectiveAddressTable[rm] + "]";

            op.src = d
                ? "[" + effectiveAddressTable[rm] + "]"
                : registerLookup( reg, w );

            break;
        }
        default:
            // "mode not implemented";
        break;
    }

    return op;
}

Operation Mov_Immediate( char*& instrPtr )
{
    Operation op;
    op.opName = "mov";

    char hi = *instrPtr;
    // std::cout << std::bitset<8>( *instrPtr ) << std::endl;

    char w = getbits( hi, 3, 1 );
    char reg = getbits( hi, 2, 3 );

    op.dest = registerLookup( reg, w );

    char lo = *(++instrPtr);
    // std::cout << std::bitset<8>( *instrPtr ) << std::endl;

    op.src = w
        ? std::to_string( (char)*(++instrPtr) << 8 | (unsigned char)lo )
        : std::to_string( lo );

    return op;
}