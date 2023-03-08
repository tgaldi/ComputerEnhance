#pragma once
#include <string>

struct Operation
{
    const char* opName = "no_op";

    std::string dest;
    std::string src;
};

struct Instruction
{
    char opCode;

    typedef Operation( *ExecuteFunc )(char*&);
    ExecuteFunc Execute;
};

Instruction GetInstruction( char op );
Operation Mov_To_From( char*& opStream );
Operation Mov_Immediate( char*& opStream );