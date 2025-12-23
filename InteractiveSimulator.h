#ifndef INTERACTIVE_SIMULATOR_H
#define INTERACTIVE_SIMULATOR_H
#pragma once
#include "Simulator.h"
#include <iostream>

class InteractiveSimulator : public Simulator
{

public:
    int loadProgramFromString(const std::string &asmText);
    void runInteractive();
    bool loadProgram(const std::string &asmFilePath);
    InteractiveSimulator(std::istream &in = std::cin, std::ostream &out = std::cout, ISAMode mode = ISAMode::RV32I)
        : Simulator(in, out, mode) {}
};
#endif