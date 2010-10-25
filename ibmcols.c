/**********************************************************************
 *
 * ibmcols.h - internal color definition
 *
 * NOTE: arch. independence header
 * for more information, please see the readme file
 *
 **********************************************************************/

#include "ibmcols.h"
#include "ibitmapm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/**********************************************************************
 * GLOBAL VARIABLES
 **********************************************************************/
long _ishift_endian = 0;
long _ishift_endmag = 0x11223344;

int _ishift_data_b1 = 0;
int _ishift_data_b2 = 8;
int _ishift_data_b3 = 16;


/* lookup table for scaling 1 bit colors up to 8 bits */
int _iscale_rgb_1[2] = { 0, 255 };

/* lookup table for scaling 4 bit colors up to 8 bits */
int _iscale_rgb_4[16] = 
{
	0, 16, 32, 49, 65, 82, 98, 115, 
	139, 156, 172, 189, 205, 222, 238, 255
};

/* lookup table for scaling 5 bit colors up to 8 bits */
int _iscale_rgb_5[32] =
{
   0,   8,   16,  24,  32,  41,  49,  57,
   65,  74,  82,  90,  98,  106, 115, 123,
   131, 139, 148, 156, 164, 172, 180, 189,
   197, 205, 213, 222, 230, 238, 246, 255
};

/* lookup table for scaling 6 bit colors up to 8 bits */
int _iscale_rgb_6[64] =
{
   0,   4,   8,   12,  16,  20,  24,  28,
   32,  36,  40,  44,  48,  52,  56,  60,
   64,  68,  72,  76,  80,  85,  89,  93,
   97,  101, 105, 109, 113, 117, 121, 125,
   129, 133, 137, 141, 145, 149, 153, 157,
   161, 165, 170, 174, 178, 182, 186, 190,
   194, 198, 202, 206, 210, 214, 218, 222,
   226, 230, 234, 238, 242, 246, 250, 255
};

IRGB _ipaletted[256];
int _icolorconv = 0;

/* color converter 15/16 bits -> 32 bits lookup table */
ICOLORD _iconvert_rgb_16_32[2048];
ICOLORD _iconvert_rgb_15_32[2048];

#ifndef IDISABLE_CONVERT
/* color converter 15 bits <-> 16 bits lookup table */
ICOLORW _iconvert_rgb_15_16[2048];
ICOLORW _iconvert_rgb_16_15[2048];
ICOLORW _iconvert_rgb_15_15[2048];
ICOLORW _iconvert_rgb_16_16[2048];

/* color converter 1555 <-> 4444 <-> 8888 lookup table */
ICOLORW _iconvert_rgb_1555_4444[2048];
ICOLORW _iconvert_rgb_4444_1555[2048];
ICOLORD _iconvert_rgb_1555_8888[2048];
ICOLORD _iconvert_rgb_4444_8888[2048];
#endif


/* 8 bits min/max saturation table */
const unsigned char _IMINMAX8[770] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,
   14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,
   29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,
   44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
   59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,
   74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,
   89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103,
  104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118,
  119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
  134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
  149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
  164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178,
  179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193,
  194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
  209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
  224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238,
  239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253,
  254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255,
};

const unsigned char *_iminmax8 = &_IMINMAX8[256];
const unsigned char *_iclip256 = &_IMINMAX8[256];


#if defined(__BORLANDC__) && !defined(__MSDOS__)
#pragma warn -8004  
#pragma warn -8057
#endif


/**********************************************************************
 * GLOBAL FUNCTION
 **********************************************************************/
void _ishift_init(void)
{
	static long endian = 0x11223344;
	static long inited = 0;

	if (inited) return;	

	if (((unsigned char*)&endian)[0] == 0x44) {	
		/* for little endian */
		_ishift_data_b1 = 0;
		_ishift_data_b2 = 8;
		_ishift_data_b3 = 16;
		_ishift_endian = 0;
	}	else {
		/* for big endian */
		_ishift_data_b1 = 16;
		_ishift_data_b2 = 8;
		_ishift_data_b3 = 0;
		_ishift_endian = 1;
	}

	assert(sizeof(ICOLORB) == 1 && sizeof(ICOLORW) == 2);
	assert(sizeof(ICOLORD) == 4);

	inited = 1;
}


/**********************************************************************
 * 256 PALETTE INTERFACE
 **********************************************************************/

/* find best fit color */
int _ibestfit_color(const IRGB *pal, int r, int g, int b, int palsize)
{ 
	static ICOLORD diff_lookup[512 * 3] = { 0 };
	long lowest = 0x7FFFFFFF, bestfit = 0;
	long coldiff, i, k;
	IRGB *rgb;

	/* calculate color difference lookup table:
	 * COLOR DIFF TABLE
	 * table1: diff_lookup[i | i = 256->511, n=0->(+255)] = (n * 30) ^ 2
	 * table2: diff_lookup[i | i = 256->1,   n=0->(-255)] = (n * 30) ^ 2
	 * result: f(n) = (n * 30) ^ 2 = diff_lookup[256 + n]
	 */
	if (diff_lookup[0] == 0) {
		for (i = 0; i < 256; i++) {
			k = i * i;
			diff_lookup[ 256 + i] = diff_lookup[ 256 - i] = k * 30 * 30;
			diff_lookup[ 768 + i] = diff_lookup[ 768 - i] = k * 59 * 59;
			diff_lookup[1280 + i] = diff_lookup[1280 - i] = k * 11 * 11;
		}
		diff_lookup[0] = 1;
	}

	/* range correction */
	r = r & 255;
	g = g & 255;
	b = b & 255;

	/*
	 * vector:   c1 = [r1, g1, b1], c2 = [r2, g2, b2]
	 * distance: dc = length(c1 - c2)
	 * coldiff:  dc^2 = (r1 - r2)^2 + (g1 - g2)^2 + (b1 - b2)^2
	 */
	for (i = palsize, rgb = (IRGB*)pal; i > 0; rgb++, i--) {
		coldiff  = diff_lookup[ 768 + rgb->g - g];
		if (coldiff >= lowest) continue;

		coldiff += diff_lookup[ 256 + rgb->r - r];
		if (coldiff >= lowest) continue;

		coldiff += diff_lookup[1280 + rgb->b - b];
		if (coldiff >= lowest) continue;

		bestfit = (int)(rgb - (IRGB*)pal); /* faster than `bestfit = i;' */
		if (coldiff == 0) return bestfit;
		lowest = coldiff;
	}

	return bestfit;
} 


int _igenerate_332_palette(IRGB *pal)
{
	ICOLORD c;

	pal = (pal)? pal : _ipaletted;
	for (c = 0; c < 256; c++) {
		pal[c].r = (unsigned char)(((c >> 5) & 7) * 255 / 7);
		pal[c].g = (unsigned char)(((c >> 2) & 7) * 255 / 7);
		pal[c].b = (unsigned char)((c & 3) * 255 / 3);
	}

	pal[0].r = 255;
	pal[0].g = 0;
	pal[0].b = 255;

	pal[255].r = pal[255].g = pal[255].b = 0;

	return 0;
}

/* generate bestfit color lookup table 
 *     nbits = 4     format = R4G4B4, tabsize = 4096       (2 ^ 12)
 *     nbits = 5     format = R5G5B5, tabsize = 32768      (2 ^ 15)
 *     nbits = 6     format = R6G6B6, tabsize = 262144     (2 ^ 18)
 *     nbits = 7     format = R7G7B7, tabsize = 2097152    (2 ^ 21)
 *     nbits = 8     format = R8G8B8, tabsize = 16777216   (2 ^ 24)
 */
int _ibestfit_table(const IRGB *pal, int len, unsigned char *out, int nbits)
{
	ICOLORD col = 0;
	if (nbits == 4) {
		for (col = 0; col < (1ul << 12); col++) {
			int r = _iscale_rgb_4[(col >> 8) & 15];
			int g = _iscale_rgb_4[(col >> 4) & 15];
			int b = _iscale_rgb_4[(col >> 0) & 15];
			out[col] = _ibestfit_color(pal, r, g, b, len);
		}
	}
	else if (nbits == 5) {
		for (col = 0; col < (1ul << 15); col++) {
			int r = _iscale_rgb_5[(col >> 10) & 31];
			int g = _iscale_rgb_5[(col >>  5) & 31];
			int b = _iscale_rgb_5[(col >>  0) & 31];
			out[col] = _ibestfit_color(pal, r, g, b, len);
		}
	}
	else if (nbits == 6) {
		for (col = 0; col < (1ul << 18); col++) {
			int r = _iscale_rgb_6[(col >> 12) & 63];
			int g = _iscale_rgb_6[(col >>  6) & 63];
			int b = _iscale_rgb_6[(col >>  0) & 63];
			out[col] = _ibestfit_color(pal, r, g, b, len);
		}
	}
	else if (nbits == 7) {
		for (col = 0; col < (1ul << 21); col++) {
			int r = (col >> 14) & 127;
			int g = (col >>  7) & 127;
			int b = (col >>  0) & 127;
			r = (r << 1) | ((r >> 6) & 1);
			g = (g << 1) | ((g >> 6) & 1);
			b = (b << 1) | ((b >> 6) & 1);
			out[col] = _ibestfit_color(pal, r, g, b, len);
		}
	}
	else if (nbits == 8) {
		for (col = 0; col < (1ul << 24); col++) {
			int r = (col >> 16) & 0xff;
			int g = (col >>  8) & 0xff;
			int b = (col >>  0) & 0xff;
			out[col] = _ibestfit_color(pal, r, g, b, len);
		}
	}
	else {
		return -1;
	}
	return 0;
}


/**********************************************************************
 * PIXEL FORMAT
 **********************************************************************/
