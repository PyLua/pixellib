/**********************************************************************
 *
 * ibmcols.h - internal color definition
 *
 * NOTE: arch. independence header
 * for more information, please see the readme file
 *
 **********************************************************************/


#ifndef __IBMCOLS_H__
#define __IBMCOLS_H__

#include "ibitmap.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


/**********************************************************************
 * COLOR SIZE DEFINITION (ICOLORB:8bit, ICOLORW:16bit, ICOLORD:32bit)
 **********************************************************************/
#ifndef __IBITMAP_COLOR_TYPES
#define __IBITMAP_COLOR_TYPES
typedef unsigned char ICOLORB;
typedef unsigned short ICOLORW;
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) \
	 || defined(__i386__) || defined(__i386) || defined(_M_X86)
	typedef unsigned int ICOLORD;
	typedef int ISTDINT32;
#elif defined(__MACOS__)
	typedef UInt32 ICOLORD;
	typedef SInt32 ISTDINT32;
#elif defined(__APPLE__) && defined(__MACH__)
	#include <sys/types.h>
	typedef u_int32_t ICOLORD;
	typedef int32_t ISTDINT32;
#elif defined(__BEOS__)
	#include <sys/inttypes.h>
	typedef u_int32_t ICOLORD;
	typedef int32_t ISTDINT32;
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64__) || \
	defined(__amd64) || defined(_M_IA64) || defined(_M_AMD64)
	typedef unsigned int ICOLORD;
	typedef int ISTDINT32;
#elif defined(_MSC_VER) || defined(__BORLANDC__)
	typedef unsigned __int32 ICOLORD;
	typedef __int32 ISTDINT32;
#elif defined(__GNUC__)
	#include <stdint.h>
	typedef uint32_t ICOLORD;
	typedef int32_t ISTDINT32;
#else 
	typedef unsigned long ICOLORD; 
	typedef long ISTDINT32;
#endif
#endif


/**********************************************************************
 * COLOR OPERATION MACRO
 **********************************************************************/
#ifndef __IBITMAP_COLOR_TYPES
#define __IBITMAP_COLOR_TYPES
typedef unsigned long ICOLORD;     /* RAW-COLOR INTEGER: 32bit */
typedef unsigned short ICOLORW;     /* RAW-COLOR INTEGER: 16bit */
typedef unsigned char ICOLORB;     /* RAW-COLOR INTEGER: 8bit. */
#endif

typedef struct IRGB
{ 
	unsigned char r, g, b; 
	unsigned char reserved;
}	IRGB;


#ifdef __cplusplus
extern "C" {
#endif

/* endian checking */
extern long _ishift_endian;
extern long _ishift_endmag;

/* bytes shift 24bits */
extern int _ishift_data_b1;
extern int _ishift_data_b2;
extern int _ishift_data_b3;

/* color scale 1/4/5/6 bits -> 8 bits lookup table */
extern int _iscale_rgb_1[];
extern int _iscale_rgb_4[];
extern int _iscale_rgb_5[];  
extern int _iscale_rgb_6[];

/* color converter 15/16 bits -> 32 bits lookup table */
extern ICOLORD _iconvert_rgb_15_32[];
extern ICOLORD _iconvert_rgb_16_32[];

/* color converter 15 bits <-> 16 bits lookup table */
extern ICOLORW _iconvert_rgb_15_16[];
extern ICOLORW _iconvert_rgb_16_15[];
extern ICOLORW _iconvert_rgb_15_15[];
extern ICOLORW _iconvert_rgb_16_16[];

/* color converter 1555 <-> 4444 <-> 8888 lookup table */
extern ICOLORW _iconvert_rgb_1555_4444[];
extern ICOLORW _iconvert_rgb_4444_1555[];
extern ICOLORD _iconvert_rgb_1555_8888[];
extern ICOLORD _iconvert_rgb_4444_8888[];

/* global palette array */
extern IRGB _ipaletted[];
extern IRGB*_icolormap;
extern int _icolorconv;

/* 8 bits min/max saturation table */
extern const unsigned char _IMINMAX8[];
extern const unsigned char*_iminmax8;
extern const unsigned char*_iclip256;


/**********************************************************************
 * COLOR READ/WRITE FUNCTION (using default A-R-G-B bits order)
 **********************************************************************/
#define _im_getr8(c) (_ipaletted[c].r)
#define _im_getg8(c) (_ipaletted[c].g)
#define _im_getb8(c) (_ipaletted[c].b)
#define _im_geta8(c) 255

#define _im_getr15(c) (_iscale_rgb_5[((c) >> 10) & 0x1F])
#define _im_getg15(c) (_iscale_rgb_5[((c) >> 5) & 0x1F])
#define _im_getb15(c) (_iscale_rgb_5[((c) >> 0) & 0x1F])
#define _im_geta15(c) 255

#define _im_getr16(c) (_iscale_rgb_5[((c) >> 11) & 0x1F])
#define _im_getg16(c) (_iscale_rgb_6[((c) >> 5) & 0x3F])
#define _im_getb16(c) (_iscale_rgb_5[((c) >> 0) & 0x1F])
#define _im_geta16(c) 255

#define _im_getr24(c) ( ((c) >> 16) & 0xFF )
#define _im_getg24(c) ( ((c) >> 8) & 0xFF )
#define _im_getb24(c) ( ((c) >> 0) & 0xFF )
#define _im_geta24(c) 255

#define _im_getr32(c) ( ((c) >> 16) & 0xFF )
#define _im_getg32(c) ( ((c) >> 8) & 0xFF )
#define _im_getb32(c) ( ((c) >> 0) & 0xFF )
#define _im_geta32(c) ( ((c) >> 24) & 0xFF )

#define _im_getr4444(c) (_iscale_rgb_4[((c) >> 8) & 15])
#define _im_getg4444(c) (_iscale_rgb_4[((c) >> 4) & 15])
#define _im_getb4444(c) (_iscale_rgb_4[((c) >> 0) & 15])
#define _im_geta4444(c) (_iscale_rgb_4[((c) >> 12) & 15])

#define _im_getr1555(c) (_iscale_rgb_5[((c) >> 10) & 31])
#define _im_getg1555(c) (_iscale_rgb_5[((c) >> 5) & 31])
#define _im_getb1555(c) (_iscale_rgb_5[((c) >> 0) & 31])
#define _im_geta1555(c) (_iscale_rgb_1[((c) >> 15) &  1])

#define _im_getr5551(c) (_iscale_rgb_5[((c) >> 11) & 31])
#define _im_getg5551(c) (_iscale_rgb_5[((c) >>  6) & 31])
#define _im_getb5551(c) (_iscale_rgb_5[((c) >>  1) & 31])
#define _im_geta5551(c) (_iscale_rgb_1[((c) >>  0) &  1])


/* approximate color fetching */
#define _im_getir8(c) _im_getr8(c)
#define _im_getig8(c) _im_getg8(c)
#define _im_getib8(c) _im_getb8(c)
#define _im_getia8(c) 255

#if 1
/* however switching the '#if' abrove to 1 seems faster then 0 in GCC -O3 */
#define _im_getir15(c) ( (((c) >> 10) & 0x1F) << 3 )
#define _im_getig15(c) ( (((c) >> 5) & 0x1F) << 3 )
#define _im_getib15(c) ( (((c) >> 0) & 0x1F) << 3 )
#define _im_getia15(c) 255

#define _im_getir16(c) ( (((c) >> 11) & 0x1F) << 3 )
#define _im_getig16(c) ( (((c) >> 5) & 0x3F) << 2 )
#define _im_getib16(c) ( (((c) >> 0) & 0x1F) << 3 )
#define _im_getia16(c) 255

#else
/* strange why the implementation below is slower than abrove in GCC -O3 */
#define _im_getir15(c) ( (((c) >> 7) & 0xf8) )
#define _im_getig15(c) ( (((c) >> 2) & 0xf8) )
#define _im_getib15(c) ( (((c) << 3) & 0xf8) )
#define _im_getia15(c) 255

#define _im_getir16(c) ( (((c) >> 8) & 0xf8) )
#define _im_getig16(c) ( (((c) >> 3) & 0xfc) )
#define _im_getib16(c) ( (((c) << 3) & 0xf8) )
#define _im_getia16(c) 255

#endif

#define _im_getir24(c) _im_getr24(c)
#define _im_getig24(c) _im_getg24(c)
#define _im_getib24(c) _im_getb24(c)
#define _im_getia24(c) 255

#define _im_getir32(c) _im_getr32(c)
#define _im_getig32(c) _im_getg32(c)
#define _im_getib32(c) _im_getb32(c)
#define _im_getia32(c) _im_geta32(c)

#define _im_getia1555(c) ( (((c) >> 15) & 0x1) << 7)
#define _im_getir1555(c) ( (((c) >> 10) & 0x1F) << 3 )
#define _im_getig1555(c) ( (((c) >> 5) & 0x1F) << 3 )
#define _im_getib1555(c) ( (((c) >> 0) & 0x1F) << 3 )

#define _im_getia5551(c) _im_geta5551(c)
#define _im_getir5551(c) ( (((c) >> 11) & 0x1f) << 3 )
#define _im_getig5551(c) ( (((c) >>  6) & 0x1f) << 3 )
#define _im_getib5551(c) ( (((c) >>  1) & 0x1f) << 3 )

#define _im_getia4444(c) ( (((c) >>12) & 0xf) << 4 )
#define _im_getir4444(c) ( (((c) >> 8) & 0xf) << 4 )
#define _im_getig4444(c) ( (((c) >> 4) & 0xf) << 4 )
#define _im_getib4444(c) ( (((c) >> 0) & 0xf) << 4 )

#define _im_getia8888(c) _im_getia32(c)
#define _im_getir8888(c) _im_getir32(c)
#define _im_getig8888(c) _im_getig32(c)
#define _im_getib8888(c) _im_getib32(c)
#define _im_getia555(c) _im_getia15(c)
#define _im_getir555(c) _im_getir15(c)
#define _im_getig555(c) _im_getig15(c)
#define _im_getib555(c) _im_getib15(c)
#define _im_getia565(c) _im_getia16(c)
#define _im_getir565(c) _im_getir16(c)
#define _im_getig565(c) _im_getig16(c)
#define _im_getib565(c) _im_getib16(c)
#define _im_getia888(c) _im_getia24(c)
#define _im_getir888(c) _im_getir24(c)
#define _im_getig888(c) _im_getig24(c)
#define _im_getib888(c) _im_getib24(c)

#define _im_unpack_rgb(col, bpp, r, g, b) do { \
		(r) = _im_getir##bpp(col); \
		(g) = _im_getig##bpp(col); \
		(b) = _im_getib##bpp(col); \
	}	while (0)

