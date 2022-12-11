#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef WASM
#include <emscripten.h>
#endif

#include "platform.h"
#include "sine_lut.h"

#include "font_big.h"
#include "font_small.h"

#if defined(PLATFORM_CANON_SDL) || defined(PLATFORM_LEXMARK_SDL)
#include <unistd.h>
#define RENDER_DELAY_US 7000
#endif

#if defined(PLATFORM_CANON) || defined(PLATFORM_CANON_SDL)
// half of the actual screen dimensions, upscaled in platform code
#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240
#define WRITER_X 50
#define BANNER_X 60
#define PRINTER_NAME "CANON  "
#elif defined(PLATFORM_LEXMARK) || defined(PLATFORM_LEXMARK_SDL)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define WRITER_X 10
#define BANNER_X 15
#define PRINTER_NAME "LEXMARK"
#else
#error "Illegal platform"
#endif

#define FRAMEBUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT * 4)

#define RGB24(r, g, b) (((r) << 16) | ((g) << 8) | (b))
#define WRITER_Y 90
#define CLAMP(v, n) (((v) > (n)) ? (n) : (v))
#define WRITER_COLOR RGB24(0xff, 0xff, 0xff)

static uint32_t *g_fb32 = (uint32_t *)1;
static uint32_t g_tick = 1;

typedef struct
{
    int char_width;
    int char_height;
    uint8_t *data;
} font_t;

static font_t *font_big;
static font_t *font_small;
static uint8_t *framebuffer;

size_t _strlen(const char *s)
{
    int sz = 0;
    while (*s != 0)
    {
        sz++;
        s++;
    }
    return sz;
}

font_t *font_load_memory(uint8_t *data, int w, int h)
{
    font_t *r = platform_malloc(sizeof(font_t));
    r->char_width = w;
    r->char_height = h;
    r->data = platform_malloc(w * h * 256);

    int offs = 0;

    for (int i = 0; i < w * h * 32; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            r->data[offs + j] = (data[i] & (1 << j)) ? 0xff : 0;
        }

        offs += 8;
    }

    return r;
}

void font_draw_char(font_t *f, int dx, int dy, unsigned char c, uint32_t color)
{
    for (int y = 0; y < f->char_height; y++)
    {
        uint32_t *p = g_fb32 + ((dy + y) * SCREEN_WIDTH) + dx;
        int idx = (f->char_width * f->char_height * c) + (f->char_width * y);
        for (int x = 0; x < f->char_width; x++)
        {
            if (f->data[idx + x] != 0)
            {
                *p++ = color;
            }
            else
            {
                p++;
            }
        }
    }
}

void font_draw_str(font_t *f, int dx, int dy, char *str, uint32_t color)
{
    int xpos = dx;
    int ypos = dy;

    for (int i = 0; i < _strlen(str); i++)
    {
        font_draw_char(f, xpos, ypos, str[i], color);
        xpos += f->char_width;
    }
}

