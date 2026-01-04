#include "../../inc/common/ElfFile.hpp"

void ElfFile::loadRelocatableFile(fstream &file) {
    file.seekg(elfHeader.e_shoff, ios::beg);

    for (Elf32_Half i = 0; i < elfHeader.e_shnum; i++) {
        Elf32_Shdr sh;
        file.read((char*)(&sh), sizeof(Elf32_Shdr));
        sectionTable.add(sh);
    }

    Elf32_Section currDataSection = 1;
    for (auto &sh: sectionTable.sectionDefs) {
        loadSection(sh, currDataSection, file);
    }
}

void ElfFile::loadSection(const Elf32_Shdr &sh, Elf32_Section &currDataSection, fstream &file) {
    file.seekg(sh.sh_offset);

    switch (sh.sh_type) {
        case SHT_PROGBITS:
        case SHT_NOBITS: {
            if (sh.sh_size > 0) {
                stringstream s;
                vector<char> v;
                v.resize(sh.sh_size);
                file.read(v.data(), sh.sh_size);
                s.write(v.data(), sh.sh_size);
                dataSections[currDataSection++] = std::move(s);
            }
            break;
        }
        case SHT_SYMTAB: {
            Elf32_Word size = sh.sh_size;
            while (size > 0) {
                Elf32_Sym sd;
                file.read((char *)(&sd), sizeof(Elf32_Sym));
                symbolTable.symbolDefinitions.push_back(sd);
                size -= sizeof(Elf32_Sym);
            }
            break;
        }
        case SHT_STRTAB: {
            Elf32_Word size = sh.sh_size;

            Elf32_Word currSym = 0;
            while (size > 0) {
                string s;
                std::getline(file, s, '\0');
                symbolTable.symbolNames.push_back(s);
                symbolTable.symbolMappings[s] = currSym++;

                size -= s.size() + 1;
            }
            break;
        }
        case SHT_RELA: {

            RelocationTable r;

            Elf32_Word size = sh.sh_size;

            while (size > 0) {
                Elf32_Rela rd;
                file.read((char *)(&rd), sizeof(Elf32_Rela));
                r.insertRelocationEntry(rd);

                size -= sizeof(Elf32_Rela);
            }

            relocationTables[sh.sh_link] = r;
            break;
        }
    }
}

void ElfFile::writeRelocatableToFile(const string &fileName) {
    fstream file{fileName, ios::out | ios::binary};

    elfHeader.e_type = ET_REL;

    file.write((char*)(&elfHeader), sizeof(Elf32_Ehdr));


    vector<Elf32_Shdr> additionalHeaders;

    writeRelDataSections(file);

    writeRelocationTables(additionalHeaders, file);

    writeSymbolTable(additionalHeaders, file);

    writeStringTable(additionalHeaders, file);

    elfHeader.e_shoff = file.tellp();

    elfHeader.e_shnum = sectionTable.sectionDefs.size() + additionalHeaders.size();

    for (Elf32_Shdr &sh: sectionTable.sectionDefs) {
        file.write((char*)(&sh), sizeof(sh));
    }

    for (auto &sh: additionalHeaders) {
        file.write((char*)(&sh), sizeof(sh));
    }

    file.seekp(SEEK_SET);
    file.write((char*)(&elfHeader), sizeof(Elf32_Ehdr));
}

void ElfFile::writeRelDataSections(fstream &file) {
    for (size_t i = 1; i <= dataSections.size(); i++) {
        stringstream &s = dataSections[i];
        Elf32_Shdr &sh = sectionTable.get(i);
        sh.sh_offset = file.tellp();
        file << s.rdbuf();
    }
}

