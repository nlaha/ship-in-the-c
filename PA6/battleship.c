#include "battleship.h"

#define _CRT_SECURE_NO_WARNINGS

Board boards[2];
Player players[2];
int current_player;
GameState current_state;
int turn_done = 0;

Cell *target_cell = NULL;
int spiral_x = 0;
int spiral_y = 0;
int spiral_dir_x = 0;
int spiral_dir_y = 0;

CurrentGameStats cgame_stats = {
	0,
	0,
	NUM_SHIPS,
	0,
	0,
	NUM_SHIPS};

int ship_placement_index = 0;
int winner = -1;
int play_again = 0;

/// <summary>
/// General game loop, called on WM_PComputerNT
/// </summary>
void UpdateGame(HWND hwnd)
{
	switch (current_state)
	{
	case PLACING:
		// player is always the one placing
		current_player = 0;
		break;
	case TARGETING:
		// player is always the one targeting
		current_player = 0;
		break;
	case DEFENDING:
		// Computer is doing it's turn during defending state
		current_player = 1;
		break;
	}

	if (cgame_stats.human_ships == 0)
	{
		winner = 1;
		fprintf(GetLogfile(), "Computer wins!\n");
	}

	if (cgame_stats.enemy_ships == 0)
	{
		winner = 0;
		fprintf(GetLogfile(), "Human wins!\n");
	}

	if (winner != -1)
	{
		current_state = GAMEOVER;
	}
}

/// <summary>
/// Rotates the currently placing ship
/// </summary>
/// <param name="hwnd"></param>
void RotatePlacementShip()
{
	if (current_state == PLACING)
	{
		int *vecx = &players[0].ships[ship_placement_index].vector.x;
		int *vecy = &players[0].ships[ship_placement_index].vector.y;

		if (*vecx > 0 || *vecy > 0)
		{
			*vecx *= -1;
			*vecy *= -1;
		}
		else
		{
			*vecx = !(*vecx);
			*vecy = !(*vecy);
		}
	}
}

/// <summary>
/// Called when a player clicks in placement mode,
/// handles ship placement and player selection
/// </summary>
/// <param name="hwnd"></param>
void PlacementMode(HWND hwnd)
{

	if (ship_placement_index >= NUM_SHIPS || players[0].ships[ship_placement_index].state == ALIVE)
	{
		// randomly select the starting player
		turn_done = 0;
		PushMessage(L"Randomly selecting player...");
		if (rand() & 1)
		{
			fprintf(GetLogfile(), "Computer is going first\n");
			PushMessage(L"Computer is going first!");
			PushMessage(L"CLICK TO CONTINUE");
			current_state = DEFENDING;
		}
		else
		{
			fprintf(GetLogfile(), "Human is going first\n");
			PushMessage(L"Human is going first!");
			PushMessage(L"Click on a cell to shoot");
			current_state = TARGETING;
		}
		return;
	}

	PushMessage(L"Click on a cell to place the ship");

	POINT cur = GetMouseInGrid(hwnd);

	Cell *clickedCell = GetCellNearPixel(cur.x, cur.y, 0);
	Ship *ship = &players[0].ships[ship_placement_index];
	int shipLength = ship->health;

	int invalid = 0;

	for (int i = 0; i < shipLength; i++)
	{
		Cell *cell = GetCell(clickedCell->x + (i * ship->vector.x), clickedCell->y + (i * ship->vector.y), 0);
		if (cell->ship != NULL)
		{
			invalid = 1;
		}
	}

	if (invalid == 0)
	{
		fprintf(GetLogfile(), "Human placed ship at (%d, %d)\n", clickedCell->x, clickedCell->y);
		ship->state = ALIVE;
		for (int i = 0; i < shipLength; i++)
		{
			Cell *cell = GetCell(clickedCell->x + (i * ship->vector.x), clickedCell->y + (i * ship->vector.y), 0);
			if (cell->ship == NULL)
			{
				cell->ship = ship;
			}
		}

		ship_placement_index++;
	}

	if (ship_placement_index >= NUM_SHIPS)
	{
		PushMessage(L"CLICK TO CONTINUE");
		turn_done = 1;
	}
}

