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
 *      Dreamcast keyboard driver.
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


static int dc_keyboard_init(void);
static void dc_keyboard_exit(void);
static void dc_keyboard_poll(void);
static void dc_keyboard_wait_for_input(void);
static void dc_keyboard_stop_waiting_for_input(void);


KEYBOARD_DRIVER keyboard_dc =
{
   KEYBOARD_DC,
   empty_string,
   empty_string,
   "Dreamcast Keyboard",
   TRUE,
   dc_keyboard_init,
   dc_keyboard_exit,
   dc_keyboard_poll,
   NULL,   // AL_METHOD(void, set_leds, (int leds));
   NULL,   // AL_METHOD(void, set_rate, (int delay, int rate));
   NULL,   // dc_keyboard_wait_for_input,
   NULL,   // dc_keyboard_stop_waiting_for_input,
   _pckey_scancode_to_ascii,
   _pckey_scancode_to_name
};

static kbd_state_t dc_keyboard_old_state;

const static unsigned char dc_keyboard_key[]= {
	0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
	'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
	'u', 'v', 'w', 'x', 'y', 'z',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\n', 0x1b, 0x8, '\t', ' ', '-', '=', '{', 
	'}', '\\' , 0, ';', '\'',
	'~', ',', '`', '/', 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, '/', '*', '-', '+', '\n', 
	'1', '2', '3', '4', '5', '6',
	'7', '8', '9', '0', 0, 0 
};

const static unsigned char dc_keyboard_code[]= {
	0, 0, 0, 0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I ,
	KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
	KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
	KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
	KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB, KEY_SPACE, KEY_MINUS, KEY_EQUALS, KEY_OPENBRACE, 
	KEY_CLOSEBRACE, KEY_BACKSLASH , 0, KEY_SEMICOLON, KEY_QUOTE,
	'~', KEY_COMMA, KEY_TILDE, KEY_SLASH, KEY_CAPSLOCK, 
	KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
	KEY_PRTSCR, KEY_SCRLOCK, KEY_PAUSE, KEY_INSERT, KEY_HOME, KEY_PGUP, KEY_DEL, KEY_END, KEY_PGDN, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP,
	KEY_NUMLOCK, KEY_SLASH_PAD, KEY_ASTERISK, KEY_MINUS_PAD, KEY_PLUS_PAD, KEY_ENTER_PAD, 
	KEY_1_PAD, KEY_2_PAD, KEY_3_PAD, KEY_4_PAD, KEY_5_PAD, KEY_6_PAD,
	KEY_7_PAD, KEY_8_PAD, KEY_9_PAD, KEY_0_PAD, KEY_DEL_PAD, 0 
};


const static unsigned char dc_keyboard_shift_code[] = {
	KEY_LCONTROL,KEY_LSHIFT,KEY_ALT,0 /* S1 */,
	KEY_RCONTROL,KEY_RSHIFT,KEY_ALTGR,0 /* S2 */,
};

const static unsigned char dc_keyboard_shift[] = {
	KB_CTRL_FLAG,KB_SHIFT_FLAG,KB_ALT_FLAG,0,
	KB_CTRL_FLAG,KB_SHIFT_FLAG,KB_ALT_FLAG,0,
};


static void dc_keyboard_handler_key(int pressed, int code)
{
	int scancode=dc_keyboard_code[code];
	if (pressed)
	{
	 	_handle_key_press(dc_keyboard_key[code], scancode);
		key[scancode]=1;
	}
	else
	{
	 	_handle_key_release(scancode);
		key[scancode]=0;
	}
}

static void dc_keyboard_handler_shift(int pressed, int code)
{
	int scancode=dc_keyboard_shift[code];
	if (pressed)
	{
//	 	_handle_key_press(0, scancode);
		key[scancode]=1;
		_key_shifts|=dc_keyboard_shift[code];
	}
	else
	{
//	 	_handle_key_release(scancode);
		key[scancode]=0;
		_key_shifts&=~dc_keyboard_shift[code];
	}
}


static void dc_keyboard_poll(void)
{
// puts(__FILE__ ": dc_keyboard_poll"); fflush(stdout);
	kbd_state_t	*state;
	uint8	addr;
	int	i,port,unit,shiftkeys;
//	int	changed=0;

	spinlock_lock(&__dc_allegro_event_mutex);

	addr = maple_first_kb();

	if (addr==0)
	{
		spinlock_unlock(&__dc_allegro_event_mutex);
		return;
	}
// puts(__FILE__ ": dc_keyboard_poll keyboard found"); fflush(stdout);

	maple_raddr(addr,&port,&unit);

	state = (kbd_state_t *) maple_dev_status (maple_enum_dev(port,unit));
	if (!state)
	{
		spinlock_unlock(&__dc_allegro_event_mutex);
		return;
	}
	else
	{
		kbd_cond_t *cond=(kbd_cond_t *)&state->cond;
		kbd_cond_t *old_cond=(kbd_cond_t *)&dc_keyboard_old_state.cond;
		if (	cond->keys[0]==old_cond->keys[0] &&
			cond->keys[1]==old_cond->keys[1] &&
			cond->keys[2]==old_cond->keys[2] &&
			cond->keys[3]==old_cond->keys[3] &&
			cond->keys[4]==old_cond->keys[4] &&
			cond->keys[5]==old_cond->keys[5] )
		{
			spinlock_unlock(&__dc_allegro_event_mutex);
			return;
		}
	}
// puts(__FILE__ ": dc_keyboard_poll state"); fflush(stdout);

	shiftkeys = state->shift_keys ^ dc_keyboard_old_state.shift_keys;
	for(i=0;i<sizeof(dc_keyboard_shift);i++) {
		if ((shiftkeys>>i)&1) {
			dc_keyboard_handler_shift(((state->shift_keys>>i)&1),i);
//			changed=1;
		}
	}

	for(i=0;i<sizeof(dc_keyboard_key);i++) {
		if (state->matrix[i]!=dc_keyboard_old_state.matrix[i]) {
			dc_keyboard_handler_key(state->matrix[i],i);
//			changed=1;
		}
	}

//	if (changed)
		dc_keyboard_old_state = *state;
	spinlock_unlock(&__dc_allegro_event_mutex);
}

/*
static void dc_keyboard_wait_for_input(void)
{
// puts(__FILE__ ": dc_keyboard_wait_for_input"); fflush(stdout);
	spinlock_lock(&__dc_allegro_event_mutex);
}

static void dc_keyboard_stop_waiting_for_input(void)
{
// puts(__FILE__ ": dc_keyboard_stop_waiting_for_input"); fflush(stdout);
	spinlock_unlock(&__dc_allegro_event_mutex);
}
*/


static int dc_keyboard_init(void)
{
puts(__FILE__ ": dc_keyboard_init"); fflush(stdout);
   _pckeys_init();
   memset(&dc_keyboard_old_state,0,sizeof(dc_keyboard_old_state));
   
   return 0;
}



static void dc_keyboard_exit(void)
{
puts(__FILE__ ": dc_keyboard_exit"); fflush(stdout);
}
