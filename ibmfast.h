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

#include <stddef.h>
#include <math.h>

#include "ibitmap.h"
#include "ibitmapm.h"
#include "ibmcols.h"
#include "iblit386.h"
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


#ifndef __IINT64_DEFINED
#define __IINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 IINT64;
#else
typedef long long IINT64;
#endif
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
	const ICOLORB *p = ITEX_UVPTR(ptr, fmt, ui, vi, shift, umask, vmask); \
	c = IPIX_FMT_READ(fmt, p); \
}	while (0)

#define ITEX_NORMGET(c, fmt, ui, vi, ptr, pitch, w, h) do { \
	unsigned int pos = vi * pitch + ui * IPIX_FMT_SIZE(fmt); \
	int condition = (ui | vi | (w - 1 - ui) | (h - 1 - vi)); \
	const ICOLORB *p = ptr + pos; \
	c = (condition >= 0)? IPIX_FMT_READ(fmt, p) : 0; \
}	while (0)



#define IOM_TRANSPARENT		0
#define IOM_REPEAT			1
#define IOM_WRAP			2
#define IOM_MIRROR			3

static inline int ioverflow_pos(int *x, int *y, int w, int h, int mode)
{
	int xx = *x;
	int yy = *y;
	if (xx >= 0 && xx < w && yy >= 0 && yy < h) return 0;
	switch (mode)
	{
	case IOM_TRANSPARENT:
		return -1;
	case IOM_REPEAT:
		xx = (xx < 0)? 0 : ((xx >= w)? (w - 1) : xx);
		yy = (yy < 0)? 0 : ((yy >= h)? (h - 1) : yy);
		break;
	case IOM_WRAP:
		xx = xx % w;
		yy = yy % h;
		if (xx < 0) xx += w;
		if (yy < 0) yy += h;
		break;
	case IOM_MIRROR:
		if (xx < 0) xx = (-xx) % w;
		else if (xx >= w) xx = w - 1 - (xx % w);
		if (yy < 0) yy = (-yy) % w;
		else if (yy >= h) yy = h - 1 - (yy % h);
		break;
	}
	*x = xx;
	*y = yy;
	return 0;
}

static inline ICOLORD ioverflow_get(int pixfmt, int x, int y, 
	const unsigned char *texture, long pitch, int w, int h, int mode, 
	ICOLORD default_color)
{
	const unsigned char *ptr;
	ICOLORD color;
	if (ioverflow_pos(&x, &y, w, h, mode) != 0)
		return default_color;
	ptr = texture + y * pitch + x * IBPP_TO_BYTES(ipixel_fmt[pixfmt].bpp);
	IPIXEL_FROM_PTR(color, pixfmt, ptr);
	return color;
}


#define IFIX_FLOOR(x) ((x) & (~0xffff))

static inline IINT32 iFixFloor(IINT32 x) {
	if (x >= 0) return (x & (~0xffff)); 
	return (x & (~0xffff)) - 0x10000;
}

static inline int ibitmap_pixfmt(IBITMAP *src) {
	if (_ibitmap_pixfmt(src) == 0) ibitmap_set_pixfmt(src, 0);
	return _ibitmap_pixfmt(src);
}

static inline int ibitmap_pixfmt_const(const IBITMAP *src) {
	int pixfmt = _ibitmap_pixfmt(src);
	if (pixfmt == 0) {
		if (src->bpp == 8) pixfmt = IPIX_FMT_8;
		else if (src->bpp == 15) pixfmt = IPIX_FMT_RGB15;
		else if (src->bpp == 16) pixfmt = IPIX_FMT_RGB16;
		else if (src->bpp == 24) pixfmt = IPIX_FMT_RGB24;
		else if (src->bpp == 32) pixfmt = IPIX_FMT_RGB32;
	}
	return pixfmt;
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

#define ITEX_GETCOL_NORMAL(c, fmt, u, v, ptr, pitch, w, h, om, dc) do { \
	ICOLORD raw, r, g, b, a; \
	IINT32 ui = u >> 16; \
	IINT32 vi = v >> 16; \
	if (ioverflow_pos(&ui, &vi, w, h, om) == 0) { \
		size_t pos = vi * pitch + ui * IPIX_FMT_SIZE(fmt); \
		const ICOLORB *p = ptr + pos; \
		raw = IPIX_FMT_READ(fmt, p); \
		IRGBA_FROM_PIXEL(raw, fmt, r, g, b, a); \
		c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
	}	else { \
		c = dc; \
	} \
}	while (0)


