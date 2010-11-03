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

static inline int iFixFloor(int x) {
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
	int ui = u >> 16; \
	int vi = v >> 16; \
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
	int f00, f01, f10, f11, sum; \
	int umid = u; \
	int vmid = v; \
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
		int xx = umint; \
		int yy = vmint; \
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
	int f00, f01, f10, f11, sum; \
	int umid = u; \
	int vmid = v; \
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


#ifdef __cplusplus
}
#endif


#endif



