#pragma once

#define RANDOM_32U ((f32)rand() / RAND_MAX)
#define RANDOM_32S (((f32)rand() / RAND_MAX) * 2.f - 1.f)

#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f
#define HALF_PI 1.57079632679489661923f

#define RAD 0.01745329251994329576f
#define DEG 57.29577951308232087679f

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SIGN(x) (-2.f * signbit(x) + 1.f )
