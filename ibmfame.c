//=====================================================================
//
// ibmfame.c - alpha blending
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================
#include "ibmfame.h"

//=====================================================================
// 混色部分
//=====================================================================


//---------------------------------------------------------------------
// 外部混色驱动
//---------------------------------------------------------------------
IBLEND_PROC iblend_proc[24][24] = {
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

//---------------------------------------------------------------------
// 裁剪矩形的接口（在ibmcols.c中定义的）
//---------------------------------------------------------------------
#ifndef __cplusplus
int ibitmap_blitclip(const IBITMAP *dst, int *dx, int *dy, 
	const IBITMAP *src, int *sx, int *sy, int *sw, int *sh, int flags);
#else
extern "C" int ibitmap_blitclip(const IBITMAP *dst, int *dx, int *dy, 
	const IBITMAP *src, int *sx, int *sy, int *sw, int *sh, int flags);
#endif

//---------------------------------------------------------------------
// C版本快速混色循环
//---------------------------------------------------------------------
#define _iblend_loop_body(dfmt, sfmt, BODY) { \
		if ((flags & IBLIT_MASK) == 0) { \
			if (changecol == 0) { \
				for (y = 0; y < h; y++, k += v) { \
					int r1, g1, b1, a1, r2, g2, b2, a2; \
					ICOLORD c1, c2; \
					s = _ilineptr(src, sy + k) + sdelta; \
					d = _ilineptr(dst, dy + y) + ddelta; \
					for (x = 0; x < w; s += incs, d += dsize, x++) { \
						c1 = IPIX_FMT_READ(sfmt, s); \
						c2 = IPIX_FMT_READ(dfmt, d); \
						IRGBA_FROM_PIXEL(c1, sfmt, r1, g1, b1, a1); \
						IRGBA_FROM_PIXEL(c2, dfmt, r2, g2, b2, a2); \
						BODY; \
						c2 = IRGBA_TO_PIXEL(dfmt, r2, g2, b2, a2); \
						IPIX_FMT_WRITE(dfmt, d, c2); \
					}	\
				} \
			}	else { \
				for (y = 0; y < h; y++, k += v) { \
					int r1, g1, b1, a1, r2, g2, b2, a2; \
					ICOLORD c1, c2; \
					s = _ilineptr(src, sy + k) + sdelta; \
					d = _ilineptr(dst, dy + y) + ddelta; \
					for (x = 0; x < w; s += incs, d += dsize, x++) { \
						c1 = IPIX_FMT_READ(sfmt, s); \
						c2 = IPIX_FMT_READ(dfmt, d); \
						IRGBA_FROM_PIXEL(c1, sfmt, r1, g1, b1, a1); \
						IRGBA_FROM_PIXEL(c2, dfmt, r2, g2, b2, a2); \
						r1 = (r1 * cr) >> 8; \
						g1 = (g1 * cg) >> 8; \
						b1 = (b1 * cb) >> 8; \
						BODY; \
						c2 = IRGBA_TO_PIXEL(dfmt, r2, g2, b2, a2); \
						IPIX_FMT_WRITE(dfmt, d, c2); \
					}	\
				} \
			} \
		}	else { \
			if (changecol == 0) { \
				for (y = 0; y < h; y++, k += v) { \
					int r1, g1, b1, a1, r2, g2, b2, a2; \
					ICOLORD c1, c2; \
					s = _ilineptr(src, sy + k) + sdelta; \
					d = _ilineptr(dst, dy + y) + ddelta; \
					for (x = 0; x < w; s += incs, d += dsize, x++) { \
						c1 = IPIX_FMT_READ(sfmt, s); \
						if (c1 == mask) continue; \
						c2 = IPIX_FMT_READ(dfmt, d); \
						IRGBA_FROM_PIXEL(c1, sfmt, r1, g1, b1, a1); \
						IRGBA_FROM_PIXEL(c2, dfmt, r2, g2, b2, a2); \
						BODY; \
						c2 = IRGBA_TO_PIXEL(dfmt, r2, g2, b2, a2); \
						IPIX_FMT_WRITE(dfmt, d, c2); \
					}	\
				} \
			}	else { \
				for (y = 0; y < h; y++, k += v) { \
					int r1, g1, b1, a1, r2, g2, b2, a2; \
					ICOLORD c1, c2; \
					s = _ilineptr(src, sy + k) + sdelta; \
					d = _ilineptr(dst, dy + y) + ddelta; \
					for (x = 0; x < w; s += incs, d += dsize, x++) { \
						c1 = IPIX_FMT_READ(sfmt, s); \
						if (c1 == mask) continue; \
						c2 = IPIX_FMT_READ(dfmt, d); \
						IRGBA_FROM_PIXEL(c1, sfmt, r1, g1, b1, a1); \
						IRGBA_FROM_PIXEL(c2, dfmt, r2, g2, b2, a2); \
						r1 = (r1 * cr) >> 8; \
						g1 = (g1 * cg) >> 8; \
						b1 = (b1 * cb) >> 8; \
						BODY; \
						c2 = IRGBA_TO_PIXEL(dfmt, r2, g2, b2, a2); \
						IPIX_FMT_WRITE(dfmt, d, c2); \
					}	\
				} \
			} \
		}	\
	}



