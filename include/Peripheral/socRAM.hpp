#pragma once

#include "peripheral.hpp"


#define HMEM_ADDRESS 0xFF80
#define HMEM_SIZE 0x7F

#define RAM_BANK_0_ADDRESS 0xC000
#define RAM_BANK_1_ADDRESS 0xD000
#define RAM_BANK_SIZE 0x1000

#define RAM_BANK_MIRROR_ADDRESS 0xE000
#define RAM_MIRROR_SIZE 0x1E00

#define RAM_UNUSED_RANGE_ADDRESS 0xFEA0
#define RAM_UNUSED_RANGE_SIZE 0x60



class SocRam : public Peripheral
{
public:
    SocRam()
    {
        m_peripheralMemoryMap.insert(m_hmem.toPair());
        m_peripheralMemoryMap.insert(m_internalRAMBank0.toPair());
        m_peripheralMemoryMap.insert(m_internalRAMBank1.toPair());
        
        m_peripheralMemoryMap.insert(m_mirrorRAM.toPair());
        m_peripheralMemoryMap.insert(m_unusedRange.toPair());

        m_peripheralMemoryMap.insert(m_unusedIORange0.toPair());
        m_peripheralMemoryMap.insert(m_unusedIORange1.toPair());
        m_peripheralMemoryMap.insert(m_unusedIORange2.toPair());
        m_peripheralMemoryMap.insert(m_unusedIORange3.toPair());

        m_peripheralMemoryMap.insert(m_unusedIORegister0.toPair());
        m_peripheralMemoryMap.insert(m_unusedIORegister1.toPair());
        m_peripheralMemoryMap.insert(m_unusedIORegister2.toPair());
        m_peripheralMemoryMap.insert(m_unusedIORegister3.toPair());
    }

private:

    MemoryRange<HMEM_ADDRESS, HMEM_SIZE> m_hmem;
    MemoryRange<RAM_BANK_0_ADDRESS, RAM_BANK_SIZE> m_internalRAMBank0;
    MemoryRange<RAM_BANK_1_ADDRESS, RAM_BANK_SIZE> m_internalRAMBank1;

    MemoryRange<RAM_BANK_MIRROR_ADDRESS, RAM_MIRROR_SIZE> m_mirrorRAM; //not emulated
    MemoryRange<RAM_UNUSED_RANGE_ADDRESS, RAM_UNUSED_RANGE_SIZE> m_unusedRange;

    MemoryRange<0xFF08, 0x07> m_unusedIORange0;
    MemoryRange<0xFF27, 0x09> m_unusedIORange1;
    MemoryRange<0xFF4C, 0x04> m_unusedIORange2;
    MemoryRange<0xFF51, 0x2F> m_unusedIORange3;

    Register<0xFF03> m_unusedIORegister0;
    Register<0xFF7F> m_unusedIORegister1;
    Register<0xFF15> m_unusedIORegister2;
    Register<0xFF1F> m_unusedIORegister3;
};