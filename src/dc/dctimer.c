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
 *      Dreamcast Timer driver.
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

#define TIMER_TO_MSEC(x)  ((long)((x) / 1193.181))
#define USEC_TO_TIMER(x)  ((long)((x) * (TIMERS_PER_SECOND / 1000000.)))

static int dc_time_init(void);
static void dc_time_exit(void);
static void dc_time_rest(long ms, AL_METHOD(void, callback, (void)));
static void dc_time_thread(void * p);

static int dc_time_thread_alive=0;


TIMER_DRIVER timer_dc = {
   TIMER_DC,		// int id;
   empty_string,	// char *name;
   empty_string,	// char *desc;
   "Dreamcast Timer",		// char *ascii_name;
   dc_time_init,	// AL_METHOD(int, init, (void));
   dc_time_exit,	// AL_METHOD(void, exit, (void));
   NULL, 		// AL_METHOD(int, install_int, (AL_METHOD(void, proc, (void)), long speed));
   NULL,		// AL_METHOD(void, remove_int, (AL_METHOD(void, proc, (void))));
   NULL,		// AL_METHOD(int, install_param_int, (AL_METHOD(void, proc, (void *param)), void *param, long speed));
   NULL,		// AL_METHOD(void, remove_param_int, (AL_METHOD(void, proc, (void *param)), void *param));
   NULL,		// AL_METHOD(int, can_simulate_retrace, (void));
   NULL,		// AL_METHOD(void, simulate_retrace, (int enable));
   dc_time_rest,	// AL_METHOD(void, rest, (long time, AL_METHOD(void, callback, (void))));
};

static int dc_time_init(void)
{
puts(__FILE__ ": dc_time_init"); fflush(stdout);
	dc_time_thread_alive=1;
	thd_create(dc_time_thread, NULL);
	irq_enable();
	return 0;
}

static void dc_time_exit(void)
{
puts(__FILE__ ": dc_time_exit"); fflush(stdout);
	dc_time_thread_alive=0;
	timer_spin_sleep(150);
}

static unsigned dc_time_get_ticks(void)
{
	uint32 s, ms;
	uint64 msec;
	timer_ms_gettime(&s, &ms);
	msec = (((uint64)s) * ((uint64)1000)) + ((uint64)ms);
	return (unsigned)msec;
}

static void dc_time_rest(long ms, AL_METHOD(void, callback, (void)))
{
// puts(__FILE__ ": dc_time_rest"); fflush(stdout);
	if (callback)
	{
		unsigned now=dc_time_get_ticks();
		unsigned toexit=now+ms;
		while(toexit>now)
		{
			(*callback)();
			thd_pass();
			now=dc_time_get_ticks();
		}
	}
	else
		timer_spin_sleep(ms);
}

static void dc_time_thread(void * p)
{
puts(__FILE__ ": dc_time_thread"); fflush(stdout);
	long interval = 0x8000;
	unsigned long long old=timer_us_gettime64();
	while(dc_time_thread_alive)
	{
		unsigned long long now;
		long tosleep=TIMER_TO_MSEC(interval);
// printf(__FILE__ "%i -> sleep %i\n",interval,tosleep);
		if (tosleep>1)
			thd_sleep(tosleep-1);
		poll_keyboard();
		poll_joystick();
		poll_mouse();
		now=timer_us_gettime64();
		interval = USEC_TO_TIMER(((double)(now-old)));
		old=now;
		interval = _handle_timer_tick(interval);
		thd_pass();
	}
puts(__FILE__ ": !dc_time_thread"); fflush(stdout);
}
