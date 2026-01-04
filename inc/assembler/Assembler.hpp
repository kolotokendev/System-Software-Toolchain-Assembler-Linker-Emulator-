#pragma once

#include <fstream>
#include "../common/SectionTable.hpp"
#include "../common/SymbolTable.hpp"
#include "../common/RelocationTable.hpp"
#include "../common/LiteralTable.hpp"
#include "../common/Instruction.hpp"
#include "../../misc/parser.hpp"
#include "../common/ElfFile.hpp"
#include "TNS.hpp"


struct Flink{
    Constant constant;

    Elf32_Addr address;

    yytokentype token;

    vector<int16_t> fields;

    Flink(const Constant& symbol, Elf32_Addr address, yytokentype token, vector<int16_t> fields) : 
        constant(std::move(symbol)), address(address), token(token), fields(fields) {}

    Flink(const Flink& f) : constant(f.constant), address(f.address), token(f.token), fields(f.fields) {}
};


class Assembler {
public:
    uint32_t currentSection = 0;
    
    static Elf32_Addr locationCounter;

    ElfFile eFile;

    vector<vector<Flink>> flinks;

    Elf32_Ehdr elfHeader;

    vector<LiteralTable> literalTable;

    unordered_map<string, TNSEntry> TNS;
    TNSEntry currTNS;

    unordered_map<string, Elf32_Rela> equExtRela;

    string outFile;

    Assembler(string outFile);

    Elf32_Sym *insertSymbolLOC(const string& sym);

    void asciiDirective(string str);

    void skipDirective(Elf32_Word size);

    void insertSection(const string &section);

    void insertSymbol(const string &symbol);

    void insertSymbolABS(const string& sym, Elf32_Addr val);

    void insertSymbolGLOB(const string &sym);

    void insertConstant(const string &sym);

    void insertConstant(Elf32_Word literal);

    void insertInstruction(yytokentype token, const vector<int16_t> &fields = {});

    void insertJmpIns(yytokentype type, const Constant &constant,
                       const vector<int16_t> &fields = {R0, R0, R0});

    void insertCallIns(const Constant &constant);

    void insertLoadIns(yytokentype type, const Constant &poolConstant, vector<int16_t> &&fields);

    void insertStoreIns(yytokentype type, const Constant &poolConstant, vector<int16_t> &&fields);

    void insertIretIns();

    void patchCode();

    void patchInstruction(const Flink& currFlink);

    void patchCallIns(const Flink& currFlink);

    void patchJmpIns(const Flink& currFlink);

    void patchLoadIns(Flink currFlink);

    void patchStoreIns(Flink currFlink);

    void insertInstructionOnAddress(yytokentype token, const vector<int16_t> &fields, Elf32_Addr addr);

    void printFlinks();

    void endDirective();

    void insertTNS(char op){
        currTNS.insertValue(op);
    }

    void insertTNS(const string &symbol){
        currTNS.insertValue(symbol);
    }

    void insertTNS(Elf32_Word literal){
        currTNS.insertValue(literal);
    }

    void closeTNSEntry(const string &symbol){
        TNS[symbol] = currTNS;
        currTNS = {};
    }

    void tryToResolveTNS();

    void insertFlink(const Flink& flink);

    Flink removeFlink();

    bool hasMoreFlinks();

    Elf32_Word generateRelocation(const string &symbol);

    Elf32_Addr getPoolConstantAddr(const Constant &constant);

    void initCurrentSectionPoolConstants();

     static void incLocationCounter(Elf32_Word bytes = 4) {
        locationCounter += bytes;
    }
    
};