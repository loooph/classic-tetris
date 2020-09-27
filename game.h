#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef union {
    uint32_t as_int;
    uint8_t as_vec[4];
} piece_positions;

struct piece {
    piece_positions positions;
    uint32_t color;
    uint32_t orientiation;
};

void reset_playfield();
void draw_playfield();
int32_t is_free(struct piece *current_piece, uint32_t delta_pos_int);
int32_t piece_can_fall(struct piece *current_piece);
void rotate_if_possible(struct piece *current_piece, int32_t direction);
void kill_myself();
int can_move_left_mod(struct piece *current_piece);
void move_left(struct piece *current_piece);
int can_move_left(struct piece *current_piece);
void move_right(struct piece *current_piece);
int can_move_right(struct piece *current_piece);
void process_input(int *last_inputs, struct piece *current_piece);
void clear_line(size_t line_index);
void clear_lines();
uint32_t next_int();
void generate_next_piece(struct piece *current_piece);
int process_falldown(struct piece *current_piece);
int next_state(int *last_input, struct piece *current_piece);
