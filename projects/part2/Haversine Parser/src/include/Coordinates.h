#pragma once

#define EARTH_RADIUS 6372.8f

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