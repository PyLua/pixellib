//=====================================================================
// 
// ibmspan.c - 
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================
#include "ibmspan.h"

//---------------------------------------------------------------------
// inline definition
//---------------------------------------------------------------------
#ifndef INLINE
#ifdef __GNUC__

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif

#elif (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#ifndef inline
#define inline INLINE
#endif


//=====================================================================
// 扫描线填充部分
//=====================================================================

// 扫描线驱动
ISPAN_PROC ispan_proc[24][8] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};

//---------------------------------------------------------------------
// NORMAL FILL
//---------------------------------------------------------------------
#define ISPAN_FLAT_LOOP(ptr, w, col, fmt, dsize, init, w1, w2, quit) { \
		ICOLORD c1, c2, r1, g1, b1, a1, r2, g2, b2, a2; \
		ICOLORD r, g, b, a, ca1, ca2; \
		IRGBA_FROM_PIXEL(col, ARGB32, r, g, b, a); \
		init; \
		ILINS_LOOP_DOUBLE( \
			{ \
				w1; \
				_ipixel_put(dsize, ptr, c1); \
				ptr += dsize; \
			}, \
			{ \
				w2; \
				_ipixel_put(dsize, ptr, c1); \
				ptr += dsize; \
				_ipixel_put(dsize, ptr, c2); \
				ptr += dsize; \
			}, \
			w); \
		quit; \
	}

// NORMAL without cover and without blend
#define ISPAN_FLAT_FILL(ptr, w, col, fmt, dsize) { \
		ISPAN_FLAT_LOOP(ptr, w, col, fmt, dsize, \
			{ \
				c1 = IRGBA_TO_PIXEL(fmt, r, g, b, a); \
				c2 = c1; \
			}, \
			{ \
			}, \
			{ \
			}, \
			{ \
				r1 = g1 = b1 = r2 = g2 = b2 = a1 = a2 = ca1 = ca2 = 0; \
				r1 = r1; g1 = g1; b1 = b1; a1 = a1; \
				r2 = r2; g2 = g2; b2 = b2; a1 = a2; \
				ca1 = ca1; ca2 = ca2; \
			} \
		); \
	}

// NORMAL without cover and with blend
#define ISPAN_FLAT_BLEND(ptr, w, col, fmt, dsize, mode) { \
		ISPAN_FLAT_LOOP(ptr, w, col, fmt, dsize, \
			{ \
			}, \
			{ \
				c1 = _ipixel_get(dsize, ptr); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				IBLEND_##mode(r, g, b, a, r1, g1, b1, a1); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			}, \
			{ \
				c1 = _ipixel_get(dsize, ptr); \
				c2 = _ipixel_get(dsize, ptr + dsize); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				IRGBA_FROM_PIXEL(c2, fmt, r2, g2, b2, a2); \
				IBLEND_##mode(r, g, b, a, r1, g1, b1, a1); \
				IBLEND_##mode(r, g, b, a, r2, g2, b2, a2); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
				c2 = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
			}, \
			{ \
				ca1 = ca2 = 0; ca1 = ca1; ca2 = ca2; \
			} \
		); \
	}

// NORMAL with cover and without blend
#define ISPAN_FLAT_C_FILL(ptr, w, col, fmt, dsize, mode) { \
		ISPAN_FLAT_LOOP(ptr, w, col, fmt, dsize, \
			{ \
			}, \
			{ \
				c1 = _ipixel_get(dsize, ptr); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				ca1 = *cover++; \
				IBLEND_##mode(r, g, b, ca1, r1, g1, b1, a1); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			}, \
			{ \
				c1 = _ipixel_get(dsize, ptr); \
				c2 = _ipixel_get(dsize, ptr + dsize); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				IRGBA_FROM_PIXEL(c2, fmt, r2, g2, b2, a2); \
				ca1 = *cover++; \
				ca2 = *cover++; \
				IBLEND_##mode(r, g, b, ca1, r1, g1, b1, a1); \
				IBLEND_##mode(r, g, b, ca2, r2, g2, b2, a2); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
				c2 = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
			}, \
			{ \
			} \
		); \
	}

