#include "game.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "constants.h"
#include "graphics.h"
#include "input.h"

uint32_t rng_state = 1337;

uint32_t score;
uint32_t play_field[PLAYFIELD_WIDTH * PLAYFIELD_HEIGHT];
uint32_t frame_counter;

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

void rotate_if_possible(struct piece *current_piece, int32_t direction)
{
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
