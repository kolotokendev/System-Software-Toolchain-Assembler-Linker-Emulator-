#include "../../inc/linker/Linker.hpp"
#include <unordered_map>
#include <stdexcept>

namespace {

bool isSymbolRelevant(const Elf32_Sym& sym) {
    return sym.st_name != 0;
}

void updateGlobalSymbolStatus(Elf32_Sym& sym, const std::string& symName, 
                              std::unordered_map<std::string, Elf32_Sym*>& symbolMap) {
    bool isDefined = (sym.st_shndx != SHN_UNDEF);

    if (isDefined) {
        auto it = symbolMap.find(symName);
        if (it != symbolMap.end() && it->second != nullptr) {
            throw std::runtime_error("Linker error: multiple definitions of symbol: " + symName);
        }
        symbolMap[symName] = &sym;
    } else {
        auto it = symbolMap.find(symName);
        if (it == symbolMap.end()) {
            symbolMap[symName] = nullptr;
        }
    }
}


void updateSymbolValue(Elf32_Sym& sym, const ElfFile& eFile, std::unordered_map<std::string, Elf32_Addr>& sectionMap) {
    bool isGlobal = (ELF32_ST_BIND(sym.st_info) == STB_GLOBAL);
    bool isSection = (ELF32_ST_TYPE(sym.st_info) == STT_SECTION);
    
    if (sym.st_shndx != SHN_ABS && sym.st_shndx != SHN_UNDEF && (isGlobal || isSection)) {
        std::string sectionName = eFile.sectionName(sym);
        sym.st_value += sectionMap[sectionName] + eFile.sectionTable.get(sym.st_shndx).sh_addr;
    }
}

void checkForUndefinedSymbols(const std::unordered_map<std::string, Elf32_Sym*>& symbolMap, bool execMode, ElfFile& outFile) {
    std::vector<std::string> undefinedSymbols;
    for (const auto& pair : symbolMap) {
        if (pair.second == nullptr) {
            undefinedSymbols.push_back(pair.first);
        }
    }

    if (undefinedSymbols.empty()) {
        return;
    }

    if (execMode) {
        std::string errorMsg = "LinkerErr: symbols not defined: ";
        for (const auto& name : undefinedSymbols) {
            errorMsg += name + " ";
        }
        throw std::runtime_error(errorMsg);
    } else {
        for (const auto& name : undefinedSymbols) {
            Elf32_Sym sym{};
            outFile.symbolTable.defineSymbol(name, sym);
        }
    }
}

}

void Linker::defineAllSymbols() {
    std::unordered_map<std::string, Elf32_Sym*> globalSymbolMap;

    for (auto& eFile : inputFileObjects) {
        for (auto& sym : eFile.symbolTable.symbolDefinitions) {
            if (!isSymbolRelevant(sym)) {
                continue;
            }

            if (ELF32_ST_BIND(sym.st_info) == STB_GLOBAL) {
                std::string symName = eFile.symbolName(sym);
                updateGlobalSymbolStatus(sym, symName, globalSymbolMap);
            }

            updateSymbolValue(sym, eFile, sectionMap);
        }
    }

    checkForUndefinedSymbols(globalSymbolMap, execMode, outFile);
}

void Linker::generateSymbols() {
    for (auto& eFile : inputFileObjects) {
        for (const auto& sym : eFile.symbolTable.symbolDefinitions) {
            bool isGlobal = (ELF32_ST_BIND(sym.st_info) == STB_GLOBAL);
            bool isDefined = (sym.st_shndx != SHN_UNDEF);

            if (isGlobal && isDefined) {
                std::string symName = eFile.symbolName(sym);
                
                if (outFile.symbolTable.get(symName) != nullptr) {
                    continue;
                }
                
                Elf32_Sym outputSymbol = sym;
                
                if (outputSymbol.st_shndx != SHN_ABS) {
                    std::string sectionName = eFile.sectionName(sym);
                    outputSymbol.st_shndx = outFile.sectionTable.getSectionIndex(sectionName);
                }
                
                outFile.symbolTable.defineSymbol(symName, outputSymbol);
            }
        }
    }
}