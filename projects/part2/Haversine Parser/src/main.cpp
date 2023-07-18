int main( int arg_count, char** argument )
{
    BeginProfile();

    if( arg_count < 2 )
    {
        LOG_ERROR_EXIT( 1, "Usage: <filename>" );
    }

    json_pairs pairs_data = {};
    std::vector<Pair> pairs = {};

    if( TryReadJsonFile( argument[1], pairs_data ) )
    {
        if( TryParseJsonPairs( &pairs_data, pairs ) )
        {
            TimeBlock( "HaversineSummation" );

            f64 haversine_summation = 0.0;
            f64 sum_coeff = 1.0 / (f64)pairs.size();
            for( auto& p : pairs )
            {
                // PRINT( "{X0:%.16f, Y0:%.16f, X1:%.16f, Y1:%.16f}", p.p0.latitude, p.p0.longitude, p.p1.latitude, p.p1.longitude );

                haversine_summation += ReferenceHaversine( p.p0.latitude, p.p0.longitude, p.p1.latitude, p.p1.longitude, EARTH_RADIUS ) * sum_coeff;
            }

            PRINT( "Haversine sum: %.16f", haversine_summation );
        }
        TryFreePairData( pairs_data );
    }

    if( arg_count == 3 )
    {
        TimeBlock( "ReadAnswerFile" );

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

    EndProfileAndPrint();

    return 0;
}