#pragma once

#include "Instructions.h"

Instruction instructionTable[] = { { 0b1011000, &Mov_Immediate }, { 0b00100010, &Mov_To_From }, { 0b0 } };

std::string registerLookupTable[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };

std::string effectiveAddressTable[] = { "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx" };

