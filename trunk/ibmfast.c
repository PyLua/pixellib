//=====================================================================
//
// ibmfast.c - ibitmap fast operation
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ibmcols.h"
#include "ibitmapm.h"
#include "ibmfast.h"


//---------------------------------------------------------------------
// endian checking
//---------------------------------------------------------------------
static unsigned long _ifetch_endian = 0x11223344;

#define ILITTLE_ENDIAN	(*(unsigned char*)(&_ifetch_endian) == 0x44)


#define ILSB_FETCH24(a) ( \
	(((IUINT32)(((unsigned char*)(a))[0]))      ) | \
	(((IUINT32)(((unsigned char*)(a))[1])) <<  8) | \
	(((IUINT32)(((unsigned char*)(a))[2])) << 16) )

#define IMSB_FETCH24(a) ( \
	(((IUINT32)(((unsigned char*)(a))[0])) << 16) | \
	(((IUINT32)(((unsigned char*)(a))[1])) <<  8) | \
	(((IUINT32)(((unsigned char*)(a))[2]))      ) )

#define ILSB_STORE24(a, c) do { \
		((unsigned char*)(a))[0] = ((c)      ) & 0xff; \
		((unsigned char*)(a))[1] = ((c) >>  8) & 0xff; \
		((unsigned char*)(a))[2] = ((c) >> 16) & 0xff; \
	}	while (0)

#define IMSB_STORE24(a, c) do { \
		((unsigned char*)(a))[0] = ((c) >> 16) & 0xff; \
		((unsigned char*)(a))[1] = ((c) >>  8) & 0xff; \
		((unsigned char*)(a))[2] = ((c)      ) & 0xff; \
	}	while (0)

#define ISPLIT_ARGB(c, a, r, g, b) do { \
		a = (((IUINT32)(c)) >> 24); \
		r = (((IUINT32)(c)) >> 16) & 0xff; \
		g = (((IUINT32)(c)) >>  8) & 0xff; \
		b = (((IUINT32)(c))      ) & 0xff; \
	}	while (0)

#define ISPLIT_RGB(c, r, g, b) do { \
		r = (((IUINT32)(c)) >> 16) & 0xff; \
		g = (((IUINT32)(c)) >>  8) & 0xff; \
		b = (((IUINT32)(c))      ) & 0xff; \
	}	while (0)

#define IRGB_TO_Y(r, g, b) \
	((19595 * (r) + 38469 * (g) + 7471 * (b)) >> 16)

#define IRGB24_TO_RGB15(c) \
	((((c) >> 3) & 0x001f) | (((c) >> 6) & 0x03e0) | (((c) >> 9) & 0x7c00))

#define IRGB15_TO_ENTRY(index, c) \
	((index)->ent[c])

#define IRGB24_TO_ENTRY(index, c) \
	IRGB15_TO_ENTRY(index, IRGB24_TO_RGB15(c))


//=====================================================================
// COLOR FETCHING PROCEDURES
//=====================================================================

static IFASTCALL void _ifetch_A8(const void *bits, int x, 
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT8 *pixel = (const IUINT8*)bits + x;
	const IUINT8 *endup = pixel + w;
	while (pixel < endup) {
		*buffer++ = ((IUINT32)(*pixel++)) << 24;
	}
}

static IFASTCALL void _ifetch_G8(const void *bits, int x, 
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT8 *pixel = (const IUINT8*)bits + x;
	const IUINT8 *endup = pixel + w;
	IUINT32 c;
	while (pixel < endup) {
		c = *pixel++;
		*buffer++ = 0xff000000 | (c << 16) | (c << 8) | c;
	}
}

static IFASTCALL void _ifetch_C8(const void *bits, int x, 
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT8 *pixel = (const IUINT8*)bits + x;
	const IUINT8 *endup = pixel + w;
	IUINT32 c;
	while (pixel < endup) {
		c = *pixel++;
#ifndef IDISABLE_INDEX
		*buffer++ = idx->rgba[c];
#else
		*buffer++ = 0xff000000 | (c << 16) | (c << 8) | c;
#endif
	}
}

static IFASTCALL void _ifetch_A2R2G2B2(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT8 *pixel = (const IUINT8*)bits + x;
	const IUINT8 *endup = pixel + w;
	IUINT32 c, r, g, b, a;
	while (pixel < endup) {
		c = *pixel++;
		a = ((c & 0xc0) * 0x55) << 18;
		r = ((c & 0x30) * 0x55) << 12;
		g = ((c & 0x0c) * 0x55) <<  6;
		b = ((c & 0x03) * 0x55);
		*buffer++ = a | r | g | b;
	}
}

static IFASTCALL void _ifetch_A2B2G2R2(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT8 *pixel = (const IUINT8*)bits + x;
	const IUINT8 *endup = pixel + w;
	IUINT32 c, r, g, b, a;
	while (pixel < endup) {
		c = *pixel++;
		a = ((c & 0xc0) * 0x55) << 18;
		b = ((c & 0x30) * 0x55) >>  6;
		g = ((c & 0x0c) * 0x55) <<  6;
		r = ((c & 0x03) * 0x55) << 16;
		*buffer++ = a | r | g | b;
	}
}

static IFASTCALL void _ifetch_R3G3B2(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT8 *pixel = (const IUINT8*)bits + x;
	const IUINT8 *endup = pixel + w;
	IUINT32 c, r, g, b;
	while (pixel < endup) {
		c = *pixel++;
		r = ((c & 0xe0) | ((c & 0xe0) >> 3) | ((c & 0xc0) >> 6)) << 16;
		g = ((c & 0x1c) | ((c & 0x18) >> 3) | ((c & 0x1c) << 3)) << 8;
		b = (((c & 3)) | ((c & 3) << 2) | ((c & 3) << 4) | ((c & 3) << 6));
		*buffer++ = 0xff000000 | r | g | b;
	}
}

