#include "renderer.h"

Pixel screen_buffer[SCREENBUFFER_RESX][SCREENBUFFER_RESY];

int counter = 0;
clock_t deltaTime = 0;

wchar_t *message_log[MESSAGE_LOG_SIZE];

/// <summary>
/// Converts clockcycles to milliseconds
/// </summary>
/// <param name="ticks"></param>
/// <returns></returns>
double ClockToMilliseconds(clock_t ticks)
{
    return (ticks / (double)CLOCKS_PER_SEC) * 1000.0;
}

/// <summary>
/// Gets the location of the mouse in game board units
/// </summary>
/// <param name="hwnd"></param>
/// <returns></returns>
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
    float center_offset = ((r.right - (r.left - 200)) / 2) - (SCREENBUFFER_RESX / 2) * psize;

    cur.x = cur.x / psize - (center_offset / psize);
    cur.y = cur.y / psize;

    return cur;
}

/// <summary>
/// main rendering funtion, called once per frame
/// </summary>
/// <param name="hwnd"></param>
void Render(HWND hwnd)
{
    RECT r;
    PAINTSTRUCT ps;

    GetClientRect(hwnd, &r);

    if (r.bottom == 0)
    {

        return;
    }

    HDC hdc = BeginPaint(hwnd, &ps);

    clock_t beginFrame = clock();

    float psize = (r.bottom - r.top) / (double)SCREENBUFFER_RESY;
    float center_offset = ((r.right - (r.left - 200)) / 2) - (SCREENBUFFER_RESX / 2) * psize;

    POINT cur = GetMouseInGrid(hwnd);

    HDC memdc = NULL;
    memdc = CreateCompatibleDC(hdc);
    HBITMAP holdbitmap = NULL;
    HBITMAP hbitmap = NULL;
    hbitmap = CreateCompatibleBitmap(hdc, SCREENBUFFER_RESX, SCREENBUFFER_RESY);
    holdbitmap = (HBITMAP)SelectObject(memdc, hbitmap);

    // update the background every second
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

    if (current_state == WELCOME)
    {
        DrawImage(0, 0, "background_welcome.bmp", memdc);
    }
    else if (current_state == GAMEOVER)
    {
        if (winner == 1) {
            // computer won
            DrawImage(0, 0, "background_lose.bmp", memdc);
            PushMessage(L"Click to play again!");
        }
        else if (winner == 0) {
            // human won
            DrawImage(0, 0, "background_win.bmp", memdc);
            PushMessage(L"Click to play again!");
        }
    }
    else
    {
        // Draw the cells
        for (int x = 0; x < SCREENBUFFER_RESX / GRID_SIZE; x++)
        {
            for (int y = 0; y < SCREENBUFFER_RESY / GRID_SIZE; y++)
            {
                // highlight cells the player is mousing over
                Cell *cplayer_cell = GetCell(x, y, 0);
                Cell *oplayer_cell = GetCell(x, y, 1);

                if (current_player == 0 && turn_done != 1)
                {
                    if (InsideCell(cplayer_cell, cur.x, cur.y))
                    {
                        RECT cell_rect = {
                            .top = y * GRID_SIZE,
                            .bottom = (y * GRID_SIZE) + GRID_SIZE + 1,
                            .left = x * GRID_SIZE,
                            .right = (x * GRID_SIZE) + GRID_SIZE + 1,
                        };

                        HBRUSH white_brush = CreateSolidBrush(RGB(255, 255, 255));
                        FrameRect(memdc, &cell_rect, white_brush);
                        DeleteObject(white_brush);

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

                        HBRUSH grey_brush = CreateSolidBrush(RGB(120, 120, 120));
                        FillRect(memdc, &ship_rect, grey_brush);
                        DeleteObject(grey_brush);
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
                        DrawSprite(x * GRID_SIZE, y * GRID_SIZE, miss_marker, memdc, RGB(200, 150, 0));
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

                        HBRUSH grey_brush = CreateSolidBrush(RGB(120, 120, 120));
                        FillRect(memdc, &ship_rect, grey_brush);
                        DeleteObject(grey_brush);
                    }

                    switch (cplayer_cell->state)
                    {
                    case HIT:
                        DrawSprite(x * GRID_SIZE, y * GRID_SIZE, hit_marker, memdc, RGB(255, 0, 0));
                        break;
                    case MISS:
                        DrawSprite(x * GRID_SIZE, y * GRID_SIZE, miss_marker, memdc, RGB(200, 150, 0));
                        break;
                    }
                    break;
                }
            }
        }
    }
    // we're drawing the game board on a lower res HDC
    // then we stretch it out and copy it to the main window HDC
    HDC memdc_resized = CreateCompatibleDC(hdc);
    HBITMAP hbitmap_resized = CreateCompatibleBitmap(hdc, SCREENBUFFER_RESX * psize, SCREENBUFFER_RESY * psize);
    SelectObject(memdc_resized, hbitmap_resized);

    StretchBlt(memdc_resized, 0, 0, SCREENBUFFER_RESX * psize, SCREENBUFFER_RESY * psize, memdc, 0, 0, SCREENBUFFER_RESX, SCREENBUFFER_RESY, SRCCOPY);
    BitBlt(hdc, 0 + center_offset, 0, r.right, r.bottom, memdc_resized, 0, 0, SRCCOPY);

    // Draw the game console log
    SetBkMode(hdc, OPAQUE);
    SetBkColor(hdc, RGB(0, 0, 0));

    RECT infopanel_rect = {
        .top = 50,
        .bottom = 255,
        .left = 0,
        .right = (center_offset - (SCREENBUFFER_RESX / 2)) + 50};

    wchar_t *info_panel[500];
    wchar_t *human_label = L"HUMAN";
    if (current_player)
    {
        human_label = L"COMPT";
    }

    SetTextColor(hdc, RGB(0, 255, 90));

    swprintf(info_panel, 500,
             L"Battleship\n\nCurrent Player: %s"
             L"\n\nShips [Human]: %d"
             L"\nHits/Misses [Human]: %d/%d"
             L"\nShips [Computer]: %d"
             L"\nHits/Misses [Computer]: %d/%d"
             L"\n\nMessage Log:",
             human_label,
             cgame_stats.human_ships,
             cgame_stats.human_hits,
             cgame_stats.human_misses,
             cgame_stats.enemy_ships,
             cgame_stats.enemy_hits,
             cgame_stats.enemy_misses);
    DrawText(hdc, info_panel, lstrlenW(info_panel), &infopanel_rect, DT_CENTER);

    SetTextColor(hdc, RGB(255, 90, 0));

    // fill the background of the message log with black
    RECT message_log_rect = {
        .top = 255,
        .bottom = r.bottom,
        .left = 0,
        .right = (center_offset - (SCREENBUFFER_RESX / 2)) + 50};

    HBRUSH black_brush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &message_log_rect, black_brush);
    DeleteObject(black_brush);

    // print each message in message_log to the screen in a list
    for (int i = 0; i < MESSAGE_LOG_SIZE; i++)
    {
        infopanel_rect.top = infopanel_rect.bottom;
        infopanel_rect.bottom = infopanel_rect.top + 15;

        // if this is the most recent message, print it in a different color
        if (i == 0)
        {
            SetTextColor(hdc, RGB(255, 90, 0));
        }
        else
        {
            // fade color brightness over time
            int fade_amount = ((MESSAGE_LOG_SIZE - i) * 255) / MESSAGE_LOG_SIZE;
            SetTextColor(hdc, RGB(fade_amount, fade_amount, fade_amount));
        }

        DrawText(hdc, message_log[i], lstrlenW(message_log[i]), &infopanel_rect, DT_CENTER);
    }

    // garbage collection after the render pass is done
    if (memdc)
    {
        SelectObject(memdc, holdbitmap);
        DeleteObject(hbitmap);
        DeleteDC(memdc);
    }

    if (hbitmap_resized)
    {
        SelectObject(memdc_resized, hbitmap_resized);
        DeleteObject(hbitmap_resized);
    }

    if (memdc_resized)
    {
        DeleteObject(memdc_resized);
    }

    ReleaseDC(hwnd, hdc);

    clock_t endFrame = clock();
    deltaTime += endFrame - beginFrame;
    counter++;

    EndPaint(hwnd, &ps);
}

