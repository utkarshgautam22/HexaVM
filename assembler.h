#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include "setup.h"
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <iostream>
#include <cctype>
#include <algorithm>

#include <iomanip> // for std::setw and std::setfill

class TextAssembler
{
private:
    // Symbol table for labels
    std::map<std::string, uint16_t> symbolTable;

    // Forward references to be resolved in second pass
    std::vector<std::pair<uint16_t, std::string>> forwardRefs;

    // Instruction size lookup table (bytes including opcode and operands)
    std::map<std::string, uint8_t> instructionSize = {
        // No operand instructions - 1 byte
        {"nop", 1},
        {"add", 1},
        {"sub", 1},
        {"mul", 1},
        {"div", 1},
        {"mod", 1},
        {"and", 1},
        {"or", 1},
        {"xor", 1},
        {"not", 1},
        {"shl", 1},
        {"shr", 1},
        {"printa", 1},
        {"printc", 1},
        {"ina", 1},
        {"ret", 1},
        {"halt", 1},
        {"reset", 1},
        {"push_a", 1},
        {"pop_a", 1},
        {"push_b", 1},
        {"pop_b", 1},
        {"cmp", 1},

        // Register + immediate value - 3 bytes (opcode + 16-bit value)
        {"lda", 3},
        {"ldb", 3},
        {"ldc", 3},

        // Single register instructions - 2 bytes (opcode + register)
        {"inc", 2},

        // Jump instructions - 3 bytes (opcode + address)
        {"jmp", 3},
        {"jz", 3},
        {"jnz", 3},
        {"jn", 3},
        {"jp", 3},
        {"jeq", 3},
        {"jgt", 3},
        {"jlt", 3},
        {"call", 3},

        // Memory operations with address - 3 bytes
        {"load_a", 3},
        {"store_a", 3},
        {"load8_a", 3},
        {"store8_a", 3},

        {"mov8_mem_imm", 4}, // memory[addr] = immediate 8-bit
        {"mov_reg_imm", 4},  // reg = immediate 16-bit
        {"mov_reg_reg", 3},  // reg1 = reg2
        {"mov_reg_mem", 4},  // reg = memory[addr] (8-bit)
        {"mov_reg_mem2", 4}, // reg = memory[addr] | (memory[addr+1] << 8) (16-bit)
        {"mov_mem_reg", 4},  // memory[addr] = reg (16-bit)
        {"mov_mem_imm", 5},  // memory[addr] = immediate 16-bit
        {"load", 4},         // reg = memory[addr]
        {"store", 4},        // memory[addr] = reg

        // Wait cycles - variable size
        {"wait", 2},

        // Syscall - variable size
        {"syscall", 1},

        // Interrupt - variable size
        {"int", 2}};

    std::map<std::string, uint8_t> opcodeMap = {
        // Basic Arithmetic & Logic
        {"nop", NOP},     // No operation
        {"lda", LDA_IMM}, // A = immediate 16-bit
        {"ldb", LDB_IMM}, // B = immediate 16-bit
        {"ldc", LDC_IMM}, // C = immediate 16-bit
        {"add", ADD},     // A = A + B
        {"sub", SUB},     // A = A - B
        {"mul", MUL},     // A = A * B
        {"div", DIV},     // A = A / B (if B != 0)
        {"mod", MOD},     // A = A % B (if B != 0)
        {"and", AND},     // A = A & B
        {"or", OR},       // A = A | B
        {"xor", XOR},     // A = A ^ B
        {"not", NOT},     // A = ~A
        {"shl", SHL},     // A = A << 1
        {"shr", SHR},     // A = A >> 1
        {"inc", INC},     // Increment register by 1

        // I/O Operations
        {"printa", PRINT_A},    // Print A register as number
        {"printc", PRINT_CHAR}, // Print A register as ASCII char
        {"ina", IN_A},          // A = getchar() (input)

        // Memory Operations
        {"load_a", LOAD_A_MEM},         // A = memory[addr] (16-bit)
        {"store_a", STORE_A_MEM},       // memory[addr] = A (16-bit)
        {"load8_a", LOAD8_A_MEM},       // A = memory[addr] (8-bit)
        {"store8_a", STORE8_A_MEM},     // memory[addr] = A & 0xFF (8-bit)
        {"mov_mem_imm", MOV_MEM_IMM},   // memory[addr] = immediate 16-bit
        {"mov8_mem_imm", MOV8_MEM_IMM}, // memory[addr] = immediate 8-bit
        {"mov_reg_imm", MOV_REG_IMM},   // reg = immediate 16-bit
        {"mov_reg_reg", MOV_REG_REG},   // reg1 = reg2
        {"mov_reg_mem", MOV_REG_MEM},   // reg = memory[addr] (8-bit)
        {"mov_reg_mem2", MOV_REG_MEM2}, // reg = memory[addr] | (memory[addr+1] << 8) (16-bit)
        {"mov_mem_reg", MOV_MEM_REG},   // memory[addr] = reg (16-bit)
        {"load", LOAD},                 // reg = memory[addr]
        {"store", STORE},               // memory[addr] = reg

        // Control Flow
        {"jmp", JMP},   // Jump to addr
        {"jz", JZ},     // Jump if zero flag
        {"jnz", JNZ},   // Jump if not zero flag
        {"jn", JN},     // Jump if negative flag
        {"jp", JP},     // Jump if positive (not negative and not zero)
        {"jeq", JEQ},   // Jump if B == C
        {"jgt", JGT},   // Jump if B > C
        {"jlt", JLT},   // Jump if B < C
        {"call", CALL}, // Push PC to stack and jump to addr
        {"ret", RET},   // Pop PC from stack

        // Stack Operations
        {"push_a", PUSH_A}, // Push A to stack
        {"pop_a", POP_A},   // Pop from stack into A
        {"push_b", PUSH_B}, // Push B to stack
        {"pop_b", POP_B},   // Pop from stack into B

        // Comparison
        {"cmp", CMP}, // Compare B and C, set flags

        // System & Misc
        {"wait", WAIT},       // Wait N cycles
        {"syscall", SYSCALL}, // System call (A = call number)
        {"int", INT},         // Interrupt (parameter = interrupt number)
        {"reset", RESET},     // Reset CPU state
        {"halt", HALT}        // Halt execution
    };
    uint16_t currentAddress = 0x9000;
    bool isSecondPass = false;

    std::string preprocessLine(const std::string &line);
    bool isLabelDefinition(const std::string &line, std::string &label);

public:
    void firstPass(const std::vector<std::string> &code);
    void doSecondPass(const std::vector<std::string> &code);
    void assemble(const std::vector<std::string> &code);
    void parseLine(const std::string &rawLine);
    std::vector<std::string> loadFromFile(const std::string &filename);
    bool saveToFile(const std::string &filename, uint16_t start, uint16_t end);
    void hexDump(uint16_t start, uint16_t end, int bytesPerLine = 16);
};

#endif // ASSEMBLER_HPP