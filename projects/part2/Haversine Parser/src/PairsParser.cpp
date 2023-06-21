#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <vector>

#include "Coordinates.h"
#include "logging.h"

static const s8* pair_keys_table[]{ "pairs", "x0", "y0", "x1", "y1" };
static u32 pair_keys_index = 0;

struct json_pairs
{
    const s8* char_ptr;

    s8* file_buffer;
    u32 buffer_length;

    // Operator overloading in C++ works for objects, not pointers to objects. 
    // The overloaded operator is invoked when it is applied to an instance of the class or struct, 
    // not to a pointer to that class or struct.
    // const s8* operator++()
    // {
    //     if( char_ptr + 1 >= file_buffer + buffer_length )
    //     {
    //         fprintf( stderr, "Attempted to access json_pair character beyond buffer_length." );
    //         exit( -1 );
    //     }
    //     return ++char_ptr;
    // }

    const s8* increment()
    {
        if( char_ptr + 1 >= file_buffer + buffer_length )
        {
            LOG_ERROR_EXIT( -1, "Attempted to access json_pair character beyond buffer_length." );
        }
        return ++char_ptr;
    }
};

inline bool TryReadJsonFile( const char* file_path, json_pairs& pairs_OUT )
{
    FILE* file = fopen( file_path, "rb" ); // read binary to ensure final null terminator is read
    if( file == nullptr )
    {
        LOG_WARNING( "Failed to open file %s", file_path );
        return false;
    }

    _fseeki64( file, 0, SEEK_END );
    u32 file_size = ftell( file );
    if( file_size == -1 )
    {
        LOG_WARNING( "Failed to determine file size." );
        fclose( file );
        return false;
    }

    if( _fseeki64( file, 0, SEEK_SET ) != 0 )
    {
        LOG_WARNING( "Failed to rewind file pointer to beginning of file (fseek)." );
        fclose( file );
        return false;
    }

    pairs_OUT.file_buffer = (s8*)malloc( file_size + 1 ); // addition byte for null terminator '\0'
    if( pairs_OUT.file_buffer == nullptr )
    {
        LOG_WARNING( "Memory allocation failed." );
        fclose( file );
        return false;
    }

    u32 file_data = fread( pairs_OUT.file_buffer, 1, file_size, file );
    if( file_data != file_size )
    {
        if( ferror( file ) )
            perror( "TryReadJsonFile(): Error occured during file read.\n" );
        else if( feof( file ) )
            LOG_WARNING( "End of file reached before reading all data." );
        else
            LOG_WARNING( "Filesize mismatch when reading file \"%s\".", file_path );

        free( pairs_OUT.file_buffer );
        fclose( file );
        return false;
    }

    pairs_OUT.buffer_length = file_size;
    pairs_OUT.char_ptr = pairs_OUT.file_buffer;

    fclose( file );
    return true;
}

inline bool TryFreePairData( json_pairs& pairs_IN )
{
    if( pairs_IN.file_buffer == nullptr )
    {
        LOG_WARNING( "json_pairs does not have a valid file_buffer." );
        return false;
    }

    free( pairs_IN.file_buffer );
    pairs_IN.file_buffer = nullptr;
    pairs_IN.buffer_length = 0;
    pairs_IN.char_ptr = nullptr;

    return true;
}

static bool IsWhiteSpace( int char_ptr )
{
    u8 mask0 = char_ptr == '\n';
    u8 mask1 = char_ptr == '\r';
    u8 mask2 = char_ptr == ' ';
    u8 mask3 = char_ptr == '\t';

    return mask0 | mask1 | mask2 | mask3;
}

static void SkipWhiteSpace( json_pairs* file_ptr )
{
    const s8* p = file_ptr->char_ptr;
    while( IsWhiteSpace( *p ) )
        ++p;

    file_ptr->char_ptr = p;
}

// Try to consume a required character
static bool ExpectCharacter( json_pairs* file_ptr, const char expected )
{
    SkipWhiteSpace( file_ptr );

    if( *file_ptr->char_ptr != expected )
    {
        LOG_WARNING( "Unexpected character encountered: %c expected: %c", *file_ptr->char_ptr, expected );
        return false;
    }

    file_ptr->increment();
    return true;
}

// Try to consume character only if it matches accepted.
// Consumes whitespace until a character is found.
static bool AcceptCharacter( json_pairs* file_ptr, const char accepted )
{
    SkipWhiteSpace( file_ptr );

    if( *file_ptr->char_ptr != accepted )
        return false;

    file_ptr->increment();
    return true;
}

// Try to consume characters while they match the next pair_key
// Increments pair_keys_index if successfull
static bool ExpectString( json_pairs* file_ptr, const char*& key_OUT )
{
    if( pair_keys_index >= (sizeof( pair_keys_table ) / sizeof( pair_keys_table[0] )) )
    {
        LOG_WARNING( "ExpectString attempted to access element outside of table: %d", pair_keys_index );
        return false;
    }

    SkipWhiteSpace( file_ptr );

    if( ExpectCharacter( file_ptr, '"' ) == false )
    {
        LOG_WARNING( "ExpectString did not encounter a '\"' prefix." );
        return false;
    }

    const s8* expected = pair_keys_table[pair_keys_index];
    while( AcceptCharacter( file_ptr, '"' ) == false )
    {
        if( *file_ptr->char_ptr != *expected )
        {
            LOG_ERROR_EXIT( -1, "ExpectString key did not match character file_ptr input: %s", expected );
        }

        file_ptr->increment();
        ++expected;
    }

    key_OUT = pair_keys_table[pair_keys_index];
    ++pair_keys_index;

    return true;
}

inline bool TryParseJsonPairs( json_pairs* file_ptr, std::vector<Pair>& pairs_OUT )
{
    const s8* key;

    if( ExpectCharacter( file_ptr, '{' ) &&
        ExpectString( file_ptr, key ) &&
        ExpectCharacter( file_ptr, ':' ) &&
        ExpectCharacter( file_ptr, '[' ) )
    {
        while( AcceptCharacter( file_ptr, ']' ) == false )
        {
            AcceptCharacter( file_ptr, ',' );

            if( ExpectCharacter( file_ptr, '{' ) )
            {
                f64 coord[4];
                u32 index = 0;
                while( AcceptCharacter( file_ptr, '}' ) == false )
                {
                    AcceptCharacter( file_ptr, ',' );

                    if( ExpectString( file_ptr, key ) && ExpectCharacter( file_ptr, ':' ) )
                    {
                        s8* endChar;
                        f64 coordinate = strtod( file_ptr->char_ptr, &endChar );
                        file_ptr->char_ptr = endChar;

                        // printf( "Coordinate: %.16f\n", coordinate );
                        coord[index++] = coordinate;
                    }
                    else
                    {
                        return false;
                    }
                }
                // reset to x0 pair_key after reading in a coordinate pair
                pair_keys_index = 1;

                Pair pair = {};
                pair.p0.latitude = coord[0];
                pair.p0.longitude = coord[1];
                pair.p1.latitude = coord[2];
                pair.p1.longitude = coord[3];

                pairs_OUT.push_back( pair );
            }
            else
            {
                return false;
            }
        }
        // NOTE: not checking for closing '}' of json file...

        return true;
    }
    else
    {
        return false;
    }
}
