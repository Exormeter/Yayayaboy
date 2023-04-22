#include "../include/Peripheral/ppu.hpp"

PictureProcessingUnit::PictureProcessingUnit(InterruptController& interruptController, LcdcStatus& lcdcStatus) : 
    m_lcdcStatus(lcdcStatus), InterruptSource(interruptController, InterruptFlags::V_BLANK_FLAG)
{
    m_peripheralMemoryMap.insert(m_oam.toPair());
    m_peripheralMemoryMap.insert(m_vram.toPair());

    m_peripheralMemoryMap.insert(m_yLine.toPair());
    
    m_peripheralMemoryMap.insert(m_scrollX.toPair());
    m_peripheralMemoryMap.insert(m_scrollY.toPair());
    m_peripheralMemoryMap.insert(m_windowX.toPair());
    m_peripheralMemoryMap.insert(m_windowY.toPair());
    m_peripheralMemoryMap.insert(m_pallet.toPair());

    m_peripheralMemoryMap.insert(m_omaDma.toPair());

    m_peripheralMemoryMap.insert(m_objectPalette0.toPair());
    m_peripheralMemoryMap.insert(m_objectPalette1.toPair());
}

void PictureProcessingUnit::registerDmaHandler(MemoryWriteHandler handler)
{
    m_omaDma.setOnWriteHandler(handler);
}

uint8_t PictureProcessingUnit::readFromPeripheral(uint16_t address)
{   
    uint8_t memoryValue = 0;

    switch(m_currentMode)
    {
        case PPUState::H_BLANK_MODE_0:
        case PPUState::V_BLANK_MODE_1:
            memoryValue = Peripheral::readFromPeripheral(address);
            break;

        case PPUState::LINE_RENDER_MODE_3:
        {
            auto memory = getMemoryForAddress(address);
            if (memory.first == OAM_ADDRESS || memory.first == VRAM_ADDRESS)
            {
                memoryValue = 0xFF;
            }
                
            else
            {
                memoryValue = memory.second->readMemory(address);
            }
            
        }

        case PPUState::OAM_SEARCH_MODE_2:
        {
            auto memory = getMemoryForAddress(address);
            memoryValue = memory.first == OAM_ADDRESS? 0xFF : memory.second->readMemory(address);
        }
    }

    return memoryValue;
}

void PictureProcessingUnit::writeToPeripheral(uint16_t address, uint8_t value)
{
    switch(m_currentMode)
    {
        case PPUState::H_BLANK_MODE_0:
        case PPUState::V_BLANK_MODE_1:
            return Peripheral::writeToPeripheral(address, value);

        case PPUState::LINE_RENDER_MODE_3:
        {
            auto memory = getMemoryForAddress(address);
            if (memory.first != OAM_ADDRESS && memory.first != VRAM_ADDRESS)
                return memory.second->writeMemory(address, value);
        }

        case PPUState::OAM_SEARCH_MODE_2:
        {
            auto memory = getMemoryForAddress(address);
            if (memory.first != OAM_ADDRESS)
                return memory.second->writeMemory(address, value);
        }
    }
}


void PictureProcessingUnit::tick(uint8_t cycles)
{

    if (!m_lcdcStatus.get().ppuEnabled())
    {
        m_currentMode = PPUState::H_BLANK_MODE_0;
        m_currentCycle = 0;
        m_yLine.value() = 0;
        m_lcdcStatus.get().updateStatus(m_currentMode, m_yLine.value());
        return;
    }
    
    m_lcdcStatus.get().updateStatus(m_currentMode, m_yLine.value());

    if (m_currentCycle == 0 && m_currentMode == PPUState::V_BLANK_MODE_1)
        raiseInterrupt();

    m_currentCycle += cycles;

    switch(m_currentMode)
    {
        case PPUState::OAM_SEARCH_MODE_2:


            if (m_currentCycle >= OAM_CYCLES)
            {
                m_currentMode = PPUState::LINE_RENDER_MODE_3;
                m_currentCycle = 0;
                searchSprites(m_yLine.value());
            }
                

            break;

        case PPUState::LINE_RENDER_MODE_3:

            if (m_currentCycle >= LINE_RENDER_CYCLES)
            {
                m_currentMode = PPUState::H_BLANK_MODE_0;
                m_currentCycle = 0;
                drawLine(m_yLine.value());
                m_yLine.value()++;
            }
            break;

        case PPUState::H_BLANK_MODE_0:
        {
            auto nextMode = m_yLine.value() == V_RES - 1 ? PPUState::V_BLANK_MODE_1 : PPUState::OAM_SEARCH_MODE_2;
 
            if (m_currentCycle >= H_BLANK_CYCLES)
            {
                m_currentCycle = 0;
                m_currentMode = nextMode;
            }

            break;
        }
        case PPUState::V_BLANK_MODE_1:

            if (m_currentCycle >= V_BLANK_CYCLES)
            {
                m_currentCycle = 0;
                m_yLine.value()++;
            }

            if (m_yLine.value() == V_BLANK_BOUND)
            {
                m_currentMode = PPUState::OAM_SEARCH_MODE_2;
                m_yLine.value() = 0;
            }
                
            break;
    }
}