const struct IPIXELFMT ipixel_fmt[] = {
	{ IPIX_FMT_NONE,  0, 0, 0, 0,  0, 0, 0, 0 },
	{ IPIX_FMT_8, 8, 0, 0, 0,  0, 0, 0, 0 },
	{ IPIX_FMT_RGB15, 15, 0, 0, 0,  0x7c00, 0x03e0, 0x001f, 0 },
	{ IPIX_FMT_BGR15, 15, 1, 0, 0,  0x001f, 0x03d0, 0x7c00, 0 },
	{ IPIX_FMT_RGB16, 16, 0, 0, 0,  0xf800, 0x07e0, 0x001f, 0 },
	{ IPIX_FMT_BGR16, 16, 1, 0, 0,  0x001f, 0x07e0, 0xf800, 0 },
	{ IPIX_FMT_RGB24, 24, 0, 0, 0,  0xff0000, 0x00ff00, 0x0000ff, 0 },
	{ IPIX_FMT_BGR24, 24, 1, 0, 0,  0x0000ff, 0x00ff00, 0xff0000, 0 },
	{ IPIX_FMT_RGB32, 32, 0, 0, 0,  0xFF0000, 0x00ff00, 0x0000ff, 0 },
	{ IPIX_FMT_BGR32, 32, 1, 0, 0,  0x0000ff, 0x00ff00, 0xff0000, 0 },
	{ IPIX_FMT_ARGB32, 32, 0, 1, 0, 0xff0000, 0xff00, 0xff, 0xff000000 },
	{ IPIX_FMT_ABGR32, 32, 1, 1, 0, 0xff, 0xff00, 0xff0000, 0xff000000 },
	{ IPIX_FMT_RGBA32, 32, 0, 1, 1, 0xff000000, 0xff0000, 0xff00, 0xff },
	{ IPIX_FMT_BGRA32, 32, 1, 1, 1, 0xff00, 0xff0000, 0xff000000, 0xff },
	{ IPIX_FMT_ARGB_4444, 16, 0, 1, 0, 0xf00, 0xf0, 0xf, 0xf000 },
	{ IPIX_FMT_ABGR_4444, 16, 1, 1, 0, 0xf, 0xf0, 0xf00, 0xf000 },
	{ IPIX_FMT_RGBA_4444, 16, 0, 1, 1, 0xf000, 0xf00, 0xf0, 0xf },
	{ IPIX_FMT_BGRA_4444, 16, 1, 1, 1, 0xf0, 0xf00, 0xf000, 0xf },
	{ IPIX_FMT_ARGB_1555, 16, 0, 1, 0, 0x7c00, 0x03e0, 0x001f, 0x8000 },
	{ IPIX_FMT_ABGR_1555, 16, 1, 1, 0, 0x001f, 0x03e0, 0x7c00, 0x8000 },
	{ IPIX_FMT_RGBA_5551, 16, 0, 1, 1, 0xf800, 0x07c0, 0x003e, 0x0001 },
	{ IPIX_FMT_BGRA_5551, 16, 1, 1, 1, 0x003e, 0x07c0, 0xf800, 0x0001 },
};



/* set pixel format */
void ibitmap_set_pixfmt(struct IBITMAP *bmp, int pixelfmt)
{
	assert(bmp);
	assert(pixelfmt >= 0 && pixelfmt <= IPIX_FMT_BGRA_5551);

	if (pixelfmt == 0) {
		switch (bmp->bpp) {
		case 8: pixelfmt = IPIX_FMT_8; break;
		case 15: pixelfmt = IPIX_FMT_RGB15; break;
		case 16: pixelfmt = IPIX_FMT_RGB16; break;
		case 24: pixelfmt = IPIX_FMT_RGB24; break;
		case 32: pixelfmt = IPIX_FMT_RGB32; break;
		}
	}

	assert(pixelfmt > 0);
	assert((int)bmp->bpp == ipixel_fmt[pixelfmt].bpp);

	_ibitmap_set_pixfmt(bmp, pixelfmt);

	if (ipixel_fmt[pixelfmt].use_bgr) {
		_ibitmap_flags_set(bmp, IFLAG_USEBGR);
	}	else {
		_ibitmap_flags_clr(bmp, IFLAG_USEBGR);
	}

	if (ipixel_fmt[pixelfmt].has_alpha) {
		_ibitmap_flags_set(bmp, IFLAG_HAVEALPHA);
	}	else {
		_ibitmap_flags_clr(bmp, IFLAG_HAVEALPHA);
	}

	if (ipixel_fmt[pixelfmt].alpha_pos) {
		_ibitmap_flags_set(bmp, IFLAG_ALPHALOW);
	}	else {
		_ibitmap_flags_clr(bmp, IFLAG_ALPHALOW);
	}
}


/* set rgb color in given format */
ICOLORD _im_color_set(int pixfmt, ICOLORD rgb, const IRGB *pal)
{
	ICOLORD raw, a, r, g, b;
	IRGBA_FROM_PIXEL(rgb, RGB32, r, g, b, a);
	if (pixfmt == IPIX_FMT_8) {
		if (pal == NULL) pal = _ipaletted;
		raw = _ibestfit_color(pal, r, g, b, 256);
	}	else {
		IRGBA_ASSEMBLE(raw, pixfmt, r, g, b, a);
	}
	return raw;
}

/* get raw color from given format */
ICOLORD _im_color_get(int pixfmt, ICOLORD raw, const IRGB *pal)
{
	ICOLORD c, a, r, g, b;
	if (pixfmt == IPIX_FMT_8) {
		a = 255;
		r = pal[raw].r;
		g = pal[raw].g;
		b = pal[raw].b;
	}	else {
		IRGBA_DISEMBLE(raw, pixfmt, r, g, b, a);
	}
	c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a);
	return c;
}


/**********************************************************************
 * make rgb-color value with raw 
 **********************************************************************/
void _igenerate_convert_table(void)
{
	ICOLORD c1, c2, r1, g1, b1, r2, g2, b2, a1, a2;
	ICOLORD checkendian = 0x11223344;
	int be, i;

	#define ICONVERT_SAVE_RGB(sbits, dbits, dtype) do { \
		_im_scale_rgb(c1, sbits, r1, g1, b1); \
		_im_scale_rgb(c2, sbits, r2, g2, b2); \
		_iconvert_rgb_##sbits##_##dbits[i] = (dtype) \
			_im_pack_rgb(dbits, r1, g1, b1); \
		_iconvert_rgb_##sbits##_##dbits[i + 256] = (dtype) \
			_im_pack_rgb(dbits, r2, g2, b2); \
		_iconvert_rgb_##sbits##_##dbits[i + 512] = (dtype) \
			_im_pack_rgb(dbits, b1, g1, r1); \
		_iconvert_rgb_##sbits##_##dbits[i + 768] = (dtype) \
			_im_pack_rgb(dbits, b2, g2, r2); \
	}	while (0)

	#define ICONVERT_SAVE_RGBA(sbits, dbits, dtype) do { \
		_im_scale_argb(c1, sbits, a1, r1, g1, b1); \
		_im_scale_argb(c2, sbits, a2, r2, g2, b2); \
		_iconvert_rgb_##sbits##_##dbits[i +    0] = (dtype) \
			_im_pack_argb(dbits, a1, r1, g1, b1); \
		_iconvert_rgb_##sbits##_##dbits[i +  256] = (dtype) \
			_im_pack_argb(dbits, a2, r2, g2, b2); \
		_iconvert_rgb_##sbits##_##dbits[i +  512] = (dtype) \
			_im_pack_argb(dbits, a1, b1, g1, r1); \
		_iconvert_rgb_##sbits##_##dbits[i +  768] = (dtype) \
			_im_pack_argb(dbits, a2, b2, g2, r2); \
		_iconvert_rgb_##sbits##_##dbits[i + 1024] = (dtype) \
			_im_pack_argb(dbits, r1, g1, b1, a1); \
		_iconvert_rgb_##sbits##_##dbits[i + 1280] = (dtype) \
			_im_pack_argb(dbits, r2, g2, b2, a2); \
		_iconvert_rgb_##sbits##_##dbits[i + 1536] = (dtype) \
			_im_pack_argb(dbits, b1, g1, r1, a1); \
		_iconvert_rgb_##sbits##_##dbits[i + 1792] = (dtype) \
			_im_pack_argb(dbits, b2, g2, r2, a2); \
	}	while (0)

	be = (*(unsigned char*)(&checkendian) == 0x44)? 0 : 1;

	for (i = 0; i < 256; i++) {
		if (be == 0) c1 = (ICOLORD)i, c2 = (ICOLORD)(i << 8);
		else c2 = (ICOLORD)i, c1 = (ICOLORD)(i << 8);

		ICONVERT_SAVE_RGBA(15, 32, ICOLORD);
		ICONVERT_SAVE_RGBA(16, 32, ICOLORD);

		#ifndef IDISABLE_CONVERT
		ICONVERT_SAVE_RGB(15, 16, ICOLORW);
		ICONVERT_SAVE_RGB(16, 15, ICOLORW);
		ICONVERT_SAVE_RGB(15, 15, ICOLORW);
		ICONVERT_SAVE_RGB(16, 16, ICOLORW);
		ICONVERT_SAVE_RGBA(1555, 4444, ICOLORW);
		ICONVERT_SAVE_RGBA(4444, 1555, ICOLORW);
		ICONVERT_SAVE_RGBA(1555, 8888, ICOLORD);
		ICONVERT_SAVE_RGBA(4444, 8888, ICOLORD);
		#endif
	}

	#undef ICONVERT_SAVE_RGB
	#undef ICONVERT_SAVE_RGBA
}


