#include <map>
#include <span>
#include "../Peripheral/peripheral.hpp"
#include "../Memory/memoryRange.hpp"

#define ROM_BANK_SIZE 16384

#define ROM_BASE_ADDRESS_BANK_0 0
#define ROM_BASE_ADDRESS_BANK_1 0x4000

#define RAM_BASE_ADDRESS_BANK_0 0xA000

class Mcb1Cartridge : public Peripheral
{
public:

    Mcb1Cartridge(std::ifstream& romFile)
    {
        std::copy_n(std::istreambuf_iterator<char>(romFile), ROM_BANK_SIZE, bankZeroROM.begin());
        romFile.seekg(ROM_BANK_SIZE);
        std::copy(std::istreambuf_iterator<char>(romFile), std::istreambuf_iterator<char>(), bankNRom.begin());

        
        m_peripheralMemoryMap.insert(bankZeroROM.toPair());
        m_peripheralMemoryMap.insert(bankNRom.toPair());

        m_peripheralMemoryMap.insert(bankZeroRAM.toPair());

        bankZeroROM.setOnWriteHandler([&](const uint16_t address, uint8_t& value)
        {
            //ROM banking
            if (address >= 0x2000 & address <= 0x3FFF)
            {
                if (value != 0) value -= 1;
                bankNRom.setOffset(ROM_BANK_SIZE * value);
            }
        });

        bankNRom.setOnWriteHandler([&](const uint16_t address, uint8_t& value)
        {
            //RAM banking
            if (address >= 0x4000 & address <= 0x5FFF)
            {

            }

            //ROM/RAM select
            if (address >= 0x6000 & address <= 0x7FFF)
            {

            }
        });
    }

private:

    MemoryRange<ROM_BASE_ADDRESS_BANK_0, ROM_BANK_SIZE, true> bankZeroROM;
    MemoryRange<ROM_BASE_ADDRESS_BANK_1, ROM_BANK_SIZE * 128, true> bankNRom;

    MemoryRange<RAM_BASE_ADDRESS_BANK_0, 0x2000> bankZeroRAM;
};