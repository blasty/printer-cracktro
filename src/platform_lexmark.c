#ifdef PLATFORM_LEXMARK

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

uint32_t *g_pixels;
uint16_t *g_pixels_native;

int g_fd;

char *kill_procs[] = {
    "ui-qt-touch",
    "printcryption2",
    "RemoteNetapps",
    "dcs-web-services",
    "mirrord",
    "supply-authenticate",
    "snmp-web-services",
    "pm-detect-input",
    "timemgrd",
    "oosw-fill-diag",
    "svcerr-system-info",
    NULL,
};

int platform_video_setup(int width, int height)
{
    setpriority(PRIO_PROCESS, 0, -19);

    // turn on screen
    system("echo 0 > /sys/class/graphics/fb0/blank");
    system("kill -STOP $(pidof firewall_app)");

    // kill some guys who fuck with our needed cpu time
    int i = 0;
    char cmd[128];
    while (kill_procs[i] != NULL)
    {
        sprintf(cmd, "killall -9 %s", kill_procs[i]);
        system(cmd);
        i++;
    }

    g_fd = open("/dev/fb0", O_RDWR);
    if (g_fd < 0)
    {
        return EXIT_FAILURE;
    }

    g_pixels = (uint32_t *)malloc(
        SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));

    if (g_pixels == NULL)
    {
        return EXIT_FAILURE;
    }

    g_pixels_native = (uint16_t *)malloc(
        SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));

    if (g_pixels_native == NULL)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int platform_video_render_start()
{
    return 0;
}

#define RGB24_TO_RGB55(v) (((((v) >> 16) >> 3) << 11) | (((((v) >> 8) & 0xff) >> 3) << 6) | ((((v)&0xff) >> 3) << 1))

int platform_video_render_end()
{
    // do conversion and copy to actual framebuffer
    // our source buffer is a linear 320x240x4 buffer
    // our dest buffer is 240x320x2 .. upside down
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            g_pixels_native[(x * SCREEN_HEIGHT) + ((SCREEN_HEIGHT - 1) - y)] =
                RGB24_TO_RGB55(g_pixels[(y * SCREEN_WIDTH) + x]);
        }
    }
    lseek(g_fd, 0, SEEK_SET);
    write(g_fd, g_pixels_native, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));

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

void *platform_malloc(uint32_t size)
{
    return malloc(size);
}

void platform_print(char *s)
{
    printf("%s", s);
}

void platform_print_u32(char *s, uint32_t v)
{
    printf(s, v);
}

#endif /* PLATFORM_LEXMARK */
