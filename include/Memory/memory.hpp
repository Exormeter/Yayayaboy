#pragma once

#include <utility>
#include <cstdint>
#include <cassert>
#include <functional>

using MemoryReadHandler = std::function<void(const uint16_t& address)>;
using MemoryWriteHandler = std::function<void(const uint16_t& address, uint8_t& value)>;

class Memory
{

public:
    virtual uint8_t readMemory(uint16_t address) = 0;

    virtual void writeMemory(uint16_t address, uint8_t value) = 0;

    virtual std::vector<uint16_t> peripheralAddresses() = 0;

    virtual std::pair<uint16_t, Memory*> toPair() = 0;

    void setOnReadHandler(MemoryReadHandler onReadHandler)
    {
        m_onReadHandler = onReadHandler;
    }

    void setOnWriteHandler(MemoryWriteHandler onWriteHandler)
    {
        m_onWriteHandler = onWriteHandler;
    }

protected:
    MemoryReadHandler m_onReadHandler = nullptr;
    MemoryWriteHandler m_onWriteHandler = nullptr;
};