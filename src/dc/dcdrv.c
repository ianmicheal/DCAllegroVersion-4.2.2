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
 *      List of Dreamcast drivers.
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

_DRIVER_INFO _system_driver_list[] =
{
   { SYSTEM_DC,      &system_dc,      TRUE  },
   { 0,                 NULL,               0     }
};

_DRIVER_INFO _keyboard_driver_list[] =
{
   { KEYBOARD_DC,      &keyboard_dc,      TRUE  },
   { 0,                 NULL,               0     }
};


_DRIVER_INFO _timer_driver_list[] =
{
   { TIMER_DC, &timer_dc, TRUE  },
   { 0,                      NULL,                    0     }
};


_DRIVER_INFO _mouse_driver_list[] =
{
   { MOUSE_DC,         &mouse_dc,         TRUE  },
   { 0,                 NULL,               0     }
};


BEGIN_GFX_DRIVER_LIST
   { GFX_DC_DMA,      &gfx_dc_dma,       TRUE  },
   { GFX_DC_TEXTURED, &gfx_dc_textured,  TRUE  },
   { GFX_DC_DIRECT,   &gfx_dc_direct,    TRUE  },
END_GFX_DRIVER_LIST


BEGIN_DIGI_DRIVER_LIST
   { DIGI_DC,         &digi_dc,         TRUE  },
END_DIGI_DRIVER_LIST


BEGIN_MIDI_DRIVER_LIST
   { MIDI_DIGMID,       &midi_digmid,       TRUE  },
END_MIDI_DRIVER_LIST


BEGIN_JOYSTICK_DRIVER_LIST
   { JOYSTICK_DC,         &joystick_dc,         TRUE  },
END_JOYSTICK_DRIVER_LIST
