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
// GLOBAL DEFINITION
//---------------------------------------------------------------------
int ibicubic_lookup[IBICUBIC_SIZE * 2];
int ibicubic_tables[(2 << IBICUBIC_BITS)][4];
int ibicubic_inited = 0;

void ibicubic_init(void)
{
	static int inited = 0;
	int i, j, size;
	if (inited != 0) return;
	for (i = 0; i < IBICUBIC_SIZE; i++) {
		float x = (i / ((float)(1 << IBICUBIC_BITS)));
		float f1 = ikernel_bicubic(x);
		float f2 = ikernel_cubic(x, -0.5f);
		ibicubic_lookup[i] = (int)(f1 * 65536.0f);
		ibicubic_lookup[i + IBICUBIC_SIZE] = (int)(f2 * 65536.0f);
	}
	size = (1 << IBICUBIC_BITS);
	for (i = 0, j = size; i < size; i++, j++) {
		float dx = i / ((float)size);
		float f1 = ikernel_bicubic(-1 - dx + 0);
		float f2 = ikernel_bicubic(-1 - dx + 1);
		float f3 = ikernel_bicubic(-1 - dx + 2);
		float f4 = ikernel_bicubic(-1 - dx + 3);
		float f5 = ikernel_cubic(-1 - dx + 0, -0.5f);
		float f6 = ikernel_cubic(-1 - dx + 1, -0.5f);
		float f7 = ikernel_cubic(-1 - dx + 2, -0.5f);
		float f8 = ikernel_cubic(-1 - dx + 3, -0.5f);
		float k1 = f1 + f2 + f3 + f4;
		float k2 = f5 + f6 + f7 + f8;
		k1 = (k1 == 0.0f)? 0.0f : 65536.0f / k1;
		k2 = (k2 == 0.0f)? 0.0f : 65536.0f / k2;
		ibicubic_tables[i][0] = (int)(f1 * k1);
		ibicubic_tables[i][1] = (int)(f2 * k1);
		ibicubic_tables[i][2] = (int)(f3 * k1);
		ibicubic_tables[i][3] = (int)(f4 * k1);
		ibicubic_tables[j][0] = (int)(f5 * k2);
		ibicubic_tables[j][1] = (int)(f6 * k2);
		ibicubic_tables[j][2] = (int)(f7 * k2);
		ibicubic_tables[j][3] = (int)(f8 * k2);
	}
	ibicubic_inited = 1;
	inited = 1;
}



int ibitmap_blitclip(const IBITMAP *dst, int *dx, int *dy, 
	const IBITMAP *src, int *sx, int *sy, int *sw, int *sh, int flags);


int ibitmap_scale_clip(const IBITMAP *dst, IRECT *drect, const IRECT *clip,
	const IBITMAP *src, IRECT *srect)
{
	int x1 = drect->left;
	int y1 = drect->top;
	int w1 = drect->right - x1;
	int h1 = drect->bottom - y1;
	int x2 = srect->left;
	int y2 = srect->top;
	int w2 = srect->right - x2;
	int h2 = srect->bottom - y2;
	int cl, ct, cr, cb;
	int cw, ch;
	if (clip == NULL) {
		cl = 0, ct = 0, cr = dst->w, cb = dst->h;
	}	else {
		cl = clip->left;
		ct = clip->top;
		cr = clip->right;
		cb = clip->bottom;
	}
	cw = cr - cl;
	ch = cb - ct;
	if (w1 <= 0 || h1 <= 0 || w2 <= 0 || h2 <= 0) return -1;
	if (x1 >= cl && y1 >= ct && w1 <= cw && h1 <= ch &&
		x2 >= 0 && y2 >= 0 && w2 <= (int)src->w && h2 <= (int)src->h) {
		return 0;
	}	else {
		float fx = ((float)w2) / w1;
		float fy = ((float)h2) / h1;
		float ix = 1.0f / fx;
		float iy = 1.0f / fy;
		int ds, d;
		if (x1 < cl) {
			d = cl - x1;
			x2 += (int)(d * fx);
			w2 -= (int)(d * fx);
			w1 -= d;
			x1 = cl;
		}
		if (y1 < ct) {
			d = ct - y1;
			y2 += (int)(d * fx);
			h2 -= (int)(d * fx);
			h1 -= (int)d;
			y1 = (int)ct;
		}
		if (x2 < 0) 
			x1 += (int)(-x2 * ix), w1 += (int)(x2 * ix), w2 += x2, x2 = 0;
		if (y2 < 0) 
			y1 += (int)(-y2 * iy), h2 += (int)(y2 * iy), h2 += y2, y2 = 0;
		if (x1 >= cr || y1 >= cb) return -2;
		if (x2 >= (int)src->w || y2 >= (int)src->h) return -3;
		if (w1 <= 0 || h1 <= 0 || w2 <= 0 || h2 <= 0) return -4;
		if (x1 + w1 > cr) 
			ds = x1 + w1 - cr, w1 -= ds, w2 -= (int)(ds * fx);
		if (y1 + h1 > cb)
			ds = y1 + h1 - cb, h1 -= ds, h2 -= (int)(ds * fx);
		if (x2 + w2 > (int)src->w)
			ds = x2 + w2 - (int)src->w, w1 -= (int)(ds * ix), w2 -= ds;
		if (y2 + h2 > (int)src->h)
			ds = y2 + h2 - (int)src->h, h1 -= (int)(ds * iy), h2 -= ds;
		if (w1 <= 0 || h1 <= 0 || w2 <= 0 || h2 <= 0) 
			return -5;
	}
	irect_set(drect, x1, y1, x1 + w1, y1 + h1);
	irect_set(srect, x2, y2, x2 + w2, y2 + h2);
	return 0;
}


