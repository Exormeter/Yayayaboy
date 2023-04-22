#pragma once

#include <iostream>

#include "peripheral.hpp"
#include "../Memory/register.hpp"

class Serial : public Peripheral
{
public:

    Serial()
    {
        // m_SerialData.setOnWriteHandler([&](uint8_t value){
        //     m_SerialControl.value() = 0;
        //     std::cout << value << std::flush;
        // });

        m_peripheralMemoryMap.insert(m_SerialControl.toPair());
        m_peripheralMemoryMap.insert(m_SerialData.toPair());
    }

private:

    Register<0xFF02> m_SerialControl;
    Register<0xFF01> m_SerialData;

};