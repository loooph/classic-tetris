#include <SDL2/SDL.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef union {
    uint32_t as_int;
    uint8_t as_vec[4];
} piece_positions;

struct piece {
    piece_positions positions;
    uint32_t color;
    uint32_t orientiation;
};

enum BUTTON {
    BUTTON_A,
    BUTTON_B,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_DOWN,
    NUM_BUTTONS
};

const int SDL_BUTTONS[NUM_BUTTONS] = {
    SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN
};


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

uint32_t play_field[PLAYFIELD_WIDTH * PLAYFIELD_HEIGHT];

uint32_t frame_counter;
uint32_t score;
uint32_t xres;
uint32_t yres;
uint32_t cell_size;
uint32_t *frame_buffer;

uint8_t vinfo_struct[160];

const uint8_t NUMBERS[] = { 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0xFF, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0xFF, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0xFF, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0xFF, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0xFF, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0xFF, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0xFF, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0xFF, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0xFF, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0xFF };

uint32_t rng_state = 1337;

SDL_GameController* controller = NULL;

void setup_inputs()
{
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE);
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
    //printf("%d\n", SDL_NumJoysticks());
}


int get_rising_edge(int *last_inputs, enum BUTTON button)
{
    int last_val = last_inputs[button];
    last_inputs[button] = SDL_GameControllerGetButton(controller, SDL_BUTTONS[button]);
    if (last_val) {
        return 0;
    } else {
        return last_inputs[button];
    }
}

