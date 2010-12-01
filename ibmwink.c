//=====================================================================
//
// ibmwink.c - 
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ibmwink.h"


//---------------------------------------------------------------------
// Simple Clip
//---------------------------------------------------------------------



//---------------------------------------------------------------------
// Bitmap Clip
//---------------------------------------------------------------------
void ibitmap_clip_create(IBITMAP *bmp, int cl, int ct, int cr, int cb)
{
	int *ptr;
	assert(bmp);
	if (_ibitmap_clip(bmp) == NULL) {
		ptr = (int*)malloc(sizeof(int) * 4);
		assert(ptr);
		_ibitmap_set_clip(bmp, ptr);
	}
	ptr = _ibitmap_clip(bmp);
	if (cl < 0) cl = 0;
	if (ct < 0) ct = 0;
	if (cr > (int)bmp->w) cr = (int)bmp->w;
	if (cb > (int)bmp->h) cb = (int)bmp->h;
	if (cr < cl) cr = cl;
	if (cb < ct) cb = ct;
	ptr[0] = cl;
	ptr[1] = ct;
	ptr[2] = cr;
	ptr[3] = cb;
}

void ibitmap_clip_delete(IBITMAP *bmp)
{
	if (_ibitmap_clip(bmp) != NULL) {
		free(_ibitmap_clip(bmp));
		_ibitmap_set_clip(bmp, NULL);
	}
}

static inline void ibitmap_get_clip(const IBITMAP *bmp, int *cl, int *ct, 
	int *cr, int *cb) 
{
	int *clip = _ibitmap_clip(bmp);
	if (clip == NULL) {
		*cl = 0;
		*ct = 0;
		*cr = (int)bmp->w;
		*cb = (int)bmp->h;
	}	else {
		*cl = *clip++;
		*ct = *clip++;
		*cr = *clip++;
		*cb = *clip++;
	}
}


//---------------------------------------------------------------------
// Pixel Macros
//---------------------------------------------------------------------
#define ibitmap_getdot_declare(bpp) \
	static inline ICOLORD \
	ibitmap_getdot##bpp(const IBITMAP *bmp, int x, int y) { \
		int cl, ct, cr, cb; \
		ibitmap_get_clip(bmp, &cl, &ct, &cr, &cb); \
		if (x >= cl && y >= ct && x < cr && y < cb) \
			return _iget##bpp(bmp, x, y); \
		return 0; \
	}

#define ibitmap_putdot_declare(bpp) \
	static inline void \
	ibitmap_putdot##bpp(IBITMAP *bmp, int x, int y, ICOLORD c) { \
		int cl, ct, cr, cb; \
		ibitmap_get_clip(bmp, &cl, &ct, &cr, &cb); \
		if (x >= cl && y >= ct && x < cr && y < cb) \
			_iput##bpp(bmp, x, y, c); \
	}

ibitmap_getdot_declare(8);
ibitmap_getdot_declare(15);
ibitmap_getdot_declare(16);
ibitmap_getdot_declare(24);
ibitmap_getdot_declare(32);

ibitmap_putdot_declare(8);
ibitmap_putdot_declare(15);
ibitmap_putdot_declare(16);
ibitmap_putdot_declare(24);
ibitmap_putdot_declare(32);


// put a dot
#define ibitmap_getdot(bmp, bpp, x, y) \
		ibitmap_getdot##bpp(bmp, x, y)

// get a dot
#define ibitmap_putdot(bmp, bpp, x, y, c) \
		ibitmap_putdot##bpp(bmp, x, y, c)

// get raw color
static inline ICOLORD ibitmap_rawcol(IBITMAP *bmp, ICOLORD argb) {
	int pixfmt;
	if (_ibitmap_pixfmt(bmp) == 0) ibitmap_set_pixfmt(bmp, 0);
	pixfmt = _ibitmap_pixfmt(bmp);
	if (pixfmt == IPIX_FMT_8) {
		return (argb & 0xff);
	}
	return _im_color_set(pixfmt, argb, NULL);
}

// abs
#define _iabs(x)    ( ((x) < 0) ? (-(x)) : (x) )
#define _imin(x, y) ( ((x) < (y)) ? (x) : (y) )
#define _imax(x, y) ( ((x) > (y)) ? (x) : (y) )


//---------------------------------------------------------------------
// this line clipping based heavily off of code from
// http://www.ncsa.uiuc.edu/Vis/Graphics/src/clipCohSuth.c 
//---------------------------------------------------------------------
#define LEFT_EDGE   0x1
#define RIGHT_EDGE  0x2
#define BOTTOM_EDGE 0x4
#define TOP_EDGE    0x8
#define INSIDE(a)   (!a)
#define REJECT(a,b) (a&b)
#define ACCEPT(a,b) (!(a|b))