// (t00 * h1 + t01 * h2) * v1 + (t10 * h1 + t11 * h2) * v2
// t00 * (h1 * v1) + t01 * (h2 * v1) + t10 * (h1 * v2) + t11 * (h2 * v2)
#define ITEX_GETCOL_BILINEAR(c, fmt, u, v, ptr, pitch, w, h, om, dc) do { \
	ICOLORD text00, text11, text01, text10, c1, c2, c3, c4; \
	ICOLORD r00, g00, b00, a00, r01, g01, b01, a01; \
	ICOLORD r10, g10, b10, a10, r11, g11, b11, a11; \
	const ICOLORB *p00, *p01, *p10, *p11; \
	ICOLORD r, g, b, a; \
	IINT32 f00, f01, f10, f11, sum; \
	IINT32 umid = u; \
	IINT32 vmid = v; \
	IINT32 umidfloor = iFixFloor(umid); \
	IINT32 vmidfloor = iFixFloor(vmid); \
	IINT32 ufactor = (((umid - umidfloor) & 0xfffe) + 1) >> 1; \
	IINT32 vfactor = (((vmid - vmidfloor) & 0xfffe) + 1) >> 1; \
	IINT32 umint = umidfloor >> 16; \
	IINT32 vmint = vmidfloor >> 16; \
	IINT32 umw = w - 2 - umint; \
	IINT32 vmh = h - 2 - vmint; \
	IINT32 condition; \
	text00 = vmint * pitch + IPIX_FMT_SIZE(fmt) * umint; \
	text01 = text00 + IPIX_FMT_SIZE(fmt); \
	text10 = text00 + pitch; \
	text11 = text00 + pitch + IPIX_FMT_SIZE(fmt); \
	condition = (umint | vmint | umw | vmh); \
	p00 = ptr + text00; \
	p01 = ptr + text01; \
	p10 = ptr + text10; \
	p11 = ptr + text11; \
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
		IINT32 xx = umint; \
		IINT32 yy = vmint; \
		IRGBA_FROM_PIXEL(dc, ARGB32, r00, g00, b00, a00); \
		if (ioverflow_pos(&xx, &yy, w, h, om) == 0) { \
			p00 = ptr + yy * pitch + xx * IPIX_FMT_SIZE(fmt); \
			c1 = IPIX_FMT_READ(fmt, p00); \
			IRGBA_FROM_PIXEL(c1, fmt, r00, g00, b00, a00); \
		}	else { \
			IRGBA_FROM_PIXEL(dc, ARGB32, r00, g00, b00, a00); \
		} \
		xx = umint + 1; \
		if (ioverflow_pos(&xx, &yy, w, h, om) == 0) { \
			p01 = ptr + yy * pitch + xx * IPIX_FMT_SIZE(fmt); \
			c2 = IPIX_FMT_READ(fmt, p00); \
			IRGBA_FROM_PIXEL(c2, fmt, r01, g01, b01, a01); \
		}	else { \
			IRGBA_FROM_PIXEL(dc, ARGB32, r01, g01, b01, a01); \
		} \
		xx = umint; \
		yy = vmint + 1; \
		if (ioverflow_pos(&xx, &yy, w, h, om) == 0) { \
			p10 = ptr + yy * pitch + xx * IPIX_FMT_SIZE(fmt); \
			c3 = IPIX_FMT_READ(fmt, p00); \
			IRGBA_FROM_PIXEL(c3, fmt, r10, g10, b10, a10); \
		}	else { \
			IRGBA_FROM_PIXEL(dc, ARGB32, r10, g10, b10, a10); \
		} \
		xx = umint + 1; \
		if (ioverflow_pos(&xx, &yy, w, h, om) == 0) { \
			p11 = ptr + yy * pitch + xx * IPIX_FMT_SIZE(fmt); \
			c4 = IPIX_FMT_READ(fmt, p00); \
			IRGBA_FROM_PIXEL(c4, fmt, r11, g11, b11, a11); \
		}	else { \
			IRGBA_FROM_PIXEL(dc, ARGB32, r11, g11, b11, a11); \
		} \
	}	\
	sum = a00 + a01 + a10 + a11; \
	f00 = ((0x8000 - ufactor) * (0x8000 - vfactor)) >> 14; \
	f01 = (ufactor * (0x8000 - vfactor)) >> 14;  \
	f10 = ((0x8000 - ufactor) * vfactor) >> 14;  \
	f11 = (ufactor * vfactor) >> 14; \
	if (sum == -1) c = 0; \
	else {	\
		if (sum == 255 * 4) { a = 255; } \
		else { a = (a00 * f00 + a01 * f01 + a10 * f10 + a11 * f11) >> 16; } \
		r = (r00 * f00 + r01 * f01 + r10 * f10 + r11 * f11) >> 16; \
		g = (g00 * f00 + g01 * f01 + g10 * f10 + g11 * f11) >> 16; \
		b = (b00 * f00 + b01 * f01 + b10 * f10 + b11 * f11) >> 16; \
		c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
	}	\
}	while (0)



