#include <SDL2/SDL.h>
#include <cstdint>
#include <cstdio>
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
  bool drawFlag;
  unsigned char fontset[80] = {
      // this is the display part referrer to 2.4 to the cowgods pdf
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
    drawFlag = false;
    memset(memory, 0, sizeof(memory));
    memset(v, 0, sizeof(v));
    memset(stack, 0, sizeof(stack));
    memset(gfx, 0, sizeof(gfx));

    for (int i = 0; i < 80; ++i) { // its 80 cuz thats the size of the fontset
      memory[i] = fontset[i];
    }
  }
  // loading rom files on the
  bool loadROM(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file)
      return false;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    fread(memory + 0x200, 1, size, file);
    cout << "ROM loaded successfully\n";
    fclose(file);
    return true;
  }

  // emulation cycle
  void cycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];
    pc += 2;
    switch (opcode & 0xF000) {
    case 0x0000: {

      if (opcode == 0x00E0) { // cls
        for (int i = 0; i < 64 * 32; i++) {
          gfx[i] = 0;
        }
        drawFlag = true;
      } else if (opcode == 0x00EE) {
        sp--;
        pc = stack[sp];
      }
      break;
    }
    case 0x1000: { // jp addr
      uint16_t address = opcode & 0x0FFF;
      pc = address;
      break;
    }
    case 0x2000: { // call addr
      stack[sp] = pc;
      sp++;
      pc = opcode & 0x0FFF;
      break;
    }
    case 0x3000: { // 3xkk - SE Vx, byte
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t kk = (opcode & 0x00FF);
      if (v[x] == kk) {
      }
      break;
    }
    case 0x4000: { // 4xkk - SNE Vx, byte
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t kk = (opcode & 0x00FF);
      if (v[x] != kk) {
        pc += 2;
      }
      break;
    }
    case 0x5000: { // 5xy0 - SE Vx, Vy
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 4;
      if (v[x] == v[y]) {
        pc += 2;
      }
      break;
    }
    case 0x6000: { // 6xkk - LD Vx , byte
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t kk = (opcode & 0x00FF);
      v[x] = kk;
      break;
    }
    case 0x7000: { // 7xkk - ADD Vx, byte
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t kk = (opcode & 0x00FF);
      v[x] = v[x] + kk;
      break;
    }
    case 0x8000: { // 8xy0 - LD Vx , Vy
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 4;
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
        if (sum > 0xFF) {
          v[0xF] = 1;
        } else {
          v[0xF] = 0;
        }
        v[x] = (sum & 0xFF);
        break;
      }
      case 0x5: { // 8xy5 - SUB Vx, Vy
        v[0xF] = (v[x] > v[y]) ? 1 : 0;
        v[x] = v[x] - v[y];
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
      break;
    }
    case 0x9000: { // 9xy0 - SNE Vx, Vy
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 4;
      if (v[x] != v[y]) {
        pc += 2;
      }
      break;
    }
    case 0xA000: { // Annn - LD I, addr
      uint16_t nnn = opcode & 0x0FFF;
      index = nnn;
      break;
    }
    case 0xB000: { // Bnnn - JP V0, addr
      uint16_t nnn = opcode & 0x0FFF;
      pc = nnn + v[0];
      break;
    }
    case 0xC000: { // Cxkk - RND Vx, byte
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t kk = (opcode & 0x00FF);
      uint8_t random_byte = rand() % 256;
      v[x] = random_byte & kk;
      break;
    }
    case 0xD000: { // Dxyn - DRW Vx, Vy, nibble
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
      drawFlag = true;
      break;
    }
    case 0xE000: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t variant = (opcode & 0x00FF);

      switch (variant) {
      case 0x9E: { // Ex9E - SKP Vx
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
  };
};

// draw the chip8 screen (this is the drawer(cuz he draws) lowkey used ai for
// this but like its easy to understand)
void drawGraphics(SDL_Renderer *renderer, unsigned char *gfx) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  const int SCALE = 10;

  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 64; x++) {
      if (gfx[y * 64 + x] == 1) {
        SDL_Rect pixel = {x * SCALE, y * SCALE, SCALE, SCALE};
        SDL_RenderFillRect(renderer, &pixel);
      }
    }
  }

  SDL_RenderPresent(renderer);
}
// it handles the input no shit
void handleInput(chip8 &emulator, bool &running) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT)
      running = false;

    if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
      case SDLK_1:
        emulator.key[0x1] = 1;
        break;
      case SDLK_2:
        emulator.key[0x2] = 1;
        break;
      case SDLK_3:
        emulator.key[0x3] = 1;
        break;
      case SDLK_4:
        emulator.key[0xC] = 1;
        break;
      case SDLK_q:
        emulator.key[0x4] = 1;
        break;
      case SDLK_w:
        emulator.key[0x5] = 1;
        break;
      case SDLK_e:
        emulator.key[0x6] = 1;
        break;
      case SDLK_r:
        emulator.key[0xD] = 1;
        break;
      case SDLK_a:
        emulator.key[0x7] = 1;
        break;
      case SDLK_s:
        emulator.key[0x8] = 1;
        break;
      case SDLK_d:
        emulator.key[0x9] = 1;
        break;
      case SDLK_f:
        emulator.key[0xE] = 1;
        break;
      case SDLK_z:
        emulator.key[0xA] = 1;
        break;
      case SDLK_x:
        emulator.key[0x0] = 1;
        break;
      case SDLK_c:
        emulator.key[0xB] = 1;
        break;
      case SDLK_v:
        emulator.key[0xF] = 1;
        break;
      }
    }

    if (event.type == SDL_KEYUP) {
      switch (event.key.keysym.sym) {
      case SDLK_1:
        emulator.key[0x1] = 0;
        break;
      case SDLK_2:
        emulator.key[0x2] = 0;
        break;
      case SDLK_3:
        emulator.key[0x3] = 0;
        break;
      case SDLK_4:
        emulator.key[0xC] = 0;
        break;
      case SDLK_q:
        emulator.key[0x4] = 0;
        break;
      case SDLK_w:
        emulator.key[0x5] = 0;
        break;
      case SDLK_e:
        emulator.key[0x6] = 0;
        break;
      case SDLK_r:
        emulator.key[0xD] = 0;
        break;
      case SDLK_a:
        emulator.key[0x7] = 0;
        break;
      case SDLK_s:
        emulator.key[0x8] = 0;
        break;
      case SDLK_d:
        emulator.key[0x9] = 0;
        break;
      case SDLK_f:
        emulator.key[0xE] = 0;
        break;
      case SDLK_z:
        emulator.key[0xA] = 0;
        break;
      case SDLK_x:
        emulator.key[0x0] = 0;
        break;
      case SDLK_c:
        emulator.key[0xB] = 0;
        break;
      case SDLK_v:
        emulator.key[0xF] = 0;
        break;
      }
    }
  }
}
int main() {
  chip8 emulator;
  emulator.initailize();
  emulator.loadROM("PONG");

  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window =
      SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       640, 320, SDL_WINDOW_SHOWN);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

  bool running = true;

  Uint32 lastTimerUpdate = SDL_GetTicks();

  while (running) {

    // Run multiple CPU cycles per frame (
    for (int i = 0; i < 10; i++)
      emulator.cycle();

    // 60 Hz timer update
    Uint32 now = SDL_GetTicks();
    if (now - lastTimerUpdate >= 1000 / 60) {
      if (emulator.delay_timer > 0)
        emulator.delay_timer--;

      if (emulator.sound_timer > 0)
        emulator.sound_timer--;

      lastTimerUpdate = now;
    }

    if (emulator.drawFlag) {
      drawGraphics(renderer, emulator.gfx);
      emulator.drawFlag = false;
    }

    handleInput(emulator, running);

    SDL_Delay(16); // ~60 FPS
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
