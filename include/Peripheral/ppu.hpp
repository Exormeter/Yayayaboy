#pragma once

#include "./peripheral.hpp"
#include "../Peripheral/lcdcStatus.hpp"
#include "../Interrupt/InterruptSource.hpp"
#include "../Memory/memoryRange.hpp"
#include "../Memory/register.hpp"

#include <map>
#include <array>
#include <span>

#define OAM_SIZE 160
#define VRAM_SIZE 0x2000

#define CYCLES_PER_LINE 456

#define OAM_CYCLES 80
#define LINE_RENDER_CYCLES 172
#define H_BLANK_CYCLES 204
#define V_BLANK_CYCLES 456

#define V_RES 144
#define H_RES 160

#define V_BLANK_BOUND 154

#define OAM_ADDRESS 0xFE00
#define VRAM_ADDRESS 0x8000

#define LCD_Y_LINE_ADDRESS 0xFF44

#define LCD_SCY_ADDRESS 0xFF42
#define LCD_SCX_ADDRESS 0xFF43

#define LCD_WY_ADDRESS 0xFF4A
#define LCD_WX_ADDRESS 0xFF4B

#define LCD_PALLET_ADDRESS 0xFF47

#define SPRITE_LIMIT 10

namespace
{
    class Sprite
    {
    
    static constexpr uint8_t verticalFlip = 0b01000000;
    static constexpr uint8_t horizontalFlip = 0b00100000;
    static constexpr uint8_t bgWindowOverObj = 0b10000000;
    
    public:
        uint8_t yPosition;
        uint8_t xPosition;
        uint8_t tilePointer;
        uint8_t attributes;
    
        bool isVerticalFlipped() { return attributes & verticalFlip; }
        bool isHorizontalFlipped() { return attributes & horizontalFlip; }
        bool isAboveBgWindow() {return attributes & bgWindowOverObj; }
    };
}

class PictureProcessingUnit : public Peripheral, public InterruptSource
{ 

public:
    PictureProcessingUnit(InterruptController& interruptController, LcdcStatus& lcdcStatus);

    uint8_t readFromPeripheral(uint16_t address);

    void writeToPeripheral(uint16_t address, uint8_t value);

    void registerDmaHandler(MemoryWriteHandler handler);

    void tick(uint8_t cycles);

    auto& objectAttributeMemory() { return m_oam; }
    ///uint8_t m_frameBuffer[V_RES][H_RES];
    uint8_t screenData[V_RES][H_RES][3];

private:

    auto getMemoryForAddress(uint16_t address)
    {
        auto memory = m_peripheralMemoryMap.upper_bound(address);
        memory--;
        return std::make_pair(memory->first, memory->second);
    }

    void drawLine(uint8_t line);
    void drawBackground(uint8_t line);
    void drawWindow(uint8_t line);
    void drawSprites(uint8_t line);
    void searchSprites(uint8_t line);
    void renderPixel(uint8_t xPos, uint8_t yPos, uint8_t pixel);

    std::reference_wrapper<LcdcStatus> m_lcdcStatus;

    uint8_t m_spritesInLine;
    std::array<Sprite, SPRITE_LIMIT> m_sprites;

    Register<LCD_SCY_ADDRESS> m_scrollY;
    Register<LCD_SCX_ADDRESS> m_scrollX;

    Register<LCD_WY_ADDRESS> m_windowY;
    Register<LCD_WX_ADDRESS> m_windowX;

    Register<LCD_Y_LINE_ADDRESS> m_yLine;

    Register<LCD_PALLET_ADDRESS> m_pallet;

    MemoryRange<OAM_ADDRESS, OAM_SIZE> m_oam;
    MemoryRange<VRAM_ADDRESS, VRAM_SIZE> m_vram;

    Register<0xFF01> m_testRegister;

    Register<0xFF46> m_omaDma;
    Register<0xFF48> m_objectPalette0;
    Register<0xFF49> m_objectPalette1;

    /*CGB Register*/

    Register<0xFF68> m_cgbPallet;
    Register<0xFF69> m_cgbDunno;
    Register<0xFF4F> m_cgbVRAM;

    PPUState m_currentMode = PPUState::OAM_SEARCH_MODE_2;
    uint16_t m_currentCycle = 0;
};