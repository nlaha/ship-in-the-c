/*
	Author: Nathan Laha
	Assignment: PA6 - Battleship
	Class: CPT_S 121
*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "battleship.h"
#include "renderer.h"

#define _CRT_SECURE_NO_WARNINGS

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Main function (program entrypoint like void main() but for windowed programs)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PWSTR lpCmdLine, int nCmdShow) {

    MSG  msg;
    WNDCLASSW wc = { 0 };

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = L"Battleship";
    wc.hInstance = hInstance;
    wc.hbrBackground = CreateSolidBrush(RGB(0,0,0)); // set background to black
    wc.lpfnWndProc = WndProc;
    wc.hCursor = LoadCursor(0, IDC_ARROW);

    RegisterClassW(&wc);
    CreateWindowW(wc.lpszClassName, L"Battleship",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 1280, 720, NULL, NULL, hInstance, NULL);

    while (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    srand((unsigned int)time(NULL));

    return (int)msg.wParam;
}

// callback
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam) {

    switch (msg) {
    case WM_CREATE:

        // init game logic
        // clear boards
        ClearBoards();
        
        // generate ships for both players
        RandomizeShips(&players[0]);
        RandomizeShips(&players[1]);

        // generate player 2's board
        RandomizeBoard(&players[1], &boards[1]);

        // enter placement mode
        current_state = PLACING;

        ClearScreenbuffer();

        return 0;

    case WM_PAINT:

        UpdateGame(hwnd);

        // render framebuffer
        // this allows for resizing the window 
        // while keeping a consistant aspect ratio
        if (!IsIconic(hwnd)) {
            Render(hwnd);
        }

        return 0;

    case WM_SIZE:
        return 0;

    case WM_MOVE:
        return 0;

    case WM_MOUSEMOVE:
        return 0;

    case WM_LBUTTONDOWN:

        switch (current_state) {
        case PLACING:
            PlacementMode(hwnd);
            break;
        case TARGETING:
            if (turn_done) {
                //PushMessage(L"CLICK TO CONTINUE");
                current_state = DEFENDING;
                turn_done = 0;
            }
            else {
                TargetingMode(hwnd);
            }
            break;
        case DEFENDING:
            if (turn_done) {
                current_state = TARGETING;
                turn_done = 0;
            }
            else {
                AIMove(hwnd);
            }
            break;
        }

        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_RETURN)
        {
            RotatePlacementShip();
        }

        return 0;
    case WM_DESTROY:

        PostQuitMessage(0);
        return 0;
    }

    InvalidateRect(hwnd, NULL, FALSE);
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}