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

    // Remove comments and trim whitespace from a line
    std::string preprocessLine(const std::string &line)
    {
        std::string result = line;

        // Remove comments (starting with ; or //)
        auto commentPos = result.find(';');
        if (commentPos != std::string::npos)
        {
            result = result.substr(0, commentPos);
        }

        commentPos = result.find("//");
        if (commentPos != std::string::npos)
        {
            result = result.substr(0, commentPos);
        }

        // Trim leading whitespace
        result.erase(result.begin(),
                     std::find_if(result.begin(), result.end(),
                                  [](unsigned char ch)
                                  { return !std::isspace(ch); }));

        // Trim trailing whitespace
        result.erase(std::find_if(result.rbegin(), result.rend(),
                                  [](unsigned char ch)
                                  { return !std::isspace(ch); })
                         .base(),
                     result.end());

        return result;
    }

    // Check if line contains a label definition
    bool isLabelDefinition(const std::string &line, std::string &label)
    {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            label = line.substr(0, colonPos);
            // Trim potential whitespace
            label.erase(label.begin(),
                        std::find_if(label.begin(), label.end(),
                                     [](unsigned char ch)
                                     { return !std::isspace(ch); }));
            return true;
        }
        return false;
    }

public:
    // TextAssembler();

    // First pass: record label positions and calculate addresses
    void firstPass(const std::vector<std::string> &code)
    {
        currentAddress = 0x9000; // Reset to default start address
        symbolTable.clear();
        forwardRefs.clear();

        for (const auto &rawLine : code)
        {
            std::string line = preprocessLine(rawLine);
            if (line.empty())
            {
                continue; // Skip empty lines
            }

            std::string label;
            if (isLabelDefinition(line, label))
            {
                // Record label position
                symbolTable[label] = currentAddress;

                // Remove label from line for further processing
                line = line.substr(line.find(':') + 1);
                line = preprocessLine(line); // Clean up again after removing label

                if (line.empty())
                {
                    continue; // Skip if only label on line
                }
            }

            // Handle directives
            std::istringstream iss(line);
            std::string directive;
            iss >> directive;

            if (directive == ".org")
            {
                uint16_t addr;
                iss >> std::hex >> addr;
                currentAddress = addr;
                continue;
            }

            // For normal instructions, increment address based on instruction size
            if (opcodeMap.find(directive) != opcodeMap.end())
            {
                if (instructionSize.find(directive) != instructionSize.end())
                {
                    currentAddress += instructionSize[directive];
                }
                else
                {
                    std::cerr << "Warning: Unknown instruction size for " << directive << std::endl;
                    currentAddress += 1; // Default size
                }
            }
        }

        // Debug output
        std::cout << "Symbol table after first pass:" << std::endl;
        for (const auto &symbol : symbolTable)
        {
            std::cout << symbol.first << " = 0x" << std::hex << symbol.second << std::endl;
        }
    }

    // Parse and assemble a single instruction (second pass)
    void parseLine(const std::string &rawLine)
    {
        std::string line = preprocessLine(rawLine);
        if (line.empty())
        {
            return; // Skip empty lines
        }

        std::string label;
        if (isLabelDefinition(line, label))
        {
            // Skip label in second pass - already processed in first pass
            line = line.substr(line.find(':') + 1);
            line = preprocessLine(line);

            if (line.empty())
            {
                return; // Skip if only label on line
            }
        }

        std::istringstream iss(line);
        std::string directive;
        iss >> directive;

        // Handle directives
        if (directive == ".org")
        {
            uint16_t addr;
            iss >> std::hex >> addr;
            currentAddress = addr;
            return;
        }

        // Not a directive, should be an opcode
        std::string opcode = directive;

        if (opcodeMap.find(opcode) == opcodeMap.end())
        {
            std::cerr << "Unknown opcode: " << opcode << std::endl;
            return;
        }

        memory[currentAddress++] = opcodeMap[opcode];

        // Handle operands based on instruction type

        // No operands instructions
        if (opcode == "nop" || opcode == "add" || opcode == "sub" || opcode == "mul" ||
            opcode == "div" || opcode == "mod" || opcode == "printa" || opcode == "printc" ||
            opcode == "and" || opcode == "or" || opcode == "xor" || opcode == "not" ||
            opcode == "shl" || opcode == "shr" || opcode == "ina" || opcode == "ret" ||
            opcode == "halt" || opcode == "reset" || opcode == "push_a" || opcode == "pop_a" ||
            opcode == "push_b" || opcode == "pop_b" || opcode == "cmp")
        {
            // No additional operands needed
        }
        // Register + immediate value instructions
        else if (opcode == "lda" || opcode == "ldb" || opcode == "ldc")
        {
            uint16_t value;
            iss >> value;
            memory[currentAddress++] = (value >> 8) & 0xFF; // High byte
            memory[currentAddress++] = value & 0xFF;        // Low byte
        }
        // Single register instructions
        else if (opcode == "inc")
        {
            char reg;
            iss >> reg;
            memory[currentAddress++] = reg;
        }
        // Jump/branch instructions (address operand)
        else if (opcode == "jmp" || opcode == "jz" || opcode == "jnz" || opcode == "jn" ||
                 opcode == "jp" || opcode == "jeq" || opcode == "jgt" || opcode == "jlt" ||
                 opcode == "call")
        {
            std::string addrOrLabel;
            iss >> addrOrLabel;

            // Check if it's a label (not starting with 0x)
            if (addrOrLabel.size() >= 2 && addrOrLabel.substr(0, 2) != "0x")
            {
                // It's a label, look it up in symbol table
                if (symbolTable.find(addrOrLabel) != symbolTable.end())
                {
                    uint16_t addr = symbolTable[addrOrLabel];
                    memory[currentAddress++] = (addr >> 8) & 0xFF; // High byte
                    memory[currentAddress++] = addr & 0xFF;        // Low byte
                }
                else
                {
                    std::cerr << "Error: Undefined label '" << addrOrLabel << "'" << std::endl;
                    // Use dummy address
                    memory[currentAddress++] = 0;
                    memory[currentAddress++] = 0;
                }
            }
            else
            {
                // It's a hex address
                uint16_t addr;
                if (addrOrLabel.substr(0, 2) == "0x")
                {
                    // Remove 0x prefix
                    std::istringstream hexStream(addrOrLabel.substr(2));
                    hexStream >> std::hex >> addr;
                }
                else
                {
                    // Try to parse as decimal
                    std::istringstream decStream(addrOrLabel);
                    decStream >> addr;
                }
                memory[currentAddress++] = (addr >> 8) & 0xFF; // High byte
                memory[currentAddress++] = addr & 0xFF;        // Low byte
            }
        }
        // Memory operations with address
        else if (opcode == "load_a" || opcode == "store_a" || opcode == "load8_a" ||
                 opcode == "store8_a" || opcode == "load" || opcode == "store")
        {
            uint16_t addr;
            iss >> std::hex >> addr;
            memory[currentAddress++] = (addr >> 8) & 0xFF; // High byte
            memory[currentAddress++] = addr & 0xFF;        // Low byte

            // For LOAD and STORE, we need an additional register operand
            if (opcode == "load" || opcode == "store")
            {
                char reg;
                iss >> reg;
                memory[currentAddress++] = reg;
            }
        }
        // Memory move operations with address and immediate value
        else if (opcode == "mov_mem_imm")
        {
            uint16_t addr;
            iss >> std::hex >> addr;
            memory[currentAddress++] = (addr >> 8) & 0xFF; // High byte
            memory[currentAddress++] = addr & 0xFF;        // Low byte

            uint16_t value;
            iss >> value;
            memory[currentAddress++] = (value >> 8) & 0xFF; // High byte
            memory[currentAddress++] = value & 0xFF;        // Low byte
        }
        // 8-bit memory move with address and immediate value
        else if (opcode == "mov8_mem_imm")
        {
            uint16_t addr;
            iss >> std::hex >> addr;
            memory[currentAddress++] = (addr >> 8) & 0xFF; // High byte
            memory[currentAddress++] = addr & 0xFF;        // Low byte

            uint8_t value;
            iss >> value;
            memory[currentAddress++] = value; // 8-bit value
        }
        // Register moves
        else if (opcode == "mov_reg_imm")
        {
            char reg;
            iss >> reg;
            memory[currentAddress++] = reg;

            uint16_t addr;
            iss >> std::hex >> addr;
            memory[currentAddress++] = (addr >> 8) & 0xFF; // High byte
            memory[currentAddress++] = addr & 0xFF;        // Low byte

            uint16_t value;
            iss >> value;
            memory[currentAddress++] = (value >> 8) & 0xFF; // High byte
            memory[currentAddress++] = value & 0xFF;        // Low byte
        }
        else if (opcode == "mov_reg_reg")
        {
            char reg1;
            iss >> reg1;
            char reg2;
            iss >> reg2;
            memory[currentAddress++] = reg1;
            memory[currentAddress++] = reg2;
        }
        else if (opcode == "mov_reg_mem" || opcode == "mov_reg_mem2")
        {
            char reg;
            iss >> reg;
            memory[currentAddress++] = reg;

            uint16_t addr;
            iss >> std::hex >> addr;
            memory[currentAddress++] = (addr >> 8) & 0xFF; // High byte
            memory[currentAddress++] = addr & 0xFF;        // Low byte
        }
        else if (opcode == "mov_mem_reg")
        {
            uint16_t addr;
            iss >> std::hex >> addr;
            memory[currentAddress++] = (addr >> 8) & 0xFF; // High byte
            memory[currentAddress++] = addr & 0xFF;        // Low byte

            char reg;
            iss >> reg;
            memory[currentAddress++] = reg;
        }
        // Wait cycles instruction
        else if (opcode == "wait")
        {
            uint8_t cycles;
            iss >> cycles;
            memory[currentAddress++] = cycles;
        }
        // Syscall and interrupt
        else if (opcode == "syscall")
        {
            // Syscall number is in register A
        }
        else if (opcode == "int")
        {
            uint8_t int_num;
            iss >> std::hex >> int_num;
            memory[currentAddress++] = int_num;
        }
    }

    // Assemble multiple lines (second pass)
    void doSecondPass(const std::vector<std::string> &code)
    {
        isSecondPass = true;
        currentAddress = 0x9000; // Reset to default start address

        for (const auto &line : code)
        {
            parseLine(line);
        }
    }

    // Complete two-pass assembly
    void assemble(const std::vector<std::string> &code)
    {
        // First pass: build symbol table
        firstPass(code);

        // Second pass: generate code
        doSecondPass(code);
    }

    // Load program from file
    std::vector<std::string> loadFromFile(const std::string &filename)
    {
        std::vector<std::string> code;
        std::ifstream file(filename);

        if (!file.is_open())
        {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return code;
        }

        std::string line;
        while (std::getline(file, line))
        {
            code.push_back(line);
        }

        file.close();
        return code;
    }

    // Save assembled program to file
    bool saveToFile(const std::string &filename, uint16_t start, uint16_t end)
    {
        std::ofstream file(filename, std::ios::binary);

        if (!file.is_open())
        {
            std::cerr << "Error: Could not open output file " << filename << std::endl;
            return false;
        }

        // Write memory contents from start to end
        for (uint16_t addr = start; addr < end; addr++)
        {
            file.put(memory[addr]);
        }

        file.close();
        return true;
    }

    // Print a hex dump of the assembled code
    void hexDump(uint16_t start, uint16_t end, int bytesPerLine = 16)
    {
        for (uint16_t addr = start; addr < end; addr += bytesPerLine)
        {
            // Print address
            std::cout << std::hex << std::setfill('0') << std::setw(4) << addr << ": ";

            // Print hex values
            for (int i = 0; i < bytesPerLine && addr + i < end; i++)
            {
                std::cout << std::hex << std::setfill('0') << std::setw(2)
                          << static_cast<int>(memory[addr + i]) << " ";
            }

            std::cout << " | ";

            // Print ASCII representation
            for (int i = 0; i < bytesPerLine && addr + i < end; i++)
            {
                char c = memory[addr + i];
                std::cout << (isprint(c) ? c : '.');
            }

            std::cout << std::endl;
        }
    }
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <input.asm> [output.bin]" << std::endl;
        std::cout << "If output file is not specified, a hex dump will be printed to stdout" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = (argc > 2) ? argv[2] : "";

    TextAssembler assembler;

    // Load and assemble the code
    std::vector<std::string> code = assembler.loadFromFile(inputFile);
    if (code.empty())
    {
        std::cerr << "Error: No code to assemble" << std::endl;
        return 1;
    }

    std::cout << "Assembling " << inputFile << "..." << std::endl;
    assembler.assemble(code);

    // Determine start and end addresses
    uint16_t start = 0x9000; // Default
    uint16_t end = 0x9000;   // Will be updated

    // Find the highest used memory address
    for (uint16_t addr = start; addr < MEMORY_MAX; addr++)
    {
        if (memory[addr] != 0)
        {
            end = addr + 1;
        }
    }

    // Either save to file or print hex dump
    if (!outputFile.empty())
    {
        if (assembler.saveToFile(outputFile, start, end))
        {
            std::cout << "Binary output saved to " << outputFile << std::endl;
            std::cout << "Size: " << (end - start) << " bytes" << std::endl;
        }
    }
    else
    {
        std::cout << "Assembly result:" << std::endl;
        assembler.hexDump(start, end);
    }

    return 0;
}