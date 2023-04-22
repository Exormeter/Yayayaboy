#pragma once

#include "peripheral.hpp"
#include "../Memory/register.hpp"

class SoundController : public Peripheral
{

public:

    SoundController()
    {
        m_peripheralMemoryMap.insert(m_ch1Sweep.toPair());
        m_peripheralMemoryMap.insert(m_ch1SoundLenght.toPair());
        m_peripheralMemoryMap.insert(m_ch1VolEnvelope.toPair());
        m_peripheralMemoryMap.insert(m_ch1Frequency.toPair());
        m_peripheralMemoryMap.insert(m_ch1Control.toPair());

        m_peripheralMemoryMap.insert(m_ch2SoundLength.toPair());
        m_peripheralMemoryMap.insert(m_ch2VolEnvelope.toPair());
        m_peripheralMemoryMap.insert(m_ch2Frequency.toPair());
        m_peripheralMemoryMap.insert(m_ch2Control.toPair());

        m_peripheralMemoryMap.insert(m_ch3Enable.toPair());
        m_peripheralMemoryMap.insert(m_ch3SoundLenght.toPair());
        m_peripheralMemoryMap.insert(m_ch3Volume.toPair());
        m_peripheralMemoryMap.insert(m_ch3Frequency.toPair());
        m_peripheralMemoryMap.insert(m_ch3Control.toPair());

        m_peripheralMemoryMap.insert(m_ch4SoundLength.toPair());
        m_peripheralMemoryMap.insert(m_ch4Volume.toPair());
        m_peripheralMemoryMap.insert(m_ch4Frequency.toPair());
        m_peripheralMemoryMap.insert(m_ch4Control.toPair());

        m_peripheralMemoryMap.insert(m_outputMapping.toPair());
        m_peripheralMemoryMap.insert(m_channelMapping.toPair());
        m_peripheralMemoryMap.insert(m_channelControl.toPair());
        m_peripheralMemoryMap.insert(m_wavePattern.toPair());
        
    }

private:

    //Channl 1
    Register<0xFF10> m_ch1Sweep;
    Register<0xFF11> m_ch1SoundLenght;
    Register<0xFF12> m_ch1VolEnvelope;
    Register<0xFF13> m_ch1Frequency;
    Register<0xFF14> m_ch1Control;

    //Channel 2
    Register<0xFF16> m_ch2SoundLength;
    Register<0xFF17> m_ch2VolEnvelope;
    Register<0xFF18> m_ch2Frequency;
    Register<0xFF19> m_ch2Control;

    //Channel 3

    Register<0xFF1A> m_ch3Enable;
    Register<0xFF1B> m_ch3SoundLenght;
    Register<0xFF1C> m_ch3Volume;
    Register<0xFF1D> m_ch3Frequency;
    Register<0xFF1E> m_ch3Control;

    //CHannel 4
    Register<0xFF20> m_ch4SoundLength;
    Register<0xFF21> m_ch4Volume;
    Register<0xFF22> m_ch4Frequency;
    Register<0xFF23> m_ch4Control;

    Register<0xFF24> m_outputMapping;
    Register<0xFF25> m_channelMapping;
    Register<0xFF26> m_channelControl;

    MemoryRange<0xFF30, 0x10> m_wavePattern;
    
    //TODO Other Sound register

};