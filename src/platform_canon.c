#ifdef PLATFORM_CANON

#include <stdint.h>

#define calloc FUNC_CALLOC
#define UART_Print FUNC_UART_PRINT
#define UART_Print_u32 FUNC_UART_PRINT_U32
#define task_sleep FUNC_TASK_SLEEP
#define task_kill FUNC_TASK_KILL
#define ui_blinktest FUNC_UI_BLINKTEST
#define task_get_info FUNC_TASK_GET_INFO

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

#define FB_BASE 0x40900000
#define FB_WIDTH (SCREEN_WIDTH * 2)
#define FB_HEIGHT (SCREEN_HEIGHT * 2)

static uint32_t *g_pixels = (uint32_t *)1;
static uint8_t *g_pixels_native = (uint8_t *)1;

// these symbols are defined in the linker script
uint8_t *calloc(uint32_t, uint32_t);
void UART_Print(char *);
void UART_Print_u32(char *, uint32_t);
void power_screen(void);
void disableSleep(uint32_t);
void scsm_doSleep1r();
int task_get_info(uint32_t, uint32_t, void *);
int task_kill(uint32_t);
void ui_setled(uint32_t mask, uint32_t mode);
void culo();
void s3();
void setPowerRequest(uint32_t);
void printerPowerSupply(uint32_t);
void blah(uint32_t);
void ui_blinktest();
uint32_t task_sleep(uint32_t *);

void *platform_malloc(uint32_t size)
{
    return calloc(1, size);
}

void platform_print(char *s)
{
    UART_Print(s);
}

void platform_print_u32(char *s, uint32_t v)
{
    UART_Print_u32(s, v);
}

void *memcpy(void *dst, const void *src, uint32_t n)
{
    uint8_t *dst8 = (uint8_t *)dst;
    uint8_t *src8 = (uint8_t *)src;
    while (n--)
    {
        *dst8++ = *src8++;
    }
    return dst;
}

void *memset(void *dst, int c, uint32_t len)
{
    uint8_t *dst8 = (uint8_t *)dst;
    for (int i = 0; i < len; i++)
    {
        *dst8++ = (c & 0xff);
    }
    return dst;
}

int strncmp(char *a, char *b, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (a[i] != b[i])
        {
            return 1;
        }
    }
    return 0;
}

void delay_ms(uint32_t ms)
{
    uint32_t sv[2] = {
        0, ms * 1000000};
    task_sleep(sv);
}

int platform_video_setup(int width, int height)
{
    uint8_t task_info[0x80];

    // TODO: turn on screen properly.
    /* TODO: the mess below doesn't actually work.
     * TODO: instead to hack around the problem we'll invoke some "LCD test"
     * TODO: routine that deals with all the wake-from-standby stuff needed lol

    disableSleep(30);
    s3();
    setPowerRequest(3);
    printerPowerSupply(1);
    scsm_doSleep1r();
    disableSleep(16);
    blah(0);
    culo();
    power_screen();*/

    UART_Print("start ui test!\r\n");
    ui_blinktest();
    for (int i = 0; i < 8; i++)
    {
        UART_Print("tick..\r\n");
        delay_ms(1000);
    }
    UART_Print("lets kill shit!\r\n");

    // find the `ui_device` RTOS task and kill it so it doesn't fuck with
    // our framebuffer
    for (int i = 0; i < 999; i++)
    {
        memset(task_info, 0, sizeof(task_info));
        if (!(task_get_info(0, i, task_info) >= 0))
        {
            continue;
        }
        char *task_name = (char *)(*(uint32_t *)(task_info + 0x14));
        if (task_name == 0)
        {
            continue;
        }
        if (strncmp(task_name, "ui_device", 9) == 0)
        {
            UART_Print_u32("found pid: %x\r\n", i);
            task_kill(i);
        }
    }

    g_pixels = (uint32_t *)platform_malloc(
        SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t)
    );

    if (g_pixels == 0)
    {
        return -1;
    }

    g_pixels_native = (uint8_t *)FB_BASE;

    return 0;
}

int platform_video_render_start()
{
    return 0;
}

int platform_video_render_end()
{
    // our actual framebuffer is twice the size of the internal one. upscale
    // both dimensions by x2
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        uint32_t *row = g_pixels + (y * SCREEN_WIDTH);
        uint8_t *row_native_a = g_pixels_native + (((y * 2) + 0) * (FB_WIDTH * 3));
        uint8_t *row_native_b = g_pixels_native + (((y * 2) + 1) * (FB_WIDTH * 3));

        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            uint8_t r, g, b;
            r = row[x] & 0xff;
            g = (row[x] >> 8) & 0xff;
            b = (row[x] >> 16) & 0xff;
            row_native_a[(x * 6) + 0] = row_native_b[(x * 6) + 0] = r;
            row_native_a[(x * 6) + 1] = row_native_b[(x * 6) + 1] = g;
            row_native_a[(x * 6) + 2] = row_native_b[(x * 6) + 2] = b;

            row_native_a[(x * 6) + 3] = row_native_b[(x * 6) + 3] = r;
            row_native_a[(x * 6) + 4] = row_native_b[(x * 6) + 4] = g;
            row_native_a[(x * 6) + 5] = row_native_b[(x * 6) + 5] = b;
        }
    }

    return 0;
}

int platform_video_cleanup()
{
    return 0;
}

int platform_events_handle()
{
    return 0;
}

uint8_t *platform_get_framebuffer()
{
    return (uint8_t *)g_pixels;
}

#endif /* PLATFORM_CANON */