#define ITEX_UVGETCOL_NORMAL(c, fmt, u, v, ptr, shift, umask, vmask) do { \
	ICOLORD r, g, b, a; \
	ICOLORB *src; \
	src = ITEX_UVPTR(ptr, fmt, (u >> 16), (v >> 16), shift, umask, vmask); \
	IPIX_FMT_READ_RGBA(fmt, src, r, g, b, a); \
	c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
}	while (0)


#define ITEX_UVGETCOL_BILINEAR(c, fmt, u, v, ptr, shift, umask, vmask) do { \
	ICOLORD r00, g00, b00, a00, r01, g01, b01, a01; \
	ICOLORD r10, g10, b10, a10, r11, g11, b11, a11; \
	ICOLORD c1, c2, c3, c4; \
	const ICOLORB *p00, *p01, *p10, *p11; \
	ICOLORD r, g, b, a; \
	IINT32 f00, f01, f10, f11, sum; \
	IINT32 umid = u; \
	IINT32 vmid = v; \
	IINT32 umidfloor = iFixFloor(umid); \
	IINT32 vmidfloor = iFixFloor(vmid); \
	IINT32 ufactor = (((umid - umidfloor) & 0xfffe) + 1) >> 1; \
	IINT32 vfactor = (((vmid - vmidfloor) & 0xfffe) + 1) >> 1; \
	IINT32 umint = umidfloor >> 16; \
	IINT32 vmint = vmidfloor >> 16; \
	IINT32 pi0 = (umint & umask) * IPIX_FMT_SIZE(fmt); \
	IINT32 pi1 = ((umint + 1) & umask) * IPIX_FMT_SIZE(fmt); \
	IINT32 pi2 = (vmint & vmask) << shift; \
	IINT32 pi3 = ((vmint + 1) & vmask) << shift; \
	p00 = ptr + pi0 + pi2; \
	p01 = ptr + pi1 + pi2; \
	p10 = ptr + pi0 + pi3; \
	p11 = ptr + pi1 + pi3; \
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
	if (sum == -1) c = 0; \
	else {	\
		if (sum == 255 * 4) { a = 255; } \
		else { a = (a00 * f00 + a01 * f01 + a10 * f10 + a11 * f11) >> 16; } \
		r = (r00 * f00 + r01 * f01 + r10 * f10 + r11 * f11) >> 16; \
		g = (g00 * f00 + g01 * f01 + g10 * f10 + g11 * f11) >> 16; \
		b = (b00 * f00 + b01 * f01 + b10 * f10 + b11 * f11) >> 16; \
		c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
	}	\
}	while (0)


#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------
// INTERPOLATION
//---------------------------------------------------------------------
#define IBICUBIC_BITS	10
#define IBICUBIC_SIZE	((2 << IBICUBIC_BITS))

extern int ibicubic_lookup[IBICUBIC_SIZE * 2];
extern int ibicubic_tables[(2 << IBICUBIC_BITS)][4];
extern int ibicubic_inited;

void ibicubic_init(void);