int ibitmap_blitclip(const IBITMAP *dst, int *dx, int *dy, 
	const IBITMAP *src, int *sx, int *sy, int *sw, int *sh, int flags)
{
	int clipdst[4], clipsrc[4], rect[4];
	int retval;
	clipdst[0] = 0;
	clipdst[1] = 0;
	clipdst[2] = (int)dst->w;
	clipdst[3] = (int)dst->h;
	clipsrc[0] = 0;
	clipsrc[1] = 0;
	clipsrc[2] = (int)src->w;
	clipsrc[3] = (int)src->h;
	rect[0] = *sx;
	rect[1] = *sy;
	rect[2] = *sx + *sw;
	rect[3] = *sy + *sh;
	retval = ibitmap_clip(clipdst, clipsrc, dx, dy, rect, flags);
	if (retval) return retval;
	*sx = rect[0];
	*sy = rect[1];
	*sw = rect[2] - rect[0];
	*sh = rect[3] - rect[1];
	return 0;
}


/**********************************************************************
 * default converter which can convert all pixel formats (but slow)
 **********************************************************************/
void _iconvert_pixfmt(IBITMAP *dst, int dx, int dy, IBITMAP *src, int sx, 
	int sy, int sw, int sh, const IRGB *dpal, const IRGB *spal, int flags)
{
	static int inited = 0;
	unsigned char *s, *d;	
	ICOLORD smask;
	int spixsize;
	int dpixsize;
	int x, y, r, k, v;
	int sdelta;
	int ddelta;
	int incs;
	int sfmt;
	int dfmt;

	assert(dst && src);

	if (inited == 0) {
		_ishift_init();
		inited = 1;
	}

	if (_ibitmap_pixfmt(dst) == 0) ibitmap_set_pixfmt(dst, 0);
	if (_ibitmap_pixfmt(src) == 0) ibitmap_set_pixfmt(src, 0);

	dfmt = _ibitmap_pixfmt(dst);
	sfmt = _ibitmap_pixfmt(src);

	r = ibitmap_blitclip(dst, &dx, &dy, src, &sx, &sy, &sw, &sh, flags);
	if (r) return;

	if (dfmt == sfmt && sfmt != IPIX_FMT_8) {
		ibitmap_blit(dst, dx, dy, src, sx, sy, sw, sh, src->mask,
			flags & (IBLIT_MASK | IBLIT_HFLIP | IBLIT_VFLIP));
		return;
	}

	spixsize = _ibitmap_npixelbytes(src);
	dpixsize = _ibitmap_npixelbytes(dst);
	sdelta = sx * spixsize;
	ddelta = dx * dpixsize;

	incs = spixsize;

	if (flags & ICONV_HFLIP) { 
		sdelta = spixsize * (sx + sw - 1); 
		incs = -spixsize; 
	}
	if (flags & ICONV_VFLIP) k = sh - 1, v = -1; 
	else k = 0, v = 1;

	spal = spal? spal : _ipaletted;
	dpal = dpal? dpal : _ipaletted;

	smask = (ICOLORD)src->mask;

	for (y = 0; y < sh; y++, k += v) {
		ICOLORD c, a, r, g, b, z;
		s = _ilineptr(src, sy + k) + sdelta;
		d = _ilineptr(dst, dy + y) + ddelta;
		c = a = r = g = b = z = 0;
		for (x = sw; x > 0; s += incs, d += dpixsize, x--) {
			a = 255;
			switch (sfmt) 
			{
			case IPIX_FMT_8:
				c = _im_get1b(s);
				r = spal[c].r, g = spal[c].g, b = spal[c].b; 
				break;
			case IPIX_FMT_RGB15:
				c = _im_get2b(s); 
				_im_scale_rgb(c, 15, r, g, b);
				break;
			case IPIX_FMT_BGR15:
				c = _im_get2b(s); 
				_im_scale_rgb(c, 15, b, g, r);
				break;
			case IPIX_FMT_RGB16:
				c = _im_get2b(s); 
				_im_scale_rgb(c, 16, r, g, b);
				break;
			case IPIX_FMT_BGR16:
				c = _im_get2b(s); 
				_im_scale_rgb(c, 16, b, g, r);
				break;
			case IPIX_FMT_RGB24:
				c = _im_get3b(s); 
				_im_scale_rgb(c, 24, r, g, b);
				break;
			case IPIX_FMT_BGR24:
				c = _im_get3b(s); 
				_im_scale_rgb(c, 24, b, g, r);
				break;
			case IPIX_FMT_RGB32:
				c = _im_get4b(s); 
				_im_scale_rgb(c, 32, r, g, b);
				break;
			case IPIX_FMT_BGR32:
				c = _im_get4b(s); 
				_im_scale_bgr(c, 32, b, g, r);
				break;
			case IPIX_FMT_ARGB32:
				c = _im_get4b(s); 
				_im_scale_argb(c, 32, a, r, g, b);
				break;
			case IPIX_FMT_ABGR32:
				c = _im_get4b(s); 
				_im_scale_argb(c, 32, a, b, g, r);
				break;
			case IPIX_FMT_RGBA32:
				c = _im_get4b(s); 
				_im_scale_argb(c, 32, r, g, b, a);
				break;
			case IPIX_FMT_BGRA32:
				c = _im_get4b(s); 
				_im_scale_argb(c, 32, b, g, r, a);
				break;
			case IPIX_FMT_ARGB_4444:
				c = _im_get2b(s);
				_im_scale_argb(c, 4444, a, r, g, b);
				break;
			case IPIX_FMT_ABGR_4444:
				c = _im_get2b(s);
				_im_scale_argb(c, 4444, a, b, g, r);
				break;
			case IPIX_FMT_RGBA_4444:
				c = _im_get2b(s);
				_im_scale_argb(c, 4444, r, g, b, a);
				break;
			case IPIX_FMT_BGRA_4444:
				c = _im_get2b(s);
				_im_scale_argb(c, 4444, b, g, r, a);
				break;
			case IPIX_FMT_ARGB_1555:
				c = _im_get2b(s);
				_im_scale_argb(c, 1555, a, r, g, b);
				break;
			case IPIX_FMT_ABGR_1555:
				c = _im_get2b(s);
				_im_scale_argb(c, 1555, a, b, g, r);
				break;
			case IPIX_FMT_RGBA_5551:
				c = _im_get2b(s);
				_im_scale_argb(c, 5551, r, g, b, a);
				break;
			case IPIX_FMT_BGRA_5551:
				c = _im_get2b(s);
				_im_scale_argb(c, 5551, r, b, b, r);
				break;
			}

			if ((flags & ICONV_MASK) && c == smask) 
				continue;

			switch (dfmt) 
			{
			case IPIX_FMT_8:
				c = _ibestfit_color(dpal, r, g, b, 256);
				_im_put1b(d, c);
				break;
			case IPIX_FMT_RGB15:
				c = _im_pack_rgb(15, r, g, b);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_BGR15:
				c = _im_pack_rgb(15, b, g, r);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_RGB16:
				c = _im_pack_rgb(16, r, g, b);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_BGR16:
				c = _im_pack_rgb(16, b, g, r);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_RGB24:
				c = _im_pack_rgb(24, r, g, b);
				_im_put3b(d, c);
				break;
			case IPIX_FMT_BGR24:
				c = _im_pack_rgb(24, b, g, r);
				_im_put3b(d, c);
				break;
			case IPIX_FMT_RGB32:
				c = _im_pack_rgb(32, r, g, b);
				_im_put4b(d, c);
				break;
			case IPIX_FMT_BGR32:
				c = _im_pack_rgb(32, b, g, r);
				_im_put4b(d, c);
				break;
			case IPIX_FMT_ARGB32:
				c = _im_pack_argb(32, a, r, g, b);
				_im_put4b(d, c);
				break;
			case IPIX_FMT_ABGR32:
				c = _im_pack_argb(32, a, b, g, r);
				_im_put4b(d, c);
				break;
			case IPIX_FMT_RGBA32:
				c = _im_pack_argb(32, r, g, b, a);
				_im_put4b(d, c);
				break;
			case IPIX_FMT_BGRA32:
				c = _im_pack_argb(32, b, g, r, a);
				_im_put4b(d, c);
				break;
			case IPIX_FMT_ARGB_4444:
				c = _im_pack_argb(4444, a, r, g, b);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_ABGR_4444:
				c = _im_pack_argb(4444, a, b, g, r);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_RGBA_4444:
				c = _im_pack_argb(4444, r, g, b, a);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_BGRA_4444:
				c = _im_pack_argb(4444, b, g, r, a);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_ARGB_1555:
				c = _im_pack_argb(1555, a, r, g, b);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_ABGR_1555:
				c = _im_pack_argb(1555, a, b, g, r);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_RGBA_5551:
				c = _im_pack_argb(5551, a, r, g, b);
				_im_put2b(d, c);
				break;
			case IPIX_FMT_BGRA_5551:
				c = _im_pack_argb(5551, a, b, g, r);
				_im_put2b(d, c);
				break;
			}
		}
	}
}


#ifndef IDISABLE_CONVERT
/**********************************************************************
 * _ICONVERT_BLIT_APPROXIMATE
 **********************************************************************/
