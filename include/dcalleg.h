#ifndef DC_ALLEGRO_H
#define DC_ALLEGRO_H

#ifndef DREAMCAST
#define DREAMCAST
#endif

#ifndef ALLEGRO_DC
#define ALLEGRO_DC
#endif

#ifndef SCAN_DEPEND
#define SCAN_DEPEND
#endif

#ifndef ALLEGRO_NO_FIX_ALIASES
#define ALLEGRO_NO_FIX_ALIASES
#endif

#include <kos.h>
#include <stdio.h>
#include <stdlib.h>

#include "allegro.h"

void dc_allegro_set_window(int width, int height);
void dc_allegro_emulate_keyboard(int value);
void dc_allegro_emulate_mouse(int value);

#endif
