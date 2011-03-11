#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ibitmap.h"
#include "ibmbits.h"
#include "ibmcols.h"
#include "iblit386.h"
#include "ipicture.h"
#include "ikitwin.h"
#include "ibmsse2.h"
#ifdef _WIN32
#include "mswindx.h"
#include <windows.h>
#endif

typedef struct { 
	char *name;
	void *func;
}	nexport_list_t;


#define IENTRYLIST(name) { ""#name, name }




//! flag: -O3, -Wall
//! int: obj
//! out: libpixel
//! mode: lib
//! src: ikitwin.c, mswindx.c, ibmfont.c, ibmsse2.c, ibmwink.c
//! src: ibitmap.c, ibmbits.c, iblit386.c, ibmcols.c, ipicture.c, ibmdata.c


//=====================================================================
// SCREEN
//=====================================================================
int iscreen_dx = 1;
int iscreen_bpp = 0;		// ÆÁÄ»µÄÉî¶È
int iscreen_fmt = 0;
int iscreen_inited = 0;
int iscreen_locked = 0;
int iscreen_mx = 0;
int iscreen_my = 0;
int iscreen_mb = 0;
int iscreen_in = 0;
int iscreen_closing = 0;
int iscreen_atexit = 0;

IBITMAP *iscreen = NULL;
IBITMAP *cscreen = NULL;

static iColorIndex *cscreen_index = NULL;
static iColorIndex *iscreen_index = NULL;

#ifdef _WIN32
CSURFACE *dxdesktop = NULL;
CSURFACE *dxscreen = NULL;
#endif


void *align_malloc(size_t size) {
	char *ptr = (char*)malloc(size + 16 + sizeof(char*));
	char *src;
	if (ptr == NULL) return NULL;
	src = ptr + sizeof(char*);
	src = src + ((16 - (((size_t)src) & 15)) & 15);
	*(char**)(src - sizeof(char*)) = ptr;
	return src;
}

void align_free(void *ptr) {
	char *src = (char*)ptr - sizeof(char*);
	free(*(char**)src);
}

extern void*(*icmalloc)(size_t size);
extern void (*icfree)(void *ptr);

void init_allocator()
{
	icmalloc = align_malloc;
	icfree = align_free;
}


IBITMAP *ibitmap_ref(void *ptr, long pitch, int w, int h, int bpp)
{
	IBITMAP *bitmap;
	int fmt;
	if (bpp == 8) fmt = IPIX_FMT_C8;
	else if (bpp == 15) fmt = IPIX_FMT_X1R5G5B5;
	else if (bpp == 16) fmt = IPIX_FMT_R5G6B5;
	else if (bpp == 24) fmt = IPIX_FMT_R8G8B8;
	else fmt = IPIX_FMT_X8R8G8B8;
	bitmap = ibitmap_reference_new(ptr, pitch, w, h, fmt);
	assert(bitmap);
	bitmap->bpp = bpp;
	return bitmap;
}

void ibitmap_unref(IBITMAP *bmp)
{
	ibitmap_reference_del(bmp);
}

void ibitmap_adjust(IBITMAP *bmp, void *ptr, long pitch)
{
	ibitmap_reference_adjust(bmp, ptr, pitch);
}

static void iscreen_atexit_hook(void);


static void iscreen_init_index(iColorIndex *index)
{
	int i;
	index->color = 256;
	for (i = 0; i < 256; i++) {
		index->rgba[i] = IRGBA_TO_A8R8G8B8(i, i, i, 255);
	}
	for (i = 0; i < 32768; i++) {
		int r, g, b, a;
		IRGBA_FROM_X1R5G5B5(i, r, g, b, a);
		a = _ipixel_to_gray(r, g, b);
		index->ent[i] = a;
	}
}

