#include "Registers.h"
#include <stdio.h>

Register A = { 0, AccessSize::wide, "A" };
Register B = { 0, AccessSize::wide, "B" };
Register C = { 0, AccessSize::wide, "C" };
Register D = { 0, AccessSize::wide, "D" };
Register SP = { 0, AccessSize::wide, "SP" };
Register BP = { 0, AccessSize::wide, "BP" };
Register SI = { 0, AccessSize::wide, "SI" };
Register DI = { 0, AccessSize::wide, "DI" };
Register INVALID;

Register* registers[] = { &A, &C, &D, &B, &A, &C, &D, &B, &A, &C, &D, &B, &SP, &BP, &SI, &DI };

Register* AccessRegister( char reg, char w )
{
    int regIndex = reg + (8 * w);
    if( regIndex < (sizeof( registers ) / sizeof( char )) )
    {
        Register* access = registers[regIndex];
        access->size = (AccessSize)(w + (w * 1));
        return access;
    }

    printf( "Invalid register access: %d %d", reg, w );
    return &INVALID;
}