#ifndef TNSENTRY_HPP
#define TNSENTRY_HPP

#include "../../inc/common/Elf.hpp"
#include <vector>
#include <string>

using namespace std;

enum ValueSelector {
    OP,
    SYM,
    NUM
};

class TNSEntry {
public:
    void insertValue(char c) {
        selector.push_back(OP);
        operators.push_back(c);
    }

    void insertValue(const string &s) {
        selector.push_back(SYM);
        syms.push_back(s);
    }

    void insertValue(Elf32_Word num) {
        selector.push_back(NUM);
        nums.push_back(num);
    }

    bool resolved = false;

    vector<ValueSelector> selector;
    vector<char> operators;
    vector<string> syms;
    vector<Elf32_Word> nums;
};

#endif