#define _im_unpack_argb(col, bpp, a, r, g, b) do { \
		(a) = _im_getia##bpp(col); \
		(r) = _im_getir##bpp(col); \
		(g) = _im_getig##bpp(col); \
		(b) = _im_getib##bpp(col); \
	}	while (0)


#define _im_scale_rgb(col, bpp, r, g, b) do { \
		(r) = _im_getr##bpp(col); \
		(g) = _im_getg##bpp(col); \
		(b) = _im_getb##bpp(col); \
	}	while (0)

#define _im_scale_bgr(col, bpp, r, g, b) \
		_im_scale_rgb(col, bpp, b, g, r)

#define _im_scale_argb(col, bpp, a, r, g, b) do { \
		(a) = _im_geta##bpp(col); \
		(r) = _im_getr##bpp(col); \
		(g) = _im_getg##bpp(col); \
		(b) = _im_getb##bpp(col); \
	}	while (0)

#define _im_scale_abgr(col, bpp, a, r, g, b) \
		_im_scale_argb(col, bpp, a, b, g, r)

#define _im_scale_rgba(col, bpp, a, r, g, b) \
		_im_scale_argb(col, bpp, r, g, b, a)

#define _im_scale_bgra(col, bpp, a, r, g, b) \
		_im_scale_argb(col, bpp, b, g, r, a)


/* color packing */
#define _im_pack_c8(r, g, b) ((ICOLORB)( \
			((ICOLORB)((r) & 0xe0) << 0) | \
			((ICOLORB)((g) & 0xe0) >> 3) | \
			((ICOLORB)((b) & 0xff) >> 6)))

#define _im_pack_c15(r, g, b) ((ICOLORW)( \
			((ICOLORW)((r) & 0xf8) << 7) | \
			((ICOLORW)((g) & 0xf8) << 2) | \
			((ICOLORW)((b) & 0xf8) >> 3)))

#define _im_pack_c16(r, g, b) ((ICOLORW)( \
			((ICOLORW)((r) & 0xf8) << 8) | \
			((ICOLORW)((g) & 0xfc) << 3) | \
			((ICOLORW)((b) & 0xf8) >> 3)))

#define _im_pack_c24(r, g, b) ((ICOLORD)( \
            ((ICOLORD)(r) << 16) | \
            ((ICOLORD)(g) << 8) | \
            ((ICOLORD)(b) << 0)))

#define _im_pack_c32(r, g, b) ((ICOLORD)( \
            ((ICOLORD)(r) << 16) | \
            ((ICOLORD)(g) << 8) | \
            ((ICOLORD)(b) << 0)))

#define _im_pack_a32(a, r, g, b) ((ICOLORD)( \
            ((ICOLORD)(r) << 16) | \
            ((ICOLORD)(g) << 8) | \
            ((ICOLORD)(b) << 0) | \
            ((ICOLORD)(a) << 24)))

#define _im_pack_a8888(a, r, g, b) \
			_im_pack_a32(a, r, g, b)

#define _im_pack_a1555(a, r, g, b) \
			(_im_pack_c15(r, g, b) | ((((ICOLORW)(a)) & 0x80) << 8))

