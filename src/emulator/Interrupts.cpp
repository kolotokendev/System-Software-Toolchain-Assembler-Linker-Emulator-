#include "../../inc/emulator/Emulator.hpp"
#include <iostream> 


void Emulator::triggerTrap(Emulator& emu, Elf32_Word trapCause) {
    emu.push(emu.cpu.getCSRX(STATUS));
    emu.push(emu.cpu.getGPRX(PC));

    emu.cpu.setCSRX(CAUSE, trapCause);

    const Elf32_Word currentStatus = emu.cpu.getCSRX(STATUS);
    emu.cpu.setCSRX(STATUS, currentStatus | GLOBAL_MASK_BIT);

    emu.cpu.setGPRX(PC, emu.cpu.getCSRX(HANDLER));
}

void Emulator::handleInterrupts() {
    Elf32_Word interruptCause = CAUSE_NO_INTR;


    if (badInstrIntrPending) {
        interruptCause = CAUSE_BAD_INSTR;
        badInstrIntrPending = false;
    } 
    else if (timerIntrPending && !interruptsMasked() && !timerMasked()) {
        interruptCause = CAUSE_INTR_TIMER;
        timerIntrPending = false;
    } 
    else if (terminalIntrPending && !interruptsMasked() && !terminalMasked()) {
        interruptCause = CAUSE_INTR_TERMINAL;
        terminalIntrPending = false;
    }

    if (interruptCause != CAUSE_NO_INTR) {
        triggerTrap(*this, interruptCause);
    }
}

void Emulator::writeToTerminal() {
    const Elf32_Word character = LDMEM(TERM_OUT);

    std::cout << (char)(character);
    std::cout.flush();

    terminalWritePending = false;
}

void Emulator::handleTerminal() {
    const int inputChar = getchar();

    if (inputChar != EOF) {
        STMEM(TERM_IN, (Elf32_Word)(inputChar));
        terminalIntrPending = true;
    }
}

void Emulator::handleTimer() {
    auto currentTime = high_resolution_clock::now();

    auto elapsedMicroseconds = duration_cast<microseconds>(currentTime - period_start).count();

    const long int currentTimerInterval = TIMER_MODES[timerMode];

    if (elapsedMicroseconds >= currentTimerInterval) {
        timerIntrPending = true;
        period_start = high_resolution_clock::now();
    }
}