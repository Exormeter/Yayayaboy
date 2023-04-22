#pragma once
#include <inttypes.h>

namespace
{
    enum RegisterFlag : uint8_t
    {
        ZERO_FLAG = 0b10000000,
        SUB_FLAG = 0b01000000,
        HALF_CARRY_FLAG = 0b00100000,
        CARRY_FLAG = 0b00010000
    };
}

class StatusRegister
{
    public:

    StatusRegister(uint8_t& registerF) : statusRegister(&registerF)
    {};

    void inline checkZeroFlag(uint8_t operand)
    {
        if (!operand)
            *statusRegister |= RegisterFlag::ZERO_FLAG;
        else
            resetZeroFlag();
    }

    void inline resetZeroFlag()
    {
        *statusRegister &= ~RegisterFlag::ZERO_FLAG;
    }

    bool inline isZeroFlagSet()
    {
        return *statusRegister & RegisterFlag::ZERO_FLAG;
    }

    void inline setNegativFlag()
    {
        *statusRegister |= RegisterFlag::SUB_FLAG;
    }

    void inline resetNegativFlag()
    {
        *statusRegister &= ~RegisterFlag::SUB_FLAG;
    }

    bool inline isNegativFlagSet()
    {
        return *statusRegister & RegisterFlag::SUB_FLAG;
    }

//------------------------------------------------------------------------------------------------//

    void inline checkHalfCarryFlag16BitAdd(uint16_t operandA, uint16_t operandB)
    {
        if ((((operandA & 0xFFF) + (operandB & 0xFFF)) & 0x1000) == 0x1000)
            *statusRegister |= RegisterFlag::HALF_CARRY_FLAG;
        else
            resetHalfCarryFlag();
    }

    void inline checkCarryFlag16BitAdd(uint32_t operandA, uint32_t operandB)
    {
        if (operandA + operandB > 0xFFFF)
            *statusRegister |= RegisterFlag::CARRY_FLAG;
        else
            resertCarryFlag();
    }

    void inline checkHalfCarryFlag8BitAdd(uint16_t operandA, uint16_t operandB)
    {
        if ((((operandA & 0x0F) + (operandB & 0x0F)) & 0x10) == 0x10)
            *statusRegister |= RegisterFlag::HALF_CARRY_FLAG;
        else
            resetHalfCarryFlag();
    }

    void inline checkCarryFlag8BitAdd(uint32_t operandA, uint32_t operandB)
    {
        if (operandA + operandB > 0xFF)
            *statusRegister |= RegisterFlag::CARRY_FLAG;
        else
            resertCarryFlag();
    }

//------------------------------------------------------------------------------------------------//

    void inline checkHalfCarryFlag16BitSub(uint16_t minuend, uint16_t subtrahend)
    {
        if ((minuend & 0xFFF) < (subtrahend & 0xFFF))
            *statusRegister |= RegisterFlag::HALF_CARRY_FLAG;
        else
            resetHalfCarryFlag();
    }

    void inline checkCarryFlag16BitSub(uint32_t minuend, uint32_t subtrahend)
    {
        if (minuend < subtrahend)
            *statusRegister |= RegisterFlag::CARRY_FLAG;
        else
            resertCarryFlag();
    }

    void inline checkHalfCarryFlag8BitSub(uint8_t minuend, uint8_t subtrahend)
    {
        if ((minuend & 0x0F) < (subtrahend & 0x0F))
            *statusRegister |= RegisterFlag::HALF_CARRY_FLAG;
        else
            resetHalfCarryFlag();
    }

    void inline checkCarryFlag8BitSub(uint32_t minuend, uint32_t subtrahend)
    {
        if (minuend < subtrahend)
            *statusRegister |= RegisterFlag::CARRY_FLAG;
        else
            resertCarryFlag();
    }

//------------------------------------------------------------------------------------------------//

    void inline resetHalfCarryFlag()
    {
        *statusRegister &= ~RegisterFlag::HALF_CARRY_FLAG;
    }

    void inline setHalfCarryFlag()
    {
        *statusRegister |= RegisterFlag::HALF_CARRY_FLAG;
    }

    bool inline isHalfCarryFlagSet()
    {
        return *statusRegister & RegisterFlag::HALF_CARRY_FLAG;
    }

    void inline setCarryFlag()
    {
        *statusRegister |= RegisterFlag::CARRY_FLAG;
    }

    void inline resertCarryFlag()
    {
        *statusRegister &= ~RegisterFlag::CARRY_FLAG;
    }

    bool inline isCarryFlagSet()
    {
        return *statusRegister & RegisterFlag::CARRY_FLAG;
    }

    const uint8_t& getStatusRegister()
    {
        return *statusRegister;
    }

    private:
    uint8_t* statusRegister;
};