int iscreen_init_main(int w, int h, int bpp)
{
	#ifdef _WIN32
	int fmt = 0;
	iscreen_dx = (bpp < 0)? 0 : 1;
	bpp = (bpp < 0)? (-bpp) : bpp;
	if (iscreen_dx) {
		if (DDrawInit() != 0) iscreen_dx = 0;
	}
	if (ikitwin_init() != 0) return -1;
	if (bpp < 0 && iscreen_dx) {
		iscreen_dx = 0;
		bpp = -bpp;
		DDrawDestroy();
	}
	if (ikitwin_set_mode(w, h, bpp, 0) != 0) return -2;
	if (iscreen_dx) {
		dxdesktop = DDrawCreateScreen(mswin_hwnd, 0, 0, 0, 0);
		if (dxdesktop != NULL) {
			dxscreen = DDrawSurfaceCreate(w, h, PIXFMT_DEFAULT,
				DDSM_SYSTEMMEM, NULL);
			if (dxscreen != NULL) {
				void *bits;
				long pitch;
				DDrawSurfaceInfo(dxscreen, 0, 0, &iscreen_bpp, &iscreen_fmt);
				if (DDrawSurfaceLock(dxscreen, &bits, &pitch, 0) == 0) {
					int fmt;
					iscreen = ibitmap_ref(bits, pitch, w, h, iscreen_bpp);
					fmt = ibitmap_pixfmt_guess(iscreen);
					if (fmt == IPIX_FMT_A8R8G8B8) fmt = IPIX_FMT_X8R8G8B8;
					ibitmap_pixfmt_set(iscreen, fmt);
					DDrawSurfaceUnlock(dxscreen);
					iscreen_locked = 0;
				}	else {
					DDrawSurfaceRelease(dxscreen);
					DDrawSurfaceRelease(dxdesktop);
					dxscreen = NULL;
					dxdesktop = NULL;
					DDrawDestroy();
					iscreen_dx = 0;
				}
			}	else {
				iscreen_dx = 0;
				DDrawSurfaceRelease(dxdesktop);
				dxdesktop = NULL;
				DDrawDestroy();
			}
		}	else {
			iscreen_dx = 0;
			DDrawDestroy();
		}
	}
	if (iscreen_dx == 0) {
		void *bits;
		long pitch;
		int fmt;
		ikitwin_lock(&bits, &pitch);
		iscreen = ibitmap_ref(bits, pitch, w, h, bpp);
		ikitwin_unlock();
		fmt = ibitmap_pixfmt_guess(iscreen);
		if (fmt == IPIX_FMT_A8R8G8B8) fmt = IPIX_FMT_X8R8G8B8;
		ibitmap_pixfmt_set(iscreen, fmt);
		iscreen_bpp = bpp;
	}
	#else
	int rmask, gmask, bmask, i, fmt;
	void *bits;
	long pitch;
	bpp = (bpp < 0)? (-bpp) : bpp;
	if (ikitwin_init() != 0) return -1;
	if (ikitwin_set_mode(w, h, bpp < 0? -bpp : bpp, 0) != 0) return -2;
	iscreen_bpp = ikitwin_depth(&rmask, &gmask, &bmask);
	ikitwin_lock(&bits, &pitch);
	iscreen = ibitmap_ref(bits, pitch, w, h, iscreen_bpp);
	ikitwin_unlock();
	for (fmt = 0, i = 0; i < IPIX_FMT_OTHER; i++) {
		if (ipixel_fmt[i].rmask == rmask &&
			ipixel_fmt[i].gmask == gmask &&
			ipixel_fmt[i].bmask == bmask &&
			ipixel_fmt[i].has_alpha == 0 &&
			ipixel_fmt[i].bpp == iscreen_bpp) {
			fmt = i;
			break;
		}
	}
	ibitmap_set_pixfmt(iscreen, fmt);
	#endif
	cscreen = ibitmap_create(w, h, bpp);
	fmt = ibitmap_pixfmt_guess(cscreen);
	if (fmt == IPIX_FMT_A8R8G8B8) fmt = IPIX_FMT_X8R8G8B8;
	ibitmap_pixfmt_set(cscreen, fmt);

	if (iscreen->bpp == 8) {
		iscreen_index = (iColorIndex*)malloc(sizeof(iColorIndex));
		iscreen->extra = iscreen_index;
		assert(iscreen_index);
		iscreen_init_index(iscreen_index);
	}
	if (cscreen->bpp == 8) {
		cscreen_index = (iColorIndex*)malloc(sizeof(iColorIndex));
		cscreen->extra = cscreen_index;
		assert(cscreen_index);
		iscreen_init_index(cscreen_index);
	}
	if (iscreen_atexit == 0) {
		iscreen_atexit = 1;
		atexit(iscreen_atexit_hook);
	}
	ikitwin_set_caption("ADI-DRAW");
	iscreen_inited = 1;
	return 0;
}