/// <summary>
/// Called when the player is in targeting mode
/// it will hit or miss a ship depending on the location clicked
/// </summary>
/// <param name="hwnd"></param>
void TargetingMode(HWND hwnd)
{
	Cell *clickedCell = NULL;

	POINT cur = GetMouseInGrid(hwnd);
	// get on opposing player's board
	clickedCell = GetCellNearPixel(cur.x, cur.y, 1);

	if (clickedCell->state == NONE)
	{
		if (clickedCell->ship != NULL)
		{
			// hit!
			//PlaySound(TEXT("explosion.wav"), NULL, SND_ASYNC);
			clickedCell->state = HIT;
			clickedCell->ship->num_hit_cells++;
			cgame_stats.human_hits++;
			PushMessage(L"HIT! Click to fire again!");
			fprintf(GetLogfile(), "Human hit Computer ship at (%d, %d)\n", clickedCell->x, clickedCell->y);
			// don't end turn until the player doesn't hit a ship

			// check the status of the enemy's ships
			for (int i = 0; i < NUM_SHIPS; i++)
			{
				if (players[1].ships[i].state == ALIVE)
				{
					if (players[1].ships[i].num_hit_cells >= players[1].ships[i].type + 1)
					{
						fprintf(GetLogfile(), "Enemy ship destroyed!\n");
						PushMessage(L"Enemy ship destroyed!");
						cgame_stats.enemy_ships--;
						players[1].ships[i].state = DEAD;
					}
				}
			}
		}
		else
		{
			// miss
			//PlaySound(TEXT("click.wav"), NULL, SND_ASYNC);
			cgame_stats.human_misses++;
			clickedCell->state = MISS;
			fprintf(GetLogfile(), "Human missed at (%d, %d)\n", clickedCell->x, clickedCell->y);
			PushMessage(L"MISS! CLICK TO CONTINUE");
			turn_done = 1;
		}
	}
}

/// <summary>
/// This function represents the Computer's turn, it's called whenever the Computer
/// needs to shoot at the human player's board
/// </summary>
/// <param name="hwnd"></param>
void ComputerMove(hwnd)
{
	Cell *cell = NULL;

	while (cell == NULL || cell->state == NONE)
	{
		if (target_cell == NULL)
		{
			cell = GetCell(rand() % (SCREENBUFFER_RESX / GRID_SIZE), rand() % (SCREENBUFFER_RESY / GRID_SIZE), 0);
		}
		else
		{
			// try to find the rest of the ship by checking the adjacent cells, vector should be random
			int x = target_cell->x + (rand() % 3 - 1);
			int y = target_cell->y + (rand() % 3 - 1);
			cell = GetCell(x, y, 0);
		}

		if (cell->ship != NULL)
		{
			// hit!
			target_cell = cell;
			fprintf(GetLogfile(), "Computer hit human ship at (%d, %d)\n", cell->x, cell->y);
			cell->state = HIT;
			cell->ship->num_hit_cells++;
			cgame_stats.enemy_hits++;
			// let the Computer fire again

			// check the status of the human's ships
			for (int i = 0; i < NUM_SHIPS; i++)
			{
				if (players[0].ships[i].state == ALIVE)
				{
					if (players[0].ships[i].num_hit_cells >= players[0].ships[i].type + 1)
					{
						fprintf(GetLogfile(), "Human ship destroyed!\n");
						PushMessage(L"Human ship destroyed!");
						target_cell = NULL;
						cgame_stats.human_ships--;
						players[0].ships[i].state = DEAD;
					}
				}
			}

			PushMessage(L"HIT! CLICK TO CONTINUE");
		}
		else
		{
			// miss
			cgame_stats.enemy_misses++;
			fprintf(GetLogfile(), "Computer missed at (%d, %d)\n", cell->x, cell->y);
			PushMessage(L"MISS! CLICK TO CONTINUE");
			cell->state = MISS;
			turn_done = 1;
		}
	}
}

