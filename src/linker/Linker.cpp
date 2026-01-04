#include "../../inc/linker/Linker.hpp"
#include <utility>
#include <map>

Linker::Linker(const vector<string>& inputFiles, string outFileName,
               const unordered_map<string, Elf32_Addr>& placeDefs, bool execMode) 
    : inputFiles(inputFiles),
      outFileName(std::move(outFileName)),
      execMode(execMode),
      placeCommandDefs(placeDefs) {}

void Linker::run() {
    initialize();

    collectAndPlaceSections();

    resolveSymbols();

    processRelocations();

    buildOutputFile();
}

void Linker::initialize() {
    for (const auto& fileName : inputFiles) {
        ElfFile e32;
        e32.loadFromInputFile(fileName);

        if (e32.elfHeader.e_type != ET_REL) {
            throw runtime_error("Linker error: non-relocatable file '" + fileName + "' provided as input.");
        }
        inputFileObjects.emplace_back(std::move(e32));
    }

    outFile.addUndefinedSym();

    if (!execMode) {
        placeCommandDefs.clear();
    }
}

void Linker::collectAndPlaceSections() {
    for (auto& file : inputFileObjects) {
        getSectionSizes(file);
    }
    getSectionMappings();
}

void Linker::resolveSymbols() {
    defineAllSymbols();
    generateSymbols();
}

void Linker::processRelocations() {
    for (auto& file : inputFileObjects) {
        if (execMode) {
            applyRelocations(file);
        } else {
            mergeRelocations(file);
        }
    }
}

void Linker::buildOutputFile() {
    addSectionsToOutput();

    if (execMode) {
        outFile.writeExecutableToFile(outFileName);
    } else {
        outFile.writeRelocatableToFile(outFileName);
    }
}