static IFASTCALL void _ifetch_B2G3R3(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT8 *pixel = (const IUINT8*)bits + x;
	const IUINT8 *endup = pixel + w;
	IUINT32 c, r, g, b;
	while (pixel < endup) {
		c = *pixel++;
		b = (((c & 0xc0)     ) |
			 ((c & 0xc0) >> 2) |
			 ((c & 0xc0) >> 4) |
			 ((c & 0xc0) >> 6));
		g = ((c & 0x38) | ((c & 0x38) >> 3) | ((c & 30) << 2)) << 8;
		r = (((c & 0x07)     ) |
			 ((c & 0x07) << 3) |
			 ((c & 0x06) << 6)) << 16;
		*buffer++ = 0xff000000 | r | g | b;
	}
}

static IFASTCALL void _ifetch_RGB15(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b;
	while (pixel < endup) {
		c = *pixel++;
		r = ((c & 0x7c00) | ((c & 0x7000) >> 5)) << 9;
		g = ((c & 0x03e0) | ((c & 0x0380) >> 5)) << 6;
		b = ((c & 0x001c) | ((c & 0x001f) << 5)) >> 2;
		*buffer++ = 0xff000000 | r | g | b;
	}
}

static IFASTCALL void _ifetch_BGR15(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b;
	while (pixel < endup) {
		c = *pixel++;
		b = ((c & 0x7c00) | ((c & 0x7000) >> 5)) >> 7;
		g = ((c & 0x03e0) | ((c & 0x0380) >> 5)) << 6;
		r = ((c & 0x001c) | ((c & 0x001f) << 5)) << 14;
		*buffer++ = 0xff000000 | r | g | b;
	}
}

static IFASTCALL void _ifetch_RGB16(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r;
	while (pixel < endup) {
		c = *pixel++;
		r = (((c) << 3) & 0xf8) |
			(((c) << 5) & 0xfc00) |
			(((c) << 8) & 0xf80000);
		r |= (r >> 5) & 0x70007;
		r |= (r >> 6) & 0x300;
		*buffer++ = r | 0xff000000;
	}
}

static IFASTCALL void _ifetch_BGR16(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b;
	while (pixel < endup) {
		c = *pixel++;
		b = ((c & 0xf800) | ((c & 0xe000) >> 5)) >> 8;
		g = ((c & 0x07e0) | ((c & 0x0600) >> 6)) << 5;
		r = ((c & 0x001c) | ((c & 0x001f) << 5)) << 14;
		*buffer++ = r | g | b | 0xff000000;
	}
}

static IFASTCALL void _ifetch_RGB24(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT8 *pixel = (const IUINT8*)bits + x * 3;
	const IUINT8 *endup = pixel + w * 3;
	if (ILITTLE_ENDIAN) {
		while (pixel < endup) {
			*buffer++ = ILSB_FETCH24(pixel) | 0xff000000;
			pixel += 3;
		}
	}	else {
		while (pixel < endup) {
			*buffer++ = IMSB_FETCH24(pixel) | 0xff000000;
			pixel += 3;
		}
	}
}

static IFASTCALL void _ifetch_BGR24(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT8 *pixel = (const IUINT8*)bits + x * 3;
	const IUINT8 *endup = pixel + w * 3;
	if (ILITTLE_ENDIAN) {
		while (pixel < endup) {
			*buffer++ = IMSB_FETCH24(pixel) | 0xff000000;
			pixel += 3;
		}
	}	else {
		while (pixel < endup) {
			*buffer++ = ILSB_FETCH24(pixel) | 0xff000000;
			pixel += 3;
		}
	}
}

static IFASTCALL void _ifetch_RGB32(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT32 *pixel = (const IUINT32*)bits + x;
	const IUINT32 *endup = pixel + w;
	while (pixel < endup) {
		*buffer++ = *pixel++ | 0xff000000;
	}
}

static IFASTCALL void _ifetch_BGR32(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT32 *pixel = (const IUINT32*)bits + x;
	const IUINT32 *endup = pixel + w;
	while (pixel < endup) {
		*buffer++ = (0xff000000 | (*pixel & 0xff00ff00) |
			((*pixel >> 16) & 0xff) | ((*pixel & 0xff) << 16));
		pixel++;
	}
}

static IFASTCALL void _ifetch_ARGB32(const void *bits, int x, 
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	memcpy(buffer, (const IUINT32*)bits + x, w * sizeof(IUINT32));
}

static IFASTCALL void _ifetch_ABGR32(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT32 *pixel = (const IUINT32*)bits + x;
	const IUINT32 *endup = pixel + w;
	while (pixel < endup) {
		*buffer++ = ((*pixel & 0xff00ff00) |
			((*pixel >> 16) & 0xff) | ((*pixel & 0xff) << 16));
		pixel++;
	}
}

static IFASTCALL void _ifetch_RGBA32(const void *bits, int x, 
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT32 *pixel = (const IUINT32*)bits + x;
	const IUINT32 *endup = pixel + w;
	while (pixel < endup) {
		*buffer++ = ((*pixel & 0xff) << 24) |
			((*pixel >> 8) & 0xffffff);
		pixel++;
	}
}

static IFASTCALL void _ifetch_BGRA32(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx) 
{
	const IUINT32 *pixel = (const IUINT32*)bits + x;
	const IUINT32 *endup = pixel + w;
	while (pixel < endup) {
		*buffer++ = 
			((*pixel & 0x000000ff) << 24) |
			((*pixel & 0x0000ff00) <<  8) |
			((*pixel & 0x00ff0000) >>  8) |
			((*pixel & 0xff000000) >> 24);
		pixel++;
	}
}

