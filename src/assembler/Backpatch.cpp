#include "../../inc/assembler/Assembler.hpp"
#include "../../inc/common/LiteralTable.hpp"
#include <iomanip>

void Assembler::patchCode(){
    currentSection = 0;
    while(currentSection < flinks.size()){
        locationCounter = eFile.sectionTable.get(currentSection).sh_size;
        for(auto& currFlink: flinks[currentSection]){
            patchInstruction(currFlink);
        }
        currentSection++;
    }
}

void Assembler::patchInstruction(const Flink& currFlink){
    switch (currFlink.token) {
        case CALL:
            patchCallIns(currFlink);
            break;
        case BEQ:
        case BNE:
        case BGT:
        case JMP:
            patchJmpIns(currFlink);
            break;    
        case LD_REG:
        case LD:
        case LD_DSP:
            patchLoadIns(currFlink);
            break;
        case ST:
        case ST_DSP:
            patchStoreIns(currFlink);
            break;
        default:
            throw runtime_error("AssemblerErr: Unknown instruction type in forward links " + currFlink.token);
    }
}

void Assembler::patchCallIns(const Flink& currFlink){
    if(!currFlink.constant.isNumeric){
        Elf32_Sym *sd = eFile.symbolTable.get(currFlink.constant.symbol);

        if (sd->st_shndx == SHN_ABS && Disp::isWithinLimit(sd->st_value)) {
            insertInstructionOnAddress(CALL, {R0, R0, R0, (int16_t)(sd->st_value)}, currFlink.address);
            return;
        }
        else if (sd->st_shndx == currentSection) {
            int16_t disp = Disp::getOffset(sd->st_value, currFlink.address + 4);
            insertInstructionOnAddress(CALL, {PC, R0, R0, disp}, currFlink.address);
            return;
        }
    }

    Elf32_Addr constant = getPoolConstantAddr(currFlink.constant);
    int16_t dest = Disp::getOffset(constant, currFlink.address + 4);
    insertInstructionOnAddress(CALL_IND, {PC, R0, R0, dest}, currFlink.address);
}

void Assembler::patchJmpIns(const Flink& currFlink){
    if(!currFlink.constant.isNumeric){
        Elf32_Sym *sd = eFile.symbolTable.get(currFlink.constant.symbol);

        if (sd->st_shndx == SHN_ABS && Disp::isWithinLimit(sd->st_value)) {
            insertInstructionOnAddress(currFlink.token, {R0, currFlink.fields[REG_B], currFlink.fields[REG_C], 
                                        (int16_t)(sd->st_value)}, currFlink.address);
            return;
        } else if (sd->st_shndx == currentSection) {
            int16_t disp = Disp::getOffset(sd->st_value, currFlink.address + 4);
            insertInstructionOnAddress(currFlink.token, {PC, currFlink.fields[REG_B], currFlink.fields[REG_C], disp}
                                                                            , currFlink.address);
            return;
        }
    }

    Elf32_Addr constant = getPoolConstantAddr(currFlink.constant);
    int16_t dest = Disp::getOffset(constant, currFlink.address + 4);

    insertInstructionOnAddress(Instruction::getIndMode(currFlink.token), {PC, currFlink.fields[REG_B], 
                                                            currFlink.fields[REG_C], dest}, currFlink.address);
}

void Assembler::patchLoadIns(Flink currFlink){
    if(!currFlink.constant.isNumeric){
        if (currFlink.token == LD_REG || currFlink.token == LD_DSP) {
            Elf32_Sym *sd = eFile.symbolTable.get(currFlink.constant.symbol);

            if (sd->st_shndx == SHN_ABS && Disp::isWithinLimit(sd->st_value)) {
                currFlink.fields.push_back((int16_t)(sd->st_value));
                insertInstructionOnAddress(currFlink.token, currFlink.fields, currFlink.address);
                return;
            }

            if (sd->st_shndx == currentSection) {
                int16_t disp = Disp::getOffset(sd->st_value, currFlink.address + 4);
                if (currFlink.token == LD_REG) {
                    insertInstructionOnAddress(LD_REG, {currFlink.fields[REG_A], PC, disp}, currFlink.address);
                } else {
                    insertInstructionOnAddress(currFlink.token, {currFlink.fields[REG_A], currFlink.fields[REG_B], PC, disp}, currFlink.address);
                }
                return;
            }
        }
    }

    Elf32_Addr constantAddress = getPoolConstantAddr(currFlink.constant);

    switch (currFlink.token) {
        case LD_REG: {
            int16_t dest = Disp::getOffset(constantAddress, currFlink.address + 4);
            currFlink.fields.push_back(dest);
            insertInstructionOnAddress(LD_PCREL, currFlink.fields, currFlink.address);
            break;
        }
        case LD_DSP:
                throw runtime_error("AssemblerErr: Memind addressing can not fit in displacement");
        default: {
            int16_t dest = Disp::getOffset(constantAddress, currFlink.address + 4);

            insertInstructionOnAddress(LD_PCREL, {currFlink.fields[REG_A], R0, dest}, currFlink.address);
            insertInstructionOnAddress(currFlink.token, {currFlink.fields[REG_A], R0, currFlink.fields[REG_A]}, currFlink.address + 4);
        }
    }
}

void Assembler::patchStoreIns(Flink currFlink){
    if(!currFlink.constant.isNumeric){
        Elf32_Sym *sd = eFile.symbolTable.get(currFlink.constant.symbol);

        if (sd->st_shndx == SHN_ABS && Disp::isWithinLimit(sd->st_value)) {
            currFlink.fields.push_back(static_cast<int16_t>(sd->st_value));
            insertInstructionOnAddress(currFlink.token, currFlink.fields, currFlink.address);
            return;
        }

        if (sd->st_shndx == currentSection) {
            int16_t disp = Disp::getOffset(sd->st_value, currFlink.address + 4);
            insertInstructionOnAddress(currFlink.token,
                              {PC, currFlink.fields[REG_B], currFlink.fields[REG_C], disp}, currFlink.address);
            return;
        }
    }

    if (currFlink.token != ST_DSP) {
        Elf32_Addr poolConstantAddr = getPoolConstantAddr(currFlink.constant);
        int16_t offsetToPoolLiteral = Disp::getOffset(poolConstantAddr, currFlink.address + 4);

        insertInstructionOnAddress(ST_IND, {PC, R0, currFlink.fields[REG_C], offsetToPoolLiteral}, currFlink.address);
    } else {
        throw runtime_error("AssemblerErr: Memind addressing can not fit in displacement");
    }
}


void Assembler::insertInstructionOnAddress(yytokentype token, const vector<int16_t> &fields, Elf32_Addr address){
    Ins ins32 = Instruction::createIns(token, fields);
    eFile.dataSections[currentSection].seekp(address);
    eFile.dataSections[currentSection].write((char*)(&ins32), sizeof(Ins));
}


void Assembler::printFlinks(){
    currentSection = 0;
    while(currentSection < flinks.size()){
        for(auto& currFlink: flinks[currentSection]){
            cout << "symbol/number: "; currFlink.constant.printConstant(); cout << endl;
            cout << "address: " << "0x" << hex << currFlink.address; cout << endl;
            cout << "instruction: " << currFlink.token; cout << endl;
            cout << "--------------------------"; cout << endl;
        }
        currentSection++;
    }
}