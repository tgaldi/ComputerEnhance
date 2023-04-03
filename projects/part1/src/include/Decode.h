#pragma once

#include <string>
#include "Registers.h"

struct Operation
{
    const char* name = "no_op";

    std::string dest;
    std::string src;
    std::string flags;
};

enum mode
{
    mem_mode,
    byte_mode,
    word_mode,
    reg_mode,
    mode_count
};

enum OperationType: char
{
    add = 0x00,
    mov = 0x01,
    no_op,
    no_op2,
    no_op3,
    sub = 0x05,
    no_op4,
    cmp = 0x07,
    opType_count
};

Operation DecodeInstruction( char*& opstream );

Operation Loop( char*& opstream );
Operation Jump( char*& opstream );
Operation Immediate( char*& opStream );
Operation To_From( char*& opStream );
Operation SimulateToFrom( Operation* op, char reg, char rm, char w, int data );
Operation Immediate_Accumulator( char*& opStream );
Operation Mov_Immediate( char*& opstream );

