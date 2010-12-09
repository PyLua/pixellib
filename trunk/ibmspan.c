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
ISPAN_PROC ispan_proc[24][16] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
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
				a = _iblend_norm_fast(a); \
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
		IINT32 c, r, g, b, a, dr, dg, db, da, cc, cr, cg, cb, ca; \
		IINT32 c1, c2, r1, g1, b1, a1, r2, g2, b2, a2, dd; \
		IRGBA_FROM_PIXEL(cs, ARGB32, r1, g1, b1, a1); \
		IRGBA_FROM_PIXEL(ce, ARGB32, r2, g2, b2, a2); \
		cr = r1 << 16; cg = g1 << 16; cb = b1 << 16; ca = a1 << 16; \
		dd = (w < 2)? 1 : (w - 1); \
		dr = ((r2 - r1) << 16) / dd; \
		dg = ((g2 - g1) << 16) / dd; \
		db = ((b2 - b1) << 16) / dd; \
		da = ((a2 - a1) << 16) / dd; \
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
				a = _iblend_norm_fast(a); \
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
	int pixfmt = IPIX_FMT_##fmt; \
	int retval = -1; \
	ISPAN_PROC proc; \
	alpha = ((cs >> 24) != 255) || ((ce >> 24) != 255); \
	if (cs == ce) { \
		if (cover == NULL) { \
			if (alpha == 0 && add == 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_FILL]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) \
					ISPAN_FLAT_FILL(ptr, w, cs, fmt, dsize); \
			} \
			else if ((cs >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_BLEND]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) \
					ISPAN_FLAT_BLEND(ptr, w, cs, fmt, dsize, mode); \
			} \
		}	else { \
			if (alpha == 0 && add == 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COVER_FILL]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) \
					ISPAN_FLAT_C_FILL(ptr, w, cs, fmt, dsize, mode); \
			} \
			else if ((cs >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COVER_BLEND]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) \
					ISPAN_FLAT_C_BLEND(ptr, w, cs, fmt, dsize, mode); \
			} \
		} \
	}	else { \
		if (cover == NULL) { \
			if (alpha == 0 && add == 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_GOURAUD_FILL]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) \
					ISPAN_GOURAUD_FILL(ptr, w, cs, ce, fmt, dsize); \
			}	\
			else if ((cs >> 24) != 0 || (ce >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_GOURAUD_BLEND]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) \
					ISPAN_GOURAUD_BLEND(ptr, w, cs, ce, fmt, dsize, mode); \
			} \
		}	else { \
			if (alpha == 0 && add == 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_GOURAUD_COVER_FILL]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) \
					ISPAN_GOURAUD_C_FILL(ptr, w, cs, ce, fmt, dsize, mode); \
			} \
			else if ((cs >> 24) != 0 || (ce >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_GOURAUD_COVER_BLEND]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) \
					ISPAN_GOURAUD_C_BLEND(ptr, w, cs, ce, fmt, dsize, mode);\
			} \
		} \
	} \
}


//---------------------------------------------------------------------
// NORMAL COPY
//---------------------------------------------------------------------
#define ISPAN_BLIT_LOOP(fmt, pixelfmt, dsize, init, work1, work2) { \
	size_t __width = w; \
	init; \
	for (; __width > 0; __width--, ptr += dsize, pixel += 4) { \
		work1; \
		pc = _ipixel_get(4, pixel); \
		IRGBA_FROM_PIXEL(pc, pixelfmt, pr, pg, pb, pa); \
		work2; \
		_ipixel_put(dsize, ptr, cc); \
	}	\
}


#define ISPAN_COLOR_NORM_0() {}

#define ISPAN_COLOR_NORM_1() { \
	ca = (_iblend_norm_fast(cr) * ca) >> 8; \
	ca = _iblend_norm_fast(ca); \
}

#define ISPAN_COLOR_NORM_2() { \
	ca = _iblend_norm_fast(ca); \
	cr = _iblend_norm_fast(cr); \
	cg = _iblend_norm_fast(cg); \
	cb = _iblend_norm_fast(cb); \
}

#define ISPAN_COLOR_CHANGE_0() {}

#define ISPAN_COLOR_CHANGE_1() { \
	pa = (pa * ca) >> 8; \
}

#define ISPAN_COLOR_CHANGE_2() { \
	pa = (ca * pa) >> 8; \
	pr = (cr * pr) >> 8; \
	pg = (cg * pg) >> 8; \
	pb = (cb * pb) >> 8; \
}