int iscreen_init(int w, int h, int bpp)
{
	init_allocator();
#ifdef __x86__
	_x86_choose_blitter();
	pixellib_mmx_init();
#endif
	return iscreen_init_main(w, h, bpp);
}

void iscreen_quit(void)
{
	if (iscreen_inited == 0) return;
	if (iscreen) ibitmap_unref(iscreen);
	if (cscreen) ibitmap_release(cscreen);
	if (cscreen_index) free(cscreen_index);
	if (iscreen_index) free(iscreen_index);
	iscreen = NULL;
	cscreen = NULL;
	cscreen_index = NULL;
	iscreen_index = NULL;
	#ifndef __unix
	if (iscreen_dx) {
		if (dxscreen) DDrawSurfaceRelease(dxscreen);
		if (dxdesktop) DDrawSurfaceRelease(dxdesktop);
		dxscreen = NULL;
		dxdesktop = NULL;
		DDrawDestroy();
	}
	#endif
	ikitwin_release_mode();
	ikitwin_quit();
	iscreen_inited = 0;
}

static void iscreen_atexit_hook(void)
{
	iscreen_quit();
}


int iscreen_lock(void)
{
	if (iscreen_inited == 0) return -100;
	#ifdef _WIN32
	if (iscreen_dx) {
		void *bits;
		long pitch;
		if (iscreen_locked == 0) {
			if (DDrawSurfaceLock(dxscreen, &bits, &pitch, 0) == 0) {
				ibitmap_adjust(iscreen, bits, pitch);
			}	else {
				return -1;
			}
		}
	}
	#else
	if (iscreen_locked == 0) {
		ikitwin_lock(NULL, NULL);
	}
	#endif
	iscreen_locked++;
	return 0;
}

int iscreen_unlock(void)
{
	if (iscreen_inited == 0) return -100;
	iscreen_locked--;
	#ifdef _WIN32
	if (iscreen_dx) {
		if (iscreen_locked == 0) {
			DDrawSurfaceUnlock(dxscreen);
		}
	}
	#endif
	return 0;
}

int iscreen_update(int *rect, int n)
{
	int fullwindow[4];
	if (iscreen_inited == 0) return -100;
	fullwindow[0] = 0;
	fullwindow[1] = 0;
	fullwindow[2] = iscreen->w;
	fullwindow[3] = iscreen->h;
	if (rect == NULL) rect = fullwindow;
	#ifdef _WIN32
	if (iscreen_dx) {
		int i;
		while (iscreen_locked) iscreen_unlock();
		for (i = 0; i < n; i++) {
			int x = rect[i * 4 + 0];
			int y = rect[i * 4 + 1];
			int w = rect[i * 4 + 2];
			int h = rect[i * 4 + 3];
			DDrawSurfaceBlit(dxdesktop, x, y, w, h, dxscreen, 
				x, y, w, h, DDBLIT_NOWAIT);
		}
		return 0;
	}
	#endif
	ikitwin_update(rect, 1);
	return 0;
}

int iscreen_convert(int *rect, int n)
{
	int fullwindow[4];
	int i;
	if (iscreen_inited == 0) return -100;
	fullwindow[0] = 0;
	fullwindow[1] = 0;
	fullwindow[2] = iscreen->w;
	fullwindow[3] = iscreen->h;
	if (rect == NULL) rect = fullwindow;
	iscreen_lock();
	for (i = 0; i < n; i++, rect += 4) {
		int x = rect[0];
		int y = rect[1];
		int w = rect[2];
		int h = rect[3];
		ibitmap_convert(iscreen, x, y, cscreen, x, y, w, h, NULL, 0);
	}
	iscreen_unlock();
	return 0;
}

int iscreen_keyon(int keycode)
{
	if (iscreen_inited == 0) return 0;
	return ikitwin_keymap[keycode & 511];
}

