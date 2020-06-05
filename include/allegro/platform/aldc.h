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
 *      Dreamcast specific driver definitions.
 *
 *      By Chui.
 *
 *      See readme.txt for copyright information.
 */

/* System driver */
#define SYSTEM_DC            AL_ID('D','S','Y','S')
AL_VAR(SYSTEM_DRIVER, system_dc);


/* Timer driver */
#define TIMER_DC            AL_ID('D','T','I','M')
AL_VAR(TIMER_DRIVER, timer_dc);



/* Keyboard driver */
#define KEYBOARD_DC          AL_ID('D','K','E','Y')
AL_VAR(KEYBOARD_DRIVER, keyboard_dc);


/* Mouse driver */
#define MOUSE_DC             AL_ID('D','M','S','E')
AL_VAR(MOUSE_DRIVER, mouse_dc);


/* Graphics drivers */
#define GFX_DC_DMA		AL_ID('D','D','M','A')
#define GFX_DC_TEXTURED		AL_ID('D','T','X','T')
#define GFX_DC_DIRECT		AL_ID('D','D','I','R')
AL_VAR(GFX_DRIVER, gfx_dc_dma);
AL_VAR(GFX_DRIVER, gfx_dc_textured);
AL_VAR(GFX_DRIVER, gfx_dc_direct);


/* Sound driver */
#define DIGI_DC			AL_ID('D','C','S','N')
AL_VAR(DIGI_DRIVER, digi_dc);


/* Joystick driver */
#define JOYSTICK_DC		AL_ID('D','J','O','Y')
AL_VAR(JOYSTICK_DRIVER, joystick_dc);