static inline int _iencode(int x, int y, int left, int top, int right, 
	int bottom)
{
	int code = 0;
	if (x < left)   code |= LEFT_EDGE;
	if (x > right)  code |= RIGHT_EDGE;
	if (y < top)    code |= TOP_EDGE;
	if (y > bottom) code |= BOTTOM_EDGE;
	return code;
}

static inline int _iencodeFloat(float x, float y, int left, int top, 
	int right, int bottom)
{
	int code = 0;
	if (x < left)   code |= LEFT_EDGE;
	if (x > right)  code |= RIGHT_EDGE;
	if (y < top)    code |= TOP_EDGE;
	if (y > bottom) code |= BOTTOM_EDGE;
	return code;
}

static inline int _iclipline(int* pts, int left, int top, int right, 
	int bottom)
{
	int x1 = pts[0];
	int y1 = pts[1];
	int x2 = pts[2];
	int y2 = pts[3];
	int code1, code2;
	int draw = 0;
	int swaptmp;
	float m; /*slope*/

	right--;
	bottom--;

	while(1)
	{
		code1 = _iencode(x1, y1, left, top, right, bottom);
		code2 = _iencode(x2, y2, left, top, right, bottom);
		if (ACCEPT(code1, code2)) {
			draw = 1;
			break;
		}
		else if (REJECT(code1, code2))
			break;
		else {
			if (INSIDE(code1)) {
				swaptmp = x2; x2 = x1; x1 = swaptmp;
				swaptmp = y2; y2 = y1; y1 = swaptmp;
				swaptmp = code2; code2 = code1; code1 = swaptmp;
			}
			if (x2 != x1)
				m = (y2 - y1) / (float)(x2 - x1);
			else
				m = 1.0f;
			if (code1 & LEFT_EDGE) {
				y1 += (int)((left - x1) * m);
				x1 = left;
			}
			else if (code1 & RIGHT_EDGE) {
				y1 += (int)((right - x1) * m);
				x1 = right;
			}
			else if(code1 & BOTTOM_EDGE) {
				if(x2 != x1)
					x1 += (int)((bottom - y1) / m);
				y1 = bottom;
			}
			else if (code1 & TOP_EDGE) {
				if (x2 != x1)
					x1 += (int)((top - y1) / m);
				y1 = top;
			}
		}
	}
	if (draw) {
		pts[0] = x1; pts[1] = y1;
		pts[2] = x2; pts[3] = y2;
	}
	return draw;
}


#undef INSIDE
#undef REJECT
#undef ACCEPT

//---------------------------------------------------------------------
// Simple Drawing
//---------------------------------------------------------------------
void ibitmap_setpixel(IBITMAP *bmp, int x, int y, ICOLORD rgb)
{
	ICOLORD raw = ibitmap_rawcol(bmp, rgb);
	switch (bmp->bpp) 
	{
	case  8: ibitmap_putdot(bmp,  8, x, y, raw); break;
	case 15: ibitmap_putdot(bmp, 15, x, y, raw); break;
	case 16: ibitmap_putdot(bmp, 16, x, y, raw); break;
	case 24: ibitmap_putdot(bmp, 24, x, y, raw); break;
	case 32: ibitmap_putdot(bmp, 32, x, y, raw); break;
	}
}


ICOLORD ibitmap_getpixel(const IBITMAP *bmp, int x, int y)
{
	ICOLORD raw = 0;
	switch (bmp->bpp) 
	{
	case  8: raw = ibitmap_getdot(bmp,  8, x, y); break;
	case 15: raw = ibitmap_getdot(bmp, 15, x, y); break;
	case 16: raw = ibitmap_getdot(bmp, 16, x, y); break;
	case 24: raw = ibitmap_getdot(bmp, 24, x, y); break;
	case 32: raw = ibitmap_getdot(bmp, 32, x, y); break;
	}
	if (_ibitmap_pixfmt(bmp) == 0) ibitmap_set_pixfmt((IBITMAP*)bmp, 0);
	if (_ibitmap_pixfmt(bmp) == IPIX_FMT_8) {
		return raw;
	}
	return _im_color_get(_ibitmap_pixfmt(bmp), raw, NULL);
}


