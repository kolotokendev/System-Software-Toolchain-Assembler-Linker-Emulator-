#include "../../inc/common/SectionTable.hpp"

#include <iostream>

using namespace std;


Elf32_Shdr* SectionTable::get(const std::string& name) {
    const auto it = sectionIndices.find(name);
    if (it == sectionIndices.end()) {
        return nullptr;
    }
    return &sectionDefs.at(it->second);
}

Elf32_Shdr &SectionTable::get(Elf32_Section s) {
    return sectionDefs[s];
}

const Elf32_Shdr &SectionTable::get(Elf32_Section s) const {
    return sectionDefs[s];
}

Elf32_Shdr& SectionTable::add(const Elf32_Shdr& newSection, const std::string& name) {
    sectionDefs.emplace_back(newSection);
    const Elf32_Section newIndex = sectionDefs.size() - 1;

    if (!name.empty()) {
        sectionIndices[name] = newIndex;
    }

    return sectionDefs.back();
}

Elf32_Section SectionTable::getSectionIndex(const string &name){
    const auto it = sectionIndices.find(name);
    if (it == sectionIndices.end()) {
        throw std::runtime_error("AssemblerErr: Section '" + name + "' not found in symbol table.");
    }
    return it->second;
}

void SectionTable::finalizeLastSection(Elf32_Addr finalSize) {
    if (sectionDefs.empty()) {
        throw std::runtime_error("AssemblerErr: Cannot finalize section size, no sections exist.");
    }
    sectionDefs.back().sh_size = finalSize;
}


