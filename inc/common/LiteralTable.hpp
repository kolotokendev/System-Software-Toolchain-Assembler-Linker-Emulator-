#pragma once

#include "Elf.hpp"

#include <unordered_map>
#include <queue>
#include <utility>
#include <iomanip>

struct Constant {
    Constant(Elf32_Word num) : number(num), isNumeric(true) {}

    Constant(string sym) : symbol(std::move(sym)), isNumeric(false) {}

    Constant(char *sym) : symbol(sym), isNumeric(false) {}

    void printConstant(){
        if(isNumeric){
            cout << "0x" << hex << number;
        }else{
            cout << symbol;
        }
    }

    Elf32_Word number = 0;
    string symbol;

    bool isNumeric = true;
};

class LiteralTable {
public:
    void insertConstant(const Constant &constant, Elf32_Addr addr) {
        if (constant.isNumeric) {
            numericMap[constant.number] = addr;
        } else {
            symbolMap[constant.symbol] = addr;
        }
        poolConstantsQueue.push(constant);
    }

    Elf32_Word getConstantAddress(const Constant &constant) {
        if (constant.isNumeric) {
            return numericMap[constant.number];
        } else {
            return symbolMap[constant.symbol];
        }
    }

    bool hasConstant(const Constant &constant) {
        if (constant.isNumeric) {
            return numericMap.find(constant.number) != numericMap.end();
        } else {
            return symbolMap.find(constant.symbol) != symbolMap.end();
        }
    }

    Constant getNextPoolConstant() {
        Constant ret = poolConstantsQueue.front();
        poolConstantsQueue.pop();
        return ret;
    }

    bool hasNextPoolConstant() {
        return !poolConstantsQueue.empty();
    }

private:
    std::unordered_map<Elf32_Word, Elf32_Word> numericMap;
    std::unordered_map<string, Elf32_Word> symbolMap;

    std::queue<Constant> poolConstantsQueue;

};

