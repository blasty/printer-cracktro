#if defined(PLATFORM_CANON_SDL) || defined(PLATFORM_LEXMARK_SDL)

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Window *g_win;
SDL_Surface *g_surface;
uint32_t *g_pixels;

int platform_video_setup(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    g_win = SDL_CreateWindow(
        "SDL WINDOW",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (g_win == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    g_surface = SDL_GetWindowSurface(g_win);
    g_pixels = (uint32_t *)g_surface->pixels;

    return EXIT_SUCCESS;
}

int platform_video_render_start()
{
    SDL_LockSurface(g_surface);
    return 0;
}

int platform_video_render_end()
{
    SDL_UnlockSurface(g_surface);
    SDL_UpdateWindowSurface(g_win);
    return 0;
}

int platform_video_cleanup()
{
    SDL_DestroyWindow(g_win);
    SDL_Quit();
    return 0;
}

int platform_events_handle()
{
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            return 1;
        }
    }

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

void *platform_memcpy(void *dst, const void *src, uint32_t n)
{
    return memcpy(dst, src, n);
}

#endif