#define _im_pack_a5551(a, r, g, b) \
			((_im_pack_c15(r, g, b) << 1) | ((a) >> 7))

#define _im_pack_a4444(a, r, g, b)  ((ICOLORW)( \
			((ICOLORW)((r) & 0xf0) << 4) | \
			((ICOLORW)((g) & 0xf0) << 0) | \
			((ICOLORW)((b) & 0xf0) >> 4) | \
			((ICOLORW)((a) & 0xf0) << 8)))

#define _im_pack_rgb(bpp, r, g, b) \
			_im_pack_c##bpp(r, g, b)

#define _im_pack_bgr(bpp, r, g, b) \
			_im_pack_rgb(bpp, b, g, r)

#define _im_pack_argb(bpp, a, r, g, b) \
			_im_pack_a##bpp(a, r, g, b)

#define _im_pack_abgr(bpp, a, r, g, b) \
			_im_pack_argb(bpp, a, b, g, r)

#define _im_pack_rgba(bpp, a, r, g, b) \
			_im_pack_argb(bpp, r, g, b, a)

#define _im_pack_bgra(bpp, a, r, g, b) \
			_im_pack_argb(bpp, b, g, r, a)


#define iRGB(r, g, b) _im_pack_rgb(32, r, g, b)
#define iARGB(a, r, g, b) _im_pack_argb(32, a, r, g, b)


/**********************************************************************
 * BYTES GET/PUT
 **********************************************************************/
#define _im_get1b(p) (((ICOLORB*)(p))[0])
#define _im_get2b(p) (((ICOLORW*)(p))[0])
#define _im_get4b(p) (((ICOLORD*)(p))[0])

#define _im_put1b(p, c) (((ICOLORB*)(p))[0] = (ICOLORB)(c))
#define _im_put2b(p, c) (((ICOLORW*)(p))[0] = (ICOLORW)(c))
#define _im_put4b(p, c) (((ICOLORD*)(p))[0] = (ICOLORD)(c))

#define _im_get3bx(p, b1, b2, b3) ( \
		(((ICOLORD)((ICOLORB*)(p))[0]) << (b1)) | \
		(((ICOLORD)((ICOLORB*)(p))[1]) << (b2)) | \
		(((ICOLORD)((ICOLORB*)(p))[2]) << (b3)) )

#define _im_put3bx(p, c, b1, b2, b3) \
		((ICOLORB*)(p))[0] = (ICOLORB)(((c) >> b1) & 0xFF), \
		((ICOLORB*)(p))[1] = (ICOLORB)(((c) >> b2) & 0xFF), \
		((ICOLORB*)(p))[2] = (ICOLORB)(((c) >> b3) & 0xFF)

#define _im_get3b(p) ( \
        (((ICOLORD)((ICOLORB*)(p))[0]) << _ishift_data_b1) | \
		(((ICOLORD)((ICOLORB*)(p))[1]) << _ishift_data_b2) | \
		(((ICOLORD)((ICOLORB*)(p))[2]) << _ishift_data_b3) )

#define _im_put3b(p, c) do { \
		((ICOLORB*)(p))[0] = (ICOLORB)(((c) >> _ishift_data_b1) & 0xFF); \
		((ICOLORB*)(p))[1] = (ICOLORB)(((c) >> _ishift_data_b2) & 0xFF); \
		((ICOLORB*)(p))[2] = (ICOLORB)(((c) >> _ishift_data_b3) & 0xFF); \
	}	while (0)


#define ICOLORCONV_MASK_TRANS	1
#define ICOLORCONV_KEEP_TRANS	2
#define ICOLORCONV_DITHER_BLIT	4


void _ishift_init(void);
int _igenerate_332_palette(IRGB *pal);



/**********************************************************************
 * PIXEL FORMAT 
 **********************************************************************/
#define IPIX_FMT_NONE	0
#define IPIX_FMT_8		1
#define IPIX_FMT_RGB15	2
#define IPIX_FMT_BGR15	3
#define IPIX_FMT_RGB16	4
#define IPIX_FMT_BGR16	5
#define IPIX_FMT_RGB24	6
#define IPIX_FMT_BGR24	7
#define IPIX_FMT_RGB32	8
#define IPIX_FMT_BGR32	9

#define IPIX_FMT_ARGB32	10
#define IPIX_FMT_ABGR32	11
#define IPIX_FMT_RGBA32	12
#define IPIX_FMT_BGRA32	13

#define IPIX_FMT_ARGB_4444	14
#define IPIX_FMT_ABGR_4444	15
#define IPIX_FMT_RGBA_4444	16
#define IPIX_FMT_BGRA_4444	17

#define IPIX_FMT_ARGB_1555 18
#define IPIX_FMT_ABGR_1555 19
#define IPIX_FMT_RGBA_5551 20
#define IPIX_FMT_BGRA_5551 21


struct IPIXELFMT
{
	int type;				// IPIX_FMT_...
	int bpp;				// pixel depth
	int use_bgr;			// 0: RGB, 1: BGR
	int has_alpha;			// 0: no alpha, 1: alpha
	int alpha_pos;			// 0: AXXX, 1: XXXA
	unsigned long rmask;
	unsigned long gmask;
	unsigned long bmask;
	unsigned long amask;
};

extern const struct IPIXELFMT ipixel_fmt[];

/* set pixel format */
void ibitmap_set_pixfmt(struct IBITMAP *bmp, int pixelfmt);


/* set rgb color in given format */
ICOLORD _im_color_set(int pixfmt, ICOLORD rgb, const IRGB *pal);

/* get raw color from given format */
ICOLORD _im_color_get(int pixfmt, ICOLORD raw, const IRGB *pal);


/**********************************************************************
 * PIXEL GET/PUT
 **********************************************************************/
#define _ipixel_get_1(ptr)   _im_get1b(ptr)
#define _ipixel_get_2(ptr)   _im_get2b(ptr)
#define _ipixel_get_3(ptr)   _im_get3b(ptr)
#define _ipixel_get_4(ptr)   _im_get4b(ptr)
#define _ipixel_get_8(ptr)   _im_get1b(ptr)
#define _ipixel_get_15(ptr)  _im_get2b(ptr)
#define _ipixel_get_16(ptr)  _im_get2b(ptr)
#define _ipixel_get_24(ptr)  _im_get3b(ptr)
#define _ipixel_get_32(ptr)  _im_get4b(ptr)

#define _ipixel_put_1(ptr, col)  _im_put1b(ptr, col)
#define _ipixel_put_2(ptr, col)  _im_put2b(ptr, col)
#define _ipixel_put_3(ptr, col)  _im_put3b(ptr, col)
#define _ipixel_put_4(ptr, col)  _im_put4b(ptr, col)
#define _ipixel_put_8(ptr, col)  _im_put1b(ptr, col)
#define _ipixel_put_15(ptr, col) _im_put2b(ptr, col)
#define _ipixel_put_16(ptr, col) _im_put2b(ptr, col)
#define _ipixel_put_24(ptr, col) _im_put3b(ptr, col)
#define _ipixel_put_32(ptr, col) _im_put4b(ptr, col)