// NORMAL with cover and with blend
#define ISPAN_FLAT_C_BLEND(ptr, w, col, fmt, dsize, mode) { \
		ISPAN_FLAT_LOOP(ptr, w, col, fmt, dsize, \
			{ \
			}, \
			{ \
				c1 = _ipixel_get(dsize, ptr); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				ca1 = ((*cover++) * a) >> 8; \
				IBLEND_##mode(r, g, b, ca1, r1, g1, b1, a1); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			}, \
			{ \
				c1 = _ipixel_get(dsize, ptr); \
				c2 = _ipixel_get(dsize, ptr + dsize); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				IRGBA_FROM_PIXEL(c2, fmt, r2, g2, b2, a2); \
				ca1 = ((*cover++) * a) >> 8; \
				ca2 = ((*cover++) * a) >> 8; \
				IBLEND_##mode(r, g, b, ca1, r1, g1, b1, a1); \
				IBLEND_##mode(r, g, b, ca2, r2, g2, b2, a2); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
				c2 = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
			}, \
			{ \
			} \
		); \
	}


//---------------------------------------------------------------------
// GOURAUD FILL
//---------------------------------------------------------------------
#define ISPAN_GOURAUD_LOOP(ptr, w, cs, ce, fmt, dsize, work) { \
		int c, r, g, b, a, dr, dg, db, da, cc, cr, cg, cb, ca; \
		int c1, c2, r1, g1, b1, a1, r2, g2, b2, a2; \
		IRGBA_FROM_PIXEL(cs, ARGB32, r1, g1, b1, a1); \
		IRGBA_FROM_PIXEL(ce, ARGB32, r2, g2, b2, a2); \
		cr = r1 << 16; cg = g1 << 16; cb = b1 << 16; ca = a1 << 16; \
		dr = ((r2 - r1) << 16) / w; \
		dg = ((g2 - g1) << 16) / w; \
		db = ((b2 - b1) << 16) / w; \
		da = ((a2 - a1) << 16) / w; \
		for (; w > 0; w--, ptr += dsize) { \
			r = cr >> 16; \
			g = cg >> 16; \
			b = cb >> 16; \
			a = ca >> 16; \
			work; \
			_ipixel_put(dsize, ptr, c); \
			cr += dr; \
			cg += dg; \
			cb += db; \
			ca += da; \
		} \
		cc = c1 = c2 = 0; cc = cc; c1 = c1; c2 = c2; \
	}

// GOURAUD without cover and without blend
#define ISPAN_GOURAUD_FILL(ptr, w, cs, ce, fmt, dsize) { \
		ISPAN_GOURAUD_LOOP(ptr, w, cs, ce, fmt, dsize, \
			{ \
				c = IRGBA_TO_PIXEL(fmt, r, g, b, a); \
			} \
		); \
	}

// GOURAUD without cover and with blend
#define ISPAN_GOURAUD_BLEND(ptr, w, cs, ce, fmt, dsize, mode) { \
		ISPAN_GOURAUD_LOOP(ptr, w, cs, ce, fmt, dsize, \
			{ \
				c1 = _ipixel_get(dsize, ptr); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				IBLEND_##mode(r, g, b, a, r1, g1, b1, a1); \
				c = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			} \
		); \
	}

// GOURAUD with cover and without blend
#define ISPAN_GOURAUD_C_FILL(ptr, w, cs, ce, fmt, dsize, mode) { \
		ISPAN_GOURAUD_LOOP(ptr, w, cs, ce, fmt, dsize, \
			{ \
				c1 = _ipixel_get(dsize, ptr); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				a = *cover++; \
				IBLEND_##mode(r, g, b, a, r1, g1, b1, a1); \
				c = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			} \
		); \
	}

// GOURAUD with cover and with blend
#define ISPAN_GOURAUD_C_BLEND(ptr, w, cs, ce, fmt, dsize, mode) { \
		ISPAN_GOURAUD_LOOP(ptr, w, cs, ce, fmt, dsize, \
			{ \
				c1 = _ipixel_get(dsize, ptr); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				a = ((*cover++) * a) >> 8; \
				IBLEND_##mode(r, g, b, a, r1, g1, b1, a1); \
				c = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			} \
		); \
	}


