#include <stdio.h>
#include <stdlib.h>
#include <bitset>
#include <iostream>

void execute( char, char );
const char getbits( char, int, int );
const char* opLookup( const char );
const char* registerLookup( const char, const char );
const char* displacementLookup( const char, const char, const char );

int main( int argc, char** argv )
{
    // ensure cmd arg was provided
    if( argc < 1 )
    {
        fprintf( stderr, "Usage: %s <filename>\n", argv[0] );
        exit( -1 );
    }

    char* fileName = argv[1];
    FILE* file = fopen( argv[1], "rb" );

    // ensure file is valid
    if( file == nullptr )
    {
        fprintf( stderr, "ERROR: Cannot open file %s\n", fileName );
        exit( 1 );
    }

    // calculate file size
    fseek( file, 0, SEEK_END );
    long fileSize = ftell( file );
    rewind( file );

    if( (fileSize % 2) != 0 )
    {
        fprintf( stderr, "ERROR: File does not conform to word size instructions %s\n", fileName );
        exit( 2 );
    }

    // allocate memory for file
    char* buffer = (char*)malloc( sizeof( char ) * fileSize );

    // ensure system memory was allocated
    if( buffer == nullptr )
    {
        fprintf( stderr, "ERROR: Cannot allocate space for buffer.\n" );
        fclose( file );
        exit( 3 );
    }

    // read file into buffer
    long result = fread( buffer, 1, fileSize, file );
    fclose( file );

    // ensure file was read
    if( result != fileSize )
    {
        fprintf( stderr, "ERROR: Error reading file.\n" );
        exit( 4 );
    }

    char* opPtr = buffer;
    char* endPtr = buffer + fileSize;

    // read op codes
    while( opPtr < endPtr )
    {
        char hi = *opPtr++;
        char lo = *opPtr++;
        execute( hi, lo );
    }

    exit( 0 );
}

void execute( char hi, char lo )
{
    // std::cout << std::bitset<8>( hi ) << std::endl;
    // std::cout << std::bitset<8>( lo ) << std::endl;

    const char opCode = getbits( hi, 7, 6 );
    const char d = getbits( hi, 1, 1 );
    const char w = getbits( hi, 0, 1 );
    const char mod = getbits( lo, 7, 2 );
    const char reg = getbits( lo, 5, 3 );
    const char rm = getbits( lo, 2, 3 );

    const char* op = opLookup( opCode );

    const char* reg1 = d
        ? registerLookup( reg, w )
        : displacementLookup( mod, rm, w );

    const char* reg2 = d
        ? displacementLookup( mod, rm, w )
        : registerLookup( reg, w );

    printf( "%s %s, %s\n", op, reg1, reg2 );
}

const char getbits( char x, int p, int n )
{
    return (x >> (p + 1 - n)) & ~(~0 << n);
}

enum opCodes
{
    no_op,
    mov,
    op_count
};
static const char opCodeTable[op_count] = { 0b0, 0b00100010 };
static const char* opNameTable[op_count] = { "no_op\0", "mov\0" };
const char* opLookup( const char op )
{
    int tableLength = sizeof( opCodeTable ) / sizeof( char );
    if( tableLength != sizeof( opNameTable ) / sizeof( char* ) )
        return "invalid op code tables\0";

    int opCode = no_op;
    const char* opPtr = opCodeTable;
    while( opPtr < opCodeTable + tableLength )
    {
        if( *opPtr++ == op )
            break;

        ++opCode;
    }

    return opNameTable[opCode];
}

enum registers
{
    AL,
    CL,
    DL,
    BL,
    AH,
    CH,
    DH,
    BH,
    reg_count
};
static const char* regNameTable[2][reg_count] = { { "al\0", "cl\0", "dl\0", "bl\0", "ah\0", "ch\0", "dh\0", "bh\0" },
                                                { "ax\0", "cx\0", "dx\0", "bx\0", "sp\0", "bp\0", "si\0", "di\0" } };
const char* registerLookup( const char reg, const char w )
{
    if( reg >= reg_count )
        return "invalid register\0";

    return regNameTable[w][reg];
}

enum mode
{
    mem_mode,
    byte_mode,
    word_mode,
    reg_mode,
    mode_count
};
const char* displacementLookup( const char mod, const char rm, const char w )
{
    switch( mod )
    {
        case reg_mode:
            return registerLookup( rm, w );
        default:
            return "mode not implemented\0";
    }
}