#define _ipixel_get(bpp, ptr)      _ipixel_get_##bpp(ptr)
#define _ipixel_put(bpp, ptr, col) _ipixel_put_##bpp(ptr, col)


/**********************************************************************
 * BITMAP OPERATION MACRO
 **********************************************************************/
typedef struct IBITMAP IBITMAP;

#define IFLAG_USEBGR	1	/* Bitmap - inverse bytes order (b,g,r,a) */
#define IFLAG_MTLOCK	2	/* Bitmap - must lock the bitmap          */
#define IFLAG_HAVEALPHA	4	/* Bitmap - have alpha channel            */
#define IFLAG_ALPHALOW  8   /* Bitmap - alpha in low bit order        */

#define IBCLIP	1			/* BLIT - use area clip			*/
#define IBCKEY	2			/* BLIT - use source color key	*/

#define _ilineptr(bmp, y) ((unsigned char*)((bmp)->line[y]))

#define _iget8(bmp, x, y ) _im_get1b(_ilineptr(bmp, y) + (x))
#define _iget15(bmp, x, y) _im_get2b(_ilineptr(bmp, y) + (x) * 2)
#define _iget16(bmp, x, y) _im_get2b(_ilineptr(bmp, y) + (x) * 2)
#define _iget24(bmp, x, y) _im_get3b(_ilineptr(bmp, y) + (x) * 3)
#define _iget32(bmp, x, y) _im_get4b(_ilineptr(bmp, y) + (x) * 4)

#define _iput8(b, x, y, c ) _im_put1b(_ilineptr(b, y) + (x), c)
#define _iput15(b, x, y, c) _im_put2b(_ilineptr(b, y) + (x) * 2, c)
#define _iput16(b, x, y, c) _im_put2b(_ilineptr(b, y) + (x) * 2, c)
#define _iput24(b, x, y, c) _im_put3b(_ilineptr(b, y) + (x) * 3, c)
#define _iput32(b, x, y, c) _im_put4b(_ilineptr(b, y) + (x) * 4, c)

#define _ibitmap_w(bmp)      ((long)((bmp)->w))
#define _ibitmap_h(bmp)      ((long)((bmp)->h))
#define _ibitmap_bpp(bmp)    ((long)((bmp)->bpp))
#define _ibitmap_pitch(bmp)  ((long)((bmp)->pitch))

#define _ibitmap_mask(bmp)   ((bmp)->mask)
#define _ibitmap_colorkey(bmp) _ibitmap_mask(bmp)
#define _ibitmap_npixelbytes(bmp)  ((_ibitmap_bpp(bmp) + 7) >> 3)

#define _ibitmap_mode(b) ((b)->mode)
#define _ibitmap_code(b) ((b)->code)

#define _ibitmap_flags(b, flag) (_ibitmap_mode(b) & flag)
#define _ibitmap_flags_chk(b, flag) _ibitmap_flags(b, flag)

#define _ibitmap_flags_set(b, flag) \
	_ibitmap_mode(b) = (_ibitmap_mode(b) | flag)
#define _ibitmap_flags_clr(b, flag) \
	_ibitmap_mode(b) = (_ibitmap_mode(b) & (~((ICOLORD)flag)))

#define _ibitmap_isbgr(b)    _ibitmap_flags(b, IFLAG_USEBGR)
#define _ibitmap_mustlock(b) _ibitmap_flags(b, IFLAG_MUSTLOCK)

/* set & get byte order */
#define _ibitmap_order(b) (_ibitmap_mode(b) >> 16)
#define _ibitmap_set_order(b, order) do { \
		_ibitmap_mode(b) = (_ibitmap_mode(b) & 0xffff) | \
		(((order) & 0x7fff) << 16); \
	}	while (0)

/* set & get color fmt */
#define _ibitmap_pixfmt(b) ((_ibitmap_mode(b) >> 8) & 0xff)
#define _ibitmap_set_pixfmt(b, fmt) do { \
		_ibitmap_mode(b) = (_ibitmap_mode(b) & (~0xff00ul)) | \
		(((fmt) & 0xff) << 8); \
	}	while (0)

/* set & get class */
#define _ibitmap_class(b) (_ibitmap_mode(b) & 0xff)
#define _ibitmap_set_class(b, c) do { \
		_ibitmap_mode(b) = (_ibitmap_mode(b) & (~0xfful)) | \
		((c & 0xff)); \
	}	while (0)	




/**********************************************************************
 * LIN's LOOP UNROLL MACROs: 
 * Actually Duff's unroll macro isn't compatible with vc7 because
 * of non-standard usage of 'switch' & 'for' statement. 
 * the macros below are standard implementation of loop unroll
 **********************************************************************/
#define ILINS_LOOP_UNROLL(action, width) do { \
	unsigned long __width = (unsigned long)(width); \
	unsigned long __increment = __width >> 3; \
	for (; __increment > 0; __increment--) { \
		action; action; action; action; \
		action; action; action; action; \
	}	\
	for (__width = __width & 7; __width > 0; __width--) { action; } \
}	while (0)

#define ILINS_LOOP_DOUBLE(actionx1, actionx2, width) do { \
	unsigned long __width = (unsigned long)(width); \
	unsigned long __increment = __width >> 2; \
	for (; __increment > 0; __increment--) { actionx2; actionx2; } \
	if (__width & 2) { actionx2; } \
	if (__width & 1) { actionx1; }  \
}	while (0)

#define ILINS_LOOP_QUATRO(actionx1, actionx2, actionx4, width) do { \
	unsigned long __width = (unsigned long)(width);	\
	unsigned long __increment = __width >> 2; \
	for (; __increment > 0; __increment--) { actionx4; }	\
	if (__width & 2) { actionx2; } \
	if (__width & 1) { actionx1; } \
}	while (0)

#define ILINS_LOOP_ONCE(action, width) do { \
	unsigned long __width = (unsigned long)(width); \
	for (; __width > 0; __width--) { action; } \
}	while (0)



/**********************************************************************
 * PIXEL ASSEMBLEY
 **********************************************************************/
#define IRGBA_FROM_8(c, r, g, b, a) _im_scale_argb(c, 8, a, r, g, b)

#define IRGBA_FROM_RGB15(c, r, g, b, a) _im_scale_argb(c, 15, a, r, g, b)
#define IRGBA_FROM_BGR15(c, r, g, b, a) _im_scale_argb(c, 15, a, b, g, r)
#define IRGBA_FROM_RGB16(c, r, g, b, a) _im_scale_argb(c, 16, a, r, g, b)
#define IRGBA_FROM_BGR16(c, r, g, b, a) _im_scale_argb(c, 16, a, b, g, r)
#define IRGBA_FROM_RGB24(c, r, g, b, a) _im_scale_argb(c, 24, a, r, g, b)
#define IRGBA_FROM_BGR24(c, r, g, b, a) _im_scale_argb(c, 24, a, b, g, r)