//---------------------------------------------------------------------
// MAIN SPAN FILL ROUTINE
//---------------------------------------------------------------------
#define ISPAN_FILL_MAIN(ptr, w, cs, ce, cover, fmt, dsize, mode, add) { \
	int pixfmt = IPIX_FMT_8; \
	int retval = -1; \
	ISPAN_PROC proc; \
	if (cs == ce) { \
		if (cover == NULL) { \
			if ((cs >> 24) == 255 && add == 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_FILL]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, w, cs, cs, cover, pixfmt, add); \
				if (retval != 0) \
					ISPAN_FLAT_FILL(ptr, w, cs, fmt, dsize); \
			} \
			else if ((cs >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_BLEND]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, w, cs, cs, cover, pixfmt, add); \
				if (retval != 0) \
					ISPAN_FLAT_BLEND(ptr, w, cs, fmt, dsize, mode); \
			} \
		}	else { \
			if ((cs >> 24) == 255 && add == 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COVER_FILL]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, w, cs, cs, cover, pixfmt, add); \
				if (retval != 0) \
					ISPAN_FLAT_C_FILL(ptr, w, cs, fmt, dsize, mode); \
			} \
			else if ((cs >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COVER_BLEND]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, w, cs, cs, cover, pixfmt, add); \
				if (retval != 0) \
					ISPAN_FLAT_C_BLEND(ptr, w, cs, fmt, dsize, mode); \
			} \
		} \
	}	else { \
		if (cover == NULL) { \
			if ((cs >> 24) == 255 && (ce >> 24) == 255 && add == 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_GOURAUD_FILL]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, w, cs, cs, cover, pixfmt, add); \
				if (retval != 0) \
					ISPAN_GOURAUD_FILL(ptr, w, cs, ce, fmt, dsize); \
			}	\
			else if ((cs >> 24) != 0 || (ce >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_GOURAUD_BLEND]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, w, cs, cs, cover, pixfmt, add); \
				if (retval != 0) \
					ISPAN_GOURAUD_BLEND(ptr, w, cs, ce, fmt, dsize, mode); \
			} \
		}	else { \
			if ((cs >> 24) == 255 && (ce >> 24) == 255 && add == 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_GOURAUD_COVER_FILL]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, w, cs, cs, cover, pixfmt, add); \
				if (retval != 0) \
					ISPAN_GOURAUD_C_FILL(ptr, w, cs, ce, fmt, dsize, mode); \
			} \
			else if ((cs >> 24) != 0 || (ce >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_GOURAUD_COVER_BLEND]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, w, cs, cs, cover, pixfmt, add); \
				if (retval != 0) \
					ISPAN_GOURAUD_C_BLEND(ptr, w, cs, ce, fmt, dsize, mode);\
			} \
		} \
	} \
}

#define ISPAN_FILL_DEBUG(ptr, w, cs, ce, cover, fmt, dsize, mode, add) { \
	int pixfmt = IPIX_FMT_##fmt; \
	int retval = -1; \
	ISPAN_PROC proc; \
	proc = ispan_proc[pixfmt][ISPAN_MODE_GOURAUD_FILL]; \
	ISPAN_GOURAUD_C_FILL(ptr, w, cs, ce, fmt, dsize, mode); \
}

#define ISPAN_FILL_ENTRY(fmt, dsize, mode) \
static inline void _ispan_fill_##fmt(unsigned char *ptr, int w, \
	ICOLORD cs, ICOLORD ce, const unsigned char *cover, int additive, \
	int noextra) \
{ \
	if (additive == 0) { \
		ISPAN_FILL_MAIN(ptr, w, cs, ce, cover, fmt, dsize, mode, 0); \
	}	else { \
		ISPAN_FILL_MAIN(ptr, w, cs, ce, cover, fmt, dsize, ADDITIVE, 1); \
	} \
} \
static void _ispan_draw_##fmt(IBITMAP *dst, const ISPAN *spans, int count, \
	int additive, int noextra) \
{ \
	for (; count > 0; count--, spans++) { \
		unsigned char *ptr = _ilineptr(dst, spans->y) + spans->x * dsize; \
		const unsigned char *cover = spans->cover; \
		ICOLORD cs = spans->color1; \
		ICOLORD ce = spans->color2; \
		int w = spans->w; \
		if (w > 0) { \
			_ispan_fill_##fmt(ptr, w, cs, ce, cover, additive, noextra); \
		} \
	} \
}


