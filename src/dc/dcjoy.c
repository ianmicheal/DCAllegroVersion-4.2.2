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
 *      Dreamcast joystick driver.
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

//#define MAX_JOYS   8
#define MAX_JOYS   4
#define MAX_AXES        6
#define MAX_BUTTONS     8
#define MAX_HATS        2
#define JOYNAMELEN      8

static int dc_joy_init(void);
static void dc_joy_exit(void);
static int dc_joy_poll(void);

JOYSTICK_DRIVER joystick_dc =
{
   JOYSTICK_DC,          // int  id; 
   empty_string,
   empty_string,
   "Dremcast Joystick",      // AL_CONST char *ascii_name;
   dc_joy_init,          // AL_METHOD(int, init, (void));
   dc_joy_exit,          // AL_METHOD(void, exit, (void));
   dc_joy_poll,          // AL_METHOD(int, poll, (void));
   NULL,                 // AL_METHOD(int, save_data, (void));
   NULL,                 // AL_METHOD(int, load_data, (void));
   NULL,                 // AL_METHOD(AL_CONST char *, calibrate_name, (int n));
   NULL                  // AL_METHOD(int, calibrate, (int n));
};

static uint8 dc_allegro_joy_addr[MAX_JOYS];
static int dc_allegro_joy_buttons[MAX_JOYS];
const static int dc_buttons[] = {
	CONT_C, CONT_B, CONT_A, CONT_START, CONT_Z, CONT_Y, CONT_X, CONT_D
};
static int __dc_allegro_emulate_keyboard=1;
static int __dc_allegro_emulate_mouse=1;
static char dc_allegro_joy_key[MAX_JOYS][13] = {
  { '\0', '\0', '\0', '\n', '3', ' ', '\0',
	  '\t', '\0', '\0', '\0', '\0', '\0' },
  { '\0', 'q', 'e', 'z', '3', 'x', 'c',
	  '1', '2', 'w', 's', 'a', 'd' },
  { '\0', 'r', 'y', 'v', '3', 'b', 'n',
	  '4', '5', 't', 'g', 'f', 'h' },
  { '\0', 'u', 'o', 'm', '3', '`', '^',
	  '8', '9', 'i', 'k', 'j', 'l' }
};
static uint8 dc_allegro_joy_scancode[MAX_JOYS][13] = {
  { KEY_ESC, KEY_ALT, KEY_LCONTROL, KEY_ENTER, KEY_3, KEY_SPACE, KEY_LSHIFT,
	  KEY_TAB, KEY_BACKSPACE, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT },
  { KEY_ESC, KEY_Q, KEY_E, KEY_Z, KEY_3, KEY_X, KEY_C,
	  KEY_1, KEY_2, KEY_W, KEY_S, KEY_A, KEY_D },
  { KEY_ESC, KEY_R, KEY_Y, KEY_V, KEY_3, KEY_B, KEY_N,
	  KEY_4, KEY_5, KEY_T, KEY_G, KEY_F, KEY_H },
  { KEY_ESC, KEY_U, KEY_O, KEY_M, KEY_3, KEY_COMMA, KEY_CLOSEBRACE,
	  KEY_8, KEY_9, KEY_I, KEY_K, KEY_J, KEY_L }
};

void dc_allegro_emulate_keyboard(int value)
{
	__dc_allegro_emulate_keyboard=value;
}

void dc_allegro_emulate_mouse(int value)
{
	__dc_allegro_emulate_mouse=value;
}

static int dc_joy_init(void)
{
puts(__FILE__ ": dc_joy_init"); fflush(stdout);
	int p,u;
	num_joysticks=0;
	memset(&dc_allegro_joy_addr[0],sizeof(dc_allegro_joy_addr),0);
	memset(&dc_allegro_joy_buttons[0],sizeof(dc_allegro_joy_buttons),0);
	__dc_allegro_emulate_keyboard=(maple_first_kb()==0);
	__dc_allegro_emulate_mouse=(maple_first_mouse()==0);
	for(p=0;p<MAPLE_PORT_COUNT && num_joysticks<MAX_JOYS;p++) {
		for(u=0;u<MAPLE_UNIT_COUNT;u++) {
			if (maple_device_func(p,u)&MAPLE_FUNC_CONTROLLER) {
				dc_allegro_joy_addr[num_joysticks] = maple_addr(p,u);
				memset(&joy[0],sizeof(joy[0]),0);
				joy[num_joysticks].button[0].name="C";
				joy[num_joysticks].button[1].name="B";
				joy[num_joysticks].button[2].name="A";
				joy[num_joysticks].button[3].name="START";
				joy[num_joysticks].button[4].name="Z";
				joy[num_joysticks].button[5].name="Y";
				joy[num_joysticks].button[6].name="X";
				joy[num_joysticks].button[7].name="D";
				joy[num_joysticks].num_buttons=MAX_BUTTONS;
				joy[num_joysticks].num_sticks=1;
				joy[num_joysticks].stick[0].num_axis=MAX_AXES;
				joy[num_joysticks].stick[0].flags=JOYFLAG_ANALOGUE;
				dc_allegro_joy_buttons[num_joysticks]=65535;
				num_joysticks++;
			}
		}
	}
	return 0;
}

static void dc_joy_exit(void)
{
puts(__FILE__ ": dc_joy_exit"); fflush(stdout);
}