#define IRGBA_FROM_RGB32(c, r, g, b, a) { \
	_im_scale_rgb(c, 32, r, g, b); a = 255; }
#define IRGBA_FROM_BGR32(c, r, g, b, a) { \
	_im_scale_rgb(c, 32, b, g, r); a = 255; }

#define IRGBA_FROM_ARGB32(c, r, g, b, a) _im_scale_argb(c, 32, a, r, g, b)
#define IRGBA_FROM_ABGR32(c, r, g, b, a) _im_scale_argb(c, 32, a, b, g, r)
#define IRGBA_FROM_RGBA32(c, r, g, b, a) _im_scale_argb(c, 32, r, g, b, a)
#define IRGBA_FROM_BGRA32(c, r, g, b, a) _im_scale_argb(c, 32, b, g, r, a)

#define IRGBA_FROM_ARGB_4444(c, r, g, b, a) \
	_im_scale_argb(c, 4444, a, r, g, b)

#define IRGBA_FROM_ABGR_4444(c, r, g, b, a) \
	_im_scale_argb(c, 4444, a, b, g, r)

#define IRGBA_FROM_RGBA_4444(c, r, g, b, a) \
	_im_scale_argb(c, 4444, r, g, b, a)

#define IRGBA_FROM_BGRA_4444(c, r, g, b, a) \
	_im_scale_argb(c, 4444, b, g, r, a)

#define IRGBA_FROM_ARGB_1555(c, r, g, b, a) \
	_im_scale_argb(c, 1555, a, r, g, b)

#define IRGBA_FROM_ABGR_1555(c, r, g, b, a) \
	_im_scale_argb(c, 1555, a, b, g, r)

#define IRGBA_FROM_RGBA_5551(c, r, g, b, a) \
	_im_scale_argb(c, 5551, a, r, g, b)

#define IRGBA_FROM_BGRA_5551(c, r, g, b, a) \
	_im_scale_argb(c, 5551, a, b, g, r)


#define IRGB_FROM_8(c, r, g, b) _im_scale_rgb(c, 8, r, g, b)
#define IRGB_FROM_RGB15(c, r, g, b) _im_scale_rgb(c, 15, r, g, b)
#define IRGB_FROM_BGR15(c, r, g, b) _im_scale_rgb(c, 15, b, g, r)
#define IRGB_FROM_RGB16(c, r, g, b) _im_scale_rgb(c, 16, r, g, b)
#define IRGB_FROM_BGR16(c, r, g, b) _im_scale_rgb(c, 16, b, g, r)
#define IRGB_FROM_RGB24(c, r, g, b) _im_scale_rgb(c, 24, r, g, b)
#define IRGB_FROM_BGR24(c, r, g, b) _im_scale_rgb(c, 24, b, g, r)
#define IRGB_FROM_RGB32(c, r, g, b) _im_scale_rgb(c, 32, r, g, b)
#define IRGB_FROM_BGR32(c, r, g, b) _im_scale_rgb(c, 32, b, g, r)
#define IRGB_FROM_ARGB32(c, r, g, b) _im_scale_rgb(c, 32, r, g, b)
#define IRGB_FROM_ABGR32(c, r, g, b) _im_scale_rgb(c, 32, b, g, r)
#define IRGB_FROM_RGBA32(c, r, g, b) { \
		r = _im_geta32(c); g = _im_getr32(c); b = _im_getg32(c); } 
#define IRGB_FROM_BGRA32(c, r, g, b) IRGB_FROM_RGBA32(c, b, g, r)

#define IRGB_FROM_ARGB_4444(c, r, g, b) _im_scale_rgb(c, 4444, r, g, b)
#define IRGB_FROM_ABGR_4444(c, r, g, b) _im_scale_rgb(c, 4444, r, g, b)
#define IRGB_FROM_RGBA_4444(c, r, g, b) { \
		r = _im_geta4444(c); g = _im_getr4444(c); b = _im_getg4444(c); }
#define IRGB_FROM_BGRA_4444(c, r, g, b) IRGB_FROM_RGBA_4444(c, b, g, r)

#define IRGB_FROM_ARGB_1555(c, r, g, b) _im_scale_rgb(c, 1555, r, g, b)
#define IRGB_FROM_ABGR_1555(c, r, g, b) _im_scale_rgb(c, 1555, b, g, r)
#define IRGB_FROM_RGBA_5551(c, r, g, b) _im_scale_rgb(c, 5551, r, g, b)
#define IRGB_FROM_BGRA_5551(c, r, g, b) _im_scale_rgb(c, 5551, b, g, r)

#define IRGBA_TO_8(r, g, b, a) _im_pack_rgb(8, r, g, b)
#define IRGBA_TO_RGB15(r, g, b, a) _im_pack_rgb(15, r, g, b)
#define IRGBA_TO_BGR15(r, g, b, a) _im_pack_rgb(15, b, g, r)
#define IRGBA_TO_RGB16(r, g, b, a) _im_pack_rgb(16, r, g, b)
#define IRGBA_TO_BGR16(r, g, b, a) _im_pack_rgb(16, b, g, r)
#define IRGBA_TO_RGB24(r, g, b, a) _im_pack_rgb(24, r, g, b)
#define IRGBA_TO_BGR24(r, g, b, a) _im_pack_rgb(24, b, g, r)
#define IRGBA_TO_RGB32(r, g, b, a) _im_pack_rgb(32, r, g, b)
#define IRGBA_TO_BGR32(r, g, b, a) _im_pack_rgb(32, b, g, r)
#define IRGBA_TO_ARGB32(r, g, b, a) _im_pack_argb(32, a, r, g, b)
#define IRGBA_TO_ABGR32(r, g, b, a) _im_pack_argb(32, a, b, g, r)
#define IRGBA_TO_RGBA32(r, g, b, a) _im_pack_argb(32, r, g, b, a)
#define IRGBA_TO_BGRA32(r, g, b, a) _im_pack_argb(32, b, g, r, a)
#define IRGBA_TO_ARGB_4444(r, g, b, a) _im_pack_argb(4444, a, r, g, b)
#define IRGBA_TO_ABGR_4444(r, g, b, a) _im_pack_argb(4444, a, b, g, r)
#define IRGBA_TO_RGBA_4444(r, g, b, a) _im_pack_argb(4444, r, g, b, a)
#define IRGBA_TO_BGRA_4444(r, g, b, a) _im_pack_argb(4444, b, g, r, a)
#define IRGBA_TO_ARGB_1555(r, g, b, a) _im_pack_argb(1555, a, r, g, b)
#define IRGBA_TO_ABGR_1555(r, g, b, a) _im_pack_argb(1555, a, b, g, r)
#define IRGBA_TO_RGBA_5551(r, g, b, a) _im_pack_argb(5551, a, r, g, b)
#define IRGBA_TO_BGRA_5551(r, g, b, a) _im_pack_argb(5551, a, b, g, r)