#define _ICONVERT_BLIT_APPROXIMATE(dbits, dsize, sbits, ssize) \
{	\
	ICOLORD sdelta = ssize * s_x;		\
	ICOLORD ddelta = dsize * d_x;		\
	unsigned char *s, *d;	\
	ICOLORD c1, r1, g1, b1;	\
	ICOLORD c2, r2, g2, b2; \
	ICOLORD smask = src->mask; \
	long y;	\
	IRGB *_ipaletted = pal;		\
	if ((flags & ICONV_MASK) == 0) { \
		if ((flags & ICONV_RGB2BGR) == 0) { \
			for (y = 0; y < h; y++) { \
				s = _ilineptr(src, s_y + y) + sdelta;	\
				d = _ilineptr(dst, d_y + y) + ddelta;	\
				ILINS_LOOP_DOUBLE( \
				_iconvert_increment_x1(dbits, dsize, sbits, ssize, rgb), \
				_iconvert_increment_x2(dbits, dsize, sbits, ssize, rgb), \
				w); \
			}	\
		}	else { \
			for (y = 0; y < h; y++) { \
				s = _ilineptr(src, s_y + y) + sdelta;	\
				d = _ilineptr(dst, d_y + y) + ddelta;	\
				ILINS_LOOP_DOUBLE( \
				_iconvert_increment_x1(dbits, dsize, sbits, ssize, bgr), \
				_iconvert_increment_x2(dbits, dsize, sbits, ssize, bgr), \
				w); \
			}	\
		} \
	}	else { \
		for (y = 0; y < h; y++) { \
			s = _ilineptr(src, s_y + y) + sdelta;	\
			d = _ilineptr(dst, d_y + y) + ddelta;	\
			if ((flags & ICONV_RGB2BGR) == 0) { \
				ILINS_LOOP_ONCE(_iconvert_increment_mask(dbits, \
					dsize, sbits, ssize, rgb), w); \
			}	else { \
				ILINS_LOOP_ONCE(_iconvert_increment_mask(dbits, \
					dsize, sbits, ssize, bgr), w); \
			} \
		}	\
	}	\
	pal = _ipaletted; \
}

#define _iconvert_increment_x1(dbits, dsize, sbits, ssize, mode) { \
	c1 = _im_get##ssize##b(s); \
	s += ssize; \
	_im_unpack_rgb(c1, sbits, r1, g1, b1); \
	c1 = _im_pack_##mode(dbits, r1, g1, b1); \
	_im_put##dsize##b(d, c1); \
	d += dsize; \
}

#define _iconvert_increment_x2(dbits, dsize, sbits, ssize, mode) { \
	c1 = _im_get##ssize##b(s); \
	s += ssize; \
	c2 = _im_get##ssize##b(s); \
	s += ssize; \
	_im_unpack_rgb(c1, sbits, r1, g1, b1); \
	_im_unpack_rgb(c2, sbits, r2, g2, b2); \
	c1 = _im_pack_##mode(dbits, r1, g1, b1); \
	c2 = _im_pack_##mode(dbits, r2, g2, b2); \
	_im_put##dsize##b(d, c1); \
	d += dsize; \
	_im_put##dsize##b(d, c2); \
	d += dsize; \
}

#define _iconvert_increment_mask(dbits, dsize, sbits, ssize, mode) { \
	c1 = _im_get##ssize##b(s); \
	s += ssize; \
	if (c1 != smask) { \
		_im_unpack_rgb(c1, sbits, r1, g1, b1); \
		c1 = _im_pack_##mode(dbits, r1, g1, b1); \
		_im_put##dsize##b(d, c1); \
	} \
	d += dsize; \
}


static void _iconvert_approxy_to_15(IBITMAP *dst, int d_x, int d_y, 
	IBITMAP *src, int s_x, int s_y, int w, int h, IRGB *pal, int flags)
{
	pal = pal? pal : _ipaletted;
	switch (src->bpp) {
	case 15: _ICONVERT_BLIT_APPROXIMATE(15, 2, 15, 2); break;
	case 16: _ICONVERT_BLIT_APPROXIMATE(15, 2, 16, 2); break;
	case 24: _ICONVERT_BLIT_APPROXIMATE(15, 2, 24, 3); break;
	case 32: _ICONVERT_BLIT_APPROXIMATE(15, 2, 32, 4); break;
	}
}

static void _iconvert_approxy_to_16(IBITMAP *dst, int d_x, int d_y, 
	IBITMAP *src, int s_x, int s_y, int w, int h, IRGB *pal, int flags)
{
	pal = pal? pal : _ipaletted;
	switch (src->bpp) {
	case 15: _ICONVERT_BLIT_APPROXIMATE(16, 2, 15, 2); break;
	case 16: _ICONVERT_BLIT_APPROXIMATE(16, 2, 16, 2); break;
	case 24: _ICONVERT_BLIT_APPROXIMATE(16, 2, 24, 3); break;
	case 32: _ICONVERT_BLIT_APPROXIMATE(16, 2, 32, 4); break;
	}
}

static void _iconvert_approxy_to_24(IBITMAP *dst, int d_x, int d_y, 
	IBITMAP *src, int s_x, int s_y, int w, int h, IRGB *pal, int flags)
{
	pal = pal? pal : _ipaletted;
	switch (src->bpp) {
	case 15: _ICONVERT_BLIT_APPROXIMATE(24, 3, 15, 2); break;
	case 16: _ICONVERT_BLIT_APPROXIMATE(24, 3, 16, 2); break;
	case 24: _ICONVERT_BLIT_APPROXIMATE(24, 3, 24, 3); break;
	case 32: _ICONVERT_BLIT_APPROXIMATE(24, 3, 32, 4); break;
	}
}

static void _iconvert_approxy_to_32(IBITMAP *dst, int d_x, int d_y, 
	IBITMAP *src, int s_x, int s_y, int w, int h, IRGB *pal, int flags)
{
	pal = pal? pal : _ipaletted;
	switch (src->bpp) {
	case 15: _ICONVERT_BLIT_APPROXIMATE(32, 4, 15, 2); break;
	case 16: _ICONVERT_BLIT_APPROXIMATE(32, 4, 16, 2); break;
	case 24: _ICONVERT_BLIT_APPROXIMATE(32, 4, 24, 3); break;
	case 32: _ICONVERT_BLIT_APPROXIMATE(32, 4, 32, 4); break;
	}
}

static void _iconvert_approxy_to_8(IBITMAP *dst, int d_x, int d_y, 
	IBITMAP *src, int s_x, int s_y, int w, int h, IRGB *pal, int flags)
{
	pal = pal? pal : _ipaletted;
	switch (src->bpp) {
	case  8: _ICONVERT_BLIT_APPROXIMATE(8, 1, 8, 1); break;
	case 15: _ICONVERT_BLIT_APPROXIMATE(8, 1, 15, 2); break;
	case 16: _ICONVERT_BLIT_APPROXIMATE(8, 1, 16, 2); break;
	case 24: _ICONVERT_BLIT_APPROXIMATE(8, 1, 24, 3); break;
	case 32: _ICONVERT_BLIT_APPROXIMATE(8, 1, 32, 4); break;
	}
}

static void _iconvert_approxy_from_8(IBITMAP *dst, int d_x, int d_y, 
	IBITMAP *src, int s_x, int s_y, int w, int h, IRGB *pal, int flags)
{
	pal = pal? pal : _ipaletted;
	switch (dst->bpp) {
	case  8: _ICONVERT_BLIT_APPROXIMATE(8, 1, 8, 1); break;
	case 15: _ICONVERT_BLIT_APPROXIMATE(15, 2, 8, 1); break;
	case 16: _ICONVERT_BLIT_APPROXIMATE(16, 2, 8, 1); break;
	case 24: _ICONVERT_BLIT_APPROXIMATE(24, 3, 8, 1); break;
	case 32: _ICONVERT_BLIT_APPROXIMATE(32, 4, 8, 1); break;
	}
}

#undef _ICONVERT_BLIT_APPROXIMATE

/**********************************************************************
 * _iconvert_approxy
 **********************************************************************/
static void _iconvert_approxy(IBITMAP *dst, int dx, int dy, IBITMAP *src,
	int sx, int sy, int w, int h, IRGB *pal, int flags)
{
	if (src->bpp == 8) {
		_iconvert_approxy_from_8(dst, dx, dy, src, sx, sy, w, h, 
			pal, flags);
		return;
	}
	switch (dst->bpp)
	{
	case 8:
		_iconvert_approxy_to_8(dst, dx, dy, src, sx, sy, w, h, pal, flags);
		break;
	case 15:
		_iconvert_approxy_to_15(dst, dx, dy, src, sx, sy, w, h, pal, flags);
		break;
	case 16:
		_iconvert_approxy_to_16(dst, dx, dy, src, sx, sy, w, h, pal, flags);
		break;
	case 24:
		_iconvert_approxy_to_24(dst, dx, dy, src, sx, sy, w, h, pal, flags);
		break;
	case 32:
		_iconvert_approxy_to_32(dst, dx, dy, src, sx, sy, w, h, pal, flags);
		break;
	}
}


#endif /* #ifndef IDISABLE_CONVERT */


/**********************************************************************
 * _ICONVERT_BLIT_2BYTES_X
 **********************************************************************/
#define _ICONVERT_BLIT_2BYTES_X(sbits, dbits, dsize, inverse) do { \
	ICOLORD sdelta = 2 * sx; \
	ICOLORD ddelta = dsize * dx; \
	ICOLORD c1, c2, c3, c4, y; \
	unsigned char *s, *d; \
	if ((flags & ICONV_ALPHALOW) == 0) { \
		if ((flags & ICONV_RGB2BGR) == 0) { \
			__iconvert_scale_rows(sbits, dbits, dsize, 0, 256); \
		}	else { \
			__iconvert_scale_rows(sbits, dbits, dsize, 512, 768); \
		} \
	}	else { \
		if ((flags & ICONV_RGB2BGR) == 0) { \
			__iconvert_scale_rows(sbits, dbits, dsize, 1024, 1280); \
		}	else { \
			__iconvert_scale_rows(sbits, dbits, dsize, 1536, 1792); \
		} \
	}	\
}	while (0)