void font_draw_str_wave(font_t *f, int dx, int dy, char *str, uint32_t color, uint32_t offs)
{
    for (int i = 0; i < _strlen(str); i++)
    {
        font_draw_char(
            f,
            dx + (i * f->char_width),
            dy + (sine_lut[((offs << 2) + (i << 2)) % 128] >> 1),
            str[i],
            color);
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color)
{
    for (int dy = 0; dy < h; dy++)
    {
        uint32_t *p = g_fb32 + ((dy + y) * SCREEN_WIDTH) + x;

        for (int dx = 0; dx < w; dx++)
        {
            *p++ = color;
        }
    }
}


void draw_colorbar(int y, int thickness, int offs, int dir)
{
    for (int dy = 0; dy < thickness; dy++)
    {
        uint32_t *p = g_fb32 + ((dy + y) * SCREEN_WIDTH);
        int x = (dir) ? (SCREEN_WIDTH - 1) : 0;
        while (1)
        {
            int or = 10 + (offs + x);
            int og = 20 + (offs + x);
            int ob = 30 + (offs + x);

            int a = (sine_lut[offs % 128] >> 1);

            *p++ = RGB24(
                CLAMP(sine_lut[or % 128] << 2, 0xc0) + a,
                CLAMP(sine_lut[og % 128] << 2, 0xc0) + a,
                CLAMP(sine_lut[ob % 128] << 2, 0xc0) + a);

            if (dir)
            {
                x--;
                if (x == -1)
                {
                    break;
                }
            }
            else
            {
                x++;
                if (x == SCREEN_WIDTH)
                {
                    break;
                }
            }
        }
    }
}


int render_writer(font_t *f)
{
    unsigned char txt[] =
        "\xde\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf"
        "\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdd\n"
        "\xde                            \xdd\n"
        "\xde KNOCK KNOCK! Who's there?? \xdd\n"
        "\xde                            \xdd\n"
        "\xde  ... IT'S YOUR BOY ~1BLASTY~  \xdd\n"
        "\xde                            \xdd\n"
        "\xde  Live from ~2PWN2OWN~ 2022 !  \xdd\n"
        "\xde                            \xdd\n"
        "\xde  Coming for your ~3" PRINTER_NAME "~   \xdd\n"
        "\xde  PRINTER using a chain of  \xdd\n"
        "\xde  silly bugs and tricks!    \xdd\n"
        "\xde                            \xdd\n"
        "\xde\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc"
        "\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdd";

    int xpos = WRITER_X;
    int ypos = WRITER_Y;
    int curlen = 0;

    int maxlen = (g_tick >> 2);

    uint32_t palette[] = {
        RGB24(0xff, 0xff, 0xff),
        RGB24(0xff, 0x00, 0x00),
        RGB24(0x00, 0xff, 0x00),
        RGB24(0x00, 0x00, 0xff),
    };

    int color = 0;

    for (int i = 0; i < _strlen((char *)txt); i++)
    {
        switch (txt[i])
        {
        case '\n':
            xpos = WRITER_X;
            ypos += f->char_height;
            break;

        case '~':
            // end of stylized string?
            if (color != 0)
            {
                color = 0;
            }
            else
            {
                color = txt[i + 1] - 0x30;
                i++;
            }
            break;

        default:
            font_draw_char(f, xpos, ypos, txt[i], palette[color]);
            xpos += f->char_width;
            curlen++;
            if (curlen == maxlen)
            {
                font_draw_char(f, xpos, ypos, 0x02, WRITER_COLOR);
                return 0;
            }
            break;
        }
    }

    return 0;
}

#ifdef WASM
void mainloop() {
#else
int mainloop() {
#endif
    if (platform_events_handle())
    {
#ifdef WASM
        emscripten_cancel_main_loop();
#elif defined(PLATFORM_LEXMARK_SDL) || defined(PLATFORM_CANON_SDL)
        return 1;
#endif
    }

    platform_video_render_start();

    // clear screen
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, RGB24(0, 0, 0));

    // draw copperbars
    draw_colorbar(4, 4, g_tick, 0);
    draw_colorbar(SCREEN_HEIGHT - 8, 4, g_tick, 1);

    uint32_t col = RGB24(
        (0xff - 128) + (sine_lut[g_tick % 128] << 1),
        (0xff - 128) + (sine_lut[(g_tick << 1) % 128] << 1),
        (0xff - 128) + (sine_lut[(g_tick << 2) % 128] << 1));

    font_draw_str_wave(font_big, BANNER_X, 20, "HACKED BY BLASTY!", col, g_tick);

    render_writer(font_small);

    memcpy(platform_get_framebuffer(), framebuffer, FRAMEBUFFER_SIZE);

    g_tick++;

    platform_video_render_end();

#if defined(PLATFORM_LEXMARK_SDL) || defined(PLATFORM_CANON_SDL)
    usleep(RENDER_DELAY_US);
#endif

#ifndef WASM
    return 0;
#endif
}

int main()
{
    if (platform_video_setup(SCREEN_WIDTH, SCREEN_HEIGHT) != EXIT_SUCCESS)
    {
        platform_print("failed to init video\n");
        return -1;
    }

    font_big = font_load_memory(font_big_bin, 17, 18);
    if (font_big == NULL)
    {
        platform_print("failed to load big font\n");
        return 0;
    }

    font_small = font_load_memory(font_small_bin, 10, 10);
    if (font_big == NULL)
    {
        platform_print("failed to load big font\n");
        return 0;
    }

    platform_print("loaded fonts\n");

    g_tick = 2;

    framebuffer = platform_malloc(FRAMEBUFFER_SIZE);
    g_fb32 = (uint32_t *)framebuffer;

#ifdef WASM
    emscripten_set_main_loop(mainloop, 0, 1);
#else
    while(1) {
        if (mainloop()) {
            break;
        }
    }
#endif

    platform_video_cleanup();

    return EXIT_SUCCESS;
}
