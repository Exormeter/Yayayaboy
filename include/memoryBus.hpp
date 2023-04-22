#pragma once

#include <map>
#include "./Peripheral/peripheral.hpp"
#include "./Peripheral/bootRom.hpp"


class MemoryBus : public Peripheral
{
public:

    MemoryBus()
    {
        m_peripheralMemoryMap.insert(bootRom.toPair());
        m_peripheralMemoryMap.insert(m_unmapBootRom.toPair());
        m_memoryMap[0x0] = this;
        m_unmapBootRom.setOnWriteHandler([&](const uint16_t address, uint8_t& value){
            m_memoryMap[0x0] = m_cartridge;
        });
        registerPeripheral(this);
    }

    void registerPeripheral(Peripheral* peripheral)
    {
        for (uint16_t address : peripheral->peripheralAddresses())
        {
            if (address != 0x0)
            {   
                m_memoryMap[address] = peripheral;
            }
            else
            {
                m_memoryMap[0x100] = peripheral;
                m_cartridge = peripheral;
            }
        }
    }

    /**
     * @brief Reads from a Peripheral on the memory map
     * 
     * @param address Address in the memory map
     * @return uint8_t memoryValue
     */
    uint8_t readMemoryBus(uint16_t address)
    {
        auto addressPeriperalIt = m_memoryMap.upper_bound(address);
        addressPeriperalIt--;
        return addressPeriperalIt->second->readFromPeripheral(address);
    }

    /**
     * @brief Write to a Peripheral on the MemoryMap
     * 
     * @param address Address in the memory map
     * @param value uint8_t value to write
     */
    void writeMemoryBus(uint16_t address, uint8_t value)
    {
        auto addressPeriperalIt = m_memoryMap.upper_bound(address);
        addressPeriperalIt--;
        return addressPeriperalIt->second->writeToPeripheral(address, value);
    }

private:
    Peripheral* m_cartridge;
    std::map<uint16_t, Peripheral*> m_memoryMap;
    Register<0xFF50> m_unmapBootRom;
    BootRom bootRom;
};