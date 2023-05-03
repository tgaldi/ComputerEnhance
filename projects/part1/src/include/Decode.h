#pragma once

#include <string>
#include "Registers.h"
#include "Types.h"

struct Operation
{
    const char* name = "no_op";

    std::string dest;
    std::string src;
    std::string flags;
    int clockCount;
    short ip;
};

enum mode
{
    mem_mode,
    byte_mode,
    word_mode,
    reg_mode,
    mode_count
};

enum OperationType : u8
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

Operation DecodeInstruction( u8*& opstream, u8* memory );

Operation Loop( u8*& opstream );
Operation Jump( u8*& opstream );
Operation Immediate( u8*& opStream );
Operation To_From( u8*& opStream );
Operation Immediate_Accumulator( u8*& opStream );
Operation Mov_Immediate_Register( u8*& opstream );
Operation Mov_Immediate_Register_Memory( u8*& opstream );

