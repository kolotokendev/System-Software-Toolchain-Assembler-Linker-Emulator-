#pragma once

#include <algorithm>

#include <chrono>

using namespace std::chrono;

#include "../common/Elf.hpp"
#include "../common/ElfFile.hpp"
#include "../common/Instruction.hpp"
#include "Terminal.hpp"
#include "Architecture.hpp"
#include "CPU.hpp"


class Emulator {
public:
    explicit Emulator(const string &inFile);

    void run();
    
private:
    void triggerTrap(Emulator& emu, Elf32_Word trapCause);

    void loadProgramData();

    void handleInstruction(Elf32_Word nextInstruction);

    void STMEM(Elf32_Addr addr, Elf32_Word word);

    Elf32_Word LDMEM(Elf32_Addr addr);

    void push(Elf32_Word reg);

    Elf32_Word pop();

    void handleHalt(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);
    void handleInt(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);
    void handleCall(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);
    void handleJmp(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);
    void handleXchg(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);
    void handleAlu(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);
    void handleLog(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);
    void handleShf(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);
    void handleSt(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);
    void handleLd(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);
    void handleBadOpCode(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp);

    using InstructionHandler = void (Emulator::*)(uint8_t, Reg, Reg, Reg, Elf32_Sword);
    
    static const vector<InstructionHandler> handlerTable;

    bool timerMasked() {
        return cpu.getCSRX(STATUS) & TIMER_MASK_BIT;
    }

    bool terminalMasked() {
        return cpu.getCSRX(STATUS) & TERMINAL_MASK_BIT;
    }

    bool interruptsMasked() {
        return cpu.getCSRX(STATUS) & GLOBAL_MASK_BIT;
    }

    void writeToTerminal();

    void handleTerminal();

    void handleInterrupts();

    void handleTimer();

    ElfFile execFile;

    unsigned char* memory;

    CPU cpu;

    bool running = false;

    bool terminalIntrPending = false;

    bool terminalWritePending = false;

    Elf32_Word timerMode = 0;

    time_point<high_resolution_clock> period_start = high_resolution_clock::now();

    time_point<high_resolution_clock> period_end;

    bool timerIntrPending = false;

    bool badInstrIntrPending = false;

};