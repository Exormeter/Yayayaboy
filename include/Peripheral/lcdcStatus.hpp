#pragma once

#include <assert.h>
#include "Peripheral/peripheral.hpp"
#include "Interrupt/InterruptSource.hpp"

namespace
{
    enum class PPUState : uint8_t
    {
        H_BLANK_MODE_0 = 0b00000000,
        V_BLANK_MODE_1 = 0b00000001,
        OAM_SEARCH_MODE_2 = 0b00000010,
        LINE_RENDER_MODE_3 = 0b00000011,
    };
}

#define LCDC_CONTROL_ADDRESS 0xFF40 
#define LCDC_STATUS_ADDRESS 0xFF41
#define LCD_Y_COMP_ADDRESS 0xFF45

#define PPU_ENABLE_FLAG             (1 << 7)
#define WINDOW_TILE_MAP_FLAG        (1 << 6) 
#define WINDOW_ENABLE_FLAG          (1 << 5)
#define BG_WINDOW_TILE_AREA_FLAG    (1 << 4)
#define BG_TILE_MAP_FLAG            (1 << 3)
#define SPRITE_SIZE_FLAG            (1 << 2)
#define SPRITE_ENABLE_FLAG          (1 << 1)
#define BG_WINDOW_PRIO_FLAG         (1 << 0)

#define MODE_LYC_IE                 (1 << 6)
#define MODE_OAM_IE                 (1 << 5)
#define MODE_VBLANK_IE              (1 << 4)
#define MODE_HBLANK_IE              (1 << 3)
#define LY_COMPARE_BIT              (1 << 2)


class LcdcStatus : public Peripheral, public InterruptSource
{

public:

    LcdcStatus(InterruptController& interruptController) : InterruptSource(interruptController, InterruptFlags::LCD_STAT_FLAG)
    {
        m_peripheralMemoryMap.insert(m_statusRegister.toPair());
        m_peripheralMemoryMap.insert(m_controlRegister.toPair());
        m_peripheralMemoryMap.insert(m_yCompareRegister.toPair());
    }

    void updateStatus(PPUState state, uint8_t line)
    {
        if (m_currentPPUState == state) return;
        
        m_statusRegister.value() &= 0b11111100;
        m_statusRegister.value() |= (uint8_t)state;

        switch(state)
        {
            case PPUState::H_BLANK_MODE_0:
                if (m_statusRegister.value() & MODE_HBLANK_IE) raiseInterrupt();
                break;
            case PPUState::V_BLANK_MODE_1:
                if (m_statusRegister.value() & MODE_VBLANK_IE) raiseInterrupt();
                break;
            case PPUState::OAM_SEARCH_MODE_2:
                if (m_statusRegister.value() & MODE_OAM_IE) raiseInterrupt();
                break;
            case PPUState::LINE_RENDER_MODE_3: break;
        }
        
        if (m_yCompareRegister.value() == line)
        {
            if(m_statusRegister.value() & MODE_LYC_IE) raiseInterrupt();
            m_statusRegister.value() |= LY_COMPARE_BIT;
        }
        else
        {
            m_statusRegister.value() &= ~LY_COMPARE_BIT;
        }
        m_currentPPUState = state;
        
    }      
    void updateCurrentLine(uint8_t line);
    
    inline bool ppuEnabled() { return m_controlRegister.value() & PPU_ENABLE_FLAG; }

    inline int windowTileMapPointer() { return m_controlRegister.value() & WINDOW_TILE_MAP_FLAG? 0x1C00 : 0x1800; }

    inline bool windowEnabled() { return m_controlRegister.value() & WINDOW_ENABLE_FLAG; }

    inline int bgWindowTileDataPointer() { return m_controlRegister.value() & BG_WINDOW_TILE_AREA_FLAG? 0x00: 0x800; }

    inline int backgroundTileMapPointer() { return m_controlRegister.value() & BG_TILE_MAP_FLAG? 0x1C00 : 0x1800; }

    inline uint8_t spriteSize() { return m_controlRegister.value() & SPRITE_SIZE_FLAG ? 16 : 8; }

    inline bool spriteEnabled() { return m_controlRegister.value() & SPRITE_ENABLE_FLAG; }

    inline bool bgWindowPrioEnabled() { return m_controlRegister.value() & BG_WINDOW_PRIO_FLAG; }

private:

    PPUState m_currentPPUState;

    Register<LCDC_STATUS_ADDRESS> m_statusRegister;
    Register<LCDC_CONTROL_ADDRESS> m_controlRegister;
    Register<LCD_Y_COMP_ADDRESS> m_yCompareRegister;
};