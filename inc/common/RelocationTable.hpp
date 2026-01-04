#pragma once

#include <vector>

#include "Elf.hpp"

using namespace std;

struct RelocationTable {
    void insertRelocationEntry(Elf32_Rela rd) {
        relocEntries.emplace_back(rd);
    }

    vector<Elf32_Rela> relocEntries;
};


