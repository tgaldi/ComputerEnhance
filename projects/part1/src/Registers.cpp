#include "Registers.h"

#include <stdio.h>
#include "Tables.h"

Register FLAGS { 0, AccessSize::wide, "FLAGS" };

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

void SetFlag( Flags flag, bool value )
{
    if( value )
    {
        FLAGS.storage.wide |= (1 << (int)flag);
    }
    else
    {
        FLAGS.storage.wide &= ~(1 << (int)flag);
    }
}

std::string GetFlagState( Flags flag )
{
    std::string flagState;
    switch( flag )
    {
        case Flags::CF:
        {
            flagState.append( "CF" );
            break;
        }
        case Flags::PF:
        {
            flagState.append( "PF" );
            break;
        }
        case Flags::AF:
        {
            flagState.append( "AF" );
            break;
        }
        case Flags::ZF:
        {
            flagState.append( "ZF" );
            break;
        }
        case Flags::SF:
        {
            flagState.append( "SF" );
            break;
        }
        case Flags::TF:
        {
            flagState.append( "TF" );
            break;
        }
        case Flags::IF:
        {
            flagState.append( "IF" );
            break;
        }
        case Flags::DF:
        {
            flagState.append( "DF" );
            break;
        }
        case Flags::OF:
        {
            flagState.append( "OF" );
            break;
        }
    }

    flagState.append( (FLAGS.storage.wide & (1 << (int)flag)) ? ": 1\t" : ": 0\t" );
    return flagState;
}