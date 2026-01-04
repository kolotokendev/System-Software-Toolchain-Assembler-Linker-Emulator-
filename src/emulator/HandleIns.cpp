#include "../../inc/emulator/Emulator.hpp"

void Emulator::STMEM(Elf32_Addr addr, Elf32_Word word) {
    if (addr >= TERM_OUT && addr < TERM_OUT + 4) {
        terminalWritePending = true;
    }

    if (addr >= TIM_CFG && addr < TIM_CFG + 4 && word < MODE_COUNT) {
        timerMode = word;
    }

    *(Elf32_Word*)(memory + addr) = word;
}

Elf32_Word Emulator::LDMEM(Elf32_Addr addr) {
    return *(Elf32_Word*)(memory + addr);
}

void Emulator::push(Elf32_Word reg) {
    cpu.setGPRX(SP, cpu.getGPRX(SP) - 4);
    STMEM(cpu.getGPRX(SP), reg);
}

Elf32_Word Emulator::pop() {
    Elf32_Word ret = LDMEM(cpu.getGPRX(SP));
    cpu.setGPRX(SP, cpu.getGPRX(SP) + 4);
    return ret;
}

const vector<Emulator::InstructionHandler> Emulator::handlerTable = {
    &Emulator::handleHalt,       // HALT_OP = 0
    &Emulator::handleInt,        // INT_OP = 1
    &Emulator::handleCall,       // CALL_OP = 2
    &Emulator::handleJmp,        // JMP_OP = 3
    &Emulator::handleXchg,       // XCHG_OP = 4
    &Emulator::handleAlu,        // ALU_OP = 5
    &Emulator::handleLog,        // LOG_OP = 6
    &Emulator::handleShf,        // SHF_OP = 7
    &Emulator::handleSt,         // ST_OP = 8
    &Emulator::handleLd,         // LD_OP = 9
};

void Emulator::handleInstruction(Elf32_Word nextInstruction) {
    uint8_t opCode = Instruction::extractOpCode(nextInstruction);
    uint8_t mode = Instruction::extractMode(nextInstruction);
    Reg regA = Instruction::extractRegA(nextInstruction);
    Reg regB = Instruction::extractRegB(nextInstruction);
    Reg regC = Instruction::extractRegC(nextInstruction);
    Elf32_Sword disp = Instruction::extractDisp(nextInstruction);

    if (opCode < handlerTable.size()) {
        (this->*handlerTable[opCode])(mode, regA, regB, regC, disp);
    } else {
        handleBadOpCode(mode, regA, regB, regC, disp);
    }

    cpu.setGPRX(R0, 0);
}



void Emulator::handleHalt(uint8_t, Reg, Reg, Reg, Elf32_Sword) {
    running = false;
}

void Emulator::handleInt(uint8_t, Reg, Reg, Reg, Elf32_Sword) {
    push(cpu.getCSRX(STATUS));
    push(cpu.getGPRX(PC));
    cpu.setCSRX(CAUSE, CAUSE_INTR_SW);
    cpu.setCSRX(STATUS, cpu.getCSRX(STATUS) & (~0x4));
    cpu.setGPRX(PC, cpu.getCSRX(HANDLER));
}

void Emulator::handleCall(uint8_t mode, Reg regA, Reg regB, Reg, Elf32_Sword disp) {
    push(cpu.getGPRX(PC));
    switch (mode) {
        case MODE_CALL:
            cpu.setGPRX(PC, cpu.getGPRX(regA) + cpu.getGPRX(regB) + disp);
            break;
        case MODE_CALL_IND:
            cpu.setGPRX(PC, LDMEM(cpu.getGPRX(regA) + cpu.getGPRX(regB) + disp));
            break;
        default:
            badInstrIntrPending = true;
    }
}

void Emulator::handleJmp(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp) {
    bool shouldJump = false;
    switch (mode) {
        case MODE_JMP: 
            shouldJump = true; break;
        case MODE_BEQ: 
            if (cpu.getGPRX(regB) == cpu.getGPRX(regC)) shouldJump = true; break;
        case MODE_BNE: 
            if (cpu.getGPRX(regB) != cpu.getGPRX(regC)) shouldJump = true; break;
        case MODE_BGT: 
            if (static_cast<Elf32_Sword>(cpu.getGPRX(regB)) > static_cast<Elf32_Sword>(cpu.getGPRX(regC))) 
                shouldJump = true; break;
        case MODE_JMP_IND: 
            cpu.setGPRX(PC, LDMEM(cpu.getGPRX(regA) + disp)); break;
        case MODE_BEQ_IND: 
            if (cpu.getGPRX(regB) == cpu.getGPRX(regC)) cpu.setGPRX(PC, LDMEM(cpu.getGPRX(regA) + disp)); break;
        case MODE_BNE_IND: 
            if (cpu.getGPRX(regB) != cpu.getGPRX(regC)) cpu.setGPRX(PC, LDMEM(cpu.getGPRX(regA) + disp)); break;
        case MODE_BGT_IND: 
            if (static_cast<Elf32_Sword>(cpu.getGPRX(regB)) > static_cast<Elf32_Sword>(cpu.getGPRX(regC))) 
                cpu.setGPRX(PC, LDMEM(cpu.getGPRX(regA) + disp)); break;
        default: badInstrIntrPending = true;
    }
    if (shouldJump) {
        cpu.setGPRX(PC, cpu.getGPRX(regA) + disp);
    }
}

