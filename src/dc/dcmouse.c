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
 *      Dreamcast mouse driver.
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


static int dc_mouse_init(void);
static void dc_mouse_exit(void);
static void dc_mouse_position(int, int);
static void dc_mouse_set_range(int, int, int, int);
static void dc_mouse_get_mickeys(int *, int *);
static void dc_mouse_poll(void);

MOUSE_DRIVER mouse_dc =
{
   MOUSE_DC,
   empty_string,
   empty_string,
   "Dreamcast Mouse",
   dc_mouse_init,
   dc_mouse_exit,
   dc_mouse_poll,       // AL_METHOD(void, poll, (void));
   NULL,       // AL_METHOD(void, timer_poll, (void));
   dc_mouse_position,
   dc_mouse_set_range,
   NULL,       // AL_METHOD(void, set_speed, (int xspeed, int yspeed));
   dc_mouse_get_mickeys,
   NULL,       // AL_METHOD(int,  analyse_data, (AL_CONST char *buffer, int size));
   NULL,       // AL_METHOD(void,  enable_hardware_cursor, (AL_CONST int mode));
   NULL        // AL_METHOD(int,  select_system_cursor, (AL_CONST int cursor));
};

static int dc_mouse_min_x = 0;
static int dc_mouse_min_y = 0;
static int dc_mouse_max_x = 319;
static int dc_mouse_max_y = 199;
static int dc_mouse_x = 0;
static int dc_mouse_y = 0;
static int dc_mouse_addr = 0;

void __dc_allegro_mouse_set(int buttons, int dx, int dy, int dz)
{
	_mouse_b=(((buttons&0x01)<<4)|((buttons&0x04)>>2)|((buttons&0x08)>>1)|((buttons&0x10)>>1)|(buttons&0xe2));
	_mouse_x+=dx;
	_mouse_y+=dy;
	_mouse_z+=dz;

	dc_mouse_x+=dx;
	dc_mouse_y+=dy;

	if ((_mouse_x < dc_mouse_min_x) || (_mouse_x > dc_mouse_max_x)
		|| (_mouse_y < dc_mouse_min_y) || (_mouse_y > dc_mouse_max_y))
	{
		_mouse_x = MID(dc_mouse_min_x, _mouse_x, dc_mouse_max_x);
		_mouse_y = MID(dc_mouse_min_y, _mouse_y, dc_mouse_max_y);
	}

	_handle_mouse_input();
}

void dc_mouse_poll(void)
{
//puts(__FILE__ ": dc_mouse_poll"); fflush(stdout);
	
	if (dc_mouse_addr)
	{
		static int prev_buttons=0;
		mouse_cond_t cond;

		spinlock_lock(&__dc_allegro_event_mutex);

		if (!((mouse_get_cond(dc_mouse_addr, &cond)<0)||(prev_buttons==cond.buttons && !cond.dx && !cond.dy && !cond.dz)))
		{
			prev_buttons=cond.buttons;
			__dc_allegro_mouse_set(cond.buttons^0xff,cond.dx,cond.dy,cond.dz);
		}
		spinlock_unlock(&__dc_allegro_event_mutex);
	}
}



static int dc_mouse_init(void)
{
puts(__FILE__ ": dc_mouse_init"); fflush(stdout);
	dc_mouse_addr = maple_first_mouse();
	if (dc_mouse_addr)
		return 3;
	return 0;
}



static void dc_mouse_exit(void)
{
puts(__FILE__ ": dc_mouse_exit"); fflush(stdout);
	dc_mouse_addr = 0;
}


static void dc_mouse_position(int x, int y)
{
//puts(__FILE__ ": dc_mouse_position"); fflush(stdout);
	spinlock_lock(&__dc_allegro_event_mutex);
	_mouse_x = x;
	_mouse_y = y;
	dc_mouse_x = dc_mouse_y = 0;
	spinlock_unlock(&__dc_allegro_event_mutex);
}



static void dc_mouse_set_range(int x1, int y1, int x2, int y2)
{
//puts(__FILE__ ": dc_mouse_set_range"); fflush(stdout);
	spinlock_lock(&__dc_allegro_event_mutex);
	dc_mouse_min_x = x1;
	dc_mouse_min_y = y1;
	dc_mouse_max_x = x2;
	dc_mouse_max_y = y2;
	_mouse_x = MID(dc_mouse_min_x, _mouse_x, dc_mouse_max_x);
	_mouse_y = MID(dc_mouse_min_y, _mouse_y, dc_mouse_max_y);
	spinlock_unlock(&__dc_allegro_event_mutex);
}



static void dc_mouse_get_mickeys(int *mickeyx, int *mickeyy)
{
//puts(__FILE__ ": dc_mouse_get_mickeys"); fflush(stdout);
	*mickeyx = dc_mouse_x;
	*mickeyy = dc_mouse_y;
	dc_mouse_x = dc_mouse_y = 0;
}
