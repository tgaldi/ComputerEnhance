#include "Decode.h"

#include <string>
#include <stdlib.h>
#include <bitset>
#include <iostream>

#include "Tables.h"
#include "Registers.h"

static void SimulateOperation( OperationType opType, Operation* op, Register* destReg, s16 srcValue );
static std::string BuildEffectiveRegisterEncoding( const char* registerName, s16* data = nullptr );
static int GetEAClocks( u8 rm );

static u8* MemoryPtr = nullptr;

Operation DecodeInstruction( u8*& opstream, u8* memory )
{
    MemoryPtr = memory;

    u8 hi = *opstream;

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
                operation = Mov_Immediate_Register( opstream );
            }
            else if( GetBits( hi, 2, 1 ) )
            {
                operation = Mov_Immediate_Register_Memory( opstream );
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


Operation Loop( u8*& opstream )
{
    Operation op;

    u8 hi = *opstream;
    op.name = loopTable[GetBits( hi, 3, 4 )];

    s8 inc8 = *(++opstream);
    op.dest = std::to_string( inc8 );

    IP.offset += 2;
    // std::cout << std::bitset<8>( hi ) << std::endl;
    // std::cout << std::bitset<8>( inc8 ) << std::endl;

    op.ip = IP.offset;
    return op;

}

Operation Jump( u8*& opstream )
{
    Operation op;

    u8 hi = *opstream;
    op.name = jumpTable[GetBits( hi, 3, 4 )];

    s8 inc8 = *(++opstream);
    op.dest = std::to_string( inc8 );

    IP.offset += 2;
    if( !GetFlagState( Flags::ZF ) )
    {
        IP.offset += inc8;
    }

    // std::cout << std::bitset<8>( hi ) << std::endl;
    // std::cout << std::bitset<8>( inc8 ) << std::endl;

    op.ip = IP.offset;
    return op;
}

Operation Immediate_Accumulator( u8*& opstream )
{
    Operation op;

    u8 hi = *opstream;
    op.name = toFromOpTable[GetBits( hi, 6, 4 )];

    u8 w = GetBits( hi, 0, 1 );
    u8 reg = 0b0;

    // std::cout << std::bitset<8>( hi ) << std::endl;

    op.dest = RegisterLookup( reg, w );

    s8 lo = *(++opstream);
    // std::cout << std::bitset<8>( lo ) << std::endl;

    IP.offset += 2;
    if( w ) IP.offset += 1;

    op.src = w
        ? std::to_string( ((s8) * (++opstream)) << 8 | (u8)lo )
        : std::to_string( lo );

    op.dest += ",";
    op.ip = IP.offset;

    return op;
}

Operation Mov_Immediate_Register( u8*& opstream )
{
    Operation op;
    op.name = "mov";

    u8 hi = *opstream;
    // std::cout << std::bitset<8>( *opstream ) << std::endl;

    u8 w = GetBits( hi, 3, 1 );
    u8 reg = GetBits( hi, 2, 3 );

    op.dest = RegisterLookup( reg, w );

    Register* dest = AccessRegister( reg, w );

    s8 lo = *(++opstream);
    // std::cout << std::bitset<8>( *opstream ) << std::endl;

    IP.offset += 2;
    if( w ) IP.offset += 1;

    s16 data = w
        ? ((s8) * (++opstream)) << 8 | (u8)lo
        : lo;

    op.src = std::to_string( data );

    // op.src = w
    //     ? std::to_string( (char)*(++opstream) << 8 | (unsigned char)lo )
    //     : std::to_string( lo );

    dest->Store( data );

    op.dest += ",";
    op.ip = IP.offset;
    op.clockCount = 4;

    return op;
}



Operation Mov_Immediate_Register_Memory( u8*& opstream )
{
    Operation op;
    op.name = "mov";

    u8 hi = *opstream;
    u8 w = GetBits( hi, 0, 1 );

    // std::cout << std::bitset<8>( hi ) << std::endl;
    // std::cout << "w: " << std::bitset<1>( w ) << std::endl;

    u8 lo = *(++opstream);
    u8 mod = GetBits( lo, 7, 2 );
    u8 rm = GetBits( lo, 2, 3 );

    // std::cout << std::bitset<8>( lo ) << std::endl;
    // std::cout << "mod: " << std::bitset<2>( mod ) << std::endl;
    // std::cout << "rm: " << std::bitset<3>( rm ) << std::endl;

    IP.offset += 2;

    op.clockCount = 4;

    Register* destReg = AccessRegister( rm, w );

    switch( mod )
    {
        case mem_mode:
            {
                if( rm == 0b110 ) // direct adddress
                {
                    s8 dispLo = *(++opstream);
                    // std::cout << std::bitset<8>( dispLo ) << std::endl;

                    s16 disp = dispLo;
                    if( w )
                    {
                        s8 dispHi = *(++opstream);
                        // std::cout << std::bitset<8>( dispHi ) << std::endl;

                        disp = dispHi << 8 | (u8)dispLo;
                        op.dest.append( "word [+" + std::to_string( disp ) + "]" );

                        IP.offset += 1 + w;
                    }
                    else
                    {
                        op.dest.append( "byte [+" + std::to_string( dispLo ) + "]" );
                    }

                    // std::cout << std::bitset<8>( disp ) << std::endl;

                    s8 data_lo = *(++opstream);
                    s8 data_hi = 0;
                    // std::cout << std::bitset<8>( *opstream ) << std::endl;

                    if( w )
                    {
                        data_hi = *(++opstream);
                        // std::cout << std::bitset<8>( *opstream ) << std::endl;
                    }
                    IP.offset += 1 + w;

                    s16 data = ((s8)data_hi) << 8 | (u8)data_lo;
                    op.src = std::to_string( data );

                    MemoryPtr[disp] = data_lo;
                    if( w ) MemoryPtr[disp + 1] = data_hi;
                }
                else
                {
                    op.dest = BuildEffectiveRegisterEncoding( effectiveAddressTable[rm] );
                }

                break;
            }
        case byte_mode:
            {
                s16 disp = *(++opstream);
                op.dest = BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], &disp );
                s16 effectiveLoad = LoadEffectiveAddress( rm, w, disp );

                u8 data = *(++opstream);
                IP.offset += 2 + w;
                op.src = std::to_string( data );

                u8* ptr = (MemoryPtr);
                ptr += effectiveLoad;
                *ptr = data;

                break;
            }

        case word_mode:
            {
                s8 lo_bit = *(++opstream);
                s16 data = ((s8) * (++opstream)) << 8 | (u8)lo_bit;

                op.dest = BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], &data );

                IP.offset += 2;

                // u8* ptr = (MemoryPtr);
                // ptr += destReg->Load() + disp;
                // *ptr = data & 0xFF; // stores the least significant byte
                // *(++ptr) = (data >> 8) & 0xFF; // stores the most significant byte


                SimulateOperation( OperationType::mov, &op, destReg, data );
                break;
            }

        case reg_mode:
            {
                destReg = AccessRegister( rm, w );
                op.dest = RegisterLookup( rm, w );
                op.clockCount = 2;
                break;
            }
        default:
            // "mode not implemented";
            break;
    }
    op.dest += ",";
    op.ip = IP.offset;

    return op;
}

