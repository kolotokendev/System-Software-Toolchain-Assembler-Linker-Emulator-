#include <sys/mman.h>

#include "../../inc/emulator/Emulator.hpp"

Emulator::Emulator(const string &inFile) {
    memory = (unsigned char*)(mmap(nullptr, MEMORY_SIZE_BYTES,
                                      PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
                                      -1, 0));
    execFile.loadFromInputFile(inFile);

    if (execFile.elfHeader.e_type != ET_EXEC) {
        throw runtime_error("Emulator error: file is not an executable");
    }

    cpu.setGPRX(PC, DEFAULT_PC);
    cpu.setGPRX(SP, DEFAULT_SP);

    loadProgramData();
}

void Emulator::loadProgramData() {
    Elf32_Section i = 1;
    for (auto &phdr: execFile.programTable.progDefs) {
        stringstream &dataSection = execFile.dataSections[i++];

        dataSection.read(reinterpret_cast<char *>(&memory[phdr.p_paddr]), phdr.p_memsz);
    }
}

void Emulator::run() {
    Terminal terminal;
    running = true;

    while (running) {
        Elf32_Word nextInstruction = LDMEM(cpu.getGPRX(PC));

        cpu.setGPRX(PC, cpu.getGPRX(PC) + 4);

        try {
            handleInstruction(nextInstruction);
        } catch (exception &e) {
            badInstrIntrPending = true;
        }

        if (terminalWritePending) {
            writeToTerminal();
        }

        handleTerminal();

        handleTimer();

        handleInterrupts();
    }

    cpu.printCPUState();
}