//---------------------------------------------------------------------
// C版本快速混色主体
//---------------------------------------------------------------------
#define _iblend_main_body(dfmt, sfmt, blendmode) \
{ \
	long ssize, dsize, incs;  \
	long sdelta, ddelta; \
	unsigned char *s, *d; \
	int cr, cg, cb, ca; \
	int x, y, k, v; \
	int destalpha; \
	int changecol; \
	ICOLORD mask; \
	IRGBA_FROM_PIXEL(color, ARGB32, cr, cg, cb, ca); \
	if (cr == cg && cg == cb) { \
		ca = ca * cr / 255; \
		cr = cg = cb = 255; \
	} \
	ssize = (src->bpp + 7) >> 3; \
	dsize = (dst->bpp + 7) >> 3; \
	incs = ssize; \
	sdelta = sx * ssize; \
	ddelta = dx * dsize; \
	if (flags & IBLIT_HFLIP) { \
		sdelta = ssize * (sx + w - 1);  \
		incs = -ssize;  \
	} \
	if (flags & IBLIT_VFLIP) k = h - 1, v = -1;  \
	else k = 0, v = 1; \
	mask = src->mask; \
	destalpha = ipixel_fmt[IPIX_FMT_##dfmt].has_alpha; \
	ca = _iblend_normalize(ca); \
	cr = _iblend_normalize(cr); \
	cg = _iblend_normalize(cg); \
	cb = _iblend_normalize(cb); \
	changecol = (cr != cg || cg != cb)? 1 : 0; \
	if (ca == 256) { \
		_iblend_loop_body(dfmt, sfmt, { \
			if (a1 == 0) continue; \
			else if (a1 == 255) { \
				r2 = r1; g2 = g1; b2 = b1; a2 = 255; \
			}	else { \
				IBLEND_##blendmode(r1, g1, b1, a1, r2, g2, b2, a2); \
			}	\
		}); \
	}	\
	else if (ca > 0) { \
		_iblend_loop_body(dfmt, sfmt, { \
			a1 = (a1 * ca) >> 8; \
			if (a1 == 0) continue; \
			else if (a1 == 255) { \
				r2 = r1; g2 = g1; b2 = b1; a2 = 255; \
			}	else { \
				IBLEND_##blendmode(r1, g1, b1, a1, r2, g2, b2, a2); \
			}	\
		}); \
	}	\
}


//---------------------------------------------------------------------
// C版本静态混色主体
//---------------------------------------------------------------------
#define _iblend_main_static(dfmt, sfmt, blendmode) \
{ \
	long ssize, dsize, incs;  \
	long sdelta, ddelta; \
	unsigned char *s, *d; \
	int cr, cg, cb, ca; \
	int x, y, k, v; \
	int destalpha; \
	int changecol; \
	ICOLORD mask; \
	IRGBA_FROM_PIXEL(color, ARGB32, cr, cg, cb, ca); \
	if (cr == cg && cg == cb) { \
		ca = ca * cr / 255; \
		cr = cg = cb = 255; \
	} \
	ssize = (src->bpp + 7) >> 3; \
	dsize = (dst->bpp + 7) >> 3; \
	incs = ssize; \
	sdelta = sx * ssize; \
	ddelta = dx * dsize; \
	if (flags & IBLIT_HFLIP) { \
		sdelta = ssize * (sx + w - 1);  \
		incs = -ssize;  \
	} \
	if (flags & IBLIT_VFLIP) k = h - 1, v = -1;  \
	else k = 0, v = 1; \
	mask = src->mask; \
	destalpha = ipixel_fmt[IPIX_FMT_##dfmt].has_alpha; \
	ca = _iblend_normalize(ca); \
	cr = _iblend_normalize(cr); \
	cg = _iblend_normalize(cg); \
	cb = _iblend_normalize(cb); \
	changecol = (cr != cg || cg != cb)? 1 : 0; \
	if (ca == 256) { \
		_iblend_loop_body(dfmt, sfmt, { \
			r2 = r1; g2 = g1; b2 = b1; a2 = 255; \
		}); \
	}	\
	else if (ca > 0) { \
		int alpha = (color >> 24) & 0xff; \
		_iblend_loop_body(dfmt, sfmt, { \
			IBLEND_##blendmode(r1, g1, b1, alpha, r2, g2, b2, a2); \
		}); \
	}	\
}

// 默认混色声明
#define _iblend_to_xx_from_xx(dfmt, sfmt, mode) \
	case IPIX_FMT_##sfmt: { _iblend_main_body(dfmt, sfmt, mode); break; }

// 静态混色声明
#define _iblend_to_xx_from_xx_static(dfmt, sfmt, mode) \
	case IPIX_FMT_##sfmt: { _iblend_main_static(dfmt, sfmt, mode); break; }

// 混色到XX的函数
#define _iblend_to_xx(dfmt, mode) \
static int _iblend_to_##dfmt(IBITMAP *dst, int dx, int dy, \
	IBITMAP *src, int sx, int sy, int w, int h, ICOLORD color, int flags) \
{ \
	int sfmt = _ibitmap_pixfmt(src); \
	switch (sfmt) { \
	_iblend_to_xx_from_xx(dfmt, ARGB32, mode); \
	_iblend_to_xx_from_xx(dfmt, ABGR32, mode); \
	_iblend_to_xx_from_xx(dfmt, RGBA32, mode); \
	_iblend_to_xx_from_xx(dfmt, BGRA32, mode); \
	_iblend_to_xx_from_xx(dfmt, ARGB_4444, mode); \
	_iblend_to_xx_from_xx(dfmt, ABGR_4444, mode); \
	_iblend_to_xx_from_xx(dfmt, RGBA_4444, mode); \
	_iblend_to_xx_from_xx(dfmt, BGRA_4444, mode); \
	_iblend_to_xx_from_xx(dfmt, ARGB_1555, mode); \
	_iblend_to_xx_from_xx(dfmt, ABGR_1555, mode); \
	_iblend_to_xx_from_xx(dfmt, RGBA_5551, mode); \
	_iblend_to_xx_from_xx(dfmt, BGRA_5551, mode); \
	_iblend_to_xx_from_xx_static(dfmt, RGB15, mode); \
	_iblend_to_xx_from_xx_static(dfmt, BGR15, mode); \
	_iblend_to_xx_from_xx_static(dfmt, RGB16, mode); \
	_iblend_to_xx_from_xx_static(dfmt, BGR16, mode); \
	_iblend_to_xx_from_xx_static(dfmt, RGB24, mode); \
	_iblend_to_xx_from_xx_static(dfmt, BGR24, mode); \
	_iblend_to_xx_from_xx_static(dfmt, RGB32, mode); \
	_iblend_to_xx_from_xx_static(dfmt, BGR32, mode); \
	} \
	return 0; \
}



//#define TINYDEBUG

#ifndef TINYDEBUG
_iblend_to_xx(RGB15, STATIC);
_iblend_to_xx(BGR15, STATIC);
_iblend_to_xx(RGB16, STATIC);
_iblend_to_xx(BGR16, STATIC);
_iblend_to_xx(RGB24, STATIC);
_iblend_to_xx(BGR24, STATIC);
_iblend_to_xx(RGB32, STATIC);
_iblend_to_xx(BGR32, STATIC);
_iblend_to_xx(ARGB32, NORMAL);
_iblend_to_xx(ABGR32, NORMAL);
_iblend_to_xx(RGBA32, NORMAL);
_iblend_to_xx(BGRA32, NORMAL);
_iblend_to_xx(ARGB_4444, NORMAL);
_iblend_to_xx(ABGR_4444, NORMAL);
_iblend_to_xx(RGBA_4444, NORMAL);
_iblend_to_xx(BGRA_4444, NORMAL);
_iblend_to_xx(ARGB_1555, NORMAL);
_iblend_to_xx(ABGR_1555, NORMAL);
_iblend_to_xx(RGBA_5551, NORMAL);
_iblend_to_xx(BGRA_5551, NORMAL);
#else
_iblend_to_xx(RGB32, STATIC);
_iblend_to_xx(ARGB32, NORMAL);
#endif


//---------------------------------------------------------------------
// Alpha Blending 主入口
// dst      - 目标 bitmap
// dx, dy   - 目标位置
// src      - 源 bitmap
// sx, sy   - 源位置
// w, h     - 图块的宽和高
// color    - 叠加上去的颜色(0xffffffff为不叠加，0x80ffffff为半透明等)
// flags    - IBLIT_HFLIP, IBLIT_VFLIP, IBLIT_MASK, IBLEND_DEFAULT
//---------------------------------------------------------------------
int iblend_blit(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h, ICOLORD color, int flags)
{
	unsigned char *s, *d;
	int cr, cg, cb, ca;
	int sfmt, dfmt, r;
	long ssize, dsize;
	IBLEND_PROC proc;

	assert(dst && src);

	// 裁剪矩形
	r = ibitmap_blitclip(dst, &dx, &dy, src, &sx, &sy, &w, &h, flags);
	if (r) return 1;

	// 设置格式
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

	proc = iblend_proc[dfmt][sfmt];

	// 是否调用外部混色器
	if (proc != NULL && (flags & IBLEND_DEFAULT) == 0) {
		int retval;

		s = _ilineptr(src, sy) + ssize * sx;
		d = _ilineptr(dst, dy) + dsize * dx;

		// 调用外部混色器
		retval = proc(d, (long)dst->pitch, s, (long)src->pitch, w, h,
			color, dfmt, sfmt, src->mask, flags);

		// 如果外部混色器完成所有宽度的绘制则直接返回
		if (retval >= w) return 0;

		// 否则修正坐标是用 C版本混色器绘制接下来的部分
		if ((flags && IBLIT_HFLIP) == 0) {
			dx += retval;
			sx += retval;
			w -= retval;
		}	else {
			dx += retval;
			w -= retval;
		}
	}

	#define _iblend_to_case(xfmt) case IPIX_FMT_##xfmt: { \
		_iblend_to_##xfmt(dst, dx, dy, src, sx, sy, w, h, color, flags); \
		break; }

	// 检测像素格式，并选取混色器进行混色
	switch (dfmt)
	{
#ifndef TINYDEBUG
	_iblend_to_case(RGB15);
	_iblend_to_case(BGR15);
	_iblend_to_case(RGB16);
	_iblend_to_case(BGR16);
	_iblend_to_case(RGB24);
	_iblend_to_case(BGR24);
	_iblend_to_case(RGB32);
	_iblend_to_case(BGR32);
	_iblend_to_case(ARGB32);
	_iblend_to_case(ABGR32);
	_iblend_to_case(RGBA32);
	_iblend_to_case(BGRA32);
	_iblend_to_case(ARGB_4444);
	_iblend_to_case(ABGR_4444);
	_iblend_to_case(RGBA_4444);
	_iblend_to_case(BGRA_4444);
	_iblend_to_case(ARGB_1555);
	_iblend_to_case(ABGR_1555);
	_iblend_to_case(RGBA_5551);
	_iblend_to_case(BGRA_5551);
#else
	_iblend_to_case(RGB32);
	_iblend_to_case(ARGB32);
#endif
	}

	return 0;
}


//---------------------------------------------------------------------
// 转换 ARGB32 -> PARGB32
// 预乘所有像素的 Alpha值
//---------------------------------------------------------------------
int iblend_pmul_convert(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h)
{
	int x, y, r, g, b, alpha;
	ICOLORD color;
	assert(dst->bpp == 32 && src->bpp == 32);
	if (dst->bpp != 32 || src->bpp != 32) return -1;
	for (y = 0; y < h; y++) {
		unsigned char *s = (unsigned char*)_ilineptr(src, sy + y) + sx * 4;
		unsigned char *d = (unsigned char*)_ilineptr(dst, dy + y) + dx * 4;
		for (x = w; x > 0; x--) {
			color = _ipixel_get(32, s);
			IRGBA_FROM_PIXEL(color, ARGB32, r, g, b, alpha);
			r = (unsigned char)((r * alpha) >> 8);
			g = (unsigned char)((g * alpha) >> 8);
			b = (unsigned char)((b * alpha) >> 8);
			alpha = (unsigned char)(255 - alpha);
			color = IRGBA_TO_PIXEL(ARGB32, r, g, b, alpha);
			_ipixel_put(32, d, color);
			d += 4;
			s += 4;
		}
	}
	return 0;
}


