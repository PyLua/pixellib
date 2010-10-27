//=====================================================================
//
// ibmfast.h - ibitmap fast operation
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================
#ifndef __IBMFAST_H__
#define __IBMFAST_H__

#include "ibitmap.h"
#include "ibitmapm.h"
#include "ibmcols.h"
#include "iblit386.h"


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


#if defined(_MSC_VER) || defined(__BORLANDC__)
#define IINT64T __int64
#else
#define IINT64T long long
#endif


//---------------------------------------------------------------------
// Basic Class
//---------------------------------------------------------------------
typedef struct IRECT
{
	int left;
	int top;
	int right;
	int bottom;
}	IRECT;




//---------------------------------------------------------------------
// Basic Texture Macro
//---------------------------------------------------------------------
#define ITEX_UVPTR(ptr, fmt, ui, vi, shift, umask, vmask) ( \
	((ICOLORB*)(ptr)) + \
	(((ui) & umask) * IPIX_FMT_SIZE(fmt)) + \
	(((vi) & vmask) << shift) )

#define ITEX_UVGET(c, fmt, ui, vi, ptr, shift, umask, vmask) do { \
	ICOLORB *p = ITEX_UVPTR(ptr, fmt, ui, vi, shift, umask, vmask); \
	c = IPIX_FMT_READ(fmt, p); \
}	while (0)

#define ITEX_NORMGET(c, fmt, ui, vi, ptr, pitch, w, h) do { \
	unsigned int pos = vi * pitch + ui * IPIX_FMT_SIZE(fmt); \
	int condition = (ui | vi | (w - 1 - ui) | (h - 1 - vi)); \
	ICOLORB *p = ptr + pos; \
	c = (condition >= 0)? IPIX_FMT_READ(fmt, p) : 0; \
}	while (0)

#define ITEX_GETCOL_LINEAR(c, fmt, u, v, ptr, pitch, w, h) do { \
	ICOLORD raw, r, g, b, a; \
	int ui = u >> 16; \
	int vi = v >> 16; \
	ITEX_NORMGET(raw, fmt, ui, vi, ptr, pitch, w, h); \
	IRGBA_FROM_PIXEL(raw, fmt, r, g, b, a); \
	c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
}	while (0)


#define IFIX_FLOOR(x) ((x) & (~0xffff))

static inline int iFixFloor(int x) {
	return (x & (~0xffff));
}

static inline int ibitmap_pixfmt(IBITMAP *src) {
	if (_ibitmap_pixfmt(src) == 0) ibitmap_set_pixfmt(src, 0);
	return _ibitmap_pixfmt(src);
}

static inline void ibilinear_alpha(int u, int v, 
	int *a00, int *a01, int *a10, int *a11)
{
	int ufloor = iFixFloor(u);
	int vfloor = iFixFloor(v);
	int ufactor = (((u - ufloor) & 0xfffe) + 1) >> 1;
	int vfactor = (((v - vfloor) & 0xfffe) + 1) >> 1;
	*a00 = ((0x8000 - ufactor) * (0x8000 - vfactor)) >> 14; 
	*a01 = (ufactor * (0x8000 - vfactor)) >> 14;  
	*a10 = ((0x8000 - ufactor) * vfactor) >> 14;  
	*a11 = (ufactor * vfactor) >> 14; 
}

static inline int ibicubic_kernel(int x)
{
	int xm1, xp1, xp2;
	int a, b, c, d, v;
	if (x > 0x20000) return 0;
	x   = x >> 4;
	xm1 = x - 0x1000;
	xp1 = x + 0x1000;
	xp2 = x + 0x2000;
	a = (xp2 <= 0)? 0 : (((xp2 * xp2) >> 12) * xp2) >> 8;
	b = (xp1 <= 0)? 0 : (((xp1 * xp1) >> 12) * xp1) >> 8;
	c = (x   <= 0)? 0 : (((x   * x  ) >> 12) * x  ) >> 8;
	d = (xm1 <= 0)? 0 : (((xm1 * xm1) >> 12) * xm1) >> 8;
	v = (a - (b << 2) + (c << 2) + (c << 1) - (d << 2));
	return ((10922 >> 4) * (v >> 4)) >> 8;
}

static inline void ibicubic_alpha(int u, int v, int *alphas)
{
	int ufloor = iFixFloor(u);
	int vfloor = iFixFloor(v);
	int ufactor = (((u - ufloor) & 0xfffe) + 1) >> 1;
	int vfactor = (((v - vfloor) & 0xfffe) + 1) >> 1;
	int i, j, m, s;
	for (j = -1, m = 0, s = 0; j <= 1; j++) {
		int k1 = ibicubic_kernel(vfactor - (j << 16));
		for (i = -1; i <= 1; i++) {
			int k2 = ibicubic_kernel(ufactor - (i << 16));
			int k = ((k1 >> 4) * (k2 >> 4)) >> 10;
			int a = ((k << 8) - k) >> 14;
			alphas[m++] = a;
			s += a;
		}
	}
	alphas[4] += 255 - s;
}


