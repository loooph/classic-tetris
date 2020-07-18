#include <stdint.h>

uint32_t xres;
uint32_t yres;
uint32_t cell_size;

void setup_graphics();
void put_pixel(uint32_t x, uint32_t y, uint32_t color);
void putrect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color);
void put_digit(uint32_t x, uint32_t y, uint32_t color, uint32_t digit);
void put_number(uint32_t x, uint32_t y, uint32_t color, uint32_t number);
