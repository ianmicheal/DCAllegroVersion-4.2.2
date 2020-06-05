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
 *      System driver for the Dreamcast video console.
 *
 *      By Chui.
 *
 *      See readme.txt for copyright information.
 */


#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/aintdc.h"

#ifndef ALLEGRO_DC
   #error something is wrong with the makefile
#endif

#include <kos.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static int dc_sys_init(void);
static void dc_sys_exit(void);
static void dc_sys_message(AL_CONST char *);
static int dc_sys_set_display_switch_mode(int mode);
static void dc_sys_get_gfx_safe_mode(int *driver, struct GFX_MODE *mode);
static void dc_sys_yield_timeslice(void);
static int dc_sys_desktop_color_depth(void);
static int dc_sys_get_desktop_resolution(int *width, int *height);
static void *dc_create_mutex(void);
static void dc_destroy_mutex(void *handle);
static void dc_lock_mutex(void *handle);
static void dc_unlock_mutex(void *handle);


SYSTEM_DRIVER system_dc =
{
   SYSTEM_DC,
   empty_string,
   empty_string,
   "Dreamcast Platform",
   dc_sys_init,
   dc_sys_exit,
   NULL, /* AL_METHOD(void, get_executable_name, (char *output, int size)); */
   NULL, /* AL_METHOD(int, find_resource, (char *dest, AL_CONST char *resource, int size)); */
   NULL, /* AL_METHOD(void, set_window_title, (AL_CONST char *name)); */
   NULL, /* AL_METHOD(int, set_close_button_callback, (AL_METHOD(void, proc, (void)))); */
   dc_sys_message,
   NULL,  /* AL_METHOD(void, assert, (AL_CONST char *msg)); */
   NULL,  /* AL_METHOD(void, save_console_state, (void)); */
   NULL,  /* AL_METHOD(void, restore_console_state, (void)); */
   NULL,  /* AL_METHOD(struct BITMAP *, create_bitmap, (int color_depth, int width, int height)); */
   NULL,  /* AL_METHOD(void, created_bitmap, (struct BITMAP *bmp)); */
   NULL,  /* AL_METHOD(struct BITMAP *, create_sub_bitmap, (struct BITMAP *parent, int x, int y, int width, int height)); */
   NULL,  /* AL_METHOD(void, created_sub_bitmap, (struct BITMAP *bmp, struct BITMAP *parent)); */
   NULL,  /* AL_METHOD(int, destroy_bitmap, (struct BITMAP *bitmap)); */
   NULL,  /* AL_METHOD(void, read_hardware_palette, (void)); */
   NULL,  /* AL_METHOD(void, set_palette_range, (AL_CONST struct RGB *p, int from, int to, int retracesync)); */
   NULL,  /* AL_METHOD(struct GFX_VTABLE *, get_vtable, (int color_depth)); */
   dc_sys_set_display_switch_mode,
   NULL,  /* AL_METHOD(void, display_switch_lock, (int lock, int foreground)); */
   dc_sys_desktop_color_depth,
   dc_sys_get_desktop_resolution,
   dc_sys_get_gfx_safe_mode,
   dc_sys_yield_timeslice,
   dc_create_mutex,
   dc_destroy_mutex,
   dc_lock_mutex,
   dc_unlock_mutex,
   NULL,  /* AL_METHOD(_DRIVER_INFO *, gfx_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, digi_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, midi_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, keyboard_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, mouse_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, joystick_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, timer_drivers, (void)); */
};


spinlock_t __dc_allegro_event_mutex;


static void dc_event_handler(int threaded)
{
puts(__FILE__ ": dc_event_handler"); fflush(stdout);
	spinlock_lock(&__dc_allegro_event_mutex);

	spinlock_unlock(&__dc_allegro_event_mutex);
}


static int dc_sys_init(void)
{
puts(__FILE__ ": dc_sys_init"); fflush(stdout);
	spinlock_init(&__dc_allegro_event_mutex);
	return 0;
}



static void dc_sys_exit(void)
{
puts(__FILE__ ": dc_sys_exit"); fflush(stdout);
}


static void dc_sys_message(AL_CONST char *msg)
{
puts(__FILE__ ": dc_sys_message"); fflush(stdout);
	puts(msg);
}


static int dc_sys_set_display_switch_mode (int mode)
{
puts(__FILE__ ": dc_sys_set_display_switch_mode"); fflush(stdout);
	return 0;
}


static int dc_sys_desktop_color_depth(void)
{
puts(__FILE__ ": dc_sys_desktop_color_depth"); fflush(stdout);
	return 16;
}



static int dc_sys_get_desktop_resolution(int *width, int *height)
{
puts(__FILE__ ": dc_sys_get_desktop_resolution"); fflush(stdout);
	*width = 640;
	*height = 480;
	return 0;
}



static void dc_sys_get_gfx_safe_mode(int *driver, struct GFX_MODE *mode)
{
puts(__FILE__ ": dc_sys_get_gfx_safe_mode"); fflush(stdout);
	*driver = GFX_DC_DMA;
	mode->width = 640;
	mode->height = 480;
	mode->bpp = 16;
}


static void dc_sys_yield_timeslice(void)
{
//puts(__FILE__ ": dc_sys_yield_timeslice"); fflush(stdout);
	thd_pass();
//	timer_spin_sleep(0);
}

struct dc_mutex {
	int recursive;
	kthread_t *owner;
	spinlock_t mutex;
};

static void *dc_create_mutex(void)
{
puts(__FILE__ ": dc_create_mutex"); fflush(stdout);
	struct dc_mutex *mutex;
	mutex = (struct dc_mutex *)malloc(sizeof(*mutex));
	if ( mutex ) {
		spinlock_init(&mutex->mutex);
		mutex->recursive = 0;
		mutex->owner = NULL;
	}
	return (void *)mutex;
}


static void dc_destroy_mutex(void *handle)
{
puts(__FILE__ ": dc_destroy_mutex"); fflush(stdout);
	if (handle)
		free(handle);
}


static void dc_lock_mutex(void *handle)
{
puts(__FILE__ ": dc_lock_mutex"); fflush(stdout);
	struct dc_mutex *mutex=(struct dc_mutex *)handle;
	if (mutex)
	{
		kthread_t *this_thread=thd_get_current();
		if ( mutex->owner == this_thread )
			mutex->recursive++;
		else
		{
			spinlock_lock(&mutex->mutex);
			mutex->owner = this_thread;
			mutex->recursive = 0;
		}
	}
}


static void dc_unlock_mutex(void *handle)
{
puts(__FILE__ ": dc_unlock_mutex"); fflush(stdout);
	struct dc_mutex *mutex=(struct dc_mutex *)handle;
	if (mutex)
	{
		kthread_t *this_thread=thd_get_current();
		if ( mutex->owner == this_thread )
		{
			if ( mutex->recursive )
				mutex->recursive--;
			else
			{
				spinlock_unlock(&mutex->mutex);
				mutex->owner = NULL;
			}
		}
	}
}

int dup(int oldfd)
{
	return (int)fs_dup((file_t)oldfd);
}

uid_t geteuid(void)
{
	return 0;
}

gid_t getegid(void)
{
	return 0;
}