// 32位的PARGB混色
static void _iblend_pmul_blit_32(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h)
{
	static unsigned long endian = 0x11223344;
	int x, y;
	for (y = 0; y < h; y++) {
		unsigned char *s = (unsigned char*)_ilineptr(src, sy + y) + sx * 4;
		unsigned char *d = (unsigned char*)_ilineptr(dst, dy + y) + dx * 4;
		int a1, r1, g1, b1;
		int a2, r2, g2, b2;
		// little endian: x86 etc, ...
		if (*((unsigned char*)&endian) == 0x44) {
			for (x = w / 2; x > 0; x--) {
				a1 = s[3];
				a2 = s[7];
				r1 = d[0] * a1;
				g1 = d[1] * a1;
				b1 = d[2] * a1;
				r2 = d[4] * a2;
				g2 = d[5] * a2;
				b2 = d[6] * a2;
				d[0] = (unsigned char)((r1 >> 8) + s[0]);
				d[1] = (unsigned char)((g1 >> 8) + s[1]);
				d[2] = (unsigned char)((b1 >> 8) + s[2]);
				d[3] = 255;
				d[4] = (unsigned char)((r2 >> 8) + s[4]);
				d[5] = (unsigned char)((g2 >> 8) + s[5]);
				d[6] = (unsigned char)((b2 >> 8) + s[6]);
				d[7] = 255;
				s += 8;
				d += 8;
			}
			if (w & 1) {
				a1 = s[3];
				r1 = d[0] * a1;
				g1 = d[1] * a1;
				b1 = d[2] * a1;
				d[0] = (unsigned char)((r1 >> 8) + s[0]);
				d[1] = (unsigned char)((g1 >> 8) + s[1]);
				d[2] = (unsigned char)((b1 >> 8) + s[2]);
				d[3] = 255;
			}
		}	
		else {	// big endian, etc: powerpc
			for (x = w / 2; x > 0; x--) {
				a1 = s[0];
				a2 = s[4];
				r1 = d[1] * a1;
				g1 = d[2] * a1;
				b1 = d[3] * a1;
				r2 = d[5] * a2;
				g2 = d[6] * a2;
				b2 = d[7] * a2;
				d[0] = 255;
				d[1] = (unsigned char)((r1 >> 8) + s[1]);
				d[2] = (unsigned char)((g1 >> 8) + s[2]);
				d[3] = (unsigned char)((b1 >> 8) + s[3]);
				d[4] = 255;
				d[5] = (unsigned char)((r2 >> 8) + s[5]);
				d[6] = (unsigned char)((g2 >> 8) + s[6]);
				d[7] = (unsigned char)((b2 >> 8) + s[7]);
				s += 8;
				d += 8;
			}
			if (w & 1) {
				a1 = s[0];
				r1 = d[1] * a1;
				g1 = d[2] * a1;
				b1 = d[3] * a1;
				d[4] = 255;
				d[1] = (unsigned char)((r1 >> 8) + s[1]);
				d[2] = (unsigned char)((g1 >> 8) + s[2]);
				d[3] = (unsigned char)((b1 >> 8) + s[3]);
			}
		}
	}
}


#define iblend_pmul_blit_xx(dfmt, dbpp, dsize) \
static void _iblend_pmul_blit_##dfmt(IBITMAP *dst, int dx, int dy, \
	IBITMAP *src, int sx, int sy, int w, int h, int alpha) \
{ \
	long offdst = dx * dsize; \
	long offsrc = sx * 4; \
	int x, y, ca1, ca2; \
	if (alpha <= 0) return; \
	for (y = 0; y < h; y++) { \
		unsigned char *s = (unsigned char*)_ilineptr(src, sy + y) + offsrc; \
		unsigned char *d = (unsigned char*)_ilineptr(dst, dy + y) + offdst; \
		int c1, a1, r1, g1, b1; \
		int c2, a2, r2, g2, b2; \
		int c3, a3, r3, g3, b3; \
		int c4, a4, r4, g4, b4; \
		if (alpha >= 255) { \
			for (x = w / 2; x > 0; x--) { \
				c1 = IPIX_FMT_READ(ARGB32, s); \
				c3 = IPIX_FMT_READ(dfmt, d); \
				s += 4; \
				d += dsize; \
				c2 = IPIX_FMT_READ(ARGB32, s); \
				c4 = IPIX_FMT_READ(dfmt, d); \
				s += 4; \
				d -= dsize; \
				IRGBA_FROM_PIXEL(c1, ARGB32, r1, g1, b1, a1); \
				IRGBA_FROM_PIXEL(c2, ARGB32, r2, g2, b2, a2); \
				IRGBA_FROM_PIXEL(c3, dfmt, r3, g3, b3, a3); \
				IRGBA_FROM_PIXEL(c4, dfmt, r4, g4, b4, a4); \
				r3 = r1 + ((r3 * a1) >> 8); \
				g3 = g1 + ((g3 * a1) >> 8); \
				b3 = b1 + ((b3 * a1) >> 8); \
				r4 = r2 + ((r4 * a2) >> 8); \
				g4 = g2 + ((g4 * a2) >> 8); \
				b4 = b2 + ((b4 * a2) >> 8); \
				c3 = IRGBA_TO_PIXEL(dfmt, r3, g3, b3, 255); \
				c4 = IRGBA_TO_PIXEL(dfmt, r4, g4, b4, 255); \
				IPIX_FMT_WRITE(dfmt, d, c3); \
				d += dsize; \
				IPIX_FMT_WRITE(dfmt, d, c4); \
				d += dsize; \
			} \
			if ((w & 1) != 0) { \
				c1 = IPIX_FMT_READ(ARGB32, s); \
				c3 = IPIX_FMT_READ(dfmt, d); \
				IRGBA_FROM_PIXEL(c1, ARGB32, r1, g1, b1, a1); \
				IRGBA_FROM_PIXEL(c3, dfmt, r3, g3, b3, a3); \
				r3 = r1 + ((r3 * a1) >> 8); \
				g3 = g1 + ((g3 * a1) >> 8); \
				b3 = b1 + ((b3 * a1) >> 8); \
				c3 = IRGBA_TO_PIXEL(dfmt, r3, g3, b3, 255); \
				IPIX_FMT_WRITE(dfmt, d, c3); \
			}	\
		}	else { \
			for (x = w / 2; x > 0; x--) { \
				c1 = IPIX_FMT_READ(ARGB32, s); \
				c3 = IPIX_FMT_READ(dfmt, d); \
				s += 4; \
				d += dsize; \
				c2 = IPIX_FMT_READ(ARGB32, s); \
				c4 = IPIX_FMT_READ(dfmt, d); \
				s += 4; \
				d -= dsize; \
				IRGBA_FROM_PIXEL(c1, ARGB32, r1, g1, b1, a1); \
				IRGBA_FROM_PIXEL(c2, ARGB32, r2, g2, b2, a2); \
				IRGBA_FROM_PIXEL(c3, dfmt, r3, g3, b3, a3); \
				IRGBA_FROM_PIXEL(c4, dfmt, r4, g4, b4, a4); \
				ca1 = (255 - a1) * alpha / 256; \
				ca2 = (255 - a2) * alpha / 256; \
				a1 = 255 - ca1; \
				a2 = 255 - ca2; \
				r3 = ((r1 * alpha) + (r3 * a1)) >> 8; \
				g3 = ((g1 * alpha) + (g3 * a1)) >> 8; \
				b3 = ((b1 * alpha) + (b3 * a1)) >> 8; \
				r4 = ((r2 * alpha) + (r4 * a2)) >> 8; \
				g4 = ((g2 * alpha) + (g4 * a2)) >> 8; \
				b4 = ((b2 * alpha) + (b4 * a2)) >> 8; \
				c3 = IRGBA_TO_PIXEL(dfmt, r3, g3, b3, 255); \
				c4 = IRGBA_TO_PIXEL(dfmt, r4, g4, b4, 255); \
				IPIX_FMT_WRITE(dfmt, d, c3); \
				d += dsize; \
				IPIX_FMT_WRITE(dfmt, d, c4); \
				d += dsize; \
			} \
			if ((w & 1) != 0) { \
				c1 = IPIX_FMT_READ(ARGB32, s); \
				c3 = IPIX_FMT_READ(dfmt, d); \
				IRGBA_FROM_PIXEL(c1, ARGB32, r1, g1, b1, a1); \
				IRGBA_FROM_PIXEL(c3, dfmt, r3, g3, b3, a3); \
				ca1 = (255 - a1) * alpha / 256; \
				a1 = 255 - ca1; \
				r3 = ((r1 * alpha) + (r3 * a1)) >> 8; \
				g3 = ((g1 * alpha) + (g3 * a1)) >> 8; \
				b3 = ((b1 * alpha) + (b3 * a1)) >> 8; \
				c3 = IRGBA_TO_PIXEL(dfmt, r3, g3, b3, 255); \
				IPIX_FMT_WRITE(dfmt, d, c3); \
			}	\
		} \
	}	\
}


iblend_pmul_blit_xx(RGB15, 15, 2);
iblend_pmul_blit_xx(BGR15, 15, 2);
iblend_pmul_blit_xx(RGB16, 16, 2);
iblend_pmul_blit_xx(BGR16, 16, 2);
iblend_pmul_blit_xx(RGB24, 24, 3);
iblend_pmul_blit_xx(BGR24, 24, 3);
iblend_pmul_blit_xx(RGB32, 32, 4);
iblend_pmul_blit_xx(BGR32, 32, 4);


