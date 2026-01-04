#include "../../inc/assembler/Assembler.hpp"

#include <cstring>

void Assembler::endDirective() {
    eFile.sectionTable.finalizeLastSection(locationCounter);
    tryToResolveTNS();
    currentSection = 0;
    locationCounter = 0;
    patchCode();
    eFile.writeRelocatableToFile(outFile);
}

void Assembler::asciiDirective(string str) {
    if (!currentSection) {
        throw runtime_error("AssemblerErr: Symbol defined in undefined section!");
    }
    str = str.substr(1, str.size() - 2);
    eFile.dataSections[currentSection].write(str.c_str(), (streamsize)(str.size()));
    incLocationCounter(str.size());
}

void Assembler::skipDirective(Elf32_Word size) {
    if (!currentSection) {
        throw runtime_error("AssemblerErr: Symbol defined in undefined section!");
    }
    char* buff = new char[size];
    memset(buff, 0x0, size);
    delete[] buff;
    eFile.dataSections[currentSection].write(buff, size);
    incLocationCounter(size);
}

