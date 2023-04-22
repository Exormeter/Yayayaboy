#pragma once

#include <span>

namespace Instructions
{

    struct Instruction
    {
        uint16_t operation;
        uint8_t length;
        uint16_t operant;
        uint8_t cycles;
    };

    Instruction getInstruction(uint16_t opCode);

    Instruction getCBInstruction(uint16_t opCode);
}