/* cubic kernel (for a=-1 it is the same as ikernel_bicubic):
          /
         | (a+2)|t|**3 - (a+3)|t|**2 + 1     , |t| <= 1
  h(t) = | a|t|**3 - 5a|t|**2 + 8a|t| - 4a   , 1 < |t| <= 2
         | 0                                 , otherwise
          \
 * Often used values for a are -1 and -1/2.
 */
static inline float ikernel_cubic(const float t, const float a)
{
  float abs_t = (float)(t >= 0.0f? t : (-t));
  float abs_t_sq = abs_t * abs_t;
  if (abs_t < 1) 
	  return (a + 2) * abs_t_sq * abs_t - (a + 3) * abs_t_sq + 1;
  if (abs_t < 2) 
	  return a * abs_t_sq * abs_t - 5 * a * abs_t_sq + 8 * a * abs_t - 4 * a;
  return 0;
}

static inline float ikernel_bicubic(const float t)
{
  float abs_t = (float)(t >= 0.0f? t : (-t));
  float abs_t_sq = abs_t * abs_t;
  if (abs_t < 1) return 1 - 2 * abs_t_sq + abs_t_sq * abs_t;
  if (abs_t < 2) return 4 - 8 * abs_t + 5 * abs_t_sq - abs_t_sq * abs_t;
  return 0;
}

// n = 0, 1, run ibicubic_init first
static inline int ibicubic_fast(int x, int n)
{
	int i = ((x < 0)? (-x) : (x)) >> (16 - IBICUBIC_BITS);
	if (i > IBICUBIC_SIZE) return 0;
	if (n != 0) return ibicubic_lookup[IBICUBIC_SIZE + i];
	return ibicubic_lookup[i];
}

static inline int ibicubic_alpha(int u, int v, int *alphas, int n)
{
	int ufloor = iFixFloor(u);
	int vfloor = iFixFloor(v);
	int ufactor = (u - ufloor) >> (16 - IBICUBIC_BITS);
	int vfactor = (v - vfloor) >> (16 - IBICUBIC_BITS);
	int i, m, s;
	if (n != 0) n = (1 << IBICUBIC_BITS);
	for (i = 0, m = 0, s = 0; i < 4; i++) {
		int ki = ibicubic_tables[vfactor][i];
		int k1 = ibicubic_tables[ufactor][0];
		int k2 = ibicubic_tables[ufactor][1];
		int k3 = ibicubic_tables[ufactor][2];
		int k4 = ibicubic_tables[ufactor][3];
		int kk = ki >> 2;
		k1 = (kk * (k1 >> 2)) >> 12;
		k2 = (kk * (k2 >> 2)) >> 12;
		k3 = (kk * (k3 >> 2)) >> 12;
		k4 = (kk * (k4 >> 2)) >> 12;
		k1 = k1 >> 8; 
		k2 = k2 >> 8;
		k3 = k3 >> 8;
		k4 = k4 >> 8;
		alphas[m++] = k1;
		alphas[m++] = k2;
		alphas[m++] = k3;
		alphas[m++] = k4;
		s += k1 + k2 + k3 + k4;
	}
	alphas[5] += 256 - s;
	return s;
}


