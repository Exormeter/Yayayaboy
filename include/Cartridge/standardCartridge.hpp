#pragma once

#include <map>
#include "../Peripheral/peripheral.hpp"
#include "../Memory/memoryRange.hpp"

#define BANK_0_ROM_SIZE 32768
#define BANK_0_RAM_SIZE 8192

#define ROM_BASE_ADDRESS 0
#define RAM_BASE_ADDRESS 0xA000

class StandardCartridge : public Peripheral
{
    public:

        StandardCartridge(std::ifstream& romFile)
        {
            std::copy(std::istreambuf_iterator<char>(romFile), std::istreambuf_iterator<char>(), bankZeroROM.begin());
            m_peripheralMemoryMap.insert(bankZeroRAM.toPair());
            m_peripheralMemoryMap.insert(bankZeroROM.toPair());
        }

    private:

        MemoryRange<ROM_BASE_ADDRESS, BANK_0_ROM_SIZE, true> bankZeroROM;
        MemoryRange<RAM_BASE_ADDRESS, BANK_0_RAM_SIZE> bankZeroRAM;
};