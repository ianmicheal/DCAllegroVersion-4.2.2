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
 *      Dreamcast Direct GFX driver.
 *
 *      By Chui.
 *
 *      See readme.txt for copyright information.
 */


#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/aintdc.h"

#ifndef ALLEGRO_DC
#error Something is wrong with the makefile
#endif

static struct BITMAP *dc_gfx_init(int w, int h, int v_w, int v_h, int color_depth);
static void dc_gfx_exit(struct BITMAP *b);
static int dc_gfx_scroll(int x, int y);
static int dc_gfx_request_scroll(int x, int y);
static int dc_gfx_poll_scroll(void);
static int dc_gfx_request_video_bitmap(struct BITMAP *bitmap);
static GFX_MODE_LIST *dc_gfx_fetch_mode_list(void);

GFX_DRIVER gfx_dc_direct = {
   GFX_DC_DIRECT,           // int id;
   empty_string,                      // char *name;
   empty_string,                      // char *desc;
   "Dreamcast Direct GFX",             // char *ascii_name;
   dc_gfx_init,   // AL_METHOD(struct BITMAP *, init, (int w, int h, int v_w, int v_h, int color_depth));
   dc_gfx_exit,         // AL_METHOD(void, exit, (struct BITMAP *b));
  /* dc_gfx_scroll */ NULL,       // AL_METHOD(int, scroll, (int x, int y));
   vid_waitvbl,         // AL_METHOD(void, vsync, (void));
   __dc_allegro_gfx_set_palette,  // AL_METHOD(void, set_palette, (struct RGB *p, int from, int to, int vsync));
  /* dc_gfx_request_scroll */ NULL,// AL_METHOD(int, request_scroll, (int x, int y));
  /* dc_gfx_poll_scroll */ NULL,  // AL_METHOD(int, poll_scroll, (void));
   NULL,                              // AL_METHOD(void, enable_triple_buffer, (void));
   NULL,                              // AL_METHOD(struct BITMAP *, create_video_bitmap, (int width, int height));
   NULL,                              // AL_METHOD(void, destroy_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, show_video_bitmap, (struct BITMAP *bitmap));
  /* dc_gfx_request_video_bitmap */ NULL,// AL_METHOD(int, request_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(struct BITMAP *, create_system_bitmap, (int width, int height));
   NULL,                              // AL_METHOD(void, destroy_system_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, set_mouse_sprite, (struct BITMAP *sprite, int xfocus, int yfocus));
   NULL,                              // AL_METHOD(int, show_mouse, (struct BITMAP *bmp, int x, int y));
   NULL,                              // AL_METHOD(void, hide_mouse, (void));
   NULL,                              // AL_METHOD(void, move_mouse, (int x, int y));
   NULL,                              // AL_METHOD(void, drawing_mode, (void));
   NULL,                              // AL_METHOD(void, save_state, (void));
   NULL,                              // AL_METHOD(void, restore_state, (void));
   NULL,                              // AL_METHOD(void, set_blender_mode, (int mode, int r, int g, int b, int a));
   dc_gfx_fetch_mode_list,// AL_METHOD(int, fetch_mode_list, (void));
   0, 0,                              // int w, h;  /* physical (not virtual!) screen size */
   TRUE,                              // int linear;  /* true if video memory is linear */
   0,                                 // long bank_size;  /* bank size, in bytes */
   0,                                 // long bank_gran;  /* bank granularity, in bytes */
   0,                                 // long vid_mem;  /* video memory size, in bytes */
   0,                                 // long vid_phys_base;  /* physical address of video memory */
   FALSE                              // int windowed;  /* true if driver runs windowed */
};

static struct BITMAP *dc_gfx_init(int w, int h, int v_w, int v_h, int color_depth)
{
printf(__FILE__ ": dc_gfx_init(w=%i, h=%i, v_m=%i, v_h=%i, bpp=%i\n",w,h,v_w,v_h,color_depth);fflush(stdout);
	BITMAP *ret=__dc_allegro_gfx_alloc_screen();
	int disp_mode=__dc_allegro_gfx_get_disp_mode(w,h);
	int pixel_mode=__dc_allegro_gfx_get_pixel_mode(color_depth);
	if (ret && color_depth>=15 && disp_mode && pixel_mode)
	{
		void *fbuf;
		vid_set_mode(disp_mode,pixel_mode);
		gfx_dc_direct.w=w; gfx_dc_direct.h=h;
		if (h==200)
		{
			memset(vram_l,w*240*(color_depth<=16?2:4),0);
			fbuf=(void *)(((unsigned)vram_l)+(20*w*(color_depth<=16?2:4)));
		}
		else if (h==400)
		{
			memset(vram_l,w*480*(color_depth<=16?2:4),0);
			fbuf=(void *)(((unsigned)vram_l)+(40*w*(color_depth<=16?2:4)));
		}
		else
		{
			memset(vram_l,w*h*(color_depth<=16?2:4),0);
			fbuf=vram_l;
		}
		__dc_allegro_gfx_set(color_depth, w, h, fbuf);
	}
	else
		ret=NULL;
	return ret;
}

static void dc_gfx_exit(struct BITMAP *b)
{
puts(__FILE__ ": dc_gfx_exit"); fflush(stdout);
	b->dat=NULL;
}

#if 0
static int dc_gfx_scroll(int x, int y)
{
//puts(__FILE__ ": dc_gfx_scroll"); fflush(stdout);
	return 0;
}

static int dc_gfx_request_scroll(int x, int y)
{
//puts(__FILE__ ": dc_gfx_request_scroll"); fflush(stdout);
	return 0;
}

static int dc_gfx_poll_scroll(void)
{
//puts(__FILE__ ": dc_gfx_poll_scroll"); fflush(stdout);
	return 0;
}

static int dc_gfx_request_video_bitmap(struct BITMAP *bitmap)
{
//puts(__FILE__ ": dc_gfx_request_video_bitmap"); fflush(stdout);
	return 0;
}
#endif

static GFX_MODE_LIST *dc_gfx_fetch_mode_list(void)
{
//puts(__FILE__ ": dc_gfx_fetch_mode_list"); fflush(stdout);
	return (&__dc_allegro_mode_list);
}

