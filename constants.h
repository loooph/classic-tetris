#include <stdint.h>

#define PLAYFIELD_WIDTH 10
#define PLAYFIELD_HEIGHT 22
const uint32_t COLOR_I = 0x00FFFF;
const uint32_t COLOR_O = 0xFFFF00;
const uint32_t COLOR_T = 0x800080;
const uint32_t COLOR_S = 0x00FF00;
const uint32_t COLOR_Z = 0xFF0000;
const uint32_t COLOR_J = 0x0000FF;
const uint32_t COLOR_L = 0xFFA500;

const uint32_t COLORS[] = { COLOR_I, COLOR_O, COLOR_T, COLOR_S, COLOR_Z, COLOR_J, COLOR_L };
const uint32_t START_POSITIONS[] = { 0x03040506, 0x04050E0F, 0x0304050E, 0x04050D0E, 0x03040E0F, 0x0304050F, 0x0304050D };
const uint32_t ORIENTATIONS[] = { 0x14151617, 0x020c1620, 0x14151617, 0x020c1620, 0x0B0C1516, 0x0B0C1516, 0x0B0C1516, 0x0B0C1516, 0x0a0b0c15, 0x010a0b15, 0x010a0b0c, 0x010b0c15, 0x0b0c1415, 0x010b0c16, 0x0b0c1415, 0x010b0c16, 0x0a0b1516, 0x020b0c15, 0x0a0b1516, 0x020b0c15, 0x0a0b0c16, 0x010b1415, 0x000a0b0c, 0x01020b15, 0x0a0b0c14, 0x00010b15, 0x020a0b0c, 0x010b1516 };
const uint8_t NUMBERS[] = { 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0xFF, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0xFF, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0xFF, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0xFF, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0xFF, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0xFF, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0xFF, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0xFF, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0xFF, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0xFF };
