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
 *      Dreamcast Sound driver.
 *
 *      By Chui.
 *
 *      See readme.txt for copyright information.
 */


#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/aintdc.h"
#include "aica.h"

#ifndef ALLEGRO_DC
#error Something is wrong with the makefile
#endif

static int dc_sound_detect(int input);
static int dc_sound_init(int input, int voices);
static void dc_sound_exit(int input);
static int dc_sound_set_mixer_volume(int volume);
static int dc_sound_get_mixer_volume(void);
static void *dc_sound_lock_voice(int voice, int start, int end);
static void dc_sound_unlock_voice(int voice);
static int dc_sound_buffer_size(void);

DIGI_DRIVER digi_dc =
{
   DIGI_DC,
   empty_string,
   empty_string,
   "Dreamcast Sound",
   0,
   0,
   MIXER_MAX_SFX,
   MIXER_DEF_SFX,

   dc_sound_detect,
   dc_sound_init,
   dc_sound_exit,
   /* dc_sound_set_mixer_volume */ NULL,
   /* dc_sound_get_mixer_volume */ NULL,
   /* dc_sound_lock_voice */ NULL,
   /* dc_sound_unlock_voice */ NULL,
   dc_sound_buffer_size,
   _mixer_init_voice,
   _mixer_release_voice,
   _mixer_start_voice,
   _mixer_stop_voice,
   _mixer_loop_voice,

   _mixer_get_position,
   _mixer_set_position,

   _mixer_get_volume,
   _mixer_set_volume,
   _mixer_ramp_volume,
   _mixer_stop_volume_ramp,

   _mixer_get_frequency,
   _mixer_set_frequency,
   _mixer_sweep_frequency,
   _mixer_stop_frequency_sweep,

   _mixer_get_pan,
   _mixer_set_pan,
   _mixer_sweep_pan,
   _mixer_stop_pan_sweep,

   _mixer_set_echo,
   _mixer_set_tremolo,
   _mixer_set_vibrato,
   0, 0,
   0,
   0,
   0,
   0,
   0,
   0
};

static int dc_sound_active=FALSE;
static unsigned char *dc_sound_mixbuf=NULL;
static unsigned dc_sound_mixlen=0;
static int dc_sound_playing=0;
static int dc_sound_leftpos=0;
static int dc_sound_rightpos=0;
static int dc_sound_nextbuf=0;
static int dc_sound_freq=0;
static unsigned short dc_sound_format=0;
static unsigned char dc_sound_channels =0;
static unsigned char dc_sound_silence =0;
static unsigned short dc_sound_samples =0;
static unsigned short dc_sound_padding =0;
static unsigned dc_sound_size =0;
static void dc_sound_thread(void * p);


static int dc_sound_detect(int input)
{
puts(__FILE__ ": dc_sound_detect"); fflush(stdout);
	if (input)
		return FALSE;
	return TRUE;
}

static int dc_sound_init(int input, int voices)
{
puts(__FILE__ ": dc_sound_init"); fflush(stdout);
	if (input)
		return -1;
	if (dc_sound_active)
		dc_sound_exit(input);
	spu_init();
	if (!_sound_freq <= 0)
		_sound_freq=44100;
	dc_sound_freq = _sound_freq;
	dc_sound_channels = (_sound_stereo) ? 2 : 1;
	dc_sound_format = (_sound_bits == 8) ? 1 : 2;
	dc_sound_samples = 1024;
printf(__FILE__ ": dc_sound_init -> freq=%i, channels=%i, format=%i\n",dc_sound_freq,dc_sound_channels,dc_sound_format); fflush(stdout);
	dc_sound_silence = 0;
	dc_sound_size = dc_sound_format * dc_sound_samples * dc_sound_channels;
	dc_sound_mixlen = dc_sound_size;
	dc_sound_mixbuf = calloc(2,dc_sound_size);
	dc_sound_leftpos = 0x11000;
	dc_sound_rightpos = 0x11000+dc_sound_size;
	dc_sound_playing = 0;
	dc_sound_nextbuf = 0;
	digi_dc.voices = voices;
	_mixer_init(dc_sound_size / (_sound_bits / 8), dc_sound_freq,
	           _sound_stereo, ((_sound_bits == 16) ? 1 : 0),
		   &digi_dc.voices);
	dc_sound_active=TRUE;
	thd_create(dc_sound_thread, NULL);
	irq_enable();
	return 0;
}

