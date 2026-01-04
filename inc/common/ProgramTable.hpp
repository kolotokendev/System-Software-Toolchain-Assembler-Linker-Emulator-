#pragma once

#include <string>
#include <vector>

#include "Elf.hpp"

using namespace std;


struct ProgramTable {
    Elf32_Phdr &get(Elf32_Section s) {
        return progDefs[s - 1];
    }

    void add(const Elf32_Phdr &ph) {
        progDefs.emplace_back(ph);
    }

    const Elf32_Phdr &get(Elf32_Section s) const {
        return progDefs[s - 1];
    }

    vector<Elf32_Phdr> progDefs;
};