static int _iblend_pmul_blit_mmx(IBITMAP *dst, int dx, int dy, 
	IBITMAP *src, int sx, int sy, int w, int h)
{
	unsigned char *ptr1 = (unsigned char*)_ilineptr(dst, dy) + dx * 4;
	unsigned char *ptr2 = (unsigned char*)_ilineptr(src, sy) + sx * 4;
	unsigned long pitch1 = dst->pitch;
	unsigned long pitch2 = src->pitch;
	unsigned long diff1 = pitch1 - w * 4;
	unsigned long diff2 = pitch2 - w * 4;

#if defined(__INLINEGNU__) && defined(__i386__)
	__asm__ __volatile__ (
	  ASM_BEGIN
	  "      mov          %1, %%edi\n"
	  "      mov          %2, %%esi\n"
	  "      pxor         %%mm7, %%mm7\n"
	  "      pcmpeqb      %%mm6, %%mm6\n"
	  "      .align       8, 0x90\n"
	  "1:    \n"
	  "      mov          %3, %%ecx\n"
	  "      shr          $0x2, %%ecx\n"
	  "      .align       8, 0x90\n"
	  "2:    \n"
	  "      pxor         %%mm7, %%mm7\n"
	  "      movd         (%%edi), %%mm0\n"
	  "      movd         0x4(%%edi), %%mm1\n"
	  "      movd         0x8(%%edi), %%mm2\n"
	  "      movd         0xc(%%edi), %%mm3\n"
	  "      punpcklbw    %%mm7, %%mm0\n"
	  "      punpcklbw    %%mm7, %%mm1\n"
	  "      punpcklbw    %%mm7, %%mm2\n"
	  "      punpcklbw    %%mm7, %%mm3\n"
	  "      movd         (%%esi), %%mm4\n"
	  "      movd         0x4(%%esi), %%mm5\n"
	  "      movd         0x8(%%esi), %%mm6\n"
	  "      movd         0xc(%%esi), %%mm7\n"
	  "      psrlq        $0x18, %%mm4\n"
	  "      psrlq        $0x18, %%mm5\n"
	  "      psrlq        $0x18, %%mm6\n"
	  "      psrlq        $0x18, %%mm7\n"
	  "      punpcklwd    %%mm4, %%mm4\n"
	  "      punpcklwd    %%mm5, %%mm5\n"
	  "      punpcklwd    %%mm6, %%mm6\n"
	  "      punpcklwd    %%mm7, %%mm7\n"
	  "      punpckldq    %%mm4, %%mm4\n"
	  "      punpckldq    %%mm5, %%mm5\n"
	  "      punpckldq    %%mm6, %%mm6\n"
	  "      punpckldq    %%mm7, %%mm7\n"
	  "      pmullw       %%mm4, %%mm0\n"
	  "      pmullw       %%mm5, %%mm1\n"
	  "      pmullw       %%mm6, %%mm2\n"
	  "      pmullw       %%mm7, %%mm3\n"
	  
	  "      pxor         %%mm7, %%mm7\n"
	  "      pcmpeqw      %%mm6, %%mm6\n"
	  "      psrlq        $0x8, %%mm0\n"
	  "      psrlq        $0x8, %%mm1\n"
	  "      psrlq        $0x8, %%mm2\n"
	  "      psrlq        $0x8, %%mm3\n"
	  "      punpcklbw    %%mm7, %%mm6\n"
	  "      pand         %%mm6, %%mm0\n"
	  "      pand         %%mm6, %%mm1\n"
	  "      pand         %%mm6, %%mm2\n"
	  "      pand         %%mm6, %%mm3\n"
	  
	  "      movd         (%%esi), %%mm4\n"
	  "      movd         0x4(%%esi), %%mm5\n"
	  "      movd         0x8(%%esi), %%mm6\n"
	  "      movd         0xc(%%esi), %%mm7\n"
	  
	  "      packuswb     %%mm0, %%mm0\n"
	  "      packuswb     %%mm1, %%mm1\n"
	  "      packuswb     %%mm2, %%mm2\n"
	  "      packuswb     %%mm3, %%mm3\n"
	  "      paddb        %%mm4, %%mm0\n"
	  "      paddb        %%mm5, %%mm1\n"
	  "      paddb        %%mm6, %%mm2\n"
	  "      paddb        %%mm7, %%mm3\n"
	  
	  "      movd         %%mm0, (%%edi)\n"
	  "      movd         %%mm1, 0x4(%%edi)\n"
	  "      movd         %%mm2, 0x8(%%edi)\n"
	  "      movd         %%mm3, 0xc(%%edi)\n"
	  
	  "      add          $0x10, %%edi\n"
	  "      add          $0x10, %%esi\n"
	  "      dec          %%ecx\n"
	  "      jnz          2b\n"
	  
	  "      mov          %3, %%ecx\n"
	  "      and          $0x3, %%ecx\n"
	  "      cmp          $0x0, %%ecx\n"
	  "      jz           4f\n"
	  
	  "3:    \n"
	  "      pxor         %%mm7, %%mm7\n"
	  "      movd         (%%edi), %%mm0\n"
	  "      movd         (%%esi), %%mm4\n"
	  "      movd         (%%esi), %%mm5\n"
	  "      punpcklbw    %%mm7, %%mm0\n"
	  "      psrlq        $0x18, %%mm4\n"
	  "      pcmpeqw      %%mm6, %%mm6\n"
	  "      punpcklwd    %%mm4, %%mm4\n"
	  "      punpckldq    %%mm4, %%mm4\n"
	  "      pmullw       %%mm4, %%mm0\n"
	  "      psrlq        $0x8, %%mm0\n"
	  "      punpcklbw    %%mm7, %%mm6\n"
	  "      pand         %%mm6, %%mm0\n"
	  "      movq         %%mm5, %%mm4\n"
	  "      packuswb     %%mm0, %%mm0\n"
	  "      paddb        %%mm4, %%mm0\n"
	  "      movd         %%mm0, (%%edi)\n"
	  "      add          $0x4, %%edi\n"
	  "      add          $0x4, %%esi\n"
	  "      dec          %%ecx\n"
	  "      jnz          3b\n"
	  
	  "4:    \n"
	  "      add          %4, %%edi\n"
	  "      add          %5, %%esi\n"
	  "      decl         %0\n"
	  "      jnz          1b\n"
	  
	  "      emms\n"
	  ASM_ENDUP
	  :"=m"(h)
	  :"m"(ptr1), "m"(ptr2), "m"(w), "m"(diff1), "m"(diff2)
	  :"memory", ASM_REGS
	);

#elif defined(__INLINEMSC__) && defined(__i386__)
	_asm {
		mov edi, ptr1;
		mov esi, ptr2;
		pxor mm7, mm7;		// mm7 = 0000....00
		pcmpeqb mm6, mm6;	// mm6 = ffff....ff
loop_line:
		mov ecx, w;
		shr ecx, 2;
loop_pixel_x4:
		pxor mm7, mm7;
		movd mm0, [edi];
		movd mm1, [edi + 4];
		movd mm2, [edi + 8];
		movd mm3, [edi + 12];
		punpcklbw mm0, mm7;
		punpcklbw mm1, mm7;
		punpcklbw mm2, mm7;
		punpcklbw mm3, mm7;
		movd mm4, [esi];
		movd mm5, [esi + 4];
		movd mm6, [esi + 8];
		movd mm7, [esi + 12];
		psrlq mm4, 24;
		psrlq mm5, 24;
		psrlq mm6, 24;
		psrlq mm7, 24;
		punpcklwd mm4, mm4;
		punpcklwd mm5, mm5;
		punpcklwd mm6, mm6;
		punpcklwd mm7, mm7;
		punpckldq mm4, mm4;
		punpckldq mm5, mm5;
		punpckldq mm6, mm6;
		punpckldq mm7, mm7;
		pmullw mm0, mm4;
		pmullw mm1, mm5;
		pmullw mm2, mm6;
		pmullw mm3, mm7;

		pxor mm7, mm7;
		pcmpeqw mm6, mm6;
		psrlq mm0, 8;
		psrlq mm1, 8;
		psrlq mm2, 8;
		psrlq mm3, 8;
		punpcklbw mm6, mm7;
		pand mm0, mm6;
		pand mm1, mm6;
		pand mm2, mm6;
		pand mm3, mm6;

		movd mm4, [esi];
		movd mm5, [esi + 4];
		movd mm6, [esi + 8];
		movd mm7, [esi + 12];

		packuswb mm0, mm0;
		packuswb mm1, mm1;
		packuswb mm2, mm2;
		packuswb mm3, mm3;
		paddb mm0, mm4;
		paddb mm1, mm5;
		paddb mm2, mm6;
		paddb mm3, mm7;

		movd [edi], mm0;
		movd [edi + 4], mm1;
		movd [edi + 8], mm2;
		movd [edi + 12], mm3;

		add edi, 16;
		add esi, 16;
		dec ecx;
		jnz loop_pixel_x4;

		mov ecx, w;
		and ecx, 3;
		cmp ecx, 0;
		jz end_line;

loop_pixel_x1:
		pxor mm7, mm7;
		movd mm0, [edi];
		movd mm4, [esi];
		movd mm5, [esi];
		punpcklbw mm0, mm7;
		psrlq mm4, 24;
		pcmpeqw mm6, mm6;
		punpcklwd mm4, mm4;
		punpckldq mm4, mm4;
		pmullw mm0, mm4;
		psrlq mm0, 8;
		punpcklbw mm6, mm7;
		pand mm0, mm6;
		movq mm4, mm5;
		packuswb mm0, mm0;
		paddb mm0, mm4;
		movd [edi], mm0;
		add edi, 4;
		add esi, 4;
		dec ecx;
		jnz loop_pixel_x1;

end_line:
		add edi, diff1;
		add esi, diff2;
		dec dword ptr h;
		jnz loop_line;

		emms;
	}
#else
	return -1;
#endif
	return 0;
}


