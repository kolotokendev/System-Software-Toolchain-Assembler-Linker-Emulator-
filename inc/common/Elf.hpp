#pragma once

#include <cstdint>

using namespace std;

#define ELFMAG                  "\177ELF"
#define EI_NIDENT               (8)

#define ET_NONE                 0
#define ET_REL                  1
#define ET_EXEC                 2

#define STB_LOCAL               0
#define STB_GLOBAL              1

#define STT_NOTYPE              0
#define STT_SECTION             3

#define ELF32_ST_BIND(val)      (((unsigned char) (val)) >> 4)
#define ELF32_ST_TYPE(val)      ((val) & 0xf)
#define ELF32_ST_INFO(bind, type) (((bind) << 4) + ((type) & 0xf))

#define ELF32_R_SYM(val)        ((val) >> 8)
#define ELF32_R_TYPE(val)       ((val) & 0xff)
#define ELF32_R_INFO(sym, type) (((sym) << 8) + ((type) & 0xff))

#define R_NONE                  0
#define R_PC32                  2
#define R_32                    10

#define SHN_UNDEF               0
#define SHN_ABS                 0xfff1

#define SHT_NULL                0
#define SHT_PROGBITS            1
#define SHT_SYMTAB              2
#define SHT_STRTAB              3
#define SHT_RELA                4
#define SHT_NOBITS              8

typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Section;
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Addr;

struct Elf32_Ehdr {
    unsigned char e_ident[EI_NIDENT] = {0x7F, 'E', 'L', 'F', 0x00, 0x00, 0x00, 0x00};
    Elf32_Half e_type;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Half e_phnum;
    Elf32_Half e_shnum;
};

struct Elf32_Sym {
    Elf32_Word st_name = 0;
    Elf32_Addr st_value = 0;
    unsigned char st_info = 0;
    Elf32_Section st_shndx = 0;
};

struct Elf32_Rela {
    Elf32_Addr r_offset;
    Elf32_Word r_info;
    Elf32_Sword r_addend;
};

struct Elf32_Shdr {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
};

struct Elf32_Phdr {
    Elf32_Off p_offset;
    Elf32_Addr p_paddr;
    Elf32_Word p_memsz;
};