#include "cpu.h"

CPU cpu;

bool cpu_running = 1;

void wait_cycles(uint8_t cycles)
{
    // Simulate waiting for a number of cycles
    auto start = std::chrono::high_resolution_clock::now();
    while (true)
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
        if (elapsed >= cycles * 1000) // Assuming 1 cycle = 1000 microseconds
            break;
    }
}

void start()

{
    int8 rem;
    uint8_t arg1, arg2;
    uint16_t addr;
    char reg, reg1;
    while (cpu_running)
    {
        int8 opcode = memory[cpu.pc++];
        switch (opcode)
        {
        case NOP:
            break;

        case INC:
            reg = (char)memory[cpu.pc++];
            switch (reg)
            {
            case 'a':
                ++cpu.a;
                break;
            case 'b':
                ++cpu.b;
                break;
            case 'c':
                ++cpu.c;
                break;
            default:
                break;
            }
            break;

        case LDA_IMM:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            cpu.a = addr;
            break;
        case LDB_IMM:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            cpu.b = addr;
            break;
        case LDC_IMM:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            cpu.c = addr;
            break;
        case ADD:
            cpu.a += cpu.b;
            break;
        case SUB:
            cpu.a -= cpu.b;
            break;
        case MUL:
            cpu.a *= cpu.b;
            break;
        case DIV:
            if (cpu.b != 0)
            {
                cpu.a /= cpu.b;
            }
            break;
        case MOD:
            if (cpu.b != 0)
            {
                cpu.a %= cpu.b;
            }
            break;
        case PRINT_A:
            std::cout << std::dec << cpu.a;
            break;
        case PRINT_R:
            reg = (char)memory[cpu.pc++];
            switch (reg)
            {
            case 'a':
                std::cout << cpu.a;
                break;
            case 'b':
                std::cout << cpu.b;
                break;
            case 'c':
                std::cout << cpu.c;
                break;
            default:
                std::cerr << "Unknown register: " << reg << std::endl;
                cpu_running = false; // Stop execution on unknown register
            }
            break;
        case PRINT_CHAR:
            std::cout << static_cast<char>(cpu.a & 0xFF);
            break;
        case IN_A:
            cpu.a = std::cin.get();
            break;
        case JMP:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            cpu.pc = addr;
            break;
        case JZ:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (cpu.zero_flag)
                cpu.pc = addr;
            break;
        case JNZ:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (!cpu.zero_flag)
                cpu.pc = addr;
            break;
        case HLT:
            cpu_running = false;
            break;
        case JN:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (cpu.negative_flag)
                cpu.pc = addr;
            break;
        case JP:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (!cpu.negative_flag && !cpu.zero_flag)
                cpu.pc = addr;
            break;
        case LOAD_A_MEM:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (addr < MEMORY_MAX)
            {
                cpu.a = (memory[addr] << 8) | memory[addr + 1];
            }
            break;
        case STORE_A_MEM:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (addr < MEMORY_MAX)
            {
                arg1 = cpu.a & 0xFF; // Lower byte
                arg2 = (cpu.a >> 8) & 0xFF;
                memory[addr] = arg1;
                memory[addr + 1] = arg2;
            }
            break;
        case LOAD8_A_MEM:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (addr < MEMORY_MAX)
            {
                cpu.a = memory[addr];
            }
            break;
        case STORE8_A_MEM:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (addr < MEMORY_MAX)
            {
                memory[addr] = cpu.a & 0xFF;
            }
            break;
        case MOV_MEM_IMM:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            memory[addr] = arg1;
            memory[addr + 1] = arg2; // Store as 16-bit
            break;

        case MOV_REG_IMM:
            reg = (char)memory[cpu.pc++];
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            switch (reg)
            {
            case 'a':
                cpu.a = (arg1 << 8) | arg2; // Combine low and high byte)
                ;
                break;
            case 'b':
                cpu.b = (arg1 << 8) | arg2; // Combine low and high byte)
                break;
            default:
                break;
            }
            break;

        case MOV_REG_REG:
            reg = (char)memory[cpu.pc++];
            reg1 = (char)memory[cpu.pc++];
            if (reg == 'a')
                cpu.a = cpu.b;
            else
                cpu.b = cpu.a;
            break;

        case MOV_MEM_REG:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            reg = (char)memory[cpu.pc++];
            switch (reg)
            {
            case 'a':
                memory[addr] = cpu.a & 0xFF;
                memory[addr + 1] = (cpu.a >> 8) & 0xFF; // Store as 16-bit
                break;
            case 'b':
                memory[addr] = cpu.b & 0xFF;
                memory[addr + 1] = (cpu.b >> 8) & 0xFF;
                break;
            default:
                break;
            }
            break;
        case MOV_REG_MEM2:
            reg = (char)memory[cpu.pc++];
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            switch (reg)
            {
            case 'a':
                cpu.a = (memory[addr] & 0xFF) | ((memory[addr + 1] & 0xFF) << 8); // Load as 16-bit
                break;
            case 'b':
                cpu.b = (memory[addr] & 0xFF) | ((memory[addr + 1] & 0xFF) << 8);
                break;
            default:
                break;
            }
            break;

        case MOV_REG_MEM:
            reg = (char)memory[cpu.pc++];
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            switch (reg)
            {
            case 'a':
                cpu.a = (memory[addr] & 0xFF); // Load as 16-bit
                break;
            case 'b':
                cpu.b = (memory[addr] & 0xFF);
                break;
            default:
                break;
            }
            break;

        case LOAD:
            reg = (char)memory[cpu.pc++];
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            switch (reg)
            {
            case 'a':
                cpu.a = memory[addr];
                break;
            case 'b':
                cpu.b = memory[addr];
                break;
            default:
                break;
            }
            break;

        case STORE:

            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            reg = (char)memory[cpu.pc++];
            switch (reg)
            {
            case 'a':
                memory[addr] = cpu.a & 0xFF;
                memory[addr + 1] = (cpu.a >> 8) & 0xFF; // Store as 16-bit
                break;
            case 'b':
                memory[addr] = cpu.b & 0xFF;
                memory[addr + 1] = (cpu.b >> 8) & 0xFF;
                break;
            default:
                break;
            }
            break;

        case MOV8_MEM_IMM:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            arg1 = memory[cpu.pc++];
            memory[addr] = arg1;
            break;

        case CMP:
            cpu.zero_flag = (cpu.c == cpu.b);
            cpu.negative_flag = (cpu.b < cpu.c);
            // std::cout << "Compare: A=" << cpu.a << " B=" << cpu.b << " zero_flag=" << cpu.zero_flag << " negative_flag=" << cpu.negative_flag << std::endl;
            break;

        case JEQ:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (cpu.b == cpu.c)
                cpu.pc = addr;
            break;

        case JGT:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (cpu.b > cpu.c)
                cpu.pc = addr;
            break;

        case JLT:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            if (cpu.b < cpu.c)         // If A > B, then B < A, which is what we want for the counter check
                cpu.pc = addr;
            break;

        case CALL:
            arg1 = memory[cpu.pc++];
            arg2 = memory[cpu.pc++];
            addr = (arg1 << 8) | arg2; // Combine low and high byte
            cpu.stack.push(cpu.pc);
            cpu.pc = addr;
            break;

        case RET:
            if (!cpu.stack.empty())
            {
                cpu.pc = cpu.stack.top();
                cpu.stack.pop();
            }
            break;

        case PUSH_A:
            cpu.stack.push(cpu.a);
            break;
        case POP_A:
            if (!cpu.stack.empty())
            {
                cpu.a = cpu.stack.top();
                cpu.stack.pop();
            }
            break;
        case PUSH_B:
            cpu.stack.push(cpu.b);
            break;

        case POP_B:
            if (!cpu.stack.empty())
            {
                cpu.b = cpu.stack.top();
                cpu.stack.pop();
            }
            break;
        case AND:
            cpu.a &= cpu.b;
            break;
        case OR:
            cpu.a |= cpu.b;
            break;
        case XOR:
            cpu.a ^= cpu.b;
            break;
        case NOT:
            cpu.a = ~cpu.a;
            break;
        case SHL:
            cpu.a <<= 1;
            break;
        case SHR:
            cpu.a >>= 1;
            break;
        case WAIT:
            arg1 = memory[cpu.pc++];
            wait_cycles(arg1);
            break;
        case SYSCALL:
        {
            uint16_t syscall_num = cpu.a;

            switch (syscall_num)
            {
            case 0x00: // SYS_NOP
                break;

            case 0x01:              // SYS_WAIT
                wait_cycles(cpu.b); // wait for B cycles
                break;

            case 0x02: // SYS_PRINTA
                std::cout << cpu.b << std::endl;
                break;

            case 0x03: // SYS_PRINTC
                std::cout << static_cast<char>(cpu.b & 0xFF);
                break;

            case 0xFF: // SYS_EXIT
                cpu_running = false;
                break;

            default:
                std::cerr << "Unknown syscall: " << syscall_num << std::endl;
                cpu_running = false;
                break;
            }
            break;
        }

        case INT:
        {
            uint8_t int_num = memory[cpu.pc++]; // Fetch interrupt number

            switch (int_num)
            {
            case 0x10: // INT 10h - print character in B
                std::cout << static_cast<char>(cpu.b & 0xFF);
                break;

            case 0x11: // INT 11h - print A as integer
                std::cout << cpu.b << std::endl;
                break;

            case 0x12: // INT 12h - wait B cycles
                wait_cycles(cpu.b);
                break;

            case 0x13: // INT 13h - reboot (reset)
                cpu.reset();
                break;

            default:
                std::cerr << "Unhandled INT " << std::hex << (int)int_num << "\n";
                cpu_running = false;
                break;
            }
            break;
        }

        case RESET:
            cpu.reset();
            break;
        case HALT:
            cpu_running = false;
            break;
        default:
            // Unknown opcode
            std::cerr << "Unknown opcode: " << static_cast<int>(opcode) << " at PC: " << cpu.pc - 1 << std::endl;
            cpu_running = false; // Stop execution on unknown opcode
            break;
        }
    }
}