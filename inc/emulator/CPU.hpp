#pragma once

#include <ostream>
#include <iomanip>
#include "../common/Elf.hpp"
#include "../common/Instruction.hpp"
#include "Architecture.hpp"

struct CPU {
public:
    Elf32_Word getGPRX(Reg reg) {
        if (reg >= NUM_GPRX_REGS) throw std::out_of_range("Gprx out of range");
        return GPRX[reg];
    }

    Elf32_Word getCSRX(Reg reg) {
        if (reg >= NUM_CSRX_REGS) throw std::out_of_range("Csrx out of range");
        return CSRX[reg];
    }

    void setGPRX(Reg reg, Elf32_Word val) {
        if (reg >= NUM_GPRX_REGS) throw std::out_of_range("Gprx out of range");
        GPRX[reg] = val;
    }

    void setCSRX(Reg reg, Elf32_Word val) {
        if (reg >= NUM_CSRX_REGS) throw std::out_of_range("Csrx out of range");
        CSRX[reg] = val;
    }

    void printCPUState();

private:
    Elf32_Word GPRX[NUM_GPRX_REGS]{};

    Elf32_Word CSRX[NUM_CSRX_REGS]{};
};