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

static f64 Haversine_Sum = 0.0;

static void ExecuteUniformMethod( FILE* file, u64 pair_count )
{
    fprintf( file, "{\"pairs\":[" );
    for( u64 i = 0; i < pair_count; ++i )
    {
        Pair pair;

        pair.p0.longitude = (f64)RANDOM_32S * PI * DEG;
        pair.p0.latitude = (f64)RANDOM_32S * HALF_PI * DEG;

        pair.p1.longitude = (f64)RANDOM_32S * PI * DEG;
        pair.p1.latitude = (f64)RANDOM_32S * HALF_PI * DEG;

        Haversine_Sum += ReferenceHaversine( pair.p0.longitude, pair.p0.latitude, pair.p1.longitude, pair.p1.latitude, EARTH_RADIUS );

        WritePairToJson( file, pair, i == pair_count - 1 );
    }
    fprintf( file, "\n]}" );
}

static void ExecuteClusterMethod( FILE* file, u64 pair_count )
{
    Point3D cluster_center[CLUSTERS];
    for( u32 i = 0; i < CLUSTERS; ++i )
    {
        cluster_center[i] = RandPointOnSphere( EARTH_RADIUS );
    }

    f32 cluster_radius = EARTH_RADIUS / CLUSTERS;
    u64 pairs_per_cluster = pair_count / CLUSTERS;
    u32 excess = pair_count % CLUSTERS;

    fprintf( file, "{\"pairs\":[" );
    for( u32 i = 0; i < CLUSTERS; ++i )
    {
        for( u32 k = 0; k < pairs_per_cluster; ++k )
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

            WritePairToJson( file, pair );

            Haversine_Sum += ReferenceHaversine( pair.p0.longitude, pair.p0.latitude, pair.p1.longitude, pair.p1.latitude, EARTH_RADIUS );
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

        WritePairToJson( file, pair, i == excess - 1 );

        Haversine_Sum += ReferenceHaversine( pair.p0.longitude, pair.p0.latitude, pair.p1.longitude, pair.p1.latitude, EARTH_RADIUS );
    }

    fprintf( file, "\n]}" );
}


int main( int argc, char** argv )
{
    if( argc != 4 )
    {
        fprintf( stderr, "Invalid number or arguments: %d\n", argc );
        fprintf( stderr, "Usage: haversine_generator_release.exe [uniform/cluster] [random seed] [number of coordinate pairs to generate]\n" );
        exit( -1 );
    }

    FILE* file = fopen( "point_pairs.json", "w" );
    if( !file )
    {
        fprintf( stderr, "Could not write file!" );
        exit( 1 );
    }

    char* method = argv[1];
    while( *method != '\0' )
    {
        *method = (char)tolower( *method );
        ++method;
    }
    method = argv[1];

    u64 seed = strtoll( argv[2], nullptr, 10 );
    if( seed > INT_MAX )
    {
        fprintf( stderr, "Warning: Seed size exceeds 32 bit integer.\n\n" );
    }
    srand( (u32)seed );

    u64 pair_count = strtoll( argv[3], nullptr, 10 );

    if( strcmp( method, "uniform" ) == 0 )
        ExecuteUniformMethod( file, pair_count );
    else
        ExecuteClusterMethod( file, pair_count );

    fclose( file );

    fprintf( stdout, "Method: %s\n", method );
    fprintf( stdout, "Random seed: %lld\n", seed );
    fprintf( stdout, "Pair count: %lld\n", pair_count );
    fprintf( stdout, "Expected sum: %.16f", Haversine_Sum / pair_count );

    return 0;
}