Operation Immediate( u8*& opstream )
{
    u8 hi = *opstream;
    u8 s = GetBits( hi, 1, 1 );
    u8 w = GetBits( hi, 0, 1 );

    // std::cout << std::bitset<8>( *opstream ) << std::endl;
    // std::cout << "s: " << std::bitset<1>( s ) << std::endl;
    // std::cout << "w: " << std::bitset<1>( w ) << std::endl;

    u8 lo = *(++opstream);
    u8 mod = GetBits( lo, 7, 2 );
    u8 type = GetBits( lo, 5, 3 );
    u8 rm = GetBits( lo, 2, 3 );

    // std::cout << std::bitset<8>( lo ) << std::endl;
    // std::cout << "mod: " << std::bitset<2>( mod ) << std::endl;
    // std::cout << "type: " << std::bitset<3>( type ) << std::endl;
    // std::cout << "rm: " << std::bitset<3>( rm ) << std::endl;

    IP.offset += 2;
    Operation op;
    op.name = toFromOpTable[type];

    // NOTE: If not in register mode (mod 11) then the register is accessed wide, hence the 1
    Register* destReg = AccessRegister( rm, 1 );

    s8 lo_bit;
    switch( mod )
    {
        case mem_mode:
            {
                if( rm == 0b110 ) // direct adddress
                {
                    lo_bit = *(++opstream);
                    s16 data = ((s8) * (++opstream)) << 8 | (u8)lo_bit;
                    op.dest = std::string( "[" ) + std::to_string( data ) + std::string( "]" );
                    // op.dest = BuildEffectiveRegisterEncoding( RegisterLookup( rm, w ), &data );

                    IP.offset += 2;
                }
                else
                {
                    op.dest = BuildEffectiveRegisterEncoding( effectiveAddressTable[rm] );
                }
                break;
            }

        case byte_mode:
            {
                s16 data = *(++opstream);
                op.dest = BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], &data );

                IP.offset += 1;
                break;
            }

        case word_mode:
            {
                lo_bit = *(++opstream);
                s16 data = ((s8) * (++opstream)) << 8 | (u8)lo_bit;

                op.dest = BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], &data );

                IP.offset += 2;

                break;
            }

        case reg_mode:
            {
                destReg = AccessRegister( rm, w );
                op.dest = RegisterLookup( rm, w );

                op.clockCount = 4;
                break;
            }
    }

    lo_bit = *(++opstream);
    IP.offset += 1;

    u8 sw = (s << 1) | w;
    switch( sw )
    {
        case 0: // unsigned byte
            {
                op.src = std::to_string( lo_bit );
                break;
            }
        case 1: // unsigned word
            {
                op.src = std::to_string( ((s8) * (++opstream)) << 8 | (u8)lo_bit );
                IP.offset += 1;
                break;
            }
        case 2: // signed byte
            {
                op.src = std::to_string( lo_bit );
                break;
            }
        case 3: // signed word
            {
                op.src = std::to_string( (s16)lo_bit );
                break;
            }
    }
    op.dest += ",";
    op.ip = IP.offset;

    SimulateOperation( (OperationType)type, &op, destReg, (s16)std::stoi( op.src ) );

    return op;
}

