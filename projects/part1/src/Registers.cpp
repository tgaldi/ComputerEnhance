#include "Registers.h"

#include <stdio.h>
#include "Tables.h"

InsturctionPointer IP = { 0 };
Register FLAGS{ 0, AccessSize::wide, "FLAGS" };

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

EffectiveRegisters effectiveRegisters[] = { { &B, &SI }, { &B, &DI }, { &BP, &SI }, { &BP, &DI }, { &SI }, { &DI}, { &BP }, { &B } };

Register* AccessRegister( u8 reg, u8 w )
{
    int regIndex = reg + (8 * w);
    if( regIndex < (sizeof( registers ) / sizeof( u8 )) )
    {
        Register* access = registers[regIndex];
        access->size = (AccessSize)(w + (w * 1));
        return access;
    }

    printf( "Invalid register access: %d %d", reg, w );
    return &INVALID;
}

s16 LoadEffectiveAddress( u8 rm, u8 w, s16 disp )
{
    EffectiveRegisters* effRegisters = &effectiveRegisters[rm];
    effRegisters->First->size = (AccessSize)(w + (w * 1));
    if( effRegisters->Second != nullptr )
        effRegisters->Second->size = (AccessSize)(w + (w * 1));

    return effRegisters->First->Load() + ((effRegisters->Second != nullptr) ? effRegisters->Second->Load() : 0) + disp;
}

void SetFlag( Flags flag, bool value )
{
    if( value )
    {
        FLAGS.storage.wide |= (1 << (u8)flag);
    }
    else
    {
        FLAGS.storage.wide &= ~(1 << (u8)flag);
    }
}

bool GetFlagState( Flags flag )
{
    return (FLAGS.storage.wide & (1 << (int)flag)) != 0;
}

std::string FlagStateToString( Flags flag )
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

    flagState.append( (FLAGS.storage.wide & (1 << (int)flag)) ? ":1 " : ":0 " );
    return flagState;
}