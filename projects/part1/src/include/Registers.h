#pragma once

#include <string>

enum AccessSize: char
{
    lo = 0x0,
    hi = 0x1,
    wide = 0x2
};
union Memory
{
    char lo;
    char hi;
    short wide;
};

enum Flags: unsigned short
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
    Memory storage;
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

    void Store( short data )
    {
        switch( size )
        {
            case lo:
            {
                storage.lo = (char)data;
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

    short Load()
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

struct InsturctionPointer
{
    short offset;
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

void SetFlag( Flags flag, bool value );
Register* AccessRegister( char reg, char w );
bool GetFlagState( Flags flag );
std::string FlagStateToString( Flags flag );
