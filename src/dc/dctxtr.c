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
 *      Dreamcast Textured GFX driver.
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
static void dc_gfx_vsync(void);
static void dc_gfx_set_palette(const struct RGB *p, int from, int to, int retracesync);
static int dc_gfx_request_scroll(int x, int y);
static int dc_gfx_poll_scroll(void);
static int dc_gfx_request_video_bitmap(struct BITMAP *bitmap);
static GFX_MODE_LIST *dc_gfx_fetch_mode_list(void);
static void dc_gfx_release(void);

GFX_DRIVER gfx_dc_textured = {
   GFX_DC_DMA,           // int id;
   empty_string,                      // char *name;
   empty_string,                      // char *desc;
   "Dreamcast Textured GFX",             // char *ascii_name;
   dc_gfx_init,   // AL_METHOD(struct BITMAP *, init, (int w, int h, int v_w, int v_h, int color_depth));
   dc_gfx_exit,         // AL_METHOD(void, exit, (struct BITMAP *b));
   NULL, // dc_gfx_scroll,       // AL_METHOD(int, scroll, (int x, int y));
   vid_waitvbl,                      // AL_METHOD(void, vsync, (void));
   __dc_allegro_gfx_set_palette,  // AL_METHOD(void, set_palette, (struct RGB *p, int from, int to, int vsync));
   NULL,// dc_gfx_request_scroll,// AL_METHOD(int, request_scroll, (int x, int y));
   NULL, //dc_gfx_poll_scroll,  // AL_METHOD(int, poll_scroll, (void));
   NULL,                              // AL_METHOD(void, enable_triple_buffer, (void));
   NULL,                              // AL_METHOD(struct BITMAP *, create_video_bitmap, (int width, int height));
   NULL,                              // AL_METHOD(void, destroy_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, show_video_bitmap, (struct BITMAP *bitmap));
   NULL, // dc_gfx_request_video_bitmap,// AL_METHOD(int, request_video_bitmap, (struct BITMAP *bitmap));
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

static int dc_allegro_width=0;
static int dc_allegro_height=0;
static int dc_allegro_bpp=0;
static int dc_allegro_wtex=0;
static int dc_allegro_htex=0;
static pvr_ptr_t dc_allegro_memtex;
static unsigned short *dc_allegro_buftex;
static void *dc_allegro_memfreed;

static float dc_allegro_u1=0.3f;
static float dc_allegro_u2=0.3f;
static float dc_allegro_v1=0.9f;
static float dc_allegro_v2=0.6f;

void dc_allegro_set_window(int width, int height)
{
	dc_allegro_width=width;
	dc_allegro_height=height;
	dc_allegro_u1=0.3f*(1.0f/((float)dc_allegro_wtex));
	dc_allegro_v1=0.3f*(1.0f/((float)dc_allegro_wtex));
	dc_allegro_u2=(((float)dc_allegro_width)+0.7f)*(1.0f/((float)dc_allegro_wtex));
	dc_allegro_v2=(((float)dc_allegro_height)+0.7f)*(1.0f/((float)dc_allegro_htex));
}

static struct BITMAP *dc_gfx_init(int w, int h, int v_w, int v_h, int color_depth)
{
printf(__FILE__ ": dc_gfx_init(w=%i, h=%i, v_m=%i, v_h=%i, bpp=%i\n",w,h,v_w,v_h,color_depth);fflush(stdout);
	BITMAP *ret=__dc_allegro_gfx_alloc_screen();
	if (!ret || w>1024 || h>1024 || color_depth!=16)
		return NULL;
	for(dc_allegro_wtex=64;dc_allegro_wtex<w;dc_allegro_wtex<<=1);
	for(dc_allegro_htex=64;dc_allegro_htex<h;dc_allegro_htex<<=1);
	dc_allegro_width=w;
	dc_allegro_height=h;
	dc_allegro_bpp=color_depth;
	
	vid_set_mode(__dc_allegro_gfx_get_disp_mode(640,480),__dc_allegro_gfx_get_pixel_mode(16));
	pvr_init_defaults();
	pvr_dma_init();
	dc_allegro_memtex = pvr_mem_malloc(dc_allegro_wtex*dc_allegro_htex*2);
	dc_allegro_memfreed = calloc(64+(dc_allegro_wtex*dc_allegro_htex*(dc_allegro_bpp>>3)),1);
	dc_allegro_buftex = (unsigned short *)(((((unsigned)dc_allegro_memfreed)+32)/32)*32);
	__dc_allegro_gfx_set(color_depth, dc_allegro_wtex, dc_allegro_htex, dc_allegro_buftex);
	gfx_dc_textured.w=__dc_allegro_gfx_bitmap->cr=w;
	gfx_dc_textured.h=__dc_allegro_gfx_bitmap->cb=h;
	__dc_allegro_gfx_release=dc_gfx_release;
	dc_allegro_set_window(w,h);
	return ret;
}

static void dc_gfx_exit(struct BITMAP *b)
{
puts(__FILE__ ": dc_gfx_exit"); fflush(stdout);
	while (!pvr_dma_ready());
	if (dc_allegro_buftex)
		free(dc_allegro_memfreed);
	dc_allegro_buftex=NULL;
	if (dc_allegro_memtex)
	{
		pvr_mem_free(dc_allegro_memtex);
		pvr_mem_reset();
	}
	dc_allegro_memtex=NULL;
	pvr_shutdown();
	__dc_allegro_gfx_release=__dc_allegro_gfx_release_dummy;
}

#if 0
static int dc_gfx_scroll(int x, int y)
{
puts(__FILE__ ": dc_gfx_scroll"); fflush(stdout);
	return 0;
}

static int dc_gfx_request_scroll(int x, int y)
{
puts(__FILE__ ": dc_gfx_request_scroll"); fflush(stdout);
	return 0;
}

static int dc_gfx_poll_scroll(void)
{
puts(__FILE__ ": dc_gfx_poll_scroll"); fflush(stdout);
	return 0;
}

static int dc_gfx_request_video_bitmap(struct BITMAP *bitmap)
{
puts(__FILE__ ": dc_gfx_request_video_bitmap"); fflush(stdout);
	return 0;
}
#endif

static GFX_MODE_LIST *dc_gfx_fetch_mode_list(void)
{
puts(__FILE__ ": dc_gfx_fetch_mode_list"); fflush(stdout);
	return NULL;
}

static void dc_gfx_release(void)
{
#define DX1 0.0f
#define DY1 0.0f
#define DZ1 1.0f
#define DWI 640.0f
#define DHE 480.0f
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert;
	pvr_poly_cxt_t cxt;

	pvr_wait_ready();
	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_OP_POLY);
	dcache_flush_range((unsigned)dc_allegro_buftex,dc_allegro_wtex*dc_allegro_htex*2);
	while (!pvr_dma_ready());
	pvr_txr_load_dma(dc_allegro_buftex, dc_allegro_memtex, dc_allegro_wtex*dc_allegro_height*2,-1,NULL,0);
	pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565|PVR_TXRFMT_NONTWIDDLED,dc_allegro_wtex, dc_allegro_htex, dc_allegro_memtex, PVR_FILTER_NEAREST);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));
	vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
	vert.oargb = 0;
	vert.flags = PVR_CMD_VERTEX;
	vert.x = DX1; vert.y = DY1; vert.z = DZ1; vert.u = dc_allegro_u1; vert.v = dc_allegro_v1;
	pvr_prim(&vert, sizeof(vert));
	vert.x = DX1+DWI; vert.y = DY1; vert.z = DZ1; vert.u = dc_allegro_u2; vert.v = dc_allegro_v1;
	pvr_prim(&vert, sizeof(vert));
	vert.x = DX1; vert.y = DY1+DHE; vert.z = DZ1; vert.u = dc_allegro_u1; vert.v = dc_allegro_v2;
	pvr_prim(&vert, sizeof(vert));
	vert.x = DX1+DWI; vert.y = DY1+DHE; vert.z = DZ1; vert.u = dc_allegro_u2; vert.v = dc_allegro_v2;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));
	pvr_list_finish();
	pvr_scene_finish();
#undef DX1
#undef DY1
#undef DZ1
#undef DWI
#undef DHE
}
