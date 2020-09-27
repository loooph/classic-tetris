#include "input.h"

const int SDL_SCANCODES[NUM_BUTTONS] = {
    SDL_SCANCODE_K,
    SDL_SCANCODE_J,
    SDL_SCANCODE_A,
    SDL_SCANCODE_D,
    SDL_SCANCODE_S
};

const int SDL_BUTTONS[NUM_BUTTONS] = {
    SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN
};

SDL_GameController* controller = NULL;

int get_rising_edge(int *last_inputs, enum BUTTON button, const uint8_t *keyboard_inputs)
{
    int last_val = last_inputs[button];
    last_inputs[button] = SDL_GameControllerGetButton(controller, SDL_BUTTONS[button]) || keyboard_inputs[SDL_SCANCODES[button]];
    if (last_val) {
        return 0;
    } else {
        return last_inputs[button];
    }
}

void setup_inputs()
{
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_EVENTS);
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                break;
            } else {
                fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
                exit(1);
            }
        }
    }
}

void update_das(int *inputs, const uint8_t *keyboard_inputs)
{
    static int das_counter = 0;
    static int das_frame = 0;
    enum BUTTON button = 0;
    for (enum BUTTON b = BUTTON_LEFT; b <= BUTTON_DOWN; ++b) {
        if (SDL_GameControllerGetButton(controller, SDL_BUTTONS[b]) || keyboard_inputs[SDL_SCANCODES[b]]) {
            button = b;
            break;
        }
    }
    if (!button) {
        das_counter = 0;
        das_frame = 0;
        return;
    } else {
        if (das_counter < 24) {
            ++das_counter;
        } else {
            if (das_frame == 0) {
                inputs[button] = 1;
            }
            das_frame = (das_frame + 1) % 8;
        }
    }
}

void get_inputs(int *last_inputs, int *inputs)
{
    SDL_GameControllerUpdate();
    const uint8_t *keyboard_inputs = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        inputs[i] = get_rising_edge(last_inputs, i, keyboard_inputs);
    }
    update_das(inputs, keyboard_inputs);
}