void PictureProcessingUnit::drawLine(uint8_t line)
{   
    if (m_lcdcStatus.get().bgWindowPrioEnabled())     
    {
        drawBackground(line);

        if (m_lcdcStatus.get().windowEnabled())
            drawWindow(line);
        
    }

    if (m_lcdcStatus.get().spriteEnabled())
        drawSprites(line);
}

void PictureProcessingUnit::drawBackground(uint8_t line)
{
    int backgroundLine = line + m_scrollY.value();

    if (backgroundLine >= 255)
        backgroundLine -= 255;

    int tileIndex = (backgroundLine / 8) * 32;

    int tileBaseIndex = tileIndex;

    tileIndex += (m_scrollX.value() / 8);

    int tileRowIndex = (backgroundLine % 8) * 2;

    int tilePixelIndex = m_scrollX.value() % 8;

    for (int framePixelIndex = 0; framePixelIndex < H_RES; framePixelIndex++)
    {

        // last pixel in current tile is reached, go to next tile
        if (tilePixelIndex == 8)
        {
            tileIndex++;
            
            //warp around
            if (tileIndex % 32 == 0)
            {
                tileIndex -= 32;
            }
            tilePixelIndex = 0;
        }


        uint16_t tileAddress = 0;

        // the pointer to the tile signed
        if (m_lcdcStatus.get().bgWindowTileDataPointer() != 0)
        {
            int8_t tileMapPointer = m_vram[m_lcdcStatus.get().backgroundTileMapPointer() + tileIndex];
            tileAddress = m_lcdcStatus.get().bgWindowTileDataPointer() + ((tileMapPointer + 128) * 16);
        }
        else
        {   
            //Multiply by 16 since the tile is 16 bytes long and we need to space the index corretly
            tileAddress = m_vram[m_lcdcStatus.get().backgroundTileMapPointer() + tileIndex] * 16;
        }
        
        //Two bytes that represent the current row we are drawing, 2bpp
        uint8_t lowByte = m_vram[tileAddress + tileRowIndex];
        uint8_t highByte = m_vram[tileAddress + tileRowIndex + 1];

        //the actually current Pixel in the row is tilePixelIndex, inverted so we can shift the bits into the pixel easily
        int invertedTilePixelIndex = 7 - tilePixelIndex;

        uint8_t pixel = 0;
        pixel |= ((lowByte >> invertedTilePixelIndex) & 0x1);
        pixel |= (((highByte >> invertedTilePixelIndex) & 0x1) << 1);

        switch(pixel)
        {
            case 0x0: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0xFF; break;
            case 0x1: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0xCC; break;
            case 0x2: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0x77; break;
            case 0x3: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0x00; break;
        }
        

        tilePixelIndex++;
    }
}

