#include "renderer.h"

Pixel screen_buffer[SCREENBUFFER_RESX][SCREENBUFFER_RESY];

int counter = 0;
clock_t deltaTime = 0;

wchar_t* message_log[10];

double ClockToMilliseconds(clock_t ticks)
{
    return (ticks / (double)CLOCKS_PER_SEC) * 1000.0;
}

POINT GetMouseInGrid(HWND hwnd)
{
    POINT cur;
    GetCursorPos(&cur);
    if (!ScreenToClient(hwnd, &cur))
    {
        cur.x = 0;
        cur.y = 0;
    }

    RECT r;
    GetClientRect(hwnd, &r);

    float psize = (r.bottom - r.top) / (double)SCREENBUFFER_RESY;
    float center_offset = ((r.right - r.left) / 2) - (SCREENBUFFER_RESX / 2) * psize;

    cur.x = cur.x / psize - (center_offset / psize);
    cur.y = cur.y / psize;

    return cur;
}

// main rendering funtion, called once per frame
void Render(HWND hwnd)
{
    clock_t beginFrame = clock();

    RECT r;
    PAINTSTRUCT ps;

    GetClientRect(hwnd, &r);

    if (r.bottom == 0)
    {

        return;
    }

    HDC hdc = BeginPaint(hwnd, &ps);
    HDC memdc = CreateCompatibleDC(hdc);

    float psize = (r.bottom - r.top) / (double)SCREENBUFFER_RESY;
    float center_offset = ((r.right - r.left) / 2) - (SCREENBUFFER_RESX / 2) * psize;

    POINT cur = GetMouseInGrid(hwnd);

    HBITMAP hbitmap = CreateCompatibleBitmap(hdc, SCREENBUFFER_RESX, SCREENBUFFER_RESY);
    SelectObject(memdc, hbitmap);

    if (ClockToMilliseconds(deltaTime) > 1000.0)
    {
        deltaTime -= CLOCKS_PER_SEC;
        counter = 0;
        ClearScreenbuffer();
    }

    for (int x = 0; x < SCREENBUFFER_RESX; x++)
    {
        for (int y = 0; y < SCREENBUFFER_RESY; y++)
        {
            SetPixel(memdc, x, y, RGB(screen_buffer[x][y].r, screen_buffer[x][y].g, screen_buffer[x][y].b));
        }
    }

    // draw cusor
    //RECT cursor_rect = {
    //    .top = cur.y,
    //    .bottom = cur.y + 2,
    //    .left = cur.x,
    //    .right = cur.x + 2
    //};

    //FillRect(memdc, &cursor_rect, CreateSolidBrush(RGB(255, 255, 255)));

    // Draw the cells
    for (int x = 0; x < SCREENBUFFER_RESX / GRID_SIZE; x++)
    {
        for (int y = 0; y < SCREENBUFFER_RESY / GRID_SIZE; y++)
        {
            // highlight cells the player is mousing over
            Cell *cplayer_cell = GetCell(x, y, 0);
            Cell *oplayer_cell = GetCell(x, y, 1);

            if (current_player == 0) {
                if (InsideCell(cplayer_cell, cur.x, cur.y))
                {
                    RECT cell_rect = {
                        .top = y * GRID_SIZE,
                        .bottom = (y * GRID_SIZE) + GRID_SIZE + 1,
                        .left = x * GRID_SIZE,
                        .right = (x * GRID_SIZE) + GRID_SIZE + 1,
                    };

                    FrameRect(memdc, &cell_rect, CreateSolidBrush(RGB(255, 255, 255)));

                    if (current_state == PLACING)
                    {
                        // draw direction arrow
                        int vecx = players[0].ships[ship_placement_index].vector.x;
                        int vecy = players[0].ships[ship_placement_index].vector.y;
                        DrawSprite(cell_rect.left + (vecx * 8), cell_rect.top + (vecy * 8), cursor_arrow, memdc, RGB(255, 255, 255));
                    }
                }
            }

            switch (current_state)
            {
            case PLACING:

                // display own board when placing
                if (cplayer_cell->ship != NULL)
                {
                    RECT ship_rect = {
                        .top = y * GRID_SIZE + 2,
                        .bottom = (y * GRID_SIZE) + GRID_SIZE - 1,
                        .left = x * GRID_SIZE + 2,
                        .right = (x * GRID_SIZE) + GRID_SIZE - 1,
                    };

                    FillRect(memdc, &ship_rect, CreateSolidBrush(RGB(120, 120, 120)));
                }

                break;
            case TARGETING:

                // display opposing board (minus ships) when targeting
                switch (oplayer_cell->state)
                {
                case HIT:
                    DrawSprite(x * GRID_SIZE, y * GRID_SIZE, hit_marker, memdc, RGB(255, 0, 0));
                    break;
                case MISS:
                    DrawSprite(x * GRID_SIZE, y * GRID_SIZE, miss_marker, memdc, RGB(90, 150, 0));
                    break;
                }
                break;
            case DEFENDING:
                // display own board when DEFENDING
                if (cplayer_cell->ship != NULL)
                {
                    RECT ship_rect = {
                        .top = y * GRID_SIZE + 2,
                        .bottom = (y * GRID_SIZE) + GRID_SIZE - 1,
                        .left = x * GRID_SIZE + 2,
                        .right = (x * GRID_SIZE) + GRID_SIZE - 1,
                    };

                    FillRect(memdc, &ship_rect, CreateSolidBrush(RGB(120, 120, 120)));
                }

                switch (cplayer_cell->state)
                {
                case HIT:
                    DrawSprite(x * GRID_SIZE, y * GRID_SIZE, hit_marker, memdc, RGB(255, 0, 0));
                    break;
                case MISS:
                    DrawSprite(x * GRID_SIZE, y * GRID_SIZE, miss_marker, memdc, RGB(90, 150, 0));
                    break;
                }
                break;
            }
        }
    }

    HDC memdc_resized = CreateCompatibleDC(hdc);
    HBITMAP hbitmap_resized = CreateCompatibleBitmap(hdc, SCREENBUFFER_RESX * psize, SCREENBUFFER_RESY * psize);
    SelectObject(memdc_resized, hbitmap_resized);

    StretchBlt(memdc_resized, 0, 0, SCREENBUFFER_RESX * psize, SCREENBUFFER_RESY * psize, memdc, 0, 0, SCREENBUFFER_RESX, SCREENBUFFER_RESY, SRCCOPY);
    BitBlt(hdc, 0 + center_offset, 0, r.right, r.bottom, memdc_resized, 0, 0, SRCCOPY);

    SetBkMode(hdc, TRANSPARENT);

    RECT infopanel_rect = {
        .top = 50,
        .bottom = 255,
        .left = 50,
        .right = 255
    };

    wchar_t* info_panel[100];
    wchar_t* warning_panel[100];
    wchar_t* human_label = L"HUMAN";
    if (current_player) {
        human_label = L"COMPT";
    }

    SetTextColor(hdc, RGB(0, 255, 90));

    swprintf(info_panel, 100, L"Battleship\n\nCurrent Player: %s", human_label);
    DrawText(hdc, info_panel, lstrlenW(info_panel), &infopanel_rect, 0);

    SetTextColor(hdc, RGB(255, 90, 0));

    for (int m = 0; m < 10; m++) {
        RECT warning_rect = {
            .top = 175 + (m * 2),
            .bottom = 255,
            .left = 50,
            .right = 255
        };
        DrawText(hdc, message_log[m], lstrlenW(message_log[m]), &warning_rect, 0);
    }

    DeleteDC(memdc);
    DeleteDC(memdc_resized);

    EndPaint(hwnd, &ps);

    clock_t endFrame = clock();
    deltaTime += endFrame - beginFrame;
    counter++;
}

// overwrites the screen buffer with a background
void ClearScreenbuffer()
{
    for (int x = 0; x < SCREENBUFFER_RESX; x++)
    {
        for (int y = 0; y < SCREENBUFFER_RESY; y++)
        {
            // draw grid
            if (x % GRID_SIZE == 0)
            {
                screen_buffer[x][y] = (Pixel){25, 25, 25};
            }
            else if (y % GRID_SIZE == 0)
            {
                screen_buffer[x][y] = (Pixel){25, 25, 25};
            }
            else
            {
                // draw ocean
                screen_buffer[x][y] = (Pixel){0, rand() % 80, (sin((x + y) * 0.5 + rand() % 3) + 3) * 50};
            }
        }
    }
}

void DrawSprite(int x, int y, int sprite[8][8], HDC hdc, COLORREF color)
{
    for (int px = x; px < x + 8; px++)
    {
        for (int py = y; py < y + 8; py++)
        {
            if (sprite[px - x][py - y])
            {
                SetPixel(hdc, px, py, color);
            }
        }
    }
}

void PushMessage(wchar_t* message)
{
    for (int i = 10 - 1; i >= 0; i--)
        message_log[i] = message_log[i - 1];
    message_log[0] = message;
}