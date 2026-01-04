#pragma once

#include <cstddef>


constexpr Elf32_Addr TERM_OUT = 0xFFFFFF00;

constexpr Elf32_Addr TERM_IN = 0xFFFFFF04;

constexpr Elf32_Addr TIM_CFG = 0xFFFFFF10;

constexpr Elf32_Word START_TIM_CFG = 0x0;

constexpr Elf32_Addr DEFAULT_PC = 0x40000000;

constexpr Elf32_Addr DEFAULT_SP = 0xFFFFFF00;

constexpr size_t MEMORY_SIZE_BYTES = 1ull << 32;

constexpr size_t NUM_GPRX_REGS = 16;

constexpr size_t NUM_CSRX_REGS = 3;

constexpr Elf32_Word MODE_COUNT = 8;

constexpr Elf32_Word TIMER_MASK_BIT = (1u << 0);

constexpr Elf32_Word TERMINAL_MASK_BIT = (1u << 1);

constexpr Elf32_Word GLOBAL_MASK_BIT = (1u << 2);

constexpr size_t TIMER_MODES[MODE_COUNT] = { 
        500000, 
        1000000, 
        1500000, 
        2000000,
        5000000, 
        10000000, 
        30000000, 
        60000000
};

enum {
    CAUSE_NO_INTR = 0,
    CAUSE_BAD_INSTR,
    CAUSE_INTR_TIMER,
    CAUSE_INTR_TERMINAL,
    CAUSE_INTR_SW
};