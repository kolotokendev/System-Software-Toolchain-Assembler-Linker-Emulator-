#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "../common/ElfFile.hpp"
#include "../../inc/common/SymbolTable.hpp"


class Linker {
public:
    Linker(const std::vector<std::string>& inputFiles, std::string outFile, 
           const std::unordered_map<std::string, Elf32_Addr>& placeDefs, bool execMode);

    void run();

private:
    void initialize();

    void collectAndPlaceSections();

    void resolveSymbols();

    void processRelocations();

    void buildOutputFile();

    void getSectionSizes(ElfFile& eFile);

    void getSectionMappings();

    void applyRelocations(ElfFile& eFile);

    void mergeRelocations(ElfFile& eFile); 
        
    void generateSymbols();

    void addSectionsToOutput();

    Elf32_Rela createMergedRelocationEntry(const ElfFile& eFile, const Elf32_Rela& inRela, const Elf32_Shdr& inSectionHeader);

    Elf32_Addr calculateRelocationValue(const Elf32_Rela& rel, const Elf32_Sym& targetSymbol);

    void patchSectionWithValue(std::stringstream& sectionData, Elf32_Off offset, Elf32_Addr value);
    
    void defineAllSymbols();

    std::vector<std::string> inputFiles;

    std::string outFileName;

    bool execMode;

    std::vector<ElfFile> inputFileObjects;

    ElfFile outFile;

    std::unordered_map<std::string, Elf32_Addr> placeCommandDefs;

    std::unordered_map<std::string, Elf32_Word> sectionSizes;

    std::unordered_map<std::string, Elf32_Addr> sectionMap;
};