// driver for mmx
int iblend_proc_mmx_32(unsigned char *dst, long dpitch, 
	const unsigned char *src, long spitch, int w, int h, ICOLORD color,
	int dfmt, int sfmt, ICOLORD mask, int flags)
{
	unsigned char *ptr1 = (unsigned char*)dst;
	unsigned char *ptr2 = (unsigned char*)src;
	unsigned long diff1 = dpitch - w * 4;
	unsigned long diff2 = spitch - w * 4;

	if (flags & (IBLIT_HFLIP | IBLIT_VFLIP | IBLIT_MASK))
		return 0;

	if (color != 0xfffffffful)
		return 0;

#if defined(__INLINEGNU__) && defined(__i386__)
	__asm__ __volatile__ (
	  ASM_BEGIN
	  "      mov          %1, %%edi\n"
	  "      mov          %2, %%esi\n"
	  "      pxor         %%mm7, %%mm7\n"
	  "      pcmpeqb      %%mm6, %%mm6\n"
	  "      .align       8, 0x90\n"
	  "1:    \n"
	  "      mov          %3, %%ecx\n"
	  "      shr          $0x1, %%ecx\n"
	  "      .align       8, 0x90\n"
	  "2:    \n"
	  "      movd         (%%esi), %%mm0\n"
	  "      movd         0x4(%%esi), %%mm1\n"
	  "      mov          (%%edi), %%eax\n"
	  "      mov          0x4(%%edi), %%edx\n"
	  "      movq         %%mm0, %%mm2\n"
	  "      movq         %%mm1, %%mm3\n"
	  "      psrlq        $0x18, %%mm2\n"
	  "      psrlq        $0x18, %%mm3\n"
	  "      punpcklwd    %%mm2, %%mm2\n"
	  "      punpcklwd    %%mm3, %%mm3\n"
	  "      punpckldq    %%mm2, %%mm2\n"
	  "      punpckldq    %%mm3, %%mm3\n"
	  "      pcmpeqb      %%mm4, %%mm4\n"
	  "      pcmpeqb      %%mm5, %%mm5\n"
	  "      punpcklbw    %%mm7, %%mm0\n"
	  "      punpcklbw    %%mm7, %%mm1\n"
	  "      punpcklbw    %%mm7, %%mm4\n"
	  "      punpcklbw    %%mm7, %%mm5\n"
	  "      psubb        %%mm2, %%mm4\n"
	  "      psubb        %%mm3, %%mm5\n"
	  
	  "      pmullw       %%mm2, %%mm0\n"
	  "      pmullw       %%mm3, %%mm1\n"
	  "      movd         %%eax, %%mm2\n"
	  "      movd         %%edx, %%mm3\n"
	  "      punpcklbw    %%mm7, %%mm2\n"
	  "      punpcklbw    %%mm7, %%mm3\n"
	  "      pmullw       %%mm4, %%mm2\n"
	  "      pmullw       %%mm5, %%mm3\n"
	  
	  "      pcmpeqw      %%mm5, %%mm5\n"
	  "      punpcklbw    %%mm7, %%mm5\n"
	  
	  "      paddw        %%mm2, %%mm0\n"
	  "      paddw        %%mm3, %%mm1\n"
	  "      psrlw        $0x8, %%mm0\n"
	  "      psrlw        $0x8, %%mm1\n"
	  "      pand         %%mm5, %%mm0\n"
	  "      pand         %%mm5, %%mm1\n"
	  "      packuswb     %%mm0, %%mm0\n"
	  "      packuswb     %%mm1, %%mm1\n"
	  
	  "      movd         %%mm0, (%%edi)\n"
	  "      movd         %%mm1, 0x4(%%edi)\n"
	  
	  "      add          $0x8, %%edi\n"
	  "      add          $0x8, %%esi\n"
	  "      dec          %%ecx\n"
	  "      jnz          2b\n"
	  
	  "      mov          %3, %%ecx\n"
	  "      and          $0x1, %%ecx\n"
	  "      cmp          $0x0, %%ecx\n"
	  "      jz           3f\n"
	  
	  "      movd         (%%esi), %%mm0\n"
	  "      mov          (%%edi), %%eax\n"
	  "      movq         %%mm0, %%mm2\n"
	  "      psrlq        $0x18, %%mm2\n"
	  "      punpcklwd    %%mm2, %%mm2\n"
	  "      punpckldq    %%mm2, %%mm2\n"
	  "      pcmpeqb      %%mm4, %%mm4\n"
	  "      punpcklbw    %%mm7, %%mm0\n"
	  "      punpcklbw    %%mm7, %%mm4\n"
	  "      psubb        %%mm2, %%mm4\n"
	  "      pmullw       %%mm2, %%mm0\n"
	  "      movd         %%eax, %%mm2\n"
	  "      punpcklbw    %%mm7, %%mm2\n"
	  "      pmullw       %%mm4, %%mm2\n"
	  "      pcmpeqw      %%mm5, %%mm5\n"
	  "      punpcklbw    %%mm7, %%mm5\n"
	  "      paddw        %%mm2, %%mm0\n"
	  "      psrlw        $0x8, %%mm0\n"
	  "      packuswb     %%mm0, %%mm0\n"
	  "      movd         %%mm0, (%%edi)\n"
	  "      add          $0x4, %%esi\n"
	  "      add          $0x4, %%edi\n"
	  
	  "3:    \n"
	  "      add          %4, %%edi\n"
	  "      add          %5, %%esi\n"
	  "      decl         %0\n"
	  "      jnz          1b\n"
	  
	  "      emms\n"
	  ASM_ENDUP
	  :"=m"(h)
	  :"m"(ptr1), "m"(ptr2), "m"(w), "m"(diff1), "m"(diff2)
	  :"memory", ASM_REGS
	);

#elif defined(__INLINEMSC__) && defined(__i386__)
	_asm {
		mov edi, ptr1;
		mov esi, ptr2;
		pxor mm7, mm7;		// mm7 = 0000....00
		pcmpeqb mm6, mm6;	// mm6 = ffff....ff
loop_line:
		mov ecx, w;
		shr ecx, 1;
loop_pixel_x2:
		movd mm0, [esi];
		movd mm1, [esi + 4];
		mov eax, [edi];
		mov ebx, [edi + 4];
		movq mm2, mm0;
		movq mm3, mm1;
		psrlq mm2, 24;
		psrlq mm3, 24;
		punpcklwd mm2, mm2;
		punpcklwd mm3, mm3;
		punpckldq mm2, mm2;		// mm2 = 0 a1 0 a1 0 a1 0 a1 (word)
		punpckldq mm3, mm3;		// mm3 = 0 a2 0 a2 0 a2 0 a2 (word)
		pcmpeqb mm4, mm4;		// mm4 = 0xffff...ff
		pcmpeqb mm5, mm5;		// mm5 = 0xffff...ff
		punpcklbw mm0, mm7;		// mm0 = src1
		punpcklbw mm1, mm7;		// mm1 = src2
		punpcklbw mm4, mm7;
		punpcklbw mm5, mm7;
		psubb mm4, mm2;			// mm4 = (0xff - a1)...
		psubb mm5, mm3;			// mm5 = (0xff - a2)...

		pmullw mm0, mm2;		// mm0 = src1 * alpha1
		pmullw mm1, mm3;		// mm1 = src2 * alpha2
		movd mm2, eax;			// mm2 = dst1
		movd mm3, ebx;			// mm3 = dst2
		punpcklbw mm2, mm7;
		punpcklbw mm3, mm7;
		pmullw mm2, mm4;		// mm2 = dst1 * (255 - a1)
		pmullw mm3, mm5;		// mm3 = dst2 * (255 - a2)

		pcmpeqw mm5, mm5;
		punpcklbw mm5, mm7;

		paddw mm0, mm2;
		paddw mm1, mm3;
		psrlw mm0, 8;
		psrlw mm1, 8;
		pand mm0, mm5;
		pand mm1, mm5;
		packuswb mm0, mm0;
		packuswb mm1, mm1;

		movd [edi], mm0;
		movd [edi + 4], mm1;

		add edi, 8;
		add esi, 8;
		dec ecx;
		jnz loop_pixel_x2;

		mov ecx, w;
		and ecx, 1;
		cmp ecx, 0;
		jz end_line;
		
		// last single pixel
		movd mm0, [esi];
		mov eax, [edi];
		movq mm2, mm0;
		psrlq mm2, 24;
		punpcklwd mm2, mm2;
		punpckldq mm2, mm2;		// mm2 = 0 a1 0 a1 0 a1 0 a1 (word)
		pcmpeqb mm4, mm4;		// mm4 = 0xffff...ff
		punpcklbw mm0, mm7;		// mm0 = src1
		punpcklbw mm4, mm7;
		psubb mm4, mm2;			// mm4 = (0xff - a1)...
		pmullw mm0, mm2;		// mm0 = src1 * alpha1
		movd mm2, eax;			// mm2 = dst1
		punpcklbw mm2, mm7;
		pmullw mm2, mm4;		// mm2 = dst1 * (255 - a1)
		pcmpeqw mm5, mm5;
		punpcklbw mm5, mm7;
		paddw mm0, mm2;
		psrlw mm0, 8;
		packuswb mm0, mm0;
		movd [edi], mm0;
		add esi, 4;
		add edi, 4;

end_line:
		add edi, diff1;
		add esi, diff2;
		dec dword ptr h;
		jnz loop_line;

		emms;
	}
#else
	return 0;
#endif
	return w;
}


//---------------------------------------------------------------------
// 是否使用MMX
//---------------------------------------------------------------------
static int iblend_usemmx = 0;


