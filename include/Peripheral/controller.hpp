#include "peripheral.hpp"
#include "Interrupt/InterruptSource.hpp"


enum class Dpad : uint8_t 
{
    DPAD_RIGHT = 0,
    DPAD_LEFT = 1,
    DPAD_UP = 2,
    DPAD_DOWN = 3
};

enum class Button : uint8_t
{
    BUTTON_A = 0,
    BUTTON_B = 1,
    BUTTON_SELECT = 2,
    BUTTON_START = 3
};

#define SELECT_BUTTONS (1 << 4)
#define SELECT_DPAD (1 << 5) 

class Controller : public Peripheral, public InterruptSource
{
public:

    Controller(InterruptController& interruptController) : InterruptSource(interruptController, InterruptFlags::JOYPAD_FLAG)
    {
        m_joypad.value() = 0xFF;
        m_peripheralMemoryMap.insert(m_joypad.toPair());
        m_joypad.setOnReadHandler([&](const uint16_t& address)
        {
            m_joypad.value() &= 0xF0;
            if ((m_joypad.value() & SELECT_BUTTONS) != 0)
            {
                m_joypad.value() |= (0x0F & m_buttons);
            }
            else if((m_joypad.value() & SELECT_DPAD) != 0)
            {
                m_joypad.value() |= (0x0F & m_dpad);
            }
            
        });
    }

    void dpadPressed(Dpad dpad)
    {
        if (m_joypad.value() & SELECT_DPAD) raiseInterrupt();
        m_dpad &= ~(1 << (uint8_t)dpad);
    }

    void dpadReleased(Dpad dpad)
    {   
        m_dpad |= (1 << (uint8_t)dpad);
    }

    void buttonPressed(Button button)
    {
        if (m_joypad.value() & SELECT_BUTTONS) raiseInterrupt();
        m_buttons &= ~(1 << (uint8_t)button);
    }

    void buttonReleased(Button button)
    {
        m_buttons |= (1 << (uint8_t)button);
    }

private:
    Register<0xFF00> m_joypad;
    uint8_t m_buttons = 0xFF;
    uint8_t m_dpad = 0xFF;
};