#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "FMath.h"
#include "Coordinates.h"
#include "haversine_formula.cpp"

#define EARTH_RADIUS 6372.8f
#define CLUSTERS 16
#define MAX_PAIR_COUNT (1ULL << 34)

static void ExecuteUniformMethod( FILE* json_file, u64 pair_count, FILE* haversine_file, f64& haversine_summation_IN_OUT );
static void ExecuteClusterMethod( FILE* json_file, u64 pair_count, FILE* haversine_file, f64& haversine_summation_IN_OUT );

static FILE* Open( u64 pair_count, const s8* label, const s8* extension )
{
    s8 buffer[256];
    sprintf( buffer, "data_%llu_%s.%s", pair_count, label, extension );

    FILE* Result = fopen( buffer, "wb" );
    if( !Result )
    {
        fprintf( stderr, "Unable to open \"%s\" for writing.\n", buffer );
    }

    return Result;
}

u32 main( u32 arg_count, s8** argument )
{
    if( arg_count != 4 )
    {
        fprintf( stderr, "Invalid number or arguments: %d\n", arg_count );
        fprintf( stderr, "Usage: haversine_generator_release.exe [uniform/cluster] [random seed] [number of coordinate pairs to generate]\n" );
        exit( -1 );
    }

    s8* method = argument[1];
    while( *method != '\0' )
    {
        *method = (s8)tolower( *method );
        ++method;
    }
    method = argument[1];

    u64 seed = atoll( argument[2] );
    if( seed > INT_MAX )
    {
        fprintf( stderr, "WARNING: Seed size exceeds 32 bit integer.\n\n" );
    }
    srand( (u32)seed );

    u64 pair_count = atoll( argument[3] );
    if( pair_count > MAX_PAIR_COUNT )
    {
        fprintf( stderr, "To avoid accidentally generating massive files, number of pairs must be less than %llu.\n", MAX_PAIR_COUNT );
        exit( 1 );
    }

    FILE* json_pairs = Open( pair_count, "coordinate_pairs", "json" );
    FILE* haversine_answer = Open( pair_count, "haversine_answer", "f64" );
    if( json_pairs && haversine_answer )
    {
        f64 haversine_summation = 0.0;

        if( strcmp( method, "cluster" ) == 0 )
        {
            ExecuteClusterMethod( json_pairs, pair_count, haversine_answer, haversine_summation );
        }
        else
        {
            if( strcmp( method, "uniform" ) != 0 )
            {
                fprintf( stderr, "WARNING: Unrecognized method name. Using 'uniform'.\n" );
            }

            ExecuteUniformMethod( json_pairs, pair_count, haversine_answer, haversine_summation );
        }

        fwrite( &haversine_summation, sizeof( haversine_summation ), 1, haversine_answer );

        fclose( json_pairs );
        fclose( haversine_answer );

        fprintf( stdout, "Method: %s\n", method );
        fprintf( stdout, "Random seed: %llu\n", seed );
        fprintf( stdout, "Pair count: %llu\n", pair_count );
        fprintf( stdout, "Expected sum: %.16f\n", haversine_summation );
    }

    return 0;
}

static void ExecuteUniformMethod( FILE* json_file, u64 pair_count, FILE* haversine_file, f64& haversine_summation_IN_OUT )
{
    f64 sum_coeff = 1.0 / (f64)pair_count;

    fprintf( json_file, "{\"pairs\":[" );
    for( u64 i = 0; i < pair_count; ++i )
    {
        Pair pair;

        pair.p0.latitude = (f64)RANDOM_32S * PI * DEG;
        pair.p0.longitude = (f64)RANDOM_32S * HALF_PI * DEG;

        pair.p1.latitude = (f64)RANDOM_32S * PI * DEG;
        pair.p1.longitude = (f64)RANDOM_32S * HALF_PI * DEG;

        WritePairToJson( json_file, pair, i == pair_count - 1 );

        f64 haversine_distance = ReferenceHaversine( pair.p0.latitude, pair.p0.longitude, pair.p1.latitude, pair.p1.longitude, EARTH_RADIUS );

        fwrite( &haversine_distance, sizeof( haversine_distance ), 1, haversine_file );

        haversine_summation_IN_OUT += sum_coeff * haversine_distance;
    }
    fprintf( json_file, "\n]}" );
}