static IFASTCALL void _ifetch_ARGB_4444(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b, a;
	while (pixel < endup) {
		c = *pixel++;
		a = ((c & 0xf000) | ((c & 0xf000) >> 4)) << 16;
		r = ((c & 0x0f00) | ((c & 0x0f00) >> 4)) << 12;
		g = ((c & 0x00f0) | ((c & 0x00f0) >> 4)) <<  8;
		b = ((c & 0x000f) | ((c & 0x000f) << 4));
		*buffer++ = a | r | g | b;
	}
}

static IFASTCALL void _ifetch_ABGR_4444(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b, a;
	while (pixel < endup) {
		c = *pixel++;
		a = ((c & 0xf000) | ((c & 0xf000) >> 4)) << 16;
		b = ((c & 0x0f00) | ((c & 0x0f00) >> 4)) >>  4;
		g = ((c & 0x00f0) | ((c & 0x00f0) >> 4)) <<  8;
		r = ((c & 0x000f) | ((c & 0x000f) << 4)) << 16;
		*buffer++ = a | r | g | b;
	}
}

static IFASTCALL void _ifetch_RGBA_4444(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b, a;
	while (pixel < endup) {
		c = *pixel++;
		r = ((c & 0xf000) | ((c & 0xf000) >> 4)) <<  8;
		g = ((c & 0x0f00) | ((c & 0x0f00) >> 4)) <<  4;
		b = ((c & 0x00f0) | ((c & 0x00f0) >> 4));
		a = ((c & 0x000f) | ((c & 0x000f) << 4)) << 24;
		*buffer++ = a | r | g | b;
	}
}

static IFASTCALL void _ifetch_BGRA_4444(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b, a;
	while (pixel < endup) {
		c = *pixel++;
		b = ((c & 0xf000) | ((c & 0xf000) >> 4)) >>  8;
		g = ((c & 0x0f00) | ((c & 0x0f00) >> 4)) <<  4;
		r = ((c & 0x00f0) | ((c & 0x00f0) >> 4)) << 16;
		a = ((c & 0x000f) | ((c & 0x000f) << 4)) << 24;
		*buffer++ = a | r | g | b;
	}
}

static IFASTCALL void _ifetch_ARGB_1555(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b, a;
	while (pixel < endup) {
		c = *pixel++;
		a = (IUINT32) ((IUINT8)(0 - ((c & 0x8000) >> 15))) << 24;
		r = ((c & 0x7c00) | ((c & 0x7000) >> 5)) << 9;
		g = ((c & 0x03e0) | ((c & 0x0380) >> 5)) << 6;
		b = ((c & 0x001c) | ((c & 0x001f) << 5)) >> 2;
		*buffer++ = a | r | g | b;
	}
}

static IFASTCALL void _ifetch_ABGR_1555(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b, a;
	while (pixel < endup) {
		c = *pixel++;
		a = (IUINT32) ((IUINT8)(0 - ((c & 0x8000) >> 15))) << 24;
		b = ((c & 0x7c00) | ((c & 0x7000) >> 5)) >> 7;
		g = ((c & 0x03e0) | ((c & 0x0380) >> 5)) << 6;
		r = ((c & 0x001c) | ((c & 0x001f) << 5)) << 14;
		*buffer++ = a | r | g | b;
	}
}

static IFASTCALL void _ifetch_RGBA_5551(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b, a;
	while (pixel < endup) {
		c = *pixel++;
		r = ((c & 0xf800) | ((c & 0xe000) >> 5)) <<  8;
		g = ((c & 0x07c0) | ((c & 0x0700) >> 5)) <<  5;
		b = ((c & 0x0038) | ((c & 0x003e) << 5)) >>  3;
		a = (IUINT32) ((IUINT8)(0 - (c & 0x1))) << 24;
		*buffer++ = a | r | g | b;
	}
}

static IFASTCALL void _ifetch_BGRA_5551(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT16 *pixel = (const IUINT16*)bits + x;
	const IUINT16 *endup = pixel + w;
	IUINT32 c, r, g, b, a;
	while (pixel < endup) {
		c = *pixel++;
		b = ((c & 0xf800) | ((c & 0xe000) >> 5)) >>  8;
		g = ((c & 0x07c0) | ((c & 0x0700) >> 5)) <<  5;
		r = ((c & 0x0038) | ((c & 0x003e) << 5)) << 13;
		a = (IUINT32) ((IUINT8)(0 - (c & 0x1))) << 24;
		*buffer++ = a | r | g | b;
	}
}

iFetchProc iFetchEntry[28] = 
{
	NULL,
	_ifetch_C8,
	_ifetch_RGB15,
	_ifetch_BGR15,
	_ifetch_RGB16,
	_ifetch_BGR16,
	_ifetch_RGB24,
	_ifetch_BGR24,
	_ifetch_RGB32,
	_ifetch_BGR32,
	_ifetch_ARGB32,
	_ifetch_ABGR32,
	_ifetch_RGBA32,
	_ifetch_BGRA32,
	_ifetch_ARGB_4444,
	_ifetch_ABGR_4444,
	_ifetch_RGBA_4444,
	_ifetch_BGRA_4444,
	_ifetch_ARGB_1555,
	_ifetch_ABGR_1555,
	_ifetch_RGBA_5551,
	_ifetch_BGRA_5551,
	_ifetch_A8,
	_ifetch_G8,
	_ifetch_A2R2G2B2,
	_ifetch_A2B2G2R2,
	_ifetch_R3G3B2,
	_ifetch_B2G3R3,
};


