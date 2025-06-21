#include "setup.h"
#include "cpu.h"
#include "assembler.h"
#include <cstring> // For std::memset

// Function to reset memory to all zeros
void clearMemory()
{
    std::memset(memory, 0, MEMORY_MAX);
}

// Function to run the assembled program on the CPU
void runProgram()
{

    // Print initial state
    std::cout << "\nRunning program...\n";
    std::cout << "Initial CPU state: PC=" << cpu.pc
              << ", A=" << cpu.a << ", B=" << cpu.b << ", C=" << cpu.c << std::endl;

    // Execute the program
    std::cout << "\nProgram output:\n";
    std::cout << "----------------------------------------\n";
    // std::cout << std::dec;

    // Main execution loop is in the cpu.cpp file's main function
    start();

    std::cout << "\n----------------------------------------\n";
    std::cout << "Program terminated.\n";
    std::cout << "Final CPU state: PC=" << cpu.pc
              << ", A=" << cpu.a << ", B=" << cpu.b << ", C=" << cpu.c << std::endl;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <input.asm> [-r] [output.bin]" << std::endl;
        std::cout << "  -r         : Run the program after assembling" << std::endl;
        std::cout << "  output.bin : Save assembled binary to file (optional)" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    bool runAfterAssembly = false;
    std::string outputFile = "";

    // Parse command line arguments
    for (int i = 2; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-r")
        {
            runAfterAssembly = true;
        }
        else
        {
            outputFile = arg;
        }
    }

    clearMemory();
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

    // Run the program if requested
    if (runAfterAssembly)
    {
        std::cout << "\n===================================\n";
        runProgram();
    }

    return 0;
}
