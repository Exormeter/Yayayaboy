#pragma once

#include "InterruptController.hpp"
#include <functional>

class InterruptSource
{

public:

    InterruptSource(InterruptController& interruptController, InterruptFlags flag) : 
        m_interruptController(interruptController), m_flag(flag)
    {}

    void raiseInterrupt()
    {
        m_interruptController.get().raiseInterrupt(m_flag);
    }

private:

    std::reference_wrapper<InterruptController> m_interruptController;
    InterruptFlags m_flag;
};