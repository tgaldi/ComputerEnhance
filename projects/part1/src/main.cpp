#include <stdio.h>
#include <stdlib.h>

#include "Instructions.h"

void Process( char* opStream, long streamSize )
{
    char* endStream = opStream + streamSize - 1;
    while( opStream < endStream )
    {
        Instruction instruction = GetInstruction( *opStream );
        Operation op = instruction.Execute( opStream );

        printf( "%s %s, %s\n", op.opName, op.dest.c_str(), op.src.c_str() );

        ++opStream;
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

