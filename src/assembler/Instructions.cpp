#include "../../inc/assembler/Assembler.hpp"
#include "../../inc/common/Disp.hpp"
#include <iomanip>

void Assembler::insertInstruction(yytokentype token, const vector<int16_t> &fields) {
    incLocationCounter();
    Ins ins32 = Instruction::createIns(token, fields);
    eFile.dataSections[currentSection].write((char *)(&ins32), sizeof(Ins));
}

void Assembler::insertIretIns() {
    insertInstruction(LD_DSP_CSR, {STATUS, SP, R0, +1 * 4});
    insertInstruction(POP_DSP, {PC, +2 * 4});
}

void Assembler::insertJmpIns(yytokentype type, const Constant &constant,
                              const vector<int16_t> &fields) {
    if (constant.isNumeric) {
        if(Disp::isWithinLimit(constant.number)){
            insertInstruction(type, {R0, fields[REG_B], fields[REG_C], static_cast<int16_t>(constant.number)});
            return;
        }
        else{
            Flink flink = {constant.number, locationCounter, type, fields};
            insertFlink(flink);
            insertInstruction(NOP);
            return;
        }
    }
    insertSymbol(constant.symbol);
    Flink flink = {constant.symbol, locationCounter, type, fields};
    insertFlink(flink);
    insertInstruction(NOP);
}

void Assembler::insertCallIns(const Constant &constant) {
    if (constant.isNumeric) {
        if(Disp::isWithinLimit(constant.number)){
            insertInstruction(CALL, {R0, R0, R0, static_cast<int16_t>(constant.number)});
            return;
        }
        else{
            Flink flink = {constant.number, locationCounter, CALL, {}};
            insertFlink(flink);
            insertInstruction(NOP); 
            return;
        }
    }
    insertSymbol(constant.symbol);
    Flink flink = {constant.symbol, locationCounter, CALL, {}};
    insertFlink(flink);
    insertInstruction(NOP);
}

void Assembler::insertLoadIns(yytokentype type, const Constant &constant, vector<int16_t> &&fields) {
    if (constant.isNumeric) {
        if (Disp::isWithinLimit(constant.number)) {
            fields.push_back(static_cast<int16_t>(constant.number));
            insertInstruction(type, fields);
            return;
        }else{
            if(type == LD_DSP){
                throw runtime_error("AssemblerErr: Memind can not fit into displacement!");
            }
            else if(type == LD_REG){
                Flink flink = {constant.number, locationCounter, type, fields};
                insertFlink(flink);
                insertInstruction(NOP);
                return; 
            }else{
                Flink flink = {constant.number, locationCounter, type, fields};
                insertFlink(flink);
                insertInstruction(NOP);
                insertInstruction(NOP);
                return;
            }
        }
    }
    if(!constant.isNumeric){
        insertSymbol(constant.symbol);
        if(type == LD_REG){
            Flink flink = {constant.symbol, locationCounter, type, fields};
            insertFlink(flink);
            insertInstruction(NOP);
            return; 
        }else{
            Flink flink = {constant.symbol, locationCounter, type, fields};
            insertFlink(flink);
            insertInstruction(NOP);
            insertInstruction(NOP);
            return;
        }
    }
}

void Assembler::insertStoreIns(yytokentype type, const Constant &constant, vector<int16_t> &&fields) {
    if(constant.isNumeric){
        if(Disp::isWithinLimit(constant.number)){
            fields.push_back((int16_t)(constant.number));
            insertInstruction(type, fields);
            return;
        }else{
            if (type != ST_DSP) {
                Flink flink = {constant.number, locationCounter, type, fields};
                insertFlink(flink);
                insertInstruction(NOP);
                return;
            }else {
                throw runtime_error("AssemblerErr: Memind can not fit into displacement!");
            }
        }
    }
    insertSymbol(constant.symbol);
    Flink flink = {constant.symbol, locationCounter, type, fields};
    insertFlink(flink);
    insertInstruction(NOP);
}