// (t00 * h1 + t01 * h2) * v1 + (t10 * h1 + t11 * h2) * v2
// t00 * (h1 * v1) + t01 * (h2 * v1) + t10 * (h1 * v2) + t11 * (h2 * v2)
#define ITEX_GETCOL_BILINEAR(c, fmt, u, v, texture, pitch, w, h) do { \
	ICOLORD text00, text11, text01, text10, c1, c2, c3, c4; \
	ICOLORD r00, g00, b00, a00, r01, g01, b01, a01; \
	ICOLORD r10, g10, b10, a10, r11, g11, b11, a11; \
	ICOLORB *p00, *p01, *p10, *p11; \
	ICOLORD r, g, b, a; \
	int f00, f01, f10, f11, sum; \
	int umid = u - 0x8000; \
	int vmid = v - 0x8000; \
	int umidfloor = iFixFloor(umid); \
	int vmidfloor = iFixFloor(vmid); \
	int ufactor = (((umid - umidfloor) & 0xfffe) + 1) >> 1; \
	int vfactor = (((vmid - vmidfloor) & 0xfffe) + 1) >> 1; \
	int umint = umidfloor >> 16; \
	int vmint = vmidfloor >> 16; \
	int umw = w - 2 - umint; \
	int vmh = h - 2 - vmint; \
	int condition; \
	text00 = vmint * pitch + IPIX_FMT_SIZE(fmt) * umint; \
	text01 = text00 + IPIX_FMT_SIZE(fmt); \
	text10 = text00 + pitch; \
	text11 = text00 + pitch + IPIX_FMT_SIZE(fmt); \
	condition = (umint | vmint | umw | vmh); \
	p00 = texture + text00; \
	p01 = texture + text01; \
	p10 = texture + text10; \
	p11 = texture + text11; \
	if (condition >= 0) { \
		c1 = IPIX_FMT_READ(fmt, p00); \
		c2 = IPIX_FMT_READ(fmt, p01); \
		c3 = IPIX_FMT_READ(fmt, p10); \
		c4 = IPIX_FMT_READ(fmt, p11); \
		IRGBA_FROM_PIXEL(c1, fmt, r00, g00, b00, a00); \
		IRGBA_FROM_PIXEL(c2, fmt, r01, g01, b01, a01); \
		IRGBA_FROM_PIXEL(c3, fmt, r10, g10, b10, a10); \
		IRGBA_FROM_PIXEL(c4, fmt, r11, g11, b11, a11); \
	}	else { \
		int test1 = (umint >= 0 && umint < w); \
		int test2 = (umint + 1 >= 0 && umint + 1 < w); \
		r00 = g00 = b00 = a00 = 0; \
		r01 = g01 = b01 = a01 = 0; \
		r10 = g10 = b10 = a10 = 0; \
		r11 = g11 = b11 = a11 = 0; \
		if (vmint >= 0 && vmint < h) { \
			if (test1) { \
				c1 = IPIX_FMT_READ(fmt, p00); \
				IRGBA_FROM_PIXEL(c1, fmt, r00, g00, b00, a00); \
			} \
			if (test2) { \
				c2 = IPIX_FMT_READ(fmt, p01); \
				IRGBA_FROM_PIXEL(c2, fmt, r01, g01, b01, a01); \
			} \
		} \
		if (vmint + 1 >= 0 && vmint + 1 < h) { \
			if (test1) { \
				c3 = IPIX_FMT_READ(fmt, p10);  \
				IRGBA_FROM_PIXEL(c3, fmt, r10, g10, b10, a10); \
			} \
			if (test2) { \
				c4 = IPIX_FMT_READ(fmt, p11);  \
				IRGBA_FROM_PIXEL(c4, fmt, r11, g11, b11, a11); \
			} \
		} \
	}	\
	sum = a00 + a01 + a10 + a11; \
	f00 = ((0x8000 - ufactor) * (0x8000 - vfactor)) >> 14; \
	f01 = (ufactor * (0x8000 - vfactor)) >> 14;  \
	f10 = ((0x8000 - ufactor) * vfactor) >> 14;  \
	f11 = (ufactor * vfactor) >> 14; \
	if (sum == 0) c = 0; \
	else {	\
		if (sum == 255 * 4) { a = 255; } \
		else { a = (a00 * f00 + a01 * f01 + a10 * f10 + a11 * f11) >> 16; } \
		r = (r00 * f00 + r01 * f01 + r10 * f10 + r11 * f11) >> 16; \
		g = (g00 * f00 + g01 * f01 + g10 * f10 + g11 * f11) >> 16; \
		b = (b00 * f00 + b01 * f01 + b10 * f10 + b11 * f11) >> 16; \
		c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
	}	\
}	while (0)



