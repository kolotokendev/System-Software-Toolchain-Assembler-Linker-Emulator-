#include "../../inc/assembler/Assembler.hpp"

void Assembler::insertSection(const string &section) {
    literalTable.emplace_back();

    flinks.emplace_back();

    eFile.sectionTable.finalizeLastSection(locationCounter);

    currentSection++;
    
    locationCounter = 0;

    Elf32_Sym *sym = insertSymbolLOC(section);

    Elf32_Shdr &sec = eFile.sectionTable.add();

    sec.sh_type = SHT_PROGBITS;
    
    sym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
    
    sec.sh_name = sym->st_name;
}
