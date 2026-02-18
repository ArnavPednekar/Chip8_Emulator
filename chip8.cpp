#include <cstdint>
#include <cstring>
#include <iostream>
using namespace std;

class chip8 {
public:
  unsigned short opcode;
  unsigned char memory[4096];
  unsigned char v[16];
  unsigned short index;
  unsigned short pc;
  unsigned char gfx[64 * 32];
  unsigned char delay_timer, sound_timer;
  unsigned short stack[16];
  unsigned short sp;
  unsigned char key[16];
  unsigned char fontset[80] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };

  // just remember who you are
  void initailize() {
    pc = 0x200;
    index = 0;
    sp = 0;
    delay_timer = 0;
    sound_timer = 0;

    memset(memory, 0, sizeof(stack));
    memset(v, 0, sizeof(v));
    memset(stack, 0, sizeof(stack));
    memset(gfx, 0, sizeof(gfx));

    for (int i = 0; i < 80; ++i) {
      memory[i] = fontset[i];
    }
  }
  // emulation cycle
  void cycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];

    switch (opcode & 0xF000) {
    case 0x0000: {
      if (opcode == 0x00E0) // cls
        for (int i = 0; i < 64 * 32; i++) {
          gfx[i] = 0;
        }
      else if (opcode == 0x00EE) {
        stack[sp] = pc;
        sp--;
      } else if (opcode == 0x0000) {
        pc = opcode & 0x0FFF;
      }
      break;
    }
    case 0x1000: { // jp addr
      uint16_t address = opcode & 0x0FFF;
      pc = address;
      break;
    }
    case 0x2000: { // call addr
      sp++;
      stack[sp] = pc;
      pc = opcode & 0x0FFF;
      break;
    }
    case 0x3000: { // 3xkk - SE Vx, byte
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t kk = (opcode & 0x00FF);
      if (v[x] == kk) { // Skip next instruction if Vx = kk.
        pc += 2;
      }
      break;
    }
    case 0x4000: { // 4xkk - SNE Vx, byte
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t kk = (opcode & 0x00FF);
      if (v[x] != kk) { // Skip next instruction if Vx != kk.
        pc += 2;
      }
    }
    case 0x5000: { // 5xy0 - SE Vx, Vy
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 8;
      if (v[x] == v[y]) { // Skip next instruction if Vx = Vy.
        pc += 2;
      }
    }
    case 0x6000: { // 6xkk - LD Vx , byte
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t kk = (opcode & 0x00FF);
      v[x] = kk;
    }
    case 0x7000: { // 7xkk - ADD Vx, byte
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t kk = (opcode & 0x00FF);
      v[x] = v[x] + kk;
    }
    case 0x8000: { // 8xy0 - LD Vx , Vy
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 8;
      switch (opcode & 0x000F) {
      case 0x0: // 8xy0 -LD Vx, Vy
        v[x] = v[y];
        break;
      case 0x1: // 8xy0 -LD Vx, Vy
        v[x] |= v[y];
        break;
      case 0x2: // 8xy0 -LD Vx, Vy
        v[x] &= v[y];
        break;
      case 0x3:
        v[x] ^= v[y];
        break;
      case 0x4: { // 8xy4: ADD Vx, Vy
        uint16_t sum = v[x] + v[y];
        // If the sum is greater than 255 (0xFF), there is a carry
        if (sum > 0xFF) {
          v[0xF] = 1;
        } else {
          v[0xF] = 0;
        }
        // Keep only the lowest 8 bits and store in Vx
        v[x] = (sum & 0xFF);
        break;
      }
      case 0x5: { // 8xy5 - SUB Vx, Vy
        if (v[y] > v[x]) {
          v[0xF] = 1;
        } else {
          v[0xF] = 0;
          v[x] = v[x] - v[y];
        }
        break;
      }
      case 0x6: { // 8xy6 - SHR Vx {, Vy}
        uint8_t lsb = (v[x] & 0x01);
        v[0xF] = lsb;
        v[x] >>= 1;
        break;
      }
      case 0x7: {
      }
      }
    }
    default:
      cout << "unknown opcode: 0x" << opcode << "\n" << endl;
      break;
    }
    // Update timers
    if (delay_timer > 0) {
      --delay_timer;
    }

    if (sound_timer > 0) {
      if (sound_timer == 1) {
        cout << ("BEEP!\n");
        --sound_timer;
      }
    }
  }
};