int iscreen_dispatch(void)
{
	int retval;
	retval = ikitwin_dispatch();
	iscreen_mx = ikitwin_mouse_x;
	iscreen_my = ikitwin_mouse_y;
	iscreen_mb = ikitwin_mouse_btn;
	iscreen_in = ikitwin_mouse_in;
	iscreen_closing = ikitwin_closing;
	return retval;
}

int iscreen_tick(int fps)
{
	return ikitwin_tick(fps);
}

float iscreen_fps(void)
{
	return ikitwin_fps();
}

IINT64 iscreen_clock(void)
{
	return ikitwin_clock();
}

void iscreen_sleep(long millisecond)
{
	ikitwin_sleep(millisecond);
}

void iscreen_caption(const char *text)
{
	if (iscreen_inited == 0) return;
	ikitwin_set_caption(text);
}

void iscreen_vsync(int mode)
{
	if (iscreen_inited == 0) return;
#ifdef _WIN32
	if (iscreen_dx) {
		DDrawWaitForVerticalBlank(mode);
	}
#endif
}

void iblend_putpixel(IBITMAP *bmp, int x, int y, IUINT32 c)
{
	if (x >= 0 && y >= 0 && x < (int)bmp->w && y < (int)bmp->h)
		ibitmap_rectfill(bmp, x, y, 1, 1, c);
}


//=====================================================================
// MISC
//=====================================================================
#include <math.h>

#ifdef _WIN32
int CSetDIBitsToDevice(void *hDC, int x, int y, const IBITMAP *bitmap,
	int sx, int sy, int sw, int sh)
{
	char _buffer[sizeof(BITMAPINFO) + (256 + 4) * sizeof(RGBQUAD)];
	BITMAPINFO *info = (BITMAPINFO*)_buffer;
	int fmt = 0;
	int width = (int)bitmap->w;
	int height = (int)bitmap->h;
	int bpp = (int)bitmap->bpp;
	int bitfield = 0;
	int palsize = 0;
	LPRGBQUAD palette = NULL;
	IRECT rect;
	int hr;
	
	fmt = ibitmap_pixfmt_guess(bitmap);

	info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info->bmiHeader.biWidth = width;
	info->bmiHeader.biHeight = -height;
	info->bmiHeader.biPlanes = 1;
	info->bmiHeader.biBitCount = bpp;
	info->bmiHeader.biCompression = BI_RGB;
	info->bmiHeader.biSizeImage = bitmap->pitch * height;
	info->bmiHeader.biXPelsPerMeter = 0;
	info->bmiHeader.biYPelsPerMeter = 0;
	info->bmiHeader.biClrUsed = 0;
	info->bmiHeader.biClrImportant = 0;

	if (bpp < 15) {
		palsize = (1 << bpp) * sizeof(PALETTEENTRY);
	}
	else if (bpp == 16 || bpp == 32 || bpp == 24) {
		bitfield = 3 * sizeof(RGBQUAD);
	}

	if (fmt == IPIX_FMT_R8G8B8 || fmt == IPIX_FMT_B8G8R8) {
		info->bmiHeader.biCompression = BI_RGB;
	}
	else if (bpp >= 15) {
		IUINT32 *data = (IUINT32*)info;
		data[10] = ipixelfmt[fmt].rmask;
		data[11] = ipixelfmt[fmt].gmask;
		data[12] = ipixelfmt[fmt].bmask;
		data[13] = ipixelfmt[fmt].amask;
		info->bmiHeader.biCompression = BI_BITFIELDS;
	}

	if (palsize > 0) {
		long offset = sizeof(BITMAPINFOHEADER) + bitfield;
		palette = (LPRGBQUAD)((char*)info + offset);
	}	else {
		palette = NULL;
	}

	if (palette) {
		const iColorIndex *index = (const iColorIndex*)bitmap->extra;
		int i;
		if (index == NULL) {
			for (i = 0; i < 256; i++) {
				palette[i].rgbBlue = i;
				palette[i].rgbGreen = i;
				palette[i].rgbRed = i;
				palette[i].rgbReserved = 0;
			}
		}	else {
			for (i = 0; i < 256; i++) {
				int r, g, b, a;
				IRGBA_FROM_A8R8G8B8(index->rgba[i], r, g, b, a);
				a = 0;
				palette[i].rgbBlue = b;
				palette[i].rgbGreen = g;
				palette[i].rgbRed = r;
				palette[i].rgbReserved = a;
			}
		}
	}

	rect.left = sx;
	rect.top = sy;
	rect.right = sx + sw;
	rect.bottom = sy + sh;

	if (rect.left < 0) x -= rect.left, rect.left = 0;
	if (rect.top < 0) y -= rect.top, rect.top = 0;
	if (rect.right > width) rect.right = width;
	if (rect.bottom > height) rect.bottom = height;

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	if (width <= 0 || height <= 0) return 0;

	hr = SetDIBitsToDevice((HDC)hDC, x, y, width, height, 
						rect.left, rect.top, 
						0, (int)bitmap->h, bitmap->pixel,
						(LPBITMAPINFO)info, DIB_RGB_COLORS);

	if (hr <= 0) return -1;
	return 0;
}