//---------------------------------------------------------------------
// ibitmap_scale
//---------------------------------------------------------------------
static int ibitmap_stretch_fast(IBITMAP *dst, const IRECT *rectdst, 
	const IBITMAP *src, const IRECT *rectsrc, int flags)
{
	int dx, dy, dw, dh, sx, sy, sw, sh;
	IRECT drect;
	IRECT srect;

	irect_copy(&drect, rectdst);
	irect_copy(&srect, rectsrc);

	if (src->bpp != dst->bpp) 
		return -100;

	dx = drect.left;
	dy = drect.top;
	dw = drect.right - dx;
	dh = drect.bottom - dy;
	sx = srect.left;
	sy = srect.top;
	sw = srect.right - sx;
	sh = srect.bottom - sy;

	if (flags & (IBLIT_VFLIP | IBLIT_HFLIP)) flags |= ISTRETCH_FAST;

	if (flags & ISTRETCH_FAST) {
		if (dx > 0x7000 || dy > 0x7000 || dw > 0x7000 || dh > 0x7000 ||
			sx > 0x7000 || sy > 0x7000 || sw > 0x7000 || sh > 0x7000) {
			flags &= ~ISTRETCH_FAST;
		}
	}

	if ((flags & (IBLIT_VFLIP | IBLIT_HFLIP)) && (!(flags & ISTRETCH_FAST)))
		return -200;

	#if 0
	printf("(%d,%d,%d,%d) <- (%d,%d,%d,%d)\n", dx, dy, dw, dh, sx, sy,
		sw, sh);
	#endif

	#define ibitmap_scale_routine(nbytes, bpp) {	\
			unsigned char *dstrow = _ilineptr(dst, dy) + dx * nbytes; \
			unsigned char *srcrow = _ilineptr(src, sy) + sx * nbytes; \
			long srcpitch = src->pitch;	\
			long dstpitch = dst->pitch; \
			int dstwidth = dw; \
			int dstheight = dh; \
			int dstwidth2 = dw * 2; \
			int dstheight2 = dh * 2; \
			int srcwidth2 = sw * 2; \
			int srcheight2 = sh * 2; \
			int werr = 0; \
			int herr = srcheight2 - dstheight2; \
			int loopw = 0; \
			int looph = 0; \
			ICOLORD mask, c; \
			mask = src->mask; \
			for (looph = 0; looph < dstheight; looph++) { \
				unsigned char *dstpix = dstrow; \
				unsigned char *srcpix = srcrow; \
				werr = srcwidth2 - dstwidth2; \
				if ((flags & IBLIT_MASK) == 0) { \
					for (loopw = dstwidth; loopw > 0; loopw--) { \
						c = _im_get##nbytes##b(srcpix); \
						_im_put##nbytes##b(dstpix, c); \
						dstpix += nbytes; \
						while (werr >= 0) { \
							srcpix += nbytes, werr -= dstwidth2; \
						}	\
						werr += srcwidth2; \
					} \
				}	else { \
					for (loopw = dstwidth; loopw > 0; loopw--) { \
						c = _im_get##nbytes##b(srcpix); \
						if (c != mask) { \
							_im_put##nbytes##b(dstpix, c); \
						}	\
						dstpix += nbytes; \
						while (werr >= 0) { \
							srcpix += nbytes, werr -= dstwidth2; \
						}	\
						werr += srcwidth2; \
					} \
				}	\
				while (herr >= 0) { \
					srcrow += srcpitch, herr -= dstheight2; \
				}	\
				dstrow += dstpitch; \
				herr += srcheight2; \
			}	\
		}

	#define ibitmap_scale_fast(nbytes, bpp) {	\
			unsigned char *dstpix = NULL; \
			unsigned char *srcpix = NULL; \
			unsigned char *texpix = NULL; \
			int du = ((sw - 1) << 16) / dw; \
			int dv = ((sh - 1) << 16) / dh; \
			int x, y, u, v; \
			int lineno; \
			ICOLORD mask, c; \
			mask = src->mask; \
			if (flags & IBLIT_HFLIP) du = -du; \
			if (flags & IBLIT_VFLIP) dv = -dv; \
			for (y = 0, v = 0; y < dh; y++, v += dv) { \
				dstpix = _ilineptr(dst, dy + y) + dx * nbytes; \
				if ((flags & IBLIT_VFLIP) == 0) lineno = sy + (v >> 16); \
				else lineno = sy + sh - 1 + (v >> 16); \
				if (lineno < sy) lineno = sy; \
				else if (lineno >= sy + sh - 1) lineno = sy + sh - 1; \
				srcpix = _ilineptr(src, lineno) + sx * nbytes; \
				if (flags & IBLIT_HFLIP) srcpix += (sw - 1) * nbytes; \
				if ((flags & IBLIT_MASK) == 0) { \
					for (x = dw, u = 0; x > 0; x--, u += du) { \
						texpix = srcpix + (u >> 16) * nbytes; \
						c = _im_get##nbytes##b(texpix); \
						_im_put##nbytes##b(dstpix, c); \
						dstpix += nbytes; \
					} \
				}	else { \
					for (x = dw, u = 0; x > 0; x--, u += du) { \
						texpix = srcpix + (u >> 16) * nbytes; \
						c = _im_get##nbytes##b(texpix); \
						if (c != mask) { \
							_im_put##nbytes##b(dstpix, c); \
						}	\
						dstpix += nbytes; \
					} \
				} \
			}	\
		}

	if (flags & ISTRETCH_FAST) {
		switch (dst->bpp) 
		{
		case  8: ibitmap_scale_fast(1,  8); break;
		case 15: ibitmap_scale_fast(2, 15); break;
		case 16: ibitmap_scale_fast(2, 16); break;
		case 24: ibitmap_scale_fast(3, 24); break;
		case 32: ibitmap_scale_fast(4, 32); break;
		}
		return 0;
	}

	switch (dst->bpp) 
	{
	case  8: ibitmap_scale_routine(1,  8); break;
	case 15: ibitmap_scale_routine(2, 15); break;
	case 16: ibitmap_scale_routine(2, 16); break;
	case 24: ibitmap_scale_routine(3, 24); break;
	case 32: ibitmap_scale_routine(4, 32); break;
	}

	#undef ibitmap_scale_routine
	#undef ibitmap_scale_fast

	return 0;
}



