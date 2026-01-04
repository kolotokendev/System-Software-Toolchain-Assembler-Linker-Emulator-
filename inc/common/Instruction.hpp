#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../../misc/parser.hpp"

constexpr int32_t MIN_DISP = ~(0x7FF);
constexpr int32_t MAX_DISP = 0x7FF;

enum { R0 = 0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15 };
enum { SP = 14, PC = 15 };
enum { STATUS = 0, HANDLER, CAUSE };
enum { REG_A = 0, REG_B, REG_C, DISP };
enum { HALT_OP = 0b0000, INT_OP, CALL_OP, JMP_OP, XCHG_OP, ALU_OP, LOG_OP, SHF_OP, ST_OP, LD_OP };
enum { MODE_CALL = 0b0000, MODE_CALL_IND };
enum { MODE_JMP = 0b0000, MODE_BEQ, MODE_BNE, MODE_BGT, MODE_JMP_IND = 0b1000, MODE_BEQ_IND, MODE_BNE_IND, MODE_BGT_IND };
enum { MODE_ADD = 0b0000, MODE_SUB, MODE_MUL, MODE_DIV };
enum { MODE_NOT = 0b0000, MODE_AND, MODE_OR, MODE_XOR };
enum { MODE_SHL = 0b0000, MODE_SHR = 0b0001 };
enum { MODE_ST_REGIND_DSP = 0b0000, MODE_ST_REGIND_PTR_UPD, MODE_ST_MEMIND_REGIND_DSP };
enum { MODE_LD_GPR_CSR = 0b0000, MODE_LD_GPR_GPR_DSP, MODE_LD_REGIND_DSP, MODE_LD_REGIND_PTR_UPD, MODE_LD_CSR_GPR, MODE_LD_CSR_OR, MODE_LD_CSR_REGIND_DSP, MODE_LD_CSR_REGIND_PTR_UPD };

using Ins = uint32_t;
using Reg = uint8_t;

class Instruction {
public:
    static Ins createIns(yytokentype token, const std::vector<int16_t>& fields);
    static yytokentype getIndMode(yytokentype token);

    static uint8_t extractOpCode(Ins val) { return val >> 28; }
    static uint8_t extractMode(Ins val) { return (val & 0x0f000000) >> 24; }
    static Reg extractRegA(Ins val) { return (val & 0x00f00000) >> 20; }
    static Reg extractRegB(Ins val) { return (val & 0x000f0000) >> 16; }
    static Reg extractRegC(Ins val) { return (val & 0x0000f000) >> 12; }
    static int16_t extractDisp(Ins val);

private:
    static constexpr int16_t NO_DEFAULT = -1; 
    
    struct InstructionDescriptor {
        uint8_t opcode;
        uint8_t mode = 0;
        int16_t defaultRegA = NO_DEFAULT;
        int16_t defaultRegB = NO_DEFAULT;
        int16_t defaultRegC = NO_DEFAULT;
        int16_t defaultDisp = NO_DEFAULT;
    };

    static const std::unordered_map<yytokentype, InstructionDescriptor> descriptors;

    static void setOpcode(Ins& val, uint8_t opcode) { val = (val & 0x0fffffff) | (opcode << 28); }
    static void setMode(Ins& val, uint8_t mode) { val = (val & 0xf0ffffff) | (mode << 24); }
    static void setRegA(Ins& val, Reg op_a) { val = (val & 0xff0fffff) | (op_a << 20); }
    static void setRegB(Ins& val, Reg op_b) { val = (val & 0xfff0ffff) | (op_b << 16); }
    static void setRegC(Ins& val, Reg op_c) { val = (val & 0xffff0fff) | (op_c << 12); }
    static void setDisp(Ins& val, int16_t op_d) { val = (val & 0xfffff000) | (op_d & 0xfff); }
};