//---------------------------------------------------------------------
// PARGB32的 AlphaBlend
// src 需要先用 iblend_pmul_convert从ARGB32转换而来
//---------------------------------------------------------------------
int iblend_pmul_blit(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h, int alpha)
{
	int dfmt;
	int sfmt;
	int r;

	// 裁剪矩形
	r = ibitmap_blitclip(dst, &dx, &dy, src, &sx, &sy, &w, &h, 0);
	if (r) return 0;

	// 设置格式
	if (_ibitmap_pixfmt(dst) == 0) ibitmap_set_pixfmt(dst, 0);
	if (_ibitmap_pixfmt(src) == 0) ibitmap_set_pixfmt(src, 0);
	dfmt = _ibitmap_pixfmt(dst);
	sfmt = _ibitmap_pixfmt(src);

	if (src->bpp != 16) return -1;

	// 如果目标色素是32位
	if (dst->bpp == 32 && alpha >= 255) {
		int available = 0;
		if (dfmt == IPIX_FMT_RGB32 || dfmt == IPIX_FMT_ARGB32) {
			if (sfmt == IPIX_FMT_ARGB32) available++;
			else if (sfmt == IPIX_FMT_RGB32) available++;
		}
		else if (dfmt == IPIX_FMT_BGR32 || dfmt == IPIX_FMT_ABGR32) {
			if (sfmt == IPIX_FMT_ABGR32) available++;
			else if (sfmt == IPIX_FMT_BGR32) available++;
		}
		if (available) {
			if (iblend_usemmx == 0) {
				_iblend_pmul_blit_32(dst, dx, dy, src, sx, sy, w, h);
			}	else {
				_iblend_pmul_blit_mmx(dst, dx, dy, src, sx, sy, w, h);
			}
		}
	}

	// 检测像素格式，并选取混色器进行混色
	switch (dfmt)
	{
	case IPIX_FMT_RGB15: 
		_iblend_pmul_blit_RGB15(dst, dx, dy, src, sx, sy, w, h, alpha);
		break;
	case IPIX_FMT_BGR15:
		_iblend_pmul_blit_BGR15(dst, dx, dy, src, sx, sy, w, h, alpha);
		break;
	case IPIX_FMT_RGB16: 
		_iblend_pmul_blit_RGB16(dst, dx, dy, src, sx, sy, w, h, alpha);
		break;
	case IPIX_FMT_BGR16:
		_iblend_pmul_blit_BGR16(dst, dx, dy, src, sx, sy, w, h, alpha);
		break;
	case IPIX_FMT_RGB24: 
		_iblend_pmul_blit_RGB24(dst, dx, dy, src, sx, sy, w, h, alpha);
		break;
	case IPIX_FMT_BGR24:
		_iblend_pmul_blit_BGR24(dst, dx, dy, src, sx, sy, w, h, alpha);
		break;
	case IPIX_FMT_RGB32: 
		_iblend_pmul_blit_RGB32(dst, dx, dy, src, sx, sy, w, h, alpha);
		break;
	case IPIX_FMT_BGR32:
		_iblend_pmul_blit_BGR32(dst, dx, dy, src, sx, sy, w, h, alpha);
		break;
	default:
		return -2;
	}

	return 0;
}


//---------------------------------------------------------------------
// 检测MMX，如果支持的话，将自动更改绘制驱动
// 成功返回1，不成功返回0
//---------------------------------------------------------------------
int iblend_mmx_detect(void)
{
	#ifdef __x86__
	_x86_detect();
	#endif
	#ifdef __i386__
	if (X86_FEATURE(X86_FEATURE_MMX)) {
		iblend_usemmx = 1;
		#define iblend_check(dfmt, sfmt) { \
			if (iblend_proc[IPIX_FMT_##dfmt][IPIX_FMT_##sfmt] == NULL) \
				iblend_proc[IPIX_FMT_##dfmt][IPIX_FMT_##sfmt] = \
					iblend_proc_mmx_32; \
		}
		iblend_check(RGB32, ARGB32);
		iblend_check(BGR32, ABGR32);
		#undef iblend_check
	}
	return iblend_usemmx;
	#else
	return 0;
	#endif
}



//---------------------------------------------------------------------
// 单位点操作
//---------------------------------------------------------------------

#define ibitmap_putpixel_proc_STATIC(fmt, dsize) \
static inline void _iputpixel_##fmt(IBITMAP *bmp, int x, int y, ICOLORD c) {\
	int r, g, b, a, c1, r1, g1, b1, a1; \
	unsigned char *ptr = _ilineptr(bmp, y) + x * dsize; \
	IRGBA_FROM_PIXEL(c, ARGB32, r, g, b, a); \
	if (a == 255) { \
		c1 = IRGBA_TO_PIXEL(fmt, r, g, b, 255); \
		_ipixel_put(dsize, ptr, c1); \
	}	\
	else if (a > 0) { \
		c1 = _ipixel_get(dsize, ptr);  \
		IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
		IBLEND_STATIC(r, g, b, a, r1, g1, b1, a1); \
		c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
		_ipixel_put(dsize, ptr, c1); \
	} \
} 

#define ibitmap_putpixel_proc_NORMAL(fmt, dsize) \
static inline void _iputpixel_##fmt(IBITMAP *bmp, int x, int y, ICOLORD c) {\
	int r, g, b, a, c1, r1, g1, b1, a1; \
	unsigned char *ptr = _ilineptr(bmp, y) + x * dsize; \
	IRGBA_FROM_PIXEL(c, ARGB32, r, g, b, a); \
	if (a == 0xff) { \
		c1 = IRGBA_TO_PIXEL(fmt, r, g, b, a); \
		_ipixel_put(dsize, ptr, c1); \
	}	\
	else if (a > 0) { \
		c1 = _ipixel_get(dsize, ptr);  \
		IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
		IBLEND_NORMAL(r, g, b, a, r1, g1, b1, a1); \
		c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
		_ipixel_put(dsize, ptr, c1); \
	} \
} 

#define ibitmap_putpixel_proc_additive(fmt, dsize) \
static inline void _iaddpixel_##fmt(IBITMAP *bmp, int x, int y, ICOLORD c) {\
	int r, g, b, a, c1, r1, g1, b1, a1; \
	unsigned char *ptr = _ilineptr(bmp, y) + x * dsize; \
	IRGBA_FROM_PIXEL(c, ARGB32, r, g, b, a); \
	c1 = _ipixel_get(dsize, ptr);  \
	IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
	IBLEND_ADDITIVE(r, g, b, a, r1, g1, b1, a1); \
	c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
	_ipixel_put(dsize, ptr, c1); \
} 

#define ibitmap_getpixel_proc(fmt, dsize) \
static inline ICOLORD _igetpixel_##fmt(IBITMAP *bmp, int x, int y) { \
	unsigned char *ptr = _ilineptr(bmp, y) + x * dsize; \
	int r, g, b, a, c; \
	c = _ipixel_get(dsize, ptr); \
	IRGBA_FROM_PIXEL(c, fmt, r, g, b, a); \
	return IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
}