Operation To_From( u8*& opstream )
{
    u8 hi = *opstream;
    u8 type = GetBits( hi, 6, 4 );

    // NOTE: Mov doesn't use the D bit, but unlike the arithmatic ops it has a 1 in the hi 6 bit 
    u8 is_mov = GetBits( hi, 6, 1 );
    u8 d = GetBits( hi, 1, 1 ) & ~is_mov;
    u8 w = GetBits( hi, 0, 1 );

    // std::cout << std::bitset<8>( hi ) << std::endl;
    // std::cout << "d: " << std::bitset<1>( d ) << std::endl;
    // std::cout << "w: " << std::bitset<1>( w ) << std::endl;

    u8 lo = *(++opstream);
    u8 mod = GetBits( lo, 7, 2 );
    u8 reg = GetBits( lo, 5, 3 );
    u8 rm = GetBits( lo, 2, 3 );

    // std::cout << std::bitset<8>( lo ) << std::endl;
    // std::cout << "mod: " << std::bitset<2>( mod ) << std::endl;
    // std::cout << "reg: " << std::bitset<3>( reg ) << std::endl;
    // std::cout << "rm: " << std::bitset<3>( rm ) << std::endl;

    IP.offset += 2;

    Operation op;
    op.name = toFromOpTable[type];

    Register* destReg = AccessRegister( reg, w );
    Register* srcReg = AccessRegister( reg, w );

    switch( mod )
    {
        case mem_mode:
            {
                if( rm == 0b110 ) // direct adddress
                {
                    s8 dispLo = *(++opstream);
                    // std::cout << std::bitset<8>( dispLo ) << std::endl;

                    s16 disp = dispLo;
                    if( w )
                    {
                        s8 dispHi = *(++opstream);
                        // std::cout << std::bitset<8>( dispHi ) << std::endl;

                        disp = dispHi << 8 | (u8)dispLo;
                        op.src.append( "word [+" + std::to_string( disp ) + "]" );

                        IP.offset += 1 + w;
                    }
                    else
                    {
                        op.src.append( "word [+" + std::to_string( disp ) + "]" );
                    }

                    // std::cout << std::bitset<8>( disp ) << std::endl;

                    op.dest = d
                        ? RegisterLookup( reg, w )
                        : BuildEffectiveRegisterEncoding( effectiveAddressTable[rm] );

                    destReg->Store( MemoryPtr[disp] );

                    // mov disp->reg : reg->disp
                    op.clockCount = (d) ? 8 + 6 : 9 + 6;
                }
                else
                {
                    op.dest = d
                        ? RegisterLookup( reg, w )
                        : BuildEffectiveRegisterEncoding( effectiveAddressTable[rm] );

                    op.src = d
                        ? BuildEffectiveRegisterEncoding( effectiveAddressTable[rm] )
                        : RegisterLookup( reg, w );


                    // mov mem->reg : reg->memory
                    op.clockCount = (d) ? 8 + GetEAClocks( rm ) : 9 + GetEAClocks( rm );
                }

                break;
            }
        case byte_mode:
            {
                s16 data = *(++opstream);

                op.dest = d
                    ? RegisterLookup( reg, w )
                    : BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], &data );

                //TODO: Is this still needed?
                if( is_mov )
                {
                    data = *(++opstream);
                    IP.offset += 1 + w;
                    op.src = std::to_string( data );

                    op.clockCount = (d) ? 8 + GetEAClocks( rm ) + (w * 4) : 4;
                }
                else
                {
                    op.src = d
                        ? BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], &data )
                        : RegisterLookup( reg, w );

                    std::cout << "ByteMode" << std::endl;
                    op.clockCount = (d) ? 8 + GetEAClocks( rm ) : 9 + GetEAClocks( rm );
                }

                IP.offset += 1;
                break;
            }
        case word_mode:
            {
                s8 lo_bit = *(++opstream);
                s16 data = ((s8) * (++opstream)) << 8 | (u8)lo_bit;

                op.dest = d
                    ? RegisterLookup( reg, w )
                    : BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], &data );

                op.src = d
                    ? BuildEffectiveRegisterEncoding( effectiveAddressTable[rm], &data )
                    : RegisterLookup( reg, w );

                if( type == 0b000 ) // add
                {
                    op.clockCount = (d) ? 9 + GetEAClocks( rm ) + (w * 4) : 16 + GetEAClocks( rm ) + (w * 4);
                }
                else
                {
                    op.clockCount = (d) ? 8 + GetEAClocks( rm ) + (w * 4) : 9 + GetEAClocks( rm ) + (w * 4);
                }

                IP.offset += 2;
                break;
            }
        case reg_mode:
            {
                destReg = d ? AccessRegister( reg, w ) : AccessRegister( rm, w );
                srcReg = d ? AccessRegister( rm, w ) : AccessRegister( reg, w );

                op.dest = d ? RegisterLookup( reg, w ) : RegisterLookup( rm, w );

                op.src = d ? RegisterLookup( rm, w ) : RegisterLookup( reg, w );

    //          // just doing mov and add for now
                op.clockCount = type ? 2 : 3;

                break;
            }
        default:
            // "mode not implemented";
            break;
    }
    op.dest += ",";
    op.ip = IP.offset;

    SimulateOperation( (OperationType)type, &op, destReg, srcReg->Load() );
    return op;
}

