#include "renderer.h"

Pixel screen_buffer[SCREENBUFFER_RESX][SCREENBUFFER_RESY];

int counter = 0;
clock_t deltaTime = 0;

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
            Cell *cplayer_cell = GetCell(x, y, current_player);
            Cell *oplayer_cell = GetCell(x, y, !current_player);

            if (InsideCell(cplayer_cell, cur.x, cur.y))
            {
                RECT cell_rect = {
                    .top = y * GRID_SIZE,
                    .bottom = (y * GRID_SIZE) + GRID_SIZE + 1,
                    .left = x * GRID_SIZE,
                    .right = (x * GRID_SIZE) + GRID_SIZE + 1,
                };

                FrameRect(memdc, &cell_rect, CreateSolidBrush(RGB(255, 255, 255)));
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
                break;
            case DEFENDING:
                // display own board when DEFENDING
                break;
            }
        }
    }

    HDC memdc_resized = CreateCompatibleDC(hdc);
    HBITMAP hbitmap_resized = CreateCompatibleBitmap(hdc, SCREENBUFFER_RESX * psize, SCREENBUFFER_RESY * psize);
    SelectObject(memdc_resized, hbitmap_resized);

    StretchBlt(memdc_resized, 0, 0, SCREENBUFFER_RESX * psize, SCREENBUFFER_RESY * psize, memdc, 0, 0, SCREENBUFFER_RESX, SCREENBUFFER_RESY, SRCCOPY);
    BitBlt(hdc, 0 + center_offset, 0, r.right, r.bottom, memdc_resized, 0, 0, SRCCOPY);

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

void LoadSprite(char *filename)
{
}