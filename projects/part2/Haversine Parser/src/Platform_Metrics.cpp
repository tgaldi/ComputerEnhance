#include <intrin.h>
#include <windows.h>

#include "types.h"
#include "logging.h"

static u64 GetOSTimerFrequency()
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency( &Frequency );
    return Frequency.QuadPart;
}

static u64 ReadOSTimer()
{
    LARGE_INTEGER Value;
    QueryPerformanceCounter( &Value );
    return Value.QuadPart;
}

inline u64 ReadCPUTimer()
{
    return __rdtsc();
}

inline u64 EstimeCPUFrequency( u64 msToWait, bool bOutputMetrics = false )
{
    u64 OSFreq = GetOSTimerFrequency(); // os timer ticks per second
    u64 OSWaitTime = OSFreq * msToWait / 1000;

    u64 CPUStart = ReadCPUTimer();

    u64 OSStart = ReadOSTimer();
    u64 OSEnd = 0;
    u64 OSElapsed = 0;
    while( OSElapsed < OSWaitTime )
    {
        OSEnd = ReadOSTimer();
        OSElapsed = OSEnd - OSStart;
    }

    u64 CPUEnd = ReadCPUTimer();

    u64 CPUElapsed = CPUEnd - CPUStart;
    u64 CPUFreq = 0;
    if( OSElapsed )
    {
        CPUFreq = OSFreq * CPUElapsed / OSElapsed;
    }

    if( bOutputMetrics )
    {
        PRINT( "-----------------------------------------------------------------------------------------------" );
        PRINT( " OS Frequency: %llu", OSFreq );
        PRINT( "     OS Timer: %llu -> %llu = %llu elapsed", OSStart, OSEnd, OSElapsed );
        PRINT( "   OS Seconds: %.4f", (f64)OSElapsed / (f64)OSFreq );
        PRINT( "    CPU Timer: %llu -> %llu = %llu elapsed", CPUStart, CPUEnd, CPUElapsed );
        PRINT( "CPU Frequency: %llu (guessed)", CPUFreq );
        WRITE_LINE( "SystemInfo:" );
        system( "systeminfo | findstr \"Intel64\"" );
        PRINT( "-----------------------------------------------------------------------------------------------\n" );
    }

    return CPUFreq;
}