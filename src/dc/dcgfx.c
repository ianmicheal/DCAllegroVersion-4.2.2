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
 *      GFX common functions for the Dreamcast video console.
 *
 *      By Chui.
 *
 *      See readme.txt for copyright information.
 */


#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/aintdc.h"
#include "60hz.h"

#ifndef ALLEGRO_DC
   #error something is wrong with the makefile
#endif


#define DC_ALLEGRO_BITMAP_SIZE (sizeof(BITMAP) + sizeof(char *) * 1024)
BITMAP *__dc_allegro_gfx_bitmap=NULL;

static unsigned short dc_allegro_palette[256];

static GFX_MODE __dc_allegro_mode[]= {
	{ 640, 480, 16 }, { 640, 480, 32 },
	{ 256, 256, 16 }, { 256, 256, 32 },
	{ 320, 200, 16 }, { 320, 200, 32 },
	{ 320, 240, 16 }, { 320, 240, 32 },
	{ 640, 400, 16 }, { 640, 400, 32 },
	{ 768, 480, 16 }, { 768, 480, 32 },
	{ 768, 576, 16 }, { 768, 576, 32 },
	{ 800, 600, 16 }, { 800, 600, 32 },
};

void __dc_allegro_gfx_release_dummy(void) { }

void (*__dc_allegro_gfx_release)(void)=__dc_allegro_gfx_release_dummy;


GFX_MODE_LIST __dc_allegro_mode_list={ 8, (GFX_MODE *)&__dc_allegro_mode };

BITMAP *__dc_allegro_gfx_alloc_screen(void)
{
	if (!__dc_allegro_gfx_bitmap)
		__dc_allegro_gfx_bitmap=malloc(DC_ALLEGRO_BITMAP_SIZE);
	memset(__dc_allegro_gfx_bitmap,0,DC_ALLEGRO_BITMAP_SIZE);
	return __dc_allegro_gfx_bitmap;
}


int __dc_allegro_gfx_get_disp_mode(int w, int h)
{
	int ret=0;
	if (!vid_check_cable())
	{
		__dc_allegro_is_60hz=1;
		if (w==320 && (h==240 || h==200)) ret=DM_320x240_VGA;
		else if (w==640 && (h==480 || h==400)) ret=DM_640x480_VGA;
		else if (w==800 && h==600) ret=DM_800x608_VGA;
		else if (w==768 && h==480) ret=DM_768x480_PAL_IL;
		else if (w==768 && h==576) ret=DM_768x576_PAL_IL;
		else if (w==256 && h==256) ret=DM_256x256_PAL_IL;
	}
	else
	if (flashrom_get_region()!=FLASHROM_REGION_US && !__dc_allegro_ask_60hz())
	{
		__dc_allegro_is_60hz=0;
		if (w==320 && (h==240 || h==200)) ret=DM_320x240_PAL;
		else if (w==640 && (h==480 || h==400)) ret=DM_640x480_PAL_IL;
		else if (w==800 && h==600) ret=DM_800x608;
		else if (w==768 && h==480) ret=DM_768x480_PAL_IL;
		else if (w==768 && h==576) ret=DM_768x576_PAL_IL;
		else if (w==256 && h==256) ret=DM_256x256_PAL_IL;
	}
	else
	{
		__dc_allegro_is_60hz=1;
		if (w==320 && (h==240 || h==200)) ret=DM_320x240;
		else if (w==640 && (h==480 || h==400)) ret=DM_640x480;
		else if (w==800 && h==600) ret=DM_800x608;
		else if (w==768 && h==480) ret=DM_768x480;
		else if (w==768 && h==576) ret=DM_768x576; //_PAL_IL;
		else if (w==256 && h==256) ret=DM_256x256; //_PAL_IL;
	}

	return ret;
}

