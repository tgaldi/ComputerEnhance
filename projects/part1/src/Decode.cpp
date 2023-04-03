#include "Decode.h"

#include <string>
#include <stdlib.h>
#include <bitset>
#include <iostream>

#include "Tables.h"
#include "Registers.h"

static void SimulateOperation( OperationType opType, Operation* op, Register* destReg, short srcValue );
static std::string BuildEffectiveRegisterEncoding( const char* registerName, int* data = nullptr );

Operation DecodeInstruction( char*& opstream )
{
    char hi = *opstream;

    Operation operation;
    if( GetBits( hi, 7, 2 ) == 0b0 )
    {
        if( GetBits( hi, 2, 1 ) )
        {
            operation = Immediate_Accumulator( opstream );
        }
        else
        {
            operation = To_From( opstream );
        }
    }
    else
    {
        if( GetBits( hi, 7, 4 ) == 0b111 )
        {
            operation = Jump( opstream );
        }
        else if( GetBits( hi, 7, 4 ) == 0b1110 )
        {
            operation = Loop( opstream );
        }
        else if( GetBits( hi, 6, 5 ) != 0b0 )
        {
            if( GetBits( hi, 4, 1 ) )
            {
                operation = Mov_Immediate( opstream );
            }
            else
            {
                operation = To_From( opstream );
            }
        }
        else
        {
            operation = Immediate( opstream );
        }
    }

    return operation;
}


Operation Loop( char*& opstream )
{
    Operation op;

    char hi = *opstream;
    op.name = loopTable[GetBits( hi, 3, 4 )];

    char inc8 = *(++opstream);
    op.dest = std::to_string( inc8 );

    // std::cout << std::bitset<8>( hi ) << std::endl;
    // std::cout << std::bitset<8>( inc8 ) << std::endl;

    return op;

}

Operation Jump( char*& opstream )
{
    Operation op;

    char hi = *opstream;
    op.name = jumpTable[GetBits( hi, 3, 4 )];

    char inc8 = *(++opstream);
    op.dest = std::to_string( inc8 );

    // std::cout << std::bitset<8>( hi ) << std::endl;
    // std::cout << std::bitset<8>( inc8 ) << std::endl;

    return op;
}

Operation Immediate_Accumulator( char*& opstream )
{
    Operation op;

    char hi = *opstream;
    op.name = toFromOpTable[GetBits( hi, 6, 4 )];

    char w = GetBits( hi, 0, 1 );
    char reg = 0b0;

    // std::cout << std::bitset<8>( hi ) << std::endl;

    op.dest = RegisterLookup( reg, w );

    char lo = *(++opstream);
    // std::cout << std::bitset<8>( lo ) << std::endl;

    op.src = w
        ? std::to_string( (char)*(++opstream) << 8 | (unsigned char)lo )
        : std::to_string( lo );

    op.dest += ",";
    return op;
}

Operation Mov_Immediate( char*& opstream )
{
    Operation op;
    op.name = "mov";

    char hi = *opstream;
    // std::cout << std::bitset<8>( *opstream ) << std::endl;

    char w = GetBits( hi, 3, 1 );
    char reg = GetBits( hi, 2, 3 );

    op.dest = RegisterLookup( reg, w );

    Register* dest = AccessRegister( reg, w );

    char lo = *(++opstream);
    // std::cout << std::bitset<8>( *opstream ) << std::endl;

    short data = w
        ? (char)*(++opstream) << 8 | (unsigned char)lo
        : lo;

    op.src = std::to_string( data );

    // op.src = w
    //     ? std::to_string( (char)*(++opstream) << 8 | (unsigned char)lo )
    //     : std::to_string( lo );

    dest->Store( data );

    op.dest += ",";
    return op;
}

Operation Immediate( char*& opstream )
{
    Operation op;

    char hi = *opstream;
    char s = GetBits( hi, 1, 1 );
    char w = GetBits( hi, 0, 1 );

    // std::cout << std::bitset<8>( *opstream ) << std::endl;
    // std::cout << "s: " << std::bitset<1>( s ) << std::endl;
    // std::cout << "w: " << std::bitset<1>( w ) << std::endl;


    char lo = *(++opstream);
    char mod = GetBits( lo, 7, 2 );
    char type = GetBits( lo, 5, 3 );
    char rm = GetBits( lo, 2, 3 );

    // std::cout << std::bitset<8>( lo ) << std::endl;
    // std::cout << "mod: " << std::bitset<2>( mod ) << std::endl;
    // std::cout << "type: " << std::bitset<3>( type ) << std::endl;
    // std::cout << "rm: " << std::bitset<3>( rm ) << std::endl;

    op.name = toFromOpTable[type];

    Register* destReg = AccessRegister( rm, 0 );
    switch( mod )
    {
        case mem_mode:
        {
            if( rm == 0b110 )
            {
                lo = *(++opstream);
                int data = (char)*(++opstream) << 8 | (unsigned char)lo;

                op.dest = BuildEffectiveRegisterEncoding( RegisterLookup( rm, 1 ), &data );

                destReg = AccessRegister( rm, 1 );
            }
            else
            {
                op.dest = BuildEffectiveRegisterEncoding( RegisterLookup( rm, 1 ) );
            }
            break;
        }

        case byte_mode:
        {
            int data = *(++opstream);
            op.dest = BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], &data );

            break;
        }

        case word_mode:
        {
            lo = *(++opstream);
            int data = (char)*(++opstream) << 8 | (unsigned char)lo;

            op.dest = BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], &data );

            break;
        }

        case reg_mode:
        {
            op.dest = RegisterLookup( rm, w );
            break;
        }
    }

    lo = *(++opstream);
    char sw = (s << 1) | w;
    switch( sw )
    {
        case 0: // unsigned byte
        {
            op.src = std::to_string( lo );
            break;
        }
        case 1: // unsigned word
        {
            op.src = std::to_string( (char)*(++opstream) << 8 | (unsigned char)lo );
            break;
        }
        case 2: // signed byte
        {
            op.src = std::to_string( lo );
            break;
        }
        case 3: // signed word
        {
            op.src = std::to_string( (short)lo );
            break;
        }
    }
    op.dest += ",";

    SimulateOperation( (OperationType)type, &op, destReg, (short)std::stoi( op.src ) );

    return op;
}

