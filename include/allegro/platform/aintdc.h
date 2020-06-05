/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Internal header for Dreamcast Allegro library.
 *
 *      By Chui.
 *
 *      See readme.txt for copyright information.
 */

#ifndef AINTDC_H
#define AINTDC_H

#include <kos.h>
#include <pthread.h>

#include "allegro/internal/aintern.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEFAULT_RGB_R_SHIFT_15
#undef DEFAULT_RGB_R_SHIFT_15
#endif
#define DEFAULT_RGB_R_SHIFT_15 10

#ifdef DEFAULT_RGB_G_SHIFT_15
#undef DEFAULT_RGB_G_SHIFT_15
#endif
#define DEFAULT_RGB_G_SHIFT_15 5

#ifdef DEFAULT_RGB_B_SHIFT_15
#undef DEFAULT_RGB_B_SHIFT_15
#endif
#define DEFAULT_RGB_B_SHIFT_15 0

#ifdef DEFAULT_RGB_R_SHIFT_16
#undef DEFAULT_RGB_R_SHIFT_16
#endif
#define DEFAULT_RGB_R_SHIFT_16 11

#ifdef DEFAULT_RGB_G_SHIFT_16
#undef DEFAULT_RGB_G_SHIFT_16
#endif
#define DEFAULT_RGB_G_SHIFT_16 5

#ifdef DEFAULT_RGB_B_SHIFT_16
#undef DEFAULT_RGB_B_SHIFT_16
#endif
#define DEFAULT_RGB_B_SHIFT_16 0

#ifdef DEFAULT_RGB_R_SHIFT_24
#undef DEFAULT_RGB_R_SHIFT_24
#endif
#define DEFAULT_RGB_R_SHIFT_24 16

#ifdef DEFAULT_RGB_G_SHIFT_24
#undef DEFAULT_RGB_G_SHIFT_24
#endif
#define DEFAULT_RGB_G_SHIFT_24 8 

#ifdef DEFAULT_RGB_B_SHIFT_24
#undef DEFAULT_RGB_B_SHIFT_24
#endif
#define DEFAULT_RGB_B_SHIFT_24 0

#ifdef DEFAULT_RGB_R_SHIFT_32
#undef DEFAULT_RGB_R_SHIFT_32
#endif
#define DEFAULT_RGB_R_SHIFT_32 16

#ifdef DEFAULT_RGB_G_SHIFT_32
#undef DEFAULT_RGB_G_SHIFT_32
#endif
#define DEFAULT_RGB_G_SHIFT_32 8 

#ifdef DEFAULT_RGB_B_SHIFT_32
#undef DEFAULT_RGB_B_SHIFT_32
#endif
#define DEFAULT_RGB_B_SHIFT_32 0

#ifdef DEFAULT_RGB_A_SHIFT_32
#undef DEFAULT_RGB_A_SHIFT_32
#endif
#define DEFAULT_RGB_A_SHIFT_32 32

extern spinlock_t __dc_allegro_event_mutex;
extern BITMAP *__dc_allegro_gfx_bitmap;
extern GFX_MODE_LIST __dc_allegro_mode_list;

BITMAP *__dc_allegro_gfx_alloc_screen(void);
int __dc_allegro_gfx_get_disp_mode(int w, int h);
int __dc_allegro_gfx_get_pixel_mode(int color_depth);
void __dc_allegro_gfx_set(int color_depth, int width, int height, void *buf);
void __dc_allegro_gfx_8to16(void *dest, void *src, int n);
void __dc_allegro_gfx_set_palette(const struct RGB *p, int from, int to, int retracesync);
void __dc_allegro_gfx_release_dummy(void);
void __dc_allegro_mouse_set(int buttons, int dx, int dy, int dz);

#ifdef __cplusplus
}
#endif


#endif