int CStretchDIBits(void *hDC, int dx, int dy, int dw, int dh, 
	const IBITMAP *bitmap, int sx, int sy, int sw, int sh)
{
	char _buffer[sizeof(BITMAPINFO) + (256 + 4) * sizeof(RGBQUAD)];
	BITMAPINFO *info = (BITMAPINFO*)_buffer;
	int fmt = 0;
	int width = (int)bitmap->w;
	int height = (int)bitmap->h;
	int bpp = (int)bitmap->bpp;
	int bitfield = 0;
	int palsize = 0;
	LPRGBQUAD palette = NULL;
	IRECT clipdst, clipsrc;
	IRECT bound_dst, bound_src;
	int hr;
	
	fmt = ibitmap_pixfmt_guess(bitmap);

	info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info->bmiHeader.biWidth = width;
	info->bmiHeader.biHeight = -height;
	info->bmiHeader.biPlanes = 1;
	info->bmiHeader.biBitCount = bpp;
	info->bmiHeader.biCompression = BI_RGB;
	info->bmiHeader.biSizeImage = bitmap->pitch * height;
	info->bmiHeader.biXPelsPerMeter = 0;
	info->bmiHeader.biYPelsPerMeter = 0;
	info->bmiHeader.biClrUsed = 0;
	info->bmiHeader.biClrImportant = 0;

	if (bpp < 15) {
		palsize = (1 << bpp) * sizeof(PALETTEENTRY);
	}
	else if (bpp == 16 || bpp == 32 || bpp == 24) {
		bitfield = 3 * sizeof(RGBQUAD);
	}

	if (fmt == IPIX_FMT_R8G8B8 || fmt == IPIX_FMT_B8G8R8) {
		info->bmiHeader.biCompression = BI_RGB;
	}
	else if (bpp >= 15) {
		IUINT32 *data = (IUINT32*)info;
		data[10] = ipixelfmt[fmt].rmask;
		data[11] = ipixelfmt[fmt].gmask;
		data[12] = ipixelfmt[fmt].bmask;
		data[13] = ipixelfmt[fmt].amask;
		info->bmiHeader.biCompression = BI_BITFIELDS;
	}

	if (palsize > 0) {
		long offset = sizeof(BITMAPINFOHEADER) + bitfield;
		palette = (LPRGBQUAD)((char*)info + offset);
	}	else {
		palette = NULL;
	}

	if (palette) {
		const iColorIndex *index = (const iColorIndex*)bitmap->extra;
		int i;
		if (index == NULL) {
			for (i = 0; i < 256; i++) {
				palette[i].rgbBlue = i;
				palette[i].rgbGreen = i;
				palette[i].rgbRed = i;
				palette[i].rgbReserved = 0;
			}
		}	else {
			for (i = 0; i < 256; i++) {
				int r, g, b, a;
				IRGBA_FROM_A8R8G8B8(index->rgba[i], r, g, b, a);
				a = 0;
				palette[i].rgbBlue = b;
				palette[i].rgbGreen = g;
				palette[i].rgbRed = r;
				palette[i].rgbReserved = a;
			}
		}
	}

	clipdst.left = 0;
	clipdst.top = 0;
	clipdst.right = dx + dw;
	clipdst.bottom = dy + dh;
	clipsrc.left = 0;
	clipsrc.top = 0;
	clipsrc.right = (int)bitmap->w;
	clipsrc.bottom = (int)bitmap->h;

	bound_dst.left = dx;
	bound_dst.top = dy;
	bound_dst.right = dx + dw;
	bound_dst.bottom = dy + dh;
	bound_src.left = sx;
	bound_src.top = sy;
	bound_src.right = sx + sw;
	bound_src.bottom = sy + sh;

	if (ibitmap_clip_scale(&clipdst, &clipsrc, &bound_dst, &bound_src, 0))
		return -100;

	dx = bound_dst.left;
	dy = bound_dst.top;
	dw = bound_dst.right - bound_dst.left;
	dh = bound_dst.bottom - bound_dst.top;
	sx = bound_src.left;
	sy = bound_src.top;
	sw = bound_src.right - bound_src.left;
	sh = bound_src.bottom - bound_src.top;

	SetStretchBltMode((HDC)hDC, COLORONCOLOR);

	hr = StretchDIBits((HDC)hDC, dx, dy, dw, dh, sx, sy, sw, sh,
				bitmap->pixel, info, DIB_RGB_COLORS, SRCCOPY);

	if (hr == GDI_ERROR) return -200;

	return 0;
}