void PictureProcessingUnit::drawWindow(uint8_t line)
{
    if (line < m_windowY.value()) return;

    int tileIndex = ((line - m_windowY.value()) / 8) * 32;
    int tilePixelIndex = 0;
    int tileBaseIndex = tileIndex;

    int tileRowIndex = ((line - m_windowY.value()) % 8) * 2;
    //start drawing from WX position
    for (int framePixelIndex = m_windowX.value() - 7; framePixelIndex < H_RES; framePixelIndex++)
    {
        //reached the end of the row, continue with next tile
        if (tilePixelIndex == 8)
        {
            tilePixelIndex = 0;
            tileIndex++;
        }

        if ((tileBaseIndex - tileIndex) == 32)
            tileIndex = 0;

        int16_t tileAddress = 0;

        // the pointer to the tile signed
        if (m_lcdcStatus.get().bgWindowTileDataPointer() != 0)
        {
            int8_t tileMapPointer = m_vram[m_lcdcStatus.get().windowTileMapPointer() + tileIndex];
            tileAddress = m_lcdcStatus.get().bgWindowTileDataPointer() + ((tileMapPointer + 128) * 16);
        }
        else
        {   
            //Multiply by 16 since the tile is 16 bytes long and we need to space the index corretly
            tileAddress = m_vram[m_lcdcStatus.get().windowTileMapPointer() + tileIndex] * 16;
        }
        
        //Two bytes that represent the current row we are drawing, 2bpp
        uint8_t lowByte = m_vram[tileAddress + tileRowIndex];
        uint8_t highByte = m_vram[tileAddress + tileRowIndex + 1];

        //the actually current Pixel in the row is tilePixelIndex, inverted so we can shift the bits into the pixel easily
        int invertedTilePixelIndex = 7 - tilePixelIndex;

        uint8_t pixel = 0;
        pixel |= ((lowByte >> invertedTilePixelIndex) & 0x1);
        pixel |= (((highByte >> invertedTilePixelIndex) & 0x1) << 1);

        switch(pixel)
        {
            case 0x0: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0xFF; break;
            case 0x1: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0xCC; break;
            case 0x2: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0x77; break;
            case 0x3: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0x00; break;
        }

        tilePixelIndex++;
    }
}

void PictureProcessingUnit::drawSprites(uint8_t line)
{

    for (int i = 0; i < m_spritesInLine; i++)
    {
        int tileRowIndex = line - m_sprites[i].yPosition;
        
        int tileIndex = 0;

        tileRowIndex *= 2;
        //Multiply by 16 since the tile is 16 bytes long and we need to space the index corretly
        int tilePointer = m_sprites[i].tilePointer * 16;
        
        //Two bytes that represent the current row we are drawing, 2bpp
        uint8_t lowByte = m_vram[tilePointer + tileRowIndex];
        uint8_t highByte = m_vram[tilePointer + tileRowIndex + 1];

        //Draw sprite row
        for (int tileRowPixelIndex = 0; tileRowPixelIndex < 8; ++tileRowPixelIndex)
        {
            //the actually current Pixel in the row is tilePixelIndex, inverted so we can shift the bits into the pixel easily
            int invertedTilePixelIndex = m_sprites[i].isHorizontalFlipped()? tileRowPixelIndex : (7 - (tileRowPixelIndex % 8));

            uint8_t pixel = 0;
            pixel |= ((lowByte >> invertedTilePixelIndex) & 0x1);
            pixel |= (((highByte >> invertedTilePixelIndex) & 0x1) << 1);

            int framePixelIndex = m_sprites[i].xPosition + tileRowPixelIndex;

            switch(pixel)
            {
                case 0x0: break;
                case 0x1: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0xCC; break;
                case 0x2: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0x77; break;
                case 0x3: screenData[line][framePixelIndex][0] = screenData[line][framePixelIndex][1] = screenData[line][framePixelIndex][2] = 0x00; break;
                default : assert(false);
            }
        }    
    }
}

void PictureProcessingUnit::searchSprites(uint8_t line)
{
    m_spritesInLine = 0;

    for (int yPosIndex = 0; yPosIndex < OAM_SIZE; yPosIndex += 4)
    {
        int spritePositionY = m_oam[yPosIndex];
        int spritePositionX = m_oam[yPosIndex + 1];
        
        spritePositionY -= 16;
        spritePositionX -= 8;
        
        if (spritePositionY < 0 || spritePositionX < 0)
            continue;

        if (line >= spritePositionY && line < (spritePositionY + m_lcdcStatus.get().spriteSize()))
        {
            if (m_spritesInLine == 10)
                break;

            m_sprites[m_spritesInLine].yPosition = m_oam[yPosIndex];
            m_sprites[m_spritesInLine].xPosition = m_oam[yPosIndex + 1];
            m_sprites[m_spritesInLine].tilePointer = m_oam[yPosIndex + 2];
            m_sprites[m_spritesInLine].attributes = m_oam[yPosIndex + 3];
            m_sprites[m_spritesInLine].yPosition -= 16;
            m_sprites[m_spritesInLine].xPosition -= 8;
            
            m_spritesInLine++;
            
        }
    }
}