//---------------------------------------------------------------------
// IGET_PIXEL_COLOR
//---------------------------------------------------------------------
#define IGET_PIXEL_COLOR(fmt) \
static inline ICOLORD interp_get_color_normal_##fmt(int u, int v, \
	const ICOLORB *ptr, long pitch, int w, int h, int om, ICOLORD dc) { \
	ICOLORD c; \
	ITEX_GETCOL_NORMAL(c, fmt, u, v, ptr, pitch, w, h, om, dc); \
	return c; \
} \
static inline ICOLORD interp_get_color_nearest_##fmt(int u, int v, \
	const ICOLORB *ptr, long pitch, int w, int h, int om, ICOLORD dc) { \
	ICOLORD c; \
	u += 0x8000; v += 0x8000; \
	ITEX_GETCOL_NORMAL(c, fmt, u, v, ptr, pitch, w, h, om, dc); \
	return c; \
} \
static inline ICOLORD interp_get_color_bilinear_##fmt(int u, int v, \
	const ICOLORB *ptr, long pitch, int w, int h, int om, ICOLORD dc) { \
	ICOLORD c; \
	ITEX_GETCOL_BILINEAR(c, fmt, u, v, ptr, pitch, w, h, om, dc); \
	return c; \
} \
static inline ICOLORD interp_get_color_bicubic_##fmt(int u, int v, \
	const ICOLORB *ptr, long pitch, int w, int h, int om, ICOLORD dc) { \
	ICOLORD c; \
	ITEX_GETCOL_BICUBIC(c, fmt, u, v, ptr, pitch, w, h, 0, om, dc); \
	return c; \
} \
static inline ICOLORD interp_get_color_bicubic2_##fmt(int u, int v, \
	const ICOLORB *ptr, long pitch, int w, int h, int om, ICOLORD dc) { \
	ICOLORD c; \
	ITEX_GETCOL_BICUBIC(c, fmt, u, v, ptr, pitch, w, h, 1, om, dc); \
	return c; \
} \
static inline ICOLORD interp_texture_get_normal_##fmt(int u, int v, \
	const unsigned char *ptr, int shift, int umask, int vmask) { \
	ICOLORD c; \
	ITEX_UVGETCOL_NORMAL(c, fmt, u, v, ptr, shift, umask, vmask); \
	return c; \
} \
static inline ICOLORD interp_texture_get_nearest_##fmt(int u, int v, \
	const unsigned char *ptr, int shift, int umask, int vmask) { \
	ICOLORD c; \
	u += 0x8000; v += 0x8000; \
	ITEX_UVGETCOL_NORMAL(c, fmt, u, v, ptr, shift, umask, vmask); \
	return c; \
} \
static inline ICOLORD interp_texture_get_bilinear_##fmt(int u, int v, \
	const unsigned char *ptr, int shift, int umask, int vmask) { \
	ICOLORD c; \
	ITEX_UVGETCOL_BILINEAR(c, fmt, u, v, ptr, shift, umask, vmask); \
	return c; \
} \
static inline ICOLORD interp_texture_get_bicubic_##fmt(int u, int v, \
	const unsigned char *ptr, int shift, int umask, int vmask) { \
	ICOLORD c; \
	ITEX_UVGETCOL_BILINEAR(c, fmt, u, v, ptr, shift, umask, vmask); \
	return c; \
} \
static inline ICOLORD interp_texture_get_bicubic2_##fmt(int u, int v, \
	const unsigned char *ptr, int shift, int umask, int vmask) { \
	ICOLORD c; \
	ITEX_UVGETCOL_BILINEAR(c, fmt, u, v, ptr, shift, umask, vmask); \
	return c; \
} 

IGET_PIXEL_COLOR(8);
IGET_PIXEL_COLOR(RGB15);
IGET_PIXEL_COLOR(BGR15);
IGET_PIXEL_COLOR(RGB16);
IGET_PIXEL_COLOR(BGR16);
IGET_PIXEL_COLOR(RGB24);
IGET_PIXEL_COLOR(BGR24);
IGET_PIXEL_COLOR(RGB32);
IGET_PIXEL_COLOR(BGR32);
IGET_PIXEL_COLOR(ARGB32);
IGET_PIXEL_COLOR(ABGR32);
IGET_PIXEL_COLOR(RGBA32);
IGET_PIXEL_COLOR(BGRA32);
IGET_PIXEL_COLOR(ARGB_4444);
IGET_PIXEL_COLOR(ABGR_4444);
IGET_PIXEL_COLOR(RGBA_4444);
IGET_PIXEL_COLOR(BGRA_4444);
IGET_PIXEL_COLOR(ARGB_1555);
IGET_PIXEL_COLOR(ABGR_1555);
IGET_PIXEL_COLOR(RGBA_5551);
IGET_PIXEL_COLOR(BGRA_5551);


#define interp_getcol(fmt, filter, u, v, sss, pitch, w, h, om, dc) \
	interp_get_color_##filter##_##fmt(u, v, sss, pitch, w, h, om, dc)