/// <summary>
///  overwrites the screen buffer with a background
/// </summary>
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

/// <summary>
/// Draws an 8x8 sprite to an HDC
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="sprite"></param>
/// <param name="hdc"></param>
/// <param name="color"></param>
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

// opens a 16 bit bitmap file of size 128x128 and draws it to the hdc
void DrawImage(int x, int y, char *filename, HDC hdc)
{
    // open the bitmap file
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        return;
    }

    // read the bitmap file header
    BITMAPFILEHEADER fileheader;
    fread(&fileheader, sizeof(BITMAPFILEHEADER), 1, file);

    // read the bitmap info header
    BITMAPINFOHEADER infoheader;
    fread(&infoheader, sizeof(BITMAPINFOHEADER), 1, file);

    // allocate enough memory for the bitmap image data
    unsigned char *bitmap = (unsigned char *)malloc(infoheader.biSizeImage);

    // read the bitmap image data
    fread(bitmap, infoheader.biSizeImage, 1, file);

    // close the file
    fclose(file);

    // create a bitmap
    HBITMAP hbitmap = CreateDIBitmap(hdc, &infoheader, CBM_INIT, bitmap, (BITMAPINFO *)&infoheader, DIB_RGB_COLORS);

    // draw the bitmap to the hdc
    SelectObject(hdc, hbitmap);
    BitBlt(hdc, x, y, 128, 128, hdc, 0, 0, SRCCOPY);

    // free the memory
    free(bitmap);
}

/// <summary>
/// Pushes a message to the onscreen log
/// </summary>
/// <param name="message"></param>
void PushMessage(wchar_t *message)
{
    // append a prefix to the message based on current_player
    wchar_t *prefix = L"";
    if (current_player)
    {
        prefix = L"COMPT: ";
    }
    else
    {
        prefix = L"HUMAN: ";
    }

    // append the message to the message log
    wchar_t *message_log_entry = (wchar_t *)malloc(sizeof(wchar_t) * (lstrlenW(prefix) + lstrlenW(message) + 1));
    wcscpy(message_log_entry, prefix);
    wcscat(message_log_entry, message);

    // insert message to message_log
    for (int i = MESSAGE_LOG_SIZE - 1; i > 0; i--)
    {
        message_log[i] = message_log[i - 1];
    }

    if (current_state != WELCOME && current_state != GAMEOVER) {
        message_log[0] = message_log_entry;
    }
    else {
        message_log[0] = message;
    }
}