//=====================================================================
//
// ibmwink.h - 
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================

#ifndef __IBMWINK_H__
#define __IBMWINK_H__

#include "ibitmap.h"
#include "ibitmapm.h"
#include "ibmcols.h"
#include "ibmfast.h"

//---------------------------------------------------------------------
// Rectangle
//---------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------
// Simple Drawing
//---------------------------------------------------------------------

void ibitmap_clip_create(IBITMAP *bmp, int cl, int ct, int cr, int cb);

void ibitmap_clip_delete(IBITMAP *bmp);

void ibitmap_setpixel(IBITMAP *bmp, int x, int y, ICOLORD rgb);

ICOLORD ibitmap_getpixel(const IBITMAP *bmp, int x, int y);

void ibitmap_line(IBITMAP *bmp, int x1, int y1, int x2, int y2, ICOLORD c);

void ibitmap_circle(IBITMAP *bmp, int x, int y, int r, ICOLORD c, int fill);

void ibitmap_rect(IBITMAP *bmp, int x, int y, int w, int h, ICOLORD c);

void ibitmap_box(IBITMAP *bmp, int x, int y, int w, int h, ICOLORD c);


//---------------------------------------------------------------------
// Bitmap Transform
//---------------------------------------------------------------------
#define ISTRETCH_FAST		256

int ibitmap_scale(IBITMAP *dst, int dx, int dy, int dw, int dh,
	const IBITMAP *src, int sx, int sy, int sw, int sh, int flags);


#ifdef __cplusplus
}
#endif

#endif


