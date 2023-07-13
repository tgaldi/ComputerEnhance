#define _CRT_SECURE_NO_WARNINGS

// extern "C" { int _fltused = 0; }

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <intrin.h>
#include <windows.h>

#include <string>
#include <vector>
#include <random>
#include <cmath>

#include "Haversine_Parser.h"

#include "Platform_Metrics.cpp"
#include "Profiler.cpp"

#include "haversine_formula.cpp"
#include "PairsParser.cpp"
#include "main.cpp"