ICOLORD interpolated_getcol(int pixfmt, int filter, int u, int v, 
	const unsigned char *src, long pitch, int w, int h, int overflow,
	ICOLORD olfColor)
{
	ICOLORD color;
	if (ibicubic_inited == 0) ibicubic_init();
	#define _interp_get_case(fmt) do { \
			switch (filter) { \
			case IFILTER_NORMAL: \
				color = interp_getcol(fmt, normal, u, v, src, pitch, \
							w, h, overflow, olfColor); \
				break; \
			case IFILTER_NEAREST: \
				color = interp_getcol(fmt, nearest, u, v, src, pitch, \
							w, h, overflow, olfColor); \
				break; \
			case IFILTER_TEND: \
			case IFILTER_BILINEAR: \
				color = interp_getcol(fmt, bilinear, u, v, src, pitch, \
							w, h, overflow, olfColor); \
				break; \
			case IFILTER_BICUBIC: \
				color = interp_getcol(fmt, bicubic, u, v, src, pitch, \
							w, h, overflow, olfColor); \
				break; \
			case IFILTER_BICUBIC2: \
				color = interp_getcol(fmt, bicubic2, u, v, src, pitch, \
							w, h, overflow, olfColor); \
				break; \
			default: \
				color = olfColor; \
				break; \
			} \
		}	while (0)
	switch (pixfmt)
	{
	case IPIX_FMT_RGB15: _interp_get_case(RGB15); break;
	case IPIX_FMT_BGR15: _interp_get_case(BGR15); break;
	case IPIX_FMT_RGB16: _interp_get_case(RGB16); break;
	case IPIX_FMT_BGR16: _interp_get_case(BGR16); break;
	case IPIX_FMT_RGB24: _interp_get_case(RGB24); break;
	case IPIX_FMT_BGR24: _interp_get_case(BGR24); break;
	case IPIX_FMT_RGB32: _interp_get_case(RGB32); break;
	case IPIX_FMT_BGR32: _interp_get_case(BGR32); break;
	case IPIX_FMT_ARGB32: _interp_get_case(ARGB32); break;
	case IPIX_FMT_ABGR32: _interp_get_case(ABGR32); break;
	case IPIX_FMT_RGBA32: _interp_get_case(RGBA32); break;
	case IPIX_FMT_BGRA32: _interp_get_case(BGRA32); break;
	case IPIX_FMT_ARGB_4444: _interp_get_case(ARGB_4444); break;
	case IPIX_FMT_ABGR_4444: _interp_get_case(ABGR_4444); break;
	case IPIX_FMT_RGBA_4444: _interp_get_case(RGBA_4444); break;
	case IPIX_FMT_BGRA_4444: _interp_get_case(BGRA_4444); break;
	case IPIX_FMT_ARGB_1555: _interp_get_case(ARGB_1555); break;
	case IPIX_FMT_ABGR_1555: _interp_get_case(ABGR_1555); break;
	case IPIX_FMT_RGBA_5551: _interp_get_case(RGBA_5551); break;
	case IPIX_FMT_BGRA_5551: _interp_get_case(BGRA_5551); break;
	default: color = olfColor; break;
	}
	#undef _interp_get_case
	return color;
}


ICOLORD interpolated_bitmap(const IBITMAP *src, int u, int v, 
	int filter, int overflow, ICOLORD olfc)
{
	int pixfmt = ibitmap_pixfmt_const(src);
	return interpolated_getcol(pixfmt, filter, u, v, src->pixel,
		(long)src->pitch, (int)src->w, (int)src->h, overflow, olfc);
}


//---------------------------------------------------------------------
// Bitmap Smooth Stretch
//---------------------------------------------------------------------
static int ifilter_shrink_x_c(ICOLORB *dstpix, ICOLORB *srcpix, int height,
	long dstpitch, long srcpitch, int dstwidth, int srcwidth)
{
	int srcdiff = srcpitch - (srcwidth * 4);
	int dstdiff = dstpitch - (dstwidth * 4);
	int x, y;
	int xspace = 0x10000 * srcwidth / dstwidth;
	int xrecip = 0;

	IINT64 zrecip = 1;
	zrecip <<= 32;
	xrecip = (int)(zrecip / xspace);

	for (y = 0; y < height; y++) {
		ICOLORW accumulate[4] = { 0, 0, 0, 0 };
		int xcounter = xspace;
		for (x = 0; x < srcwidth; x++) {
			if (xcounter > 0x10000) {
				accumulate[0] += (ICOLORW) *srcpix++;
				accumulate[1] += (ICOLORW) *srcpix++;
				accumulate[2] += (ICOLORW) *srcpix++;
				accumulate[3] += (ICOLORW) *srcpix++;
				xcounter -= 0x10000;
			}	else {
				int xfrac = 0x10000 - xcounter;
				#define ismooth_putpix_x(n) { \
						*dstpix++ = (ICOLORB)(((accumulate[n] + ((srcpix[n] \
							* xcounter) >> 16)) * xrecip) >> 16); \
					}
				ismooth_putpix_x(0);
				ismooth_putpix_x(1);
				ismooth_putpix_x(2);
				ismooth_putpix_x(3);
				#undef ismooth_putpix_x
				accumulate[0] = (ICOLORW)((*srcpix++ * xfrac) >> 16);
				accumulate[1] = (ICOLORW)((*srcpix++ * xfrac) >> 16);
				accumulate[2] = (ICOLORW)((*srcpix++ * xfrac) >> 16);
				accumulate[3] = (ICOLORW)((*srcpix++ * xfrac) >> 16);
				xcounter = xspace - xfrac;
			}
		}
		srcpix += srcdiff;
		dstpix += dstdiff;
	}
	return 0;
}

static int ifilter_shrink_y_c(ICOLORB *dstpix, ICOLORB *srcpix, int width, 
	long dstpitch, long srcpitch, int dstheight, int srcheight)
{
	int srcdiff = srcpitch - (width * 4);
	int dstdiff = dstpitch - (width * 4);
	int x, y;
	int yspace = 0x10000 * srcheight / dstheight;
	int yrecip = 0;
	int ycounter = yspace;
	ICOLORW *templine;

	IINT64 zrecip = 1;
	zrecip <<= 32;
	yrecip = (int)(zrecip / yspace);

	templine = (ICOLORW*)malloc(dstpitch * 2);
	assert(templine);

	for (y = 0; y < srcheight; y++) {
		ICOLORW *accumulate = templine;
		if (ycounter > 0x10000) {
			for (x = 0; x < width; x++) {
				*accumulate++ += (ICOLORW) *srcpix++;
				*accumulate++ += (ICOLORW) *srcpix++;
				*accumulate++ += (ICOLORW) *srcpix++;
				*accumulate++ += (ICOLORW) *srcpix++;
			}
			ycounter -= 0x10000;
		}	else {
			int yfrac = 0x10000 - ycounter;
			for (x = 0; x < width; x++) {
				#define ismooth_putpix_y() { \
					*dstpix++ = (ICOLORB) (((*accumulate++ + ((*srcpix++ * \
						ycounter) >> 16)) * yrecip) >> 16); \
				}
				ismooth_putpix_y();
				ismooth_putpix_y();
				ismooth_putpix_y();
				ismooth_putpix_y();
				#undef ismooth_putpix_y
			}
			dstpix += dstdiff;
			accumulate = templine;
			srcpix -= 4 * width;
			for (x = 0; x < width; x++) {
				*accumulate++ = (ICOLORW) ((*srcpix++ * yfrac) >> 16);
				*accumulate++ = (ICOLORW) ((*srcpix++ * yfrac) >> 16);
				*accumulate++ = (ICOLORW) ((*srcpix++ * yfrac) >> 16);
				*accumulate++ = (ICOLORW) ((*srcpix++ * yfrac) >> 16);
			}
			ycounter = yspace - yfrac;
		}
		srcpix += srcdiff;
	}

	free(templine);
	return 0;
}