#define __iconvert_scale_rows(sbits, dbits, dsize, off1, off2)  \
	for (y = 0; (int)y < h; y++) { \
		s = _ilineptr(src, sy + y) + sdelta;	\
		d = _ilineptr(dst, dy + y) + ddelta;	\
		ILINS_LOOP_DOUBLE( \
			{ \
				c1 = _iconvert_rgb_##sbits##_##dbits[*s++ + off1]; \
				c2 = _iconvert_rgb_##sbits##_##dbits[*s++ + off2]; \
				c1 |= c2; \
				_im_put##dsize##b(d, c1); \
				d += dsize; \
			}, \
			{	\
				c1 = _iconvert_rgb_##sbits##_##dbits[*s++ + off1]; \
				c2 = _iconvert_rgb_##sbits##_##dbits[*s++ + off2]; \
				c3 = _iconvert_rgb_##sbits##_##dbits[*s++ + off1]; \
				c4 = _iconvert_rgb_##sbits##_##dbits[*s++ + off2]; \
				c1 |= c2; \
				c3 |= c4; \
				_im_put##dsize##b(d, c1); \
				d += dsize; \
				_im_put##dsize##b(d, c3); \
				d += dsize; \
			}, \
			w); \
	}


#ifndef IDISABLE_CONVERT

static void _iconvert_blit_scale(IBITMAP *dst, int dx, int dy, IBITMAP 
	*src, int sx, int sy, int w, int h, const IRGB *pal, int flags)
{
	static ICOLORD *_iconvert_rgb_15_24 = _iconvert_rgb_15_32;
	static ICOLORD *_iconvert_rgb_16_24 = _iconvert_rgb_16_32;
	static int inited = 0;
	int r = 0;

	if (dst->bpp == src->bpp || dst->bpp == 8) r++;
	if (src->bpp != 15 && src->bpp != 16) r++;
	
	if (r) {
		_iconvert_approxy(dst, dx, dy, src, sx, sy, w, h, (IRGB*)pal, flags);
		return;
	}

	if (inited == 0) {
		_igenerate_convert_table();
		inited++;
	}

	if (src->bpp == 15) {
		switch (dst->bpp) {
		case 15: _ICONVERT_BLIT_2BYTES_X(15, 15, 2, flags); break;
		case 16: _ICONVERT_BLIT_2BYTES_X(15, 16, 2, flags); break;
		case 24: _ICONVERT_BLIT_2BYTES_X(15, 24, 3, flags); break;
		case 32: _ICONVERT_BLIT_2BYTES_X(15, 32, 4, flags); break;
		} 
	}	else 
	if (src->bpp == 16) {
		switch (dst->bpp) {
		case 15: _ICONVERT_BLIT_2BYTES_X(16, 15, 2, flags); break;
		case 16: _ICONVERT_BLIT_2BYTES_X(16, 16, 2, flags); break;
		case 24: _ICONVERT_BLIT_2BYTES_X(16, 24, 3, flags); break;
		case 32: _ICONVERT_BLIT_2BYTES_X(16, 32, 4, flags); break;
		} 
	}
}

#endif


/**********************************************************************
 * COLOR CONVERTERS
 **********************************************************************/
ICONVERTER _iconverter[24][24] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};


/**********************************************************************
 * CONVERT BLIT
 **********************************************************************/

/**
 * _iconvert_blit:
 *
 * color format converting routine, supports endian indepedance, 
 * mask color converting, lookup-table color scaling and color order
 * swapping. EACH converting is compiled into DIFFERENT-C-FUNCTION
 * to get the high performance.
 * 
 * parameters:
 * dst, dx, dy - destination bitmap and position
 * src, sx, sy - source bitmap and position
 * w, h        - size
 * pal         - palette (can be used when srcbpp/dstbpp is 8 bits
 * flags       - enable ICONV_RGB2BGR or ICONV_SHIFT or ICONV_MASK
*/

void _iconvert_blit(IBITMAP *dst, int dx, int dy, IBITMAP *src,
	int sx, int sy, int w, int h, const IRGB *dpal, const IRGB *spal, 
	int flags)
{
	static int inited = 0;
	int mask, r;
	int sfmt, dfmt;
	ICONVERTER conv;

	if (inited == 0) {
		_ishift_init();
		inited = 1;
	}
	
	assert(dst && src);
	r = ibitmap_blitclip(dst, &dx, &dy, src, &sx, &sy, &w, &h, flags);
	if (r) return;

	if (_ibitmap_pixfmt(dst) == 0) ibitmap_set_pixfmt(dst, 0);
	if (_ibitmap_pixfmt(src) == 0) ibitmap_set_pixfmt(src, 0);

	dfmt = _ibitmap_pixfmt(dst);
	sfmt = _ibitmap_pixfmt(src);

	if (dfmt == sfmt && sfmt != IPIX_FMT_8) {
		ibitmap_blit(dst, dx, dy, src, sx, sy, w, h, src->mask,
			flags & (IBLIT_MASK | IBLIT_HFLIP | IBLIT_VFLIP));
		return;
	}

	/* check if need convert rgb -> bgr */
	if (ipixel_fmt[sfmt].use_bgr != ipixel_fmt[dfmt].use_bgr) {
		flags |= ICONV_RGB2BGR;
	}
	
	/* check if need convert ARGB -> RGBA */
	if (ipixel_fmt[sfmt].alpha_pos != ipixel_fmt[dfmt].alpha_pos) {
		flags |= ICONV_ALPHALOW;
	}

	conv = _iconverter[dfmt][sfmt];
	mask = ICONV_HFLIP | ICONV_VFLIP | ICONV_DEFAULT;

	if (conv != NULL && ((flags & mask) == 0)) {
		long spixelsize = _ibitmap_npixelbytes(src);
		long dpixelsize = _ibitmap_npixelbytes(dst);
		unsigned char *s = _ilineptr(src, sy) + sx * spixelsize;
		unsigned char *d = _ilineptr(dst, dy) + dx * dpixelsize;
		r = conv(d, dst->pitch, dfmt, s, src->pitch, sfmt, 
			w, h, src->mask, flags);
		if (r != 0) 
			return;
	}

	#ifndef IDISABLE_CONVERT
	if ((flags & (ICONV_HFLIP | ICONV_VFLIP)) == 0) {
		int condition = 0;
		if ((src->bpp == 15 || src->bpp == 16) && dst->bpp != 8) {
			if (flags & (ICONV_SHIFT | ICONV_MASK))
				condition++;
			else if (ipixel_fmt[sfmt].has_alpha) 
				condition++;
			if (condition == 0) {
				_iconvert_blit_scale(dst, dx, dy, src, sx, sy, w, h, 
					(IRGB*)spal, flags);
				return;
			}
		}
		condition = 0;
		if (ipixel_fmt[sfmt].has_alpha)
			condition++;
		else if (ipixel_fmt[dfmt].has_alpha)
			condition++;
		if (condition == 0) {
			_iconvert_approxy(dst, dx, dy, src, sx, sy, w, h, 
				(IRGB*)spal, flags);
			return;
		}
	}
	#endif
	_iconvert_pixfmt(dst, dx, dy, src, sx, sy, w, h, spal, dpal, flags);
}


#ifndef IDISABLE_CONVERT
/**********************************************************************
 * _ICONVERT_BLIT_ARGB
 **********************************************************************/
#define _ICONVERT_BLIT_ARGB_EX(dbits, dsize, sbits, ssize, mode) \
{	\
	ICOLORD sdelta = ssize * s_x;		\
	ICOLORD ddelta = dsize * d_x;		\
	unsigned char *s, *d;	\
	ICOLORD c1, a1, r1, g1, b1; \
	ICOLORD c2, a2, r2, g2, b2; \
	ICOLORD smask;	\
	long y;	\
	IRGB *_ipaletted = pal;		\
	if ((ismask) == 0) { \
		for (y = 0; y < h; y++) { \
			s = _ilineptr(src, s_y + y) + sdelta;	\
			d = _ilineptr(dst, d_y + y) + ddelta;	\
			ILINS_LOOP_DOUBLE( \
				_iconvert_argb_x1(dbits, dsize, sbits, ssize, mode), \
				_iconvert_argb_x2(dbits, dsize, sbits, ssize, mode), \
				w); \
		}	\
	}	else { \
		smask = src->mask;		\
		for (y = 0; y < h; y++) { \
			s = _ilineptr(src, s_y + y) + sdelta;	\
			d = _ilineptr(dst, d_y + y) + ddelta;	\
			ILINS_LOOP_ONCE( \
				_iconvert_argb_mask(dbits, dsize, sbits, ssize, mode), \
				w); \
		}	\
	} \
	pal = _ipaletted; \
}

#define _iconvert_argb_x1(dbits, dsize, sbits, ssize, mode) { \
	c1 = _im_get##ssize##b(s); \
	s += ssize; \
	_im_unpack_argb(c1, sbits, a1, r1, g1, b1); \
	c1 = _im_pack_##mode(dbits, a1, r1, g1, b1); \
	_im_put##dsize##b(d, c1); \
	d += dsize; \
}

#define _iconvert_argb_x2(dbits, dsize, sbits, ssize, mode) { \
	c1 = _im_get##ssize##b(s); \
	s += ssize; \
	c2 = _im_get##ssize##b(s); \
	s += ssize; \
	_im_unpack_argb(c1, sbits, a1, r1, g1, b1); \
	_im_unpack_argb(c2, sbits, a2, r2, g2, b2); \
	c1 = _im_pack_##mode(dbits, a1, r1, g1, b1); \
	c2 = _im_pack_##mode(dbits, a2, r2, g2, b2); \
	_im_put##dsize##b(d, c1); \
	d += dsize; \
	_im_put##dsize##b(d, c2); \
	d += dsize; \
}

#define _iconvert_argb_mask(dbits, dsize, sbits, ssize, mode) { \
	c1 = _im_get##ssize##b(s); \
	_im_unpack_rgb(c1, sbits, r1, g1, b1); \
	a1 = (c1 != smask)? 255 : 0; \
	c1 = _im_pack_##mode(dbits, a1, r1, g1, b1); \
	_im_put##dsize##b(d, c1); \
	s += ssize; \
	d += dsize; \
}


