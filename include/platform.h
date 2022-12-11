#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdint.h>

int platform_video_setup(int width, int height);
int platform_video_render_start();
int platform_video_render_end();
int platform_video_cleanup();
int platform_events_handle();
uint8_t *platform_get_framebuffer();
void *platform_malloc(uint32_t size);
void platform_print(char *);

void platform_print_u32(char *s, uint32_t v);

#endif