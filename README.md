# System-Software-Toolchain-Assembler-Linker-Emulator-
This project implements a complete software development toolchain and an emulator for a custom abstract computer system.  It includes an assembler, a linker, and an emulator capable of executing binary code.

# Features
- One-Pass Assembler Design: Converts assembly source code into relocatable object files.
- Symbol Management: Handles local and global symbols, section relocation, and backpatching.
- Directives: Supports .global, .extern, .section, .word, .skip, .ascii, and .end.
- Object Merging: Combines multiple object files into a single executable or relocatable file.
- Section Placement: Supports placing sections at specific memory addresses via command-line arguments.
- Address Resolution: Performs final relocation and resolves all undefined symbols.
- Instruction Execution: Simulates a 32-bit RISC-V processor with 16 general-purpose registers.
- Memory-Mapped I/O: Simulates peripheral devices including a terminal (input/output) and a system timer.
- Interrupt System: Handles hardware interrupts, software traps, and illegal instruction exceptions.
