#pragma once

#include <cassert>
#include "memory.hpp"

template<uint16_t t_registerAddress>
class Register : public Memory
{
    public:

        uint8_t readMemory(uint16_t address)
        {
            assert(address == t_registerAddress);
            if (m_onReadHandler) { m_onReadHandler(m_register); }
            return m_register;
        }

        void writeMemory(uint16_t address, uint8_t value)
        {
            assert(address == t_registerAddress);
            if (m_onWriteHandler) { m_onWriteHandler(address, value); }
            m_register = value;
        }

        std::vector<uint16_t> peripheralAddresses()
        {
            return {t_registerAddress};
        }

        std::pair<uint16_t, Memory*> toPair()
        {
            return std::make_pair(t_registerAddress, this);
        }

        uint8_t& value()
        {
            return m_register;
        }

    private:
        uint8_t m_register;
};