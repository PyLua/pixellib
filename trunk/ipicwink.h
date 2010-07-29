//=====================================================================
//
// ipicwink.h - picture loading extra module
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================

#ifndef __IPICWINK_H__
#define __IPICWINK_H__

#include "ibitmap.h"
#include "ipicture.h"

#include <stdio.h>
#include <stdlib.h>

//=====================================================================
// Animate Definition
//=====================================================================
struct IANIMATE
{
	int w;				// width of animate
	int h;				// height of animate
	int bpp;			// bpp of animate
	int hotx;			// key point x
	int hoty;			// key point y
	int framecount;		// frame count
	int reserved;		// reserved
	int nrow;			// row count
	int ncol;			// col count
	IRGB *pal;			// palette
	IBITMAP **frames;	// bitmaps 
};

typedef struct IANIMATE iAnimate;


#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------
// memory hook
//---------------------------------------------------------------------
extern void*(*icmalloc)(size_t size);
extern void (*icfree)(void *ptr);


//---------------------------------------------------------------------
// Animate Operation
//---------------------------------------------------------------------
iAnimate *iAnimate_New(int w, int h, int bpp, int count, int hotx, int hoty);

void iAnimate_Free(iAnimate *ani);


//---------------------------------------------------------------------
// Animate Loading 
//---------------------------------------------------------------------

// load gif stream
iAnimate *iAnimate_Load_Gif_Stream(IMDIO *stream);

// load tcp stream 
iAnimate *iAnimate_Load_Tcp_Stream(IMDIO *stream);

// load animate with format detectiing
iAnimate *iAnimate_Load_Stream(IMDIO *stream);

// load animate from file
iAnimate *iAnimate_Load_File(const char *filename, long pos);

// load animate from memory
iAnimate *iAnimate_Load_Memory(const void *ptr, long size);


//---------------------------------------------------------------------
// Animate Saving 
//---------------------------------------------------------------------

void Animate_Save_Tga(iAnimate *ani, const char *file_name_prefix);

#if 0
// save gif stream
void *Animate_Save_Gif_Stream(iAnimate *ani, IMDIO *stream);

// save tcp stream
void *Animate_Save_Tcp_Stream(iAnimate *ani, IMDIO *stream);

#endif




//=====================================================================
// Color Quantization (octree algorithm and median-cut algorithm)
//=====================================================================

// octree color quantization algorithm
// using less memory and cpu with normal quality
// nbits (4 - 8) (the higher value the better quality with higher cpu)
int iquantize_octree(	const IRGB *input, 
						long input_size, 
						IRGB *output,
						long output_size, 
						int nbits);


// median-cut color quantization algorithm
// using more memory and cpu with better quality
// nbits (4 - 8) (the higher value the better quality with higher cpu)
int iquantize_median_cut(	const IRGB *input, 
							long input_size, 
							IRGB *output,
							long output_size, 
							int nbits);


// reduce bitmap color
IBITMAP *ibitmap_reduce (	const IBITMAP *src, 
							const IRGB *palsrc,
							IRGB *palout,
							int palsize,
							int reserved,
							int method,
							int dither);


// calculate two bitmap's error
// the size of two bitmap must equal to each other
double ibitmap_error(	const IBITMAP *src, 
						const IBITMAP *dst,
						const IRGB *palsrc,
						const IRGB *paldst,
						int *red_max_error,
						int *green_max_error,
						int *blue_max_error);



//=====================================================================
// Color Space Convertion
//=====================================================================
typedef struct IYCRCB
{
	unsigned char Y, Cr, Cb;
	unsigned char reserved;
}	IYCrCb;

typedef struct IHSV
{
	float H, S, V;
	char reserved;
}	IHSV;


void iconv_RGB_to_YCrCb(const IRGB *input, long size, IYCrCb *output);

void iconv_YCrCb_to_RGB(const IYCrCb *input, long size, IRGB *output);

void iconv_RGB_to_HSV(const IRGB *input, long size, IHSV *output);

void iconv_HSV_to_RGB(const IHSV *input, long size, IRGB *output);




#ifdef __cplusplus
}
#endif

#endif