void Emulator::handleXchg(uint8_t, Reg, Reg regB, Reg regC, Elf32_Sword) {
    Elf32_Word temp = cpu.getGPRX(regB);
    cpu.setGPRX(regB, cpu.getGPRX(regC));
    cpu.setGPRX(regC, temp);
}

void Emulator::handleAlu(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword) {
    switch (mode) {
        case MODE_ADD: 
            cpu.setGPRX(regA, cpu.getGPRX(regB) + cpu.getGPRX(regC)); break;
        case MODE_SUB: 
            cpu.setGPRX(regA, cpu.getGPRX(regB) - cpu.getGPRX(regC)); break;
        case MODE_MUL: 
            cpu.setGPRX(regA, cpu.getGPRX(regB) * cpu.getGPRX(regC)); break;
        case MODE_DIV:
            if (cpu.getGPRX(regC) == 0) badInstrIntrPending = true;
            else cpu.setGPRX(regA, cpu.getGPRX(regB) / cpu.getGPRX(regC));
            break;
        default: badInstrIntrPending = true;
    }
}

void Emulator::handleLog(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword) {
    switch (mode) {
        case MODE_NOT: cpu.setGPRX(regA, ~cpu.getGPRX(regB)); break;
        case MODE_AND: cpu.setGPRX(regA, cpu.getGPRX(regB) & cpu.getGPRX(regC)); break;
        case MODE_OR: cpu.setGPRX(regA, cpu.getGPRX(regB) | cpu.getGPRX(regC)); break;
        case MODE_XOR: cpu.setGPRX(regA, cpu.getGPRX(regB) ^ cpu.getGPRX(regC)); break;
        default: badInstrIntrPending = true;
    }
}

void Emulator::handleShf(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword) {
    switch (mode) {
        case MODE_SHL: cpu.setGPRX(regA, cpu.getGPRX(regB) << cpu.getGPRX(regC)); break;
        case MODE_SHR: cpu.setGPRX(regA, cpu.getGPRX(regB) >> cpu.getGPRX(regC)); break;
        default: badInstrIntrPending = true;
    }
}

void Emulator::handleSt(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp) {
    switch (mode) {
        case MODE_ST_REGIND_DSP:
            STMEM(cpu.getGPRX(regA) + cpu.getGPRX(regB) + disp, cpu.getGPRX(regC));
            break;
        case MODE_ST_REGIND_PTR_UPD:
            cpu.setGPRX(regA, cpu.getGPRX(regA) + disp);
            STMEM(cpu.getGPRX(regA), cpu.getGPRX(regC));
            break;
        case MODE_ST_MEMIND_REGIND_DSP:
            STMEM(LDMEM(cpu.getGPRX(regA) + cpu.getGPRX(regB) + disp), cpu.getGPRX(regC));
            break;
        default:
            badInstrIntrPending = true;
    }
}

void Emulator::handleLd(uint8_t mode, Reg regA, Reg regB, Reg regC, Elf32_Sword disp) {
    switch (mode) {
        case MODE_LD_GPR_CSR: cpu.setGPRX(regA, cpu.getCSRX(regB)); break;
        case MODE_LD_GPR_GPR_DSP: cpu.setGPRX(regA, cpu.getGPRX(regB) + disp); break;
        case MODE_LD_REGIND_DSP: cpu.setGPRX(regA, LDMEM(cpu.getGPRX(regB) + cpu.getGPRX(regC) + disp)); break;
        case MODE_LD_REGIND_PTR_UPD: cpu.setGPRX(regA, LDMEM(cpu.getGPRX(regB))); cpu.setGPRX(regB, cpu.getGPRX(regB) + disp); break;
        case MODE_LD_CSR_GPR: cpu.setCSRX(regA, cpu.getGPRX(regB)); break;
        case MODE_LD_CSR_OR: cpu.setCSRX(regA, cpu.getCSRX(regB) | disp); break;
        case MODE_LD_CSR_REGIND_DSP: cpu.setCSRX(regA, LDMEM(cpu.getGPRX(regB) + cpu.getGPRX(regC) + disp)); break;
        case MODE_LD_CSR_REGIND_PTR_UPD: cpu.setCSRX(regA, LDMEM(cpu.getGPRX(regB))); cpu.setGPRX(regB, cpu.getGPRX(regB) + disp); break;
        default: badInstrIntrPending = true;
    }
}

void Emulator::handleBadOpCode(uint8_t, Reg, Reg, Reg, Elf32_Sword) {
    badInstrIntrPending = true;
}