static std::string BuildEffectiveRegisterEncoding( const char* registerName, s16* data )
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

static void SimulateOperation( OperationType opType, Operation* op, Register* destReg, s16 srcValue )
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
                s16 sub = destReg->Load() - srcValue;

                SetFlag( Flags::SF, sub < 0 );
                SetFlag( Flags::ZF, sub == 0 );

                // NOTE: cmp uses the sign and zero flags to determine the result of the comparison 
                // greater_than = !SF && !ZF
                // less_than = SF
                // equal = ZF

                op->flags.append( FlagStateToString( Flags::SF ) );
                op->flags.append( FlagStateToString( Flags::ZF ) );

                break;
            }
        case add:
            {
                s16 add = destReg->Load() + srcValue;
                destReg->Store( add );

                SetFlag( Flags::SF, add < 0 );
                SetFlag( Flags::ZF, add == 0 );

                op->flags.append( FlagStateToString( Flags::SF ) );
                op->flags.append( FlagStateToString( Flags::ZF ) );

                break;
            }
        case sub:
            {
                s16 sub = destReg->Load() - srcValue;
                destReg->Store( sub );

                SetFlag( Flags::SF, sub < 0 );
                SetFlag( Flags::ZF, sub == 0 );

                op->flags.append( FlagStateToString( Flags::SF ) );
                op->flags.append( FlagStateToString( Flags::ZF ) );

                break;
            }
    }
}

static int GetEAClocks( u8 rm )
{
    switch( rm )
    {
        case 0b000: // [bx + si]
        case 0b011: // [bp + di]
            {
                return 7;
            }

        case 0b001: // [bx + di]
        case 0b010: // [bp + si]
            {
                return 8;
            }

        case 0b100: // [si]
        case 0b101: // [di]
        case 0b110: // BX,
        case 0b111: // [bx]
            {
                return 5;
            }
    }
    return 0;
}