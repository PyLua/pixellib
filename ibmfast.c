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
int ibitmap_resize(IBITMAP *dst, const IRECT *rectdst, const IRECT *clip,
	const IBITMAP *src, const IRECT *rectsrc, int flags)
{
	int dx, dy, dw, dh, sx, sy, sw, sh, r;
	IRECT drect;
	IRECT srect;

	irect_copy(&drect, rectdst);
	irect_copy(&srect, rectsrc);

	if (src->bpp != dst->bpp) 
		return -100;

	r = ibitmap_scale_clip(dst, &drect, clip, src, &srect);
	if (r) return r;

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
			int du = (sw << 16) / dw; \
			int dv = (sh << 16) / dh; \
			int x, y, u, v; \
			int lineno; \
			ICOLORD mask, c; \
			mask = src->mask; \
			if (flags & IBLIT_HFLIP) du = -du; \
			if (flags & IBLIT_VFLIP) dv = -dv; \
			for (y = 0, v = 0x8000; y < dh; y++, v += dv) { \
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

	IINT64T zrecip = 1;
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

	IINT64T zrecip = 1;
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
		xmult0[x] = 0x10000 * ((x * (srcwidth - 1)) % dstwidth) / dstwidth;
		xmult1[x] = 0x10000 - xmult1[x];
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
		ICOLORB *temp = (ICOLORB*)malloc((long)srcwidth * dstheight * 4);
		if (temp == NULL) return -4;
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