// Simple Line Macro
#define _ibitmap_simple_line(bmp, x1, y1, x2, y2, putfunc, c) { \
		int x, y, p, n, tn; \
		if (y1 == y2) {		\
			if (x1 > x2) x = x1, x1 = x2, x2 = x;	\
			for (x = x1; x <= x2; x++) putfunc(bmp, x, y1, c);	\
		}	\
		else if (x1 == x2)	{  \
			if (y1 > y2) y = y2, y2 = y1, y1 = y;	\
			for (y = y1; y <= y2; y++) putfunc(bmp, x1, y, c);	\
		}	\
		else if (_iabs(y2 - y1) <= _iabs(x2 - x1)) {	\
			if ((y2 < y1 && x2 < x1) || (y1 <= y2 && x1 > x2)) {	\
				x = x2, y = y2, x2 = x1, y2 = y1, x1 = x, y1 = y;	\
			}	\
			if (y2 >= y1 && x2 >= x1) {	\
				x = x2 - x1, y = y2 - y1;	\
				p = 2 * y, n = 2 * x - 2 * y, tn = x;	\
				for (; x1 <= x2; x1++) {	\
					if (tn >= 0) tn -= p;	\
					else tn += n, y1++;	\
					putfunc(bmp, x1, y1, c);	\
				}	\
			}	else {	\
				x = x2 - x1; y = y2 - y1;	\
				p = -2 * y; n = 2 * x + 2 * y; tn = x;	\
				for (; x1 <= x2; x1++) {	\
					if (tn >= 0) tn -= p;	\
					else tn += n, y1--;	\
					putfunc(bmp, x1, y1, c);	\
				}	\
			}	\
		}	else {	\
			x = x1; x1 = y2; y2 = x; y = y1; y1 = x2; x2 = y;	\
			if ((y2 < y1 && x2 < x1) || (y1 <= y2 && x1 > x2)) {	\
				x = x2, y = y2, x2 = x1, x1 = x, y2 = y1, y1 = y;	\
			}	\
			if (y2 >= y1 && x2 >= x1) {	\
				x = x2 - x1; y = y2 - y1;	\
				p = 2 * y; n = 2 * x - 2 * y; tn = x;	\
				for (; x1 <= x2; x1++)  {	\
					if (tn >= 0) tn -= p;	\
					else tn += n, y1++;	\
					putfunc(bmp, y1, x1, c);	\
				}	\
			}	else	{	\
				x = x2 - x1; y = y2 - y1;	\
				p = -2 * y; n = 2 * x + 2 * y; tn = x;	\
				for (; x1 <= x2; x1++) {	\
					if (tn >= 0) tn -= p;	\
					else { tn += n; y1--; }	\
					putfunc(bmp, y1, x1, c);	\
				}	\
			}	\
		}	\
	}


#define _line_putdot(bpp) _iput##bpp



void ibitmap_line8(IBITMAP *bmp, int x1, int y1, int x2, int y2, ICOLORD c) {
	_ibitmap_simple_line(bmp, x1, y1, x2, y2, _line_putdot(8), c); 
}

void ibitmap_line15(IBITMAP *bmp, int x1, int y1, int x2, int y2, ICOLORD c) {
	_ibitmap_simple_line(bmp, x1, y1, x2, y2, _line_putdot(15), c); 
}

void ibitmap_line16(IBITMAP *bmp, int x1, int y1, int x2, int y2, ICOLORD c) {
	_ibitmap_simple_line(bmp, x1, y1, x2, y2, _line_putdot(16), c); 
}

void ibitmap_line24(IBITMAP *bmp, int x1, int y1, int x2, int y2, ICOLORD c) {
	_ibitmap_simple_line(bmp, x1, y1, x2, y2, _line_putdot(24), c); 
}

void ibitmap_line32(IBITMAP *bmp, int x1, int y1, int x2, int y2, ICOLORD c) {
	_ibitmap_simple_line(bmp, x1, y1, x2, y2, _line_putdot(32), c); 
}


// drawline
void ibitmap_line(IBITMAP *bmp, int x1, int y1, int x2, int y2, ICOLORD c)
{
	int cl, ct, cr, cb;
	ICOLORD raw;
	int pts[4];

	raw = ibitmap_rawcol(bmp, c);
	pts[0] = x1; 
	pts[1] = y1;
	pts[2] = x2;
	pts[3] = y2;
	ibitmap_get_clip(bmp, &cl, &ct, &cr, &cb);
	_iclipline(pts, cl, ct, cr, cb);
	x1 = pts[0];
	y1 = pts[1];
	x2 = pts[2];
	y2 = pts[3];
	switch (bmp->bpp)
	{
	case  8: ibitmap_line8 (bmp, x1, y1, x2, y2, raw); break;
	case 15: ibitmap_line15(bmp, x1, y1, x2, y2, raw); break;
	case 16: ibitmap_line16(bmp, x1, y1, x2, y2, raw); break;
	case 24: ibitmap_line24(bmp, x1, y1, x2, y2, raw); break;
	case 32: ibitmap_line32(bmp, x1, y1, x2, y2, raw); break;
	}
}

