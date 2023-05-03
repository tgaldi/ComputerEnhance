#include <stdio.h>
#include <stdlib.h>

#include "Types.h"
#include "Decode.h"

u8 Memory[65536] = {};
int Clocks = 0;

void Process( u8* opStream, long streamSize )
{
    u8* beginStream = opStream;
    u8* endStream = opStream + streamSize - 1;
    while( opStream < endStream )
    {
        Operation op = DecodeInstruction( opStream, Memory );
        Clocks += op.clockCount;
        printf( "Operation: %s %s %s | ", op.name, op.dest.c_str(), op.src.c_str() );
        printf( "Clocks: +%d = %d | ", op.clockCount, Clocks );
        printf( "IP 0x% x", op.ip );
        if( op.flags.length() > 0 ) printf( "FLAGS %s", op.flags.c_str() );
        printf( "\n\n" );
        opStream = beginStream + IP.offset;
    }
    printf( "%s: %d\n", A.toString().c_str(), A.Load() );
    printf( "%s: %d\n", B.toString().c_str(), B.Load() );
    printf( "%s: %d\n", C.toString().c_str(), C.Load() );
    printf( "%s: %d\n", D.toString().c_str(), D.Load() );
    printf( "%s: %d\n", SP.toString().c_str(), SP.Load() );
    printf( "%s: %d\n", BP.toString().c_str(), BP.Load() );
    printf( "%s: %d\n", SI.toString().c_str(), SI.Load() );
    printf( "%s: %d\n", DI.toString().c_str(), DI.Load() );

    for( int i = 0; i < 65536 / 2; i += 2 )
    {
        if( Memory[i] != 0 )
        {
            printf( "Memory[%d]: %d\n", i, (s16)Memory[i] );
        }
    }

}

int main( int argc, char** argv )
{
    // ensure cmd arg was provided
    if( argc < 1 )
    {
        fprintf( stderr, "Usage: %s <filename>\n", argv[0] );
        exit( -1 );
    }

    // open the binary file
    FILE* file;
    char* fileName = argv[1];
    auto error = fopen_s( &file, argv[1], "rb" );

    // ensure file is valid
    if( error )
    {
        fprintf( stderr, "ERROR: Cannot open file %s\n", fileName );
        exit( 1 );
    }

    // calculate file size
    fseek( file, 0, SEEK_END );
    long fileSize = ftell( file );
    rewind( file );

    // allocate memory for file
    u8* buffer = (u8*)malloc( sizeof( u8 ) * fileSize );

    // ensure system memory was allocated
    if( buffer == nullptr )
    {
        fprintf( stderr, "ERROR: Cannot allocate space for buffer.\n" );
        fclose( file );
        exit( 2 );
    }

    // read file into buffer
    long result = fread( buffer, 1, fileSize, file );
    fclose( file );

    // ensure file was read
    if( result != fileSize )
    {
        fprintf( stderr, "ERROR: Error reading file.\n" );
        exit( 3 );
    }

    Process( buffer, fileSize );

    exit( 0 );
}