#define IRGBA_FROM_PIXEL(c, fmt, r, g, b, a) IRGBA_FROM_##fmt(c, r, g, b, a)
#define IRGB_FROM_PIXEL(c, fmt, r, g, b) IRGB_FROM_##fmt(c, r, g, b)

#define IRGBA_TO_PIXEL(fmt, r, g, b, a) IRGBA_TO_##fmt(r, g, b, a)
#define IRGB_TO_PIXEL(fmt, r, g, b) IRGBA_TO_PIXEL(fmt, r, g, b, 255)


/**********************************************************************
 * PLATFORM DEPEDENCE
 **********************************************************************/
#if (defined(__BORLANDC__) || defined(__WATCOMC__))
#if (defined(__BORLANDC__) && (!defined(__MSDOS__)))
#pragma warn -8002  
#pragma warn -8004  
#pragma warn -8008  
#pragma warn -8012
#pragma warn -8027
#pragma warn -8057  
#pragma warn -8066
#elif !defined(__BORLANDC__)
#pragma warn -533
#endif
#endif

#if (defined(BUILD_TINY) || defined(__MSDOS__))
#ifndef IDISABLE_CONVERT
#define IDISABLE_CONVERT
#endif
#ifndef IDISABLE_BLEND
#define IDISABLE_BLEND
#endif
#endif



/**********************************************************************
 * PALETTE OPERATION
 **********************************************************************/

/* find best fit color */
int _ibestfit_color(const IRGB *pal, int r, int g, int b, int palsize);

/* generate bestfit color lookup table 
 *     nbits = 4     format = R4G4B4, tabsize = 4096       (2 ^ 12)
 *     nbits = 5     format = R5G5B5, tabsize = 32768      (2 ^ 15)
 *     nbits = 6     format = R6G6B6, tabsize = 262144     (2 ^ 18)
 *     nbits = 7     format = R7G7B7, tabsize = 2097152    (2 ^ 21)
 *     nbits = 8     format = R8G8B8, tabsize = 16777216   (2 ^ 24)
 */
int _ibestfit_table(const IRGB *pal, int len, unsigned char *out, int nbits);


/**********************************************************************
 * BITMAP COLOR CONVERTION
 **********************************************************************/

/* skipping src-mask (src colorkey) when performancing converting,
   enable this flag will using color-shifting method (no matter
   wheather ICONV_SHIFT is set */
#define ICONV_MASK         IBLIT_MASK

/* ICONV_RGB2BGR / ICONV_BGR2RGB is needed when converting needs to swap 
   color order from RGB to BGR or from BGR to RGB. For example, when you 
   want to convert R8G8B8 to B8G8R8, R5B5G5 to B8G8R8, just enable it */
#define ICONV_RGB2BGR      32
#define ICONV_BGR2RGB      ICONV_RGB2BGR

/* when converting lowwer color bits to higher bits, 16bits -> 24bits etc,
   there are two way to do it: lookup-table (default) & bits-shifting
   lookup-table will not cause color losing and fast in most platform,
   bits-shifting will cause color losing, enable ICONV_SHIFT to use it */
#define ICONV_SHIFT        64

#define ICONV_ALPHALOW     128

/* don't call extra converter */
#define ICONV_DEFAULT      256

#define ICONV_CLIP         IBLIT_CLIP
#define ICONV_HFLIP        IBLIT_HFLIP
#define ICONV_VFLIP        IBLIT_VFLIP


/* default converter which can convert all pixel formats (but slow) */
void _iconvert_pixfmt(IBITMAP *dst, int dx, int dy, IBITMAP *src, int x, 
	int y, int w, int h, const IRGB *dpal, const IRGB *spal, int flag);


/**********************************************************************
 * CONVERT BLIT
 **********************************************************************/

/**
 * _iconvert_blit:
 *
 * color format converting routine, supports endian indepedance, 
 * mask color converting, lookup-table color scaling and color order
 * swapping. EACH converting is compiled into DIFFERENT-C-FUNCTION
 * to get the high performance.
 * 
 * parameters:
 * dst, dx, dy - destination bitmap and position
 * src, sx, sy - source bitmap and position
 * w, h        - size
 * pal         - palette (can be used when srcbpp/dstbpp is 8 bits
 * flags       - enable ICONV_RGB2BGR or ICONV_SHIFT or ICONV_MASK
*/
void _iconvert_blit(IBITMAP *dst, int dx, int dy, IBITMAP *src,
	int sx, int sy, int w, int h, const IRGB *dpal, const IRGB *spal, 
	int flags);


/* converter interface */
typedef int (*ICONVERTER)(unsigned char *dst, long dpitch, int dfmt,
	const unsigned char *src, long spitch, int sfmt, int w, int h, 
	ICOLORD mask, int flags);

/* converter list: external converter [dst][src] */
extern ICONVERTER _iconverter[24][24];



#ifndef IDISABLE_CONVERT

void _iconvert_argb(IBITMAP *dst, int dx, int dy, IBITMAP *src,
	int sx, int sy, int w, int h, IRGB *pal, int ismask);

#endif


/**********************************************************************
 * BITMAP BLIT EFFECT
 **********************************************************************/

/* filling driver */
typedef int (*IBLIT_FILL_PROC)(unsigned char *ptr, long pitch, int w, int h, ICOLORD color);

/* blending driver: returns zero for successful non-zero for failed */
typedef int (*IBLIT_BLEND_PROC)(unsigned char *dst, long dpitch, 
	const unsigned char *src, long spitch, int w, int h, ICOLORD color,
	int dfmt, int sfmt, ICOLORD mask, int flags);

/* external fill proc by pixfmt */
extern IBLIT_FILL_PROC _iblit_fill_proc[];

/* external blend proc by pixfmt [dst][src] */
extern IBLIT_BLEND_PROC _iblit_blend_proc[24][24];

void _iblit_fastfill(unsigned char *ptr, long pitch, int w, int h, 
	ICOLORD color, int fmt);

void _iblit_fill(IBITMAP *dst, int dx, int dy, int w, int h, ICOLORD col);

int _iblit_alpha(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h, ICOLORD color, int flags);


/**********************************************************************
 * COLOR SHIFTING
 **********************************************************************/


/**********************************************************************
 * COLOR Macros
 **********************************************************************/