#define ISPAN_FLAT_COPY(fmt, dsize, mode) { \
	if (cs == 0xffffffff) { \
		ILINS_LOOP_DOUBLE( \
			{ \
				c1 = _ipixel_get(4, pixel); \
				IRGBA_FROM_PIXEL(c1, RGB32, r1, g1, b1, a1); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
				_ipixel_put(dsize, ptr, c1); \
				ptr += dsize; \
				pixel += 4; \
			}, \
			{ \
				c1 = _ipixel_get(4, pixel); \
				pixel += 4; \
				c2 = _ipixel_get(4, pixel); \
				pixel += 4; \
				IRGBA_FROM_PIXEL(c1, RGB32, r1, g1, b1, a1); \
				IRGBA_FROM_PIXEL(c2, RGB32, r2, g2, b2, a2); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
				c2 = IRGBA_TO_PIXEL(fmt, r2, g2, b2, 255); \
				_ipixel_put(dsize, ptr, c1); \
				ptr += dsize; \
				_ipixel_put(dsize, ptr, c2); \
				ptr += dsize; \
			}, \
			w); \
	}	else { \
		IRGBA_FROM_PIXEL(cs, ARGB32, cr, cg, cb, ca); \
		if (cr == cg && cg == cb) { \
			ca = (_iblend_norm_fast(cr) * ca) >> 8; \
			for (; w > 0; w--, ptr += dsize, pixel += 4) { \
				pc = _ipixel_get(4, pixel); \
				c1 = _ipixel_get(dsize, ptr); \
				IRGBA_FROM_PIXEL(pc, RGB32, pr, pg, pb, pa); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				pa = ca; \
				IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
				_ipixel_put(dsize, ptr, c1); \
			}	\
		}	else { \
			cr = _iblend_norm_fast(cr); \
			cg = _iblend_norm_fast(cg); \
			cb = _iblend_norm_fast(cb); \
			if (ca == 255) { \
				for (; w > 0; w--, ptr += dsize, pixel += 4) { \
					pc = _ipixel_get(4, pixel); \
					IRGBA_FROM_PIXEL(pc, RGB32, pr, pg, pb, pa); \
					pr = (cr * pr) >> 8; \
					pg = (cg * pg) >> 8; \
					pb = (cb * pb) >> 8; \
					c1 = IRGBA_TO_PIXEL(fmt, pr, pg, pb, 255); \
					_ipixel_put(dsize, ptr, c1); \
				}	\
			}	else { \
				for (; w > 0; w--, ptr += dsize, pixel += 4) { \
					pc = _ipixel_get(4, pixel); \
					c1 = _ipixel_get(dsize, ptr); \
					IRGBA_FROM_PIXEL(pc, RGB32, pr, pg, pb, pa); \
					IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
					pa = ca; \
					pr = (cr * pr) >> 8; \
					pg = (cg * pg) >> 8; \
					pb = (cb * pb) >> 8; \
					IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
					c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
					_ipixel_put(dsize, ptr, c1); \
				}	\
			} \
		}	\
	} \
}

#define ISPAN_FLAT_ROUTINE_1(fmt, pixelfmt, dsize, mode, CHG) { \
	ISPAN_BLIT_LOOP(fmt, pixelfmt, dsize, \
		{ \
			ISPAN_COLOR_NORM_##CHG(); \
		}, \
		{ \
		}, \
		{ \
			if (pa == 0) continue; \
			c1 = _ipixel_get(dsize, ptr); \
			ISPAN_COLOR_CHANGE_##CHG(); \
			IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
			IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
			cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
		}); \
}

#define ISPAN_FLAT_ROUTINE_2(fmt, pixelfmt, dsize, mode, CHG) { \
	ISPAN_BLIT_LOOP(fmt, RGB32, dsize, \
		{ \
			ISPAN_COLOR_NORM_##CHG(); \
		}, \
		{ \
			mask = *cover++; \
		}, \
		{ \
			c1 = _ipixel_get(dsize, ptr); \
			pa = mask; \
			ISPAN_COLOR_CHANGE_##CHG(); \
			IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
			IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
			cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
		}); \
}