#define _ICONVERT_BLIT_ARGB(dbits, dsize, sbits, ssize, order) {\
	if (order == 4123) { \
		_ICONVERT_BLIT_ARGB_EX(dbits, dsize, sbits, ssize, argb); \
	}	else \
	if (order == 4321) { \
		_ICONVERT_BLIT_ARGB_EX(dbits, dsize, sbits, ssize, abgr); \
	}	\
}

#define _ICONVERT_BLIT_ARGB2(dbits, dsize, sbits, ssize, order) {\
	if (order == 4123) { \
		_ICONVERT_BLIT_ARGB_EX(dbits, dsize, sbits, ssize, argb); \
	}	else \
	if (order == 4321) { \
		_ICONVERT_BLIT_ARGB_EX(dbits, dsize, sbits, ssize, abgr); \
	}	else \
	if (order == 1234) { \
		_ICONVERT_BLIT_ARGB_EX(dbits, dsize, sbits, ssize, rgba); \
	}	else \
	if (order == 3214) { \
		_ICONVERT_BLIT_ARGB_EX(dbits, dsize, sbits, ssize, bgra); \
	}	\
}

static void _iconvert_argb_to_8888(IBITMAP *dst, int d_x, int d_y, IBITMAP 
	*src, int s_x, int s_y, int w, int h, IRGB *pal, int ismask, int inverse)
{
	pal = pal? pal : _ipaletted;
	switch (_ibitmap_order(src)) {
	case 1555: _ICONVERT_BLIT_ARGB2(8888, 4, 1555, 2, inverse); break;
	case  555: _ICONVERT_BLIT_ARGB2(8888, 4,  555, 2, inverse); break;
	case 4444: _ICONVERT_BLIT_ARGB2(8888, 4, 4444, 2, inverse); break;
	case  565: _ICONVERT_BLIT_ARGB2(8888, 4,  565, 2, inverse); break;
	case  888: _ICONVERT_BLIT_ARGB2(8888, 4,  888, 3, inverse); break;
	}
}

static void _iconvert_argb_to_1555(IBITMAP *dst, int d_x, int d_y, IBITMAP 
	*src, int s_x, int s_y, int w, int h, IRGB *pal, int ismask, int inverse)
{
	pal = pal? pal : _ipaletted;
	switch (_ibitmap_order(src)) {
	case  555: _ICONVERT_BLIT_ARGB(1555, 2,  555, 2, inverse); break;
	case 4444: _ICONVERT_BLIT_ARGB(1555, 2, 4444, 2, inverse); break;
	case  565: _ICONVERT_BLIT_ARGB(1555, 2,  565, 2, inverse); break;
	case  888: _ICONVERT_BLIT_ARGB(1555, 2,  888, 3, inverse); break;
	case 8888: _ICONVERT_BLIT_ARGB(1555, 2, 8888, 4, inverse); break;
	}
}

static void _iconvert_argb_to_4444(IBITMAP *dst, int d_x, int d_y, IBITMAP 
	*src, int s_x, int s_y, int w, int h, IRGB *pal, int ismask, int inverse)
{
	pal = pal? pal : _ipaletted;
	switch (_ibitmap_order(src)) {
	case 1555: _ICONVERT_BLIT_ARGB(4444, 2, 1555, 2, inverse); break;
	case  555: _ICONVERT_BLIT_ARGB(4444, 2,  555, 2, inverse); break;
	case  565: _ICONVERT_BLIT_ARGB(4444, 2,  565, 2, inverse); break;
	case  888: _ICONVERT_BLIT_ARGB(4444, 2,  888, 3, inverse); break;
	case 8888: _ICONVERT_BLIT_ARGB(4444, 2, 8888, 4, inverse); break;
	}
}

static void _iconvert_argb_from_8(IBITMAP *dst, int d_x, int d_y, IBITMAP 
	*src, int s_x, int s_y, int w, int h, IRGB *pal, int ismask, int inverse)
{
	pal = pal? pal : _ipaletted;
	switch (_ibitmap_order(dst)) {
	case 1555: _ICONVERT_BLIT_ARGB(1555, 2, 8, 1, inverse); break;
	case 4444: _ICONVERT_BLIT_ARGB(4444, 2, 8, 1, inverse); break;
	case 8888: _ICONVERT_BLIT_ARGB(8888, 4, 8, 1, inverse); break;
	}
}

void _ibitmap_make_order(IBITMAP *bmp)
{
	if (_ibitmap_order(bmp) != 0) return;
	if (_ibitmap_flags(bmp, IFLAG_HAVEALPHA) == 0) {
		switch (bmp->bpp) 
		{
		case  8: _ibitmap_set_order(bmp,  332); break;
		case 15: _ibitmap_set_order(bmp,  555); break;
		case 16: _ibitmap_set_order(bmp,  565); break;
		case 24: _ibitmap_set_order(bmp,  888); break;
		case 32: _ibitmap_set_order(bmp, 8888); break;
		default: _ibitmap_set_order(bmp,    0); break;
		}
		return;
	}
	switch (bmp->bpp)
	{
	case  8: _ibitmap_flags_clr(bmp, IFLAG_HAVEALPHA); break;
	case 15: _ibitmap_set_order(bmp, 1555); break;
	case 16: _ibitmap_set_order(bmp, 4444); break;
	case 24: _ibitmap_flags_clr(bmp, IFLAG_HAVEALPHA); break;
	case 32: _ibitmap_set_order(bmp, 8888); break;
	}
}

int _ibitmap_check_inverse(IBITMAP *bmp) 
{
	int inverse = 0;
	if (_ibitmap_flags(bmp, IFLAG_HAVEALPHA) == 0) {
		inverse = _ibitmap_flags(bmp, IFLAG_USEBGR)? 4321 : 4123; 
		return inverse;
	}
	if (_ibitmap_flags(bmp, IFLAG_ALPHALOW) == 0) 
		inverse = _ibitmap_flags(bmp, IFLAG_USEBGR)? 4321 : 4123; 
	else
		inverse = _ibitmap_flags(bmp, IFLAG_USEBGR)? 3214 : 1234; 
	if (bmp->bpp != 32) {
		if (inverse == 3214) inverse = 4321;
		if (inverse == 1234) inverse = 4123;
	}
	return inverse? inverse : 4123;
}

#undef _ICONVERT_BLIT_ARGB_EX


/**********************************************************************
 * _iconvert_argb
 **********************************************************************/
void _iconvert_argb(IBITMAP *dst, int dx, int dy, IBITMAP *src,
	int sx, int sy, int w, int h, IRGB *pal, int ismask)
{
	int checksame;
	int inverse1;
	int inverse2;
	int flags = 0, r;

	assert(dst && src);

	r = ibitmap_blitclip(dst, &dx, &dy, src, &sx, &sy, &w, &h, flags);
	if (r) return;

	checksame = IFLAG_USEBGR | IFLAG_HAVEALPHA | IFLAG_ALPHALOW;
	if (dst->bpp == src->bpp && _ibitmap_flags(dst, checksame) == 
		_ibitmap_flags(src, checksame)) {
		ibitmap_blit(dst, dx, dy, src, sx, sy, w, h, src->mask, ismask);
		return;
	}

	if (_ibitmap_flags(dst, IFLAG_HAVEALPHA) == 0 &&
		_ibitmap_flags(src, IFLAG_HAVEALPHA) == 0) {
		checksame = (_ibitmap_flags(dst, IFLAG_USEBGR) ==
					 _ibitmap_flags(src, IFLAG_USEBGR))? 0 : 1;
		flags = (checksame? ICONV_RGB2BGR : 0) | (ismask? ICONV_MASK : 0);
		_iconvert_blit(dst, dx, dy, src, sx, sy, w, h, NULL, pal, flags);
		return;
	}

	_ibitmap_make_order(dst);
	_ibitmap_make_order(src);

	inverse1 = _ibitmap_check_inverse(dst);
	inverse2 = _ibitmap_check_inverse(src);

	if (inverse1 == inverse2) r = 4123;
	else r = inverse1;

	if (src->bpp == 8) {
		_iconvert_argb_from_8(dst, dx, dy, src, sx, sy, w, h, 
			pal, ismask, r);
		return;
	}

	switch (_ibitmap_order(dst)) 
	{
	case 8888:
		_iconvert_argb_to_8888(dst, dx, dy, src, sx, sy, w, h, 
			pal, ismask, r); 
		break;
	case 1555:
		_iconvert_argb_to_1555(dst, dx, dy, src, sx, sy, w, h, 
			pal, ismask, r); 
		break;
	case 4444:
		_iconvert_argb_to_4444(dst, dx, dy, src, sx, sy, w, h, 
			pal, ismask, r); 
		break;
	}
}

#endif


/**********************************************************************
 * Filling and Blending proc
 **********************************************************************/

/* external fill proc by pixfmt */
IBLIT_FILL_PROC _iblit_fill_proc[24] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
};

/* external blend proc by pixfmt */
IBLIT_BLEND_PROC _iblit_blend_proc[24][24] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};



/**********************************************************************
 * _IBLIT_FILL_EX
 **********************************************************************/
#ifndef IDISABLE_BLEND

