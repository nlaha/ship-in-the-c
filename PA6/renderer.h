/*
	Author: Nathan Laha
	Name: renderer.h
	Description: handles rendering graphics to the 
	screen and adding/moving around sprites in the framebuffer.
*/

#ifndef RENDERER_H
#define RENDERER_H

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "structs.h"
#include "battleship.h"
#include "sprites.h"

extern Pixel screen_buffer[SCREENBUFFER_RESX][SCREENBUFFER_RESY];

double ClockToMilliseconds(clock_t ticks);
void Render(HWND hwnd);
void ClearScreenbuffer();
POINT GetMouseInGrid(HWND hwnd);

extern wchar_t* message_log[MESSAGE_LOG_SIZE];

void DrawImage(int x, int y, char* filename, HDC hdc);
void DrawSprite(int x, int y, int sprite[8][8], HDC hdc);
void PushMessage(wchar_t* message);

#endif