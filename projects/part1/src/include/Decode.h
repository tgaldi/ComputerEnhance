#pragma once

#include <string>
#include "Registers.h"

struct Operation
{
    const char* name = "no_op";

    std::string dest;
    std::string src;

    Register* stored;
};

enum mode
{
    mem_mode,
    byte_mode,
    word_mode,
    reg_mode,
    mode_count
};

Operation DecodeInstruction( char*& opstream );

Operation Loop( char*& opstream );
Operation Jump( char*& opstream );
Operation Immediate( char*& opStream );
Operation To_From( char*& opStream );
Operation Immediate_Accumulator( char*& opStream );
Operation Mov_Immediate( char*& opstream );
std::string BuildEffectiveRegisterEncoding( const char* registerName, int* data = nullptr );