#define _IFILL_RECT_NORMAL(dfmt, dsize) { \
		ICOLORD c1, c2, r1, g1, b1, a1, r2, g2, a2, b2; \
		ICOLORD r, g, b, a, oma; \
		int y; \
		IRGBA_FROM_PIXEL(color, ARGB32, r, g, b, a); \
		if (a == 255) {	\
			c1 = IRGBA_TO_PIXEL(dfmt, r, g, b, a); \
			for (y = 0; y < h; y++, ptr += pitch) { \
				unsigned char *dst = ptr; \
				ILINS_LOOP_DOUBLE(  \
				{ _ipixel_put(dsize, dst, c1); dst += dsize; }, \
				{ \
					_ipixel_put(dsize, dst, c1); dst += dsize; \
					_ipixel_put(dsize, dst, c1); dst += dsize; \
				}, \
				w); \
			} \
		}	\
		else if (a > 0) { \
			a = _iblend_normalize(a); \
			if (ipixel_fmt[IPIX_FMT_##dfmt].has_alpha == 0) {	\
				oma = 256 - a; \
				r *= a; g *= a; b *= a; \
				for (y = 0; y < h; y++, ptr += pitch) { \
					unsigned char *dst = ptr;	\
					ILINS_LOOP_DOUBLE( \
					{	\
						c1 = _ipixel_get(dsize, dst); \
						IRGB_FROM_PIXEL(c1, dfmt, r1, g1, b1); \
						r1 = (r1 * oma + r) >> 8; \
						g1 = (g1 * oma + g) >> 8; \
						b1 = (b1 * oma + b) >> 8; \
						c1 = IRGBA_TO_PIXEL(dfmt, r1, g1, b1, 255); \
						_ipixel_put(dsize, dst, c1); \
						dst += dsize; \
					}, \
					{	\
						c1 = _ipixel_get(dsize, dst); \
						c2 = _ipixel_get(dsize, dst + dsize); \
						IRGB_FROM_PIXEL(c1, dfmt, r1, g1, b1); \
						IRGB_FROM_PIXEL(c2, dfmt, r2, g2, b2); \
						r1 = (r1 * oma + r) >> 8; \
						g1 = (g1 * oma + g) >> 8; \
						b1 = (b1 * oma + b) >> 8; \
						r2 = (r2 * oma + r) >> 8; \
						g2 = (g2 * oma + g) >> 8; \
						b2 = (b2 * oma + b) >> 8; \
						c1 = IRGBA_TO_PIXEL(dfmt, r1, g1, b1, 255); \
						c2 = IRGBA_TO_PIXEL(dfmt, r2, g2, b2, 255); \
						_ipixel_put(dsize, dst, c1); \
						dst += dsize; \
						_ipixel_put(dsize, dst, c2); \
						dst += dsize; \
					},	\
					w); \
				}	\
			}	else { \
				for (y = 0; y < h; y++, ptr += pitch) { \
					ICOLORD FA1, SA1, DA1, FA2, SA2, DA2; \
					unsigned char *dst = ptr;	\
					oma = a << 8; \
					ILINS_LOOP_DOUBLE( \
					{	\
						c1 = _ipixel_get(dsize, dst); \
						IRGBA_FROM_PIXEL(c1, dfmt, r1, g1, b1, a1); \
						a1 = _iblend_norm_fast(a1); \
						FA1 = _iblend_final_alpha(a1, a); \
						SA1 = oma / FA1; \
						DA1 = 256 - SA1; \
						a1 = _iblend_unnorm(FA1); \
						r1 = (r1 * DA1 + r * SA1) >> 8; \
						g1 = (g1 * DA1 + g * SA1) >> 8; \
						b1 = (b1 * DA1 + b * SA1) >> 8; \
						c1 = IRGBA_TO_PIXEL(dfmt, r1, g1, b1, a1); \
						_ipixel_put(dsize, dst, c1); \
						dst += dsize; \
					},	\
					{	\
						c1 = _ipixel_get(dsize, dst); \
						c2 = _ipixel_get(dsize, dst + dsize); \
						IRGBA_FROM_PIXEL(c1, dfmt, r1, g1, b1, a1); \
						IRGBA_FROM_PIXEL(c2, dfmt, r2, g2, b2, a2); \
						a1 = _iblend_norm_fast(a1); \
						a2 = _iblend_norm_fast(a2); \
						FA1 = _iblend_final_alpha(a1, a); \
						FA2 = _iblend_final_alpha(a2, a); \
						SA1 = oma / FA1; \
						SA2 = oma / FA2; \
						DA1 = 256 - SA1; \
						DA2 = 256 - SA2; \
						a1 = _iblend_unnorm(FA1); \
						a2 = _iblend_unnorm(FA2); \
						r1 = (r1 * DA1 + r * SA1) >> 8; \
						g1 = (g1 * DA1 + g * SA1) >> 8; \
						b1 = (b1 * DA1 + b * SA1) >> 8; \
						r2 = (r2 * DA2 + r * SA2) >> 8; \
						g2 = (g2 * DA2 + g * SA2) >> 8; \
						b2 = (b2 * DA2 + b * SA2) >> 8; \
						c1 = IRGBA_TO_PIXEL(dfmt, r1, g1, b1, a1); \
						c2 = IRGBA_TO_PIXEL(dfmt, r2, g2, b2, a2); \
						_ipixel_put(dsize, dst, c1); \
						dst += dsize; \
						_ipixel_put(dsize, dst, c2); \
						dst += dsize; \
					},	\
					w);	\
				}	\
			} \
		} \
	}


void _ibinary_fill(unsigned char *ptr, long pitch, int w, int h, 
	ICOLORD color, int fmt)
{
	switch (fmt)
	{
	case IPIX_FMT_RGB15: _IFILL_RECT_NORMAL(RGB15, 2); break;
	case IPIX_FMT_BGR15: _IFILL_RECT_NORMAL(BGR15, 2); break;
	case IPIX_FMT_RGB16: _IFILL_RECT_NORMAL(RGB16, 2); break;
	case IPIX_FMT_BGR16: _IFILL_RECT_NORMAL(BGR16, 2); break;
	case IPIX_FMT_RGB24: _IFILL_RECT_NORMAL(RGB24, 3); break;
	case IPIX_FMT_BGR24: _IFILL_RECT_NORMAL(BGR24, 3); break;
	case IPIX_FMT_RGB32: _IFILL_RECT_NORMAL(RGB32, 4); break;
	case IPIX_FMT_BGR32: _IFILL_RECT_NORMAL(BGR32, 4); break;
	case IPIX_FMT_ARGB32: _IFILL_RECT_NORMAL(ARGB32, 4); break;
	case IPIX_FMT_ABGR32: _IFILL_RECT_NORMAL(ABGR32, 4); break;
	case IPIX_FMT_RGBA32: _IFILL_RECT_NORMAL(RGBA32, 4); break;
	case IPIX_FMT_BGRA32: _IFILL_RECT_NORMAL(BGRA32, 4); break;
	case IPIX_FMT_ARGB_4444: _IFILL_RECT_NORMAL(ARGB_4444, 2); break;
	case IPIX_FMT_ABGR_4444: _IFILL_RECT_NORMAL(ABGR_4444, 2); break;
	case IPIX_FMT_RGBA_4444: _IFILL_RECT_NORMAL(RGBA_4444, 2); break;
	case IPIX_FMT_BGRA_4444: _IFILL_RECT_NORMAL(BGRA_4444, 2); break;
	case IPIX_FMT_ARGB_1555: _IFILL_RECT_NORMAL(ARGB_1555, 2); break;
	case IPIX_FMT_ABGR_1555: _IFILL_RECT_NORMAL(ABGR_1555, 2); break;
	case IPIX_FMT_RGBA_5551: _IFILL_RECT_NORMAL(RGBA_5551, 2); break;
	case IPIX_FMT_BGRA_5551: _IFILL_RECT_NORMAL(BGRA_5551, 2); break;
	}
}

void _iblit_fastfill(unsigned char *ptr, long pitch, int w, int h, 
	ICOLORD color, int fmt)
{
	IBLIT_FILL_PROC proc = _iblit_fill_proc[fmt];
	if (proc != NULL) {
		if (proc(ptr, pitch, w, h, color) == 0)
			return;
	}
	_ibinary_fill(ptr, pitch, w, h, color, fmt);
}

#endif



/**********************************************************************
 * _iblit_flip
 **********************************************************************/
void _iblit_fill(IBITMAP *dst, int dx, int dy, int w, int h, ICOLORD rawcol)
{
	IBLIT_FILL_PROC proc;
	unsigned char *ptr;
	int fmt;

	if (dx + w <= 0 || dy + h <= 0 || w < 0 || h < 0) return;
	if (dx >= (long)dst->w || dy >= (long)dst->h) return;

	if (dx < 0) w += dx, dx = 0;
	if (dy < 0) h += dy, dy = 0;
	if (dx + w >= (long)dst->w) w = (long)dst->w - dx;
	if (dy + h >= (long)dst->h) h = (long)dst->h - dy;
	
	if (_ibitmap_pixfmt(dst) == 0) ibitmap_set_pixfmt(dst, 0);
	fmt = _ibitmap_pixfmt(dst);

	ptr = _ilineptr(dst, dy) + dx * (_ibitmap_npixelbytes(dst));

	proc = _iblit_fill_proc[fmt];

	if (proc != NULL) {
		if (proc(ptr, dst->pitch, w, h, rawcol) == 0)
			return;
	}

#ifndef IDISABLE_BLEND
	_ibinary_fill(ptr, dst->pitch, w, h, rawcol, _ibitmap_pixfmt(dst));
#endif
}

#undef _IBLIT_FILL_EX



/**********************************************************************
 * ALPHA BLIT: (only support 32bit -> 32bit 
 **********************************************************************/
