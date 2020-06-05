#ifndef _DC_AICA_H
#define _DC_AICA_H

#include <arch/irq.h>
#include <sys/types.h>
#include <dc/g2bus.h>

#define	DC_AICA_MEM	0xa0800000

#define DC_AICA_SM_8BIT	1
#define DC_AICA_SM_16BIT	0
#define DC_AICA_SM_ADPCM	2

#define DC_AICA_SND_BASE ((volatile unsigned char *)0xa0700000)

#define	DC_AICA_SPU_RAM_BASE	0xa0800000

#define	DC_AICA_SNDREGADDR(x)	(0xa0700000 + (x))
#define	DC_AICA_CHNREGADDR(ch,x)	DC_AICA_SNDREGADDR(0x80*(ch)+(x))


#define DC_AICA_SNDREG32(x)	(*(volatile unsigned long *)DC_AICA_SNDREGADDR(x))
#define DC_AICA_SNDREG8(x)	(*(volatile unsigned char *)DC_AICA_SNDREGADDR(x))
#define DC_AICA_CHNREG32(ch, x) (*(volatile unsigned long *)DC_AICA_CHNREGADDR(ch,x))
#define DC_AICA_CHNREG8(ch, x)	(*(volatile unsigned long *)DC_AICA_CHNREGADDR(ch,x))

#define DC_AICA_G2_LOCK_SIMPLE(OLD) \
	do { \
		if (!irq_inside_int()) \
			OLD = irq_disable(); \
		/* suspend any G2 DMA here... */ \
		while((*(volatile unsigned int *)0xa05f688c) & 0x20) \
			; \
	} while(0)

#define DC_AICA_G2_UNLOCK_SIMPLE(OLD) \
	do { \
		/* resume any G2 DMA here... */ \
		if (!irq_inside_int()) \
			irq_restore(OLD); \
	} while(0)

#define DC_AICA_DMAC_CHCR3 *((vuint32 *)0xffa0003c)

#define DC_AICA_G2_LOCK(OLD1, OLD2) \
	do { \
		OLD1 = irq_disable(); \
		OLD2 = DC_AICA_DMAC_CHCR3; \
		DC_AICA_DMAC_CHCR3 = OLD2 & ~1; \
		while((*(vuint32 *)0xa05f688c) & 0x20) \
			; \
	} while(0)

#define DC_AICA_G2_UNLOCK(OLD1, OLD2) \
	do { \
		DC_AICA_DMAC_CHCR3 = OLD2; \
		irq_restore(OLD1); \
	} while(0)

#define DC_AICA_G2_WRITE_32(ADDR,VALUE) \
	*((vuint32*)ADDR) = VALUE

#define DC_AICA_G2_READ_32(ADDR) \
	*((vuint32*)ADDR)

#define DC_AICA_G2_FIFO_WAIT() \
	{ \
		vuint32 const *g2_fifo = (vuint32*)0xa05f688c; \
		int i; \
		for (i=0; i<0x1800; i++) \
			if (!(*g2_fifo & 0x11)) break; \
	} 


/* Translates a volume from linear form to logarithmic form (required by
   the AICA chip */
const static unsigned char dc_allegro_logs[] = {
	0, 15, 22, 27, 31, 35, 39, 42, 45, 47, 50, 52, 55, 57, 59, 61,
	63, 65, 67, 69, 71, 73, 74, 76, 78, 79, 81, 82, 84, 85, 87, 88,
	90, 91, 92, 94, 95, 96, 98, 99, 100, 102, 103, 104, 105, 106,
	108, 109, 110, 111, 112, 113, 114, 116, 117, 118, 119, 120, 121,
	122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
	135, 136, 137, 138, 138, 139, 140, 141, 142, 143, 144, 145, 146,
	146, 147, 148, 149, 150, 151, 152, 152, 153, 154, 155, 156, 156,
	157, 158, 159, 160, 160, 161, 162, 163, 164, 164, 165, 166, 167,
	167, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176,
	177, 178, 178, 179, 180, 181, 181, 182, 183, 183, 184, 185, 185,
	186, 187, 187, 188, 189, 189, 190, 191, 191, 192, 193, 193, 194,
	195, 195, 196, 197, 197, 198, 199, 199, 200, 200, 201, 202, 202,
	203, 204, 204, 205, 205, 206, 207, 207, 208, 209, 209, 210, 210,
	211, 212, 212, 213, 213, 214, 215, 215, 216, 216, 217, 217, 218,
	219, 219, 220, 220, 221, 221, 222, 223, 223, 224, 224, 225, 225,
	226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 232, 232, 233,
	233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 239, 239, 240,
	240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246,
	247, 247, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 254, 255
};

