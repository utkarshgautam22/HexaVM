#include "assembler.h"

// Remove comments and trim whitespace from a line
std::string TextAssembler::preprocessLine(const std::string &line)
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
bool TextAssembler::isLabelDefinition(const std::string &line, std::string &label)
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

// First pass: record label positions and calculate addresses
void TextAssembler::firstPass(const std::vector<std::string> &code)
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
        else if (directive == ".db")
        {
            // Process data bytes directive
            uint8_t value;
            while (iss >> value)
            {
                // Just increment the address counter for first pass
                currentAddress++;

                // Skip any commas
                char comma;
                iss >> comma;
            }
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
void TextAssembler::parseLine(const std::string &rawLine)
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
void TextAssembler::doSecondPass(const std::vector<std::string> &code)
{
    isSecondPass = true;
    currentAddress = 0x9000; // Reset to default start address

    for (const auto &line : code)
    {
        parseLine(line);
    }
}

// Complete two-pass assembly
void TextAssembler::assemble(const std::vector<std::string> &code)
{
    // First pass: build symbol table
    firstPass(code);

    // Second pass: generate code
    doSecondPass(code);
}

// Load program from file
std::vector<std::string> TextAssembler::loadFromFile(const std::string &filename)
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
bool TextAssembler::saveToFile(const std::string &filename, uint16_t start, uint16_t end)
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
void TextAssembler::hexDump(uint16_t start, uint16_t end, int bytesPerLine)
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

// int main(int argc, char **argv)

// {
//     if (argc < 2)
//     {
//         std::cout << "Usage: " << argv[0] << " <input.asm> [output.bin]" << std::endl;
//         std::cout << "If output file is not specified, a hex dump will be printed to stdout" << std::endl;
//         return 1;
//     }

//     std::string inputFile = argv[1];
//     std::string outputFile = (argc > 2) ? argv[2] : "";

//     TextAssembler assembler;

//     // Load and assemble the code
//     std::vector<std::string> code = assembler.loadFromFile(inputFile);
//     if (code.empty())
//     {
//         std::cerr << "Error: No code to assemble" << std::endl;
//         return 1;
//     }

//     std::cout << "Assembling " << inputFile << "..." << std::endl;
//     assembler.assemble(code);

//     // Determine start and end addresses
//     uint16_t start = 0x9000; // Default
//     uint16_t end = 0x9000;   // Will be updated

//     // Find the highest used memory address
//     for (uint16_t addr = start; addr < MEMORY_MAX; addr++)
//     {
//         if (memory[addr] != 0)
//         {
//             end = addr + 1;
//         }
//     }

//     // Either save to file or print hex dump
//     if (!outputFile.empty())
//     {
//         if (assembler.saveToFile(outputFile, start, end))
//         {
//             std::cout << "Binary output saved to " << outputFile << std::endl;
//             std::cout << "Size: " << (end - start) << " bytes" << std::endl;
//         }
//     }
//     else
//     {
//         std::cout << "Assembly result:" << std::endl;
//         assembler.hexDump(start, end);
//     }

//     return 0;
// }
