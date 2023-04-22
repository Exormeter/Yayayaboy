#pragma once 
#include <span>
#include <map>

#include "instruction.hpp"
#include "statusRegister.hpp"
#include "Peripheral/peripheral.hpp"
#include "Interrupt/InterruptController.hpp"
#include "memoryBus.hpp"

using namespace std;

namespace
{
    struct GeneralRegister
    {
        uint8_t registerA = 0;
        uint8_t registerB = 0;
        uint8_t registerC = 0;
        uint8_t registerD = 0;
        uint8_t registerE = 0;
        uint8_t registerF = 0;
        uint8_t registerH = 0;
        uint8_t registerL = 0;

        uint16_t registerAF()
        {
            return ((uint16_t)registerA << 8) | registerF;
        }

        void registerAF(uint16_t value)
        {
            registerA = (uint8_t)(value >> 8);
            registerF = (uint8_t)value;
        }
        
        uint16_t registerBC()
        {
            return ((uint16_t) registerB << 8) | registerC;
        }

        void registerBC(uint16_t value)
        {
            registerB = (uint8_t)(value >> 8);
            registerC = (uint8_t)value;
        }

        uint16_t registerDE()
        {
            return ((uint16_t) registerD << 8) | registerE;
        }

        void registerDE(uint16_t value)
        {
            registerD = (uint8_t)(value >> 8);
            registerE = (uint8_t)value;
        }
        
        uint16_t registerHL()
        {
            return ((uint16_t) registerH << 8) | registerL;
        }

        void registerHL(uint16_t value)
        {
            registerH = (uint8_t)(value >> 8);
            registerL = (uint8_t)value;
        }
    };

    
}


class Cpu
{
public:

    Cpu(InterruptController& interruptController, MemoryBus& memoryBus) : 
        m_interruptController(&interruptController), m_memoryMap(&memoryBus), statusRegister(gpRegister.registerF)
    { }

    uint8_t step();

private:

    void fetch();

    void decode();

    void execute();

    void add8Bit(uint8_t operant);

    void add16Bit(uint16_t operant);

    void addCarry8Bit(uint8_t operant);

    void sub8Bit(uint8_t operant);

    void sub16Bit(uint16_t operant);

    void subCarry8Bit(uint8_t operant);

    void and8Bit(uint8_t operant);

    void or8Bit(uint8_t operant);

    void xor8Bit(uint8_t operant);

    void compare8Bit(uint8_t operant);

    void increment8Bit(uint8_t& operant);

    void decrement8Bit(uint8_t& operant);

    void swap(uint8_t& operant);

    void rotateLeft(uint8_t& operant);

    void rotateLeftCB(uint8_t& operant);

    void rotateLeftThroughCarry(uint8_t& operant);

    void rotateLeftThroughtCarryCB(uint8_t& operant);

    void rotateRight(uint8_t& operant);

    void rotateRightCB(uint8_t& operant);

    void rotateRightThroughCarry(uint8_t& operant);

    void rotateRightThroughCarryCB(uint8_t& operant);

    void shiftLeft(uint8_t& operant);

    void shiftRight(uint8_t& operant);

    void shiftRightKeepMSB(uint8_t& operant);

    void testBit(uint8_t bit, uint8_t inRegister);

    void setBit(uint8_t bit, uint8_t& operant);

    void resetBit(uint8_t bit, uint8_t& operant);

    void call(uint16_t address);

    void reset(uint8_t address);

    void funcReturn();


    MemoryBus* m_memoryMap;
    GeneralRegister gpRegister;
    InterruptController* m_interruptController;
    StatusRegister statusRegister;
    uint16_t stackPointer;
    uint16_t programmCounter;
    uint16_t currentOpCode;
    Instructions::Instruction currentInstruction;
    bool m_isHalted = false;
};