//---------------------------------------------------------------------
// get fetching procedure
//---------------------------------------------------------------------
iFetchProc iFetchGetProc(int fmt)
{
	switch (fmt)
	{
	case IPIX_FMT_8: return (iFetchProc)_ifetch_C8;
	case IPIX_FMT_RGB15: return (iFetchProc)_ifetch_RGB15;
	case IPIX_FMT_BGR15: return (iFetchProc)_ifetch_BGR15;
	case IPIX_FMT_RGB16: return (iFetchProc)_ifetch_RGB16;
	case IPIX_FMT_BGR16: return (iFetchProc)_ifetch_BGR16;
	case IPIX_FMT_RGB24: return (iFetchProc)_ifetch_RGB24;
	case IPIX_FMT_BGR24: return (iFetchProc)_ifetch_BGR24;
	case IPIX_FMT_RGB32: return (iFetchProc)_ifetch_RGB32;
	case IPIX_FMT_BGR32: return (iFetchProc)_ifetch_BGR32;

	case IPIX_FMT_ARGB32: return (iFetchProc)_ifetch_ARGB32;
	case IPIX_FMT_ABGR32: return (iFetchProc)_ifetch_ABGR32;
	case IPIX_FMT_RGBA32: return (iFetchProc)_ifetch_RGBA32;
	case IPIX_FMT_BGRA32: return (iFetchProc)_ifetch_BGRA32;

	case IPIX_FMT_ARGB_4444: return (iFetchProc)_ifetch_ARGB_4444;
	case IPIX_FMT_ABGR_4444: return (iFetchProc)_ifetch_ABGR_4444;
	case IPIX_FMT_RGBA_4444: return (iFetchProc)_ifetch_RGBA_4444;
	case IPIX_FMT_BGRA_4444: return (iFetchProc)_ifetch_BGRA_4444;

	case IPIX_FMT_ARGB_1555: return (iFetchProc)_ifetch_ARGB_1555;
	case IPIX_FMT_ABGR_1555: return (iFetchProc)_ifetch_ABGR_1555;
	case IPIX_FMT_RGBA_5551: return (iFetchProc)_ifetch_RGBA_5551;
	case IPIX_FMT_BGRA_5551: return (iFetchProc)_ifetch_BGRA_5551;
	}

	return NULL;
}


//=====================================================================
// COLOR STORING PROCEDURES
//=====================================================================

static IFASTCALL void _istore_A8(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x;
	int i;
	for (i = w; i > 0; i--) {
		*pixel++ = (IUINT8)((*values++) >> 24);
	}
}

static IFASTCALL void _istore_G8(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x;
	IUINT32 c, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_RGB(c, r, g, b);
		*pixel++ = (IUINT8)IRGB_TO_Y(r, g, b);
	}
}

static IFASTCALL void _istore_C8(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x;
	IUINT32 c;
	int i;
	for (i = w; i > 0; i--) {
#ifndef IDISABLE_INDEX
		c = *values++;
		*pixel++ = IRGB24_TO_ENTRY(idx, c);
#else
		IUINT32 r, g, b;
		c = *values++;
		ISPLIT_RGB(c, r, g, b);
		*pixel++ = (IUINT8)IRGB_TO_Y(r, g, b);
#endif
	}
}

static IFASTCALL void _istore_A2R2G2B2(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = (IUINT8) (((a     ) & 0xc0) |
							 ((r >> 2) & 0x30) |
							 ((g >> 4) & 0x0c) |
							 ((b >> 6)       ));
	}
}

static IFASTCALL void _istore_A2B2G2R2(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = (IUINT8) (((a     ) & 0xc0) |
							 ((b >> 2) & 0x30) |
							 ((g >> 4) & 0x0c) |
							 ((r >> 6)       ));
	}
}

static IFASTCALL void _istore_R3G3B2(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x;
	IUINT32 c, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_RGB(c, r, g, b);
		*pixel++ = (IUINT8) (((r     ) & 0xe0) |
							 ((g >> 3) & 0x1c) |
							 ((r >> 6)       ));
	}
}

static IFASTCALL void _istore_B2G3R3(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x;
	IUINT32 c, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_RGB(c, r, g, b);
		*pixel++ = (IUINT8) (((b     ) & 0xc0) |
							 ((g >> 2) & 0x38) |
							 ((r >> 5)       ));
	}
}

static IFASTCALL void _istore_RGB15(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_RGB(c, r, g, b);
		*pixel++ = (IUINT16) (((IUINT16)(r & 0xf8) << 7) |
							  ((IUINT16)(g & 0xf8) << 2) |
							  ((IUINT16)(b & 0xf8) >> 3));
	}
}

static IFASTCALL void _istore_BGR15(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_RGB(c, r, g, b);
		*pixel++ = (IUINT16) (((IUINT16)(b & 0xf8) << 7) |
							  ((IUINT16)(g & 0xf8) << 2) |
							  ((IUINT16)(r & 0xf8) >> 3));
	}
}

static IFASTCALL void _istore_RGB16(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		*pixel++ = (IUINT16) (((IUINT16)(c >> 3) & 0x001f) |
							  ((IUINT16)(c >> 5) & 0x07e0) |
							  ((IUINT16)(c >> 8) & 0xf800));
	}
}

static IFASTCALL void _istore_BGR16(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_RGB(c, r, g, b);
		*pixel++ = (IUINT16) (((IUINT16)(b & 0xf8) << 8) |
							  ((IUINT16)(g & 0xfc) << 3) |
							  ((IUINT16)(r & 0xf8) >> 3));
	}
}

static IFASTCALL void _istore_RGB24(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x * 3;
	IUINT32 c;
	int i;
	if (ILITTLE_ENDIAN) {
		for (i = w; i > 0; i--) {
			c = *values++;
			ILSB_STORE24(pixel, c);
			pixel += 3;
		}
	}	else {
		for (i = w; i > 0; i--) {
			c = *values++;
			IMSB_STORE24(pixel, c);
			pixel += 3;
		}
	}
}

