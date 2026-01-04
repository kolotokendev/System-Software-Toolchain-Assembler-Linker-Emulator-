#include "../../inc/assembler/Assembler.hpp"

Elf32_Sym* Assembler::insertSymbolLOC(const string& symbol) {
    if (currentSection == SHN_UNDEF) {
        return nullptr;
    }

    auto* existingSymbol = eFile.symbolTable.get(symbol);

    if (existingSymbol) { 
        if (existingSymbol->st_shndx != SHN_UNDEF) {
            throw runtime_error("Error: Symbol '" + symbol + "' is already defined.");
        }
        existingSymbol->st_shndx = currentSection;
        existingSymbol->st_value = locationCounter;
        return existingSymbol;

    } else { 
        Elf32_Sym newSymbol{};
        newSymbol.st_shndx = currentSection;
        newSymbol.st_value = locationCounter;
        newSymbol.st_info = ELF32_ST_INFO(STB_LOCAL, STT_NOTYPE);
        return eFile.symbolTable.defineSymbol(symbol, newSymbol);
    }
}

void Assembler::insertSymbolABS(const string& symbol, uint32_t symbolValue) {
    auto* existingSymbol = eFile.symbolTable.get(symbol);

    if (existingSymbol) {
        bool isRedefinable = (existingSymbol->st_shndx == SHN_UNDEF || existingSymbol->st_shndx == SHN_ABS);
        if (!isRedefinable) {
            throw runtime_error("Error: Symbol '" + symbol + "' is already defined in a section and cannot be made absolute.");
        }
                existingSymbol->st_shndx = SHN_ABS;
        existingSymbol->st_value = symbolValue;

    } else {
        Elf32_Sym newSymbol{};
        newSymbol.st_shndx = SHN_ABS;
        newSymbol.st_value = symbolValue;
        eFile.symbolTable.defineSymbol(symbol, newSymbol);
    }
}

void Assembler::insertSymbolGLOB(const string& symbol) {
    auto* existingSymbol = eFile.symbolTable.get(symbol);

    if (existingSymbol) { 
        if (ELF32_ST_TYPE(existingSymbol->st_info) == STT_SECTION) {
            cerr << "Warning: Section '" << symbol << "' is global by default." << endl;
            return;
        }
        existingSymbol->st_info = ELF32_ST_INFO(STB_GLOBAL, ELF32_ST_TYPE(existingSymbol->st_info));

    } else {
        Elf32_Sym newSymbol{};
        newSymbol.st_shndx = SHN_UNDEF;
        newSymbol.st_value = 0;
        newSymbol.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
        eFile.symbolTable.defineSymbol(symbol, newSymbol);
    }
}

void Assembler::insertSymbol(const string &symbol) {
    if (eFile.symbolTable.get(symbol)) return;
    eFile.symbolTable.defineSymbol(symbol);
}