static void ExecuteClusterMethod( FILE* json_file, u64 pair_count, FILE* haversine_file, f64& haversine_summation_IN_OUT )
{
    f64 sum_coeff = 1.0 / (f64)pair_count;

    Point3D cluster_center[CLUSTERS];
    for( u32 i = 0; i < CLUSTERS; ++i )
    {
        cluster_center[i] = RandPointOnSphere( EARTH_RADIUS );
    }

    f32 cluster_radius = EARTH_RADIUS / CLUSTERS;
    u64 pairs_per_cluster = pair_count / CLUSTERS;
    u32 excess = pair_count % CLUSTERS;

    fprintf( json_file, "{\"pairs\":[" );
    for( u32 i = 0; i < CLUSTERS; ++i )
    {
        for( u64 k = 0; k < pairs_per_cluster; ++k )
        {
            Point2D cluster_point = RandPointInCicle( cluster_radius );
            cluster_point.x += cluster_center[i].x;
            cluster_point.y += cluster_center[i].y;
            f32 elevation = RANDOM_32S * HALF_PI;

            Point3D p0 = ProjectPointOnSphere( cluster_point, elevation, EARTH_RADIUS );

            cluster_point = RandPointInCicle( cluster_radius );
            cluster_point.x += cluster_center[i].x;
            cluster_point.y += cluster_center[i].y;
            elevation = RANDOM_32S * HALF_PI;

            Point3D p1 = ProjectPointOnSphere( cluster_point, elevation, EARTH_RADIUS );

            Pair pair;
            pair.p0 = CartesianToLatLong( p0 );
            pair.p1 = CartesianToLatLong( p1 );

            if( i == CLUSTERS - 1 && excess == 0 )
            {
                WritePairToJson( json_file, pair, k == pairs_per_cluster - 1 );
            }
            else
            {
                WritePairToJson( json_file, pair );
            }

            f64 haversine_distance = ReferenceHaversine( pair.p0.latitude, pair.p0.longitude, pair.p1.latitude, pair.p1.longitude, EARTH_RADIUS );

            fwrite( &haversine_distance, sizeof( haversine_distance ), 1, haversine_file );

            haversine_summation_IN_OUT += sum_coeff * haversine_distance;
        }
    }

    for( u32 i = 0; i < excess; ++i )
    {
        Point2D cluster_point = RandPointInCicle( cluster_radius );
        cluster_point.x += cluster_center[i].x;
        cluster_point.y += cluster_center[i].y;
        f32 elevation = RANDOM_32S * HALF_PI;

        Point3D p0 = ProjectPointOnSphere( cluster_point, elevation, EARTH_RADIUS );

        cluster_point = RandPointInCicle( cluster_radius );
        cluster_point.x += cluster_center[i].x;
        cluster_point.y += cluster_center[i].y;
        elevation = RANDOM_32S * HALF_PI;

        Point3D p1 = ProjectPointOnSphere( cluster_point, elevation, EARTH_RADIUS );

        Pair pair;
        pair.p0 = CartesianToLatLong( p0 );
        pair.p1 = CartesianToLatLong( p1 );

        WritePairToJson( json_file, pair, i == excess - 1 );

        f64 haversine_distance = ReferenceHaversine( pair.p0.latitude, pair.p0.longitude, pair.p1.latitude, pair.p1.longitude, EARTH_RADIUS );

        fwrite( &haversine_distance, sizeof( haversine_distance ), 1, haversine_file );

        haversine_summation_IN_OUT += sum_coeff * haversine_distance;
    }

    fprintf( json_file, "\n]}" );
}