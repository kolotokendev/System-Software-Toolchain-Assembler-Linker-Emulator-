#include "../../inc/common/Instruction.hpp"
#include <stdexcept>

const std::unordered_map<yytokentype, Instruction::InstructionDescriptor> Instruction::descriptors = {
    {HALT,       {HALT_OP}},
    {INT,        {INT_OP}},
    {XCHG,       {XCHG_OP, 0, R0}},
    {NOP,        {LOG_OP, MODE_XOR, R0, R0, R0}},

    {CALL,       {CALL_OP, MODE_CALL}},
    {CALL_IND,   {CALL_OP, MODE_CALL_IND}},

    {JMP,        {JMP_OP, MODE_JMP}},
    {BEQ,        {JMP_OP, MODE_BEQ}},
    {BNE,        {JMP_OP, MODE_BNE}},
    {BGT,        {JMP_OP, MODE_BGT}},
    {JMP_IND,    {JMP_OP, MODE_JMP_IND}},
    {BEQ_IND,    {JMP_OP, MODE_BEQ_IND}},
    {BNE_IND,    {JMP_OP, MODE_BNE_IND}},
    {BGT_IND,    {JMP_OP, MODE_BGT_IND}},

    {ADD,        {ALU_OP, MODE_ADD}},
    {SUB,        {ALU_OP, MODE_SUB}},
    {MUL,        {ALU_OP, MODE_MUL}},
    {DIV,        {ALU_OP, MODE_DIV}},

    {NOT,        {LOG_OP, MODE_NOT}},
    {AND,        {LOG_OP, MODE_AND}},
    {OR,         {LOG_OP, MODE_OR}},
    {XOR,        {LOG_OP, MODE_XOR}},

    {SHL,        {SHF_OP, MODE_SHL}},
    {SHR,        {SHF_OP, MODE_SHR}},

    {ST,         {ST_OP, MODE_ST_REGIND_DSP}},
    {ST_DSP,     {ST_OP, MODE_ST_REGIND_DSP}},
    {PUSH,       {ST_OP, MODE_ST_REGIND_PTR_UPD, SP, R0, NO_DEFAULT, -4}},
    {ST_IND,     {ST_OP, MODE_ST_MEMIND_REGIND_DSP}},

    {LD,         {LD_OP, MODE_LD_REGIND_DSP}},
    {LD_DSP,     {LD_OP, MODE_LD_REGIND_DSP}},
    {LD_PCREL,   {LD_OP, MODE_LD_REGIND_DSP, NO_DEFAULT, PC}},
    {LD_REG,     {LD_OP, MODE_LD_GPR_GPR_DSP, NO_DEFAULT, NO_DEFAULT, R0}},
    {POP,        {LD_OP, MODE_LD_REGIND_PTR_UPD, NO_DEFAULT, SP, NO_DEFAULT, 4}},
    {POP_DSP,    {LD_OP, MODE_LD_REGIND_PTR_UPD, NO_DEFAULT, SP, R0}},
    {LD_DSP_CSR, {LD_OP, MODE_LD_CSR_REGIND_DSP}},
    {POP_CS,     {LD_OP, MODE_LD_CSR_REGIND_PTR_UPD, NO_DEFAULT, SP, NO_DEFAULT, 4}},
    {POP_CS_DSP, {LD_OP, MODE_LD_CSR_REGIND_PTR_UPD, NO_DEFAULT, SP, R0}},
    {CSRRD,      {LD_OP, MODE_LD_GPR_CSR}},
    {CSRWR,      {LD_OP, MODE_LD_CSR_GPR}}
};

Ins Instruction::createIns(yytokentype token, const std::vector<int16_t>& parameters) {
    auto it = descriptors.find(token);
    if (it == descriptors.end()) {
        throw std::runtime_error("Couldn't find instruction descriptor!");
    }

    const auto& desc = it->second;
    Ins ins32 = 0u;
    unsigned int paramIndex = 0;

    setOpcode(ins32, desc.opcode);
    setMode(ins32, desc.mode);

    if (desc.defaultRegA != NO_DEFAULT) {
        setRegA(ins32, desc.defaultRegA);
    } else if (paramIndex < parameters.size()) {
        setRegA(ins32, parameters[paramIndex++]);
    }
    
    if (desc.defaultRegB != NO_DEFAULT) {
        setRegB(ins32, desc.defaultRegB);
    } else if (paramIndex < parameters.size()) {
        setRegB(ins32, parameters[paramIndex++]);
    }

    if (desc.defaultRegC != NO_DEFAULT) {
        setRegC(ins32, desc.defaultRegC);
    } else if (paramIndex < parameters.size()) {
        setRegC(ins32, parameters[paramIndex++]);
    }

    if (desc.defaultDisp != NO_DEFAULT) {
        setDisp(ins32, desc.defaultDisp);
    } else if (paramIndex < parameters.size()) {
        setDisp(ins32, parameters[paramIndex++]);
    }

    return ins32;
}

yytokentype Instruction::getIndMode(yytokentype token) {
    static const std::unordered_map<yytokentype, yytokentype> modeMap = {
        {JMP, JMP_IND}, {BEQ, BEQ_IND}, {BNE, BNE_IND}, {BGT, BGT_IND}
    };

    auto it = modeMap.find(token);
    if (it == modeMap.end()) {
        throw std::runtime_error("AssemblerErr: illegal indirect mode");
    }
    return it->second;
}

int16_t Instruction::extractDisp(Ins val) {
    uint16_t extracted = val & 0x0fff;
    if (extracted & 0x800) {
        extracted |= 0xf000;
    }
    return (int16_t)(extracted);
}