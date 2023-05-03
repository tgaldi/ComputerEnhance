#pragma once

#include <string>
#include "Types.h"

enum AccessSize: u8
{
    lo = 0x0,
    hi = 0x1,
    wide = 0x2
};

enum Flags: u16
{
    CF = 0x1,
    PF = 0x4,
    AF = 0x10,
    ZF = 0x40,
    SF = 0x80,
    TF = 0x100,
    IF = 0x200,
    DF = 0x400,
    OF = 0x800,
};

struct Register
{
    union Memory
    {
        u8 lo;
        u8 hi;
        u16 wide;
    } storage;

    AccessSize size = AccessSize::wide;
    std::string name = "invalid";

    std::string toString()
    {
        if( name.length() == 2 )
        {
            return name;
        }

        switch( size )
        {
            case lo:
            {
                return name.append( "L" );
            }
            case hi:
            {
                return name.append( "H" );
            }
            case wide:
            {
                return name.append( "X" );
            }
        }
        return name;
    }

    void Store( u16 data )
    {
        switch( size )
        {
            case lo:
            {
                storage.lo = (s8)data;
                break;
            }
            case hi:
            {
                storage.hi = data >> 8;
                break;
            }
            case wide:
            {
                storage.wide = data;
                break;
            }
        }
    }

    s16 Load()
    {
        switch( size )
        {
            case lo:
            {
                return storage.lo;
            }
            case hi:
            {
                return storage.hi;
            }
            case wide:
            {
                return storage.wide;
            }
        }

        return storage.wide;
    }
};

struct EffectiveRegisters
{
    Register* First;
    Register* Second;
};

struct InsturctionPointer
{
    u16 offset;
};

extern InsturctionPointer IP;
extern Register FLAGS;

extern Register A;
extern Register B;
extern Register C;
extern Register D;
extern Register SP;
extern Register BP;
extern Register SI;
extern Register DI;
extern Register INVALID;

extern Register* registers[];
extern EffectiveRegisters effectiveRegisters[];

void SetFlag( Flags flag, bool value );
Register* AccessRegister( u8 reg, u8 w );
bool GetFlagState( Flags flag );
std::string FlagStateToString( Flags flag );
s16 LoadEffectiveAddress( u8 rm, u8 w, s16 disp );