#ifndef TINYDEBUG
ISPAN_FILL_ENTRY(8, 1, STATIC);
ISPAN_FILL_ENTRY(RGB15, 2, STATIC);
ISPAN_FILL_ENTRY(BGR15, 2, STATIC);
ISPAN_FILL_ENTRY(RGB16, 2, STATIC);
ISPAN_FILL_ENTRY(BGR16, 2, STATIC);
ISPAN_FILL_ENTRY(RGB24, 3, STATIC);
ISPAN_FILL_ENTRY(BGR24, 3, STATIC);
ISPAN_FILL_ENTRY(RGB32, 4, STATIC);
ISPAN_FILL_ENTRY(BGR32, 4, STATIC);
ISPAN_FILL_ENTRY(ARGB32, 4, NORMAL);
ISPAN_FILL_ENTRY(ABGR32, 4, NORMAL);
ISPAN_FILL_ENTRY(RGBA32, 4, NORMAL);
ISPAN_FILL_ENTRY(BGRA32, 4, NORMAL);
ISPAN_FILL_ENTRY(ARGB_4444, 2, NORMAL);
ISPAN_FILL_ENTRY(ABGR_4444, 2, NORMAL);
ISPAN_FILL_ENTRY(RGBA_4444, 2, NORMAL);
ISPAN_FILL_ENTRY(BGRA_4444, 2, NORMAL);
ISPAN_FILL_ENTRY(ARGB_1555, 2, NORMAL);
ISPAN_FILL_ENTRY(ABGR_1555, 2, NORMAL);
ISPAN_FILL_ENTRY(RGBA_5551, 2, NORMAL);
ISPAN_FILL_ENTRY(BGRA_5551, 2, NORMAL);
#else
ISPAN_FILL_ENTRY(ARGB32, 4, NORMAL);
#endif


//---------------------------------------------------------------------
// draw a span
//---------------------------------------------------------------------
void ispan_draw(IBITMAP *dst, const ISPAN *spans, int count, int flags)
{
	int additive = (flags & ISPAN_FLAG_ADDITIVE)? 1 : 0;
	int noextra = (flags & ISPAN_FLAG_DEFAULT)? 1 : 0;
	int fmt;

	if (_ibitmap_pixfmt(dst) == 0) ibitmap_set_pixfmt(dst, 0);
	fmt = _ibitmap_pixfmt(dst);

	#define ispan_draw_case(fmt) \
		case IPIX_FMT_##fmt: \
			_ispan_draw_##fmt(dst, spans, count, additive, noextra); \
			break;

	switch (fmt)
	{
	#ifndef TINYDEBUG
		ispan_draw_case(8);
		ispan_draw_case(RGB15);
		ispan_draw_case(BGR15);
		ispan_draw_case(RGB16);
		ispan_draw_case(BGR16);
		ispan_draw_case(RGB24);
		ispan_draw_case(BGR24);
		ispan_draw_case(RGB32);
		ispan_draw_case(BGR32);
		ispan_draw_case(ARGB32);
		ispan_draw_case(ABGR32);
		ispan_draw_case(RGBA32);
		ispan_draw_case(BGRA32);
		ispan_draw_case(ARGB_4444);
		ispan_draw_case(ABGR_4444);
		ispan_draw_case(RGBA_4444);
		ispan_draw_case(BGRA_4444);
		ispan_draw_case(ARGB_1555);
		ispan_draw_case(ABGR_1555);
		ispan_draw_case(RGBA_5551);
		ispan_draw_case(BGRA_5551);
	#else
		ispan_draw_case(ARGB32);
	#endif
	}

	#undef ispan_draw_case
}