//---------------------------------------------------------------------
// BICUBIC INTERPOLATION
//---------------------------------------------------------------------
#define ITEX_GETCOL_BICUBIC(c, fmt, u, v, ptr, pitch, w, h, z, om, dc) do { \
	const ICOLORB *src, *pos; \
	ICOLORD colors[16], cc; \
	int cr, cg, cb, ca; \
	int r, g, b, a, i, j, k, aa; \
	int umid = u - 0; \
	int vmid = v - 0; \
	int umidfloor = iFixFloor(umid); \
	int vmidfloor = iFixFloor(vmid); \
	int ufactor = (((umid - umidfloor) & 0xfffe) + 1); \
	int vfactor = (((vmid - vmidfloor) & 0xfffe) + 1); \
	int umint = (umidfloor >> 16); \
	int vmint = (vmidfloor >> 16); \
	int um1 = umint - 1; \
	int vm1 = vmint - 1; \
	int umw = w - 3 - umint; \
	int vmh = h - 3 - vmint; \
	int condition; \
	int alphas[16]; \
	src = ptr + (vmint - 1) * pitch + IPIX_FMT_SIZE(fmt) * (umint - 1); \
	condition = (um1 | vm1 | umw | vmh); \
	if (condition >= 0) { \
		pos = src; \
		colors[ 0] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[ 1] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[ 2] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[ 3] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		pos = src + pitch; \
		colors[ 4] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[ 5] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[ 6] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[ 7] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		pos = src + pitch * 2; \
		colors[ 8] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[ 9] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[10] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[11] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		pos = src + pitch * 3; \
		colors[12] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[13] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[14] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
		colors[15] = IPIX_FMT_READ(fmt, pos); pos += IPIX_FMT_SIZE(fmt); \
	}	else { \
		ICOLORD DC; \
		IRGBA_FROM_PIXEL(dc, ARGB32, r, g, b, a); \
		DC = IRGBA_TO_PIXEL(fmt, r, g, b, a); \
		for (j = 0, k = 0; j < 4; j++) { \
			for (i = 0; i < 4; i++) { \
				int x = umint - 1 + i; \
				int y = vmint - 1 + i; \
				if (ioverflow_pos(&x, &y, w, h, om) == 0) { \
					pos = ptr + pitch * y + x * IPIX_FMT_SIZE(fmt); \
					colors[k++] = IPIX_FMT_READ(fmt, pos); \
				}	else { \
					colors[k++] = DC; \
				} \
			} \
		} \
	}	\
	r = g = b = a = 0; \
	ibicubic_alpha(ufactor, vfactor, alphas, z); \
	for (i = 0; i < 16; i++) { \
		aa = alphas[i]; \
		cc = colors[i]; \
		IRGBA_FROM_PIXEL(cc, fmt, cr, cg, cb, ca); \
		r += cr * aa; \
		g += cg * aa; \
		b += cb * aa; \
		a += ca * aa; \
	} \
	r = (r + 0) >> 8; \
	g = (g + 0) >> 8; \
	b = (b + 0) >> 8; \
	a = (a + 0) >> 8; \
	r = _iclip256[r]; \
	g = _iclip256[g]; \
	b = _iclip256[b]; \
	a = _iclip256[a]; \
	c = IRGBA_TO_PIXEL(ARGB32, r, g, b, a); \
}	while (0)


#define ITEX_GETCOL_BICUBIC1(c, fmt, u, v, ptr, pitch, w, h, om, dc) \
		ITEX_GETCOL_BICUBIC(c, fmt, u, v, ptr, pitch, w, h, 0, om, dc)
	
#define ITEX_GETCOL_BICUBIC2(c, fmt, u, v, ptr, pitch, w, h, om, dc) \
		ITEX_GETCOL_BICUBIC(c, fmt, u, v, ptr, pitch, w, h, 0, om, dc)


//---------------------------------------------------------------------
// Inline Utilities
//---------------------------------------------------------------------
static inline IRECT *irect_set(IRECT *dst, int l, int t, int r, int b)
{
	dst->left = l;
	dst->top = t;
	dst->right = r;
	dst->bottom = b;
	return dst;
}

static inline IRECT *irect_copy(IRECT *dst, const IRECT *src)
{
	dst->left = src->left;
	dst->top = src->top;
	dst->right = src->right;
	dst->bottom = src->bottom;
	return dst;
}

static inline IRECT *irect_offset(IRECT *self, int x, int y)
{
	self->left += x;
	self->top += y;
	self->right += x;
	self->bottom += y;
	return self;
}

static inline int irect_contains(const IRECT *self, int x, int y)
{
	return (x >= self->left && y >= self->top &&
		x < self->right && y < self->bottom);
}

static inline int irect_intersects(const IRECT *self, const IRECT *src)
{
	return !((src->right <= self->left) || (src->bottom <= self->top) ||
		(src->left >= self->right) || (src->top >= self->bottom));
}

static inline IRECT *irect_intersection(IRECT *dst, const IRECT *src)
{
	int x1 = (dst->left > src->left)? dst->left : src->left;
	int x2 = (dst->right < src->right)? dst->right : src->right;
	int y1 = (dst->top > src->top)? dst->top : src->top;
	int y2 = (dst->bottom < src->bottom)? dst->bottom : src->bottom;
	if (x1 > x2 || y1 > y2) {
		dst->left = 0;
		dst->top = 0;
		dst->right = 0;
		dst->bottom = 0;
	}	else {
		dst->left = x1;
		dst->top = y1;
		dst->right = x2;
		dst->bottom = y2;
	}
	return dst;
}