static void dc_sound_exit(int input)
{
puts(__FILE__ ": dc_sound_exit"); fflush(stdout);
	if (dc_sound_active)
	{
		dc_sound_active=FALSE;
		timer_spin_sleep(150);
		_mixer_exit();
		spu_shutdown();
		free(dc_sound_mixbuf);
	}
}

#if 0
static int dc_sound_set_mixer_volume(int volume)
{
puts(__FILE__ ": dc_sound_set_mixer_volume"); fflush(stdout);
	return 0;
}

static int dc_sound_get_mixer_volume(void)
{
puts(__FILE__ ": dc_sound_get_mixer_volume"); fflush(stdout);
	return 255;
}

static void *dc_sound_lock_voice(int voice, int start, int end)
{
puts(__FILE__ ": dc_sound_lock_voice"); fflush(stdout);
	return NULL;
}

static void dc_sound_unlock_voice(int voice)
{
puts(__FILE__ ": dc_sound_unlock_voice"); fflush(stdout);
}
#endif

static int dc_sound_buffer_size(void)
{
//puts(__FILE__ ": dc_sound_buffer_size"); fflush(stdout);
	return dc_sound_size / (_sound_bits / 8) / (_sound_stereo ? 2 : 1);
}

static void dc_sound_spu_memload_stereo8(int leftpos,int rightpos,void *__restrict__ src0,size_t size)
{
	uint8 *src = src0;
	uint32 *left  = (uint32*)(leftpos + DC_AICA_SPU_RAM_BASE);
	uint32 *right = (uint32*)(rightpos+ DC_AICA_SPU_RAM_BASE);
	unsigned old1,old2;
	DC_AICA_G2_LOCK(old1, old2);
	size >>= 5;
	while(size--) {
		register unsigned lval,rval;

		lval = *src++; rval = *src++;
		lval|= (*src++)<<8; rval|= (*src++)<<8;
		lval|= (*src++)<<16; rval|= (*src++)<<16;
		lval|= (*src++)<<24; rval|= (*src++)<<24;
		DC_AICA_G2_WRITE_32(left++,lval);
		DC_AICA_G2_WRITE_32(right++,rval);

		lval = *src++; rval = *src++;
		lval|= (*src++)<<8; rval|= (*src++)<<8;
		lval|= (*src++)<<16; rval|= (*src++)<<16;
		lval|= (*src++)<<24; rval|= (*src++)<<24;
		DC_AICA_G2_WRITE_32(left++,lval);
		DC_AICA_G2_WRITE_32(right++,rval);

		lval = *src++; rval = *src++;
		lval|= (*src++)<<8; rval|= (*src++)<<8;
		lval|= (*src++)<<16; rval|= (*src++)<<16;
		lval|= (*src++)<<24; rval|= (*src++)<<24;
		DC_AICA_G2_WRITE_32(left++,lval);
		DC_AICA_G2_WRITE_32(right++,rval);

		lval = *src++; rval = *src++;
		lval|= (*src++)<<8; rval|= (*src++)<<8;
		lval|= (*src++)<<16; rval|= (*src++)<<16;
		lval|= (*src++)<<24; rval|= (*src++)<<24;
		DC_AICA_G2_WRITE_32(left++,lval);
		DC_AICA_G2_WRITE_32(right++,rval);
	}
	DC_AICA_G2_UNLOCK(old1, old2);
	DC_AICA_G2_FIFO_WAIT();
}

