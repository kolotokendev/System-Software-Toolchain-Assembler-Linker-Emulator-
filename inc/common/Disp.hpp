#pragma once

#include "Elf.hpp"
#include "Instruction.hpp"
#include <cmath>

class Disp {
public:
    static bool isInRange(Elf32_Sword value) {
        return value <= MAX_DISP && value >= MIN_DISP;
    }

    static bool isWithinLimit(Elf32_Word value) {
        return value <= MAX_DISP;
    }

    static int16_t getOffset(Elf32_Addr to, Elf32_Addr from){
        auto disp = (Elf32_Sword)(to - from);

        if (!isInRange(disp)) {
            throw runtime_error("AssemblerErr: Displacement can not fit in 12 bits");
        }

        return (int16_t)(disp);
    }
};