static int ifilter_expand_x_c(ICOLORB *dstpix, ICOLORB *srcpix, int height,
	long dstpitch, long srcpitch, int dstwidth, int srcwidth)
{
	int dstdiff = dstpitch - (dstwidth * 4);
	int *xidx0, *xmult0, *xmult1;
	int x, y;
	int factorwidth = 4;

	xidx0 = (int*)malloc(dstwidth * 4);
	if (xidx0 == NULL) return -1;
	xmult0 = (int*)malloc(dstwidth * factorwidth);
	xmult1 = (int*)malloc(dstwidth * factorwidth);

	if (xmult0 == NULL || xmult1 == NULL) {
		free(xidx0);
		if (xmult0) free(xmult0);
		if (xmult1) free(xmult1);
		return -1;
	}

	for (x = 0; x < dstwidth; x++) {
		xidx0[x] = x * (srcwidth - 1) / dstwidth;
		xmult1[x] = 0x10000 * ((x * (srcwidth - 1)) % dstwidth) / dstwidth;
		xmult0[x] = 0x10000 - xmult1[x];
	}

	for (y = 0; y < height; y++) {
		ICOLORB *srcrow0 = srcpix + y * srcpitch;
		for (x = 0; x < dstwidth; x++) {
			ICOLORB *src = srcrow0 + xidx0[x] * 4;
			int xm0 = xmult0[x];
			int xm1 = xmult1[x];
			*dstpix++ = (ICOLORB)(((src[0] * xm0) + (src[4] * xm1)) >> 16);
			*dstpix++ = (ICOLORB)(((src[1] * xm0) + (src[5] * xm1)) >> 16);
			*dstpix++ = (ICOLORB)(((src[2] * xm0) + (src[6] * xm1)) >> 16);
			*dstpix++ = (ICOLORB)(((src[3] * xm0) + (src[7] * xm1)) >> 16);
		}
		dstpix += dstdiff;
	}

	free(xidx0);
	free(xmult0);
	free(xmult1);
	return 0;
}

static int ifilter_expand_y_c(ICOLORB *dstpix, ICOLORB *srcpix, int width, 
	long dstpitch, long srcpitch, int dstheight, int srcheight)
{
	int x, y;
	for (y = 0; y < dstheight; y++) {
		int yidx0 = y * (srcheight - 1) / dstheight;
		ICOLORB *s0 = srcpix + yidx0 * srcpitch;
		ICOLORB *s1 = s0 + srcpitch;
		int ym1 = 0x10000 * ((y * (srcheight - 1)) % dstheight) / dstheight;
		int ym0 = 0x10000 - ym1;
		for (x = 0; x < width; x++) {
			*dstpix++ = (ICOLORB)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
			*dstpix++ = (ICOLORB)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
			*dstpix++ = (ICOLORB)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
			*dstpix++ = (ICOLORB)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
		}
		dstpix += dstpitch - 4 * width;
	}

	return 0;
}


static int ismooth_resize_32(ICOLORB *dstpix, ICOLORB *srcpix, int dstwidth,
	int srcwidth, int dstheight, int srcheight, long dstpitch, long srcpitch)
{
	ICOLORB *temp = NULL;

	if (srcwidth == dstwidth && srcheight == dstheight) {
		long size, y;
		for (y = 0, size = srcwidth * 4; y < dstheight; y++) {
			memcpy(dstpix + y * dstpitch, srcpix + y * srcpitch, size);
		}
		return 0;
	}

	temp = (ICOLORB*)malloc((long)srcwidth * dstheight * 4);

	if (temp == NULL) return -1;

	if (dstheight < srcheight) {
		if (ifilter_shrink_y_c(temp, srcpix, srcwidth, srcwidth * 4, 
			srcpitch, dstheight, srcheight) != 0) {
			free(temp);
			return -2;
		}
	}
	else if (dstheight > srcheight) {
		if (ifilter_expand_y_c(temp, srcpix, srcwidth, srcwidth * 4,
			srcpitch, dstheight, srcheight) != 0) {
			free(temp);
			return -3;
		}
	}
	else {
		long size, y;
		for (y = 0, size = srcwidth * 4; y < dstheight; y++) {
			memcpy(temp + y * size, srcpix + y * srcpitch, size);
		}
	}

	if (dstwidth < srcwidth) {
		if (ifilter_shrink_x_c(dstpix, temp, dstheight, dstpitch, 
			srcwidth * 4, dstwidth, srcwidth) != 0) {
			free(temp);
			return -4;
		}
	}
	else if (dstwidth > srcwidth) {
		if (ifilter_expand_x_c(dstpix, temp, dstheight, dstpitch, 
			srcwidth * 4, dstwidth, srcwidth) != 0) {
			free(temp);
			return -5;
		}
	}
	else {
		long size, y;
		for (y = 0, size = dstwidth * 4; y < dstheight; y++) {
			memcpy(dstpix + y * dstpitch, temp + y * size, size);
		}
	}

	free(temp);

	return 0;
}