#define IRGBA_DISEMBLE(c, fmt, r, g, b, a) do { \
	switch(fmt) { \
	case IPIX_FMT_8: IRGBA_FROM_8(c, r, g, b, a); break; \
	case IPIX_FMT_RGB15: IRGBA_FROM_RGB15(c, r, g, b, a); break; \
	case IPIX_FMT_BGR15: IRGBA_FROM_BGR15(c, r, g, b, a); break; \
	case IPIX_FMT_RGB16: IRGBA_FROM_RGB16(c, r, g, b, a); break; \
	case IPIX_FMT_BGR16: IRGBA_FROM_BGR16(c, r, g, b, a); break; \
	case IPIX_FMT_RGB24: IRGBA_FROM_RGB24(c, r, g, b, a); break; \
	case IPIX_FMT_BGR24: IRGBA_FROM_BGR24(c, r, g, b, a); break; \
	case IPIX_FMT_RGB32: IRGBA_FROM_RGB32(c, r, g, b, a); break; \
	case IPIX_FMT_BGR32: IRGBA_FROM_BGR32(c, r, g, b, a); break; \
	case IPIX_FMT_ARGB32: IRGBA_FROM_ARGB32(c, r, g, b, a); break; \
	case IPIX_FMT_ABGR32: IRGBA_FROM_ABGR32(c, r, g, b, a); break; \
	case IPIX_FMT_RGBA32: IRGBA_FROM_RGBA32(c, r, g, b, a); break; \
	case IPIX_FMT_BGRA32: IRGBA_FROM_BGRA32(c, r, g, b, a); break; \
	case IPIX_FMT_ARGB_4444: IRGBA_FROM_ARGB_4444(c, r, g, b, a); break; \
	case IPIX_FMT_ABGR_4444: IRGBA_FROM_ABGR_4444(c, r, g, b, a); break; \
	case IPIX_FMT_RGBA_4444: IRGBA_FROM_RGBA_4444(c, r, g, b, a); break; \
	case IPIX_FMT_BGRA_4444: IRGBA_FROM_BGRA_4444(c, r, g, b, a); break; \
	case IPIX_FMT_ARGB_1555: IRGBA_FROM_ARGB_1555(c, r, g, b, a); break; \
	case IPIX_FMT_ABGR_1555: IRGBA_FROM_ABGR_1555(c, r, g, b, a); break; \
	case IPIX_FMT_RGBA_5551: IRGBA_FROM_RGBA_5551(c, r, g, b, a); break; \
	case IPIX_FMT_BGRA_5551: IRGBA_FROM_BGRA_5551(c, r, g, b, a); break; \
	default: a = r = g = b = 0; break; \
	} \
}	while (0)

#define IRGBA_ASSEMBLE(c, fmt, r, g, b, a) do { \
	switch (fmt) { \
	case IPIX_FMT_8: c = IRGBA_TO_8(r, g, b, a); break; \
	case IPIX_FMT_RGB15: c = IRGBA_TO_RGB15(r, g, b, a); break; \
	case IPIX_FMT_BGR15: c = IRGBA_TO_BGR15(r, g, b, a); break; \
	case IPIX_FMT_RGB16: c = IRGBA_TO_RGB16(r, g, b, a); break; \
	case IPIX_FMT_BGR16: c = IRGBA_TO_BGR16(r, g, b, a); break; \
	case IPIX_FMT_RGB24: c = IRGBA_TO_RGB24(r, g, b, a); break; \
	case IPIX_FMT_BGR24: c = IRGBA_TO_BGR24(r, g, b, a); break; \
	case IPIX_FMT_RGB32: c = IRGBA_TO_RGB32(r, g, b, a); break; \
	case IPIX_FMT_BGR32: c = IRGBA_TO_BGR32(r, g, b, a); break; \
	case IPIX_FMT_ARGB32: c = IRGBA_TO_ARGB32(r, g, b, a); break; \
	case IPIX_FMT_ABGR32: c = IRGBA_TO_ABGR32(r, g, b, a); break; \
	case IPIX_FMT_RGBA32: c = IRGBA_TO_RGBA32(r, g, b, a); break; \
	case IPIX_FMT_BGRA32: c = IRGBA_TO_BGRA32(r, g, b, a); break; \
	case IPIX_FMT_ARGB_4444: c = IRGBA_TO_ARGB_4444(r, g, b, a); break; \
	case IPIX_FMT_ABGR_4444: c = IRGBA_TO_ABGR_4444(r, g, b, a); break; \
	case IPIX_FMT_RGBA_4444: c = IRGBA_TO_RGBA_4444(r, g, b, a); break; \
	case IPIX_FMT_BGRA_4444: c = IRGBA_TO_BGRA_4444(r, g, b, a); break; \
	case IPIX_FMT_ARGB_1555: c = IRGBA_TO_ARGB_1555(r, g, b, a); break; \
	case IPIX_FMT_ABGR_1555: c = IRGBA_TO_ABGR_1555(r, g, b, a); break; \
	case IPIX_FMT_RGBA_5551: c = IRGBA_TO_RGBA_5551(r, g, b, a); break; \
	case IPIX_FMT_BGRA_5551: c = IRGBA_TO_BGRA_5551(r, g, b, a); break; \
	default: c = 0; break; \
	}	\
}	while (0)

#define IBPP_TO_BYTES(bpp) (((bpp) + 7) >> 3)

#define IPIXEL_FROM_PTR(pixel, fmt, ptr) do { \
	int dsize = IBPP_TO_BYTES(ipixel_fmt[fmt].bpp); \
	pixel = 0; \
	switch (dsize) { \
	case 1: pixel = _ipixel_get(1, ptr); break; \
	case 2: pixel = _ipixel_get(2, ptr); break; \
	case 3: pixel = _ipixel_get(3, ptr); break; \
	case 4: pixel = _ipixel_get(4, ptr); break; \
	} \
}	while (0)

#define IPIXEL_TO_PTR(pixel, fmt, ptr) do { \
	int dsize = IBPP_TO_BYTES(ipixel_fmt[fmt].bpp); \
	switch (dsize) { \
	case 1: _ipixel_put(1, ptr, pixel); break; \
	case 2: _ipixel_put(2, ptr, pixel); break; \
	case 3: _ipixel_put(3, ptr, pixel); break; \
	case 4: _ipixel_put(4, ptr, pixel); break; \
	}	\
}	while (0)

#define IRGBA_FROM_PTR(ptr, fmt, r, g, b, a) do { \
	ICOLORD raw = 0; \
	IPIXEL_FROM_PTR(raw, fmt, ptr); \
	IRGBA_DISEMBLE(raw, fmt, r, g, b, a); \
}	while (0)

#define IRGBA_TO_PTR(ptr, fmt, r, g, b, a) do { \
	int dsize = IBPP_TO_BYTES(ipixel_fmt[fmt].bpp); \
	ICOLORD raw = 0; \
	IRGBA_ASSEMBLE(raw, fmt, r, g, b, a); \
	IPIXEL_TO_PTR(raw, fmt, ptr); \
}	while (0)


#define IPIX_SIZE_8				1
#define IPIX_SIZE_RGB15			2
#define IPIX_SIZE_BGR15			2
#define IPIX_SIZE_RGB16			2
#define IPIX_SIZE_BGR16			2
#define IPIX_SIZE_RGB24			3
#define IPIX_SIZE_BGR24			3
#define IPIX_SIZE_RGB32			4
#define IPIX_SIZE_BGR32			4
#define IPIX_SIZE_ARGB32		4
#define IPIX_SIZE_ABGR32		4
#define IPIX_SIZE_RGBA32		4
#define IPIX_SIZE_BGRA32		4
#define IPIX_SIZE_ARGB_4444		2
#define IPIX_SIZE_ABGR_4444		2
#define IPIX_SIZE_RGBA_4444		2
#define IPIX_SIZE_BGRA_4444		2
#define IPIX_SIZE_ARGB_1555		2
#define IPIX_SIZE_ABGR_1555		2
#define IPIX_SIZE_RGBA_5551		2
#define IPIX_SIZE_BGRA_5551		2

