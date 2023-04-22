#pragma once

#include <iostream>
#include <fstream>
#include <memory>

#include "../Peripheral/peripheral.hpp"
#include "./standardCartridge.hpp"
#include "./mcb1Cartrdige.hpp"


class CartridgeBuilder
{

public:

    static std::unique_ptr<Peripheral> openROM(const char* fileName)
    {
        std::ifstream romFile;

        romFile.open(fileName, ios::binary);

        char controllerIdentifier = 0;

        romFile.seekg(0x147);
        romFile.read(&controllerIdentifier, 1);
        romFile.seekg(0, std::ios::seekdir::beg);

        std::unique_ptr<Peripheral> cartridge;
        switch(controllerIdentifier)
        {
            case 0x00: cartridge = std::make_unique<StandardCartridge>(romFile); break;
            case 0x01: cartridge = std::make_unique<Mcb1Cartridge>(romFile); break;
            default: assert(false);
        }

        romFile.close();

        return cartridge;
    }
};