static int ismooth_resize(IBITMAP *dst, const IRECT *rectdst, IBITMAP *src, 
						  const IRECT *rectsrc)
{
	unsigned char *ss;
	unsigned char *dd;
	int dstwidth, dstheight;
	int srcwidth, srcheight;
	int sfmt, dfmt;
	int retval;
	IBITMAP *tmp1, *tmp2;

	sfmt = ibitmap_pixfmt(src);
	dfmt = ibitmap_pixfmt(dst);

	dstwidth = rectdst->right - rectdst->left;
	srcwidth = rectsrc->right - rectsrc->left;
	dstheight = rectdst->bottom - rectdst->top;
	srcheight = rectsrc->bottom - rectsrc->top;

	if (src->bpp == 32 && dst->bpp == 32 && sfmt == dfmt) {
		dd = _ilineptr(dst, rectdst->top) + rectdst->left * 4;
		ss = _ilineptr(src, rectsrc->top) + rectsrc->left * 4;
		retval = ismooth_resize_32(dd, ss, dstwidth, srcwidth, dstheight,
			srcheight, dst->pitch, src->pitch);
		return retval;
	}

	if (src->bpp == 32) {
		tmp1 = ibitmap_create(dstwidth, dstheight, 32);
		if (tmp1 == 0) return -20;
		ibitmap_set_pixfmt(tmp1, sfmt);
		dd = _ilineptr(tmp1, 0);
		ss = _ilineptr(src, rectsrc->top) + rectsrc->left * 4;
		retval = ismooth_resize_32(dd, ss, dstwidth, srcwidth, 
			dstheight, srcheight, tmp1->pitch, src->pitch);
		if (retval == 0) {
			_iconvert_blit(dst, rectdst->left, rectdst->top, tmp1, 
				0, 0, dstwidth, dstheight, NULL, NULL, 0);
		}
		ibitmap_release(tmp1);
		return 0;
	}

	if (dst->bpp == 32) {
		tmp1 = ibitmap_create(srcwidth, srcheight, 32);
		if (tmp1 == NULL) return -30;
		ibitmap_set_pixfmt(tmp1, dfmt);
		_iconvert_blit(tmp1, 0, 0, src, rectsrc->left, rectsrc->top,
			srcwidth, srcheight, NULL, NULL, 0);
		dd = _ilineptr(dst, rectdst->top) + rectdst->left * 4;
		ss = _ilineptr(tmp1, 0);
		retval = ismooth_resize_32(dd, ss, dstwidth, srcwidth,
			dstheight, srcheight, dst->pitch, tmp1->pitch);
		ibitmap_release(tmp1);
		return retval;
	}

	tmp1 = ibitmap_create(dstwidth, dstheight, 32);
	tmp2 = ibitmap_create(srcwidth, srcheight, 32);

	if (tmp1 == NULL || tmp2 == NULL) {
		if (tmp1) ibitmap_release(tmp1);
		if (tmp2) ibitmap_release(tmp2);
		return -40;
	}

	ibitmap_set_pixfmt(tmp1, IPIX_FMT_ARGB32);
	ibitmap_set_pixfmt(tmp2, IPIX_FMT_ARGB32);

	_iconvert_blit(tmp2, 0, 0, src, rectsrc->left, rectsrc->top,
			srcwidth, srcheight, NULL, NULL, 0);

	dd = _ilineptr(tmp1, 0);
	ss = _ilineptr(tmp2, 0);

	retval = ismooth_resize_32(dd, ss, dstwidth, srcwidth,
		dstheight, srcheight, tmp1->pitch, tmp2->pitch);

	if (retval == 0) {
		_iconvert_blit(dst, rectdst->left, rectdst->top, tmp1, 
			0, 0, dstwidth, dstheight, NULL, NULL, 0);
	}

	ibitmap_release(tmp1);
	ibitmap_release(tmp2);

	return retval;
}


//---------------------------------------------------------------------
// Bitmap Nearest / Bilinear / BiCubic Resize
//---------------------------------------------------------------------
#define ibitmap_scale_main(fmt, filter, ofmode, ofcolor) {	\
		unsigned char *dstpix = NULL; \
		unsigned char *srcpix = NULL; \
		int du = ((sw - 1) << 16) / dw; \
		int dv = ((sh - 1) << 16) / dh; \
		int x, y, u, v, srcw, srch; \
		long spitch; \
		ICOLORD mask, c; \
		mask = src->mask; \
		if (flags & IBLIT_HFLIP) du = -du; \
		if (flags & IBLIT_VFLIP) { \
			dv = -dv; \
			v = ((sy + sh - 1) << 16) + 0x8000; \
		}	else { \
			v = (sy << 16) + 0; \
		} \
		srcpix = _ilineptr(src, 0); \
		spitch = (long)src->pitch; \
		srcw = (int)src->w; \
		srch = (int)src->h; \
		for (y = 0; y < dh; y++, v += dv) { \
			dstpix = _ilineptr(dst, dy + y) + dx * IPIX_FMT_SIZE(fmt); \
			if (flags & IBLIT_HFLIP) { \
				u = ((sx + sw - 1) << 16) + 0x6000; \
			}	else { \
				u = (sx << 16) + 0; \
			} \
			for (x = dw; x > 0; x--, u += du) { \
				c = interp_getcol(fmt, filter, u, v, srcpix, spitch, \
					srcw, srch, ofmode, ofcolor); \
				IPIX_FMT_WRITE(fmt, dstpix, c); \
				dstpix += IPIX_FMT_SIZE(fmt); \
			} \
		}	\
	}

