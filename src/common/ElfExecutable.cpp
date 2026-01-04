#include "../../inc/common/ElfFile.hpp"

void ElfFile::writeExecutableToFile(const string &fileName) {
    fstream file{fileName, ios::out | ios::binary};

    elfHeader.e_type = ET_EXEC;
    elfHeader.e_phnum = dataSections.size();

    file.write((char*)(&elfHeader), sizeof(Elf32_Ehdr));

    writeExecDataSections(file);

    elfHeader.e_phoff = file.tellp();

    for (auto &ph: programTable.progDefs) {
        file.write((char*)(&ph), sizeof(Elf32_Phdr));
    }

    file.seekg(SEEK_SET, ios::beg);
    file.write((char*)(&elfHeader), sizeof(Elf32_Ehdr));
}


void ElfFile::loadSection(const Elf32_Phdr &ph, Elf32_Section &currDataSection, fstream &file) {
    stringstream s;
    file.seekg(ph.p_offset);
    vector<char> v;
    v.resize(ph.p_memsz);
    file.read(v.data(), ph.p_memsz);
    s.write(v.data(), ph.p_memsz);
    dataSections[currDataSection++] = std::move(s);
}


void ElfFile::writeExecDataSections(fstream &file) {
    for (size_t i = 1; i <= dataSections.size(); i++) {
        stringstream &s = dataSections[i];

        Elf32_Shdr &sh = sectionTable.get(i);

        Elf32_Phdr ph;
        ph.p_memsz = sh.sh_size;
        ph.p_offset = file.tellp();
        ph.p_paddr = sh.sh_addr;

        programTable.add(ph);

        file << s.rdbuf();
    }
}

void ElfFile::loadExecFile(fstream &file) {
    file.seekg(elfHeader.e_phoff, ios::beg);

    for (Elf32_Half i = 0; i < elfHeader.e_phnum; i++) {
        Elf32_Phdr ph;
        file.read((char*)(&ph), sizeof(Elf32_Phdr));

        programTable.add(ph);
    }

    Elf32_Section currDataSection = 1;
    for (auto &ph: programTable.progDefs) {
        loadSection(ph, currDataSection, file);
    }
}