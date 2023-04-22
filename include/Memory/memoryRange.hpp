#pragma once

#include "memory.hpp"

template<uint16_t t_startAddress, int t_size, bool t_readOnly = false>
class MemoryRange : public Memory
{

    public:

        virtual uint8_t readMemory(uint16_t absolutAddress) override
        {
            uint16_t relativeAddress = (absolutAddress - t_startAddress) + m_offset;
            assert((relativeAddress) <= t_size);
            if (m_onReadHandler) { m_onReadHandler(absolutAddress); }
            return m_memory[relativeAddress];
        }

        virtual void writeMemory(uint16_t absolutAddress, uint8_t value) override
        {   
            if (m_onWriteHandler) { m_onWriteHandler(absolutAddress, value); }
            if constexpr(t_readOnly) return;

            uint16_t relativeAddress = (absolutAddress - t_startAddress) + m_offset;
            assert(relativeAddress <= t_size);
            m_memory[relativeAddress] = value;
        }

        virtual std::vector<uint16_t> peripheralAddresses() override
        {
            return {t_startAddress};
        }

        std::pair<uint16_t, Memory*> toPair() override
        {
            return std::make_pair(t_startAddress, this);
        }

        uint8_t& operator [](int i) { return m_memory[i]; }

        uint8_t operator [](int i) const { return m_memory[i]; }

        uint8_t* begin() { return m_memory; }

        void setOffset(uint32_t offset) { m_offset = offset; }

    protected:

        uint8_t m_memory[t_size];
        uint32_t m_offset = 0;
};