#define ibitmap_batch_putpixel(fmt, dsize) \
static inline void _isetpixel_##fmt(IBITMAP *bmp, const int *xy, \
	int n, ICOLORD color, int additive) \
{ \
	int r1, g1, b1, a1, c1, r, g, b, a; \
	unsigned char **lines; \
	unsigned char *ptr; \
	lines = (unsigned char**)bmp->line; \
	IRGBA_FROM_PIXEL(color, ARGB32, r, g, b, a); \
	if (a == 0) return; \
	if (additive) { \
		for (; n > 0; n--, xy += 2) { \
			ptr = lines[xy[1]] + xy[0] * dsize; \
			c1 = _ipixel_get(dsize, ptr); \
			IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
			IBLEND_ADDITIVE(r, g, b, a, r1, g1, b1, a1); \
			c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			_ipixel_put(dsize, ptr, c1); \
		} \
		return; \
	} \
	if (a == 255) { \
		c1 = IRGBA_TO_PIXEL(fmt, r, g, b, a); \
		for (; n > 0; n--, xy += 2) { \
			ptr = lines[xy[1]] + xy[0] * dsize; \
			_ipixel_put(dsize, ptr, c1); \
		} \
		return; \
	} \
	if (ipixel_fmt[IPIX_FMT_##fmt].has_alpha == 0) { \
		for (; n > 0; n--, xy += 2) { \
			ptr = lines[xy[1]] + xy[0] * dsize; \
			c1 = _ipixel_get(dsize, ptr); \
			IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
			IBLEND_STATIC(r, g, b, a, r1, g1, b1, a1); \
			c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			_ipixel_put(dsize, ptr, c1); \
		} \
	}	else { \
		for (; n > 0; n--, xy += 2) { \
			ptr = lines[xy[1]] + xy[0] * dsize; \
			c1 = _ipixel_get(dsize, ptr); \
			IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
			IBLEND_NORMAL(r, g, b, a, r1, g1, b1, a1); \
			c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			_ipixel_put(dsize, ptr, c1); \
		} \
	} \
}

#define ibitmap_batch_dispixel(fmt, dsize) \
static inline void _idispixel_##fmt(IBITMAP *bmp, const int *xy, \
	const ICOLORD *colors, int n, int additive) \
{ \
	int r1, g1, b1, a1, c1, c, r, g, b, a; \
	unsigned char **lines; \
	unsigned char *ptr; \
	lines = (unsigned char**)bmp->line; \
	if (additive) { \
		for (; n > 0; n--, xy += 2, colors++) { \
			ptr = lines[xy[1]] + xy[0] * dsize; \
			c = colors[0]; \
			IRGBA_FROM_PIXEL(c, ARGB32, r, g, b, a); \
			if (a != 0) { \
				c1 = _ipixel_get(dsize, ptr); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				IBLEND_ADDITIVE(r, g, b, a, r1, g1, b1, a1); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
				_ipixel_put(dsize, ptr, c1); \
			} \
		} \
		return; \
	} \
	if (ipixel_fmt[IPIX_FMT_##fmt].has_alpha == 0) { \
		for (; n > 0; n--, xy += 2, colors++) { \
			ptr = lines[xy[1]] + xy[0] * dsize; \
			c = colors[0]; \
			IRGBA_FROM_PIXEL(c, ARGB32, r, g, b, a); \
			if (a == 0xff) { \
				c1 = IRGBA_TO_PIXEL(fmt, r, g, b, 0xff); \
				_ipixel_put(dsize, ptr, c1); \
			}	\
			else if (a > 0) { \
				c1 = _ipixel_get(dsize, ptr); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				IBLEND_STATIC(r, g, b, a, r1, g1, b1, a1); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
				_ipixel_put(dsize, ptr, c1); \
			} \
		} \
	}	else { \
		for (; n > 0; n--, xy += 2, colors++) { \
			ptr = lines[xy[1]] + xy[0] * dsize; \
			c = colors[0]; \
			IRGBA_FROM_PIXEL(c, ARGB32, r, g, b, a); \
			if (a == 0xff) { \
				c1 = IRGBA_TO_PIXEL(fmt, r, g, b, 0xff); \
				_ipixel_put(dsize, ptr, c1); \
			} \
			else if (a > 0) { \
				c1 = _ipixel_get(dsize, ptr); \
				IRGBA_FROM_PIXEL(c1, fmt, r1, g1, b1, a1); \
				IBLEND_NORMAL(r, g, b, a, r1, g1, b1, a1); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
				_ipixel_put(dsize, ptr, c1); \
			} \
		} \
	}  \
}

#define ibitmap_putpixel_pp(fmt, dsize, mode) \
	ibitmap_putpixel_proc_##mode(fmt, dsize) \
	ibitmap_putpixel_proc_additive(fmt, dsize)  \
	ibitmap_getpixel_proc(fmt, dsize) \
	ibitmap_batch_putpixel(fmt, dsize) \
	ibitmap_batch_dispixel(fmt, dsize)

ibitmap_putpixel_pp(8, 1, STATIC);
ibitmap_putpixel_pp(RGB15, 2, STATIC);
ibitmap_putpixel_pp(BGR15, 2, STATIC);
ibitmap_putpixel_pp(RGB16, 2, STATIC);
ibitmap_putpixel_pp(BGR16, 2, STATIC);
ibitmap_putpixel_pp(RGB24, 3, STATIC);
ibitmap_putpixel_pp(BGR24, 3, STATIC);
ibitmap_putpixel_pp(RGB32, 4, STATIC);
ibitmap_putpixel_pp(BGR32, 4, STATIC);
ibitmap_putpixel_pp(ARGB32, 4, NORMAL);
ibitmap_putpixel_pp(ABGR32, 4, NORMAL);
ibitmap_putpixel_pp(RGBA32, 4, NORMAL);
ibitmap_putpixel_pp(BGRA32, 4, NORMAL);
ibitmap_putpixel_pp(ARGB_4444, 2, NORMAL);
ibitmap_putpixel_pp(ABGR_4444, 2, NORMAL);
ibitmap_putpixel_pp(RGBA_4444, 2, NORMAL);
ibitmap_putpixel_pp(BGRA_4444, 2, NORMAL);
ibitmap_putpixel_pp(ARGB_1555, 2, NORMAL);
ibitmap_putpixel_pp(ABGR_1555, 2, NORMAL);
ibitmap_putpixel_pp(RGBA_5551, 2, NORMAL);
ibitmap_putpixel_pp(BGRA_5551, 2, NORMAL);

#undef ibitmap_putpixel_pp

ICOLORD iblend_getpixel(IBITMAP *bmp, int x, int y)
{
	ICOLORD color;
	int sfmt;
	if (_ibitmap_pixfmt(bmp) == 0) ibitmap_set_pixfmt(bmp, 0);
	sfmt = _ibitmap_pixfmt(bmp);
	if ((unsigned int)x >= bmp->w || (unsigned int)y >= bmp->h)
		return 0;
	switch (sfmt)
	{
	case IPIX_FMT_8: color = _igetpixel_8(bmp, x, y); break;
	case IPIX_FMT_RGB15: color = _igetpixel_RGB15(bmp, x, y); break;
	case IPIX_FMT_BGR15: color = _igetpixel_BGR15(bmp, x, y); break;
	case IPIX_FMT_RGB16: color = _igetpixel_RGB15(bmp, x, y); break;
	case IPIX_FMT_BGR16: color = _igetpixel_BGR15(bmp, x, y); break;
	case IPIX_FMT_RGB24: color = _igetpixel_RGB24(bmp, x, y); break;
	case IPIX_FMT_BGR24: color = _igetpixel_BGR24(bmp, x, y); break;
	case IPIX_FMT_RGB32: color = _igetpixel_RGB32(bmp, x, y); break;
	case IPIX_FMT_BGR32: color = _igetpixel_BGR32(bmp, x, y); break;
	case IPIX_FMT_ARGB32: color = _igetpixel_ARGB32(bmp, x, y); break;
	case IPIX_FMT_ABGR32: color = _igetpixel_ABGR32(bmp, x, y); break;
	case IPIX_FMT_RGBA32: color = _igetpixel_RGBA32(bmp, x, y); break;
	case IPIX_FMT_BGRA32: color = _igetpixel_BGRA32(bmp, x, y); break;
	case IPIX_FMT_ARGB_4444: color = _igetpixel_ARGB_4444(bmp, x, y); break;
	case IPIX_FMT_ABGR_4444: color = _igetpixel_ABGR_4444(bmp, x, y); break;
	case IPIX_FMT_RGBA_4444: color = _igetpixel_RGBA_4444(bmp, x, y); break;
	case IPIX_FMT_BGRA_4444: color = _igetpixel_BGRA_4444(bmp, x, y); break;
	case IPIX_FMT_ARGB_1555: color = _igetpixel_ARGB_1555(bmp, x, y); break;
	case IPIX_FMT_ABGR_1555: color = _igetpixel_ABGR_1555(bmp, x, y); break;
	case IPIX_FMT_RGBA_5551: color = _igetpixel_RGBA_5551(bmp, x, y); break;
	case IPIX_FMT_BGRA_5551: color = _igetpixel_BGRA_5551(bmp, x, y); break;
	default: color = 0; break;
	}
	return color;
}

void iblend_putpixel(IBITMAP *bmp, int x, int y, ICOLORD c)
{
	int sfmt;
	if (_ibitmap_pixfmt(bmp) == 0) ibitmap_set_pixfmt(bmp, 0);
	sfmt = _ibitmap_pixfmt(bmp);
	if ((unsigned int)x >= bmp->w || (unsigned int)y >= bmp->h)
		return;
	switch (sfmt)
	{
	case IPIX_FMT_8: _iputpixel_8(bmp, x, y, c); break;
	case IPIX_FMT_RGB15: _iputpixel_RGB15(bmp, x, y, c); break;
	case IPIX_FMT_BGR15: _iputpixel_BGR15(bmp, x, y, c); break;
	case IPIX_FMT_RGB16: _iputpixel_RGB15(bmp, x, y, c); break;
	case IPIX_FMT_BGR16: _iputpixel_BGR15(bmp, x, y, c); break;
	case IPIX_FMT_RGB24: _iputpixel_RGB24(bmp, x, y, c); break;
	case IPIX_FMT_BGR24: _iputpixel_BGR24(bmp, x, y, c); break;
	case IPIX_FMT_RGB32: _iputpixel_RGB32(bmp, x, y, c); break;
	case IPIX_FMT_BGR32: _iputpixel_BGR32(bmp, x, y, c); break;
	case IPIX_FMT_ARGB32: _iputpixel_ARGB32(bmp, x, y, c); break;
	case IPIX_FMT_ABGR32: _iputpixel_ABGR32(bmp, x, y, c); break;
	case IPIX_FMT_RGBA32: _iputpixel_RGBA32(bmp, x, y, c); break;
	case IPIX_FMT_BGRA32: _iputpixel_BGRA32(bmp, x, y, c); break;
	case IPIX_FMT_ARGB_4444: _iputpixel_ARGB_4444(bmp, x, y, c); break;
	case IPIX_FMT_ABGR_4444: _iputpixel_ABGR_4444(bmp, x, y, c); break;
	case IPIX_FMT_RGBA_4444: _iputpixel_RGBA_4444(bmp, x, y, c); break;
	case IPIX_FMT_BGRA_4444: _iputpixel_BGRA_4444(bmp, x, y, c); break;
	case IPIX_FMT_ARGB_1555: _iputpixel_ARGB_1555(bmp, x, y, c); break;
	case IPIX_FMT_ABGR_1555: _iputpixel_ABGR_1555(bmp, x, y, c); break;
	case IPIX_FMT_RGBA_5551: _iputpixel_RGBA_5551(bmp, x, y, c); break;
	case IPIX_FMT_BGRA_5551: _iputpixel_BGRA_5551(bmp, x, y, c); break;
	}
}

void iblend_addpixel(IBITMAP *bmp, int x, int y, ICOLORD c)
{
	int sfmt;
	if (_ibitmap_pixfmt(bmp) == 0) ibitmap_set_pixfmt(bmp, 0);
	sfmt = _ibitmap_pixfmt(bmp);
	if ((unsigned int)x >= bmp->w || (unsigned int)y >= bmp->h)
		return;
	switch (sfmt)
	{
	case IPIX_FMT_8: _iaddpixel_8(bmp, x, y, c); break;
	case IPIX_FMT_RGB15: _iaddpixel_RGB15(bmp, x, y, c); break;
	case IPIX_FMT_BGR15: _iaddpixel_BGR15(bmp, x, y, c); break;
	case IPIX_FMT_RGB16: _iaddpixel_RGB15(bmp, x, y, c); break;
	case IPIX_FMT_BGR16: _iaddpixel_BGR15(bmp, x, y, c); break;
	case IPIX_FMT_RGB24: _iaddpixel_RGB24(bmp, x, y, c); break;
	case IPIX_FMT_BGR24: _iaddpixel_BGR24(bmp, x, y, c); break;
	case IPIX_FMT_RGB32: _iaddpixel_RGB32(bmp, x, y, c); break;
	case IPIX_FMT_BGR32: _iaddpixel_BGR32(bmp, x, y, c); break;
	case IPIX_FMT_ARGB32: _iaddpixel_ARGB32(bmp, x, y, c); break;
	case IPIX_FMT_ABGR32: _iaddpixel_ABGR32(bmp, x, y, c); break;
	case IPIX_FMT_RGBA32: _iaddpixel_RGBA32(bmp, x, y, c); break;
	case IPIX_FMT_BGRA32: _iaddpixel_BGRA32(bmp, x, y, c); break;
	case IPIX_FMT_ARGB_4444: _iaddpixel_ARGB_4444(bmp, x, y, c); break;
	case IPIX_FMT_ABGR_4444: _iaddpixel_ABGR_4444(bmp, x, y, c); break;
	case IPIX_FMT_RGBA_4444: _iaddpixel_RGBA_4444(bmp, x, y, c); break;
	case IPIX_FMT_BGRA_4444: _iaddpixel_BGRA_4444(bmp, x, y, c); break;
	case IPIX_FMT_ARGB_1555: _iaddpixel_ARGB_1555(bmp, x, y, c); break;
	case IPIX_FMT_ABGR_1555: _iaddpixel_ABGR_1555(bmp, x, y, c); break;
	case IPIX_FMT_RGBA_5551: _iaddpixel_RGBA_5551(bmp, x, y, c); break;
	case IPIX_FMT_BGRA_5551: _iaddpixel_BGRA_5551(bmp, x, y, c); break;
	}
}

void iblend_setpixel(IBITMAP *bmp, const int *xy, int n, ICOLORD c, int add)
{
	int sfmt;
	if (_ibitmap_pixfmt(bmp) == 0) ibitmap_set_pixfmt(bmp, 0);
	sfmt = _ibitmap_pixfmt(bmp);
	switch (sfmt)
	{
	case IPIX_FMT_8: _isetpixel_8(bmp, xy, n, c, add); break;
	case IPIX_FMT_RGB15: _isetpixel_RGB15(bmp, xy, n, c, add); break;
	case IPIX_FMT_BGR15: _isetpixel_BGR15(bmp, xy, n, c, add); break;
	case IPIX_FMT_RGB16: _isetpixel_RGB16(bmp, xy, n, c, add); break;
	case IPIX_FMT_BGR16: _isetpixel_BGR16(bmp, xy, n, c, add); break;
	case IPIX_FMT_RGB24: _isetpixel_RGB24(bmp, xy, n, c, add); break;
	case IPIX_FMT_BGR24: _isetpixel_BGR24(bmp, xy, n, c, add); break;
	case IPIX_FMT_RGB32: _isetpixel_RGB32(bmp, xy, n, c, add); break;
	case IPIX_FMT_BGR32: _isetpixel_BGR32(bmp, xy, n, c, add); break;
	case IPIX_FMT_ARGB32: _isetpixel_ARGB32(bmp, xy, n, c, add); break;
	case IPIX_FMT_ABGR32: _isetpixel_ABGR32(bmp, xy, n, c, add); break;
	case IPIX_FMT_RGBA32: _isetpixel_RGBA32(bmp, xy, n, c, add); break;
	case IPIX_FMT_BGRA32: _isetpixel_BGRA32(bmp, xy, n, c, add); break;
	case IPIX_FMT_ARGB_4444: _isetpixel_ARGB_4444(bmp, xy, n, c, add); break;
	case IPIX_FMT_ABGR_4444: _isetpixel_ABGR_4444(bmp, xy, n, c, add); break;
	case IPIX_FMT_RGBA_4444: _isetpixel_RGBA_4444(bmp, xy, n, c, add); break;
	case IPIX_FMT_BGRA_4444: _isetpixel_BGRA_4444(bmp, xy, n, c, add); break;
	case IPIX_FMT_ARGB_1555: _isetpixel_ARGB_1555(bmp, xy, n, c, add); break;
	case IPIX_FMT_ABGR_1555: _isetpixel_ABGR_1555(bmp, xy, n, c, add); break;
	case IPIX_FMT_RGBA_5551: _isetpixel_RGBA_5551(bmp, xy, n, c, add); break;
	case IPIX_FMT_BGRA_5551: _isetpixel_BGRA_5551(bmp, xy, n, c, add); break;
	}
}


// 点操作：批量绘制颜色表
void iblend_dispixel(IBITMAP *bmp, const int *xy, const ICOLORD *cc, 
	int n, int aa)
{
	int sfmt;
	if (_ibitmap_pixfmt(bmp) == 0) ibitmap_set_pixfmt(bmp, 0);
	sfmt = _ibitmap_pixfmt(bmp);
	switch (sfmt)
	{
	case IPIX_FMT_8: _idispixel_8(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_RGB15: _idispixel_RGB15(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_BGR15: _idispixel_BGR15(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_RGB16: _idispixel_RGB16(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_BGR16: _idispixel_BGR16(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_RGB24: _idispixel_RGB24(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_BGR24: _idispixel_BGR24(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_RGB32: _idispixel_RGB32(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_BGR32: _idispixel_BGR32(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_ARGB32: _idispixel_ARGB32(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_ABGR32: _idispixel_ABGR32(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_RGBA32: _idispixel_RGBA32(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_BGRA32: _idispixel_BGRA32(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_ARGB_4444: _idispixel_ARGB_4444(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_ABGR_4444: _idispixel_ABGR_4444(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_RGBA_4444: _idispixel_RGBA_4444(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_BGRA_4444: _idispixel_BGRA_4444(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_ARGB_1555: _idispixel_ARGB_1555(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_ABGR_1555: _idispixel_ABGR_1555(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_RGBA_5551: _idispixel_RGBA_5551(bmp, xy, cc, n, aa); break;
	case IPIX_FMT_BGRA_5551: _idispixel_BGRA_5551(bmp, xy, cc, n, aa); break;
	}
}


void *iblend_putpixel_vtable[] = {
	NULL,
	_iputpixel_8, 
	_iputpixel_RGB15, _iputpixel_BGR15, 
	_iputpixel_RGB16, _iputpixel_BGR16, 
	_iputpixel_RGB24, _iputpixel_BGR24, 
	_iputpixel_RGB32, _iputpixel_BGR32, 
	_iputpixel_ARGB32, _iputpixel_ABGR32, 
	_iputpixel_RGBA32, _iputpixel_BGRA32,
	_iputpixel_ARGB_4444, _iputpixel_ABGR_4444, 
	_iputpixel_RGBA_4444, _iputpixel_BGRA_4444,
	_iputpixel_ARGB_1555, _iputpixel_ABGR_1555, 
	_iputpixel_RGBA_5551, _iputpixel_BGRA_5551,
	NULL, 
};

void *iblend_addpixel_vtable[] = {
	NULL,
	_iaddpixel_8, 
	_iaddpixel_RGB15, _iaddpixel_BGR15, 
	_iaddpixel_RGB16, _iaddpixel_BGR16, 
	_iaddpixel_RGB24, _iaddpixel_BGR24, 
	_iaddpixel_RGB32, _iaddpixel_BGR32, 
	_iaddpixel_ARGB32, _iaddpixel_ABGR32, 
	_iaddpixel_RGBA32, _iaddpixel_BGRA32,
	_iaddpixel_ARGB_4444, _iaddpixel_ABGR_4444, 
	_iaddpixel_RGBA_4444, _iaddpixel_BGRA_4444,
	_iaddpixel_ARGB_1555, _iaddpixel_ABGR_1555, 
	_iaddpixel_RGBA_5551, _iaddpixel_BGRA_5551,
	NULL, 
};

void *iblend_getpixel_vtable[] = {
	NULL,
	_igetpixel_8, 
	_igetpixel_RGB15, _igetpixel_BGR15, 
	_igetpixel_RGB16, _igetpixel_BGR16, 
	_igetpixel_RGB24, _igetpixel_BGR24, 
	_igetpixel_RGB32, _igetpixel_BGR32, 
	_igetpixel_ARGB32, _igetpixel_ABGR32, 
	_igetpixel_RGBA32, _igetpixel_BGRA32,
	_igetpixel_ARGB_4444, _igetpixel_ABGR_4444, 
	_igetpixel_RGBA_4444, _igetpixel_BGRA_4444,
	_igetpixel_ARGB_1555, _igetpixel_ABGR_1555, 
	_igetpixel_RGBA_5551, _igetpixel_BGRA_5551,
	NULL,
};