#endif

void imisc_bitmap_demo(IBITMAP *bmp, int type)
{
	int w = (int)bmp->w;
	int h = (int)bmp->h;
	int x, y, i, j, k;
	int pixfmt;
	pixfmt = ibitmap_pixfmt_guess(bmp);
	if (type == 0) {
		int size = 12, dd = 0;
#if 1
		iStoreProc store;
		IUINT32 *card;
		card = (IUINT32*)malloc(sizeof(IUINT32) * (bmp->w + size));
		if (card == NULL) return;
		for (i = 0, dd = 0, k = 0; i < (int)bmp->w + size; i++) {
			card[i] = (k == 0)? 0xffcccccc : 0xffffffff;
			if (++dd >= size) dd = 0, k ^= 1;
		}
		store = ipixel_get_store(ibitmap_pixfmt_guess(bmp), 0);
		for (j = 0, dd = 0, k = 0; j < h; j++) {
			store(bmp->line[j], card + k * size, 0, w, 
				(iColorIndex*)bmp->extra);
			if (++dd >= size) dd = 0, k ^= 1;
		}
		free(card);
#else
		for (y = 0, k = 0; y < h; y += size) {
			k = dd;
			dd ^= 1;
			for (x = 0; x < w; x += size, k++) {
				IUINT32 col = (k & 1)? 0xffcccccc : 0xffffffff;
				ibitmap_rectfill(bmp, x, y, size, size, col);
			}
		}
#endif
	}
	else if (type == 1) {
		int size = 32, dd = 0;
		for (y = 0, k = 0; y < h; y += size) {
			k = dd;
			dd ^= 1;
			for (x = 0; x < w; x += size, k++) {
				IUINT32 col = (k & 1)? 0xff0000ff : 0xff00003f;
				ibitmap_rectfill(bmp, x, y, size, size, col);
			}
		}
	}
	else if (type == 2) {
		ibitmap_rectfill(bmp, 0, 0, w, h, 0xffff00ff);
		for (j = -w - h; j < w + h; j += 32) {
			for (y = 0; y < h; y++) {
				iblend_putpixel(bmp, j + y, y, 0xffaaaaaa);
				iblend_putpixel(bmp, j - y, y, 0xffaaaaaa);
			}
		}
		for (i = 0; i < w; i += 96) {
			h = (int)(sin(3.14 * i / 640.0) * bmp->h * 0.3 + bmp->h * 0.7);
			for (y = 0; y < h; y++) {
				for (x = 0; x < 40; x++) {
					j = (40 - x) * 255 / 40;
					iblend_putpixel(bmp, i + x, bmp->h - 1 - y, 
						(j << 16) | (j << 8) | j | 0xff000000);
				}
			}
		}
		bmp->mask = ipixel_assemble(pixfmt, 0xff, 0, 0xff, 0xff);
	}
}


void imisc_bitmap_systext(IBITMAP *dst, int x, int y, const char *str, 
	IUINT32 color, IUINT32 bk, const IRECT *clip, int additive)
{
	//ifont_glyph_builtin(dst, x, y, str, color, bk, clip, additive);
}





// ibitmap_stretch(


