#include "../../inc/linker/Linker.hpp"
#include <algorithm>
#include <vector>

namespace { 

bool isLinkableSection(const Elf32_Shdr& sh) {
    return sh.sh_type == SHT_PROGBITS || sh.sh_type == SHT_NOBITS;
}

void checkForOverlaps(const vector<pair<Elf32_Addr, Elf32_Addr>>& intervals) {
    if (intervals.size() < 2) {
        return;
    }

    auto sortedIntervals = intervals;
    sort(sortedIntervals.begin(), sortedIntervals.end());

    for (size_t i = 1; i < sortedIntervals.size(); ++i) {
        if (sortedIntervals[i].first < sortedIntervals[i - 1].second) {
            throw runtime_error("LinkerErr: Sections are overlapped.");
        }
    }
}

}

void Linker::getSectionSizes(ElfFile& eFile) {
    for (auto& sh : eFile.sectionTable.sectionDefs) {
        if (!isLinkableSection(sh)) {
            continue;
        }

        string sectionName = eFile.sectionName(sh);
        
        sh.sh_addr = sectionSizes[sectionName];
        
        sectionSizes[sectionName] += sh.sh_size;
    }
}

void Linker::getSectionMappings() {
    Elf32_Addr nextAvailableAddress = 0;

    for (const auto& pair : placeCommandDefs) {
        const string& sectionName = pair.first;
        const Elf32_Addr startAddr = pair.second;
        nextAvailableAddress = max(nextAvailableAddress, startAddr + sectionSizes[sectionName]);
    }

    vector<pair<Elf32_Addr, Elf32_Addr>> intervals;

    for (auto& eFile : inputFileObjects) {
        for (auto& sh : eFile.sectionTable.sectionDefs) {
            if (!isLinkableSection(sh)) {
                continue;
            }

            string sectionName = eFile.sectionName(sh);

            if (placeCommandDefs.find(sectionName) == placeCommandDefs.end() && execMode) {
                placeCommandDefs[sectionName] = nextAvailableAddress;
                nextAvailableAddress += sectionSizes[sectionName];
            }

            Elf32_Addr sectionStartAddr = placeCommandDefs[sectionName];
            sh.sh_addr += sectionStartAddr;

            if (outFile.sectionTable.get(sectionName) == nullptr) {
                Elf32_Shdr outSh = sh;
                outSh.sh_addr = sectionStartAddr;
                outSh.sh_size = sectionSizes[sectionName]; 

                outFile.sectionTable.add(outSh, sectionName);
                
                Elf32_Sym sym{};
                sym.st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
                sym.st_value = sectionStartAddr;
                sym.st_shndx = outFile.sectionTable.getSectionIndex(sectionName);
                outFile.symbolTable.defineSymbol(sectionName, sym);

                outFile.sectionTable.get(sectionName)->sh_name = outFile.symbolTable.getSymbolIndex(sectionName);
                
                if (execMode) {
                    intervals.push_back({sectionStartAddr, sectionStartAddr + sectionSizes[sectionName]});
                }
            }
        }
    }

    if (execMode) {
        checkForOverlaps(intervals);
    }
}



void Linker::addSectionsToOutput() {
    for (auto& file : inputFileObjects) {
        for (auto& pair : file.dataSections) {
            Elf32_Section sectionIndexInFile = pair.first;
            stringstream& sectionData = pair.second;

            string sectionName = file.sectionName(sectionIndexInFile);
            Elf32_Section outSectionIndex = outFile.sectionTable.getSectionIndex(sectionName);

            if (outFile.dataSections.find(outSectionIndex) == outFile.dataSections.end()) {
                outFile.dataSections[outSectionIndex] = std::move(sectionData);
            } else {
                outFile.dataSections[outSectionIndex] << sectionData.rdbuf();
            }
        }
    }
}