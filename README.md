# HexaVM: A 16-bit Virtual Machine & Assembler

**Developed by: Utkarsh Gautam**

## Overview

HexaVM is a lightweight 16-bit virtual machine and assembler implementation that provides a complete environment for executing assembly-like instructions. This project simulates a simple computer architecture with registers, memory, and a comprehensive instruction set for executing programs written in custom assembly language.

## Features

### Virtual Machine Architecture

- **CPU**: 16-bit architecture with three general-purpose registers (A, B, C)
- **Memory**: 64KB (0xFFFF bytes) addressable memory space
- **Program Counter**: 16-bit program counter (PC) for instruction execution
- **Stack**: Support for function calls and stack operations (PUSH/POP)
- **Flags**: Status flags for comparison and conditional operations (zero_flag, negative_flag)

### Instruction Set

The instruction set is categorized into several groups:

#### Arithmetic & Math Operations
- Basic operations: ADD, SUB, MUL, DIV, MOD
- Bit manipulation: AND, OR, XOR, NOT, SHL, SHR

#### Control Flow
- Jump instructions: JMP, JZ, JNZ, JN, JP
- Conditional jumps: JEQ, JGT, JLT
- Function handling: CALL, RET

#### I/O Operations
- PRINT_A: Print register A as a number
- PRINT_CHAR: Print register A as an ASCII character
- IN_A: Input to register A

#### Memory Access
- Load/Store operations with different sizes (8-bit, 16-bit)
- Register-memory operations
- Immediate value operations

#### Stack Operations
- PUSH_A, POP_A, PUSH_B, POP_B
- Stack-based function call management

#### System Operations
- HALT: Stop program execution
- RESET: Reset CPU state
- WAIT: Wait for N cycles
- SYSCALL/INT: System calls and interrupts

### Assembler

The project includes a fully functional two-pass assembler that:

1. Processes assembly language source code
2. Handles labels and forward references
3. Translates mnemonics to machine code
4. Provides debug information through memory dumps
5. Saves assembled programs as binary files

## Getting Started

### Prerequisites

- C++ compiler supporting C++11 or later (g++ recommended)
- Linux/Unix environment (for terminal-based execution)

### Building the VM

```bash
# Clone the repository (if applicable)
git clone <repository-url>
cd vm

# Build the VM using the build script
./build.sh
```

### Running Assembly Programs

```bash
# Assemble and run a program
./vm my_program.asm -r

# Assemble only, saving to a binary file
./vm my_program.asm output.bin
```

## Assembly Language Syntax

### Basic Structure

```assembly
; Comment line
.org 0x9000    ; Set the starting memory address

label:         ; Define a label
    instruction [operands]  ; An instruction with optional operands
    
; Example program
start:
    lda 10     ; Load immediate value 10 into register A
    ldb 20     ; Load immediate value 20 into register B
    add        ; A = A + B
    printa     ; Print value in register A
    halt       ; End program
```

### Memory Organization

- Default program start: 0x9000
- Stack: Grows downward from higher memory addresses
- Program code: Placed starting at .org directive address

## Example Programs

### Sample 1: Countdown

```assembly
; Countdown program
.org 0x9000

    lda 10        ; Start countdown from 10
loop:
    printa        ; Print current value
    sub 1         ; Decrease by 1
    jz done       ; If zero, we're done
    jmp loop      ; Otherwise, continue counting
done:
    printa        ; Print the final 0
    halt          ; End program
```

### Sample 2: Multiplication Table

```assembly
; Multiplication table of 10
.org 0x9000

; Print table header
    lda 77        ; ASCII 'M' 
    printc
    lda 117       ; ASCII 'u'
    printc
    ; ... more printc operations ...

; Table calculation
    lda 1         ; Start with 1
loop:
    printa        ; Print the counter
    ; ... print " x 10 = " ...
    
    ldb 10        ; B = 10
    mul           ; A = A * B
    printa        ; Print result
    
    ; ... increment counter and check for end ...
    jmp loop
```

## Debug Features

The VM provides comprehensive debug information:
- Initial and final CPU state (registers, PC)
- Memory dumps in hexadecimal format
- Program output visualization
- Error reporting for assembly process

## Architecture Diagram

```
+----------------+        +----------------+        +----------------+
| Assembly Code  |------->|    Assembler   |------->| Machine Code   |
| (.asm file)    |        | (Two-pass)     |        | (Binary)       |
+----------------+        +----------------+        +----------------+
                                                           |
                                                           v
                    +---------------+        +---------------+
                    | CPU Execution |<-------| Memory        |
                    | (Fetch-Decode-| -----> | (64KB)        |
                    |  Execute)     |        |               |
                    +---------------+        +---------------+
                         |     ^
                         v     |
                    +---------------+
                    | I/O Subsystem |
                    | (Terminal)    |
                    +---------------+
```

## Implementation Details

- **CPU Implementation**: Classic fetch-decode-execute cycle in cpu.cpp
- **Memory Model**: Linear byte-addressable memory in setup.cpp
- **Assembler**: Two-pass assembler for resolving symbols in assembler.cpp
- **Program Execution**: Main control flow in run.cpp

## Future Enhancements

- Floating-point operations
- More complex memory addressing modes
- Enhanced debugging tools
- GUI-based visual debugger
- External device emulation

## License

This project is licensed under the MIT License - see the LICENSE file for details.

---

© 2025 Utkarsh Gautam. All rights reserved.