static void dc_joy_key_handle(int value, int n_joy, int i)
{
	int scancode=dc_allegro_joy_scancode[n_joy][i];
	if (value)
	{
		_handle_key_press(dc_allegro_joy_key[n_joy][i], scancode);
		key[scancode]=1;
	}
	else
	{
		_handle_key_release(scancode);
		key[scancode]=0;
	}
}

static int dc_joy_poll(void)
{
//puts(__FILE__ ": dc_joy_poll"); fflush(stdout);
	int n_joy;
	static int escaped=0;
	spinlock_lock(&__dc_allegro_event_mutex);
	for(n_joy=0;n_joy<num_joysticks;n_joy++)
	{
		int buttons, prev_buttons, changed, i, max;
		cont_cond_t cond;
		uint8 addr=dc_allegro_joy_addr[n_joy];
		if (cont_get_cond(addr,&cond)<0)
			continue;
		buttons = cond.buttons;
		prev_buttons = dc_allegro_joy_buttons[n_joy];
		changed = buttons^prev_buttons;

		if ((changed)&(CONT_DPAD_UP|CONT_DPAD_DOWN|CONT_DPAD_LEFT|CONT_DPAD_RIGHT))
		{
			joy[n_joy].stick[0].axis[0].d1=!(buttons&CONT_DPAD_LEFT);
			joy[n_joy].stick[0].axis[0].d2=!(buttons&CONT_DPAD_RIGHT);
			joy[n_joy].stick[0].axis[1].d1=!(buttons&CONT_DPAD_UP);
			joy[n_joy].stick[0].axis[1].d2=!(buttons&CONT_DPAD_DOWN);
		}
		if ((changed)&(CONT_DPAD2_UP|CONT_DPAD2_DOWN|CONT_DPAD2_LEFT|CONT_DPAD2_RIGHT))
		{
			joy[n_joy].stick[1].axis[0].d1=!(buttons&CONT_DPAD2_LEFT);
			joy[n_joy].stick[1].axis[0].d2=!(buttons&CONT_DPAD2_RIGHT);
			joy[n_joy].stick[1].axis[1].d1=!(buttons&CONT_DPAD2_UP);
			joy[n_joy].stick[1].axis[1].d2=!(buttons&CONT_DPAD2_DOWN);
		}
		for(i=0,max=0;i<sizeof(dc_buttons)/sizeof(dc_buttons[0]);i++)
	       	{
			if (changed & dc_buttons[i])
			{
				joy[n_joy].button[i].b=!(buttons&dc_buttons[i]);
				if (__dc_allegro_emulate_keyboard)
				{
					if (joy[n_joy].button[i].b)
						max++;
					dc_joy_key_handle(joy[n_joy].button[i].b,n_joy,i);
				}
			}
		}
		if (max>=4 && !escaped)
		{
			_handle_key_press(0,KEY_ESC);
			key[KEY_ESC]=1;
			escaped=1;
		}
		if (__dc_allegro_emulate_keyboard)
		{
			if (changed & CONT_DPAD_UP)
				dc_joy_key_handle(!(buttons&CONT_DPAD_UP),n_joy,9);
			if (changed & CONT_DPAD_DOWN)
				dc_joy_key_handle(!(buttons&CONT_DPAD_DOWN),n_joy,10);
			if (changed & CONT_DPAD_LEFT)
				dc_joy_key_handle(!(buttons&CONT_DPAD_LEFT),n_joy,11);
			if (changed & CONT_DPAD_RIGHT)
				dc_joy_key_handle(!(buttons&CONT_DPAD_RIGHT),n_joy,12);
		}

		joy[n_joy].stick[0].axis[0].pos=cond.joyx-128;
		joy[n_joy].stick[0].axis[1].pos=cond.joyy-128;
		if (__dc_allegro_emulate_mouse)
			__dc_allegro_mouse_set(((buttons^0xffff)&(CONT_X|CONT_Y))>>8,(cond.joyx-128)/8,(cond.joyy-128)/8,0);
		if (__dc_allegro_emulate_keyboard)
		{
			int back_rtrig=joy[n_joy].stick[0].axis[2].pos+128;
			int back_ltrig=joy[n_joy].stick[0].axis[3].pos+128;
			if (((back_rtrig) && (!cond.rtrig)) ||
			    ((!back_rtrig) && (cond.rtrig)))
				dc_joy_key_handle(cond.rtrig,n_joy,7);
			if (((back_ltrig) && (!cond.ltrig)) ||
			    ((!back_ltrig) && (cond.ltrig)))
				dc_joy_key_handle(cond.ltrig,n_joy,8);
			joy[n_joy].stick[0].axis[2].pos=cond.rtrig-128;
			joy[n_joy].stick[0].axis[3].pos=cond.ltrig-128;
		}
		else
		{
			joy[n_joy].stick[0].axis[2].pos=cond.rtrig-128;
			joy[n_joy].stick[0].axis[3].pos=cond.ltrig-128;
		}
		joy[n_joy].stick[0].axis[4].pos=cond.joy2x-128;
		joy[n_joy].stick[0].axis[5].pos=cond.joy2y-128;
		dc_allegro_joy_buttons[n_joy]=buttons;
	}
	switch(escaped)
	{
		case 0: break;
		case 8: _handle_key_release(KEY_ESC); key[KEY_ESC]=0; break;
		case 16: escaped=0; break;
		default: escaped++;
	}
	spinlock_unlock(&__dc_allegro_event_mutex);
	return 0;
}