static void dc_sound_spu_memload_stereo16(int leftpos,int rightpos,void *__restrict__ src0,size_t size)
{
	uint16 *src = src0;
	uint32 *left  = (uint32*)(leftpos + DC_AICA_SPU_RAM_BASE);
	uint32 *right = (uint32*)(rightpos+ DC_AICA_SPU_RAM_BASE);
	unsigned old1,old2;
	DC_AICA_G2_LOCK(old1, old2);
	size >>= 5;
	while(size--) {
		register unsigned lval,rval;

		lval = (*src++); rval = *src++;
		lval|= (*src++)<<16; rval|= (*src++)<<16;
		DC_AICA_G2_WRITE_32(left++,lval);
		DC_AICA_G2_WRITE_32(right++,rval);

		lval = (*src++); rval = *src++;
		lval|= (*src++)<<16; rval|= (*src++)<<16;
		DC_AICA_G2_WRITE_32(left++,lval);
		DC_AICA_G2_WRITE_32(right++,rval);

		lval = (*src++); rval = *src++;
		lval|= (*src++)<<16; rval|= (*src++)<<16;
		DC_AICA_G2_WRITE_32(left++,lval);
		DC_AICA_G2_WRITE_32(right++,rval);

		lval = (*src++); rval = *src++;
		lval|= (*src++)<<16; rval|= (*src++)<<16;
		DC_AICA_G2_WRITE_32(left++,lval);
		DC_AICA_G2_WRITE_32(right++,rval);
	}
	DC_AICA_G2_UNLOCK(old1, old2);
	DC_AICA_G2_FIFO_WAIT();
}

static void dc_sound_spu_memload_mono(uint32 dst, uint32 *__restrict__ src,size_t size)
{
	register uint32 *dat  = (uint32*)(dst + DC_AICA_SPU_RAM_BASE);

	unsigned old1,old2;
	DC_AICA_G2_LOCK(old1, old2);
	size >>= 5;
	while(size--) {
		DC_AICA_G2_WRITE_32(dat++,*src++);
		DC_AICA_G2_WRITE_32(dat++,*src++);
		DC_AICA_G2_WRITE_32(dat++,*src++);
		DC_AICA_G2_WRITE_32(dat++,*src++);
		DC_AICA_G2_WRITE_32(dat++,*src++);
		DC_AICA_G2_WRITE_32(dat++,*src++);
		DC_AICA_G2_WRITE_32(dat++,*src++);
		DC_AICA_G2_WRITE_32(dat++,*src++);
	}
	DC_AICA_G2_UNLOCK(old1, old2);
	DC_AICA_G2_FIFO_WAIT();
}

static void dc_sound_thread(void * p)
{
puts(__FILE__ ": dc_sound_thread"); fflush(stdout);
	while(dc_sound_active)
	{
		unsigned offset;
		_mix_some_samples((unsigned long)dc_sound_mixbuf,0,(dc_sound_format==1)?FALSE:TRUE);
		if (dc_sound_playing)
			while(dc_aica_get_pos(0)/dc_sound_samples == dc_sound_nextbuf) 
				thd_pass();
		offset = dc_sound_nextbuf*dc_sound_size;
		dc_sound_nextbuf^=1;
		if (dc_sound_channels==1)
			dc_sound_spu_memload_mono(dc_sound_leftpos+offset,(uint32 *)dc_sound_mixbuf,dc_sound_mixlen);
		else
		{
			offset>>=1;
			if (dc_sound_format==1)
				dc_sound_spu_memload_stereo8(dc_sound_leftpos+offset,dc_sound_rightpos+offset,dc_sound_mixbuf,dc_sound_mixlen);
			else
				dc_sound_spu_memload_stereo16(dc_sound_leftpos+offset,dc_sound_rightpos+offset,dc_sound_mixbuf,dc_sound_mixlen);
		}
		if (!dc_sound_playing)
		{
			int mode;
			dc_sound_playing=1;
			mode = (dc_sound_format==1)?DC_AICA_SM_8BIT:DC_AICA_SM_16BIT;
			if (dc_sound_channels==1)
				dc_aica_play(0,mode,dc_sound_leftpos,0,dc_sound_samples<<1,dc_sound_freq,255,128,1);
			else
			{
				dc_aica_play(0,mode,dc_sound_leftpos ,0,dc_sound_samples<<1,dc_sound_freq>>1,255,0,1);
				dc_aica_play(1,mode,dc_sound_rightpos,0,dc_sound_samples<<1,dc_sound_freq>>1,255,255,1);
			}
		}
//		else
//			_mix_some_samples((unsigned long)dc_sound_mixbuf,0,(dc_sound_format==1)?FALSE:TRUE);
	}
puts(__FILE__ ": !dc_sound_thread"); fflush(stdout);
}