#define ibitmap_scale_interp(fmt) \
static void ibitmap_scale_proc_##fmt(IBITMAP *dst, const IRECT *drect, \
	IBITMAP *src, const IRECT *srect, int flags, int filter, int overflow, \
	ICOLORD olfcolor) \
{ \
	int dx = drect->left; \
	int dy = drect->top; \
	int dw = drect->right - drect->left; \
	int dh = drect->bottom - drect->top; \
	int sx = srect->left; \
	int sy = srect->top; \
	int sw = srect->right - srect->left; \
	int sh = srect->bottom - srect->top; \
	switch (filter) \
	{ \
	case IFILTER_NORMAL: \
		ibitmap_scale_main(fmt, normal, overflow, olfcolor); \
		break; \
	case IFILTER_NEAREST: \
		ibitmap_scale_main(fmt, nearest, overflow, olfcolor); \
		break; \
	case IFILTER_TEND: \
	case IFILTER_BILINEAR: \
		ibitmap_scale_main(fmt, bilinear, overflow, olfcolor); \
		break; \
	case IFILTER_BICUBIC: \
		ibitmap_scale_main(fmt, bicubic, overflow, olfcolor); \
		break; \
	case IFILTER_BICUBIC2: \
		ibitmap_scale_main(fmt, bicubic2, overflow, olfcolor); \
		break; \
	} \
}

ibitmap_scale_interp(RGB15);
ibitmap_scale_interp(BGR15);
ibitmap_scale_interp(RGB16);
ibitmap_scale_interp(BGR16);
ibitmap_scale_interp(RGB24);
ibitmap_scale_interp(BGR24);
ibitmap_scale_interp(RGB32);
ibitmap_scale_interp(BGR32);
ibitmap_scale_interp(ARGB32);
ibitmap_scale_interp(ABGR32);
ibitmap_scale_interp(RGBA32);
ibitmap_scale_interp(BGRA32);
ibitmap_scale_interp(ARGB_4444);
ibitmap_scale_interp(ABGR_4444);
ibitmap_scale_interp(RGBA_4444);
ibitmap_scale_interp(BGRA_4444);
ibitmap_scale_interp(ARGB_1555);
ibitmap_scale_interp(ABGR_1555);
ibitmap_scale_interp(RGBA_5551);
ibitmap_scale_interp(BGRA_5551);


//---------------------------------------------------------------------
// 平滑缩放：支持BILINEAR, BICUBIC和SMOOTH几种算法，但无混色
//---------------------------------------------------------------------
int ibitmap_stretch(IBITMAP *dst, const IRECT *rectdst, 
	const IRECT *clip, IBITMAP *src, const IRECT *rectsrc, int flags,
	int filter, int ofmode, ICOLORD ofcol)
{
	int sfmt, dfmt, r, m;
	IRECT drect;
	IRECT srect;

	irect_copy(&drect, rectdst);
	irect_copy(&srect, rectsrc);

	if (src->bpp != dst->bpp) 
		return -100;

	// check clip area
	r = ibitmap_scale_clip(dst, &drect, clip, src, &srect);
	if (r) return r;

	// tend mode can not flip
	if ((flags & (IBLIT_HFLIP | IBLIT_VFLIP)) != 0 && filter == IFILTER_TEND)
		return -200;

	// use tend mode
	if (filter == IFILTER_TEND) {
		ismooth_resize(dst, &drect, src, &srect);
		return 0;
	}

	dfmt = ibitmap_pixfmt(dst);
	sfmt = ibitmap_pixfmt(src);

	// pixel format doesn't support
	if (dfmt != sfmt) 
		return -300;

	// flags indicate to use fast mode
	m = ISTRETCH_FAST | IBLIT_MASK;

	// use fast mode
	if (filter == IFILTER_NORMAL || (flags & m) != 0 || src->bpp == 8) {
		return ibitmap_stretch_fast(dst, &drect, src, &srect, flags);
	}

	// size out of range
	if (srect.right >= 0x7fff || srect.bottom >= 0x7fff ||
		drect.right >= 0x7fff || drect.bottom >= 0x7fff) 
		return -400;

	if (ibicubic_inited == 0) 
		ibicubic_init();
	
	#define ibitmap_interp_case(fmt) \
		case IPIX_FMT_##fmt: ibitmap_scale_proc_##fmt(dst, &drect, \
			src, &srect, flags, filter, ofmode, ofcol); break; 

	switch (sfmt)
	{
		ibitmap_interp_case(RGB15);
		ibitmap_interp_case(BGR15);
		ibitmap_interp_case(RGB16);
		ibitmap_interp_case(BGR16);
		ibitmap_interp_case(RGB24);
		ibitmap_interp_case(BGR24);
		ibitmap_interp_case(RGB32);
		ibitmap_interp_case(BGR32);
		ibitmap_interp_case(ARGB32);
		ibitmap_interp_case(ABGR32);
		ibitmap_interp_case(RGBA32);
		ibitmap_interp_case(BGRA32);
		ibitmap_interp_case(ARGB_4444);
		ibitmap_interp_case(ABGR_4444);
		ibitmap_interp_case(RGBA_4444);
		ibitmap_interp_case(BGRA_4444);
		ibitmap_interp_case(ARGB_1555);
		ibitmap_interp_case(ABGR_1555);
		ibitmap_interp_case(RGBA_5551);
		ibitmap_interp_case(BGRA_5551);
	}

	#undef ibitmap_interp_case

	return 0;
}


//---------------------------------------------------------------------
// 位图重采样
//---------------------------------------------------------------------
IBITMAP *ibitmap_resample(IBITMAP *src, int newwidth, int newheight,
	int filter, int ofmode, ICOLORD ofcolor)
{
	IRECT srect, drect;
	IBITMAP *dst;
	int fmt;

	if (newwidth <= 0 || newheight <= 0 || src == NULL) 
		return NULL;

	dst = ibitmap_create(newwidth, newheight, src->bpp);
	if (dst == NULL) return NULL;

	fmt = ibitmap_pixfmt(src);
	ibitmap_set_pixfmt(dst, fmt);

	irect_set(&drect, 0, 0, newwidth, newheight);
	irect_set(&srect, 0, 0, (int)src->w, (int)src->h);

	ibitmap_stretch(dst, &drect, NULL, src, &srect, 0,
		filter, ofmode, ofcolor);

	return dst;
}


