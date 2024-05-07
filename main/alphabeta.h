/**
 * This file includes some functionalities and general ideas from Connect4 Game Solver.
 * That project includes code in C++ to analyse positions and compute scores of all possible moves
 * in the game. The code is published under the AGPL v3 license.
 * Author: Pascal Pons
 * Link to the repository: https://github.com/PascalPons/connect4
 */

#ifndef GAME_ALPHABETA_H
#define GAME_ALPHABETA_H

#include "board.h"

typedef struct {
    uint64_t position;
    uint64_t mask;
    int no_moves;
} bitboard;

/**
 * check if a column is playable, which means
 * check whether a column is full
 */
bool can_play(uint64_t bb_mask, int col);

/**
 * make move into column:
 * 1) switch bits of current player and opponent
 * 2) add extra bit to mask (into specific @param col)
 */
void play(bitboard *bb, int col);

/**
 * method checks whether there is a four in a row
 * for the current player
 */
bool check_win(uint64_t position);

/**
 * checks whether the move into @param col
 * leads to immediate win
 */
bool is_win(bitboard bb, int col);

/**
 * @param width is the number of columns
 * @return order in which the columns are explored
 * from centre to the edges
 */
int* get_exploration_order(int width);

/**
 * negamax algorithm with alpha-beta pruning
 * with bitboards implementation and specific @param depth
 */
move negamax_ab_bb(const bitboard bb, int alpha, int beta, int depth);

/**
 * generate a score for a specific position board
 * with the current player's coins in row
 * (or column, or diagonal) in mind (hence, @param position);
 * higher score is generated when board has 3 or 2 in a line
 */
int evaluate_bb(const uint64_t position);

// HELPER METHODS
/**
 * aids in identification of the lowest available slot
 * in a specific column of a bitwise representation of a game board;
 */
uint64_t bottom_cell(int col);

/**
 * aids in identification of top cell in a specific column
 * of a bitwise representation of a game board
 */
uint64_t top_cell(int col);

/**
 * aids in selection of a specific column from a board
 */
uint64_t get_column(int col);

#endif //GAME_ALPHABETA_H