/* For the moment this is going to have to suffice, until we really
   figure out what these mean. */
#define DC_AICAPAN(x) ((x)==0x80?(0):((x)<0x80?(0x1f):(0x0f)))
#define DC_AICAVOL(x) (0xff - dc_allegro_logs[128 + (((x) & 0xff) / 2)])

static __inline__ unsigned  DC_AICAFREQ(unsigned freq)	{
	unsigned long freq_lo, freq_base = 5644800;
	int freq_hi = 7;

	/* Need to convert frequency to floating point format
	   (freq_hi is exponent, freq_lo is mantissa)
	   Formula is ferq = 44100*2^freq_hi*(1+freq_lo/1024) */
	while (freq < freq_base && freq_hi > -8) {
		freq_base >>= 1;
		--freq_hi;
	}
	while (freq < freq_base && freq_hi > -8) {
		freq_base >>= 1;
		freq_hi--;
	}
	freq_lo = (freq<<10) / freq_base;
	return (freq_hi << 11) | (freq_lo & 1023);
}

static __inline__ void dc_aica_play(int ch,int mode,unsigned long smpptr,int loopst,int loopend,int freq,int vol,int pan,int loopflag) {
	int val;
	int old=0;

	DC_AICA_G2_LOCK_SIMPLE(old);
	DC_AICA_G2_WRITE_32(DC_AICA_CHNREGADDR(ch, 0),(DC_AICA_G2_READ_32(DC_AICA_CHNREGADDR(ch, 0)) & ~0x4000) | 0x8000);
	DC_AICA_CHNREG32(ch, 8) = loopst & 0xffff;
	DC_AICA_CHNREG32(ch, 12) = loopend & 0xffff;
	DC_AICA_CHNREG32(ch, 24) = DC_AICAFREQ(freq);
	DC_AICA_CHNREG32(ch, 36) = DC_AICAPAN(pan) | (0xf<<8);
	vol = DC_AICAVOL(vol);
	DC_AICA_CHNREG32(ch, 40) = 0x24 | (vol<<8);
	DC_AICA_CHNREG32(ch, 16) = 0x1f;	/* No volume envelope */
	DC_AICA_CHNREG32(ch, 4) = smpptr & 0xffff;
	val = 0xc000 | 0x0000 | (mode<<7) | (smpptr >> 16);
	if (loopflag) val|=0x200;
	DC_AICA_CHNREG32(ch, 0) = val;
	DC_AICA_G2_UNLOCK_SIMPLE(old);
	DC_AICA_G2_FIFO_WAIT();
}

static __inline__ int dc_aica_get_pos(int ch) {
	int ret;
	unsigned old1,old2;
	DC_AICA_G2_LOCK(old1, old2);
	DC_AICA_G2_WRITE_32(DC_AICA_SNDREGADDR(0x280c),(DC_AICA_G2_READ_32(DC_AICA_SNDREGADDR(0x280c))&0xffff00ff) | (ch<<8));
	DC_AICA_G2_FIFO_WAIT();
	ret = DC_AICA_G2_READ_32(DC_AICA_SNDREGADDR(0x2814)) & 0xffff;
	DC_AICA_G2_UNLOCK(old1, old2);
	return ret;
}


#define DC_AICA_STOP(CH) { \
	g2_write_32(DC_AICA_CHNREGADDR((CH), 0),(g2_read_32(DC_AICA_CHNREGADDR((CH), 0)) & ~0x4000) | 0x8000); \
	DC_AICA_G2_FIFO_WAIT(); \
}


#define DC_AICA_VOL(CH,VOL) { \
	g2_write_32(DC_AICA_CHNREGADDR((CH), 40),(g2_read_32(DC_AICA_CHNREGADDR((CH), 40))&0xffff00ff)|(DC_AICAVOL((VOL))<<8) ); \
	DC_AICA_G2_FIFO_WAIT(); \
}

#define DC_AICA_PAN(CH,PAN) { \
	g2_write_32(DC_AICA_CHNREGADDR((CH), 36),(g2_read_32(DC_AICA_CHNREGADDR((CH), 36))&0xffffff00)|(DC_AICAPAN((PAN))) ); \
	DC_AICA_G2_FIFO_WAIT(); \
}

#define DC_AICA_FREQ(CH,FREQ) { \
	g2_write_32(DC_AICA_CHNREGADDR((CH), 24),DC_AICAFREQ((FREQ))); \
	DC_AICA_G2_FIFO_WAIT(); \
}


#endif