static IFASTCALL void _istore_BGR24(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x * 3;
	IUINT32 c;
	int i;
	if (ILITTLE_ENDIAN) {
		for (i = w; i > 0; i--) {
			c = *values++;
			IMSB_STORE24(pixel, c);
			pixel += 3;
		}
	}	else {
		for (i = w; i > 0; i--) {
			c = *values++;
			ILSB_STORE24(pixel, c);
			pixel += 3;
		}
	}
}

static IFASTCALL void _istore_RGB32(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT32 *pixel = (IUINT32*)bits + x;
	int i;
	for (i = w; i > 0; i--) {
		*pixel++ = values[0] & 0xffffff;
		values++;
	}
}

static IFASTCALL void _istore_BGR32(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT32 *pixel = (IUINT32*)bits + x;
	int i;
	for (i = w; i > 0; i--) {
		*pixel++ = (values[0] & 0x0000ff00) |
			((values[0] >> 16) & 0xff) | ((values[0] & 0xff) << 16);
		values++;
	}
}

static IFASTCALL void _istore_ARGB32(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	memcpy(((IUINT32*)bits) + x, values, w * sizeof(IUINT32));
}

static IFASTCALL void _istore_ABGR32(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT32 *pixel = (IUINT32*)bits + x;
	int i;
	for (i = w; i > 0; i--) {
		*pixel++ = (values[0] & 0xff00ff00) |
			((values[0] >> 16) & 0xff) | ((values[0] & 0xff) << 16);
		values++;
	}
}

static IFASTCALL void _istore_RGBA32(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT32 *pixel = (IUINT32*)bits + x;
	int i;
	for (i = w; i > 0; i--) {
		*pixel++ = ((values[0] & 0xffffff) << 8) |
			((values[0] & 0xff000000) >> 24);
		values++;
	}
}

static IFASTCALL void _istore_BGRA32(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT32 *pixel = (IUINT32*)bits + x;
	int i;
	for (i = w; i > 0; i--) {
		*pixel++ =  ((values[0] & 0xff000000) >> 24) |
					((values[0] & 0x00ff0000) >>  8) |
					((values[0] & 0x0000ff00) <<  8) |
					((values[0] & 0x000000ff) << 24);
		values++;
	}
}

static IFASTCALL void _istore_ARGB_4444(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = ((IUINT16)( 
			((IUINT16)((a) & 0xf0) << 8) | 
			((IUINT16)((r) & 0xf0) << 4) | 
			((IUINT16)((g) & 0xf0) >> 0) | 
			((IUINT16)((b) & 0xf0) >> 4)));
	}
}

static IFASTCALL void _istore_ABGR_4444(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = ((IUINT16)( 
			((IUINT16)((a) & 0xf0) << 8) | 
			((IUINT16)((b) & 0xf0) << 4) | 
			((IUINT16)((g) & 0xf0) >> 0) | 
			((IUINT16)((r) & 0xf0) >> 4)));
	}
}

static IFASTCALL void _istore_RGBA_4444(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = ((IUINT16)( 
			((IUINT16)((r) & 0xf0) << 8) | 
			((IUINT16)((g) & 0xf0) << 4) | 
			((IUINT16)((b) & 0xf0) >> 0) | 
			((IUINT16)((a) & 0xf0) >> 4)));
	}
}

static IFASTCALL void _istore_BGRA_4444(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = ((IUINT16)( 
			((IUINT16)((b) & 0xf0) << 8) | 
			((IUINT16)((g) & 0xf0) << 4) | 
			((IUINT16)((r) & 0xf0) >> 0) | 
			((IUINT16)((a) & 0xf0) >> 4)));
	}
}


static IFASTCALL void _istore_ARGB_1555(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = ((IUINT16)( 
			((IUINT16)((a) & 0x80) << 8) |
			((IUINT16)((r) & 0xf8) << 7) | 
			((IUINT16)((g) & 0xf8) << 2) | 
			((IUINT16)((b) & 0xf8) >> 3)));
	}
}

static IFASTCALL void _istore_ABGR_1555(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = ((IUINT16)( 
			((IUINT16)((a) & 0x80) << 8) |
			((IUINT16)((b) & 0xf8) << 7) | 
			((IUINT16)((g) & 0xf8) << 2) | 
			((IUINT16)((r) & 0xf8) >> 3)));
	}
}

static IFASTCALL void _istore_RGBA_5551(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = ((IUINT16)( 
			((IUINT16)((r) & 0xf8) << 7) | 
			((IUINT16)((g) & 0xf8) << 2) | 
			((IUINT16)((b) & 0xf8) >> 3)) << 1) | 
			((a) >> 7);
	}
}

static IFASTCALL void _istore_BGRA_5551(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = ((IUINT16)( 
			((IUINT16)((b) & 0xf8) << 7) | 
			((IUINT16)((g) & 0xf8) << 2) | 
			((IUINT16)((r) & 0xf8) >> 3)) << 1) | 
			((a) >> 7);
	}
}

iStoreProc iStoreEntry[28] = 
{
	NULL,
	_istore_C8,
	_istore_RGB15,
	_istore_BGR15,
	_istore_RGB16,
	_istore_BGR16,
	_istore_RGB24,
	_istore_BGR24,
	_istore_RGB32,
	_istore_BGR32,
	_istore_ARGB32,
	_istore_ABGR32,
	_istore_RGBA32,
	_istore_BGRA32,
	_istore_ARGB_4444,
	_istore_ABGR_4444,
	_istore_RGBA_4444,
	_istore_BGRA_4444,
	_istore_ARGB_1555,
	_istore_ABGR_1555,
	_istore_RGBA_5551,
	_istore_BGRA_5551,
	_istore_A8,
	_istore_G8,
	_istore_A2R2G2B2,
	_istore_A2B2G2R2,
	_istore_R3G3B2,
	_istore_B2G3R3,
};