void update_das(int *inputs)
{
    static int das_counter = 0;
    static int das_frame = 0;
    enum BUTTON button = 0;
    for (enum BUTTON b = BUTTON_LEFT; b <= BUTTON_DOWN; ++b) {
        if (SDL_GameControllerGetButton(controller, SDL_BUTTONS[b])) {
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
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        inputs[i] = get_rising_edge(last_inputs, i);
    }
    update_das(inputs);
}

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

void alarm_handler()
{
}

void reset_playfield()
{
    frame_counter = 0;
    score = 0;
    putrect(0, 0, xres, yres, 0x7A7A7A7A);
    for (size_t i = 0; i < PLAYFIELD_WIDTH * PLAYFIELD_HEIGHT; ++i) {
        play_field[i] = 0;
    }
}

void draw_playfield()
{
    if (score > 999) {
        score = 420;
    }
    put_number(xres / 2 - 48, 16, 0xFF070000, score);
    uint32_t x_off = xres / 2 - cell_size * PLAYFIELD_WIDTH / 2;
    uint32_t y_off = 2 * cell_size;
    // lines
    for (size_t i = 0; i < PLAYFIELD_HEIGHT - 2; ++i) {
        for (size_t k = 0; k < PLAYFIELD_WIDTH; ++k) {
            putrect(k * cell_size + x_off, i * cell_size + y_off, (k + 1) * cell_size + x_off, (i + 1) * cell_size + y_off, play_field[(i + 2) * PLAYFIELD_WIDTH + k]);
        }
    }
}

int32_t is_free(struct piece *current_piece, uint32_t delta_pos_int)
{
    uint8_t *delta_pos = (uint8_t*) &delta_pos_int;
    struct piece new_piece;
    memcpy(&new_piece, current_piece, sizeof(struct piece));
    for (size_t i = 0; i < 4; ++i) {
        new_piece.positions.as_vec[i] += delta_pos[i];
        if (new_piece.positions.as_vec[i] > 219) {
            return 0;
        }
        int32_t goto_next_piece = 0;
        for (size_t k = 0; k < 4 && !goto_next_piece; ++k) {
            if (new_piece.positions.as_vec[i] == current_piece->positions.as_vec[k]) {
                goto_next_piece = 1;
            }
        }
        if (!goto_next_piece && play_field[new_piece.positions.as_vec[i]] != 0) {
            return 0;
        }
    }
    return 1;
}

int32_t piece_can_fall(struct piece *current_piece)
{
    return is_free(current_piece, 0x0a0a0a0a);
}

void rotate_if_possible(struct piece *current_piece, int32_t direction) {
    uint32_t new_orientation = ((current_piece->orientiation + direction) & 3) | (current_piece->orientiation & ~3);
    uint32_t delta_pos_int = ORIENTATIONS[new_orientation] - ORIENTATIONS[current_piece->orientiation];
    if(is_free(current_piece, delta_pos_int)) {
        current_piece->positions.as_int += delta_pos_int;
        current_piece->orientiation = new_orientation;
    }
}

void kill_myself()
{
    pid_t pid = getpid();
    kill(pid, SIGKILL);

    // if everything else fails
    volatile int32_t important = *((volatile int32_t*) NULL);
}

int can_move_left_mod(struct piece *current_piece)
{
    for (size_t i = 0; i < 4; ++i) {
        if (current_piece->positions.as_vec[i] % PLAYFIELD_WIDTH == 0) {
            return 0;
        }
    }
    return 1;
}

void move_left(struct piece *current_piece)
{
    current_piece->positions.as_int -= 0x01010101;
}

int can_move_left(struct piece *current_piece)
{
    if (!is_free(current_piece, -1u)) {
        return 0;
    }
    return can_move_left_mod(current_piece);
}

void move_right(struct piece *current_piece)
{
    current_piece->positions.as_int += 0x01010101;
}

int can_move_right(struct piece *current_piece)
{
    if (!is_free(current_piece, 0x01010101)) {
        return 0;
    }
    struct piece new_piece;
    memcpy(&new_piece, current_piece, sizeof(struct piece));
    move_right(&new_piece);
    return can_move_left_mod(&new_piece);
}

void process_input(int *last_inputs, struct piece *current_piece)
{
    int inputs[NUM_BUTTONS];
    get_inputs(last_inputs, inputs);

    if (inputs[BUTTON_DOWN]) {
        kill_myself();
    }

    int32_t rotation = 0;
    if (inputs[BUTTON_B]) {
        rotation = -1;
    }
    if (inputs[BUTTON_A]) {
        rotation = 1;
    }
    if (rotation) {
        rotate_if_possible(current_piece, rotation);
    }
    if (inputs[BUTTON_LEFT]) {
        if (can_move_left(current_piece)) {
            move_left(current_piece);
        }
    }
    if (inputs[BUTTON_RIGHT]) {
        if (can_move_right(current_piece)) {
            move_right(current_piece);
        }
    }
}

void clear_line(size_t line_index)
{
    for (size_t i = (line_index + 1) * PLAYFIELD_WIDTH - 1; PLAYFIELD_WIDTH <= i; --i) {
        play_field[i] = play_field[i - PLAYFIELD_WIDTH];
    }
}

void clear_lines()
{
    uint32_t line_counter = 0;
    for (size_t i = 0; i < PLAYFIELD_HEIGHT; ++i) {
        int is_full = 1;
        for (size_t k = 0; k < PLAYFIELD_WIDTH; ++k) {
            if (play_field[i * PLAYFIELD_WIDTH + k] == 0) {
                is_full = 0;
            }
        }
        if (is_full) {
            clear_line(i);
            ++line_counter;
        }
    }
    score += line_counter * line_counter;
}

uint32_t next_int()
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

void generate_next_piece(struct piece *current_piece)
{
    uint32_t next_piece_num;
    do {
        next_piece_num = next_int() & 7u;
    } while (next_piece_num == 7);
    current_piece->positions.as_int = START_POSITIONS[next_piece_num];
    current_piece->orientiation = next_piece_num * 4;
    current_piece->color = COLORS[next_piece_num];
}

int process_falldown(struct piece *current_piece)
{
    if (piece_can_fall(current_piece)) {
        current_piece->positions.as_int += 0x0a0a0a0a;
    } else {
        for (size_t i = 0; i < 4; ++i) {
            play_field[current_piece->positions.as_vec[i]] = current_piece->color;
        }
        clear_lines();
        generate_next_piece(current_piece);
        if (!piece_can_fall(current_piece)) {
            return 0;
        }
    }
    return 1;
}

int next_state(int *last_input, struct piece *current_piece)
{
    for (size_t i = 0; i < 4; ++i) {
        play_field[current_piece->positions.as_vec[i]] = 0;
    }
    process_input(last_input, current_piece);
    ++frame_counter;
    if ((frame_counter %= 6) == 0) {
        if (!process_falldown(current_piece)) {
            return 0;
        }
    }

    for (size_t i = 0; i < 4; ++i) {
        play_field[current_piece->positions.as_vec[i]] = current_piece->color;
    }
    return 1;
}

uint32_t struct_itimverval[] = { 0, 10000, 0, 10000 };
uint32_t struct_sa_handler[] = { 0, 0, 0, 0, 0 };
uint32_t struct_timespec[] = { 1, 0 };

struct itimerval val;
struct timeval interval;
struct timeval value;

int main()
{
    // ignore alarm signal
    //*struct_sa_handler = (uint32_t) &alarm_handler;
    //syscall(SYS_rt_sigaction, SIGALRM, (const struct sigaction*) struct_sa_handler, NULL);
    interval.tv_sec = 0;
    interval.tv_usec = 10000;
    value.tv_sec = 0;
    value.tv_usec = 10000;
    val.it_interval = interval;
    val.it_value = value;
    signal(SIGALRM, alarm_handler);

    // set a periodic interrupt
    syscall(SYS_setitimer, ITIMER_REAL, &val, NULL);

    setup_graphics();
    setup_inputs();

    for (;;) {
        struct piece current_piece;
        int last_inputs[NUM_BUTTONS] = { 0 };
        reset_playfield();
        draw_playfield();
        generate_next_piece(&current_piece);

        while (next_state(last_inputs, &current_piece)) {
            draw_playfield();
            syscall(SYS_nanosleep, struct_timespec, NULL);
        }
    }

    exit(0);
}