#define ITEX_UVGETCOL_LINEAR(c, fmt, u, v, ptr, shift, umask, vmask) do { \
	ICOLORD r, g, b, a; \
	ICOLORB *src; \
	src = ITEX_UVPTR(ptr, fmt, (u >> 16), (v >> 16), shift, umask, vmask); \
	IPIX_FMT_READ_RGBA(fmt, src, r, g, b, a); \
	c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
}	while (0)


#define ITEX_UVGETCOL_BILINEAR(c, fmt, u, v, ptr, shift, umask, vmask) do { \
	ICOLORD text00, text11, text01, text10, c1, c2, c3, c4; \
	ICOLORD r00, g00, b00, a00, r01, g01, b01, a01; \
	ICOLORD r10, g10, b10, a10, r11, g11, b11, a11; \
	ICOLORB *p00, *p01, *p10, *p11; \
	ICOLORD r, g, b, a; \
	int f00, f01, f10, f11, sum; \
	int umid = u - 0x8000; \
	int vmid = v - 0x8000; \
	int umidfloor = iFixFloor(umid); \
	int vmidfloor = iFixFloor(vmid); \
	int ufactor = (((umid - umidfloor) & 0xfffe) + 1) >> 1; \
	int vfactor = (((vmid - vmidfloor) & 0xfffe) + 1) >> 1; \
	int umint = umidfloor >> 16; \
	int vmint = vmidfloor >> 16; \
	int pi0 = (umint & umask) * IPIX_FMT_SIZE(fmt); \
	int pi1 = ((umint + 1) & umask) * IPIX_FMT_SIZE(fmt); \
	int pi2 = (vmint & vmask) << shift; \
	int pi3 = ((vmint + 1) & vmask) << shift; \
	p00 = texture + pi0 + pi2; \
	p01 = texture + pi1 + pi2; \
	p10 = texture + pi0 + pi3; \
	p11 = texture + pi1 + pi3; \
	c1 = IPIX_FMT_READ(fmt, p00); \
	c2 = IPIX_FMT_READ(fmt, p01); \
	c3 = IPIX_FMT_READ(fmt, p10); \
	c4 = IPIX_FMT_READ(fmt, p11); \
	IRGBA_FROM_PIXEL(c1, fmt, r00, g00, b00, a00); \
	IRGBA_FROM_PIXEL(c2, fmt, r01, g01, b01, a01); \
	IRGBA_FROM_PIXEL(c3, fmt, r10, g10, b10, a10); \
	IRGBA_FROM_PIXEL(c4, fmt, r11, g11, b11, a11); \
	sum = a00 + a01 + a10 + a11; \
	f00 = ((0x8000 - ufactor) * (0x8000 - vfactor)) >> 14; \
	f01 = (ufactor * (0x8000 - vfactor)) >> 14;  \
	f10 = ((0x8000 - ufactor) * vfactor) >> 14;  \
	f11 = (ufactor * vfactor) >> 14; \
	if (sum == 0) c = 0; \
	else {	\
		if (sum == 255 * 4) { a = 255; } \
		else { a = (a00 * f00 + a01 * f01 + a10 * f10 + a11 * f11) >> 16; } \
		r = (r00 * f00 + r01 * f01 + r10 * f10 + r11 * f11) >> 16; \
		g = (g00 * f00 + g01 * f01 + g10 * f10 + g11 * f11) >> 16; \
		b = (b00 * f00 + b01 * f01 + b10 * f10 + b11 * f11) >> 16; \
		c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
	}	\
}	while (0)


//---------------------------------------------------------------------
// Inline Utilities
//---------------------------------------------------------------------
static inline IRECT *irect_set(IRECT *rect, int l, int t, int r, int b)
{
	rect->left = l;
	rect->top = t;
	rect->right = r;
	rect->bottom = b;
	return rect;
}

static inline IRECT *irect_copy(IRECT *dst, const IRECT *src)
{
	dst->left = src->left;
	dst->top = src->top;
	dst->right = src->right;
	dst->bottom = src->bottom;
	return dst;
}



#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------
// Interfaces
//---------------------------------------------------------------------
int ibitmap_scale_clip(const IBITMAP *dst, IRECT *drect, const IRECT *clip,
	const IBITMAP *src, IRECT *srect);

#define ISTRETCH_FAST		256

// 图形快速缩放（不带抗锯齿）
// 可用参数：ISTRETCH_FAST，IBLIT_HFLIP, IBLIT_VFLIP
// clip可以为NULL
int ibitmap_resize(IBITMAP *dst, const IRECT *rectdst, const IRECT *clip,
	const IBITMAP *src, const IRECT *rectsrc, int flags);


#ifdef __cplusplus
}
#endif


#endif



