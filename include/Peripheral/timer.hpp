#pragma once

#include "peripheral.hpp"
#include "../Memory/register.hpp"


#define TIMER_ENABLE (1 << 2)

#define CLOCK_DIV_1024 0b00000000
#define CLOCK_DIV_256  0b00000011
#define CLOCK_DIV_64   0b00000010
#define CLOCK_DIV_16   0b00000001

class Timer : public Peripheral, public InterruptSource
{
public:
    Timer(InterruptController& interruptController) : InterruptSource(interruptController, InterruptFlags::TIMER_FLAG)
    {
        m_peripheralMemoryMap.insert(m_divider.toPair());
        m_peripheralMemoryMap.insert(m_counter.toPair());
        m_peripheralMemoryMap.insert(m_modulo.toPair());
        m_peripheralMemoryMap.insert(m_control.toPair());

        m_divider.setOnWriteHandler([&](const uint16_t address, uint8_t& value)
        {
            value = 0x00;
        });

        m_control.setOnWriteHandler([&](const uint16_t address, uint8_t& value)
        {
            m_currentDivider = clockDivider(value);
        });

    }

    void step(uint8_t cycle)
    {
        m_dividerCycle += cycle;
        m_divider.value() += m_dividerCycle / 1024;
        if (m_dividerCycle >= 1024) m_dividerCycle = 0;

        if (isEnabled()) 
        {
            m_counterCycle += cycle;
            uint8_t timerDelta = m_counterCycle / m_currentDivider;
            m_counter.value() += timerDelta;
            if (m_counter.value() == 0 && timerDelta)
            {
                raiseInterrupt();
                m_counter.value() = m_modulo.value();
            }
            if (m_counterCycle >= m_currentDivider) m_counterCycle = 0;
        }
    }

private:
    bool isEnabled() { return (m_control.value() & TIMER_ENABLE); }

    uint16_t clockDivider(uint8_t controlValue)
    {
        switch(controlValue & 0b00000011)
        {
            case CLOCK_DIV_1024: return 1024;
            case CLOCK_DIV_256: return 256;
            case CLOCK_DIV_64: return 64;
            case CLOCK_DIV_16: return 16;
            default: assert(false);
        }
        return 0;
    }
    
    uint16_t m_currentDivider = 1024;
    uint16_t m_counterCycle = 0;
    uint16_t m_dividerCycle = 0;

    Register<0xFF04> m_divider;
    Register<0xFF05> m_counter;
    Register<0xFF06> m_modulo;
    Register<0xFF07> m_control;
};