/// <summary>
/// This function clears the logical (not renderer) board
/// of cell structures
/// </summary>
void ClearBoards()
{
	for (int x = 0; x < SCREENBUFFER_RESX / GRID_SIZE; x++)
	{
		for (int y = 0; y < SCREENBUFFER_RESY / GRID_SIZE; y++)
		{
			Cell empty_cell = {
				.ship = NULL,
				.state = NONE,
				.x = x,
				.y = y,
			};

			boards[0].board[x][y] = empty_cell;
			boards[1].board[x][y] = empty_cell;
		}
	}
}

/// <summary>
/// This function gives a player some randomly generated
/// ships, the number of ships is defined by NUM_SHIPS
/// </summary>
void RandomizeShips(Player *player)
{
	for (int s = 0; s < NUM_SHIPS; s++)
	{
		int shipType = rand() % 5;
		int randDir = rand() % 2;
		Ship ship = {
			.state = UNPLACED,
			.type = shipType,
			.health = (shipType + 1),
			.num_hit_cells = 0,
			.vector = {randDir, !randDir},
		};

		player->ships[s] = ship;
	}
}

/// <summary>
/// This function randomizes a player's ships on the board
/// </summary>
void RandomizeBoard(Player *player, Board *board)
{
	int boardresx = SCREENBUFFER_RESX / GRID_SIZE;
	int boardresy = SCREENBUFFER_RESY / GRID_SIZE;

	for (int i = 0; i < NUM_SHIPS; i++)
	{
		int valid = 0;

		Ship *ship = &player->ships[i];
		ship->state = ALIVE;
		int x = 0;
		int y = 0;
		while (valid == 0)
		{
			x = rand() % boardresx - 1;
			y = rand() % boardresy - 1;

			// make sure, taking into account the ship vector and length that the ship isn't going off the board
			// also check if the ship is overlapping with another ship
			if (x + ship->vector.x * ship->health < boardresx && x + ship->vector.x * ship->health >= 0 &&
				y + ship->vector.y * ship->health < boardresy && y + ship->vector.y * ship->health >= 0)
			{
				valid = 1;
				for (int j = 0; j < ship->health; j++)
				{
					Cell *cell = &board->board[x + ship->vector.x * j][y + ship->vector.y * j];
					if (cell->ship != NULL)
					{
						valid = 0;
						break;
					}
				}
			}
		}

		fprintf(GetLogfile(), "Computer placing ship at (%d, %d)\n", x, y);
		for (int j = 0; j < ship->health; j++)
		{
			board->board[x + (j * ship->vector.x)][y + (j * ship->vector.y)].ship = ship;
		}
	}
}

/// <summary>
/// Checks if a coordinate is inside a certain cell
/// </summary>
int InsideCell(const Cell *cell, int x, int y)
{
	if (x < SCREENBUFFER_RESX && y < SCREENBUFFER_RESY)
	{
		if (x > 0 && y > 0)
		{
			if (cell->x * GRID_SIZE <= x && cell->x * GRID_SIZE + GRID_SIZE > x)
			{
				if (cell->y * GRID_SIZE <= y && cell->y * GRID_SIZE + GRID_SIZE > y)
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

/// <summary>
/// Finds the cell near a set of screen coordinates
/// </summary>
Cell *GetCellNearPixel(int x, int y, int player)
{
	return &boards[player].board[x / GRID_SIZE][y / GRID_SIZE];
}

/// <summary>
/// Finds the cell at specific board coordinates
/// </summary>
Cell *GetCell(int x, int y, int player)
{
	return &boards[player].board[x][y];
}