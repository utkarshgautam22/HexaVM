#ifndef CPU_HPP
#define CPU_HPP

#include <iostream>
#include <chrono>
#include "setup.h"

extern CPU cpu;
extern bool cpu_running;

// Function declarations
void wait_cycles(uint8_t cycles);
void start();

#endif // CPU_HPP