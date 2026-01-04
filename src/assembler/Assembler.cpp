#include "../../inc/assembler/Assembler.hpp"

#include <stdexcept>
#include <utility>

Elf32_Addr Assembler::locationCounter = 0;

Assembler::Assembler(string outFile) : outFile(outFile) {
    eFile.addUndefinedSym();

    literalTable.emplace_back();

    flinks.emplace_back();

    elfHeader.e_type = ET_REL;

}
