/**
 * This file includes some functionalities and general ideas from Connect4 Game Solver.
 * That project includes code in C++ to analyse positions and compute scores of all possible moves
 * in the game. The code is published under the AGPL v3 license.
 * Author: Pascal Pons
 * Link to the repository: https://github.com/PascalPons/connect4
 */

/**
 * NOTE
 * the methods included in this header file correspond to the negamax algorithm
 * from before the bitboard implementation, the difference is the board here is
 * one array of ints
 * because of that, the methods used for game logic and the computation of the next
 * computer move differ (are based on mathematical operations)
 */

#ifndef GAME_BOARD_H
#define GAME_BOARD_H

#include <stdbool.h>

// specify board dimensions
#define ROWS 6
#define COLS 7
// specify game move details
#define HUMAN 2
#define COMPUTER 3
#define EMPTY 0
// specify scores for evaluation
#define WIN_SCORE 110
#define HIGH_SCORE 50
#define MEDIUM_SCORE 20

typedef struct {
    int board[ROWS*COLS]; //using modulo -- "field_idx = row * COLS + col"
    char position_notation[ROWS*COLS];
} board;

typedef struct {
    int col;
    int score;
} move;

/**
 * fills the @param board with zeros
 * meaning EMPTY cells
 */
void initializeBoard(int *board);

/**
 * prints out the present @param board set up
 */
__attribute__((unused)) void printBoard(const int *board);

/**
 * checks whether the move into @param col is possible;
 * check whether a column is full
 * @return true is the move is valid
 */
bool can_add_coin(const int board[ROWS*COLS], int col);

/**
 * for a specific @param turn, it updates the virtual connect 4 @param board;
 * adds a coin associated with player's @param turn into specif @param col
 */
void add_coin(int *board, int col, int turn);

/**
 * check if there is a line of four on the referenced @param board
 */
bool check_four(const int board[ROWS*COLS]);

/**
 * checks whether a specific @param col in a @param board is full;
 * necessary to check available moves
 * @return true if the top field within the column is not empty
 */
bool is_column_full(const int board[ROWS*COLS], int col);

/**
 * @return the row of the first 0 in the @param col
 */
int filled_level(const int board[ROWS*COLS], int col);

/**
 * @return number of moves made since the beginning of the game;
 * based on non-zero fields on the board
 */
int getNoMoves(const int board[ROWS*COLS]);

/**
 * copies the contents of @param board into @param copy
 */
void copy_board(const int board[ROWS*COLS], int copy[ROWS*COLS]);

/**
 * checks whether the move into @param col leads to immediate win;
 * the methods are grouped since they have the same purpose but are checking
 * different orientations of the lines
 */
bool is_it_win_move(const int board[ROWS*COLS], int col, int turn);
bool check_vertical(const int board[ROWS*COLS], int col,int turn);
bool check_horizontal(const int board[ROWS*COLS], int col,int turn);
bool check_diagonal_positive(const int board[ROWS*COLS], int col,int turn);
bool check_diagonal_negative(const int board[ROWS*COLS], int col,int turn);

/**
 * computes the score of the specific @param board set up;
 * the methods are grouped since they have the same purpose but are
 * evaluating different orientations of lines
 */
int evaluate(const int board[ROWS*COLS]);
int evaluate_vertical(const int board[ROWS*COLS]);
int evaluate_horizontal(const int board[ROWS*COLS]);
int evaluate_positive_diagonal(const int board[ROWS*COLS]);
int evaluate_negative_diagonal(const int board[ROWS*COLS]);

/**
 * simple negamax algorithm to find the next best move on
 * the @param board, for the player's @param turn;
 * checks only for specific @param depth of the search tree
 */
move negamax(int board[ROWS*COLS], int depth, int turn);

/**
 * negamax algorithm with alpha-beta pruning implementation
 * with ints (not bits)
 */
move negamax_ab(int board[ROWS*COLS], int alpha, int beta, int depth, int turn);

#endif // GAME_BOARD_H
