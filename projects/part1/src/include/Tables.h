#pragma once

#include <string>

static const char* registerLookupTable[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };

static const char* effectiveAddressTable[] = { "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx" };

static const char* jumpTable[] = { "jo", "jno", "jb", "jnb", "je", "jne", "jbe", "jnbe", "js", "jns", "jp", "jnp", "jl", "jnl", "jle", "jnle" };

static const char* loopTable[]{ "loopnz", "loopz", "loop", "jcxz" };

static const char* toFromOpTable[]{ "add", "mov", "no_op", "no_op", "no_op", "sub", "no_op", "cmd" };

char GetBits( char x, int p, int n );

const char* RegisterLookup( char reg, char w );