#define ISPAN_FLAT_ROUTINE_3(fmt, pixelfmt, dsize, mode, CHG) { \
	ISPAN_BLIT_LOOP(fmt, RGB32, dsize, \
		{ \
			ISPAN_COLOR_NORM_##CHG(); \
		}, \
		{ \
			mask = *cover++; \
		}, \
		{ \
			mask = _iblend_norm_fast(mask); \
			if (pa == 0 || mask == 0) continue; \
			c1 = _ipixel_get(dsize, ptr); \
			pa = (mask * pa) >> 8; \
			ISPAN_COLOR_CHANGE_##CHG(); \
			IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
			IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
			cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
		}); \
}

#define ISPAN_FLAT_BLIT(fmt, dsize, mode) { \
	if (cs == 0xffffffff) { \
		if (additive == 0) { \
			ISPAN_BLIT_LOOP(fmt, ARGB32, dsize, \
				{ \
				}, \
				{ \
				}, \
				{ \
					if (pa == 255) { \
						cc = IRGBA_TO_PIXEL(fmt, pr, pg, pb, 255); \
					} \
					else if (pa > 0) { \
						c1 = _ipixel_get(dsize, ptr); \
						IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
						IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
						cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
					}	else { \
						continue; \
					}	\
				}); \
		}	else { \
			ISPAN_FLAT_ROUTINE_1(fmt, ARGB32, dsize, mode, 0); \
		} \
	}	else { \
		IRGBA_FROM_PIXEL(cs, ARGB32, cr, cg, cb, ca); \
		if (cr == cg && cg == cb) { \
			ISPAN_FLAT_ROUTINE_1(fmt, ARGB32, dsize, mode, 1); \
		}	else { \
			ISPAN_FLAT_ROUTINE_1(fmt, ARGB32, dsize, mode, 2); \
		} \
	} \
}

#define ISPAN_FLAT_C_COPY(fmt, dsize, mode) { \
	if (cs == 0xffffffff) { \
		ISPAN_BLIT_LOOP(fmt, RGB32, dsize, \
			{ \
			}, \
			{ \
				mask = *cover++; \
			}, \
			{ \
				if (mask == 255) { \
					cc = IRGBA_TO_PIXEL(fmt, pr, pg, pb, 255); \
				}	\
				else if (mask > 0) { \
					c1 = _ipixel_get(dsize, ptr); \
					pa = mask; \
					IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
					IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
					cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
				}	else { \
					continue; \
				} \
			}); \
	}	else { \
		IRGBA_FROM_PIXEL(cs, ARGB32, cr, cg, cb, ca); \
		if (cr == cg && cg == cb) { \
			ISPAN_FLAT_ROUTINE_2(fmt, RGB32, dsize, mode, 1); \
		}	else { \
			ISPAN_FLAT_ROUTINE_2(fmt, RGB32, dsize, mode, 2); \
		} \
	}	\
}

#define ISPAN_FLAT_C_BLIT(fmt, dsize, mode) { \
	if (cs == 0xffffffff) { \
		ISPAN_FLAT_ROUTINE_3(fmt, ARGB32, dsize, mode, 0); \
	}	else { \
		IRGBA_FROM_PIXEL(cs, ARGB32, cr, cg, cb, ca); \
		if (cr == cg && cg == cb) { \
			ISPAN_FLAT_ROUTINE_3(fmt, ARGB32, dsize, mode, 1); \
		}	else { \
			ISPAN_FLAT_ROUTINE_3(fmt, ARGB32, dsize, mode, 2); \
		} \
	}	\
}

// 高罗德着色
#define ISPAN_GOURAUD_BLIT_LOOP(fmt, fmt2, dsize, init, work1, work2) {\
		IINT32 rr, gg, bb, aa, dr, dg, db, da, dd; \
		IRGBA_FROM_PIXEL(cs, ARGB32, r1, g1, b1, a1); \
		IRGBA_FROM_PIXEL(ce, ARGB32, r2, g2, b2, a2); \
		rr = r1 << 16; gg = g1 << 16; bb = b1 << 16; aa = a1 << 16; \
		dd = (w < 2)? 1 : (w - 1); \
		dr = ((((IINT32)r2) - ((IINT32)r1)) << 16) / dd; \
		dg = ((((IINT32)g2) - ((IINT32)g1)) << 16) / dd; \
		db = ((((IINT32)b2) - ((IINT32)b1)) << 16) / dd; \
		da = ((((IINT32)a2) - ((IINT32)a1)) << 16) / dd; \
		init; \
		for (; w > 0; w--, ptr += dsize, pixel += 4) { \
			cr = rr >> 16; \
			cg = gg >> 16; \
			cb = bb >> 16; \
			ca = aa >> 16; \
			work1; \
			pc = _ipixel_get(4, pixel); \
			IRGBA_FROM_PIXEL(pc, fmt2, pr, pg, pb, pa); \
			work2; \
			_ipixel_put(dsize, ptr, cc); \
			rr += dr; \
			gg += dg; \
			bb += db; \
			aa += da; \
		} \
		cc = c1 = c2 = 0; cc = cc; c1 = c1; c2 = c2; \
	}


