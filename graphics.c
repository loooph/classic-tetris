#include "graphics.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "constants.h"

uint32_t *frame_buffer;

uint8_t vinfo_struct[160];

void setup_graphics()
{
    int fb0_fd = open("/dev/fb0", O_RDWR | O_SYNC);
    if (fb0_fd < 0) {
        perror("Fehler bei Framebufferöffnung");
        exit(1);
    }
    // resolution
    ioctl(fb0_fd, 17920, vinfo_struct);
    xres = *((uint32_t*) vinfo_struct);
    yres = *((uint32_t*) (vinfo_struct + 4));

    cell_size = yres / PLAYFIELD_HEIGHT;

    frame_buffer= mmap(NULL, xres * yres * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fb0_fd, 0);
    if (frame_buffer < 0) {
        perror("Fehler bei Speichermappe");
        exit(1);
    }

    close(fb0_fd);
}

void put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    frame_buffer[x + y * xres] = color;
}

void putrect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color)
{
    for (uint32_t x = x0; x < x1; ++x) {
        for (uint32_t y = y0; y < y1; ++y) {
            put_pixel(x, y, color);
        }
    }
}

void put_digit(uint32_t x, uint32_t y, uint32_t color, uint32_t digit)
{
    for (uint32_t i = 0; i < 5; ++i) {
        for (uint32_t k = 0; k < 3; ++k) {
            putrect(x + 8 * k, y + 8 * i, x + 8 * (k + 1), y + 8 * (i + 1), COLOR_I * NUMBERS[digit + 3 * i + k]);
        }
    }
}

void put_number(uint32_t x, uint32_t y, uint32_t color, uint32_t number)
{
    put_digit(x, y, color, number / 100);
    number %= 100;
    x += 32;
    put_digit(x, y, color, number / 10);
    number %= 10;
    x += 32;
    put_digit(x, y, color, number);
}
