#include <cstdint>
#include <cstdlib>
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
      case 0x0: { // 8xy0 -LD Vx, Vy
        v[x] = v[y];
        break;
      }
      case 0x1: { // 8xy0 -LD Vx, Vy
        v[x] |= v[y];
        break;
      }
      case 0x2: { // 8xy0 -LD Vx, Vy
        v[x] &= v[y];
        break;
      }
      case 0x3: {
        v[x] ^= v[y];
        break;
      }
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
      case 0x7: { // 8xy7 SUBN Vx, Vy
        if (v[y] > v[x]) {
          v[0xF] = 1;
        } else {
          v[0xF] = 0;
        }
        v[x] = v[y] - v[x];
        break;
      }
      case 0xE: { // 8xyE - SHL Vx {, Vy}
        uint8_t msb = (v[x] & 0x80) >> 7;
        v[0xF] = msb;
        v[x] <<= 1;
        break;
      }
      }
    }
    case 0x9000: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 8;
      if (v[x] != v[y]) {
        pc += 2;
      }
      break;
    }
    case 0xA000: {
      uint16_t nnn = opcode & 0x0FFF;
      index = nnn;
      break;
    }
    case 0xB000: {
      uint16_t nnn = opcode & 0x0FFF;
      pc = nnn + v[0];
      break;
    }
    case 0xC000: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t kk = (opcode & 0x00FF);
      uint8_t random_byte = rand() % 256;
      v[x] = random_byte & kk;
      break;
    }
    case 0xD000: {
      uint8_t xPos = v[(opcode & 0x0F00) >> 8] % 64;
      uint8_t yPos = v[(opcode & 0x00F0) >> 4] % 32;
      uint8_t height = (opcode & 0x000F);
      v[0xF] = 0;
      for (int row = 0; row < height; row++) {
        uint8_t spriteByte = memory[index + row];
        for (int col = 0; col < 8; col++) {
          if ((spriteByte & (0x80 >> col)) != 0) {
            int screenindex = (xPos + col) % 64 + ((yPos + row) % 32) * 64;
            if (gfx[screenindex] == 1) {
              v[0xF] = 1;
            }
            gfx[screenindex] ^= 1;
          }
        }
      }
      // Note: You'll need to tell your graphics library (SDL/Raylib) to redraw
      // here drawFlag = true
      break;
    }
    case 0xE000: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t variant = (opcode & 0x00FF);

      switch (variant) {
      case 0x9E: { // Ex9E: Skip if key pressed
        if (key[v[x]] != 0) {
          pc += 2;
        }
        break;
      }

      case 0xA1: { // ExA1: Skip if key NOT pressed
        if (key[v[x]] == 0) {
          pc += 2;
        }
        break;
      }
      }
      break;
    }
    case 0xF000: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      switch (opcode & 0x00FF) {

      case 0x07: // Fx07: Set Vx to Delay Timer
        v[x] = delay_timer;
        break;

      case 0x0A: { // Fx0A: Wait for key press (BLOCKING)
        bool keyPressed = false;
        for (int i = 0; i < 16; ++i) {
          if (key[i] != 0) {
            v[x] = i;
            keyPressed = true;
            break;
          }
        }
        if (!keyPressed)
          return; // "Freeze" the PC by returning without incrementing
        break;
      }

      case 0x15: // Fx15: Set Delay Timer
        delay_timer = v[x];
        break;

      case 0x18: // Fx18: Set Sound Timer
        sound_timer = v[x];
        break;

      case 0x1E: // Fx1E: Add Vx to I
        index += v[x];
        break;

      case 0x29:          // Fx29: Set I to Font Character
        index = v[x] * 5; // Each font char is 5 bytes tall
        break;

      case 0x33: // Fx33: Binary-Coded Decimal (The math "trick")
        memory[index] = v[x] / 100;           // Hundreds place
        memory[index + 1] = (v[x] / 10) % 10; // Tens place
        memory[index + 2] = v[x] % 10;        // Ones place
        break;

      case 0x55: // Fx55: Store registers in memory
        for (int i = 0; i <= x; ++i)
          memory[index + i] = v[i];
        break;

      case 0x65: // Fx65: Load registers from memory
        for (int i = 0; i <= x; ++i)
          v[i] = memory[index + i];
        break;
      }
      break;
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