#define ISPAN_GOURAUD_COPY(fmt, dsize, mode) { \
	ISPAN_GOURAUD_BLIT_LOOP(fmt, RGB32, dsize, \
		{ \
		}, \
		{ \
			cr = _iblend_norm_fast(cr); \
			cg = _iblend_norm_fast(cg); \
			cb = _iblend_norm_fast(cb); \
		}, \
		{ \
			if (ca == 0) continue; \
			c1 = _ipixel_get(dsize, ptr); \
			pa = ca; \
			pr = (cr * pr) >> 8; \
			pg = (cg * pg) >> 8; \
			pb = (cb * pb) >> 8; \
			IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
			IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
			cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
		}); \
}

#define ISPAN_GOURAUD_BLIT(fmt, dsize, mode) { \
	ISPAN_GOURAUD_BLIT_LOOP(fmt, ARGB32, dsize, \
		{ \
		}, \
		{ \
			ISPAN_COLOR_NORM_2(); \
		}, \
		{ \
			if (ca == 0 || pa == 0) continue; \
			c1 = _ipixel_get(dsize, ptr); \
			ISPAN_COLOR_CHANGE_2(); \
			IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
			IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
			cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
		}); \
}

#define ISPAN_GOURAUD_C_COPY(fmt, dsize, mode) { \
	IINT32 mask; \
	ISPAN_GOURAUD_BLIT_LOOP(fmt, RGB32, dsize, \
		{ \
		}, \
		{ \
			mask = *cover++; \
			ISPAN_COLOR_NORM_2(); \
		}, \
		{ \
			pa = mask; \
			if (ca == 0 || mask == 0) continue; \
			c1 = _ipixel_get(dsize, ptr); \
			ISPAN_COLOR_CHANGE_2(); \
			IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
			IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
			cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
		}); \
}

#define ISPAN_GOURAUD_C_BLIT(fmt, dsize, mode) { \
	IINT32 mask; \
	ISPAN_GOURAUD_BLIT_LOOP(fmt, ARGB32, dsize, \
		{ \
		}, \
		{ \
			mask = *cover++; \
			ISPAN_COLOR_NORM_2(); \
			mask = _iblend_norm_fast(mask); \
		}, \
		{ \
			if (ca == 0 || mask == 0 || pa == 0) continue; \
			c1 = _ipixel_get(dsize, ptr); \
			ISPAN_COLOR_CHANGE_2(); \
			pa = (pa * mask) >> 8; \
			IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
			IBLEND_##mode(pr, pg, pb, pa, r1, g1, b1, a1); \
			cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
		}); \
}


