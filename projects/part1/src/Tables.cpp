#include "Tables.h"

char GetBits( char x, int p, int n )
{
    return (x >> (p + 1 - n)) & ~(~0 << n);
}

const char* RegisterLookup( char reg, char w )
{
    int regIndex = reg + (8 * w);
    if( regIndex < (sizeof( registerLookupTable ) / sizeof( char )) )
        return registerLookupTable[regIndex];

    return "invalid register";
}