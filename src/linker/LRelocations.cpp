#include "../../inc/linker/Linker.hpp"
#include <stdexcept>

void Linker::mergeRelocations(ElfFile& eFile) {
    for (const auto& pair : eFile.relocationTables) {
        const Elf32_Section sectionIndex = pair.first;
        const auto& relocationTable = pair.second;
        const auto& sectionHeader = eFile.sectionTable.get(sectionIndex);

        for (const auto& rel : relocationTable.relocEntries) {
            Elf32_Rela outRela = createMergedRelocationEntry(eFile, rel, sectionHeader);
            
            std::string sectionName = eFile.sectionName(sectionHeader);
            Elf32_Section outSectionIndex = outFile.sectionTable.getSectionIndex(sectionName);

            outFile.relocationTables[outSectionIndex].insertRelocationEntry(outRela);
        }
    }
}

Elf32_Rela Linker::createMergedRelocationEntry(const ElfFile& eFile, const Elf32_Rela& inRela, const Elf32_Shdr& inSectionHeader) {
    std::string symName = eFile.symbolName(eFile.symbolTable.symbolDefinitions.at(ELF32_R_SYM(inRela.r_info)));
    Elf32_Sym* outSym = outFile.symbolTable.get(symName);
    if (!outSym) {
        throw std::runtime_error("LinkerErr: Symbol '" + symName + "' not found while merging relocations.");
    }
    
    Elf32_Rela outRela = inRela;

    outRela.r_offset += inSectionHeader.sh_addr;

    if (ELF32_ST_TYPE(outSym->st_info) == STT_SECTION) {
        outRela.r_addend += (Elf32_Sword)(inSectionHeader.sh_addr);
    }

    outRela.r_info = ELF32_R_INFO(outSym->st_name, ELF32_R_TYPE(inRela.r_info));

    return outRela;
}


void Linker::applyRelocations(ElfFile& eFile) {
    for (const auto& pair : eFile.relocationTables) {
        const Elf32_Section sectionIndex = pair.first;
        const auto& relocationTable = pair.second;
        std::stringstream& sectionData = eFile.dataSections.at(sectionIndex);

        for (const auto& rel : relocationTable.relocEntries) {
            const auto& inSym = eFile.symbolTable.symbolDefinitions.at(ELF32_R_SYM(rel.r_info));
            Elf32_Sym* outSym = outFile.symbolTable.get(eFile.symbolName(inSym));
            if (!outSym) {
                throw std::runtime_error("LinkerErr: Symbol not found while applying relocations.");
            }
            
            Elf32_Addr valueToPatch = calculateRelocationValue(rel, *outSym);

            patchSectionWithValue(sectionData, rel.r_offset, valueToPatch);
        }
    }
}

Elf32_Addr Linker::calculateRelocationValue(const Elf32_Rela& rel, const Elf32_Sym& targetSymbol) {
    const Elf32_Addr S = targetSymbol.st_value;
    const Elf32_Sword A = rel.r_addend;
    
    if(ELF32_R_TYPE(rel.r_info) == R_32) {
        return S + A;
    }else{
        throw std::runtime_error("LinkerErr: Unrecognized relocation type.");
    }
}

void Linker::patchSectionWithValue(std::stringstream& sectionData, Elf32_Off offset, Elf32_Addr value) {
    sectionData.seekp(offset, ios::beg);
    sectionData.write((const char*)(&value), sizeof(value));
}

