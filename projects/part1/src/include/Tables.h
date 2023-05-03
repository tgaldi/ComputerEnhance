#pragma once

#include <string>
#include "Types.h"

extern const char* registerLookupTable[];

extern const char* effectiveAddressTable[];

extern const char* jumpTable[];

extern const char* loopTable[];

extern const char* toFromOpTable[];

u8 GetBits( u8 x, int p, int n );

const char* RegisterLookup( u8 reg, u8 w );
