#pragma once

#include <string>

extern const char* registerLookupTable[];

extern const char* effectiveAddressTable[];

extern const char* jumpTable[];

extern const char* loopTable[];

extern const char* toFromOpTable[];

char GetBits( char x, int p, int n );

const char* RegisterLookup( char reg, char w );