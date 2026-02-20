HIEEEEE :3 this was my first time makeing a solo c++ project like this one so plesase forgive me if you find any errors.


# CHIP-8 Emulator

A lightweight CHIP-8 interpreter written in C++. This project was developed as a solo learning exercise to explore systems programming, memory management, and opcode emulation.

## Overview

This emulator implements the core instruction set of the CHIP-8 virtual machine. It was built using the Cowgod's CHIP-8 Technical Reference as the primary guide for memory mapping, register configuration, and instruction logic.

## Prerequisites

To compile and run this project, you will need:

* A C++ compiler (g++, clang, or MSVC)
* [Insert Library Name, e.g., SDL2 or SFML] for graphics and input

## Installation and Usage

1. **Clone the repository:**
```bash
git clone https://github.com/ArnavPednekar/Chip8_Emulator.git
cd Chip8_Emulator

```


2. **Compile the project:**
```bash
g++ chip8.cpp -lSDL2 -o chip8

./chip*

```


3. **Run the emulator:**
```
just keep the file(make sure its a chip8 rom file ) you want to run in the same folder as the chip.cpp file and edit "emulator.loadROM("PONG");" to the file name you want to run.
currently i have put in pong as defualt to test

```



## Key Mapping

The original CHIP-8 hex keypad is mapped to the following layout on a standard QWERTY keyboard:

| CHIP-8 Key | PC Key |
| --- | --- |
| 1, 2, 3, C | 1, 2, 3, 4 |
| 4, 5, 6, D | Q, W, E, R |
| 7, 8, 9, E | A, S, D, F |
| A, 0, B, F | Z, X, C, V |

## Technical Details

* **Memory:** 4KB of RAM.
* **Registers:** 16 8-bit general-purpose registers (V0-VF) and a 16-bit Index register (I).
* **Stack:** Support for nested subroutines.
* **Timers:** Implementation of both Delay and Sound timers.

## Reference Materials

A PDF of the Cowgod's CHIP-8 Technical Reference is included in this repository. It provides the necessary specifications for the instruction set and memory layout used during development.

---