//---------------------------------------------------------------------
// get fetching procedure
//---------------------------------------------------------------------
iStoreProc iStoreGetProc(int fmt)
{
	switch (fmt)
	{
	case IPIX_FMT_8: return (iStoreProc)_istore_C8;
	case IPIX_FMT_RGB15: return (iStoreProc)_istore_RGB15;
	case IPIX_FMT_BGR15: return (iStoreProc)_istore_BGR15;
	case IPIX_FMT_RGB16: return (iStoreProc)_istore_RGB16;
	case IPIX_FMT_BGR16: return (iStoreProc)_istore_BGR16;
	case IPIX_FMT_RGB24: return (iStoreProc)_istore_RGB24;
	case IPIX_FMT_BGR24: return (iStoreProc)_istore_BGR24;
	case IPIX_FMT_RGB32: return (iStoreProc)_istore_RGB32;
	case IPIX_FMT_BGR32: return (iStoreProc)_istore_BGR32;

	case IPIX_FMT_ARGB32: return (iStoreProc)_istore_ARGB32;
	case IPIX_FMT_ABGR32: return (iStoreProc)_istore_ABGR32;
	case IPIX_FMT_RGBA32: return (iStoreProc)_istore_RGBA32;
	case IPIX_FMT_BGRA32: return (iStoreProc)_istore_BGRA32;

	case IPIX_FMT_ARGB_4444: return (iStoreProc)_istore_ARGB_4444;
	case IPIX_FMT_ABGR_4444: return (iStoreProc)_istore_ABGR_4444;
	case IPIX_FMT_RGBA_4444: return (iStoreProc)_istore_RGBA_4444;
	case IPIX_FMT_BGRA_4444: return (iStoreProc)_istore_BGRA_4444;

	case IPIX_FMT_ARGB_1555: return (iStoreProc)_istore_ARGB_1555;
	case IPIX_FMT_ABGR_1555: return (iStoreProc)_istore_ABGR_1555;
	case IPIX_FMT_RGBA_5551: return (iStoreProc)_istore_RGBA_5551;
	case IPIX_FMT_BGRA_5551: return (iStoreProc)_istore_BGRA_5551;
	}
	return NULL;
}



//=====================================================================
// PIXEL FETCHING PROCEDURES
//=====================================================================

static IFASTCALL IUINT32 _ifetch_pixel_A8(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT8*)bits)[offset];
	return c << 24;
}

static IFASTCALL IUINT32 _ifetch_pixel_G8(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT8*)bits)[offset];
	return 0xff000000 | c | (c << 16) | (c << 24);
}

static IFASTCALL IUINT32 _ifetch_pixel_C8(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT8*)bits)[offset];
#ifndef IDISABLE_INDEX
	return idx->rgba[c];
#else
	return 0xff000000 | c | (c << 16) | (c << 24);
#endif
}

static IFASTCALL IUINT32 _ifetch_pixel_A2R2G2B2(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT8*)bits)[offset];
	IUINT32 a, r, g, b;
	a = ((c & 0xc0) * 0x55) << 18;
	r = ((c & 0x30) * 0x55) << 12;
	g = ((c & 0x0c) * 0x55) <<  6;
	b = ((c & 0x03) * 0x55);
	return (a | r | g | b);
}

static IFASTCALL IUINT32 _ifetch_pixel_A2B2G2R2(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT8*)bits)[offset];
	IUINT32 a, r, g, b;
	a = ((c & 0xc0) * 0x55) << 18;
	b = ((c & 0x30) * 0x55) >>  6;
	g = ((c & 0x0c) * 0x55) <<  6;
	r = ((c & 0x03) * 0x55) << 16;
	return (a | r | g | b);
}

static IFASTCALL IUINT32 _ifetch_pixel_R3G3B2(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT8*)bits)[offset];
	IUINT32 r, g, b;
	r = ((c & 0xe0) | ((c & 0xe0) >> 3) | ((c & 0xc0) >> 6)) << 16;
	g = ((c & 0x1c) | ((c & 0x18) >> 3) | ((c & 0x1c) << 3)) << 8;
	b = (((c & 3)) | ((c & 3) << 2) | ((c & 3) << 4) | ((c & 3) << 6));
	return 0xff000000 | r | g | b;
}

static IFASTCALL IUINT32 _ifetch_pixel_B2G3R3(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT8*)bits)[offset];
	IUINT32 r, g, b;
	b = (((c & 0xc0)     ) |
		 ((c & 0xc0) >> 2) |
		 ((c & 0xc0) >> 4) |
		 ((c & 0xc0) >> 6));
	g = ((c & 0x38) | ((c & 0x38) >> 3) | ((c & 30) << 2)) << 8;
	r = (((c & 0x07)     ) |
		 ((c & 0x07) << 3) |
		 ((c & 0x06) << 6)) << 16;
	return 0xff000000 | r | g | b;
}

static IFASTCALL IUINT32 _ifetch_pixel_RGB15(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)bits)[offset];
	IUINT32 r, g, b;
	r = ((c & 0x7c00) | ((c & 0x7000) >> 5)) << 9;
	g = ((c & 0x03e0) | ((c & 0x0380) >> 5)) << 6;
	b = ((c & 0x001c) | ((c & 0x001f) << 5)) >> 2;
	return 0xff000000 | r | g | b;
}

static IFASTCALL IUINT32 _ifetch_pixel_BGR15(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)bits)[offset];
	IUINT32 r, g, b;
	b = ((c & 0x7c00) | ((c & 0x7000) >> 5)) >> 7;
	g = ((c & 0x03e0) | ((c & 0x0380) >> 5)) << 6;
	r = ((c & 0x001c) | ((c & 0x001f) << 5)) << 14;
	return 0xff000000 | r | g | b;
}

