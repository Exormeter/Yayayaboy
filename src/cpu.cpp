#include "../include/cpu.hpp"


uint8_t Cpu::step()
{
    // printf("A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X SP: %04X PC: 00:%04X (%02X %02X %02X %02X)\n", 
    //         gpRegister.registerA, gpRegister.registerF, gpRegister.registerB, gpRegister.registerC, gpRegister.registerD, gpRegister.registerE, gpRegister.registerH, gpRegister.registerL,
    //         stackPointer, programmCounter,
    //         m_memoryMap->readMemoryBus(programmCounter), m_memoryMap->readMemoryBus(programmCounter + 1), m_memoryMap->readMemoryBus(programmCounter + 2), m_memoryMap->readMemoryBus(programmCounter + 3));
    if (m_interruptController->shouldWakeupFronHalt()) m_isHalted = false;
    if (m_isHalted) return 1;

    fetch();
    decode();
    execute();
    if (m_interruptController->hasPendingInterrupt())
    {
        m_interruptController->disableInterrupts();
        call(m_interruptController->pendingInterruptAddress());
        currentInstruction.cycles+=20;
    }
    return currentInstruction.cycles;
}

void Cpu::fetch()
{
    currentOpCode = m_memoryMap->readMemoryBus(programmCounter);

    if (currentOpCode == 0xCB)
    {
        currentOpCode = currentOpCode << 8;
        currentOpCode |= (m_memoryMap->readMemoryBus(programmCounter + 1));
    }
}

void Cpu::decode()
{
    currentInstruction = Instructions::getInstruction(currentOpCode);
    
    //All 0xCB OpCode are 2 bytes, but they never have an operant assigned to them, so
    //we can just interpret the scound byte of a 0xCB instruction as an operant without
    //bad things happening
    switch(currentInstruction.length)
    {
        //Only OpCode, no operant so decode is done at this point
        case 1:
            break;

        //8 bit operant
        case 2:
            currentInstruction.operant = m_memoryMap->readMemoryBus(programmCounter + 1);
            break;

        //16 bit operant, little endian
        case 3:
        {
            currentInstruction.operant |= m_memoryMap->readMemoryBus(programmCounter + 2);
            currentInstruction.operant <<= 8;
            currentInstruction.operant |= m_memoryMap->readMemoryBus(programmCounter + 1);
            break;
        }
            
        default:
            assert(false);
    }
}

