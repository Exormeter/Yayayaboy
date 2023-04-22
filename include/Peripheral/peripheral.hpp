#pragma once


#include "../Memory/memory.hpp"
#include <stdint.h>
#include <vector>
#include <map>

class Peripheral
{
public:

    virtual uint8_t readFromPeripheral(uint16_t address)
    {
        auto peripheralItr = m_peripheralMemoryMap.upper_bound(address);
        peripheralItr--;
        return peripheralItr->second->readMemory(address);
    }

    virtual void writeToPeripheral(uint16_t address, uint8_t value)
    {
        auto peripheralItr = m_peripheralMemoryMap.upper_bound(address);
        peripheralItr--;
        peripheralItr->second->writeMemory(address, value);
    }

    virtual std::vector<uint16_t> peripheralAddresses()
    {
        std::vector<uint16_t> addressVector;
        for (auto peripheral : m_peripheralMemoryMap)
        {
            addressVector.push_back(peripheral.first);
        }
        return addressVector;
    }

protected:

    std::map<uint16_t, Memory*> m_peripheralMemoryMap;

};