static IFASTCALL IUINT32 _ifetch_pixel_RGB16(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)bits)[offset];
	IUINT32 r;
	r = (((c) << 3) & 0xf8) |
		(((c) << 5) & 0xfc00) |
		(((c) << 8) & 0xf80000);
	r |= (r >> 5) & 0x70007;
	r |= (r >> 6) & 0x300;
	return 0xff000000 | r;
}

static IFASTCALL IUINT32 _ifetch_pixel_BGR16(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)bits)[offset];
	IUINT32 r, g, b;
	b = ((c & 0xf800) | ((c & 0xe000) >> 5)) >> 8;
	g = ((c & 0x07e0) | ((c & 0x0600) >> 6)) << 5;
	r = ((c & 0x001c) | ((c & 0x001f) << 5)) << 14;
	return 0xff000000 | r | g | b;
}

static IFASTCALL IUINT32 _ifetch_pixel_RGB24(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + offset * 3;
	if (ILITTLE_ENDIAN) return ILSB_FETCH24(pixel) | 0xff000000;
	return IMSB_FETCH24(pixel) | 0xff000000;
}

static IFASTCALL IUINT32 _ifetch_pixel_BGR24(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + offset * 3;
	if (ILITTLE_ENDIAN) return IMSB_FETCH24(pixel) | 0xff000000;
	return ILSB_FETCH24(pixel) | 0xff000000;
}

static IFASTCALL IUINT32 _ifetch_pixel_RGB32(const void *bits,
	int offset, const iColorIndex *idx)
{
	return ((IUINT32*)(bits))[offset] | 0xff000000;
}

static IFASTCALL IUINT32 _ifetch_pixel_BGR32(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 pixel = ((IUINT32*)(bits))[offset];
	return ((pixel & 0x0000ff00) | ((pixel >> 16) & 0xff) | 
		((pixel & 0xff) << 16)) | 0xff000000;
}

static IFASTCALL IUINT32 _ifetch_pixel_ARGB32(const void *bits,
	int offset, const iColorIndex *idx)
{
	return ((IUINT32*)(bits))[offset];
}

static IFASTCALL IUINT32 _ifetch_pixel_ABGR32(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 pixel = ((IUINT32*)(bits))[offset];
	return ((pixel & 0xff00ff00) | ((pixel >> 16) & 0xff) | 
		((pixel & 0xff) << 16));
}

static IFASTCALL IUINT32 _ifetch_pixel_RGBA32(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 pixel = ((IUINT32*)(bits))[offset];
	return ((pixel & 0xff) << 24) | ((pixel >> 8) & 0xffffff);
}

static IFASTCALL IUINT32 _ifetch_pixel_BGRA32(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 pixel = ((IUINT32*)(bits))[offset];
	return		((pixel & 0x000000ff) << 24) |
				((pixel & 0x0000ff00) <<  8) |
				((pixel & 0x00ff0000) >>  8) |
				((pixel & 0xff000000) >> 24);
}

static IFASTCALL IUINT32 _ifetch_pixel_ARGB_4444(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)(bits))[offset];
	IUINT32 r, g, b, a;
	a = ((c & 0xf000) | ((c & 0xf000) >> 4)) << 16;
	r = ((c & 0x0f00) | ((c & 0x0f00) >> 4)) << 12;
	g = ((c & 0x00f0) | ((c & 0x00f0) >> 4)) <<  8;
	b = ((c & 0x000f) | ((c & 0x000f) << 4));
	return (a | r | g | b);
}


static IFASTCALL IUINT32 _ifetch_pixel_ABGR_4444(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)(bits))[offset];
	IUINT32 r, g, b, a;
	a = ((c & 0xf000) | ((c & 0xf000) >> 4)) << 16;
	b = ((c & 0x0f00) | ((c & 0x0f00) >> 4)) >>  4;
	g = ((c & 0x00f0) | ((c & 0x00f0) >> 4)) <<  8;
	r = ((c & 0x000f) | ((c & 0x000f) << 4)) << 16;
	return (a | r | g | b);
}


static IFASTCALL IUINT32 _ifetch_pixel_RGBA_4444(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)(bits))[offset];
	IUINT32 r, g, b, a;
	r = ((c & 0xf000) | ((c & 0xf000) >> 4)) <<  8;
	g = ((c & 0x0f00) | ((c & 0x0f00) >> 4)) <<  4;
	b = ((c & 0x00f0) | ((c & 0x00f0) >> 4));
	a = ((c & 0x000f) | ((c & 0x000f) << 4)) << 24;
	return (a | r | g | b);
}


static IFASTCALL IUINT32 _ifetch_pixel_BGRA_4444(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)(bits))[offset];
	IUINT32 r, g, b, a;
	b = ((c & 0xf000) | ((c & 0xf000) >> 4)) >>  8;
	g = ((c & 0x0f00) | ((c & 0x0f00) >> 4)) <<  4;
	r = ((c & 0x00f0) | ((c & 0x00f0) >> 4)) << 16;
	a = ((c & 0x000f) | ((c & 0x000f) << 4)) << 24;
	return (a | r | g | b);
}

static IFASTCALL IUINT32 _ifetch_pixel_ARGB_1555(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)(bits))[offset];
	IUINT32 r, g, b, a;
	a = (IUINT32) ((IUINT8)(0 - ((c & 0x8000) >> 15))) << 24;
	r = ((c & 0x7c00) | ((c & 0x7000) >> 5)) << 9;
	g = ((c & 0x03e0) | ((c & 0x0380) >> 5)) << 6;
	b = ((c & 0x001c) | ((c & 0x001f) << 5)) >> 2;
	return (a | r | g | b);
}