#define IPIX_READ_8(ptr)			_ipixel_get(1, ptr)
#define IPIX_READ_RGB15(ptr)		_ipixel_get(2, ptr)
#define IPIX_READ_BGR15(ptr)		_ipixel_get(2, ptr)
#define IPIX_READ_RGB16(ptr)		_ipixel_get(2, ptr)
#define IPIX_READ_BGR16(ptr)		_ipixel_get(2, ptr)
#define IPIX_READ_RGB24(ptr)		_ipixel_get(3, ptr)
#define IPIX_READ_BGR24(ptr)		_ipixel_get(3, ptr)
#define IPIX_READ_RGB32(ptr)		_ipixel_get(4, ptr)
#define IPIX_READ_BGR32(ptr)		_ipixel_get(4, ptr)
#define IPIX_READ_ARGB32(ptr)		_ipixel_get(4, ptr)
#define IPIX_READ_ABGR32(ptr)		_ipixel_get(4, ptr)
#define IPIX_READ_RGBA32(ptr)		_ipixel_get(4, ptr)
#define IPIX_READ_BGRA32(ptr)		_ipixel_get(4, ptr)
#define IPIX_READ_ARGB_4444(ptr)	_ipixel_get(2, ptr)
#define IPIX_READ_ABGR_4444(ptr)	_ipixel_get(2, ptr)
#define IPIX_READ_RGBA_4444(ptr)	_ipixel_get(2, ptr)
#define IPIX_READ_BGRA_4444(ptr)	_ipixel_get(2, ptr)
#define IPIX_READ_ARGB_1555(ptr)	_ipixel_get(2, ptr)
#define IPIX_READ_ABGR_1555(ptr)	_ipixel_get(2, ptr)
#define IPIX_READ_RGBA_5551(ptr)	_ipixel_get(2, ptr)
#define IPIX_READ_BGRA_5551(ptr)	_ipixel_get(2, ptr)

#define IPIX_WRITE_8(ptr, c)		_ipixel_put(1, ptr, c)
#define IPIX_WRITE_RGB15(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_BGR15(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_RGB16(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_BGR16(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_RGB24(ptr, c)	_ipixel_put(3, ptr, c)
#define IPIX_WRITE_BGR24(ptr, c)	_ipixel_put(3, ptr, c)
#define IPIX_WRITE_RGB32(ptr, c)	_ipixel_put(4, ptr, c)
#define IPIX_WRITE_BGR32(ptr, c)	_ipixel_put(4, ptr, c)
#define IPIX_WRITE_ARGB32(ptr, c)	_ipixel_put(4, ptr, c)
#define IPIX_WRITE_ABGR32(ptr, c)	_ipixel_put(4, ptr, c)
#define IPIX_WRITE_RGBA32(ptr, c)	_ipixel_put(4, ptr, c)
#define IPIX_WRITE_BGRA32(ptr, c)	_ipixel_put(4, ptr, c)
#define IPIX_WRITE_ARGB_4444(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_ABGR_4444(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_RGBA_4444(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_BGRA_4444(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_ARGB_1555(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_ABGR_1555(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_RGBA_5551(ptr, c)	_ipixel_put(2, ptr, c)
#define IPIX_WRITE_BGRA_5551(ptr, c)	_ipixel_put(2, ptr, c)

#define IPIX_FMT_SIZE(fmt) IPIX_SIZE_##fmt
#define IPIX_FMT_READ(fmt, ptr) IPIX_READ_##fmt(ptr)
#define IPIX_FMT_WRITE(fmt, ptr, c) IPIX_WRITE_##fmt(ptr, c)

#define IPIX_FMT_READ_RGBA(fmt, ptr, r, g, b, a) { \
	ICOLORD col = IPIX_FMT_READ(fmt, ptr); \
	IRGBA_FROM_PIXEL(col, fmt, r, g, b, a); \
}


/**********************************************************************
 * BLEND HELP
 **********************************************************************/

// normal blend
#define _iblend_normalize(alpha) (((alpha) << 8) / 255)
#define _iblend_norm_fast(alpha) (((alpha) >> 7) + (alpha))
#define _iblend_unnorm(alpha) ((((alpha) << 8) - (alpha)) >> 8)

// destalpha and srcalpha must be normalized: 
#define _iblend_final_alpha(destalpha, srcalpha) \
	((destalpha) + (((256 - (destalpha)) * (srcalpha)) >> 8))


#define IBLEND_STATIC(sr, sg, sb, sa, dr, dg, db, da) do { \
	int SA = _iblend_norm_fast(sa); \
	dr = (((((int)sr) - ((int)dr)) * SA) >> 8) + dr; \
	dg = (((((int)sg) - ((int)dg)) * SA) >> 8) + dg; \
	db = (((((int)sb) - ((int)db)) * SA) >> 8) + db; \
	da = 255; \
}	while (0)


#define IBLEND_NORMAL(sr, sg, sb, sa, dr, dg, db, da) do { \
	int SA = _iblend_norm_fast(sa); \
	int DA = _iblend_norm_fast(da); \
	int FA = DA + (((256 - DA) * SA) >> 8); \
	SA = (FA != 0)? ((SA << 8) / FA) : (0); \
	da = _iblend_unnorm(FA); \
	dr = (((((int)sr) - ((int)dr)) * SA) >> 8) + dr; \
	dg = (((((int)sg) - ((int)dg)) * SA) >> 8) + dg; \
	db = (((((int)sb) - ((int)db)) * SA) >> 8) + db; \
}	while (0)

#define IBLEND_ADDITIVE(sr, sg, sb, sa, dr, dg, db, da) do { \
	int xa = _iblend_norm_fast(sa); \
	int xr = sr * xa; \
	int xg = sg * xa; \
	int xb = sb * xa; \
	xr = xr >> 8; \
	xg = xg >> 8; \
	xb = xb >> 8; \
	xa = sa + da; \
	xr += dr; \
	xg += dg; \
	xb += db; \
	dr = _iclip256[xr]; \
	dg = _iclip256[xg]; \
	db = _iclip256[xb]; \
	da = _iclip256[xa]; \
}	while (0)



/**********************************************************************
 * COLOR DITHER
 **********************************************************************/
long _iconvert_dither(IBITMAP *dst, int dx, int dy, IBITMAP *src,
	int sx, int sy, int w, int h, IRGB *pal, int flags, int dmode, 
	void *buffer, long bufsize);



#ifdef __cplusplus
}
#endif


#endif



