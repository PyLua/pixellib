/**********************************************************************
 *
 * npixia.h - pixia main header file
 *
 * NOTE: arch. independence header,
 * for more information, please see the readme file
 *
 **********************************************************************/

#ifndef __NPIXEL_H__
#define __NPIXEL_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ibitmap.h"
#include "ibmbits.h"
#include "ibmcols.h"
#include "ibmwink.h"
#include "iblit386.h"
#include "ipicture.h"
#include "ikitwin.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(WIN32) && (!defined(_WIN32))
#define _WIN32
#endif

#ifndef __IINT64_DEFINED
#define __IINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 IINT64;
#else
typedef long long IINT64;
#endif
#endif

/**********************************************************************
 * GLOBAL MACRO
 **********************************************************************/


#if (defined(WIN32) || defined(_WIN32))
#define IEXPORTING __declspec (dllexport)
#else
#define IEXPORTING 
#endif

#ifdef __cplusplus
extern "C" {
#endif


int ipicture_save(const char *filename, const IBITMAP *bmp, const IRGB *pal);

//=====================================================================
// SCREEN
//=====================================================================
extern IBITMAP *iscreen;	// 和屏幕色深一样的位图
extern IBITMAP *cscreen;	// 和初始化色深一样的位图

extern int iscreen_mx;			// 鼠标X
extern int iscreen_my;			// 鼠标Y
extern int iscreen_mb;			// 鼠标按钮
extern int iscreen_in;			// 鼠标焦点
extern int iscreen_closing;		// 是否关闭

int iscreen_init(int w, int h, int bpp);

void iscreen_quit(void);

int iscreen_lock(void);

int iscreen_unlock(void);

int iscreen_update(int *rect, int n);

int iscreen_convert(int *rect, int n);

int iscreen_keyon(int keycode);

int iscreen_dispatch(void);

int iscreen_tick(int fps);

float iscreen_fps(void);

IINT64 iscreen_clock(void);

void iscreen_sleep(long millisecond);

void iscreen_caption(const char *text);

void iscreen_vsync(int mode);


//=====================================================================
// MISC
//=====================================================================
void imisc_bitmap_demo(IBITMAP *bmp, int type);

void imisc_bitmap_systext(IBITMAP *dst, int x, int y, const char *string, 
	IUINT32 color, IUINT32 bk, const IRECT *clip, int additive);

#ifdef _WIN32
int CSetDIBitsToDevice(void *hDC, int x, int y, const IBITMAP *bitmap,
	int sx, int sy, int sw, int sh);

int CStretchDIBits(void *hDC, int dx, int dy, int dw, int dh, 
	const IBITMAP *bitmap, int sx, int sy, int sw, int sh);

#endif

#ifdef __cplusplus
}
#endif


#endif


