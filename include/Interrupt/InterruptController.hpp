#pragma once

#include "./../Peripheral/peripheral.hpp"
#include "./../Memory/register.hpp"
#include <map>
#include <assert.h> 

#define INTERRUPT_ENABLE_ADDR 0xFFFF
#define INTERRUPT_FLAGS_ADDR 0xFF0F


enum class InterruptFlags : uint8_t
{
    NO_INTERRUPT    = 0,
    V_BLANK_FLAG    = 0b00000001,
    LCD_STAT_FLAG   = 0b00000010,
    TIMER_FLAG      = 0b00000100,
    SERIAL_FLAG     = 0b00001000,
    JOYPAD_FLAG     = 0b00010000
};


class InterruptController : public Peripheral
{

public:

    InterruptController()
    {
        m_peripheralMemoryMap.insert(m_interruptEnable.toPair());
        m_peripheralMemoryMap.insert(m_interruptFlags.toPair());
    }

    inline void enableInterrupts()
    {
        m_masterInterruptEnabled = true;
    }

    inline void disableInterrupts()
    {
        m_masterInterruptEnabled =  false;
    }

    inline void raiseInterrupt(InterruptFlags flags)
    {
        m_interruptFlags.value() |= (uint8_t) flags;
    }

    inline bool hasPendingInterrupt()
    {
        if (!m_masterInterruptEnabled)
            return false;

        return (m_interruptEnable.value() & m_interruptFlags.value());
    }

    inline bool shouldWakeupFronHalt()
    {
        return (m_interruptEnable.value() & m_interruptFlags.value());
    }

    inline uint16_t pendingInterruptAddress()
    {
        if (m_interruptFlags.value() & m_interruptEnable.value() & (uint8_t)InterruptFlags::V_BLANK_FLAG)
        {
            m_interruptFlags.value() &= ~(uint8_t)InterruptFlags::V_BLANK_FLAG;
            return 0x40;
        }
            

        else if (m_interruptFlags.value() & m_interruptEnable.value() & (uint8_t)InterruptFlags::LCD_STAT_FLAG)
        {
            m_interruptFlags.value() &= ~(uint8_t)InterruptFlags::LCD_STAT_FLAG;
            return 0x48;
        }

        else if (m_interruptFlags.value() & m_interruptEnable.value() & (uint8_t)InterruptFlags::TIMER_FLAG)
        {
            m_interruptFlags.value() &= ~(uint8_t)InterruptFlags::TIMER_FLAG;
            return 0x50;
        }

        else if (m_interruptFlags.value() & m_interruptEnable.value() & (uint8_t)InterruptFlags::SERIAL_FLAG)
        {
            m_interruptFlags.value() &= ~(uint8_t)InterruptFlags::SERIAL_FLAG;
            return 0x58;
        }
        else if (m_interruptFlags.value() & m_interruptEnable.value() & (uint8_t)InterruptFlags::JOYPAD_FLAG)
        {
            m_interruptFlags.value() &= ~(uint8_t)InterruptFlags::JOYPAD_FLAG;
            return 0x60;
        }
        //assert(false);
        return 0x0;
    }

private:

    bool m_masterInterruptEnabled = false;

    Register<INTERRUPT_ENABLE_ADDR> m_interruptEnable;
    Register<INTERRUPT_FLAGS_ADDR> m_interruptFlags;
};