static inline IRECT *irect_union(IRECT *dst, const IRECT *src)
{
	int x1 = (dst->left < src->left)? dst->left : src->left;
	int x2 = (dst->right > src->right)? dst->right : src->right;
	int y1 = (dst->top < src->top)? dst->top : src->top;
	int y2 = (dst->bottom > src->bottom)? dst->bottom : src->bottom;
	dst->left = x1;
	dst->top = y1;
	dst->right = x2;
	dst->bottom = y2;
	return dst;
}


//---------------------------------------------------------------------
// floating point operation
//---------------------------------------------------------------------
// fast (1 / sqrt(x)) (from quake3)
static inline float _isqrtinv_ieee(float x) {
	union { IINT32 intpart; float floatpart; } convert;
	float xhalf = 0.5f * x;
	convert.floatpart = x;
	convert.intpart = 0x5f37642f - (convert.intpart >> 1);
	x = convert.floatpart;
	return x * (1.5f - xhalf * x * x);
}

// fast sqrt(x) (from flipcode)
static inline float _isqrt_ieee(float x) {
	union { IINT32 intpart; float floatpart; } convert1, convert2;
	convert1.floatpart = x;
	convert2.floatpart = x;
	convert1.intpart = 0x1fbcf800 + (convert1.intpart >> 1);
	convert2.intpart = 0x5f37642f - (convert2.intpart >> 1);
	return 0.5f * (convert1.floatpart + convert2.floatpart * x);
}

// fast ftoi(x) (from flipcode) - changed from:
static inline IINT32 _iftoi_ieee(float f) {
	union { IINT32 intpart; float floatpart; } convert;
	IINT32 sign, mantissa, exponent, r;
	convert.floatpart = f;
	sign = (convert.intpart >> 31);
	mantissa = (convert.intpart & ((1 << 23) - 1)) | (1 << 23);
	exponent = ((convert.intpart & 0x7ffffffful) >> 23) - 127;
	r = ((IUINT32)(mantissa) << 8) >> (31 - exponent);
	return ((r ^ sign) - sign) & ~(exponent >> 31);
}

// fast double2int (from lua)
// cannot used with D3D9
static inline IINT32 _idtoi_ieee(double d) {
	union { double l_d; IINT32 l_l; } u;
	u.l_d = (d) + 6755399441055744.0;
	return u.l_l;
}


#if defined(__i386__) || defined(__amd64__)
#define IHAVE_FAST_FLOAT
#endif

static inline float _isqrtinv(float x) {
#ifdef IHAVE_FAST_FLOAT
	return _isqrtinv_ieee(x);
#else
	return 1.0f / sqrtf(x);
#endif
}

static inline float _isqrt(float x) {
#ifdef IHAVE_FAST_FLOAT
	return _isqrt_ieee(x);
#else
	return sqrt(x);
#endif
}

static inline IINT32 _ifloat2int(float x) {
#ifdef IHAVE_FAST_FLOAT
	return _iftoi_ieee(x);
#else
	return (IINT32)(x);
#endif
}

static inline IINT32 _idouble2int(float x) {
	return (IINT32)(x);
}

static inline float _ifloat_abs(float x) {
	return (x >= 0.0f)? x : (-x);
}

static inline IINT32 _iint_abs(IINT32 x) {
	return (x >= 0)? x : (-x);
}


#define IMATH_PI        ((float)3.141592654f)
#define IMATH_PI2       ((float)6.283185307f)
#define IMATH_PI_DIV_2	((float)1.570796327f)
#define IMATH_PI_DIV_4  ((float)0.785398163f)
#define IMATH_INV       ((float)0.318309886f)

#define IEPSILON_E4     ((float)1E-4)
#define IEPSILON_E5     ((float)1E-5)
#define IEPSILON_E6     ((float)1E-6)

#define IFLOAT_ABS(x)          _ifloat_abs(x)
#define IFLOAT_EQUALZ(x, y, z) (IFLOAT_ABS((x) - (y)) < (z))? 1 : 0)
#define IFLOAT_EQUAL(x, y)     IFLOAT_EQUALZ(x, y, IEPSILON_E5)

