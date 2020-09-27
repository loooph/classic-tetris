#pragma once

#include <SDL2/SDL.h>

enum BUTTON {
    BUTTON_A,
    BUTTON_B,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_DOWN,
    NUM_BUTTONS
};

extern const int SDL_SCANCODES[NUM_BUTTONS];

extern const int SDL_BUTTONS[NUM_BUTTONS];

int get_rising_edge(int *last_inputs, enum BUTTON button, const uint8_t *keyboard_inputs);
void setup_inputs();
void update_das(int *inputs, const uint8_t *keyboard_inputs);
void get_inputs(int *last_inputs, int *inputs);
