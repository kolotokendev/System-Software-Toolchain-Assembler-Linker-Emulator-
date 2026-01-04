#include "../../inc/assembler/Assembler.hpp"

Elf32_Addr Assembler::getPoolConstantAddr(const Constant &constant) {
    auto &currentLiteralTable = literalTable[currentSection];
    if (currentLiteralTable.hasConstant(constant)) {
        return currentLiteralTable.getConstantAddress(constant);
    }
    Elf32_Shdr &section = eFile.sectionTable.get(currentSection);
    Elf32_Addr sectionEnd = section.sh_size;
    currentLiteralTable.insertConstant(constant, sectionEnd);
    section.sh_size += 4;
    eFile.dataSections[currentSection].seekp(0, ios::end);
    if(constant.isNumeric){
        Elf32_Addr literal = constant.number;
        eFile.dataSections[currentSection].write((char*)(&literal), sizeof(Elf32_Word));
    }else{
        Elf32_Word relocValue = generateRelocation(constant.symbol);
        eFile.dataSections[currentSection].write((char*)(&relocValue), sizeof(Elf32_Word));
    }
    incLocationCounter();
    return sectionEnd;
}

Elf32_Word Assembler::generateRelocation(const string &sym) {
    Elf32_Sym *symbolData = eFile.symbolTable.get(sym);

    if (symbolData->st_shndx == SHN_ABS && equExtRela.find(sym) == equExtRela.end()) {
        return symbolData->st_value;
    }

    Elf32_Rela relocationEntry;
    relocationEntry.r_offset = locationCounter;

    bool isLocal = (ELF32_ST_BIND(symbolData->st_info) == STB_LOCAL);
    bool isAbsoluteFromEqu = (symbolData->st_shndx == SHN_ABS);

    if (isAbsoluteFromEqu) {
        relocationEntry = equExtRela[sym];

    } else if (symbolData->st_shndx != SHN_UNDEF && isLocal) {
        relocationEntry.r_addend = (Elf32_Sword)(symbolData->st_value);
        
        Elf32_Word sectionNameIndex = eFile.sectionTable.get(symbolData->st_shndx).sh_name;
        relocationEntry.r_info = ELF32_R_INFO(sectionNameIndex, R_32);
        
    } else {
        relocationEntry.r_addend = 0;
        
        relocationEntry.r_info = ELF32_R_INFO(symbolData->st_name, R_32);
    }
    
    eFile.relocationTables[currentSection].insertRelocationEntry(relocationEntry);

    return 0;
}