//---------------------------------------------------------------------
// fixed point number definition
//---------------------------------------------------------------------
#define IFIX_SHIFT      16

#define IFIX_MAG        (1 << IFIX_SHIFT)
#define IFIX_DP_MASK    (IFIX_MAG - 1)
#define IFIX_WP_MASK    (~IFIX_DP_MASK)
#define IFIX_ROUNDUP    (IFIX_MAG - 1)

typedef IINT32 IFIXED;
typedef IINT64 IFIX64;

static inline IFIXED ifixed_float2fixed(float x) {
	return _ifloat2int(x * 65536.0f);
}

static inline float ifixed_fixed2float(IFIXED f) {
	return (f / 65536.0f);
}

static inline IFIXED ifixed_int2fixed(IINT32 i) {
	return (i << 16);
}

static inline IINT32 ifixed_fixed2int(IFIXED f) {
	return (f >> 16);
}

static inline IFIXED ifixed_integer_part(IFIXED x) {
	return (x & (~0xffff));
}

static inline IFIXED ifixed_floor(IFIXED x) {
	if (x >= 0) return (x & (~0xffff)); 
	return (x & (~0xffff)) - 0x10000;
}

static inline IFIXED ifixed_ceil(IFIXED x) {
	if (x > 0) return ifixed_integer_part((x - 1) + 0x10000);
	return ifixed_int2fixed(-(-x >> 16));
}

static inline int ifixed_is_integer(IFIXED x) {
	return (x & 0xffff) == 0;
}

static inline IFIXED ifixed_double2fixed(double f) {
	return _idouble2int(f * 65536.0);
}

static inline double ifixed_fixed2double(IFIXED f) {
	return f / 65536.0;
}

static inline IFIXED ifixed_from_26_6(IINT32 x) {
	return x << 10;
}

static inline IINT32 ifixed_to_26_6(IFIXED x) {
	return x >> 10;
}

static inline IFIXED ifixed_from_int(IINT32 x) {
	return ifixed_int2fixed(x);
}

static inline IFIXED ifixed_from_float(float x) {
	return ifixed_float2fixed(x);
}

static inline IFIXED ifixed_from_double(double x) {
	return ifixed_double2fixed(x);
}

static inline IINT32 ifixed_to_int(IFIXED x) {
	return ifixed_fixed2int(x);
}

static inline float ifixed_to_float(IFIXED x) {
	return ifixed_fixed2float(x);
}

static inline double ifixed_to_double(IFIXED x) {
	return ifixed_fixed2double(x);
}

static inline IFIXED ifixed_mul(IFIXED x, IFIXED y) {
	return (IFIXED)(((IINT64)x) * y) >> 16;
}

static inline IFIXED ifixed_div(IFIXED x, IFIXED y) {
	float fx = ifixed_fixed2float(x);
	float fy = ifixed_fixed2float(y);
	return ifixed_float2fixed(fx / fy);
}

static inline IFIXED ifixed_div2(IFIXED x, IFIXED y) {
	double dx = ifixed_fixed2double(x);
	double dy = ifixed_fixed2double(y);
	return ifixed_double2fixed(dx / dy);
}

static inline IFIXED ifixed_sin(IFIXED x) {
	return ifixed_float2fixed(sinf(ifixed_fixed2float(x)));
}

static inline IFIXED ifixed_cos(IFIXED x) {
	return ifixed_float2fixed(cosf(ifixed_fixed2float(x)));
}

static inline IFIXED ifixed_tan(IFIXED x) {
	return ifixed_float2fixed(tanf(ifixed_fixed2float(x)));
}

static inline IFIXED ifixed_atan2(IFIXED y, IFIXED x) {
	float fx = ifixed_fixed2float(x);
	float fy = ifixed_fixed2float(y);
	return ifixed_float2fixed(atan2f(fy, fx));
}

static inline IFIXED ifixed_sqrt(IFIXED x) {
	float fx = ifixed_fixed2float(x);
	return ifixed_float2fixed(_isqrt(fx));
}