static IFASTCALL IUINT32 _ifetch_pixel_ABGR_1555(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)(bits))[offset];
	IUINT32 r, g, b, a;
	a = (IUINT32) ((IUINT8)(0 - ((c & 0x8000) >> 15))) << 24;
	b = ((c & 0x7c00) | ((c & 0x7000) >> 5)) >> 7;
	g = ((c & 0x03e0) | ((c & 0x0380) >> 5)) << 6;
	r = ((c & 0x001c) | ((c & 0x001f) << 5)) << 14;
	return (a | r | g | b);
}


static IFASTCALL IUINT32 _ifetch_pixel_RGBA_5551(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)(bits))[offset];
	IUINT32 r, g, b, a;
	r = ((c & 0xf800) | ((c & 0xe000) >> 5)) <<  8;
	g = ((c & 0x07c0) | ((c & 0x0700) >> 5)) <<  5;
	b = ((c & 0x0038) | ((c & 0x003e) << 5)) >>  3;
	a = (IUINT32) ((IUINT8)(0 - (c & 0x1))) << 24;
	return (a | r | g | b);
}


static IFASTCALL IUINT32 _ifetch_pixel_BGRA_5551(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 c = ((IUINT16*)(bits))[offset];
	IUINT32 r, g, b, a;
	b = ((c & 0xf800) | ((c & 0xe000) >> 5)) >>  8;
	g = ((c & 0x07c0) | ((c & 0x0700) >> 5)) <<  5;
	r = ((c & 0x0038) | ((c & 0x003e) << 5)) << 13;
	a = (IUINT32) ((IUINT8)(0 - (c & 0x1))) << 24;
	return (a | r | g | b);
}


iFetchPixelProc iFetchPixelEntry[28] = 
{
	_ifetch_pixel_C8,
	_ifetch_pixel_RGB15,
	_ifetch_pixel_BGR15,
	_ifetch_pixel_RGB16,
	_ifetch_pixel_BGR16,
	_ifetch_pixel_RGB24,
	_ifetch_pixel_BGR24,
	_ifetch_pixel_RGB32,
	_ifetch_pixel_BGR32,
	_ifetch_pixel_ARGB32,
	_ifetch_pixel_ABGR32,
	_ifetch_pixel_RGBA32,
	_ifetch_pixel_BGRA32,
	_ifetch_pixel_ARGB_4444,
	_ifetch_pixel_ABGR_4444,
	_ifetch_pixel_RGBA_4444,
	_ifetch_pixel_BGRA_4444,
	_ifetch_pixel_ARGB_1555,
	_ifetch_pixel_ABGR_1555,
	_ifetch_pixel_RGBA_5551,
	_ifetch_pixel_BGRA_5551,
	_ifetch_pixel_A8,
	_ifetch_pixel_G8,
	_ifetch_pixel_A2R2G2B2,
	_ifetch_pixel_A2B2G2R2,
	_ifetch_pixel_R3G3B2,
	_ifetch_pixel_B2G3R3,
};


//---------------------------------------------------------------------
// get pixel fetching procedure
//---------------------------------------------------------------------
iFetchPixelProc iFetchPixelGetProc(int fmt)
{
	switch (fmt)
	{
	case IPIX_FMT_8: return (iFetchPixelProc)_ifetch_pixel_C8;
	case IPIX_FMT_RGB15: return (iFetchPixelProc)_ifetch_pixel_RGB15;
	case IPIX_FMT_BGR15: return (iFetchPixelProc)_ifetch_pixel_BGR15;
	case IPIX_FMT_RGB16: return (iFetchPixelProc)_ifetch_pixel_RGB16;
	case IPIX_FMT_BGR16: return (iFetchPixelProc)_ifetch_pixel_BGR16;
	case IPIX_FMT_RGB24: return (iFetchPixelProc)_ifetch_pixel_RGB24;
	case IPIX_FMT_BGR24: return (iFetchPixelProc)_ifetch_pixel_BGR24;
	case IPIX_FMT_RGB32: return (iFetchPixelProc)_ifetch_pixel_RGB32;
	case IPIX_FMT_BGR32: return (iFetchPixelProc)_ifetch_pixel_BGR32;

	case IPIX_FMT_ARGB32: return (iFetchPixelProc)_ifetch_pixel_ARGB32;
	case IPIX_FMT_ABGR32: return (iFetchPixelProc)_ifetch_pixel_ABGR32;
	case IPIX_FMT_RGBA32: return (iFetchPixelProc)_ifetch_pixel_RGBA32;
	case IPIX_FMT_BGRA32: return (iFetchPixelProc)_ifetch_pixel_BGRA32;

	case IPIX_FMT_ARGB_4444: return (iFetchPixelProc)_ifetch_pixel_ARGB_4444;
	case IPIX_FMT_ABGR_4444: return (iFetchPixelProc)_ifetch_pixel_ABGR_4444;
	case IPIX_FMT_RGBA_4444: return (iFetchPixelProc)_ifetch_pixel_RGBA_4444;
	case IPIX_FMT_BGRA_4444: return (iFetchPixelProc)_ifetch_pixel_BGRA_4444;

	case IPIX_FMT_ARGB_1555: return (iFetchPixelProc)_ifetch_pixel_ARGB_1555;
	case IPIX_FMT_ABGR_1555: return (iFetchPixelProc)_ifetch_pixel_ABGR_1555;
	case IPIX_FMT_RGBA_5551: return (iFetchPixelProc)_ifetch_pixel_RGBA_5551;
	case IPIX_FMT_BGRA_5551: return (iFetchPixelProc)_ifetch_pixel_BGRA_5551;
	}

	return NULL;
}

