#ifndef STRUCTS_H
#define STRUCTS_H

#define SCREENBUFFER_RESX 128
#define SCREENBUFFER_RESY 128
#define GRID_SIZE 8

#define MESSAGE_LOG_SIZE 10
#define MAX_MESSAGE_LENGTH 255

#define NUM_SHIPS 5

#include <Windows.h>
#include <stdint.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} Pixel;

typedef enum {
	Destroyer,
	Submarine,
	Cruiser,
	Battleship,
	Carrier
} ShipType;

typedef enum {
	UNPLACED,
	ALIVE,
	DEAD
} ShipState;

typedef struct {
	int x;
	int y;
} Vector;

typedef struct {
	ShipType type;
	ShipState state;
	int health;
	int num_hit_cells;
	Vector vector;
} Ship;

typedef enum {
	HIT,
	MISS,
	NONE
} CellState;

typedef struct {
	int x;
	int y;
	CellState state;
	Ship* ship;
} Cell;

typedef struct {
	Cell board[SCREENBUFFER_RESX / GRID_SIZE][SCREENBUFFER_RESY / GRID_SIZE];
} Board;

typedef struct {
	Ship ships[NUM_SHIPS];
} Player;

typedef enum {
	PLACING,
	TARGETING,
	DEFENDING,
	WELCOME,
	GAMEOVER,
} GameState;

typedef struct {
	int human_hits;
	int human_misses;
	int human_ships;
	int enemy_hits;
	int enemy_misses;
	int enemy_ships;
} CurrentGameStats;

#endif