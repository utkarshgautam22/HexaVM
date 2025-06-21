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

## Complete Instruction Reference

### Basic Arithmetic & Logic Instructions

| Instruction | Syntax     | Description                                  | Operands          | Flags Affected  |
|-------------|------------|----------------------------------------------|-------------------|-----------------|
| `nop`       | `nop`      | No operation                                 | None              | None            |
| `lda`       | `lda <val>`| Load immediate 16-bit value into register A  | 16-bit immediate  | None            |
| `ldb`       | `ldb <val>`| Load immediate 16-bit value into register B  | 16-bit immediate  | None            |
| `ldc`       | `ldc <val>`| Load immediate 16-bit value into register C  | 16-bit immediate  | None            |
| `add`       | `add`      | A = A + B                                    | None              | Zero, Negative  |
| `sub`       | `sub`      | A = A - B                                    | None              | Zero, Negative  |
| `mul`       | `mul`      | A = A * B                                    | None              | Zero, Negative  |
| `div`       | `div`      | A = A / B (only if B != 0)                   | None              | Zero, Negative  |
| `mod`       | `mod`      | A = A % B (only if B != 0)                   | None              | Zero, Negative  |
| `and`       | `and`      | A = A & B (bitwise AND)                      | None              | Zero, Negative  |
| `or`        | `or`       | A = A \| B (bitwise OR)                      | None              | Zero, Negative  |
| `xor`       | `xor`      | A = A ^ B (bitwise XOR)                      | None              | Zero, Negative  |
| `not`       | `not`      | A = ~A (bitwise NOT)                         | None              | Zero, Negative  |
| `shl`       | `shl`      | A = A << 1 (shift left)                      | None              | Zero, Negative  |
| `shr`       | `shr`      | A = A >> 1 (shift right)                     | None              | Zero, Negative  |
| `inc`       | `inc <reg>`| Increment register by 1                      | Register (a/b/c)  | None            |

### I/O Operations

| Instruction | Syntax     | Description                               | Operands | Notes                              |
|-------------|------------|-------------------------------------------|----------|-----------------------------------|
| `printa`    | `printa`   | Print register A as a decimal number      | None     | Displays value to standard output  |
| `printc`    | `printc`   | Print register A as an ASCII character    | None     | Converts the low byte to character |
| `ina`       | `ina`      | Input a character into register A         | None     | Reads from standard input          |

### Memory Operations

| Instruction    | Syntax                | Description                                          | Operands                      |
|----------------|------------------------|------------------------------------------------------|-------------------------------|
| `load_a`       | `load_a <addr>`       | A = memory[addr] (16-bit)                            | 16-bit address                |
| `store_a`      | `store_a <addr>`      | memory[addr] = A (16-bit)                            | 16-bit address                |
| `load8_a`      | `load8_a <addr>`      | A = memory[addr] (8-bit, zero-extended)              | 16-bit address                |
| `store8_a`     | `store8_a <addr>`     | memory[addr] = A & 0xFF (lower 8-bit)                | 16-bit address                |
| `mov_mem_imm`  | `mov_mem_imm <addr> <val>` | memory[addr] = immediate 16-bit value           | 16-bit address, 16-bit value  |
| `mov8_mem_imm` | `mov8_mem_imm <addr> <val>` | memory[addr] = immediate 8-bit value           | 16-bit address, 8-bit value   |
| `mov_reg_imm`  | `mov_reg_imm <reg> <val>`  | reg = immediate 16-bit value                    | Register (a/b/c), 16-bit value|
| `mov_reg_reg`  | `mov_reg_reg <reg1> <reg2>`| reg1 = reg2                                     | Two registers                 |
| `mov_reg_mem`  | `mov_reg_mem <reg> <addr>` | reg = memory[addr] (8-bit)                      | Register, 16-bit address      |
| `mov_reg_mem2` | `mov_reg_mem2 <reg> <addr>`| reg = memory[addr] \| (memory[addr+1] << 8)      | Register, 16-bit address      |
| `mov_mem_reg`  | `mov_mem_reg <addr> <reg>` | memory[addr] = reg (16-bit)                     | 16-bit address, register      |
| `load`         | `load <reg> <addr>`        | reg = memory[addr]                              | Register, 16-bit address      |
| `store`        | `store <reg> <addr>`       | memory[addr] = reg                              | Register, 16-bit address      |

### Control Flow Instructions

| Instruction | Syntax        | Description                                             | Operands        |
|-------------|---------------|---------------------------------------------------------|-----------------|
| `jmp`       | `jmp <addr>`  | Jump to address                                         | 16-bit address  |
| `jz`        | `jz <addr>`   | Jump to address if zero flag is set                     | 16-bit address  |
| `jnz`       | `jnz <addr>`  | Jump to address if zero flag is not set                 | 16-bit address  |
| `jn`        | `jn <addr>`   | Jump to address if negative flag is set                 | 16-bit address  |
| `jp`        | `jp <addr>`   | Jump if positive (not negative and not zero)            | 16-bit address  |
| `jeq`       | `jeq <addr>`  | Jump to address if B == C                               | 16-bit address  |
| `jgt`       | `jgt <addr>`  | Jump to address if B > C                                | 16-bit address  |
| `jlt`       | `jlt <addr>`  | Jump to address if B < C                                | 16-bit address  |
| `call`      | `call <addr>` | Push PC to stack and jump to address                    | 16-bit address  |
| `ret`       | `ret`         | Pop PC from stack (return from function)                | None            |

### Stack Operations

| Instruction | Syntax     | Description                                  | Operands |
|-------------|------------|----------------------------------------------|----------|
| `push_a`    | `push_a`   | Push register A onto the stack               | None     |
| `pop_a`     | `pop_a`    | Pop value from stack into register A         | None     |
| `push_b`    | `push_b`   | Push register B onto the stack               | None     |
| `pop_b`     | `pop_b`    | Pop value from stack into register B         | None     |

### Comparison Operations

| Instruction | Syntax     | Description                                      | Operands | Flags Affected     |
|-------------|------------|--------------------------------------------------|----------|-------------------|
| `cmp`       | `cmp`      | Compare B and C, set flags                       | None     | Zero, Negative    |

### System & Miscellaneous

| Instruction | Syntax          | Description                                  | Operands         |
|-------------|-----------------|----------------------------------------------|------------------|
| `wait`      | `wait <cycles>` | Wait for the specified number of cycles      | 8-bit immediate  |
| `syscall`   | `syscall`       | System call (A = call number)                | None             |
| `int`       | `int <num>`     | Trigger interrupt with specified number      | 8-bit immediate  |
| `reset`     | `reset`         | Reset CPU state                              | None             |
| `halt`      | `halt`          | Halt CPU execution (stop program)            | None             |

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
    lda 77            ; ASCII 'M' 
    printc            ; Print 'M'
    lda 117           ; ASCII 'u'
    printc            ; Print 'u'
    lda 108           ; ASCII 'l'
    printc            ; Print 'l'
    lda 116           ; ASCII 't'
    printc            ; Print 't'
    lda 105           ; ASCII 'i'
    printc            ; Print 'i'
    lda 112           ; ASCII 'p'
    printc            ; Print 'p'
    lda 108           ; ASCII 'l'
    printc            ; Print 'l'
    lda 101           ; ASCII 'e'
    printc            ; Print 'e'
    lda 115           ; ASCII 's'
    printc            ; Print 's'
    lda 32            ; ASCII Space
    printc            ; Print ' '
    lda 111           ; ASCII 'o'
    printc            ; Print 'o'
    lda 102           |
