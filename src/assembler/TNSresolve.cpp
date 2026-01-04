#include "../../inc/assembler/Assembler.hpp"

#include <stack>
#include <unordered_map>

void Assembler::tryToResolveTNS() {
    unordered_map<string, int> inDegree;
    unordered_map<string, vector<string>> dependents;

    for (auto const& pair : TNS) {
        const string& symbolName = pair.first;
        const TNSEntry& tnsEntry = pair.second;

        if (inDegree.find(symbolName) == inDegree.end()) {
            inDegree[symbolName] = 0;
        }

        for (const auto& dependencySymbol : tnsEntry.syms) {
            Elf32_Sym* sd = eFile.symbolTable.get(dependencySymbol);
            if (sd && sd->st_shndx != SHN_UNDEF) {
                continue;
            }
            inDegree[symbolName]++;
            dependents[dependencySymbol].push_back(symbolName);
        }
    }

    queue<string> readyQueue;
    for (auto const& pair : inDegree) {
        const string& symbolName = pair.first;
        int degree = pair.second;
        if (degree == 0) {
            readyQueue.push(symbolName);
        }
    }

    size_t resolvedCount = 0;
    while (!readyQueue.empty()) {
        string currentSymbol = readyQueue.front();
        readyQueue.pop();
        resolvedCount++;

        auto& tnsEntry = TNS.at(currentSymbol);

        stack<Elf32_Word> s;
        stack<Elf32_Sym*> symS;
        unordered_map<Elf32_Section, Elf32_Sword> clsNdx;
        unordered_map<Elf32_Section, Elf32_Sym*> boundSym;
        vector<size_t> indices = {0, 0, 0};

        for (auto &selector : tnsEntry.selector) {
            switch (selector) {
                case OP: {
                    int signB = 1;
                    Elf32_Word b = s.top(); s.pop();
                    Elf32_Sym* symB = symS.top(); symS.pop();
                    char op = tnsEntry.operators[indices[OP]++];
                    Elf32_Word res;

                    if (!s.empty()) {
                        Elf32_Word a = s.top(); s.pop();
                        symS.pop(); 
                        if (op == '+') res = a + b;
                        else { res = a - b; signB = -1; }
                    } else {
                        res = b;
                        if (op == '-') { res = -b; signB = -1; }
                    }
                    
                    if (symB != nullptr && symB->st_shndx != SHN_ABS) {
                        clsNdx[symB->st_shndx] += signB;
                        boundSym[symB->st_shndx] = symB;
                    }
                    s.push(res);
                    symS.push(nullptr);
                    break;
                }
                case NUM:
                    s.push(tnsEntry.nums[indices[NUM]++]);
                    symS.push(nullptr);
                    break;
                case SYM: {
                    Elf32_Sym* sd = eFile.symbolTable.get(tnsEntry.syms[indices[SYM]++]);
                    if (sd == nullptr || (sd->st_shndx == SHN_UNDEF && ELF32_ST_BIND(sd->st_info) != STB_GLOBAL)) {
                         throw runtime_error("Invalid state in TNS resolver for symbol " + currentSymbol);
                    }
                    if (s.empty() && sd->st_shndx != SHN_ABS) {
                        clsNdx[sd->st_shndx]++;
                        boundSym[sd->st_shndx] = sd;
                    }
                    s.push(sd->st_value);
                    symS.push(sd);
                    break;
                }
            }
        }

        Elf32_Section sndx = SHN_ABS;
        Elf32_Sym *bSym = nullptr;
        for (auto &cls : clsNdx) {
            if (cls.second != 0) {
                if (sndx == SHN_ABS) {
                    sndx = cls.first;
                    bSym = boundSym[cls.first];
                } else {
                    throw runtime_error("AssemblerErr: Invalid EQU directive: bad index classification for " + currentSymbol);
                }
            }
        }
        
        if (sndx == SHN_ABS) {
            insertSymbolABS(currentSymbol, s.top());
        } else if (sndx != SHN_UNDEF) {
            uint32_t temp_sec = currentSection;
            Elf32_Addr temp_lc = locationCounter;
            currentSection = sndx;
            locationCounter = s.top();
            insertSymbolLOC(currentSymbol);
            currentSection = temp_sec;
            locationCounter = temp_lc;
        } else {
            Elf32_Rela rd;
            rd.r_addend = (Elf32_Sword)(s.top());
            if (!bSym) throw runtime_error("AssemblerErr: Invalid EQU expression depending on external symbol");
            rd.r_info = ELF32_R_INFO(bSym->st_name, R_32);
            rd.r_offset = 0;
            equExtRela[currentSymbol] = rd;
            insertSymbolABS(currentSymbol, 0);
        }

        if (dependents.count(currentSymbol)) {
            for (const auto& dependentSymbol : dependents.at(currentSymbol)) {
                inDegree[dependentSymbol]--;
                if (inDegree[dependentSymbol] == 0) {
                    readyQueue.push(dependentSymbol);
                }
            }
        }
    }

    if (resolvedCount != TNS.size()) {
        throw runtime_error("AssemblerErr: .equ directives have circular dependecies");
    }
}

