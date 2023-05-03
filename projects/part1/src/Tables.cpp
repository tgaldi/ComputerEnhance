#include "Tables.h"

const char* registerLookupTable[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };

const char* effectiveAddressTable[] = { "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx" };

const char* jumpTable[] = { "jo", "jno", "jb", "jnb", "je", "jne", "jbe", "jnbe", "js", "jns", "jp", "jnp", "jl", "jnl", "jle", "jnle" };

const char* loopTable[]{ "loopnz", "loopz", "loop", "jcxz" };

const char* toFromOpTable[]{ "add", "mov", "no_op", "no_op", "no_op", "sub", "no_op", "cmp", "mov" };

u8 GetBits( u8 x, int p, int n )
{
    return (x >> (p + 1 - n)) & ~(~0 << n);
}

const char* RegisterLookup( u8 reg, u8 w )
{
    int regIndex = reg + (8 * w);
    if( regIndex < (sizeof( registerLookupTable ) / sizeof( char )) )
        return registerLookupTable[regIndex];

    return "invalid register";
}