//---------------------------------------------------------------------
// 取得一段扫描线，保存成ARGB32的格式到 output中
// 返回扫描线内所有点的Alpha值的 AND结果
//---------------------------------------------------------------------
int ibitmap_scan(IBITMAP *src, ISCANLINE *scanlines, int count,
	int filter, const IRGB *pal, int om, ICOLORD dc)
{
	const IRGB *_ipaletted = pal;
	unsigned char *pixel, *ptr;
	IUINT32 CC, CMASK;
	IINT32 ww, hh;
	long pitch;
	int pixfmt;
	IINT32 x;
	IINT32 y;

	assert(src && scanlines);

	if (scanlines == NULL || src == NULL) {
		abort();
		return 0;
	}

	ptr = (unsigned char*)src->pixel;
	pitch = (long)src->pitch;
	pixfmt = ibitmap_pixfmt(src);
	ww = (IINT32)src->w;
	hh = (IINT32)src->h;
	x = 0;
	y = 0;
	CMASK = (IUINT32)src->mask;

	if (ibicubic_inited == 0) 
		ibicubic_init();

	#define ITEX_GETCOL_MASK(c, fmt, u, v, ptr, pitch, w, h, om, dc) do { \
		ICOLORD raw, r, g, b, a; \
		IINT32 ui = u >> 16; \
		IINT32 vi = v >> 16; \
		if (ioverflow_pos(&ui, &vi, w, h, om) == 0) { \
			size_t pos = vi * pitch + ui * IPIX_FMT_SIZE(fmt); \
			const ICOLORB *p = ptr + pos; \
			raw = IPIX_FMT_READ(fmt, p); \
			if (raw != CMASK) { \
				IRGBA_FROM_PIXEL(raw, fmt, r, g, b, a); \
				c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
			}	else { \
				c = 0; \
			} \
		}	else { \
			c = dc; \
		} \
	}	while (0)

	#define IBITMAP_SCAN_SPAN(fmt, mode) do { \
		for (; width > 0; width--) { \
			ITEX_GETCOL_##mode(CC, fmt, U, V, ptr, pitch, ww, hh, om, dc); \
			_ipixel_put(4, pixel, CC); \
			magic &= (CC >> 24) & 0xff; \
			pixel += 4; \
			U += DU; \
			V += DV; \
		} \
	}	while (0)
	
	#define IBITMAP_SCAN_MAIN(fmt, mode) do { \
		for (; count > 0; count--, scanlines++) { \
			IINT32 U = scanlines->u + x; \
			IINT32 V = scanlines->v + y; \
			IINT32 DU = scanlines->du; \
			IINT32 DV = scanlines->dv; \
			int width = scanlines->width; \
			int magic = 0xff; \
			pixel = scanlines->pixel; \
			IBITMAP_SCAN_SPAN(fmt, mode); \
			scanlines->magic = magic; \
		} \
	}	while (0)
	
	#define IBITMAP_SCAN_LINE(fmt) do { \
		switch (filter) { \
		case IFILTER_MASK: IBITMAP_SCAN_MAIN(fmt, MASK); break; \
		case IFILTER_NEAREST: x = 0x8000; y = 0x8000; \
		case IFILTER_NORMAL: IBITMAP_SCAN_MAIN(fmt, NORMAL); break; \
		case IFILTER_TEND: \
		case IFILTER_BILINEAR: IBITMAP_SCAN_MAIN(fmt, BILINEAR); break; \
		case IFILTER_BICUBIC: IBITMAP_SCAN_MAIN(fmt, BICUBIC1); break; \
		case IFILTER_BICUBIC2: IBITMAP_SCAN_MAIN(fmt, BICUBIC2); break; \
		} \
	}	while (0)

	switch (pixfmt)
	{
	case IPIX_FMT_8: IBITMAP_SCAN_LINE(8); break;
	case IPIX_FMT_RGB15: IBITMAP_SCAN_LINE(RGB15); break;
	case IPIX_FMT_BGR15: IBITMAP_SCAN_LINE(BGR15); break;
	case IPIX_FMT_RGB16: IBITMAP_SCAN_LINE(RGB16); break;
	case IPIX_FMT_BGR16: IBITMAP_SCAN_LINE(BGR16); break;
	case IPIX_FMT_RGB24: IBITMAP_SCAN_LINE(RGB24); break;
	case IPIX_FMT_BGR24: IBITMAP_SCAN_LINE(BGR24); break;
	case IPIX_FMT_RGB32: IBITMAP_SCAN_LINE(RGB32); break;
	case IPIX_FMT_BGR32: IBITMAP_SCAN_LINE(BGR32); break;
	case IPIX_FMT_ARGB32: IBITMAP_SCAN_LINE(ARGB32); break;
	case IPIX_FMT_ABGR32: IBITMAP_SCAN_LINE(ABGR32); break;
	case IPIX_FMT_RGBA32: IBITMAP_SCAN_LINE(RGBA32); break;
	case IPIX_FMT_BGRA32: IBITMAP_SCAN_LINE(BGRA32); break;
	case IPIX_FMT_ARGB_4444: IBITMAP_SCAN_LINE(ARGB_4444); break;
	case IPIX_FMT_ABGR_4444: IBITMAP_SCAN_LINE(ABGR_4444); break;
	case IPIX_FMT_RGBA_4444: IBITMAP_SCAN_LINE(RGBA_4444); break;
	case IPIX_FMT_BGRA_4444: IBITMAP_SCAN_LINE(BGRA_4444); break;
	case IPIX_FMT_ARGB_1555: IBITMAP_SCAN_LINE(ARGB_1555); break;
	case IPIX_FMT_ABGR_1555: IBITMAP_SCAN_LINE(ABGR_1555); break;
	case IPIX_FMT_RGBA_5551: IBITMAP_SCAN_LINE(RGBA_5551); break;
	case IPIX_FMT_BGRA_5551: IBITMAP_SCAN_LINE(BGRA_5551); break;
	}

	#undef IBITMAP_SCAN_LINE
	#undef IBITMAP_SCAN_MAIN
	#undef IBITMAP_SCAN_SPAN
	
	return 0;
}


/*
ibmint.h
ibmint.c
*/

