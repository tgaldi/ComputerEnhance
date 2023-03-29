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

    void Store( int data )
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
                storage.hi = (char)data;
                break;
            }
            case wide:
            {
                storage.wide = (short)data;
                break;
            }
        }
    }

    int Load()
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

Register* AccessRegister( char reg, char w );
