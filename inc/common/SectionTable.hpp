#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Elf.hpp"

using namespace std;


struct SectionTable {
    Elf32_Shdr &get(Elf32_Section s);

    Elf32_Shdr *get(const string &s);

    Elf32_Section getSectionIndex(const string &s);

    vector<Elf32_Shdr> sectionDefs;

    unordered_map<string, Elf32_Section> sectionIndices;

    const Elf32_Shdr &get(Elf32_Section s) const;

    Elf32_Shdr& add(const Elf32_Shdr& newSection = {}, const std::string& name = "");

    void finalizeLastSection(Elf32_Addr endLocation);

};