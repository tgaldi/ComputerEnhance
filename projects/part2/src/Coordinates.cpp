#include "Coordinates.h"

#include "FMath.h"

f32 Length( Point2D point ) { return sqrt( point.x * point.x + point.y * point.y ); };

void Normalize( Point2D& point )
{
    f32 mag = Length( point );

    point.x /= mag;
    point.y /= mag;
}

f32 Length( Point3D point ) { return sqrt( point.x * point.x + point.y * point.y + point.z * point.z ); }

void Normalize( Point3D& point )
{
    f32 mag = Length( point );

    point.x /= mag;
    point.y /= mag;
    point.z /= mag;
}

Coordinate CartesianToLatLong( Point3D point )
{
    // calculate the distance from the center of earth
    f64 distance = Length( point );

    // calculate the angle between poles
    // normalize because asin is [-1,1]
    f64 elevation = asin( point.z / distance );

    // calculate the angle around the equator
    f64 azimuth = atan2( point.y, point.x );

    Coordinate coordinate;
    coordinate.latitude = elevation * DEG;
    coordinate.longitude = azimuth * DEG;

    return coordinate;
}

Point3D ProjectPointOnSphere( Point2D pointIn, f32 elevationAngle, f32 radius )
{
    Point3D pointOut;

    // transform into normalized 3d point
    f32 distance = Length( pointIn );
    pointOut.x = pointIn.x / distance;
    pointOut.y = pointIn.y / distance;

    // calculate vertical displacement
    pointOut.z = cos( elevationAngle );

    // subtract horizontal displacement
    f32 zSquared = 1.f - (pointOut.x * pointOut.x - pointOut.y * pointOut.y);

    // normalize to surface of a unit sphere, using the direction of the sign of the angle
    pointOut.z *= sqrt( zSquared ) * SIGN( elevationAngle );

    // scale by radius
    pointOut.x *= radius;
    pointOut.y *= radius;
    pointOut.z *= radius;

    return pointOut;
}

//NOTE: To get a random point on the surface of a sphere:
// 1. Generate a random angle within a circle's range [0,2pi]
// 2. Generate a random angle within a half-circle's range [0,pi]
// 3. Covert the polar coordinates to cartesian
Point3D RandPointOnSphere( f32 radius )
{
    Point3D point;

    // generate number between 0 and 1 to get a random sample of 2pi
    // this represents the horizontal coordinate of the sphere (a circle)
    f32 azimuth = RANDOM_32U * TWO_PI;

    // generate a number between -1 and 1 to get a random sample of pi/2
    // this represents the vertical coordinate of the sphere (a half circle)
    f32 elevation = RANDOM_32S * HALF_PI;

    // calculate the vertical displacement along the y-axis and the horizontal along the x-axis
    point.x = radius * sin( elevation ) * cos( azimuth );

    // calculate the vertical and horizontal displacements along the y-axis
    point.y = radius * sin( elevation ) * sin( azimuth );

    // calculate the horizontal displacement along the z-axis, which is perpendicular to the horizontal plane
    point.z = radius * cos( elevation );

    return point;
}

//NOTE: To get a random point in a circle:
// 1. Generate a random angle within a circle's range [0,2pi]
// 2. Generate a random radius by scaling the radius of the circle
// 3. Covert the polar coordinates to cartesian
Point2D RandPointInCicle( f32 radius )
{
    Point2D point;

    f32 angle = RANDOM_32U * TWO_PI;
    f32 r = RANDOM_32U * radius;

    point.x = r * cos( angle );
    point.y = r * sin( angle );

    return point;
}

void WritePairsToJson( FILE* file, Pair* pairs, u64 count )
{
    fprintf( file, "{\"pairs\":[" );

    for( u64 i = 0; i < count; ++i )
    {
        fprintf( file, "\n\t{" );

        fprintf( file, "\"x0\":%.16f, ", pairs[i].p0.longitude );
        fprintf( file, "\"y0\":%.16f, ", pairs[i].p0.latitude );
        fprintf( file, "\"x1\":%.16f, ", pairs[i].p1.longitude );
        fprintf( file, "\"y1\":%.16f", pairs[i].p1.latitude );

        fprintf( file, "}%s", (i < count - 1) ? "," : "" );
    }

    fprintf( file, "\n]}" );
}

void WritePairToJson( FILE* file, Pair& pair, bool lastElement )
{
    fprintf( file, "\n\t{" );

    fprintf( file, "\"x0\":%.16f, ", pair.p0.longitude );
    fprintf( file, "\"y0\":%.16f, ", pair.p0.latitude );
    fprintf( file, "\"x1\":%.16f, ", pair.p1.longitude );
    fprintf( file, "\"y1\":%.16f", pair.p1.latitude );

    fprintf( file, "}%s", lastElement ? "" : "," );
}