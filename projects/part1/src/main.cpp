#include <stdio.h>
#include <stdlib.h>

#include "Decode.h"

void Process( char* opStream, long streamSize )
{
    char* endStream = opStream + streamSize - 1;
    while( opStream < endStream )
    {
        Operation op = DecodeInstruction( opStream );
        printf( "Operation: %s %s %s\n", op.name, op.dest.c_str(), op.src.c_str() );
        printf( "FLAGS %s\n\n", op.flags.c_str() );
        ++opStream;
    }
    // printf( "%s: %d\n", A.toString().c_str(), A.Load() );
    // printf( "%s: %d\n", B.toString().c_str(), B.Load() );
    // printf( "%s: %d\n", C.toString().c_str(), C.Load() );
    // printf( "%s: %d\n", D.toString().c_str(), D.Load() );
    // printf( "%s: %d\n", SP.toString().c_str(), SP.Load() );
    // printf( "%s: %d\n", BP.toString().c_str(), BP.Load() );
    // printf( "%s: %d\n", SI.toString().c_str(), SI.Load() );
    // printf( "%s: %d\n", DI.toString().c_str(), DI.Load() );
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
    char* buffer = (char*)malloc( sizeof( char ) * fileSize );

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

