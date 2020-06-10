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
 *      Configuration defines for use with Dreamcast.
 *
 *      By Chui.
 *
 *      See readme.txt for copyright information.
 */

#include<kos.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>

/* a static auto config */
#define ALLEGRO_HAVE_INTTYPES_H 1
#define ALLEGRO_HAVE_STDINT_H   1
#define ALLEGRO_HAVE_STRICMP    1
#define ALLEGRO_HAVE_STRLWR     1
#define ALLEGRO_HAVE_STRUPR     1
#define ALLEGRO_HAVE_MEMCMP     1
#define ALLEGRO_HAVE_MKSTEMP    1
#define ALLEGRO_HAVE_DIRENT_H   1
#define ALLEGRO_HAVE_SYS_UTSNAME_H 1
#define ALLEGRO_HAVE_SYS_TIME_H 1
#define ALLEGRO_HAVE_LIBPTHREAD 1

/* describe this platform */
#define ALLEGRO_PLATFORM_STR  "DREAMCAST"
#define ALLEGRO_LITTLE_ENDIAN
#undef ALLEGRO_CONSOLE_OK
#define ALLEGRO_HAVE_SCHED_YIELD
#define ALLEGRO_USE_CONSTRUCTOR
#undef ALLEGRO_MULTITHREADED 

#define O_BINARY        0

#ifdef AL_INLINE
#undef AL_INLINE
#endif
#define AL_INLINE(type, name, args, code)    static __inline__ type name args code

#ifdef AL_FUNC
#undef AL_FUNC
#endif
#define AL_FUNC(type, name, args)               type name args

#ifdef AL_CONST
#undef AL_CONST
#endif
#define AL_CONST const

#ifdef AL_METHOD
#undef AL_METHOD
#endif
#define AL_METHOD(type, name, args)             type (*name) args

#ifdef AL_VAR
#undef AL_VAR
#endif
#define AL_VAR(type, name)			extern type name

typedef int			intptr_t;
typedef unsigned int		uintptr_t;
typedef signed char		int8_t;
typedef short int		int16_t;
typedef int			int32_t;
typedef long long		int64_t;
typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long long	uint64_t;

/* arrange for other headers to be included later on */
#define ALLEGRO_EXTRA_HEADER     "allegro/platform/aldc.h"
#define ALLEGRO_INTERNAL_HEADER  "allegro/platform/aintdc.h"
