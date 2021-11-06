#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#include <stdlib.h>
#include <stdio.h>
#include "structs.h"
#include "renderer.h"

extern Board boards[2];
extern Player players[2];
extern int current_player;
extern GameState current_state;
extern int turn_done;
extern CurrentGameStats cgame_stats;

extern int ship_placement_index;
extern int winner;
extern int play_again;

void UpdateGame(HWND hwnd);

void RotatePlacementShip();
void PlacementMode(HWND hwnd);
void TargetingMode(HWND hwnd);
void ComputerMove(HWND hwnd);

void ClearBoards();
void RandomizeBoard(Player* player, Board* board);
void RandomizeShips(Player* player);

int InsideCell(const Cell* cell, int x, int y);
Cell* GetCell(int x, int y, int player);
Cell* GetCellNearPixel(int x, int y, int player);

#endif