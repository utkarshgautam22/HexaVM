#ifndef SETUP_H
#define SETUP_H

#include <cstdint>
#include <stack>
#define MEMORY_MAX 0xffff // in bytes
#define uc unsigned char
#define int8 uint8_t

extern uint8_t memory[MEMORY_MAX];

enum opcodes : uint8_t
{
    // ───────────────────────
    // Basic Arithmetic & Math
    // ───────────────────────
    NOP = 0x00,     // No operation
    LDA_IMM = 0x01, // A = immediate 16-bit
    LDB_IMM = 0x02, // B = immediate 16-bit
    ADD = 0x03,     // A = A + B
    SUB = 0x04,     // A = A - B
    MUL = 0x05,     // A = A * B
    DIV = 0x06,     // A = A / B
    MOD = 0x07,     // A = A % B

    // ───────────────────────
    // I/O
    // ───────────────────────
    PRINT_A = 0x08,    // Print A (for debugging or CLI output)
    PRINT_CHAR = 0x09, // Print A as char (ASCII)
    IN_A = 0x0A,       // A = getchar() or port input

    // ───────────────────────
    // Control Flow
    // ───────────────────────
    JMP = 0x10, // Jump to addr
    JZ = 0x11,  // Jump if Zero flag
    JNZ = 0x12, // Jump if Not Zero
    HLT = 0x13, // Halt CPU (original)
    JN = 0x14,  // Jump if Negative
    JP = 0x15,  // Jump if Positive

    // ───────────────────────
    // Memory Access (A)
    // ───────────────────────
    LOAD_A_MEM = 0x20,   // A = memory[addr] (16-bit)
    STORE_A_MEM = 0x21,  // memory[addr] = A (16-bit)
    LOAD8_A_MEM = 0x22,  // A = zero-extended memory[addr] (8-bit)
    STORE8_A_MEM = 0x23, // memory[addr] = A & 0xFF (8-bit)

    // ───────────────────────
    // Generic 2-Operand Format
    // ───────────────────────
    MOV8_MEM_IMM = 0x31, // mov [addr], imm8
    MOV_REG_IMM = 0x32,  // mov reg, imm16
    MOV_REG_REG = 0x33,  // mov reg1, reg2
    MOV_REG_MEM = 0x34,  // mov reg, [addr]
    MOV_REG_MEM2 = 0x35, // mov reg, [addr,addr+1]
    MOV_MEM_REG = 0x36,  // mov [addr], reg
    MOV_MEM_IMM = 0x37,  // mov [addr], imm16
    LOAD = 0x38,         // reg = memory[addr]
    STORE = 0x39,        // memory[addr] = reg

    // ───────────────────────
    // Compare and Conditional
    // ───────────────────────
    CMP = 0x40, // Compare A and B (set flags)
    JEQ = 0x41, // Jump if A == B
    JGT = 0x42, // Jump if A > B
    JLT = 0x43, // Jump if A < B

    INC = 0x44,     // increment reg
    LDC_IMM = 0x45, // Load constant into reg (16-bit)
    PRINT_R = 0x46, // Print register

    // ───────────────────────
    // Stack & Subroutine
    // ───────────────────────
    CALL = 0x50,   // Push PC, jump to addr
    RET = 0x51,    // Pop PC
    PUSH_A = 0x52, // Push A
    POP_A = 0x53,  // Pop into A
    PUSH_B = 0x54, // Push B
    POP_B = 0x55,  // Pop into B

    // ───────────────────────
    // Logic & Bitwise
    // ───────────────────────
    AND = 0x60, // A = A & B
    OR = 0x61,  // A = A | B
    XOR = 0x62, // A = A ^ B
    NOT = 0x63, // A = ~A
    SHL = 0x64, // A = A << 1
    SHR = 0x65, // A = A >> 1

    // ───────────────────────
    // Timer / Delays
    // ───────────────────────
    WAIT = 0x70, // Wait N cycles

    // ───────────────────────
    // System
    // ───────────────────────
    SYSCALL = 0xF0, // Software syscall: A = syscall number
    INT = 0xF1,     // Interrupt (optional BIOS call)
    RESET = 0xFE,   // Reset VM state
    HALT = 0xFF     // True HALT
};

extern uint16_t instruction_base;

struct CPU
{
    uint16_t pc = instruction_base; // Program Counter
    uint16_t a = 0;                 // Accumulator
    uint16_t b = 0;                 // General Purpose Register
    uint16_t c = 0;
    uint16_t mem_base = 0x0000; // Base address for memory operations
    uint8_t flag = 0;           // Status Flags
    uint8_t zero_flag = 0,
            negative_flag = 0;
    std::stack<uint16_t> stack;
    void reset()
    {
        pc = instruction_base;
        a = 0;
        b = 0;
        flag = 0;
        zero_flag = 0;
        negative_flag = 0;
        while (!stack.empty())
            stack.pop();
    }
};

#endif // SETUP_H