// 1 / sqrt(x)
static inline IFIXED ifixed_sqrtinv(IFIXED x) {
	float fx = ifixed_fixed2float(x);
	return ifixed_float2fixed(_isqrtinv(fx));
}

static inline IFIX64 ifix64_from_int(IINT32 x) {
	return ((IINT64)x) << 32;
}

static inline IINT32 ifix64_to_int(IFIX64 x) {
	return (IINT32)(x >> 32);
}

static inline IFIX64 ifix64_from_float(float x) {
	return (IFIX64)(x * 4294967296.0f);
}

static inline float ifix64_to_float(IFIX64 x) {
	return x / 4294967296.0f;
}

static inline IFIX64 ifix64_from_double(double x) {
	return (IFIX64)(x * 4294967296.0);
}

static inline double ifix64_to_double(IFIX64 x) {
	return x / 4294967296.0;
}

static inline IFIX64 ifix64_mul(IFIX64 x, IFIX64 y) {
	double dx = ifix64_to_double(x);
	double dy = ifix64_to_double(y);
	return ifix64_from_double(dx * dy);
}

static inline IFIX64 ifix64_div(IFIX64 x, IFIX64 y) {
	double dx = ifix64_to_double(x);
	double dy = ifix64_to_double(y);
	return ifix64_from_double(dx / dy);
}

static inline IFIX64 ifix64_integer_part(IFIX64 x) {
	return x & (~((IINT64)0xffffffff));
}

static inline IFIX64 ifix64_floor(IFIX64 x) {
	if (x >= 0) return (x & (~((IINT64)0xffffffff))); 
	return (x & (~((IINT64)0xffffffff))) - (((IINT64)1) << 32);
}

static inline IFIX64 ifix64_ceil(IFIX64 x) {
	if (x > 0) return ifix64_integer_part((x - 1) + (((IINT64)1) << 32));
	return ifix64_from_int(-(-x >> 32));
}

static inline IFIX64 ifix64_from_fixed(IFIXED x) {
	return ((IFIX64)x) << 16;
}

static inline IFIXED ifix64_to_fixed(IFIX64 x) {
	return (IFIXED)(x >> 16);
}


//---------------------------------------------------------------------
// Interfaces
//---------------------------------------------------------------------
int ibitmap_scale_clip(const IBITMAP *dst, IRECT *drect, const IRECT *clip,
	const IBITMAP *src, IRECT *srect);

#define ISTRETCH_FAST		256

#define IFILTER_NORMAL		0
#define IFILTER_NEAREST		1
#define IFILTER_BILINEAR	2
#define IFILTER_BICUBIC		3
#define IFILTER_BICUBIC2	4
#define IFILTER_TEND		5


// 插值方式取得 ARGB32颜色
ICOLORD interpolated_getcol(int pixfmt, int filter, int u, int v, 
	const unsigned char *src, long pitch, int w, int h, int ofmode,
	ICOLORD ofcol);

// 插值方式取得 ARGB32颜色
ICOLORD interpolated_bitmap(const IBITMAP *bmp, int u, int v, 
	int filter, int ofmode, ICOLORD ofcol);


// 平滑缩放：支持BILINEAR, BICUBIC和SMOOTH几种算法，但无混色
int ibitmap_stretch(IBITMAP *dst, const IRECT *rectdst, 
	const IRECT *clip, IBITMAP *src, const IRECT *rectsrc, int flags,
	int filter, int ofmode, ICOLORD ofcol);


// 位图重新采样
IBITMAP *ibitmap_resample(IBITMAP *src, int newwidth, int newheight,
	int filter, int ofmode, ICOLORD ofcolor);


//---------------------------------------------------------------------
// 扫描线：光栅化
//---------------------------------------------------------------------
typedef struct
{
	unsigned char *pixel;
	int width;
	int magic;
	IINT32 u;
	IINT32 v;
	IINT32 du;
	IINT32 dv;
}	ISCANLINE;

#define IFILTER_MASK	6		// 关键色过滤为 0

// 取得扫描线，保存成ARGB32的格式到 scanlines定义的扫描线中
int ibitmap_scan(IBITMAP *src, ISCANLINE *scanlines, int count,
	int filter, const IRGB *pal, int om, ICOLORD dc);


#ifdef __cplusplus
}
#endif


#endif