//---------------------------------------------------------------------
// MAIN SPAN BLIT ROUTINE
//---------------------------------------------------------------------
#define ISPAN_BLIT_MAIN(ptr, w, cs, ce, cover, fmt, dsize, mode, add) { \
	ICOLORD cc, pc, cr, cg, cb, ca, pr, pg, pb, pa, mask; \
	ICOLORD c1, c2, r1, g1, b1, a1, r2, g2, b2, a2; \
	int pixfmt = IPIX_FMT_##fmt; \
	int retval = -1; \
	ISPAN_PROC proc; \
	if (cs == ce) { \
		if (cover == NULL) { \
			if (alpha == 0 && add == 0 && cs != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COPY]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) ISPAN_FLAT_COPY(fmt, dsize, mode); \
			} \
			else if ((cs >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COPY]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) ISPAN_FLAT_BLIT(fmt, dsize, mode); \
			} \
		}	else { \
			if (alpha == 0 && add == 0 && cs != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COPY]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) ISPAN_FLAT_C_COPY(fmt, dsize, mode); \
			} \
			else if ((cs >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COPY]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) ISPAN_FLAT_C_BLIT(fmt, dsize, mode); \
			} \
		} \
	}	else { \
		if (cover == NULL) { \
			if (alpha == 0 && add == 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COPY]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) ISPAN_GOURAUD_COPY(fmt, dsize, mode); \
			} \
			else if ((cs >> 24) != 0 || (ce >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COPY]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) ISPAN_GOURAUD_BLIT(fmt, dsize, mode); \
			} \
		}	else { \
			if (alpha == 0 && add == 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COPY]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) ISPAN_GOURAUD_C_COPY(fmt, dsize, mode); \
			} \
			else if ((cs >> 24) != 0 || (ce >> 24) != 0) { \
				proc = ispan_proc[pixfmt][ISPAN_MODE_FLAT_COPY]; \
				if (proc && noextra == 0) \
					retval = proc(ptr, span, pixfmt, add); \
				if (retval != 0) ISPAN_GOURAUD_C_BLIT(fmt, dsize, mode); \
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
static inline void _ispan_fill_##fmt(unsigned char *ptr, const ISPAN *span, \
	int additive, int noextra) \
{ \
	const unsigned char *cover = span->cover; \
	const unsigned char *pixel = span->pixel; \
	ICOLORD cs = span->color1; \
	ICOLORD ce = span->color2; \
	int alpha = span->alpha; \
	int w = span->w; \
	if (w <= 0) return; \
	if (pixel == NULL) { \
		if (additive == 0) { \
			ISPAN_FILL_MAIN(ptr, w, cs, ce, cover, fmt, dsize, mode, 0); \
		}	else { \
			ISPAN_FILL_MAIN(ptr, w, cs, ce, cover, fmt, dsize, ADDITIVE, 1);\
		} \
	}	else {  \
		if (additive == 0) { \
			ISPAN_BLIT_MAIN(ptr, w, cs, ce, cover, fmt, dsize, mode, 0); \
		}	else { \
			ISPAN_BLIT_MAIN(ptr, w, cs, ce, cover, fmt, dsize, ADDITIVE, 0);\
		} \
	} \
} \
static void _ispan_draw_##fmt(IBITMAP *dst, const ISPAN *spans, int count, \
	int additive, int noextra) \
{ \
	for (; count > 0; count--, spans++) { \
		unsigned char *ptr = _ilineptr(dst, spans->y) + spans->x * dsize; \
		_ispan_fill_##fmt(ptr, spans, additive, noextra); \
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
// 绘制线段主函数
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


//---------------------------------------------------------------------
// 线段收缩，大于零是左边，小于零是右边
//---------------------------------------------------------------------
void ispan_shrink(ISPAN *span, int width)
{
	IINT32 r1, g1, b1, a1, r2, g2, b2, a2;
	IINT32 dr, dg, db, da, dd;
	if (width > 0) {		// 收缩左边
		if (width > span->w) width = span->w;
		span->x += width;
		if (span->cover) span->cover += width;
		if (span->pixel) span->pixel += width * 4;
		if (span->color1 != span->color2) {
			IRGBA_FROM_PIXEL(span->color1, ARGB32, r1, g1, b1, a1);
			IRGBA_FROM_PIXEL(span->color2, ARGB32, r2, g2, b2, a2);
			dd = (span->w > 1)? (span->w - 1) : 1;
			dr = (r2 - r1) / dd;
			dg = (g2 - g1) / dd;
			db = (b2 - b1) / dd;
			da = (a2 - a1) / dd;
			r1 += dr * width;
			g1 += dg * width;
			b1 += db * width;
			a1 += da * width;
			span->color1 = IRGBA_TO_PIXEL(ARGB32, r1, g1, b1, a1);
		}
		span->w -= width;
	}
	else if (width < 0) {	// 收缩右边
		width = -width;
		if (width > span->w) width = span->w;
		if (span->color1 != span->color2) {
			IRGBA_FROM_PIXEL(span->color1, ARGB32, r1, g1, b1, a1);
			IRGBA_FROM_PIXEL(span->color2, ARGB32, r2, g2, b2, a2);
			dd = (span->w > 1)? (span->w - 1) : 1;
			dr = (r2 - r1) / dd;
			dg = (g2 - g1) / dd;
			db = (b2 - b1) / dd;
			da = (a2 - a1) / dd;
			r2 -= dr * width;
			g2 -= dg * width;
			b2 -= db * width;
			a2 -= da * width;
			span->color2 = IRGBA_TO_PIXEL(ARGB32, r2, g2, b2, a2);
		}
		span->w -= width;
	}
}


