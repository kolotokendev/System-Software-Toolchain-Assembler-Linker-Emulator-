#include "../../inc/assembler/Assembler.hpp"

void Assembler::insertConstant(const string &symbol) {
    if (!currentSection) {
        throw runtime_error("AssemblerErr: Symbol defined in undefined section!");
    }
    insertSymbol(symbol);
    Elf32_Word relocationValue = generateRelocation(symbol);
    eFile.dataSections[currentSection].seekp(0, ios::end);
    eFile.dataSections[currentSection].write(reinterpret_cast<char *>(&relocationValue), sizeof(Elf32_Word));
    incLocationCounter();
}

void Assembler::insertConstant(Elf32_Word literal) {
    if (!currentSection) {
        throw runtime_error("AssemblerErr: Symbol defined in undefined section!");
    }
    eFile.dataSections[currentSection].seekp(0, ios::end);
    eFile.dataSections[currentSection].write(reinterpret_cast<char *>(&literal), sizeof(Elf32_Word));
    incLocationCounter();
}