int _iblit_alpha(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h, ICOLORD color, int flags)
{
	long ssize, dsize, incs; 
	long sdelta, ddelta;
	IBLIT_BLEND_PROC proc;
	unsigned char *s, *d;
	int cr, cg, cb, ca;
	int r, x, y, k, v;
	int sfmt, dfmt;
	int destalpha;
	ICOLORD mask;

	assert(dst && src);
	r = ibitmap_blitclip(dst, &dx, &dy, src, &sx, &sy, &w, &h, flags);
	if (r) return 1;

	if (_ibitmap_pixfmt(dst) == 0) ibitmap_set_pixfmt(dst, 0);
	if (_ibitmap_pixfmt(src) == 0) ibitmap_set_pixfmt(src, 0);
	dfmt = _ibitmap_pixfmt(dst);
	sfmt = _ibitmap_pixfmt(src);

	IRGBA_FROM_PIXEL(color, ARGB32, cr, cg, cb, ca);

	if (cr == cg && cg == cb) {
		ca = ca * cr / 255;
		cr = cg = cb = 255;
	}

	if (ca == 0) return 0;

	ssize = (src->bpp + 7) >> 3;
	dsize = (dst->bpp + 7) >> 3;

	proc = _iblit_blend_proc[dfmt][sfmt];

	if (proc != NULL) {
		s = _ilineptr(src, sy) + ssize * sx;
		d = _ilineptr(dst, dy) + dsize * dx;
		if (proc(d, (long)dst->pitch, s, (long)src->pitch, w, h,
			color, dfmt, sfmt, src->mask, flags) == 0) {
			return 0;
		}
	}

	incs = ssize;
	sdelta = sx * ssize;
	ddelta = dx * dsize;

	if (flags & ICONV_HFLIP) { 
		sdelta = ssize * (sx + w - 1); 
		incs = -ssize; 
	}
	if (flags & ICONV_VFLIP) k = h - 1, v = -1; 
	else k = 0, v = 1;

	mask = src->mask;
	destalpha = ipixel_fmt[dfmt].has_alpha;

	ca = _iblend_normalize(ca);
	cr = _iblend_normalize(cr);
	cg = _iblend_normalize(cg);
	cb = _iblend_normalize(cb);

	for (y = 0; y < h; y++, k += v) {
		int r1, g1, b1, a1, r2, g2, b2, a2;
		ICOLORD c1, c2;
		s = _ilineptr(src, sy + k) + sdelta;
		d = _ilineptr(dst, dy + y) + ddelta;
		for (x = 0; x < w; s += incs, d += dsize, x++) {
			IPIXEL_FROM_PTR(c1, sfmt, s);

			if ((flags & IBLIT_MASK) && c1 == mask) continue;

			IPIXEL_FROM_PTR(c2, dfmt, d);
			IRGBA_DISEMBLE(c1, sfmt, r1, g1, b1, a1);
			IRGBA_DISEMBLE(c2, dfmt, r2, g2, b2, a2);

			if (cr != cg || cg != cb) {
				r1 = (r1 * cr) >> 8;
				g1 = (g1 * cg) >> 8;
				b1 = (b1 * cb) >> 8;
			}
			if (ca != 256) {
				a1 = (a1 * ca) >> 8;
			}
			if (a1 == 0) continue;

			if (destalpha == 0) {
				if (a1 == 255) {
					r2 = r1; g2 = g1; b2 = b1; a2 = 255;
				}	else {
					IBLEND_STATIC(r1, g1, b1, a1, r2, g2, b2, a2);
				}
			}	else {
				IBLEND_NORMAL(r1, g1, b1, a1, r2, g2, b2, a2);
			}

			IRGBA_ASSEMBLE(c2, dfmt, r2, g2, b2, a2);
			IPIXEL_TO_PTR(c2, dfmt, d);
		}		
	}

	return 0;
}


/**********************************************************************
 * COLOR DITHER
 **********************************************************************/
long _iconvert_dither(IBITMAP *dst, int dx, int dy, IBITMAP *src,
	int sx, int sy, int w, int h, IRGB *pal, int flags, int dmode, 
	void *buffer, long bufsize)
{
	ICOLORD sdelta, ddelta;
	unsigned char *s, *d;	
	ICOLORD smask, c;	
	long ssize, dsize, incs; 
	int sbits, dbits;
	int x, y, k, v, r, g, b, a;
	int *line[3];
	int *errtmp;
	int e[3], nr, ng, nb;
	long size, pitch;
	int emax, emin, tmax, tmin;
	int sfmt, dfmt;

	assert(dst && src);

	if (flags & ICONV_CLIP) {
		r = ibitmap_blitclip(dst, &dx, &dy, src, &sx, &sy, &w, &h, flags);
		if (r) return 0;
	}

	if (_ibitmap_pixfmt(dst) == 0) ibitmap_set_pixfmt(dst, 0);
	if (_ibitmap_pixfmt(src) == 0) ibitmap_set_pixfmt(src, 0);
	dfmt = _ibitmap_pixfmt(dst);
	sfmt = _ibitmap_pixfmt(src);

	if (sfmt == sfmt) {
		ibitmap_blit(dst, dx, dy, src, sx, sy, w, h, src->mask, 
			flags & (IBLIT_MASK | IBLIT_HFLIP | IBLIT_VFLIP));
		return 0;
	}

	ssize = (src->bpp + 7) >> 3;
	dsize = (dst->bpp + 7) >> 3;
	incs = ssize;
	sdelta = sx * ssize;
	ddelta = dx * dsize;

	if (flags & ICONV_HFLIP) { 
		sdelta = ssize * (sx + w - 1); 
		incs = -ssize; 
	}
	if (flags & ICONV_VFLIP) k = h - 1, v = -1; 
	else k = 0, v = 1;

	smask = src->mask;
	sbits = src->bpp;
	dbits = dst->bpp;
	pal = pal? pal : _ipaletted;

	pitch = 4 * (src->w + 6) * sizeof(int);
	size = 3 * pitch;

	if (size > bufsize) return size;

	line[0] = (int*)((char*)buffer) + 2 * 4;
	line[1] = (int*)((char*)buffer + pitch) + 2 * 4;
	line[2] = (int*)((char*)buffer + pitch * 2) + 2 * 4;

	memset(buffer, 0, size);

	#define ipixerr(__yy, __xx, __ii) line[__yy][((__xx) << 2) + (__ii)]
	
	tmax = tmin = 0;

	for (y = 0; y < h; y++, k += v) {

		s = _ilineptr(src, sy + k) + sdelta;
		d = _ilineptr(dst, dy + y) + ddelta;
		emax = emin = 0;

		for (x = 0; x < w; s += incs, d += dsize, x++) {
			IPIXEL_FROM_PTR(c, sfmt, s);
			if (sfmt == IPIX_FMT_8) {
				r = pal[c].r, g = pal[c].g, b = pal[c].b; 
			}	else {
				IRGBA_DISEMBLE(c, sfmt, r, g, b, a);
			}

			r += ipixerr(0, x, 0);
			g += ipixerr(0, x, 1);
			b += ipixerr(0, x, 2);

			if (r > 255) r = 255;
			else if (r < 0) r = 0;
			if (g > 255) g = 255;
			else if (g < 0) g = 0;
			if (b > 255) b = 255;
			else if (b < 0) b = 0;

			nr = ng = nb = 0;
			a = 255;

			if ((flags & ICONV_MASK) == 0 || c != smask) {
				if (dfmt == IPIX_FMT_8) {
					c = _ibestfit_color(pal, r, g, b, 256);
					nr = pal[c].r;
					ng = pal[c].g;
					nb = pal[c].b;
				}	else {
					IRGBA_ASSEMBLE(c, dfmt, r, g, b, 255);
					IRGBA_DISEMBLE(c, dfmt, nr, ng, nb, a);
				}

				IPIXEL_TO_PTR(c, dfmt, d);
			}
			
			e[0] = (int)r - (int)nr;
			e[1] = (int)g - (int)ng;
			e[2] = (int)b - (int)nb;

			switch (dmode)
			{
			case 0:
				for (c = 0; c < 3; c++) {
					ipixerr(0, x + 1, c) += e[c] * 7 / 16;
					ipixerr(1, x - 1, c) += e[c] * 3 / 16;
					ipixerr(1, x + 0, c) += e[c] * 5 / 16;
					ipixerr(1, x + 1, c) += e[c] * 1 / 16;
				}
				break;
			case 1:
				for (c = 0; c < 3; c++) {
					ipixerr(0, x + 1, c) += e[c] * 8 / 32;
					ipixerr(0, x + 2, c) += e[c] * 4 / 32;
					ipixerr(1, x - 2, c) += e[c] * 2 / 32;
					ipixerr(1, x - 1, c) += e[c] * 4 / 32;
					ipixerr(1, x + 0, c) += e[c] * 8 / 32;
					ipixerr(1, x + 1, c) += e[c] * 4 / 32;
					ipixerr(1, x + 2, c) += e[c] * 2 / 32;
				}
				break;
			default:
				for (c = 0; c < 3; c++) {
					ipixerr(0, x + 1, c) += e[c] * 8 / 42;
					ipixerr(0, x + 2, c) += e[c] * 4 / 42;
					ipixerr(1, x - 2, c) += e[c] * 2 / 42;
					ipixerr(1, x - 1, c) += e[c] * 4 / 42;
					ipixerr(1, x + 0, c) += e[c] * 8 / 42;
					ipixerr(1, x + 1, c) += e[c] * 4 / 42;
					ipixerr(1, x + 2, c) += e[c] * 2 / 42;
					ipixerr(2, x - 2, c) += e[c] * 1 / 42;
					ipixerr(2, x - 1, c) += e[c] * 2 / 42;
					ipixerr(2, x + 0, c) += e[c] * 4 / 42;
					ipixerr(2, x + 1, c) += e[c] * 2 / 42;
					ipixerr(2, x + 2, c) += e[c] * 1 / 42;
				}
				break;
			}
			
		}

		errtmp = line[0];
		line[0] = line[1], line[1] = line[2], line[2] = errtmp;

		memset(line[2], 0, pitch);
	}

	#undef ipixerr

	return 0;
}