void Cpu::execute()
{
    programmCounter += currentInstruction.length;
    
    //Don't worry, I generated most of this switch with python
    switch(currentInstruction.operation)
    {
        case 0x00:
            break;
        
        case 0x01:
            gpRegister.registerBC(currentInstruction.operant);
            break;

        case 0x02:
            m_memoryMap->writeMemoryBus(gpRegister.registerBC(), gpRegister.registerA);
            break;
        
        case 0x03:
        {
            uint16_t value = gpRegister.registerBC();
            gpRegister.registerBC(++value);
            break;
        }

        case 0x04:
            increment8Bit(gpRegister.registerB);
            break;

        case 0x5:
            decrement8Bit(gpRegister.registerB);
            break;

        case 0x06:
            gpRegister.registerB = currentInstruction.operant;
            break;

        case 0x07:
            rotateLeft(gpRegister.registerA);
            break;

        //Timing, this is a two byte write
        case 0x08:
            m_memoryMap->writeMemoryBus(currentInstruction.operant, (uint8_t) stackPointer);
            m_memoryMap->writeMemoryBus(currentInstruction.operant + 1, (uint8_t) (stackPointer >> 8));
            break;

        case 0x09:
            add16Bit(gpRegister.registerBC());
            break;

        case 0x0A:
            gpRegister.registerA = m_memoryMap->readMemoryBus(gpRegister.registerBC());
            break;
        
        case 0x0B:
        {
            uint16_t value = gpRegister.registerBC();
            gpRegister.registerBC(--value);
            break;
        }

        case 0x0C:
            increment8Bit(gpRegister.registerC);
            break;

        case 0x0D:
            decrement8Bit(gpRegister.registerC);
            break;

        case 0x0E:
            gpRegister.registerC = currentInstruction.operant;
            break;
        
        case 0x0F:
            rotateRight(gpRegister.registerA);
            break;

        case 0x10:
            break;

        case 0x11:
            gpRegister.registerDE(currentInstruction.operant);
            break;

        case 0x12:
            m_memoryMap->writeMemoryBus(gpRegister.registerDE(), gpRegister.registerA);
            break;

        case 0x13:
        {
            uint16_t value = gpRegister.registerDE();
            gpRegister.registerDE(++value);
            break;
        }

        case 0x14:
            increment8Bit(gpRegister.registerD);
            break;

        case 0x15:
            decrement8Bit(gpRegister.registerD);
            break;

        case 0x16:
            gpRegister.registerD = currentInstruction.operant;
            break;

        case 0x17:
            rotateLeftThroughCarry(gpRegister.registerA);
            break;

        case 0x18:
            programmCounter += static_cast<int8_t>(currentInstruction.operant);
            break;

        case 0x19:
            add16Bit(gpRegister.registerDE());
            break;

        case 0x1A:
            gpRegister.registerA = m_memoryMap->readMemoryBus(gpRegister.registerDE());
            break;

        case 0x1B:
        {
            uint16_t value = gpRegister.registerDE();
            gpRegister.registerDE(--value);
            break;
        }

        case 0x1C:
            increment8Bit(gpRegister.registerE);
            break;

        case 0x1D:  
            decrement8Bit(gpRegister.registerE);
            break;

        case 0x1E:
            gpRegister.registerE = currentInstruction.operant;
            break;

        case 0x1F:
            rotateRightThroughCarry(gpRegister.registerA);
            break;

        case 0x20:
            if (!statusRegister.isZeroFlagSet())
            {

                programmCounter += static_cast<int8_t>(currentInstruction.operant);
            }
            break;

        case 0x21:
            gpRegister.registerHL(currentInstruction.operant);
            break;

        case 0x22:
        {
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), gpRegister.registerA);
            uint16_t value = gpRegister.registerHL();
            gpRegister.registerHL(++value);
            break;
        }

        case 0x23:
        {
            uint16_t value = gpRegister.registerHL();
            gpRegister.registerHL(++value);
            break;
        }

        case 0x24:
            increment8Bit(gpRegister.registerH);
            break;

        case 0x25:
            decrement8Bit(gpRegister.registerH);
            break;

        case 0x26:
            gpRegister.registerH = currentInstruction.operant;
            break;
        
        case 0x27:
            if (!statusRegister.isNegativFlagSet()) 
            {  

                if (statusRegister.isCarryFlagSet() || gpRegister.registerA > 0x99) 
                { 
                    gpRegister.registerA += 0x60;
                    statusRegister.setCarryFlag();
                }

                if (statusRegister.isHalfCarryFlagSet() || (gpRegister.registerA & 0x0f) > 0x09) 
                { 
                    gpRegister.registerA += 0x6; 
                }

            } 
            else
            {

                if (statusRegister.isCarryFlagSet()) 
                { 
                    gpRegister.registerA -= 0x60;
                    statusRegister.setCarryFlag(); 
                }

                if (statusRegister.isHalfCarryFlagSet()) 
                { 
                    gpRegister.registerA -= 0x6; 
                }
            }
            statusRegister.checkZeroFlag(gpRegister.registerA);
            statusRegister.resetHalfCarryFlag();
            break;

        case 0x28:
            if (statusRegister.isZeroFlagSet())
            {
                programmCounter += static_cast<int8_t>(currentInstruction.operant);
            }
            break;

        case 0x29:
            add16Bit(gpRegister.registerHL());
            break;

        case 0x2A:
        {
            gpRegister.registerA = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            uint16_t value = gpRegister.registerHL();
            gpRegister.registerHL(++value);
            break;
        }

        case 0x2B:
        {
            uint16_t value = gpRegister.registerHL();
            gpRegister.registerHL(--value);
            break;
        }

        case 0x2C:  
            increment8Bit(gpRegister.registerL);
            break;

        case 0x2D:
            decrement8Bit(gpRegister.registerL);
            break;

        case 0x2E:
            gpRegister.registerL = currentInstruction.operant;
            break;

        case 0x2F:
            statusRegister.setNegativFlag();
            statusRegister.setHalfCarryFlag();
            gpRegister.registerA = ~gpRegister.registerA;
            break;

        case 0x30:
            if (!statusRegister.isCarryFlagSet())
            {
                programmCounter += static_cast<int8_t>(currentInstruction.operant);
            }
            break;

        case 0x31:
            stackPointer = currentInstruction.operant;
            break;

        case 0x36:
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), currentInstruction.operant);
            break;

        case 0x32:
        {
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), gpRegister.registerA);
            uint16_t value = gpRegister.registerHL();
            gpRegister.registerHL(--value);
            break;
        }
        
        case 0x33:
            stackPointer++;
            break;

        case 0x34:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            increment8Bit(value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0x35:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            decrement8Bit(value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0x37:
            statusRegister.resetNegativFlag();
            statusRegister.resetHalfCarryFlag();
            statusRegister.setCarryFlag();
            break;
        
        case 0x38:
            if (statusRegister.isCarryFlagSet())
            {
                programmCounter += static_cast<int8_t>(currentInstruction.operant);
            }
            break;

        case 0x39:
            add16Bit(stackPointer);
            break;

        case 0x3A:
        {
            gpRegister.registerA = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            uint16_t value = gpRegister.registerHL();
            gpRegister.registerHL(--value);
            break;
        }

        case 0x3B:
            stackPointer--;
            break;

        case 0x3C:
            increment8Bit(gpRegister.registerA);
            break;

        case 0x3D:
            decrement8Bit(gpRegister.registerA);
            break;

        case 0x3E:
            gpRegister.registerA = currentInstruction.operant;
            break;

        case 0x3F:
            statusRegister.resetNegativFlag();
            statusRegister.resetHalfCarryFlag();
            statusRegister.isCarryFlagSet() ? statusRegister.resertCarryFlag() : statusRegister.setCarryFlag();
            break;

        case 0x40:
            gpRegister.registerB = gpRegister.registerB;
            break;
        
        case 0x41:
            gpRegister.registerB = gpRegister.registerC;
            break;

        case 0x42:
            gpRegister.registerB = gpRegister.registerD;
            break;
        
        case 0x43:
            gpRegister.registerB = gpRegister.registerE;
            break;

        case 0x44:
            gpRegister.registerB = gpRegister.registerH;
            break;

        case 0x45:
            gpRegister.registerB = gpRegister.registerL;
            break;

        case 0x46:
            gpRegister.registerB = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            break;

        case 0x47:
            gpRegister.registerB = gpRegister.registerA;
            break;

        case 0x48:
            gpRegister.registerC = gpRegister.registerB;
            break;

        case 0x49:
            gpRegister.registerC = gpRegister.registerC;
            break;

        case 0x4A:
            gpRegister.registerC = gpRegister.registerD;
            break;

        case 0x4B:
            gpRegister.registerC = gpRegister.registerE;
            break;

        case 0x4C:
            gpRegister.registerC = gpRegister.registerH;
            break;
    
        case 0x4D:
            gpRegister.registerC = gpRegister.registerL;
            break;
        
        case 0x4E:
            gpRegister.registerC = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            break;

        case 0x4F:
            gpRegister.registerC = gpRegister.registerA;
            break;

        case 0x50:
            gpRegister.registerD = gpRegister.registerB;
            break;

        case 0x51:
            gpRegister.registerD = gpRegister.registerC;
            break;

        case 0x52:
            gpRegister.registerD = gpRegister.registerD;
            break;

        case 0x53:
            gpRegister.registerD = gpRegister.registerE;
            break;

        case 0x54:
            gpRegister.registerD = gpRegister.registerH;
            break;

        case 0x55:
            gpRegister.registerD = gpRegister.registerL;
            break;

        case 0x56:
            gpRegister.registerD = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            break;

        case 0x57:
            gpRegister.registerD = gpRegister.registerA;
            break;

        case 0x58:
            gpRegister.registerE = gpRegister.registerB;
            break;

        case 0x59:
            gpRegister.registerE = gpRegister.registerC;
            break;
        
        case 0x5A:
            gpRegister.registerE = gpRegister.registerD;
            break;

        case 0x5B:
            gpRegister.registerE = gpRegister.registerE;
            break;

        case 0x5C:
            gpRegister.registerE = gpRegister.registerH;
            break;

        case 0x5D:
            gpRegister.registerE = gpRegister.registerL;
            break;

        case 0x5E:
            gpRegister.registerE = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            break;

        case 0x5F:
            gpRegister.registerE = gpRegister.registerA;
            break;

        case 0x60:
            gpRegister.registerH = gpRegister.registerB;
            break;

        case 0x61:
            gpRegister.registerH = gpRegister.registerC;
            break;
            
        case 0x62:
            gpRegister.registerH = gpRegister.registerD;
            break;

        case 0x63:
            gpRegister.registerH = gpRegister.registerE;
            break;

        case 0x64:
            gpRegister.registerH = gpRegister.registerH;
            break;
        
        case 0x65:
            gpRegister.registerH = gpRegister.registerL;
            break;

        case 0x66:
            gpRegister.registerH = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            break;

        case 0x67:
            gpRegister.registerH = gpRegister.registerA;
            break;

        case 0x68:
            gpRegister.registerL = gpRegister.registerB;
            break;

        case 0x69:
            gpRegister.registerL = gpRegister.registerC;
            break;
            
        case 0x6A:
            gpRegister.registerL = gpRegister.registerD;
            break;

        case 0x6B:
            gpRegister.registerL = gpRegister.registerE;
            break;

        case 0x6C:
            gpRegister.registerL = gpRegister.registerH;
            break;
        
        case 0x6D:
            gpRegister.registerL = gpRegister.registerL;
            break;

        case 0x6E:
            gpRegister.registerL = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            break;

        case 0x6F:
            gpRegister.registerL = gpRegister.registerA;
            break;

        case 0x70:
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), gpRegister.registerB);
            break;

        case 0x71:
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), gpRegister.registerC);
            break;

        case 0x72:
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), gpRegister.registerD);
            break;

        case 0x73:
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), gpRegister.registerE);
            break;

        case 0x74:
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), gpRegister.registerH);
            break;

        case 0x75:
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), gpRegister.registerL);
            break;

        case 0x76:
            m_isHalted = true;
            break;

        case 0x77:
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), gpRegister.registerA);
            break;

        case 0x78: 
            gpRegister.registerA = gpRegister.registerB;
            break;

        case 0x79:
            gpRegister.registerA = gpRegister.registerC;
            break;

        case 0x7A:
            gpRegister.registerA = gpRegister.registerD;
            break;

        case 0x7B:
            gpRegister.registerA = gpRegister.registerE;
            break;

        case 0x7C:
            gpRegister.registerA = gpRegister.registerH;
            break;

        case 0x7D:
            gpRegister.registerA = gpRegister.registerL;
            break;

        case 0x7E:
            gpRegister.registerA = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            break;

        case 0x7F:
            gpRegister.registerA = gpRegister.registerA;
            break;
        
        case 0x80:
            add8Bit(gpRegister.registerB);
            break;

        case 0x81:
            add8Bit(gpRegister.registerC);
            break;
        
        case 0x82:
            add8Bit(gpRegister.registerD);
            break;

        case 0x83:
            add8Bit(gpRegister.registerE);
            break;

        case 0x84:
            add8Bit(gpRegister.registerH);
            break;

        case 0x85:
            add8Bit(gpRegister.registerL);
            break;

        case 0x86:
        {
            uint8_t registerHLImmidate = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            add8Bit(registerHLImmidate);
            break;
        }

        case 0x87:
            add8Bit(gpRegister.registerA);
            break;

        case 0x88:
            addCarry8Bit(gpRegister.registerB);
            break;

        case 0x89:
            addCarry8Bit(gpRegister.registerC);
            break;

        case 0x8A:
            addCarry8Bit(gpRegister.registerD);
            break;

        case 0x8B:
            addCarry8Bit(gpRegister.registerE);
            break;

        case 0x8C:
            addCarry8Bit(gpRegister.registerH);
            break;

        case 0x8D:
            addCarry8Bit(gpRegister.registerL);
            break;
        
        case 0x8E:
            addCarry8Bit(m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0x8F:  
            addCarry8Bit(gpRegister.registerA);
            break;

        case 0x90:
            sub8Bit(gpRegister.registerB);
            break;

        case 0x91:
            sub8Bit(gpRegister.registerC);
            break;

        case 0x92:
            sub8Bit(gpRegister.registerD);
            break;

        case 0x93:
            sub8Bit(gpRegister.registerE);
            break;

        case 0x94:
            sub8Bit(gpRegister.registerH);
            break;

        case 0x95:
            sub8Bit(gpRegister.registerL);
            break;

        case 0x96:
            sub8Bit(m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0x97:
            sub8Bit(gpRegister.registerA);
            break;

        case 0x98:
            subCarry8Bit(gpRegister.registerB);
            break;

        case 0x99:
            subCarry8Bit(gpRegister.registerC);
            break;

        case 0x9A:
            subCarry8Bit(gpRegister.registerD);
            break;
        
        case 0x9B:
            subCarry8Bit(gpRegister.registerE);
            break;

        case 0x9C:
            subCarry8Bit(gpRegister.registerH);
            break;
        
        case 0x9D:
            subCarry8Bit(gpRegister.registerL);
            break;

        case 0x9E:
            subCarry8Bit(m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0x9F:
            subCarry8Bit(gpRegister.registerA);
            break;

        case 0xA0:
            and8Bit(gpRegister.registerB);
            break;

        case 0xA1:
            and8Bit(gpRegister.registerC);
            break;

        case 0xA2:
            and8Bit(gpRegister.registerD);
            break;

        case 0xA3:
            and8Bit(gpRegister.registerE);
            break;

        case 0xA4:
            and8Bit(gpRegister.registerH);
            break;

        case 0xA5:
            and8Bit(gpRegister.registerL);
            break;

        case 0xA6:
            and8Bit(m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xA7:
            and8Bit(gpRegister.registerA);
            break;

        case 0xA8:
            xor8Bit(gpRegister.registerB);
            break;

        case 0xA9:
            xor8Bit(gpRegister.registerC);
            break;
        
        case 0xAA:
            xor8Bit(gpRegister.registerD);
            break;
        
        case 0xAB:
            xor8Bit(gpRegister.registerE);
            break;

        case 0xAC:
            xor8Bit(gpRegister.registerH);
            break;

        case 0xAD:
            xor8Bit(gpRegister.registerL);
            break;

        case 0xAE:
            xor8Bit(m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xAF:
            xor8Bit(gpRegister.registerA);
            break;

        case 0xB0:
            or8Bit(gpRegister.registerB);
            break;

        case 0xB1:  
            or8Bit(gpRegister.registerC);
            break;

        case 0xB2:
            or8Bit(gpRegister.registerD);
            break;

        case 0xB3:
            or8Bit(gpRegister.registerE);
            break;

        case 0xB4:
            or8Bit(gpRegister.registerH);
            break;

        case 0xB5:
            or8Bit(gpRegister.registerL);
            break;

        case 0xB6:
            or8Bit(m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xB7:
            or8Bit(gpRegister.registerA);
            break;

        case 0xB8:
            compare8Bit(gpRegister.registerB);
            break;

        case 0xB9:
            compare8Bit(gpRegister.registerC);
            break;

        case 0xBA:
            compare8Bit(gpRegister.registerD);
            break;

        case 0xBB:
            compare8Bit(gpRegister.registerE);
            break;

        case 0xBC:
            compare8Bit(gpRegister.registerH);
            break;

        case 0xBD:
            compare8Bit(gpRegister.registerL);
            break;

        case 0xBE:
            compare8Bit(m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xBF:
            compare8Bit(gpRegister.registerA);
            break;

        case 0xC0:
            if (!statusRegister.isZeroFlagSet())
            {
                funcReturn();
            }
            break;

        case 0xC1:
            gpRegister.registerC = m_memoryMap->readMemoryBus(stackPointer);
            stackPointer++;
            gpRegister.registerB  = m_memoryMap->readMemoryBus(stackPointer);
            stackPointer++;
            
            break;

        case 0xC2:
            if (!statusRegister.isZeroFlagSet())
            {
                programmCounter = currentInstruction.operant;
            }
            break;

        case 0xC3:
            programmCounter = currentInstruction.operant;
            break;

        case 0xC4:
            if (!statusRegister.isZeroFlagSet())
            {
                call(currentInstruction.operant);
            }
            break;

        case 0xC5:
            stackPointer--;
            m_memoryMap->writeMemoryBus(stackPointer, gpRegister.registerB);
            stackPointer--;
            m_memoryMap->writeMemoryBus(stackPointer, gpRegister.registerC);
            break;

        case 0xC6:
            add8Bit(currentInstruction.operant);
            break;

        case 0xC7:
            reset(0x00);
            break;

        case 0xC8:
            if (statusRegister.isZeroFlagSet())
            {
                funcReturn();
            }
            break;

        case 0xC9:
            funcReturn();
            break;

        case 0xCA:
            if (statusRegister.isZeroFlagSet())
            {
                programmCounter = currentInstruction.operant;
            }
            break;

        case 0xCC:
            if (statusRegister.isZeroFlagSet())
            {
                call(currentInstruction.operant);
            }
            break;
            
        case 0xCD:
            call(currentInstruction.operant);
            break;

        case 0xCE:
            addCarry8Bit(currentInstruction.operant);
            break;

        case 0xCF:
            reset(0x08);
            break;

        case 0xD0:
            if (!statusRegister.isCarryFlagSet())
            {
                funcReturn();
            }
            break;

        case 0xD1:
            gpRegister.registerE = m_memoryMap->readMemoryBus(stackPointer);
            stackPointer++;
            gpRegister.registerD  = m_memoryMap->readMemoryBus(stackPointer);
            stackPointer++;
            break;

        case 0xD2:
            if (!statusRegister.isCarryFlagSet())
            {
                programmCounter = currentInstruction.operant;
            }
            break;

        case 0xD4:
            if (!statusRegister.isCarryFlagSet())
            {
                call(currentInstruction.operant);
            }
            break;

        case 0xD5:
            stackPointer--;
            m_memoryMap->writeMemoryBus(stackPointer, gpRegister.registerD);
            stackPointer--;
            m_memoryMap->writeMemoryBus(stackPointer, gpRegister.registerE);
            break;

        case 0xD6:
            sub8Bit(currentInstruction.operant);
            break;

        case 0xD7:
            reset(0x10);
            break;
        
        case 0xD8:
            if (statusRegister.isCarryFlagSet())
            {
                funcReturn();
            }
            break;

        case 0xD9:
            funcReturn();
            m_interruptController->enableInterrupts();
            break;

        case 0xDA:
            if (statusRegister.isCarryFlagSet())
            {
                programmCounter = currentInstruction.operant;
            }
            break;

        case 0xDC:
            if (statusRegister.isCarryFlagSet())
            {
                call(currentInstruction.operant);
            }
            break;

        case 0xDE:
            subCarry8Bit(currentInstruction.operant);
            break;

        case 0xDF:
            reset(0x18);
            break;

        case 0xE0:
            m_memoryMap->writeMemoryBus(0xFF00 + currentInstruction.operant, gpRegister.registerA);
            break;

        case 0xE1:
            gpRegister.registerL = m_memoryMap->readMemoryBus(stackPointer);
            stackPointer++;
            gpRegister.registerH  = m_memoryMap->readMemoryBus(stackPointer);
            stackPointer++;
            break;

        case 0xE2:
            m_memoryMap->writeMemoryBus(0xFF00 + gpRegister.registerC, gpRegister.registerA);
            break;

        case 0xE5:
            stackPointer--;
            m_memoryMap->writeMemoryBus(stackPointer, gpRegister.registerH);
            stackPointer--;
            m_memoryMap->writeMemoryBus(stackPointer, gpRegister.registerL);
            break;

        case 0xE6:
            and8Bit(currentInstruction.operant);
            break;

        case 0xE7:
            reset(0x20);
            break;

        case 0xE8:
            {
                int8_t operant = static_cast<int8_t>(currentInstruction.operant);
                statusRegister.resetNegativFlag();
                statusRegister.resetZeroFlag();
                statusRegister.checkCarryFlag8BitAdd(uint8_t (stackPointer), currentInstruction.operant);
                statusRegister.checkHalfCarryFlag8BitAdd(uint8_t (stackPointer), currentInstruction.operant);
                

                stackPointer += operant;
            }
            break;

        case 0xE9:
            programmCounter = gpRegister.registerHL();
            break;

        case 0xEA:
            m_memoryMap->writeMemoryBus(currentInstruction.operant, gpRegister.registerA);
            break;

        case 0xEE:
            xor8Bit(currentInstruction.operant);
            break;  
        
        case 0xEF:
            reset(0x28);
            break;

        case 0xF0:
            gpRegister.registerA = m_memoryMap->readMemoryBus(0xFF00 + currentInstruction.operant);
            break;

        case 0xF1:
            gpRegister.registerF = m_memoryMap->readMemoryBus(stackPointer);
            gpRegister.registerF &= 0xF0;
            stackPointer++;
            gpRegister.registerA  = m_memoryMap->readMemoryBus(stackPointer);
            stackPointer++;
            break;

        case 0xF2:
            gpRegister.registerA = m_memoryMap->readMemoryBus(0xFF00 + gpRegister.registerC);
            break;

        case 0xF3:
            m_interruptController->disableInterrupts();
            break;

        //Timing, this is two byte write
        case 0xF5:
            stackPointer--;
            m_memoryMap->writeMemoryBus(stackPointer, gpRegister.registerA);
            stackPointer--;
            m_memoryMap->writeMemoryBus(stackPointer, gpRegister.registerF);
            break;

        case 0xF6:
            or8Bit(currentInstruction.operant);
            break;

        case 0xF7:
            reset(0x30);
            break;

        case 0xF8:
            statusRegister.checkCarryFlag8BitAdd((uint8_t)stackPointer, currentInstruction.operant);
            statusRegister.checkHalfCarryFlag8BitAdd((uint8_t)stackPointer, currentInstruction.operant);
            statusRegister.resetZeroFlag();
            statusRegister.resetNegativFlag();
            gpRegister.registerHL(stackPointer + static_cast<int8_t>(currentInstruction.operant));
            break;

        case 0xF9:
            stackPointer = gpRegister.registerHL();
            break;

        case 0xFA:
            gpRegister.registerA = m_memoryMap->readMemoryBus(currentInstruction.operant);
            break;

        case 0xFB:
            m_interruptController->enableInterrupts();
            break;

        case 0xFE:
            compare8Bit(currentInstruction.operant);
            break;

        case 0xFF:
            reset(0x38);
            break;

        case 0xCB00:
            rotateLeftCB(gpRegister.registerB);
            break;

        case 0xCB01:
            rotateLeftCB(gpRegister.registerC);
            break;
        
        case 0xCB02:
            rotateLeftCB(gpRegister.registerD);
            break;
        
        case 0xCB03:
            rotateLeftCB(gpRegister.registerE);
            break;

        case 0xCB04:
            rotateLeftCB(gpRegister.registerH);
            break;

        case 0xCB05:
            rotateLeftCB(gpRegister.registerL);
            break;

        case 0xCB06:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            rotateLeftCB(value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCB07:
            rotateLeftCB(gpRegister.registerA);
            break;

        case 0xCB08:
            rotateRightCB(gpRegister.registerB);
            break;

        case 0xCB09:
            rotateRightCB(gpRegister.registerC);
            break;

        case 0xCB0A:
            rotateRightCB(gpRegister.registerD);
            break;

        case 0xCB0B:
            rotateRightCB(gpRegister.registerE);
            break;
        
        case 0xCB0C:
            rotateRightCB(gpRegister.registerH);
            break;

        case 0xCB0D:
            rotateRightCB(gpRegister.registerL);
            break;

        case 0xCB0E:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            rotateRightCB(value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }   

        case 0xCB0F:
            rotateRightCB (gpRegister.registerA);
            break;

        case 0xCB10:
            rotateLeftThroughtCarryCB(gpRegister.registerB);
            break;

        case 0xCB11:
            rotateLeftThroughtCarryCB(gpRegister.registerC);
            break;

        case 0xCB12:
            rotateLeftThroughtCarryCB(gpRegister.registerD);
            break;

        case 0xCB13:
            rotateLeftThroughtCarryCB(gpRegister.registerE);
            break;

        case 0xCB14:
            rotateLeftThroughtCarryCB(gpRegister.registerH);
            break;

        case 0xCB15:
            rotateLeftThroughtCarryCB(gpRegister.registerL);
            break;

        case 0xCB16:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            rotateLeftThroughtCarryCB(value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCB17:
            rotateLeftThroughtCarryCB(gpRegister.registerA);
            break;

        case 0xCB18:
            rotateRightThroughCarryCB(gpRegister.registerB);
            break;

        case 0xCB19:
            rotateRightThroughCarryCB(gpRegister.registerC);
            break;

        case 0xCB1A:
            rotateRightThroughCarryCB(gpRegister.registerD);
            break;

        case 0xCB1B:
            rotateRightThroughCarryCB(gpRegister.registerE);
            break;

        case 0xCB1C:
            rotateRightThroughCarryCB(gpRegister.registerH);
            break;

        case 0xCB1D:
            rotateRightThroughCarryCB(gpRegister.registerL);
            break;

        case 0xCB1E:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            rotateRightThroughCarryCB(value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCB1F:
            rotateRightThroughCarryCB(gpRegister.registerA);
            break;

        case 0xCB20:
            shiftLeft(gpRegister.registerB);
            break;

        case 0xCB21:
            shiftLeft(gpRegister.registerC);
            break;

        case 0xCB22:
            shiftLeft(gpRegister.registerD);
            break;

        case 0xCB23:
            shiftLeft(gpRegister.registerE);
            break;

        case 0xCB24:
            shiftLeft(gpRegister.registerH);
            break;

        case 0xCB25:
            shiftLeft(gpRegister.registerL);
            break;

        case 0xCB26:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            shiftLeft(value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCB27:
            shiftLeft(gpRegister.registerA);
            break;

        case 0xCB28:
            shiftRightKeepMSB(gpRegister.registerB);
            break;

        case 0xCB29:
            shiftRightKeepMSB(gpRegister.registerC);
            break;

        case 0xCB2A:
            shiftRightKeepMSB(gpRegister.registerD);
            break;

        case 0xCB2B:
            shiftRightKeepMSB(gpRegister.registerE);
            break;

        case 0xCB2C:
            shiftRightKeepMSB(gpRegister.registerH);
            break;

        case 0xCB2D:
            shiftRightKeepMSB(gpRegister.registerL);
            break;

        case 0xCB2E:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            shiftRightKeepMSB(value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCB2F:
            shiftRightKeepMSB(gpRegister.registerA);
            break;


        case 0xCB30:
            swap(gpRegister.registerB);
            break;

        case 0xCB31:
            swap(gpRegister.registerC);
            break;

        case 0xCB32:
            swap(gpRegister.registerD);
            break;

        case 0xCB33:
            swap(gpRegister.registerE);
            break;

        case 0xCB34:
            swap(gpRegister.registerH);
            break;

        case 0xCB35:
            swap(gpRegister.registerL);
            break;

        case 0xCB36:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            swap(value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }
            
        case 0xCB37:
            swap(gpRegister.registerA);
            break;

        case 0xCB38:
            shiftRight(gpRegister.registerB);
            break;

        case 0xCB39:
            shiftRight(gpRegister.registerC);
            break;

        case 0xCB3A:
            shiftRight(gpRegister.registerD);
            break;

        case 0xCB3B:
            shiftRight(gpRegister.registerE);
            break;

        case 0xCB3C:
            shiftRight(gpRegister.registerH);
            break;

        case 0xCB3D:
            shiftRight(gpRegister.registerL);
            break;

        case 0xCB3E:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            shiftRight(value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCB3F:
            shiftRight(gpRegister.registerA);
            break;

        case 0xCB40:
            testBit(0, gpRegister.registerB);
            break;

        case 0xCB41:
            testBit(0, gpRegister.registerC);
            break;

        case 0xCB42:
            testBit(0, gpRegister.registerD);
            break;

        case 0xCB43:
            testBit(0, gpRegister.registerE);
            break;

        case 0xCB44:
            testBit(0, gpRegister.registerH);
            break;

        case 0xCB45:
            testBit(0, gpRegister.registerL);
            break;

        case 0xCB46:
            testBit(0, m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xCB47:
            testBit(0, gpRegister.registerA);
            break;

        case 0xCB48:
            testBit(1, gpRegister.registerB);
            break;

        case 0xCB49:
            testBit(1, gpRegister.registerC);
            break;

        case 0xCB4A:
            testBit(1, gpRegister.registerD);
            break;

        case 0xCB4B:
            testBit(1, gpRegister.registerE);
            break;

        case 0xCB4C:
            testBit(1, gpRegister.registerH);
            break;

        case 0xCB4D:
            testBit(1, gpRegister.registerL);
            break;

        case 0xCB4E:
            testBit(1, m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xCB4F:
            testBit(1, gpRegister.registerA);
            break;

        case 0xCB50:
            testBit(2, gpRegister.registerB);
            break;

        case 0xCB51:
            testBit(2, gpRegister.registerC);
            break;

        case 0xCB52:
            testBit(2, gpRegister.registerD);
            break;

        case 0xCB53:
            testBit(2, gpRegister.registerE);
            break;

        case 0xCB54:
            testBit(2, gpRegister.registerH);
            break;

        case 0xCB55:
            testBit(2, gpRegister.registerL);
            break;

        case 0xCB56:
            testBit(2, m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xCB57:
            testBit(2, gpRegister.registerA);
            break;

        case 0xCB58:
            testBit(3, gpRegister.registerB);
            break;

        case 0xCB59:
            testBit(3, gpRegister.registerC);
            break;

        case 0xCB5A:
            testBit(3, gpRegister.registerD);
            break;

        case 0xCB5B:
            testBit(3, gpRegister.registerE);
            break;

        case 0xCB5C:
            testBit(3, gpRegister.registerH);
            break;

        case 0xCB5D:
            testBit(3, gpRegister.registerL);
            break;

        case 0xCB5E:
            testBit(3, m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xCB5F:
            testBit(3, gpRegister.registerA);
            break;

        case 0xCB60:
            testBit(4, gpRegister.registerB);
            break;

        case 0xCB61:
            testBit(4, gpRegister.registerC);
            break;

        case 0xCB62:
            testBit(4, gpRegister.registerD);
            break;

        case 0xCB63:
            testBit(4, gpRegister.registerE);
            break;

        case 0xCB64:
            testBit(4, gpRegister.registerH);
            break;

        case 0xCB65:
            testBit(4, gpRegister.registerL);
            break;

        case 0xCB66:
            testBit(4, m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xCB67:
            testBit(4, gpRegister.registerA);
            break;

        case 0xCB68:
            testBit(5, gpRegister.registerB);
            break;

        case 0xCB69:
            testBit(5, gpRegister.registerC);
            break;

        case 0xCB6A:
            testBit(5, gpRegister.registerD);
            break;

        case 0xCB6B:
            testBit(5, gpRegister.registerE);
            break;

        case 0xCB6C:
            testBit(5, gpRegister.registerH);
            break;

        case 0xCB6D:
            testBit(5, gpRegister.registerL);
            break;

        case 0xCB6E:
            testBit(5, m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xCB6F:
            testBit(5, gpRegister.registerA);
            break;

        case 0xCB70:
            testBit(6, gpRegister.registerB);
            break;

        case 0xCB71:
            testBit(6, gpRegister.registerC);
            break;

        case 0xCB72:
            testBit(6, gpRegister.registerD);
            break;

        case 0xCB73:
            testBit(6, gpRegister.registerE);
            break;

        case 0xCB74:
            testBit(6, gpRegister.registerH);
            break;

        case 0xCB75:
            testBit(6, gpRegister.registerL);
            break;

        case 0xCB76:
            testBit(6, m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xCB77:
            testBit(6, gpRegister.registerA);
            break;

        case 0xCB78:
            testBit(7, gpRegister.registerB);
            break;

        case 0xCB79:
            testBit(7, gpRegister.registerC);
            break;

        case 0xCB7A:
            testBit(7, gpRegister.registerD);
            break;

        case 0xCB7B:
            testBit(7, gpRegister.registerE);
            break;

        case 0xCB7C:
            testBit(7, gpRegister.registerH);
            break;

        case 0xCB7D:
            testBit(7, gpRegister.registerL);
            break;

        case 0xCB7E:
            testBit(7, m_memoryMap->readMemoryBus(gpRegister.registerHL()));
            break;

        case 0xCB7F:
            testBit(7, gpRegister.registerA);
            break;

        case 0xCB80:
            resetBit(0, gpRegister.registerB);
            break;

        case 0xCB81:
            resetBit(0, gpRegister.registerC);
            break;

        case 0xCB82:
            resetBit(0, gpRegister.registerD);
            break;

        case 0xCB83:
            resetBit(0, gpRegister.registerE);
            break;

        case 0xCB84:
            resetBit(0, gpRegister.registerH);
            break;

        case 0xCB85:
            resetBit(0, gpRegister.registerL);
            break;

        case 0xCB86:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            resetBit(0, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCB87:
            resetBit(0, gpRegister.registerA);
            break;

        case 0xCB88:
            resetBit(1, gpRegister.registerB);
            break;

        case 0xCB89:
            resetBit(1, gpRegister.registerC);
            break;

        case 0xCB8A:
            resetBit(1, gpRegister.registerD);
            break;

        case 0xCB8B:
            resetBit(1, gpRegister.registerE);
            break;

        case 0xCB8C:
            resetBit(1, gpRegister.registerH);
            break;

        case 0xCB8D:
            resetBit(1, gpRegister.registerL);
            break;

        case 0xCB8E:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            resetBit(1, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCB8F:
            resetBit(1, gpRegister.registerA);
            break;

        case 0xCB90:
            resetBit(2, gpRegister.registerB);
            break;

        case 0xCB91:
            resetBit(2, gpRegister.registerC);
            break;

        case 0xCB92:
            resetBit(2, gpRegister.registerD);
            break;

        case 0xCB93:
            resetBit(2, gpRegister.registerE);
            break;

        case 0xCB94:
            resetBit(2, gpRegister.registerH);
            break;

        case 0xCB95:
            resetBit(2, gpRegister.registerL);
            break;

        case 0xCB96:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            resetBit(2, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCB97:
            resetBit(2, gpRegister.registerA);
            break;

        case 0xCB98:
            resetBit(3, gpRegister.registerB);
            break;

        case 0xCB99:
            resetBit(3, gpRegister.registerC);
            break;

        case 0xCB9A:
            resetBit(3, gpRegister.registerD);
            break;

        case 0xCB9B:
            resetBit(3, gpRegister.registerE);
            break;

        case 0xCB9C:
            resetBit(3, gpRegister.registerH);
            break;

        case 0xCB9D:
            resetBit(3, gpRegister.registerL);
            break;

        case 0xCB9E:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            resetBit(3, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCB9F:
            resetBit(3, gpRegister.registerA);
            break;

        case 0xCBA0:
            resetBit(4, gpRegister.registerB);
            break;

        case 0xCBA1:
            resetBit(4, gpRegister.registerC);
            break;

        case 0xCBA2:
            resetBit(4, gpRegister.registerD);
            break;

        case 0xCBA3:
            resetBit(4, gpRegister.registerE);
            break;

        case 0xCBA4:
            resetBit(4, gpRegister.registerH);
            break;

        case 0xCBA5:
            resetBit(4, gpRegister.registerL);
            break;

        case 0xCBA6:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            resetBit(4, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBA7:
            resetBit(4, gpRegister.registerA);
            break;

        case 0xCBA8:
            resetBit(5, gpRegister.registerB);
            break;

        case 0xCBA9:
            resetBit(5, gpRegister.registerC);
            break;

        case 0xCBAA:
            resetBit(5, gpRegister.registerD);
            break;

        case 0xCBAB:
            resetBit(5, gpRegister.registerE);
            break;

        case 0xCBAC:
            resetBit(5, gpRegister.registerH);
            break;

        case 0xCBAD:
            resetBit(5, gpRegister.registerL);
            break;

        case 0xCBAE:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            resetBit(5, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBAF:
            resetBit(5, gpRegister.registerA);
            break;

        case 0xCBB0:
            resetBit(6, gpRegister.registerB);
            break;

        case 0xCBB1:
            resetBit(6, gpRegister.registerC);
            break;

        case 0xCBB2:
            resetBit(6, gpRegister.registerD);
            break;

        case 0xCBB3:
            resetBit(6, gpRegister.registerE);
            break;

        case 0xCBB4:
            resetBit(6, gpRegister.registerH);
            break;

        case 0xCBB5:
            resetBit(6, gpRegister.registerL);
            break;

        case 0xCBB6:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            resetBit(6, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBB7:
            resetBit(6, gpRegister.registerA);
            break;

        case 0xCBB8:
            resetBit(7, gpRegister.registerB);
            break;

        case 0xCBB9:
            resetBit(7, gpRegister.registerC);
            break;

        case 0xCBBA:
            resetBit(7, gpRegister.registerD);
            break;

        case 0xCBBB:
            resetBit(7, gpRegister.registerE);
            break;

        case 0xCBBC:
            resetBit(7, gpRegister.registerH);
            break;

        case 0xCBBD:
            resetBit(7, gpRegister.registerL);
            break;

        case 0xCBBE:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            resetBit(7, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBBF:
            resetBit(7, gpRegister.registerA);
            break;

        case 0xCBC0:
            setBit(0, gpRegister.registerB);
            break;

        case 0xCBC1:
            setBit(0, gpRegister.registerC);
            break;

        case 0xCBC2:
            setBit(0, gpRegister.registerD);
            break;

        case 0xCBC3:
            setBit(0, gpRegister.registerE);
            break;

        case 0xCBC4:
            setBit(0, gpRegister.registerH);
            break;

        case 0xCBC5:
            setBit(0, gpRegister.registerL);
            break;

        case 0xCBC6:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            setBit(0, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBC7:
            setBit(0, gpRegister.registerA);
            break;

        case 0xCBC8:
            setBit(1, gpRegister.registerB);
            break;

        case 0xCBC9:
            setBit(1, gpRegister.registerC);
            break;

        case 0xCBCA:
            setBit(1, gpRegister.registerD);
            break;

        case 0xCBCB:
            setBit(1, gpRegister.registerE);
            break;

        case 0xCBCC:
            setBit(1, gpRegister.registerH);
            break;

        case 0xCBCD:
            setBit(1, gpRegister.registerL);
            break;

        case 0xCBCE:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            setBit(1, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBCF:
            setBit(1, gpRegister.registerA);
            break;

        case 0xCBD0:
            setBit(2, gpRegister.registerB);
            break;

        case 0xCBD1:
            setBit(2, gpRegister.registerC);
            break;

        case 0xCBD2:
            setBit(2, gpRegister.registerD);
            break;

        case 0xCBD3:
            setBit(2, gpRegister.registerE);
            break;

        case 0xCBD4:
            setBit(2, gpRegister.registerH);
            break;

        case 0xCBD5:
            setBit(2, gpRegister.registerL);
            break;

        case 0xCBD6:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            setBit(2, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBD7:
            setBit(2, gpRegister.registerA);
            break;

        case 0xCBD8:
            setBit(3, gpRegister.registerB);
            break;

        case 0xCBD9:
            setBit(3, gpRegister.registerC);
            break;

        case 0xCBDA:
            setBit(3, gpRegister.registerD);
            break;

        case 0xCBDB:
            setBit(3, gpRegister.registerE);
            break;

        case 0xCBDC:
            setBit(3, gpRegister.registerH);
            break;

        case 0xCBDD:
            setBit(3, gpRegister.registerL);
            break;

        case 0xCBDE:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            setBit(3, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBDF:
            setBit(3, gpRegister.registerA);
            break;

        case 0xCBE0:
            setBit(4, gpRegister.registerB);
            break;

        case 0xCBE1:
            setBit(4, gpRegister.registerC);
            break;

        case 0xCBE2:
            setBit(4, gpRegister.registerD);
            break;

        case 0xCBE3:
            setBit(4, gpRegister.registerE);
            break;

        case 0xCBE4:
            setBit(4, gpRegister.registerH);
            break;

        case 0xCBE5:
            setBit(4, gpRegister.registerL);
            break;

        case 0xCBE6:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            setBit(4, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBE7:
            setBit(4, gpRegister.registerA);
            break;

        case 0xCBE8:
            setBit(5, gpRegister.registerB);
            break;

        case 0xCBE9:
            setBit(5, gpRegister.registerC);
            break;

        case 0xCBEA:
            setBit(5, gpRegister.registerD);
            break;

        case 0xCBEB:
            setBit(5, gpRegister.registerE);
            break;

        case 0xCBEC:
            setBit(5, gpRegister.registerH);
            break;

        case 0xCBED:
            setBit(5, gpRegister.registerL);
            break;

        case 0xCBEE:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            setBit(5, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBEF:
            setBit(5, gpRegister.registerA);
            break;

        case 0xCBF0:
            setBit(6, gpRegister.registerB);
            break;

        case 0xCBF1:
            setBit(6, gpRegister.registerC);
            break;

        case 0xCBF2:
            setBit(6, gpRegister.registerD);
            break;

        case 0xCBF3:
            setBit(6, gpRegister.registerE);
            break;

        case 0xCBF4:
            setBit(6, gpRegister.registerH);
            break;

        case 0xCBF5:
            setBit(6, gpRegister.registerL);
            break;

        case 0xCBF6:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            setBit(6, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBF7:
            setBit(6, gpRegister.registerA);
            break;

        case 0xCBF8:
            setBit(7, gpRegister.registerB);
            break;

        case 0xCBF9:
            setBit(7, gpRegister.registerC);
            break;

        case 0xCBFA:
            setBit(7, gpRegister.registerD);
            break;

        case 0xCBFB:
            setBit(7, gpRegister.registerE);
            break;

        case 0xCBFC:
            setBit(7, gpRegister.registerH);
            break;

        case 0xCBFD:
            setBit(7, gpRegister.registerL);
            break;

        case 0xCBFE:
        {
            uint8_t value = m_memoryMap->readMemoryBus(gpRegister.registerHL());
            setBit(7, value);
            m_memoryMap->writeMemoryBus(gpRegister.registerHL(), value);
            break;
        }

        case 0xCBFF:
            setBit(7, gpRegister.registerA);
            break;
            
        // case 0xCBC7:    
        //     setBit(currentInstruction.operant, gpRegister.registerA);
        //     break;
        //case 0x1000:

        //TODO Implement STOP
        default:
            printf("Unsupported OpCode %d, HALT", currentInstruction.operation);
            //assert(false);
    }   
}

void Cpu::add8Bit(uint8_t operant)
{
    statusRegister.checkCarryFlag8BitAdd(gpRegister.registerA, operant);
    statusRegister.checkHalfCarryFlag8BitAdd(gpRegister.registerA, operant);
    statusRegister.resetNegativFlag();
    statusRegister.checkZeroFlag(gpRegister.registerA + operant);
    gpRegister.registerA += operant;
}

void Cpu::add16Bit(uint16_t operant)
{
    statusRegister.checkCarryFlag16BitAdd(gpRegister.registerHL(), operant);
    statusRegister.checkHalfCarryFlag16BitAdd(gpRegister.registerHL(), operant);
    statusRegister.resetNegativFlag();
    gpRegister.registerHL(gpRegister.registerHL() + operant);
}

void Cpu::addCarry8Bit(uint8_t operant)
{   
    uint8_t carry = statusRegister.isCarryFlagSet() ? 1 : 0;
    uint16_t result = gpRegister.registerA + operant + carry;
    statusRegister.resetNegativFlag();
    statusRegister.checkZeroFlag(result & 0xFF);
    (gpRegister.registerA ^ operant ^ result) & 0x10 ? statusRegister.setHalfCarryFlag() : statusRegister.resetHalfCarryFlag();
    (result & 0xFF00) ? statusRegister.setCarryFlag() : statusRegister.resertCarryFlag();
    gpRegister.registerA = result & 0xFF;
}


void Cpu::sub8Bit(uint8_t operant)
{
    statusRegister.checkCarryFlag8BitSub(gpRegister.registerA, operant);
    statusRegister.checkHalfCarryFlag8BitSub(gpRegister.registerA, operant);
    statusRegister.setNegativFlag();
    gpRegister.registerA -= operant;
    statusRegister.checkZeroFlag(gpRegister.registerA);
}

void Cpu::sub16Bit(uint16_t operant)
{
    statusRegister.checkCarryFlag16BitSub(gpRegister.registerHL(), operant);
    statusRegister.checkHalfCarryFlag16BitSub(gpRegister.registerHL(), operant);
    statusRegister.resetZeroFlag();
    statusRegister.resetNegativFlag();
    gpRegister.registerHL(gpRegister.registerHL() - operant);
}

void Cpu::subCarry8Bit(uint8_t operant)
{
    uint8_t carry = statusRegister.isCarryFlagSet() ? 1 : 0;
    uint16_t result = gpRegister.registerA - operant - carry;
    statusRegister.setNegativFlag();
    statusRegister.checkZeroFlag(result & 0xFF);
    (gpRegister.registerA ^ operant ^ result) & 0x10 ? statusRegister.setHalfCarryFlag() : statusRegister.resetHalfCarryFlag();
    (result & 0xFF00) ? statusRegister.setCarryFlag() : statusRegister.resertCarryFlag();
    gpRegister.registerA = result & 0xFF;
}

void Cpu::and8Bit(uint8_t operant)
{
    statusRegister.resetNegativFlag();
    statusRegister.resertCarryFlag();
    statusRegister.setHalfCarryFlag();
    gpRegister.registerA &= operant;
    statusRegister.checkZeroFlag(gpRegister.registerA);
}

void Cpu::or8Bit(uint8_t operant)
{
    statusRegister.resetNegativFlag();
    statusRegister.resetHalfCarryFlag();
    statusRegister.resertCarryFlag();
    gpRegister.registerA |= operant;
    statusRegister.checkZeroFlag(gpRegister.registerA);
}

void Cpu::xor8Bit(uint8_t operant)
{
    statusRegister.resetNegativFlag();
    statusRegister.resetHalfCarryFlag();
    statusRegister.resertCarryFlag();
    gpRegister.registerA ^= operant;
    statusRegister.checkZeroFlag(gpRegister.registerA);
}

void Cpu::compare8Bit(uint8_t operant)
{
    statusRegister.setNegativFlag();
    if (gpRegister.registerA - operant == 0)
    {
        statusRegister.checkZeroFlag(0);
    }
    else
    {
        statusRegister.resetZeroFlag();
    }
    statusRegister.checkCarryFlag8BitSub(gpRegister.registerA, operant);
    statusRegister.checkHalfCarryFlag8BitSub(gpRegister.registerA, operant);
}

void Cpu::increment8Bit(uint8_t& operant)
{
    statusRegister.checkHalfCarryFlag8BitAdd(operant, 1);
    statusRegister.resetNegativFlag();
    operant++;
    statusRegister.checkZeroFlag(operant);
}

void Cpu::decrement8Bit(uint8_t& operant)
{
    statusRegister.checkHalfCarryFlag8BitSub(operant, 1);
    statusRegister.setNegativFlag();
    operant--;
    statusRegister.checkZeroFlag(operant);
}

void Cpu::swap(uint8_t& operant)
{
    statusRegister.resetHalfCarryFlag();
    statusRegister.resertCarryFlag();
    statusRegister.resetNegativFlag();
    uint8_t lower = operant & 0x0F;
    uint8_t upper = operant & 0xF0;
    operant = 0;
    operant |= (lower << 4);
    operant |= (upper >> 4);
    statusRegister.checkZeroFlag(operant);
}

void Cpu::rotateLeft(uint8_t& operant)
{
    operant & 0x80? statusRegister.setCarryFlag() : statusRegister.resertCarryFlag();
    uint8_t oldMSB = operant & 0x80;
    operant <<= 1;
    operant |= (oldMSB >> 7);
    statusRegister.resetZeroFlag();
    statusRegister.resetNegativFlag();
    statusRegister.resetHalfCarryFlag();
}

void Cpu::rotateLeftCB(uint8_t& operant)
{
    rotateLeft(operant);
    statusRegister.checkZeroFlag(operant);
}

void Cpu::rotateLeftThroughCarry(uint8_t& operant)
{
    uint8_t oldCarryBit = statusRegister.isCarryFlagSet()? 0x01 : 0x00;
    operant & 0x80? statusRegister.setCarryFlag() : statusRegister.resertCarryFlag();
    operant <<= 1;
    operant |= oldCarryBit;
    statusRegister.resetZeroFlag();
    statusRegister.resetNegativFlag();
    statusRegister.resetHalfCarryFlag();
}

void Cpu::rotateLeftThroughtCarryCB(uint8_t& operant)
{
    rotateLeftThroughCarry(operant);
    statusRegister.checkZeroFlag(operant);
}

void Cpu::rotateRight(uint8_t& operant)
{
    operant & 0x01? statusRegister.setCarryFlag() : statusRegister.resertCarryFlag();
    uint8_t oldLSB = operant & 0x01;
    operant = (operant >> 1);
    operant |= (oldLSB <<7);
    statusRegister.resetZeroFlag();
    statusRegister.resetNegativFlag();
    statusRegister.resetHalfCarryFlag();
}

void Cpu::rotateRightCB(uint8_t& operant)
{
    rotateRight(operant);
    statusRegister.checkZeroFlag(operant);
}

void Cpu::rotateRightThroughCarry(uint8_t& operant)
{
    uint8_t oldCarryBit = statusRegister.isCarryFlagSet()? 0x80 : 0x00;
    operant & 0x01? statusRegister.setCarryFlag() : statusRegister.resertCarryFlag();
    operant >>= 1;
    operant |= oldCarryBit;
    statusRegister.resetZeroFlag();
    statusRegister.resetNegativFlag();
    statusRegister.resetHalfCarryFlag();
}

void Cpu::rotateRightThroughCarryCB(uint8_t& operant)
{
    rotateRightThroughCarry(operant);
    statusRegister.checkZeroFlag(operant);

}

void Cpu::shiftLeft(uint8_t& operant)
{
    statusRegister.resetNegativFlag();
    statusRegister.resetHalfCarryFlag();
    operant & 0x80 ? statusRegister.setCarryFlag() : statusRegister.resertCarryFlag();
    operant <<= 1;
    statusRegister.checkZeroFlag(operant);
}

void Cpu::shiftRight(uint8_t& operant)
{
    statusRegister.resetNegativFlag();
    statusRegister.resetHalfCarryFlag();
    operant & 0x01 ? statusRegister.setCarryFlag() : statusRegister.resertCarryFlag();
    operant >>= 1;
    statusRegister.checkZeroFlag(operant);
}

void Cpu::shiftRightKeepMSB(uint8_t& operant)
{
    uint8_t oldMSB = operant & 0x80;
    statusRegister.resetNegativFlag();
    statusRegister.resetHalfCarryFlag();
    operant & 0x01 ? statusRegister.setCarryFlag() : statusRegister.resertCarryFlag();
    operant >>= 1;
    operant |= oldMSB;
    statusRegister.checkZeroFlag(operant);
}

void Cpu::testBit(uint8_t bit, uint8_t inRegister)
{
    statusRegister.resetNegativFlag();
    statusRegister.setHalfCarryFlag();
    if ((inRegister >> bit) & 0x1)
    {
        statusRegister.resetZeroFlag();
    }
    else
    {
        statusRegister.checkZeroFlag(0);
    }
}

void Cpu::setBit(uint8_t bit, uint8_t& operant)
{
    operant |= 1 << bit;
}

void Cpu::resetBit(uint8_t bit, uint8_t& operant)
{
    operant &= ~(1 << bit);
}

void Cpu::call(uint16_t address)
{
    stackPointer--;
    m_memoryMap->writeMemoryBus(stackPointer, (uint8_t) (programmCounter >> 8));
    stackPointer--;
    m_memoryMap->writeMemoryBus(stackPointer, (uint8_t) programmCounter);
    programmCounter = address;
}

void Cpu::reset(uint8_t address)
{

    stackPointer--;
    m_memoryMap->writeMemoryBus(stackPointer, (uint8_t) (programmCounter >> 8));
    stackPointer--;
    m_memoryMap->writeMemoryBus(stackPointer, (uint8_t) programmCounter);
    programmCounter = address;
}

void Cpu::funcReturn()
{
    uint8_t lower = m_memoryMap->readMemoryBus(stackPointer);
    stackPointer++;
    uint8_t upper = m_memoryMap->readMemoryBus(stackPointer);
    stackPointer++;
    programmCounter = ((uint16_t) upper << 8) | lower;
}