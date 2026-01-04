#pragma once

#include "Elf.hpp"


#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include "../../inc/common/Disp.hpp"

#include <iostream>

using namespace std;


struct SymbolTable {
    Elf32_Sym *get(const string &s);

    Elf32_Word getSymbolIndex(const string &s);

    Elf32_Sym *defineSymbol(const string &name, Elf32_Sym sd = {});

    unordered_map<string, Elf32_Word> symbolMappings;

    vector<Elf32_Sym> symbolDefinitions;

    vector<string> symbolNames;

};