// horizen line
void ibitmap_hline(IBITMAP *bmp, int left, int top, int right, ICOLORD col)
{
	unsigned long len;
	unsigned char *p8;

	assert(bmp);

	if ((long)right <= 0 || left >= (long)bmp->w) return;
	if (top < 0 || top >= (long)bmp->h) return;

	left = (left < 0)? 0 : left;
	right= (right> (long)bmp->w)? bmp->w : right;
	len = (unsigned long)(right - left);

	switch (bmp->bpp)
	{
	case 8:
		p8 = _ilineptr(bmp, top) + left;
		for (; len > 0; len--, p8 += 1) _im_put1b(p8, col);
		break;
	case 15:
	case 16:
		p8 = _ilineptr(bmp, top) + left * 2;
		for (; len > 0; len--, p8 += 2) _im_put2b(p8, col);
		break;
	case 24:
		p8 = _ilineptr(bmp, top) + left * 3;
		for (; len > 0; len--, p8 += 3) _im_put3b(p8, col);
		break;
	case 32:
		p8 = _ilineptr(bmp, top) + left * 4;
		for (; len > 0; len--, p8 += 4) _im_put4b(p8, col);
		break;
	}
}

// 
#define _ibitmap_simple_circle(bmp, x0, y0, r, col, bpp, isfilled) { \
		int cx = 0, cy = r;		\
		int df = 1 - r, d_e = 3, d_se = -2 * r + 5;		\
		int x = 0, y = 0, xmax, tn;		\
		switch (isfilled) {	\
		case 0:			\
			y = r; x = 0; xmax = (int)(r * 0.70710678f + 0.5f);		\
			tn = (1 - r * 2);	\
			while (x <= xmax) {		\
				if (tn >= 0) {		\
					tn += (6 + ((x - y) << 2));	\
					y--;	\
				} else tn += ((x << 2) + 2);	\
				ibitmap_putdot(bmp, bpp, x0 + y, y0 + x, col);	\
				ibitmap_putdot(bmp, bpp, x0 + x, y0 + y, col);	\
				ibitmap_putdot(bmp, bpp, x0 - x, y0 + y, col);	\
				ibitmap_putdot(bmp, bpp, x0 - y, y0 + x, col);	\
				ibitmap_putdot(bmp, bpp, x0 - y, y0 - x, col);	\
				ibitmap_putdot(bmp, bpp, x0 - x, y0 - y, col);	\
				ibitmap_putdot(bmp, bpp, x0 + x, y0 - y, col);	\
				ibitmap_putdot(bmp, bpp, x0 + y, y0 - x, col);	\
				x++;	\
			}	\
			break;	\
		case 1:		\
			do {	\
				ibitmap_hline(bmp, x0 - cy, y0 - cx,		\
					x0 - cy + (cy << 1) + 1, col);	\
				if (cx) ibitmap_hline(bmp, x0 - cy, y0 + cx, \
					x0 - cy + (cy << 1) + 1, col);	\
				if (df < 0) df += d_e, d_e += 2, d_se += 2;		\
				else {	\
					if (cx != cy) {		\
						ibitmap_hline(bmp, x0 - cx, y0 - cy, \
								x0 - cx + (cx << 1) + 1, col);	\
						if (cy) ibitmap_hline(bmp, x0 - cx, \
							y0 + cy, x0 - cx + (cx << 1) + 1, col);	\
					}	\
					df += d_se, d_e += 2, d_se += 4;	\
					cy--;	\
				}	\
				cx++;	\
			}	while (cx <= cy);	\
			break;	\
		}	\
	}


void ibitmap_circle(IBITMAP *bmp, int x0, int y0, int r, ICOLORD c, int fill)
{
	ICOLORD raw = ibitmap_rawcol(bmp, c);
	switch (bmp->bpp)
	{
	case  8: _ibitmap_simple_circle(bmp, x0, y0, r, raw,  8, fill); break;
	case 15: _ibitmap_simple_circle(bmp, x0, y0, r, raw, 15, fill); break;
	case 16: _ibitmap_simple_circle(bmp, x0, y0, r, raw, 16, fill); break;
	case 24: _ibitmap_simple_circle(bmp, x0, y0, r, raw, 24, fill); break;
	case 32: _ibitmap_simple_circle(bmp, x0, y0, r, raw, 32, fill); break;
	}
}

void ibitmap_rect(IBITMAP *bmp, int x, int y, int w, int h, ICOLORD c)
{
	ICOLORD raw = ibitmap_rawcol(bmp, c);
	ibitmap_fill(bmp, x, y, w, h, raw, 0);
}

void ibitmap_box(IBITMAP *bmp, int x, int y, int w, int h, ICOLORD c)
{
	ibitmap_line(bmp, x, y, x + w - 1, y, c);
	ibitmap_line(bmp, x, y + h - 1, x + w - 1, y + h - 1, c);
	ibitmap_line(bmp, x, y, x, y + h - 1, c);
	ibitmap_line(bmp, x + w - 1, y, x + w - 1, y + h - 1, c);
}