Operation To_From( char*& opstream )
{
    Operation op;

    char hi = *opstream;
    op.name = toFromOpTable[GetBits( hi, 6, 4 )];
    OperationType opType = (OperationType)GetBits( hi, 6, 4 );

    char d = GetBits( hi, 1, 1 );
    char w = GetBits( hi, 0, 1 );

    // std::cout << std::bitset<8>( hi ) << std::endl;
    // std::cout << "d: " << std::bitset<1>( d ) << std::endl;
    // std::cout << "w: " << std::bitset<1>( w ) << std::endl;

    char lo = *(++opstream);
    char mod = GetBits( lo, 7, 2 );
    char reg = GetBits( lo, 5, 3 );
    char rm = GetBits( lo, 2, 3 );

    // std::cout << std::bitset<8>( lo ) << std::endl;
    // std::cout << "mod: " << std::bitset<2>( mod ) << std::endl;
    // std::cout << "reg: " << std::bitset<3>( reg ) << std::endl;
    // std::cout << "rm: " << std::bitset<3>( rm ) << std::endl;

    int* data = nullptr;
    Register* destReg = d ? AccessRegister( reg, w ) : AccessRegister( rm, w );
    Register* srcReg = d ? AccessRegister( rm, w ) : AccessRegister( reg, w );

    switch( mod )
    {
        case byte_mode:
        {
            *data = *(++opstream);
            op.dest = d
                ? RegisterLookup( reg, w )
                : BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], data );

            op.src = d
                ? BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], data )
                : RegisterLookup( reg, w );

            break;
        }
        case word_mode:
        {
            lo = *(++opstream);
            *data = (char)*(++opstream) << 8 | (unsigned char)lo;

            op.dest = d
                ? RegisterLookup( reg, w )
                : BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], data );

            op.src = d
                ? BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], data )
                : RegisterLookup( reg, w );

            break;
        }
        case reg_mode:
        {
            op.dest = d ? RegisterLookup( reg, w ) : RegisterLookup( rm, w );

            op.src = d ? RegisterLookup( rm, w ) : RegisterLookup( reg, w );

            SimulateOperation( opType, &op, destReg, srcReg->Load() );

            break;
        }
        case mem_mode:
        {
            if( rm == 0b110 )
            {
                *data = (char)*(++opstream) << 8 | (unsigned char)lo;

                op.dest = d
                    ? RegisterLookup( reg, w )
                    : BuildEffectiveRegisterEncoding( RegisterLookup( rm, 1 ), data );

                op.src = d
                    ? BuildEffectiveRegisterEncoding( RegisterLookup( rm, 1 ), data )
                    : RegisterLookup( reg, w );

                destReg = d ? AccessRegister( reg, w ) : AccessRegister( rm, 1 );
                srcReg = d ? AccessRegister( rm, w ) : AccessRegister( reg, 1 );
            }
            else
            {
                op.dest = d
                    ? RegisterLookup( reg, w )
                    : BuildEffectiveRegisterEncoding( effectiveAddressTable[rm] );

                op.src = d
                    ? BuildEffectiveRegisterEncoding( effectiveAddressTable[rm] )
                    : RegisterLookup( reg, w );
            }
            break;
        }
        default:
            // "mode not implemented";
        break;
    }
    op.dest += ",";

    return op;
}

static std::string BuildEffectiveRegisterEncoding( const char* registerName, int* data )
{
    std::string buffer = "[";
    buffer += registerName;

    if( data )
    {
        buffer += " + ";
        buffer += std::to_string( *data );
    }

    buffer += "]";

    return buffer;
}

static void SimulateOperation( OperationType opType, Operation* op, Register* destReg, short srcValue )
{
    switch( opType )
    {
        case mov:
        {
            destReg->Store( srcValue );

            break;
        }
        case cmp:
        {
            short sub = destReg->Load() - srcValue;

            SetFlag( Flags::SF, sub < 0.f );
            SetFlag( Flags::ZF, sub == 0.f );

            // NOTE: cmp uses the sign and zero flags to determine the result of the comparison 
            // greater_than = !SF && !ZF
            // less_than = SF
            // equal = ZF

            op->flags.append( GetFlagState( Flags::SF ) );
            op->flags.append( GetFlagState( Flags::ZF ) );

            break;
        }
        case add:
        {
            short add = destReg->Load() + srcValue;
            destReg->Store( add );

            SetFlag( Flags::SF, add < 0.f );
            SetFlag( Flags::ZF, add == 0.f );

            op->flags.append( GetFlagState( Flags::SF ) );
            op->flags.append( GetFlagState( Flags::ZF ) );

            break;
        }
        case sub:
        {
            short sub = destReg->Load() - srcValue;
            destReg->Store( sub );

            SetFlag( Flags::SF, sub < 0.f );
            SetFlag( Flags::ZF, sub == 0.f );

            op->flags.append( GetFlagState( Flags::SF ) );
            op->flags.append( GetFlagState( Flags::ZF ) );

            break;
        }
    }
}