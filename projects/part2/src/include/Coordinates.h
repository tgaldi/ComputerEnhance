#pragma once

#include <stdio.h>

#include "types.h"

struct Coordinate
{
    f64 longitude;
    f64 latitude;
};

struct Pair
{
    Coordinate p0;
    Coordinate p1;
};

struct Point2D
{
    f32 x;
    f32 y;
};

struct Point3D
{
    f32 x;
    f32 y;
    f32 z;
};

f32 Length( Point2D point );

void Normalize( Point2D& point );

f32 Length( Point3D point );

void Normalize( Point3D& point );

Coordinate CartesianToLatLong( Point3D point );

Point3D ProjectPointOnSphere( Point2D pointIn, f32 elevationAngle, f32 radius = 1.f );

Point3D RandPointOnSphere( f32 radius = 1.f );

Point2D RandPointInCicle( f32 radius = 1.f );

void WritePairsToJson( FILE* file, Pair* pairs, u64 count );

void WritePairToJson( FILE* file, Pair& pair, bool lastElement = false );