int __dc_allegro_gfx_get_pixel_mode(int color_depth)
{
	switch(color_depth)
	{
		case 8:
			 __dc_allegro_gfx_set_palette(default_palette,0,255,0);
		 	 return PM_RGB565;
		case 15:
		         _rgb_r_shift_15=10;
		         _rgb_g_shift_15=5;
		         _rgb_b_shift_15=0;
			 return PM_RGB555;
		case 16:
		         _rgb_r_shift_16=11;
		         _rgb_g_shift_16=5;
		         _rgb_b_shift_16=0;
		 	 return PM_RGB565;
		case 24:
		case 32:
		         _rgb_r_shift_24=_rgb_r_shift_32=16;
		         _rgb_g_shift_24=_rgb_g_shift_32=8;
		         _rgb_b_shift_24=_rgb_b_shift_32=0;
			 _rgb_a_shift_32=32;
			 return PM_RGB888;
	}
	return 0;
}

static unsigned long __dc_allegro_gfx_select_line(BITMAP *bmp, int line)
{
	return (unsigned long)(bmp->line[line]);
}

void __dc_allegro_gfx_set(int color_depth, int width, int height, void *buf)
{
	unsigned i,nex,val=(unsigned)buf;
	switch(color_depth)
	{
		case 8:
			__dc_allegro_gfx_bitmap->vtable=&__linear_vtable8;
			nex=width;
			break;
		case 15:
			__dc_allegro_gfx_bitmap->vtable=&__linear_vtable15;
			nex=width*2;
			break;
		case 16:
			__dc_allegro_gfx_bitmap->vtable=&__linear_vtable16;
			nex=width*2;
			break;
		case 24:
			__dc_allegro_gfx_bitmap->vtable=&__linear_vtable24;
			nex=width*4;
		default:
			__dc_allegro_gfx_bitmap->vtable=&__linear_vtable32;
			nex=width*4;

	}
	__dc_allegro_gfx_bitmap->w=__dc_allegro_gfx_bitmap->cr=width;
	__dc_allegro_gfx_bitmap->h=__dc_allegro_gfx_bitmap->cb=height;
	__dc_allegro_gfx_bitmap->clip=-1;
	for(i=0;i<height;i++,val+=nex)
		__dc_allegro_gfx_bitmap->line[i]=(unsigned char *)val;
	__dc_allegro_gfx_bitmap->dat=buf;
	__dc_allegro_gfx_bitmap->write_bank=__dc_allegro_gfx_select_line;
	__dc_allegro_gfx_bitmap->write_bank=__dc_allegro_gfx_select_line;
	__dc_allegro_gfx_bitmap->read_bank=__dc_allegro_gfx_select_line;
}

#define SET_WITH_PALETTE(NN) \
{ \
	register unsigned o=((unsigned)dc_allegro_palette[*s++])&0x0000ffff; \
	o|=(((unsigned)dc_allegro_palette[*s++])<<16); \
	d[(NN)]=o; \
}

void __dc_allegro_gfx_8to16(void *dest, void *src, int n)
{
	unsigned int *d = (unsigned int *)(void *)
		(0xe0000000 | (((unsigned long)dest) & 0x03ffffe0));
	unsigned char *s = src;
	QACR0 = ((((unsigned int)dest)>>26)<<2)&0x1c;
	QACR1 = ((((unsigned int)dest)>>26)<<2)&0x1c;
	n>>=5;
	while(n--)
	{
		asm("pref @%0" : : "r" (s + 8));
		SET_WITH_PALETTE(0)
		SET_WITH_PALETTE(1)
		SET_WITH_PALETTE(2)
		SET_WITH_PALETTE(3)
		SET_WITH_PALETTE(4)
		SET_WITH_PALETTE(5)
		SET_WITH_PALETTE(6)
		SET_WITH_PALETTE(7)
		asm("pref @%0" : : "r" (d));
		d+=8;
	}
	d = (unsigned int *)0xe0000000;
	d[0] = d[8] = 0;
}

#undef SET_WITH_PALETTE

void __dc_allegro_gfx_set_palette(const struct RGB *p, int from, int to, int retracesync)
{
//puts(__FILE__ ": __dc_allegro_gfx_set_palette"); fflush(stdout);
	int i;
	if (to>255)
		to=255;
	if (from<0)
		from=0;
	for(i=from;i<=to;i++)
		dc_allegro_palette[i]=(((p[i].r>>1)<<10)&0xf800)|(((p[i].g)<<5)&0x07e0)|(((p[i].b)>>1)&0x001f);
}

