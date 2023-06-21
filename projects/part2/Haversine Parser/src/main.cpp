
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <vector>

#include "types.h"
#include "PairsParser.cpp"
#include "haversine_formula.cpp"

#include "Platform_Metrics.cpp"

int main( int arg_count, char** argument )
{
    u64 CPUFreq = EstimeCPUFrequency( 100 );
    u64 mainStart = ReadCPUTimer();

    if( arg_count < 2 )
    {
        LOG_ERROR_EXIT( 1, "Usage: <filename>" );
    }

    json_pairs pairs_data = {};
    std::vector<Pair> pairs = {};

    u64 readStart = ReadCPUTimer();
    u64 readEnd = 0;
    u64 parseStart = 0;
    u64 parseEnd = 0;
    u64 haverSumStart = 0;
    u64 haverSumEnd = 0;
    u64 parseMemFreeStart = 0;
    u64 parseMemFreeEnd = 0;
    if( TryReadJsonFile( argument[1], pairs_data ) )
    {
        readEnd = ReadCPUTimer();
        parseStart = ReadCPUTimer();
        if( TryParseJsonPairs( &pairs_data, pairs ) )
        {
            parseEnd = ReadCPUTimer();

            haverSumStart = ReadCPUTimer();
            f64 haversine_summation = 0.0;
            f64 sum_coeff = 1.0 / (f64)pairs.size();
            for( auto& p : pairs )
            {
                // PRINT( "{X0:%.16f, Y0:%.16f, X1:%.16f, Y1:%.16f}", p.p0.latitude, p.p0.longitude, p.p1.latitude, p.p1.longitude );

                haversine_summation += ReferenceHaversine( p.p0.latitude, p.p0.longitude, p.p1.latitude, p.p1.longitude, EARTH_RADIUS ) * sum_coeff;
            }
            haverSumEnd = ReadCPUTimer();

            PRINT( "Haversine sum: %.16f", haversine_summation );
        }
        parseMemFreeStart = ReadCPUTimer();
        TryFreePairData( pairs_data );
        parseMemFreeEnd = ReadCPUTimer();
    }

    u64 answerStart = ReadCPUTimer();
    if( arg_count == 3 )
    {
        FILE* answer_file = fopen( argument[2], "rb" );
        if( answer_file )
        {
            struct __stat64 stat;
            _stat64( argument[2], &stat );

            u8* data = (u8*)malloc( stat.st_size );
            if( data )
            {
                if( fread( data, stat.st_size, 1, answer_file ) != 1 )
                {
                    LOG_WARNING( "Unable to read \"%s\"", argument[2] );
                }
                else
                {
                    // total sum was written as last sum in answer file
                    u64 answer_count = (stat.st_size - sizeof( f64 )) / sizeof( f64 );
                    f64* answer_values = (f64*)data;
                    PRINT( "Reference sum: %.16f", answer_values[answer_count] );
                }
                free( data );
            }
            fclose( answer_file );
        }
        else
        {
            LOG_WARNING( "Unable to open \"%s\"", argument[2] );
        }
    }
    u64 answerEnd = ReadCPUTimer();


    u64 mainEnd = ReadCPUTimer();
    u64 runtime = mainEnd - mainStart;
    u64 read = readEnd - readStart;
    u64 parse = parseEnd - parseStart;
    u64 sum = haverSumEnd - haverSumStart;
    u64 memFree = parseMemFreeEnd - parseMemFreeStart;
    u64 answer = answerEnd - answerStart;

    WRITE_LINE( "Total runtime: %.4fms (CPU freq %llu)", (f64)runtime * .0001, CPUFreq );
    PRINT( "         Read: %llu (%.2f%%)", read, ((f64)read / (f64)runtime) * 100.0 );
    PRINT( "        Parse: %llu (%.2f%%)", parse, ((f64)parse / (f64)runtime) * 100.0 );
    PRINT( "          Sum: %llu (%.2f%%)", sum, ((f64)sum / (f64)runtime) * 100.0 );
    PRINT( "     Mem Free: %llu (%.2f%%)", memFree, ((f64)memFree / (f64)runtime) * 100.0 );
    PRINT( "       Answer: %llu (%.2f%%)", answer, ((f64)answer / (f64)runtime) * 100.0 );
    return 0;
}
