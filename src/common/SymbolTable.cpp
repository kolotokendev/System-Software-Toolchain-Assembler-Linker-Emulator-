#include "../../inc/common/SymbolTable.hpp"
#include "../../inc/assembler/Assembler.hpp"

#include <iostream>

using namespace std;

Elf32_Word SymbolTable::getSymbolIndex(const string &s) {
    return symbolMappings[s];
}

Elf32_Sym *SymbolTable::get(const string &s) {
    if (symbolMappings.find(s) == symbolMappings.end()) return nullptr;
    return &symbolDefinitions[symbolMappings[s]];
}

Elf32_Sym *SymbolTable::defineSymbol(const string &name, Elf32_Sym sd) {

    symbolMappings[name] = sd.st_name = symbolDefinitions.size();

    symbolNames.emplace_back(name);

    symbolDefinitions.emplace_back(sd);

    return &symbolDefinitions.back();
}

