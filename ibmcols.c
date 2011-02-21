//=====================================================================
//
// ibmcols.c - ibitmap color components interface
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================

#include "ibmcols.h"
#include "ibmbits.h"


#ifdef __BORLANDC__
#pragma warn -8027
#pragma warn -8066
#endif

extern void*(*icmalloc)(size_t size);
extern void (*icfree)(void *ptr);

//---------------------------------------------------------------------
// 位图操作
//---------------------------------------------------------------------

#define IFLAG_PIXFMT_SET	0x80

// 取得像素格式，见ibmbits.h中的 IPIX_FMT_xxx
int ibitmap_pixfmt_get(const IBITMAP *bmp)
{
	IMODE mode;
	mode.mode = bmp->mode;
	if (mode.fmtset == 0) return -1;
	return mode.pixfmt;
}

// 设置像素格式
void ibitmap_pixfmt_set(IBITMAP *bmp, int pixfmt)
{
	IMODE mode;
	if (ipixelfmt[pixfmt].bpp != (int)bmp->bpp) {
		assert(ipixelfmt[pixfmt].bpp == (int)bmp->bpp);
		abort();
	}
	mode.mode = bmp->mode;
	mode.pixfmt = (unsigned char)(pixfmt & 63);
	mode.fmtset = 1;
	bmp->mode = mode.mode;
}

// 取得over flow模式
enum IBOM ibitmap_overflow_get(const IBITMAP *bmp)
{
	IMODE mode;
	mode.mode = bmp->mode;
	return (enum IBOM)mode.overflow;
}

// 设置over flow模式
void ibitmap_overflow_set(IBITMAP *bmp, enum IBOM of)
{
	IMODE mode;
	mode.mode = bmp->mode;
	mode.overflow = (unsigned char)(of & 3);
	bmp->mode = mode.mode;
}

// 取得颜色索引
iColorIndex *ibitmap_index_get(IBITMAP *bmp)
{
	return (iColorIndex*)bmp->extra;
}

// 设置颜色索引
void ibitmap_index_set(IBITMAP *bmp, iColorIndex *index)
{
	bmp->extra = index;
}

// 设置滤波器
void ibitmap_filter_set(IBITMAP *bmp, enum IPIXELFILTER filter)
{
	IMODE mode;
	mode.mode = bmp->mode;
	mode.filter = (unsigned char)(filter & 3);
	bmp->mode = mode.mode;
}

// 取得滤波器
enum IPIXELFILTER ibitmap_filter_get(const IBITMAP *bmp)
{
	IMODE mode;
	mode.mode = bmp->mode;
	return (IPIXELFILTER)mode.filter;
}


// BLIT 裁剪
int ibitmap_clipex(const IBITMAP *dst, int *dx, int *dy, const IBITMAP *src,
	int *sx, int *sy, int *sw, int *sh, const IRECT *clip, int flags)
{
	int clipdst[4], clipsrc[4], rect[4];
	int retval;
	if (clip == NULL) {
		clipdst[0] = 0;
		clipdst[1] = 0;
		clipdst[2] = (int)dst->w;
		clipdst[3] = (int)dst->h;
	}	else {
		clipdst[0] = clip->left;
		clipdst[1] = clip->top;
		clipdst[2] = clip->right;
		clipdst[3] = clip->bottom;
	}
	clipsrc[0] = 0;
	clipsrc[1] = 0;
	clipsrc[2] = (int)src->w;
	clipsrc[3] = (int)src->h;
	rect[0] = *sx;
	rect[1] = *sy;
	rect[2] = *sx + *sw;
	rect[3] = *sy + *sh;
	retval = ibitmap_clip(clipdst, clipsrc, dx, dy, rect, flags);
	if (retval) return retval;
	*sx = rect[0];
	*sy = rect[1];
	*sw = rect[2] - rect[0];
	*sh = rect[3] - rect[1];
	return 0;
}

// 混色绘制
void ibitmap_blend(IBITMAP *dst, int dx, int dy, const IBITMAP *src, int sx, 
	int sy, int w, int h, IUINT32 color, const IRECT *clip, int flags)
{
	unsigned char _buffer[IBITMAP_STACK_BUFFER];
	unsigned char *buffer = _buffer;
	int operate = IPIXEL_BLEND_OP_BLEND;
	int retval = 0, flip = 0, sfmt, dfmt;
	const iColorIndex *sindex;
	const iColorIndex *dindex;

	if ((flags & IBLIT_NOCLIP) == 0) {
		retval = ibitmap_clipex(dst, &dx, &dy, src, &sx, &sy, &w, &h, 
								clip, flags);
		if (retval) return;
	}

	if (w * 4 > IBITMAP_STACK_BUFFER) {
		buffer = (unsigned char*)icmalloc(w * 4);
		if (buffer == NULL) return;
	}

	if (flags & IBLIT_ADDITIVE) 
		operate = IPIXEL_BLEND_OP_ADD;

	if (flags & IBLIT_HFLIP) flip |= IPIXEL_FLIP_HFLIP;
	if (flags & IBLIT_VFLIP) flip |= IPIXEL_FLIP_VFLIP;

	sfmt = ibitmap_pixfmt_guess(src);
	dfmt = ibitmap_pixfmt_guess(dst);

	sindex = (const iColorIndex*)(src->extra);
	dindex = (iColorIndex*)(dst->extra);

	if (sindex == NULL) sindex = _ipixel_src_index;
	if (dindex == NULL) dindex = _ipixel_dst_index;

	ipixel_blend(dfmt, dst->line[dy], (long)dst->pitch, dx, sfmt,
		src->line[sy], (long)src->pitch, sx, w, h, color, 
		operate, flip, dindex, sindex, buffer);

	if (buffer != _buffer) {
		icfree(buffer);
	}
}

// 格式转换
void ibitmap_convert(IBITMAP *dst, int dx, int dy, const IBITMAP *src,
	int sx, int sy, int w, int h, const IRECT *clip, int flags)
{
	unsigned char _buffer[IBITMAP_STACK_BUFFER];
	unsigned char *buffer = _buffer;
	int retval = 0, flip = 0, sfmt, dfmt;
	const iColorIndex *sindex;
	const iColorIndex *dindex;

	if ((flags & IBLIT_NOCLIP) == 0) {
		retval = ibitmap_clipex(dst, &dx, &dy, src, &sx, &sy, &w, &h,
								clip, flags);
		if (retval) return;
	}

	if (flags & IBLIT_HFLIP) flip |= IPIXEL_FLIP_HFLIP;
	if (flags & IBLIT_VFLIP) flip |= IPIXEL_FLIP_VFLIP;

	sfmt = ibitmap_pixfmt_guess(src);
	dfmt = ibitmap_pixfmt_guess(dst);

	sindex = (const iColorIndex*)(src->extra);
	dindex = (iColorIndex*)(dst->extra);

	if (sindex == NULL) sindex = _ipixel_src_index;
	if (dindex == NULL) dindex = _ipixel_dst_index;

	// do not need convert
	if (ipixelfmt[dfmt].type != IPIX_FMT_TYPE_INDEX && dfmt == sfmt) {
		int newflags = (flags & (IBLIT_HFLIP | IBLIT_VFLIP));
		ibitmap_blit(dst, dx, dy, src, sx, sy, w, h, newflags);
		return;
	}

	if (w * 4 > IBITMAP_STACK_BUFFER) {
		buffer = (unsigned char*)icmalloc(w * 4);
		if (buffer == NULL) return;
	}

	ipixel_blend(dfmt, dst->line[dy], (long)dst->pitch, dx, sfmt,
		src->line[sy], (long)src->pitch, sx, w, h, 0xffffffff, 
		IPIXEL_BLEND_OP_COPY, flip, dindex, sindex, buffer);

	if (buffer != _buffer) {
		icfree(buffer);
	}
}


// 猜测颜色格式
int ibitmap_pixfmt_guess(const IBITMAP *src)
{
	int fmt;
	fmt = ibitmap_pixfmt_get(src);
	if (fmt < 0) {
		if (src->bpp == 8) return IPIX_FMT_C8;
		else if (src->bpp == 15) return IPIX_FMT_X1R5G5B5;
		else if (src->bpp == 16) return IPIX_FMT_R5G6B5;
		else if (src->bpp == 24) return IPIX_FMT_R8G8B8;
		else if (src->bpp == 32) return IPIX_FMT_A8R8G8B8;
	}
	return fmt;
}


// 颜色转换，生成新的IBITMAP
IBITMAP *ibitmap_convfmt(int dfmt, const IBITMAP *src, const IRGB *spal, 
	const IRGB *dpal)
{
	iColorIndex *sindex = NULL;
	iColorIndex *dindex = NULL;
	IBITMAP *dst = NULL;
	int sfmt, i;
	int w, h;

	sfmt = ibitmap_pixfmt_get(src);

	if (sfmt < 0) {
		if (src->bpp == 8) sfmt = IPIX_FMT_C8;
		else if (src->bpp == 15) sfmt = IPIX_FMT_X1R5G5B5;
		else if (src->bpp == 16) sfmt = IPIX_FMT_R5G6B5;
		else if (src->bpp == 24) sfmt = IPIX_FMT_R8G8B8;
		else if (src->bpp == 32) sfmt = IPIX_FMT_A8R8G8B8;
		else return NULL;
	}

	dst = ibitmap_create((int)src->w, (int)src->h, ipixelfmt[dfmt].bpp);
	if (dst == NULL) return NULL;

	ibitmap_pixfmt_set(dst, dfmt);

	w = (int)src->w;
	h = (int)src->h;

	if (ipixelfmt[sfmt].type == IPIX_FMT_TYPE_INDEX) {
		sindex = (iColorIndex*)icmalloc(sizeof(iColorIndex));
		if (sindex == NULL) {
			ibitmap_release(dst);
			return NULL;
		}
		if (spal == NULL) spal = _ipaletted;
		for (i = 0; i < 256; i++) {
			sindex->rgba[i] = IRGBA_TO_A8R8G8B8(spal[i].r, spal[i].g, 
				spal[i].b, 255);
		}
	}

	if (ipixelfmt[dfmt].type == IPIX_FMT_TYPE_INDEX) {
		dindex = (iColorIndex*)icmalloc(sizeof(iColorIndex));
		if (dindex == NULL) {
			if (sindex != NULL) icfree(sindex);
			ibitmap_release(dst);
			return NULL;
		}
		if (dpal == NULL) dpal = _ipaletted;
		ipalette_to_index(dindex, dpal, 256);
	}

	// do not need convert
	if (ipixelfmt[dfmt].type != IPIX_FMT_TYPE_INDEX && dfmt == sfmt) {
		ibitmap_blit(dst, 0, 0, src, 0, 0, w, h, 0);
	}	else {
		IUINT8 *buffer;
		buffer = (IUINT8*)icmalloc(w * 4);
		if (buffer) {
			ipixel_blend(dfmt, dst->line[0], (long)dst->pitch, 0, sfmt,
				src->line[0], (long)src->pitch, 0, w, h, 0xffffffff, 
				IPIXEL_BLEND_OP_COPY, 0, dindex, sindex, buffer);
			icfree(buffer);
		}	else {
			ibitmap_release(dst);
			dst = NULL;
		}
	}

	if (sindex) icfree(sindex);
	if (dindex) icfree(dindex);

	return dst;
}


// 绘制矩形
void ibitmap_rectfill(IBITMAP *dst, int x, int y, int w, int h, IUINT32 c)
{
	int pixfmt = ibitmap_pixfmt_guess(dst);
	iHLineDrawProc proc;
	iColorIndex *index;
	proc = ipixel_get_hline_proc(pixfmt, 0, 0);
	index = (iColorIndex*)dst->extra;
	if (x >= (int)dst->w || y >= (int)dst->h) return;
	if (x < 0) w += x, x = 0;
	if (x + w >= (int)dst->w) w = (int)dst->w - x;
	if (w <= 0) return;
	if (y < 0) h += y, y = 0;
	if (y + h >= (int)dst->h) h = (int)dst->h - y;
	if (h <= 0) return;
	for (; h > 0; y++, h--) {
		proc(dst->line[y], x, w, c, NULL, index);
	}
}


// 引用BITMAP
IBITMAP *ibitmap_reference_new(void *ptr, long pitch, int w, int h, int fmt)
{
	IBITMAP *bitmap;
	int i;
	bitmap = (IBITMAP*)icmalloc(sizeof(IBITMAP));
	if (bitmap == NULL) return NULL;
	bitmap->pixel = (unsigned char*)ptr;
	bitmap->line = (void**)icmalloc(sizeof(void*) * h);
	if (bitmap->line == NULL) {
		icfree(bitmap);
		return NULL;
	}
	for (i = 0; i < h; i++) 
		bitmap->line[i] = (char*)ptr + pitch * i;
	bitmap->bpp = ipixelfmt[fmt].bpp;
	bitmap->w = (unsigned)w;
	bitmap->h = (unsigned)h;
	bitmap->pitch = (unsigned long)pitch;
	bitmap->mask = 0;
	bitmap->code = 0;
	bitmap->mode = 0;
	bitmap->extra = 0;
	ibitmap_imode(bitmap, refbits) = 1;
	ibitmap_pixfmt_set(bitmap, fmt);
	return bitmap;
}


// 删除引用
void ibitmap_reference_del(IBITMAP *bmp)
{
	if (bmp) {
		if (ibitmap_imode(bmp, refbits) == 0) {
			assert(ibitmap_imode(bmp, refbits) != 0);
			abort();
			return;
		}
		if (bmp->line) {
			icfree(bmp->line);
			bmp->line = NULL;
		}
		bmp->pixel = NULL;
		bmp->mode = 0;
		icfree(bmp);
	}
}


// 调整引用
void ibitmap_reference_adjust(IBITMAP *bmp, void *ptr, long pitch)
{
	int i;
	if (bmp->pixel == ptr && bmp->pitch == (unsigned long)pitch)
		return;
	bmp->pixel = ptr;
	bmp->pitch = (unsigned long)pitch;
	for (i = 0; i < (int)bmp->h; i++) {
		bmp->line[i] = (char*)ptr + i * bmp->pitch;
	}
}


// 带遮罩的填充，alpha必须是一个8位格式的alpha位图(IPIX_FMT_A8)
void ibitmap_maskfill(IBITMAP *dst, int dx, int dy, const IBITMAP *alpha,
	int sx, int sy, int sw, int sh, IUINT32 color, const IRECT *clip)
{
	iHLineDrawProc proc;
	int y;
	assert(alpha->bpp == 8);
	if (alpha->bpp != 8) return;
	if (ibitmap_clipex(dst, &dx, &dy, alpha, &sx, &sy, &sw, &sh, 
						clip, 0) != 0) {
		return;
	}
	proc = ipixel_get_hline_proc(ibitmap_pixfmt_guess(dst), 0, 0);
	for (y = 0; y < sh; y++) {
		const IUINT8 *source = (IUINT8*)alpha->line[sy + y] + sx;
		IUINT8 *dest = (IUINT8*)dst->line[dy + y];
		proc(dest, dx, sw, color, source, NULL);
	}
}


// 截取IBITMAP 区域中的部分
IBITMAP *ibitmap_chop(const IBITMAP *src, int x, int y, int w, int h)
{
	IBITMAP *bitmap;
	assert(w > 0 && h > 0);
	if (w <= 0 || h <= 0) return NULL;
	bitmap = ibitmap_create(w, h, src->bpp);
	assert(bitmap);
	if (bitmap == NULL) return NULL;
	ibitmap_pixfmt_set(bitmap, ibitmap_pixfmt_guess(src));
	ibitmap_blit(bitmap, 0, 0, src, x, y, w, h, 0);
	return bitmap;
}


//=====================================================================
// 扫描线存储或绘制
//=====================================================================
void ibitmap_scanline_store(IBITMAP *bmp, int x, int y, int w,
	const IUINT32 *card, const IRECT *clip, iStoreProc store)
{
	const iColorIndex *index = (const iColorIndex*)bmp->extra;
	if (y < clip->top || y >= clip->bottom || w <= 0) return;
	if (x >= clip->right || x + w <= clip->left) return;
	if (x < clip->left) {
		int diff = clip->left - x;
		card += diff;
		x = clip->left;
		w -= diff;
	}
	if (x + w >= clip->right) {
		int diff = x + w - clip->right;
		w -= diff;
	}
	if (w < 0) return;
	store(bmp->line[y], card, x, w, index);
}

void ibitmap_scanline_blend(IBITMAP *bmp, int x, int y, int w, const IUINT32 
	*card, const IUINT8 *cover, const IRECT *clip, iSpanDrawProc span)
{
	const iColorIndex *index = (const iColorIndex*)bmp->extra;
	if (y < clip->top || y >= clip->bottom || w <= 0) return;
	if (x >= clip->right || x + w <= clip->left) return;
	if (x < clip->left) {
		int diff = clip->left - x;
		card += diff;
		if (cover) cover += diff;
		x = clip->left;
		w -= diff;
	}
	if (x + w >= clip->right) {
		int diff = x + w - clip->right;
		w -= diff;
	}
	if (w < 0) return;
	span(bmp->line[y], x, w, card, cover, index);
}


//=====================================================================
// 读点操作
//=====================================================================
static inline int ipixel_overflow(int *X, int *Y, const IRECT *rect,
	enum IBOM mode)
{
	int x = *X;
	int y = *Y;
	if (x >= rect->left && y >= rect->top && x < rect->right &&
		y < rect->bottom) 
		return 0;
	switch (mode) 
	{
	case IBOM_TRANSPARENT:
		return -1;
		break;
	case IBOM_REPEAT:
		x = (x < rect->left)? rect->left : 
			(x >= rect->right)? (rect->right - 1) : x;
		y = (y < rect->top)? rect->top : 
			(y >= rect->bottom)? (rect->bottom - 1) : y;
		break;
	case IBOM_WRAP: 
		{
			int w = rect->right - rect->left;
			int h = rect->bottom - rect->top;
			x = (x - rect->left) % w;
			y = (y - rect->top) % h;
			if (x < 0) x += w;
			if (y < 0) y += h;
			x += rect->left;
			y += rect->top;
			break;
		}
	case IBOM_MIRROR:
		{
			int w = rect->right - rect->left;
			int h = rect->bottom - rect->top;
			x -= rect->left;
			y -= rect->top;
			if (x < 0) x = (-x) % w;
			else if (x >= w) x = w - 1 - (x % w);
			if (y < 0) y = (-y) % h;
			else if (y >= h) y = h - 1 - (y % h);
			x += rect->left;
			y += rect->top;
			break;
		}
	}
	*X = x;
	*Y = y;
	return 0;
}


static inline IUINT32 ipixel_biline_interp (IUINT32 tl, IUINT32 tr,
	IUINT32 bl, IUINT32 br, int distx, int disty)
{
    int distxy, distxiy, distixy, distixiy;
    IUINT32 f, r;

    distxy = distx * disty;
    distxiy = (distx << 8) - distxy;	/* distx * (256 - disty) */
    distixy = (disty << 8) - distxy;	/* disty * (256 - distx) */
    distixiy =
	256 * 256 - (disty << 8) -
	(distx << 8) + distxy;		/* (256 - distx) * (256 - disty) */

    /* Blue */
    r = (tl & 0x000000ff) * distixiy + (tr & 0x000000ff) * distxiy
      + (bl & 0x000000ff) * distixy  + (br & 0x000000ff) * distxy;

    /* Green */
    f = (tl & 0x0000ff00) * distixiy + (tr & 0x0000ff00) * distxiy
      + (bl & 0x0000ff00) * distixy  + (br & 0x0000ff00) * distxy;
    r |= f & 0xff000000;

    tl >>= 16;
    tr >>= 16;
    bl >>= 16;
    br >>= 16;
    r >>= 16;

    /* Red */
    f = (tl & 0x000000ff) * distixiy + (tr & 0x000000ff) * distxiy
      + (bl & 0x000000ff) * distixy  + (br & 0x000000ff) * distxy;
    r |= f & 0x00ff0000;

    /* Alpha */
    f = (tl & 0x0000ff00) * distixiy + (tr & 0x0000ff00) * distxiy
      + (bl & 0x0000ff00) * distixy  + (br & 0x0000ff00) * distxy;
    r |= f & 0xff000000;

    return r;
}

// 取得over flow模式
static inline enum IBOM ibitmap_overflow_get_fast(const IBITMAP *bmp)
{
	IMODE mode;
	mode.mode = bmp->mode;
	return (enum IBOM)mode.overflow;
}

static inline IUINT32 ibitmap_fetch_pixel(const IBITMAP *bmp, 
	int x, int y, const IRECT *clip, iFetchPixelProc get_pixel)
{
	enum IBOM mode = ibitmap_overflow_get_fast(bmp);
	const iColorIndex *index = (const iColorIndex*)bmp->extra;
	if (ipixel_overflow(&x, &y, clip, mode) != 0) {
		return (IUINT32)bmp->mask;
	}
	return get_pixel(bmp->line[y], x, index);
}

static inline IUINT32 ibitmap_fetch_pixel_nearest(const IBITMAP *bmp,
	cfixed x, cfixed y, const IRECT *clip, iFetchPixelProc get_pixel)
{
	enum IBOM mode = ibitmap_overflow_get_fast(bmp);
	const iColorIndex *index = (const iColorIndex*)bmp->extra;
	int x0 = cfixed_to_int(x - cfixed_const_e);
	int y0 = cfixed_to_int(y - cfixed_const_e);
	if (ipixel_overflow(&x0, &y0, clip, mode) != 0) {
		return (IUINT32)bmp->mask;
	}
	return get_pixel(bmp->line[y0], x0, index);
}

static inline IUINT32 ibitmap_fetch_pixel_bilinear(const IBITMAP *bmp,
	cfixed x, cfixed y, const IRECT *clip, iFetchPixelProc get_pixel)
{
	const iColorIndex *index = (const iColorIndex*)bmp->extra;
	IBOM mode = ibitmap_overflow_get_fast(bmp);
	IUINT32 c00, c01, c10, c11;
	IINT32 distx, disty;
	IINT32 x1, y1, x2, y2;
	x1 = x - cfixed_const_1 / 2;
	y1 = y - cfixed_const_1 / 2;
	distx = (x1 >> 8) & 0xff;
	disty = (y1 >> 8) & 0xff;
	x1 = cfixed_to_int(x1);
	y1 = cfixed_to_int(y1);
	x2 = x1 + 1;
	y2 = y1 + 1;
	if (x1 >= clip->left && y1 >= clip->top && x2 < clip->right &&
		y2 < clip->bottom) {
		c00 = get_pixel(bmp->line[y1], x1, index);
		c01 = get_pixel(bmp->line[y1], x2, index);
		c10 = get_pixel(bmp->line[y2], x1, index);
		c11 = get_pixel(bmp->line[y2], x2, index);
	}	
	else {
		if (mode == IBOM_TRANSPARENT) {
			if (x1 >= clip->right || y1 >= clip->bottom ||
				x2 < clip->left || y2 < clip->top)
				return (IUINT32)bmp->mask;
		}
		c00 = ibitmap_fetch_pixel(bmp, x1, y1, clip, get_pixel);
		c01 = ibitmap_fetch_pixel(bmp, x2, y1, clip, get_pixel);
		c10 = ibitmap_fetch_pixel(bmp, x1, y1, clip, get_pixel);
		c11 = ibitmap_fetch_pixel(bmp, x2, y1, clip, get_pixel);
	}
	return ipixel_biline_interp(c00, c01, c10, c11, distx, disty);
}




//=====================================================================
// 扫描线获取函数
//=====================================================================
static iBitmapFetchProc ibitmap_fetch_proc_table[IPIX_FMT_COUNT][18][2];
static iBitmapFetchFloat ibitmap_fetch_float_table[IPIX_FMT_COUNT][2];

// 清空函数列表
static void ibitmap_fetch_proc_table_clear(void)
{
	static int inited = 0;
	if (inited == 0) {
		int i, j;
		for (i = 0; i < IPIX_FMT_COUNT; i++) {
			for (j = 0; j < 18; j++) {
				ibitmap_fetch_proc_table[i][j][0] = NULL;
				ibitmap_fetch_proc_table[i][j][1] = NULL;
			}
			ibitmap_fetch_float_table[i][0] = NULL;
			ibitmap_fetch_float_table[i][1] = NULL;
		}
		inited = 1;
	}
}

// 初始化函数列表
static void ibitmap_fetch_proc_table_init(void);


// 通用读取扫描线
static void ibitmap_fetch_general(const IBITMAP *bmp, IUINT32 *card, 
	int w, const cfixed *source, const cfixed *step, const IRECT *clip);

// 通用读取浮点扫描线
static void ibitmap_fetch_general_float(const IBITMAP *bmp, IUINT32 *card, 
	int width, const float *source, const float *step, const IRECT *clip);


//---------------------------------------------------------------------
// 取得扫描线函数
//---------------------------------------------------------------------
iBitmapFetchProc ibitmap_scanline_get_proc(int pixfmt, int mode, int isdef)
{
	iBitmapFetchProc proc;
	static int inited = 0;

	if (inited == 0) {
		ibitmap_fetch_proc_table_init();
		inited = 1;
	}

	assert(pixfmt >= 0 && pixfmt < IPIX_FMT_COUNT && mode >= 0 && mode < 18);

	if (pixfmt < 0 || pixfmt >= IPIX_FMT_COUNT || mode < 0 || mode >= 18)
		return NULL;

	proc = ibitmap_fetch_proc_table[pixfmt][mode][(isdef)? 0 : 1];

	if (proc == NULL) {
		return ibitmap_fetch_general;
	}

	return proc;
}

// 取得浮点数扫描线函数
iBitmapFetchFloat ibitmap_scanline_get_float(int pixfmt, int isdefault)
{
	iBitmapFetchFloat proc;
	static int inited = 0;

	if (inited == 0) {
		ibitmap_fetch_proc_table_init();
		inited = 1;
	}

	assert(pixfmt >= 0 && pixfmt < IPIX_FMT_COUNT);

	if (pixfmt < 0 || pixfmt >= IPIX_FMT_COUNT)
		return NULL;

	proc = ibitmap_fetch_float_table[pixfmt][(isdefault)? 0 : 1];

	if (proc == NULL) {
		return ibitmap_fetch_general_float;
	}

	return proc;
}


//---------------------------------------------------------------------
// 设置扫描线函数
//---------------------------------------------------------------------
void ibitmap_scanline_set_proc(int pixfmt, int mode, iBitmapFetchProc proc)
{
	static int inited = 0;
	if (inited == 0) {
		ibitmap_fetch_proc_table_init();
		inited = 1;
	}
	assert(pixfmt >= 0 && pixfmt < IPIX_FMT_COUNT && mode >= 0 && mode < 18);

	if (pixfmt < 0 || pixfmt >= IPIX_FMT_COUNT || mode < 0 || mode >= 18)
		return;

	ibitmap_fetch_proc_table[pixfmt][mode][1] = proc;
}

// 设置扫描线浮点函数
void ibitmap_scanline_set_float(int pixfmt, iBitmapFetchFloat proc)
{
	static int inited = 0;
	if (inited == 0) {
		ibitmap_fetch_proc_table_init();
		inited = 1;
	}
	assert(pixfmt >= 0 && pixfmt < IPIX_FMT_COUNT);

	if (pixfmt < 0 || pixfmt >= IPIX_FMT_COUNT)
		return;

	ibitmap_fetch_float_table[pixfmt][1] = proc;
}


//---------------------------------------------------------------------
// 取得模式
//---------------------------------------------------------------------
int ibitmap_scanline_get_mode(const IBITMAP *bmp, const cfixed *src,
	const cfixed *step)
{
	IPIXELFILTER filter;
	IBOM overflow;
	int basetype;
	int index;

	overflow = (IBOM)ibitmap_imode_const(bmp, overflow);
	filter = (IPIXELFILTER)ibitmap_imode_const(bmp, filter);

	if (src[2] != cfixed_const_1 || step[1] != 0 || step[2] != 0) {
		if (filter == IPIXEL_FILTER_BILINEAR) 
			return IBITMAP_FETCH_GENERAL_BILINEAR;
		return IBITMAP_FETCH_GENERAL_NEAREST;
	}

	basetype = 1;

	switch (overflow)
	{
	case IBOM_TRANSPARENT: index = IBITMAP_FETCH_TRANSLATE_NEAREST; break;
	case IBOM_REPEAT: index = IBITMAP_FETCH_REPEAT_TRANSLATE_NEAREST; break;
	case IBOM_WRAP: index = IBITMAP_FETCH_WRAP_TRANSLATE_NEAREST; break;
	case IBOM_MIRROR: index = IBITMAP_FETCH_MIRROR_TRANSLATE_NEAREST; break;
	default:
		if (filter == IPIXEL_FILTER_BILINEAR) 
			return IBITMAP_FETCH_GENERAL_BILINEAR;
		return IBITMAP_FETCH_GENERAL_NEAREST;
	}

	index += basetype * 2;

	if (filter == IPIXEL_FILTER_BILINEAR) return index + 1;

	return index;
}


// 通用取扫描线
static void ibitmap_fetch_general(const IBITMAP *bmp, IUINT32 *card, 
	int width, const cfixed *source, const cfixed *step, const IRECT *clip)
{
	cfixed u, v, w, du, dv, dw, x, y;
	iFetchPixelProc proc;
	IPIXELFILTER filter;
	IBOM mode;

	u = source[0];
	v = source[1];
	w = source[2];
	du = step[0];
	dv = step[1];
	dw = step[2];

	mode = ibitmap_overflow_get_fast(bmp);
	filter = (IPIXELFILTER)ibitmap_imode_const(bmp, filter);
	proc = ipixel_get_fetchpixel(ibitmap_imode_const(bmp, pixfmt), 0);

	if (filter == IPIXEL_FILTER_BILINEAR) {
		if (w == cfixed_const_1 && dw == 0) {
			for (; width > 0; u += du, v += dv, card++, width--) {
				*card = ibitmap_fetch_pixel_bilinear(bmp, u, v, clip, proc);
			}
		}	else {
			for (; width > 0; u += du, v += dv, w += dw, card++, width--) {
				if (w != 0) {
					x = cfixed_div(u, w);
					y = cfixed_div(v, w);
				}	else { 
					x = 0, y = 0;
				}
				*card = ibitmap_fetch_pixel_bilinear(bmp, x, y, clip, proc);
			}
		}
	}
	else if (filter == IPIXEL_FILTER_NEAREST) {
		if (w == cfixed_const_1 && dw == 0) {
			for (; width > 0; u += du, v += dv, card++, width--) {
				*card = ibitmap_fetch_pixel_nearest(bmp, u, v, clip, proc);
			}
		}	else {
			for (; width > 0; u += du, v += dv, w += dw, card++, width--) {
				if (w != 0) {
					x = cfixed_div(u, w);
					y = cfixed_div(v, w);
				}	else { 
					x = 0, y = 0;
				}
				*card = ibitmap_fetch_pixel_nearest(bmp, x, y, clip, proc);
			}
		}
	}
}

// 通用取扫描线
static void ibitmap_fetch_general_float(const IBITMAP *bmp, IUINT32 *card, 
	int width, const float *source, const float *step, const IRECT *clip)
{
	float u, v, w, du, dv, dw, iw;
	iFetchPixelProc proc;
	IPIXELFILTER filter;
	cfixed x, y;
	IBOM mode;

	u = source[0];
	v = source[1];
	w = source[2];
	du = step[0];
	dv = step[1];
	dw = step[2];

	mode = ibitmap_overflow_get_fast(bmp);
	filter = (IPIXELFILTER)ibitmap_imode_const(bmp, filter);
	proc = ipixel_get_fetchpixel(ibitmap_imode_const(bmp, pixfmt), 0);

	if (cfloat_ieee_enable()) {
		if (filter == IPIXEL_FILTER_BILINEAR) {
			if (w == 1.0f && dw == 0.0f) {
				for (; width > 0; u += du, v += dv, card++, width--) {
					x = cfixed_from_float_ieee(u);
					y = cfixed_from_float_ieee(v);
					*card = ibitmap_fetch_pixel_bilinear(bmp, 
						x, y, clip, proc);
				}
			}	else {
				iw = (w == 0.0f)? 0.0f : (65536.0f / w);
				for (; width > 0; width--) {
					x = (cfixed)cfloat_to_int_ieee(u * iw);
					y = (cfixed)cfloat_to_int_ieee(v * iw);
					u += du, v += dv, w += dw;
					iw = (w == 0.0f)? 0.0f : (65536.0f / w);
					*card = ibitmap_fetch_pixel_bilinear(bmp, x, y, clip, 
						proc);
					card++;
				}
			}
		}
		else if (filter == IPIXEL_FILTER_NEAREST) {
			if (w == 1.0f && dw == 0.0f) {
				for (; width > 0; u += du, v += dv, card++, width--) {
					x = cfixed_from_float_ieee(u);
					y = cfixed_from_float_ieee(v);
					*card = ibitmap_fetch_pixel_nearest(bmp, x, y,
						clip, proc);
				}
			}	else {
				iw = (w == 0.0f)? 0.0f : (65536.0f / w);
				for (; width > 0; width--) {
					x = (cfixed)cfloat_to_int_ieee(u * iw);
					y = (cfixed)cfloat_to_int_ieee(v * iw);
					u += du, v += dv, w += dw;
					iw = (w == 0.0f)? 0.0f : (65536.0f / w);
					*card = ibitmap_fetch_pixel_nearest(bmp, x, y, clip, 
						proc);
					card++;
				}
			}
		}
		return;
	}

	if (filter == IPIXEL_FILTER_BILINEAR) {
		if (w == 1.0f && dw == 0.0f) {
			for (; width > 0; u += du, v += dv, card++, width--) {
				x = cfixed_from_float(u);
				y = cfixed_from_float(v);
				*card = ibitmap_fetch_pixel_bilinear(bmp, x, y, clip, proc);
			}
		}	else {
			iw = (w == 0.0f)? 0.0f : (65536.0f / w);
			for (; width > 0; u += du, v += dv, w += dw, card++, width--) {
				x = (cfixed)(u * iw);
				y = (cfixed)(v * iw);
				u += du, v += dv, w += dw;
				iw = (w == 0.0f)? 0.0f : (65536.0f / w);
				*card = ibitmap_fetch_pixel_bilinear(bmp, x, y, clip, proc);
			}
		}
	}
	else if (filter == IPIXEL_FILTER_NEAREST) {
		if (w == 1.0f && dw == 0.0f) {
			for (; width > 0; u += du, v += dv, card++, width--) {
				x = cfixed_from_float(u);
				y = cfixed_from_float(v);
				*card = ibitmap_fetch_pixel_nearest(bmp, x, y, clip, proc);
			}
		}	else {
			iw = (w == 0.0f)? 0.0f : (65536.0f / w);
			for (; width > 0; u += du, v += dv, w += dw, card++, width--) {
				x = (cfixed)(u * iw);
				y = (cfixed)(v * iw);
				u += du, v += dv, w += dw;
				iw = (w == 0.0f)? 0.0f : (65536.0f / w);
				*card = ibitmap_fetch_pixel_nearest(bmp, x, y, clip, proc);
			}
		}
	}
}


// 通用取得浮点扫描线
void ibitmap_scanline_float(const IBITMAP *bmp, IUINT32 *card, int w,
	const float *srcvec, const float *stepvec, const IRECT *clip)
{
	iBitmapFetchFloat fscanline;
	fscanline = ibitmap_scanline_get_float(ibitmap_pixfmt_guess(bmp), 0);
	fscanline(bmp, card, w, srcvec, stepvec, clip);
}

// 通用取得描线：使用浮点数内核的定点数接口
void ibitmap_scanline_fixed(const IBITMAP *bmp, IUINT32 *card, int w,
	const cfixed *srcvec, const cfixed *stepvec, const IRECT *clip)
{
	float src[3], step[3];
	src[0] = cfixed_to_float(srcvec[0]);
	src[1] = cfixed_to_float(srcvec[1]);
	src[2] = cfixed_to_float(srcvec[2]);
	step[0] = cfixed_to_float(stepvec[0]);
	step[1] = cfixed_to_float(stepvec[1]);
	step[2] = cfixed_to_float(stepvec[2]);
	ibitmap_scanline_float(bmp, card, w, src, step, clip);
}


// 读取像素
static inline IUINT32 ibitmap_fetch_pixel_int_A8R8G8B8(
	const IBITMAP *bmp, int x, int y, const IRECT *clip) 
{
	enum IBOM mode = ibitmap_overflow_get_fast(bmp); 
	if (ipixel_overflow(&x, &y, clip, mode) != 0) { 
		return (IUINT32)bmp->mask; 
	} 
	return ((IUINT32*)bmp->line[y])[x];
}

// 读取像素
static inline IUINT32 ibitmap_fetch_pixel_nearest_A8R8G8B8( 
	const IBITMAP *bmp, cfixed x, cfixed y, const IRECT *clip) 
{ 
	enum IBOM mode = ibitmap_overflow_get_fast(bmp); 
	int x0 = cfixed_to_int(x - cfixed_const_e); 
	int y0 = cfixed_to_int(y - cfixed_const_e); 
	if (ipixel_overflow(&x0, &y0, clip, mode) != 0) { 
		return (IUINT32)bmp->mask; 
	} 
	return ((IUINT32*)bmp->line[y0])[x0];
} 

// 读取像素
static inline IUINT32 ibitmap_fetch_pixel_int_X8R8G8B8(
	const IBITMAP *bmp, int x, int y, const IRECT *clip) 
{
	enum IBOM mode = ibitmap_overflow_get_fast(bmp); 
	if (ipixel_overflow(&x, &y, clip, mode) != 0) { 
		return (IUINT32)bmp->mask; 
	} 
	return ((IUINT32*)bmp->line[y])[x] | 0xff000000;
}

// 读取像素
static inline IUINT32 ibitmap_fetch_pixel_nearest_X8R8G8B8( 
	const IBITMAP *bmp, cfixed x, cfixed y, const IRECT *clip) 
{ 
	enum IBOM mode = ibitmap_overflow_get_fast(bmp); 
	int x0 = cfixed_to_int(x - cfixed_const_e); 
	int y0 = cfixed_to_int(y - cfixed_const_e); 
	if (ipixel_overflow(&x0, &y0, clip, mode) != 0) { 
		return (IUINT32)bmp->mask; 
	} 
	return ((IUINT32*)bmp->line[y0])[x0] | 0xff000000;
} 

// 读取像素
static inline IUINT32 ibitmap_fetch_pixel_int_R8G8B8(
	const IBITMAP *bmp, int x, int y, const IRECT *clip) 
{
	enum IBOM mode = ibitmap_overflow_get_fast(bmp); 
	if (ipixel_overflow(&x, &y, clip, mode) != 0) { 
		return (IUINT32)bmp->mask; 
	} 
	return _ipixel_fetch(24, bmp->line[y], x) | 0xff000000;
}

// 读取像素
static inline IUINT32 ibitmap_fetch_pixel_nearest_R8G8B8( 
	const IBITMAP *bmp, cfixed x, cfixed y, const IRECT *clip) 
{ 
	enum IBOM mode = ibitmap_overflow_get_fast(bmp); 
	int x0 = cfixed_to_int(x - cfixed_const_e); 
	int y0 = cfixed_to_int(y - cfixed_const_e); 
	if (ipixel_overflow(&x0, &y0, clip, mode) != 0) { 
		return (IUINT32)bmp->mask; 
	}
	return _ipixel_fetch(24, bmp->line[y0], x0) | 0xff000000;
} 

static inline IUINT32 ibitmap_fetch_pixel_bilinear_A8R8G8B8( 
	const IBITMAP *bmp, cfixed x, cfixed y, const IRECT *clip) 
{ 
	IBOM mode = ibitmap_overflow_get_fast(bmp); 
	IUINT32 c00, c01, c10, c11; 
	IINT32 distx, disty; 
	IINT32 x1, y1, x2, y2; 
	x1 = x - cfixed_const_1 / 2; 
	y1 = y - cfixed_const_1 / 2; 
	distx = (x1 >> 8) & 0xff; 
	disty = (y1 >> 8) & 0xff; 
	x1 = cfixed_to_int(x1); 
	y1 = cfixed_to_int(y1); 
	x2 = x1 + 1; 
	y2 = y1 + 1; 
	if (x1 >= clip->left && y1 >= clip->top && x2 < clip->right && 
		y2 < clip->bottom) { 
		c00 = _ipixel_fetch(32, bmp->line[y1], x1); 
		c01 = _ipixel_fetch(32, bmp->line[y1], x2); 
		c10 = _ipixel_fetch(32, bmp->line[y2], x1); 
		c11 = _ipixel_fetch(32, bmp->line[y2], x2); 
	}	
	else { 
		if (mode == IBOM_TRANSPARENT) { 
			if (x1 >= clip->right || y1 >= clip->bottom || 
				x2 < clip->left || y2 < clip->top) 
				return (IUINT32)bmp->mask; 
		} 
		c00 = ibitmap_fetch_pixel_int_A8R8G8B8(bmp, x1, y1, clip); 
		c01 = ibitmap_fetch_pixel_int_A8R8G8B8(bmp, x2, y1, clip); 
		c10 = ibitmap_fetch_pixel_int_A8R8G8B8(bmp, x1, y1, clip); 
		c11 = ibitmap_fetch_pixel_int_A8R8G8B8(bmp, x2, y1, clip); 
	} 
	return ipixel_biline_interp(c00, c01, c10, c11, distx, disty); 
}

static inline IUINT32 ibitmap_fetch_pixel_bilinear_X8R8G8B8( 
	const IBITMAP *bmp, cfixed x, cfixed y, const IRECT *clip) 
{ 
	IBOM mode = ibitmap_overflow_get_fast(bmp); 
	IUINT32 c00, c01, c10, c11; 
	IINT32 distx, disty; 
	IINT32 x1, y1, x2, y2; 
	x1 = x - cfixed_const_1 / 2; 
	y1 = y - cfixed_const_1 / 2; 
	distx = (x1 >> 8) & 0xff; 
	disty = (y1 >> 8) & 0xff; 
	x1 = cfixed_to_int(x1); 
	y1 = cfixed_to_int(y1); 
	x2 = x1 + 1; 
	y2 = y1 + 1; 
	if (x1 >= clip->left && y1 >= clip->top && x2 < clip->right && 
		y2 < clip->bottom) { 
		c00 = _ipixel_fetch(32, bmp->line[y1], x1); 
		c01 = _ipixel_fetch(32, bmp->line[y1], x2); 
		c10 = _ipixel_fetch(32, bmp->line[y2], x1); 
		c11 = _ipixel_fetch(32, bmp->line[y2], x2); 
		return ipixel_biline_interp(c00, c01, c10, c11, distx, disty) |
			0xff000000;
	}	
	else { 
		if (mode == IBOM_TRANSPARENT) { 
			if (x1 >= clip->right || y1 >= clip->bottom || 
				x2 < clip->left || y2 < clip->top) 
				return (IUINT32)bmp->mask; 
		} 
		c00 = ibitmap_fetch_pixel_int_X8R8G8B8(bmp, x1, y1, clip); 
		c01 = ibitmap_fetch_pixel_int_X8R8G8B8(bmp, x2, y1, clip); 
		c10 = ibitmap_fetch_pixel_int_X8R8G8B8(bmp, x1, y1, clip); 
		c11 = ibitmap_fetch_pixel_int_X8R8G8B8(bmp, x2, y1, clip); 
	} 
	return ipixel_biline_interp(c00, c01, c10, c11, distx, disty);
}


static inline IUINT32 ibitmap_fetch_pixel_bilinear_R8G8B8( 
	const IBITMAP *bmp, cfixed x, cfixed y, const IRECT *clip) 
{ 
	IBOM mode = ibitmap_overflow_get_fast(bmp); 
	IUINT32 c00, c01, c10, c11; 
	IINT32 distx, disty; 
	IINT32 x1, y1, x2, y2; 
	x1 = x - cfixed_const_1 / 2; 
	y1 = y - cfixed_const_1 / 2; 
	distx = (x1 >> 8) & 0xff; 
	disty = (y1 >> 8) & 0xff; 
	x1 = cfixed_to_int(x1); 
	y1 = cfixed_to_int(y1); 
	x2 = x1 + 1; 
	y2 = y1 + 1; 
	if (x1 >= clip->left && y1 >= clip->top && x2 < clip->right && 
		y2 < clip->bottom) { 
		c00 = _ipixel_fetch(24, bmp->line[y1], x1); 
		c01 = _ipixel_fetch(24, bmp->line[y1], x2); 
		c10 = _ipixel_fetch(24, bmp->line[y2], x1); 
		c11 = _ipixel_fetch(24, bmp->line[y2], x2); 
		return ipixel_biline_interp(c00, c01, c10, c11, distx, disty) |
			0xff000000;
	}	
	else { 
		if (mode == IBOM_TRANSPARENT) { 
			if (x1 >= clip->right || y1 >= clip->bottom || 
				x2 < clip->left || y2 < clip->top) 
				return (IUINT32)bmp->mask; 
		} 
		c00 = ibitmap_fetch_pixel_int_R8G8B8(bmp, x1, y1, clip); 
		c01 = ibitmap_fetch_pixel_int_R8G8B8(bmp, x2, y1, clip); 
		c10 = ibitmap_fetch_pixel_int_R8G8B8(bmp, x1, y1, clip); 
		c11 = ibitmap_fetch_pixel_int_R8G8B8(bmp, x2, y1, clip); 
	} 
	return ipixel_biline_interp(c00, c01, c10, c11, distx, disty);
}


// 通用读点的模板
#define IBITMAP_FETCH_PIXEL(fmt, bpp) \
static inline IUINT32 ibitmap_fetch_pixel_int_##fmt( \
	const IBITMAP *bmp, int x, int y, const IRECT *clip) \
{ \
	enum IBOM mode = ibitmap_overflow_get_fast(bmp); \
	const iColorIndex *_ipixel_src_index = (const iColorIndex*)bmp->extra; \
	IUINT32 c, r, g, b, a; \
	if (ipixel_overflow(&x, &y, clip, mode) != 0) { \
		return (IUINT32)bmp->mask; \
	} \
	c = _ipixel_fetch(bpp, bmp->line[y], x); \
	IRGBA_FROM_PIXEL(fmt, c, r, g, b, a); \
	_ipixel_src_index = _ipixel_src_index; \
	return IRGBA_TO_PIXEL(A8R8G8B8, r, g, b, a); \
} \
static inline IUINT32 ibitmap_fetch_pixel_nearest_##fmt( \
	const IBITMAP *bmp, cfixed x, cfixed y, const IRECT *clip) \
{ \
	enum IBOM mode = ibitmap_overflow_get_fast(bmp); \
	const iColorIndex *_ipixel_src_index = (const iColorIndex*)bmp->extra; \
	int x0 = cfixed_to_int(x - cfixed_const_e); \
	int y0 = cfixed_to_int(y - cfixed_const_e); \
	IUINT32 c, r, g, b, a; \
	if (ipixel_overflow(&x0, &y0, clip, mode) != 0) { \
		return (IUINT32)bmp->mask; \
	} \
	c = _ipixel_fetch(bpp, bmp->line[y0], x0); \
	IRGBA_FROM_PIXEL(fmt, c, r, g, b, a); \
	_ipixel_src_index = _ipixel_src_index; \
	return IRGBA_TO_PIXEL(A8R8G8B8, r, g, b, a); \
} \
static inline IUINT32 ibitmap_fetch_pixel_bilinear_##fmt( \
	const IBITMAP *bmp, cfixed x, cfixed y, const IRECT *clip) \
{ \
	const iColorIndex *_ipixel_src_index = (const iColorIndex*)bmp->extra; \
	IBOM mode = ibitmap_overflow_get_fast(bmp); \
	IUINT32 c00, c01, c10, c11; \
	IINT32 distx, disty; \
	IINT32 x1, y1, x2, y2; \
	x1 = x - cfixed_const_1 / 2; \
	y1 = y - cfixed_const_1 / 2; \
	distx = (x1 >> 8) & 0xff; \
	disty = (y1 >> 8) & 0xff; \
	x1 = cfixed_to_int(x1); \
	y1 = cfixed_to_int(y1); \
	x2 = x1 + 1; \
	y2 = y1 + 1; \
	if (x1 >= clip->left && y1 >= clip->top && x2 < clip->right && \
		y2 < clip->bottom) { \
		IUINT32 r1, g1, b1, a1, r2, g2, b2, a2; \
		c00 = _ipixel_fetch(bpp, bmp->line[y1], x1); \
		c01 = _ipixel_fetch(bpp, bmp->line[y1], x2); \
		c10 = _ipixel_fetch(bpp, bmp->line[y2], x1); \
		c11 = _ipixel_fetch(bpp, bmp->line[y2], x2); \
		IRGBA_FROM_PIXEL(fmt, c00, r1, g1, b1, a1); \
		IRGBA_FROM_PIXEL(fmt, c01, r2, g2, b2, a2); \
		c00 = IRGBA_TO_PIXEL(A8R8G8B8, r1, g1, b1, a1); \
		c01 = IRGBA_TO_PIXEL(A8R8G8B8, r2, g2, b2, a2); \
		IRGBA_FROM_PIXEL(fmt, c10, r1, g1, b1, a1); \
		IRGBA_FROM_PIXEL(fmt, c11, r2, g2, b2, a2); \
		c10 = IRGBA_TO_PIXEL(A8R8G8B8, r1, g1, b1, a1); \
		c11 = IRGBA_TO_PIXEL(A8R8G8B8, r2, g2, b2, a2); \
	}	\
	else { \
		if (mode == IBOM_TRANSPARENT) { \
			if (x1 >= clip->right || y1 >= clip->bottom || \
				x2 < clip->left || y2 < clip->top) \
				return (IUINT32)bmp->mask; \
		} \
		c00 = ibitmap_fetch_pixel_int_##fmt(bmp, x1, y1, clip); \
		c01 = ibitmap_fetch_pixel_int_##fmt(bmp, x2, y1, clip); \
		c10 = ibitmap_fetch_pixel_int_##fmt(bmp, x1, y1, clip); \
		c11 = ibitmap_fetch_pixel_int_##fmt(bmp, x2, y1, clip); \
	} \
	_ipixel_src_index = _ipixel_src_index; \
	return ipixel_biline_interp(c00, c01, c10, c11, distx, disty); \
}


// 通用取扫描线模板
#define IBITMAP_FETCH_GENERAL(fmt) \
static void ibitmap_fetch_general_##fmt(const IBITMAP *bmp, IUINT32 *card, \
	int width, const cfixed *source, const cfixed *step, const IRECT *clip) \
{ \
	cfixed u, v, w, du, dv, dw, x, y; \
	IPIXELFILTER filter; \
	IBOM mode; \
	u = source[0]; \
	v = source[1]; \
	w = source[2]; \
	du = step[0]; \
	dv = step[1]; \
	dw = step[2]; \
	mode = ibitmap_overflow_get_fast(bmp); \
	filter = (IPIXELFILTER)ibitmap_imode_const(bmp, filter); \
	if (filter == IPIXEL_FILTER_BILINEAR) { \
		if (w == cfixed_const_1 && dw == 0) { \
			for (; width > 0; u += du, v += dv, card++, width--) { \
				*card = ibitmap_fetch_pixel_bilinear_##fmt( \
							bmp, u, v, clip); \
			} \
		}	else { \
			for (; width > 0; u += du, v += dv, w += dw, card++, width--) { \
				if (w != 0) { \
					x = cfixed_div(u, w); \
					y = cfixed_div(v, w); \
				}	else {  \
					x = 0, y = 0; \
				} \
				*card = ibitmap_fetch_pixel_bilinear_##fmt(bmp, x, y, clip);\
			} \
		} \
	} \
	else if (filter == IPIXEL_FILTER_NEAREST) { \
		if (w == cfixed_const_1 && dw == 0) { \
			for (; width > 0; u += du, v += dv, card++, width--) { \
				*card = ibitmap_fetch_pixel_nearest_##fmt( \
							bmp, u, v, clip); \
			} \
		}	else { \
			for (; width > 0; u += du, v += dv, w += dw, card++, width--) { \
				if (w != 0) { \
					x = cfixed_div(u, w); \
					y = cfixed_div(v, w); \
				}	else {  \
					x = 0, y = 0; \
				} \
				*card = ibitmap_fetch_pixel_nearest_##fmt( \
							bmp, x, y, clip); \
			} \
		} \
	} \
}

IBITMAP_FETCH_PIXEL(A8B8G8R8, 32); 
IBITMAP_FETCH_PIXEL(R8G8B8A8, 32);
IBITMAP_FETCH_PIXEL(B8G8R8A8, 32); 
IBITMAP_FETCH_PIXEL(X8B8G8R8, 32);
IBITMAP_FETCH_PIXEL(R8G8B8X8, 32);
IBITMAP_FETCH_PIXEL(B8G8R8X8, 32);
IBITMAP_FETCH_PIXEL(B8G8R8, 24);
IBITMAP_FETCH_PIXEL(R5G6B5, 16);
IBITMAP_FETCH_PIXEL(B5G6R5, 16);
IBITMAP_FETCH_PIXEL(X1R5G5B5, 16);
IBITMAP_FETCH_PIXEL(X1B5G5R5, 16);
IBITMAP_FETCH_PIXEL(R5G5B5X1, 16);
IBITMAP_FETCH_PIXEL(B5G5R5X1, 16);
IBITMAP_FETCH_PIXEL(A1R5G5B5, 16);
IBITMAP_FETCH_PIXEL(A1B5G5R5, 16);
IBITMAP_FETCH_PIXEL(R5G5B5A1, 16);
IBITMAP_FETCH_PIXEL(B5G5R5A1, 16);
IBITMAP_FETCH_PIXEL(A4R4G4B4, 16);
IBITMAP_FETCH_PIXEL(A4B4G4R4, 16);
IBITMAP_FETCH_PIXEL(R4G4B4A4, 16);
IBITMAP_FETCH_PIXEL(B4G4R4A4, 16);
IBITMAP_FETCH_PIXEL(C8, 8);
IBITMAP_FETCH_PIXEL(A8, 8);
IBITMAP_FETCH_PIXEL(G8, 8);

IBITMAP_FETCH_GENERAL(A8R8G8B8);
IBITMAP_FETCH_GENERAL(A8B8G8R8);
IBITMAP_FETCH_GENERAL(R8G8B8A8);
IBITMAP_FETCH_GENERAL(B8G8R8A8);
IBITMAP_FETCH_GENERAL(X8R8G8B8);
IBITMAP_FETCH_GENERAL(X8B8G8R8);
IBITMAP_FETCH_GENERAL(R8G8B8X8);
IBITMAP_FETCH_GENERAL(B8G8R8X8);
IBITMAP_FETCH_GENERAL(R8G8B8);
IBITMAP_FETCH_GENERAL(B8G8R8);
IBITMAP_FETCH_GENERAL(R5G6B5);
IBITMAP_FETCH_GENERAL(B5G6R5);
IBITMAP_FETCH_GENERAL(X1R5G5B5);
IBITMAP_FETCH_GENERAL(X1B5G5R5);
IBITMAP_FETCH_GENERAL(R5G5B5X1);
IBITMAP_FETCH_GENERAL(B5G5R5X1);
IBITMAP_FETCH_GENERAL(A1R5G5B5);
IBITMAP_FETCH_GENERAL(A1B5G5R5);
IBITMAP_FETCH_GENERAL(R5G5B5A1);
IBITMAP_FETCH_GENERAL(B5G5R5A1);
IBITMAP_FETCH_GENERAL(A4R4G4B4);
IBITMAP_FETCH_GENERAL(A4B4G4R4);
IBITMAP_FETCH_GENERAL(R4G4B4A4);
IBITMAP_FETCH_GENERAL(B4G4R4A4);
IBITMAP_FETCH_GENERAL(C8);
IBITMAP_FETCH_GENERAL(A8);
IBITMAP_FETCH_GENERAL(G8);



// 平移的fetch
static void ibitmap_fetch_translate_int(const IBITMAP *bmp, IUINT32 *card,
	int width, const cfixed *source, const cfixed *step, const IRECT *clip) 
{
	const iColorIndex *index = (const iColorIndex*)bmp->extra;
	cfixed u, v, w, du, dv, dw;
	iFetchProc fetch;
	IUINT32 mask;
	int x, y, i, d;
	int overflow;
	int left;
	int right;

	u = source[0];
	v = source[1];
	w = source[2];
	du = step[0];
	dv = step[1];
	dw = step[2];

	if (!(cfixed_is_zero(dw) && cfixed_is_zero(dv) && 
		cfixed_is_one(du) && cfixed_is_one(w))) {
		assert(0);
		abort();
	}

	fetch = ipixel_get_fetch(ibitmap_pixfmt_guess(bmp), 0);
	
	x = cfixed_to_int(u - cfixed_const_e);
	y = cfixed_to_int(v - cfixed_const_e);
	overflow = (int)ibitmap_imode_const(bmp, overflow);
	mask = (IUINT32)bmp->mask;

	left = clip->left;
	right = clip->right;
	
	switch (overflow) 
	{
	case IBOM_TRANSPARENT: 
		{
			if (y < clip->top || y >= clip->bottom || 
				x >= right || x + width <= left) {
				for (i = 0; i < width; i++) *card++ = mask;
			}	
			else {
				if (x < left) {
					int d = left - x;
					for (i = 0; i < d; i++) *card++ = mask;
					x += d;
					width -= d;
				}
				if (x + width > right) {
					d = x + width - right;
					for (i = 0; i < d; i++) card[width - 1 - i] = mask;
					width -= d;
				}
				assert(x >= left && x + width <= right && width > 0);
				fetch(bmp->line[y], x, width, card, index);
			}
		}
		break;
	case IBOM_REPEAT: 
		{
			if (y < clip->top) y = clip->top;
			if (y >= clip->bottom) y = clip->bottom - 1;
			if (x + width > right) {
				IUINT32 cc; 
				fetch(bmp->line[y], right - 1, 1, &cc, index);
				d = x + width - right;
				if (d > width) d = width;
				for (i = 0; i < d; i++) card[width - 1 - i] = cc;
				width -= d;
			}
			if (x < left) {
				IUINT32 cc; 
				fetch(bmp->line[y], left, 1, &cc, index);
				d = left - x;
				if (d > width) d = width;
				for (i = 0; i < d; i++) *card++ = cc;
				x += d;
				width -= d;
			}
			if (width > 0) {
				fetch(bmp->line[y], x, width, card, index);
			}
		}
		break;
	case IBOM_WRAP: 
		{
			int cw = right - left;
			int ch = clip->bottom - clip->top;
			if (y < clip->top || y >= clip->bottom) 
				y = clip->top + (y - clip->top) % ch;
			while (width > 0) {
				int canread;
				if (x < left || x >= right) 
					x = left + (x - left) % cw;
				canread = right - x;
				if (canread > width) canread = width;
				fetch(bmp->line[y], x, canread, card, index);
				x += canread;
				card += canread;
				width -= canread;
			}
		}
		break;
	case IBOM_MIRROR:
		{
			iBitmapFetchProc proc;
			proc = ibitmap_scanline_get_proc(ibitmap_pixfmt_guess(bmp),
				IBITMAP_FETCH_GENERAL_NEAREST, 0);
			proc(bmp, card, w, source, step, clip);
		}
		break;
	}
}


// 初始化函数查找表
static void ibitmap_fetch_proc_table_init(void)
{
	static int inited = 0;
	int fmt, i;
	if (inited != 0) return;
	ibitmap_fetch_proc_table_clear();
	#define ibitmap_fetch_proc_table_init_general(fmt) do { \
				int z; \
				for (z = 0; z < 18; z++) { \
					ibitmap_fetch_proc_table[IPIX_FMT_##fmt][z][0] = \
						ibitmap_fetch_general_##fmt; \
					ibitmap_fetch_proc_table[IPIX_FMT_##fmt][z][1] = \
						ibitmap_fetch_general_##fmt; \
				}	\
			}	while (0)
#if 1
	ibitmap_fetch_proc_table_init_general(A8R8G8B8);
	ibitmap_fetch_proc_table_init_general(A8B8G8R8);
	ibitmap_fetch_proc_table_init_general(R8G8B8A8);
	ibitmap_fetch_proc_table_init_general(B8G8R8A8);
	ibitmap_fetch_proc_table_init_general(X8R8G8B8);
	ibitmap_fetch_proc_table_init_general(X8B8G8R8);
	ibitmap_fetch_proc_table_init_general(R8G8B8X8);
	ibitmap_fetch_proc_table_init_general(B8G8R8X8);
	ibitmap_fetch_proc_table_init_general(R8G8B8);
	ibitmap_fetch_proc_table_init_general(B8G8R8);
	ibitmap_fetch_proc_table_init_general(R5G6B5);
	ibitmap_fetch_proc_table_init_general(B5G6R5);
	ibitmap_fetch_proc_table_init_general(X1R5G5B5);
	ibitmap_fetch_proc_table_init_general(X1B5G5R5);
	ibitmap_fetch_proc_table_init_general(R5G5B5X1);
	ibitmap_fetch_proc_table_init_general(B5G5R5X1);
	ibitmap_fetch_proc_table_init_general(A1R5G5B5);
	ibitmap_fetch_proc_table_init_general(A1B5G5R5);
	ibitmap_fetch_proc_table_init_general(R5G5B5A1);
	ibitmap_fetch_proc_table_init_general(B5G5R5A1);
	ibitmap_fetch_proc_table_init_general(A4R4G4B4);
	ibitmap_fetch_proc_table_init_general(A4B4G4R4);
	ibitmap_fetch_proc_table_init_general(R4G4B4A4);
	ibitmap_fetch_proc_table_init_general(B4G4R4A4);
	ibitmap_fetch_proc_table_init_general(C8);
	ibitmap_fetch_proc_table_init_general(A8);
	ibitmap_fetch_proc_table_init_general(G8);
	for (fmt = 0; fmt < 48; fmt++) {
		for (i = 0; i < 3; i++) {
			int pos = 2 + i * 4;
			ibitmap_fetch_proc_table[fmt][pos + 0][0] = 
				ibitmap_fetch_translate_int;
			ibitmap_fetch_proc_table[fmt][pos + 0][1] = 
				ibitmap_fetch_translate_int;
			ibitmap_fetch_proc_table[fmt][pos + 1][0] = 
				ibitmap_fetch_translate_int;
			ibitmap_fetch_proc_table[fmt][pos + 1][1] = 
				ibitmap_fetch_translate_int;
		}
	}
#endif
	#undef ibitmap_fetch_proc_table_init_general
	inited = 1;
}


//=====================================================================
// 批量写点
//=====================================================================
struct iPixelListProcTable 
{
	iPixelListSC pixel_list_sc, pixel_list_sc_default;
	iPixelListMC pixel_list_mc, pixel_list_mc_default;
};


#define IPIXEL_DOT_DRAW_PROC_X(fmt, bpp, mode) \
static void ipixel_list_proc_sc_##fmt(IBITMAP *bmp, const IUINT32 *xy, \
	int size, IUINT32 color, int add) \
{ \
	IUINT32 r1, g1, b1, a1, c1, r, g, b, a; \
	unsigned char *ptr; \
	void **lines = bmp->line; \
	iColorIndex *_ipixel_src_index = (iColorIndex*)bmp->extra; \
	iColorIndex *_ipixel_dst_index = (iColorIndex*)bmp->extra; \
	IRGBA_FROM_A8R8G8B8(color, r, g, b, a); \
	if (a == 0) return; \
	if (add) { \
		r1 = g1 = b1 = a1 = 0; \
		IBLEND_ADDITIVE(r, g, b, a, r1, g1, b1, a1); \
		r = r1; g = g1; b = b1; a = a1; \
		for (; size > 0; size--, xy += 2) { \
			ptr = (unsigned char*)lines[xy[1]]; \
			c1 = _ipixel_fetch(bpp, ptr, xy[0]); \
			IRGBA_FROM_PIXEL(fmt, c1, r1, g1, b1, a1); \
			r1 = ICLIP_256(r1 + r); \
			g1 = ICLIP_256(g1 + g); \
			b1 = ICLIP_256(b1 + b); \
			a1 = ICLIP_256(a1 + a); \
			c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			_ipixel_store(bpp, ptr, xy[0], c1); \
		} \
		return; \
	}	\
	if (a == 255) { \
		c1 = IRGBA_TO_PIXEL(fmt, r, g, b, 255); \
		for (; size > 0; size--, xy += 2) { \
			ptr = (unsigned char*)lines[xy[1]]; \
			_ipixel_store(bpp, ptr, xy[0], c1); \
		} \
	}	else { \
		for (; size > 0; size--, xy += 2) { \
			ptr = (unsigned char*)lines[xy[1]]; \
			c1 = _ipixel_fetch(bpp, ptr, xy[0]); \
			IRGBA_FROM_PIXEL(fmt, c1, r1, g1, b1, a1); \
			IBLEND_##mode(r, g, b, a, r1, g1, b1, a1); \
			c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			_ipixel_store(bpp, ptr, xy[0], c1); \
		} \
	} \
	_ipixel_src_index = _ipixel_src_index; \
	_ipixel_dst_index = _ipixel_dst_index; \
} \
static void ipixel_list_proc_mc_##fmt(IBITMAP *bmp, const IUINT32 *data, \
	int size, int additive) \
{ \
	IUINT32 r1, g1, b1, a1, c1, c, r, g, b, a; \
	unsigned char *ptr; \
	void **lines = bmp->line; \
	iColorIndex *_ipixel_src_index = (iColorIndex*)bmp->extra; \
	iColorIndex *_ipixel_dst_index = (iColorIndex*)bmp->extra; \
	if (additive) { \
		for (; size > 0; size--, data += 3) { \
			ptr = (unsigned char*)lines[data[1]]; \
			c = data[2]; \
			IRGBA_FROM_PIXEL(A8R8G8B8, c, r, g, b, a); \
			if (a != 0) { \
				c1 = _ipixel_fetch(bpp, ptr, data[0]); \
				IRGBA_FROM_PIXEL(fmt, c1, r1, g1, b1, a1); \
				IBLEND_ADDITIVE(r, g, b, a, r1, g1, b1, a1); \
				c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
				_ipixel_store(bpp, ptr, data[0], c1); \
			} \
		} \
		return; \
	} \
	for (; size > 0; size--, data += 3) { \
		ptr = (unsigned char*)lines[data[1]]; \
		c = data[2]; \
		IRGBA_FROM_PIXEL(A8R8G8B8, c, r, g, b, a); \
		if (a == 255) { \
			c1 = IRGBA_TO_PIXEL(fmt, r, g, b, 0xff); \
			_ipixel_store(bpp, ptr, data[0], c1); \
		} \
		else if (a > 0) { \
			c1 = _ipixel_fetch(bpp, ptr, data[0]); \
			IRGBA_FROM_PIXEL(fmt, c1, r1, g1, b1, a1); \
			IBLEND_STATIC(r, g, b, a, r1, g1, b1, a1); \
			c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
			_ipixel_store(bpp, ptr, data[0], c1); \
		} \
	} \
	_ipixel_src_index = _ipixel_src_index; \
	_ipixel_dst_index = _ipixel_dst_index; \
}


IPIXEL_DOT_DRAW_PROC_X(A8R8G8B8, 32, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(A8B8G8R8, 32, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(R8G8B8A8, 32, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(B8G8R8A8, 32, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(X8R8G8B8, 32, STATIC)
IPIXEL_DOT_DRAW_PROC_X(X8B8G8R8, 32, STATIC)
IPIXEL_DOT_DRAW_PROC_X(R8G8B8X8, 32, STATIC)
IPIXEL_DOT_DRAW_PROC_X(B8G8R8X8, 32, STATIC)
IPIXEL_DOT_DRAW_PROC_X(P8R8G8B8, 32, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(R8G8B8, 24, STATIC)
IPIXEL_DOT_DRAW_PROC_X(B8G8R8, 24, STATIC)
IPIXEL_DOT_DRAW_PROC_X(R5G6B5, 16, STATIC)
IPIXEL_DOT_DRAW_PROC_X(B5G6R5, 16, STATIC)
IPIXEL_DOT_DRAW_PROC_X(X1R5G5B5, 16, STATIC)
IPIXEL_DOT_DRAW_PROC_X(X1B5G5R5, 16, STATIC)
IPIXEL_DOT_DRAW_PROC_X(R5G5B5X1, 16, STATIC)
IPIXEL_DOT_DRAW_PROC_X(B5G5R5X1, 16, STATIC)
IPIXEL_DOT_DRAW_PROC_X(A1R5G5B5, 16, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(A1B5G5R5, 16, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(R5G5B5A1, 16, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(B5G5R5A1, 16, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(X4R4G4B4, 16, STATIC)
IPIXEL_DOT_DRAW_PROC_X(X4B4G4R4, 16, STATIC)
IPIXEL_DOT_DRAW_PROC_X(R4G4B4X4, 16, STATIC)
IPIXEL_DOT_DRAW_PROC_X(B4G4R4X4, 16, STATIC)
IPIXEL_DOT_DRAW_PROC_X(A4R4G4B4, 16, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(A4B4G4R4, 16, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(R4G4B4A4, 16, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(B4G4R4A4, 16, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(C8, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(G8, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(A8, 8, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(R3G3B2, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(B2G3R3, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(X2R2G2B2, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(X2B2G2R2, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(R2G2B2X2, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(B2G2R2X2, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(A2R2G2B2, 8, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(A2B2G2R2, 8, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(R2G2B2A2, 8, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(B2G2R2A2, 8, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(X4C4, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(X4G4, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(X4A4, 8, NORMAL_FAST)
IPIXEL_DOT_DRAW_PROC_X(C4X4, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(G4X4, 8, STATIC)
IPIXEL_DOT_DRAW_PROC_X(A4X4, 8, NORMAL_FAST)


#define ITABLE_ITEM(fmt) \
	{ ipixel_list_proc_sc_##fmt, ipixel_list_proc_sc_##fmt, \
	  ipixel_list_proc_mc_##fmt, ipixel_list_proc_mc_##fmt }

static struct iPixelListProcTable ipixel_list_proc_table[IPIX_FMT_COUNT] = 
{
	ITABLE_ITEM(A8R8G8B8),
	ITABLE_ITEM(A8B8G8R8),
	ITABLE_ITEM(R8G8B8A8),
	ITABLE_ITEM(B8G8R8A8),
	ITABLE_ITEM(X8R8G8B8),
	ITABLE_ITEM(X8B8G8R8),
	ITABLE_ITEM(R8G8B8X8),
	ITABLE_ITEM(B8G8R8X8),
	ITABLE_ITEM(P8R8G8B8),
	ITABLE_ITEM(R8G8B8),
	ITABLE_ITEM(B8G8R8),
	ITABLE_ITEM(R5G6B5),
	ITABLE_ITEM(B5G6R5),
	ITABLE_ITEM(X1R5G5B5),
	ITABLE_ITEM(X1B5G5R5),
	ITABLE_ITEM(R5G5B5X1),
	ITABLE_ITEM(B5G5R5X1),
	ITABLE_ITEM(A1R5G5B5),
	ITABLE_ITEM(A1B5G5R5),
	ITABLE_ITEM(R5G5B5A1),
	ITABLE_ITEM(B5G5R5A1),
	ITABLE_ITEM(X4R4G4B4),
	ITABLE_ITEM(X4B4G4R4),
	ITABLE_ITEM(R4G4B4X4),
	ITABLE_ITEM(B4G4R4X4),
	ITABLE_ITEM(A4R4G4B4),
	ITABLE_ITEM(A4B4G4R4),
	ITABLE_ITEM(R4G4B4A4),
	ITABLE_ITEM(B4G4R4A4),
	ITABLE_ITEM(C8),
	ITABLE_ITEM(G8),
	ITABLE_ITEM(A8),
	ITABLE_ITEM(R3G3B2),
	ITABLE_ITEM(B2G3R3),
	ITABLE_ITEM(X2R2G2B2),
	ITABLE_ITEM(X2B2G2R2),
	ITABLE_ITEM(R2G2B2X2),
	ITABLE_ITEM(B2G2R2X2),
	ITABLE_ITEM(A2R2G2B2),
	ITABLE_ITEM(A2B2G2R2),
	ITABLE_ITEM(R2G2B2A2),
	ITABLE_ITEM(B2G2R2A2),
	ITABLE_ITEM(X4C4),
	ITABLE_ITEM(X4G4),
	ITABLE_ITEM(X4A4),
	ITABLE_ITEM(C4X4),
	ITABLE_ITEM(G4X4),
	ITABLE_ITEM(A4X4),
};

#undef ITABLE_ITEM


iPixelListSC ibitmap_get_pixel_list_sc_proc(int pixfmt, int isdefault)
{
	assert(pixfmt >= 0 && pixfmt <= IPIX_FMT_A4X4);
	if (pixfmt < 0 || pixfmt > IPIX_FMT_A4X4) return NULL;
	if (isdefault == 0) return ipixel_list_proc_table[pixfmt].pixel_list_sc;
	return ipixel_list_proc_table[pixfmt].pixel_list_sc_default;
}

iPixelListMC ibitmap_get_pixel_list_mc_proc(int pixfmt, int isdefault)
{
	assert(pixfmt >= 0 && pixfmt <= IPIX_FMT_A4X4);
	if (pixfmt < 0 || pixfmt > IPIX_FMT_A4X4) return NULL;
	if (isdefault == 0) return ipixel_list_proc_table[pixfmt].pixel_list_mc;
	return ipixel_list_proc_table[pixfmt].pixel_list_mc_default;
}

void ibitmap_set_pixel_list_sc_proc(int pixfmt, iPixelListSC proc)
{
	assert(pixfmt >= 0 && pixfmt <= IPIX_FMT_A4X4);
	if (pixfmt < 0 || pixfmt > IPIX_FMT_A4X4) return;
	ipixel_list_proc_table[pixfmt].pixel_list_sc = proc;
}

void ibitmap_set_pixel_list_mc_proc(int pixfmt, iPixelListMC proc)
{
	assert(pixfmt >= 0 && pixfmt <= IPIX_FMT_A4X4);
	if (pixfmt < 0 || pixfmt > IPIX_FMT_A4X4) return;
	ipixel_list_proc_table[pixfmt].pixel_list_mc = proc;
}

void ibitmap_draw_pixel_list_sc(IBITMAP *bmp, const IUINT32 *xy, int count,
	IUINT32 color, int additive)
{
	iPixelListSC proc;
	int pixfmt;
	pixfmt = ibitmap_pixfmt_guess(bmp);
	proc = ibitmap_get_pixel_list_sc_proc(pixfmt, 0);
	if (proc) {
		proc(bmp, xy, count, color, additive);
	}	else {
		assert(proc);
	}
}

void ibitmap_draw_pixel_list_mc(IBITMAP *bmp, const IUINT32 *xyc, int count,
	int additive)
{
	iPixelListMC proc;
	int pixfmt;
	pixfmt = ibitmap_pixfmt_guess(bmp);
	proc = ibitmap_get_pixel_list_mc_proc(pixfmt, 0);
	if (proc) {
		proc(bmp, xyc, count, additive);
	}	else {
		assert(proc);
	}
}



//=====================================================================
// GLYPH IMPLEMENTATION
//=====================================================================
#define IDRAW_GLYPH_PROC(draw_pixel) { \
	const unsigned char *data = glyph->data; \
	const unsigned char *src; \
	int w = glyph->w; \
	int h = glyph->h; \
	int pitch = (w + 7) / 8; \
	int lgap = 0; \
	int d, i, j, xx, yy; \
	int shift; \
	int drawit; \
	if (clip) { \
		if (y < clip->top) { \
			d = clip->top - y; \
			h -= d; \
			if (h <= 0) return; \
			data += d * pitch; \
			y = clip->top; \
		} \
		if (y + h > clip->bottom) { \
			h = clip->bottom - y; \
			if (h <= 0) return; \
		} \
		if (x < clip->left) { \
			d = clip->left - x; \
			w -= d; \
			if (w <= 0) return; \
			data += d / 8; \
			lgap = d & 7; \
			x = clip->left; \
		} \
		if (x + w > clip->right) { \
			w = clip->right - x; \
			if (w <= 0) return; \
		} \
	} \
	while (h--) { \
		j = 0; \
		i = 0x80 >> lgap; \
		src = data; \
		d = *(src++); \
		shift = 7 - lgap; \
		yy = y; xx = x; \
		for (; ; ) { \
			drawit = (d & i) >> shift; \
			draw_pixel; \
			j++; \
			if (j >= w) break; \
			shift--; \
			i >>= 1; \
			if (i == 0) { \
				i = 0x80; \
				shift = 7; \
				d = *(src++); \
			} \
			xx++; \
		} \
		data += pitch; \
		y++; \
		yy++; \
	} \
}

void ibitmap_draw_glyph_font(IBITMAP *dst, const IGLYPHCHAR *glyph,
	const IRECT *clip, int x, int y, IUINT32 color, IUINT32 bk, 
	int additive, void *workmem)
{
	IUINT32 *xy1, *xy2;
	int p1, p2;
	xy1 = (IUINT32*)workmem;
	xy2 = (IUINT32*)workmem + glyph->w * glyph->h * 2;
	p1 = p2 = 0;
	if ((bk >> 24) == 0) {
		IDRAW_GLYPH_PROC( 
			{
				if (drawit) {
					xy1[p1++] = (IUINT32)xx;
					xy1[p1++] = (IUINT32)yy;
				}
			}
		);
		ibitmap_draw_pixel_list_sc(dst, xy1, p1 / 2, color, additive);
	}	else {
		IDRAW_GLYPH_PROC( 
			{
				if (drawit) {
					xy1[p1++] = (IUINT32)xx;
					xy1[p1++] = (IUINT32)yy;
				}	else {
					xy2[p2++] = (IUINT32)xx;
					xy2[p2++] = (IUINT32)yy;
				}
			}
		);
		ibitmap_draw_pixel_list_sc(dst, xy1, p1 / 2, color, additive);
		ibitmap_draw_pixel_list_sc(dst, xy2, p2 / 2, bk, additive);
	}
}

void ibitmap_draw_glyph(IBITMAP *dst, const IGLYPHCHAR *glyph, int x, int y,
	const IRECT *clip, IUINT32 color, IUINT32 bk, int add)
{
	char _buffer[IBITMAP_STACK_BUFFER];
	char *buffer = _buffer;
	long size = glyph->w * glyph->h * 16;
	if (size >= IBITMAP_STACK_BUFFER) {
		buffer = (char*)icmalloc(size);
		if (buffer == NULL) return;
	}

	ibitmap_draw_glyph_font(dst, glyph, clip, x, y, color, bk, add, buffer);

	if (buffer != _buffer) {
		icfree(buffer);
	}
}

void ibitmap_draw_ascii(IBITMAP *dst, const IGLYPHFONT *font, int x, int y,
	const char *string, const IRECT *clip, IUINT32 col, IUINT32 bk, int add)
{
	char _buffer[IBITMAP_STACK_BUFFER];
	char *buffer = _buffer;
	long size = font->w * font->h * 16;
	long unit = ((font->w + 7) / 8) * font->h;
	IGLYPHCHAR *glyph;
	IRECT rect;
	int left, i, p;

	if (size + unit + sizeof(IGLYPHCHAR) >= IBITMAP_STACK_BUFFER) {
		buffer = (char*)icmalloc(size + unit + sizeof(IGLYPHCHAR));
		if (buffer == NULL) return;
	}

	glyph = (IGLYPHCHAR*)(buffer + size);

	if (clip == NULL) {
		clip = &rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = (int)dst->w;
		rect.bottom = (int)dst->h;
	}

	glyph->w = font->w;
	glyph->h = font->h;

	for (i = 0, left = x, p = 0; string[i]; i++) {
		int ch = (unsigned char)string[i];
		if (ch == '\n') {
			x = left;
			y += font->h;
			p = 0;
		}
		else if (ch == '\r') {
			x = left;
			p = 0;
		}
		else if (ch == '\t') {
			int d = 8 - (p & 7);
			x += font->w * d;
			p += d;
		}
		else if (ch == ' ') {
			p += 1;
			x += font->w;
		}
		else {
			memcpy(glyph->data, &font->font[unit * ch], unit);
			ibitmap_draw_glyph_font(dst, glyph, clip, x, y, col, bk,
				add, buffer);
			x += font->w;
			p++;
		}
	}

	if (buffer != _buffer) {
		icfree(buffer);
	}
}


//=====================================================================
// 平滑缩放
//=====================================================================
static int ifilter_shrink_x_c(IUINT8 *dstpix, IUINT8 *srcpix, int height,
	long dstpitch, long srcpitch, int dstwidth, int srcwidth)
{
	IINT32 srcdiff = srcpitch - (srcwidth * 4);
	IINT32 dstdiff = dstpitch - (dstwidth * 4);
	IINT32 x, y;
	IINT32 xspace = 0x10000 * srcwidth / dstwidth;
	IINT32 xrecip = 0;
	IINT64 zrecip = 1;

	zrecip <<= 32;
	xrecip = (int)(zrecip / xspace);

	for (y = 0; y < height; y++) {
		IUINT16 accumulate[4] = { 0, 0, 0, 0 };
		int xcounter = xspace;
		for (x = 0; x < srcwidth; x++) {
			if (xcounter > 0x10000) {
				accumulate[0] += (IUINT16) *srcpix++;
				accumulate[1] += (IUINT16) *srcpix++;
				accumulate[2] += (IUINT16) *srcpix++;
				accumulate[3] += (IUINT16) *srcpix++;
				xcounter -= 0x10000;
			}	else {
				int xfrac = 0x10000 - xcounter;
				#define ismooth_putpix_x(n) { \
						*dstpix++ = (IUINT8)(((accumulate[n] + ((srcpix[n] \
							* xcounter) >> 16)) * xrecip) >> 16); \
					}
				ismooth_putpix_x(0);
				ismooth_putpix_x(1);
				ismooth_putpix_x(2);
				ismooth_putpix_x(3);
				#undef ismooth_putpix_x
				accumulate[0] = (IUINT16)((*srcpix++ * xfrac) >> 16);
				accumulate[1] = (IUINT16)((*srcpix++ * xfrac) >> 16);
				accumulate[2] = (IUINT16)((*srcpix++ * xfrac) >> 16);
				accumulate[3] = (IUINT16)((*srcpix++ * xfrac) >> 16);
				xcounter = xspace - xfrac;
			}
		}
		srcpix += srcdiff;
		dstpix += dstdiff;
	}
	return 0;
}

static int ifilter_shrink_y_c(IUINT8 *dstpix, IUINT8 *srcpix, int width, 
	long dstpitch, long srcpitch, int dstheight, int srcheight)
{
	IINT32 srcdiff = srcpitch - (width * 4);
	IINT32 dstdiff = dstpitch - (width * 4);
	IINT32 x, y;
	IINT32 yspace = 0x10000 * srcheight / dstheight;
	IINT32 yrecip = 0;
	IINT32 ycounter = yspace;
	IUINT16 *templine;

	IINT64 zrecip = 1;
	zrecip <<= 32;
	yrecip = (IINT32)(zrecip / yspace);

	templine = (IUINT16*)icmalloc(dstpitch * 2);
	assert(templine);

	for (y = 0; y < srcheight; y++) {
		IUINT16 *accumulate = templine;
		if (ycounter > 0x10000) {
			for (x = 0; x < width; x++) {
				*accumulate++ += (IUINT16) *srcpix++;
				*accumulate++ += (IUINT16) *srcpix++;
				*accumulate++ += (IUINT16) *srcpix++;
				*accumulate++ += (IUINT16) *srcpix++;
			}
			ycounter -= 0x10000;
		}	else {
			int yfrac = 0x10000 - ycounter;
			for (x = 0; x < width; x++) {
				#define ismooth_putpix_y() { \
					*dstpix++ = (IUINT8) (((*accumulate++ + ((*srcpix++ * \
						ycounter) >> 16)) * yrecip) >> 16); \
				}
				ismooth_putpix_y();
				ismooth_putpix_y();
				ismooth_putpix_y();
				ismooth_putpix_y();
				#undef ismooth_putpix_y
			}
			dstpix += dstdiff;
			accumulate = templine;
			srcpix -= 4 * width;
			for (x = 0; x < width; x++) {
				*accumulate++ = (IUINT16) ((*srcpix++ * yfrac) >> 16);
				*accumulate++ = (IUINT16) ((*srcpix++ * yfrac) >> 16);
				*accumulate++ = (IUINT16) ((*srcpix++ * yfrac) >> 16);
				*accumulate++ = (IUINT16) ((*srcpix++ * yfrac) >> 16);
			}
			ycounter = yspace - yfrac;
		}
		srcpix += srcdiff;
	}

	icfree(templine);
	return 0;
}

static int ifilter_expand_x_c(IUINT8 *dstpix, IUINT8 *srcpix, int height,
	long dstpitch, long srcpitch, int dstwidth, int srcwidth)
{
	IINT32 dstdiff = dstpitch - (dstwidth * 4);
	IINT32 *xidx0, *xmult0, *xmult1;
	IINT32 x, y;
	IINT32 factorwidth = 4;

	xidx0 = (IINT32*)icmalloc(dstwidth * 4);
	if (xidx0 == NULL) return -1;
	xmult0 = (IINT32*)icmalloc(dstwidth * factorwidth);
	xmult1 = (IINT32*)icmalloc(dstwidth * factorwidth);

	if (xmult0 == NULL || xmult1 == NULL) {
		icfree(xidx0);
		if (xmult0) icfree(xmult0);
		if (xmult1) icfree(xmult1);
		return -1;
	}

	for (x = 0; x < dstwidth; x++) {
		xidx0[x] = x * (srcwidth - 1) / dstwidth;
		xmult1[x] = 0x10000 * ((x * (srcwidth - 1)) % dstwidth) / dstwidth;
		xmult0[x] = 0x10000 - xmult1[x];
	}

	for (y = 0; y < height; y++) {
		IUINT8 *srcrow0 = srcpix + y * srcpitch;
		for (x = 0; x < dstwidth; x++) {
			IUINT8 *src = srcrow0 + xidx0[x] * 4;
			IINT32 xm0 = xmult0[x];
			IINT32 xm1 = xmult1[x];
			*dstpix++ = (IUINT8)(((src[0] * xm0) + (src[4] * xm1)) >> 16);
			*dstpix++ = (IUINT8)(((src[1] * xm0) + (src[5] * xm1)) >> 16);
			*dstpix++ = (IUINT8)(((src[2] * xm0) + (src[6] * xm1)) >> 16);
			*dstpix++ = (IUINT8)(((src[3] * xm0) + (src[7] * xm1)) >> 16);
		}
		dstpix += dstdiff;
	}

	icfree(xidx0);
	icfree(xmult0);
	icfree(xmult1);
	return 0;
}

static int ifilter_expand_y_c(IUINT8 *dstpix, IUINT8 *srcpix, int width, 
	long dstpitch, long srcpitch, int dstheight, int srcheight)
{
	IINT32 x, y;
	for (y = 0; y < dstheight; y++) {
		int yidx0 = y * (srcheight - 1) / dstheight;
		IUINT8 *s0 = srcpix + yidx0 * srcpitch;
		IUINT8 *s1 = s0 + srcpitch;
		int ym1 = 0x10000 * ((y * (srcheight - 1)) % dstheight) / dstheight;
		int ym0 = 0x10000 - ym1;
		for (x = 0; x < width; x++) {
			*dstpix++ = (IUINT8)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
			*dstpix++ = (IUINT8)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
			*dstpix++ = (IUINT8)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
			*dstpix++ = (IUINT8)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
		}
		dstpix += dstpitch - 4 * width;
	}

	return 0;
}


// 平滑缩放
static int ipixel_smooth_resize(IUINT8 *dstpix, IUINT8 *srcpix, int dstwidth,
	int srcwidth, int dstheight, int srcheight, long dstpitch, long srcpitch)
{
	IUINT8 *temp = NULL;

	if (srcwidth == dstwidth && srcheight == dstheight) {
		long size, y;
		for (y = 0, size = srcwidth * 4; y < dstheight; y++) {
			memcpy(dstpix + y * dstpitch, srcpix + y * srcpitch, size);
		}
		return 0;
	}

	temp = (IUINT8*)icmalloc((long)srcwidth * dstheight * 4);

	if (temp == NULL) return -1;

	if (dstheight < srcheight) {
		if (ifilter_shrink_y_c(temp, srcpix, srcwidth, srcwidth * 4, 
			srcpitch, dstheight, srcheight) != 0) {
			icfree(temp);
			return -2;
		}
	}
	else if (dstheight > srcheight) {
		if (ifilter_expand_y_c(temp, srcpix, srcwidth, srcwidth * 4,
			srcpitch, dstheight, srcheight) != 0) {
			icfree(temp);
			return -3;
		}
	}
	else {
		long size, y;
		for (y = 0, size = srcwidth * 4; y < dstheight; y++) {
			memcpy(temp + y * size, srcpix + y * srcpitch, size);
		}
	}

	if (dstwidth < srcwidth) {
		if (ifilter_shrink_x_c(dstpix, temp, dstheight, dstpitch, 
			srcwidth * 4, dstwidth, srcwidth) != 0) {
			icfree(temp);
			return -4;
		}
	}
	else if (dstwidth > srcwidth) {
		if (ifilter_expand_x_c(dstpix, temp, dstheight, dstpitch, 
			srcwidth * 4, dstwidth, srcwidth) != 0) {
			icfree(temp);
			return -5;
		}
	}
	else {
		long size, y;
		for (y = 0, size = dstwidth * 4; y < dstheight; y++) {
			memcpy(dstpix + y * dstpitch, temp + y * size, size);
		}
	}

	icfree(temp);

	return 0;
}


// 平滑过渡：无裁剪
int ibitmap_smooth_resize(IBITMAP *dst, const IRECT *rectdst, 
	const IBITMAP *src, const IRECT *rectsrc)
{
	unsigned char *ss;
	unsigned char *dd;
	int dstwidth, dstheight;
	int srcwidth, srcheight;
	int sfmt, dfmt;
	int retval;
	IBITMAP *tmp1, *tmp2;

	sfmt = ibitmap_pixfmt_guess(src);
	dfmt = ibitmap_pixfmt_guess(dst);

	dstwidth = rectdst->right - rectdst->left;
	srcwidth = rectsrc->right - rectsrc->left;
	dstheight = rectdst->bottom - rectdst->top;
	srcheight = rectsrc->bottom - rectsrc->top;

	#define _ilineptr(x, y) ((IUINT8*)((x)->line[y]))

	if (src->bpp == 32 && dst->bpp == 32 && sfmt == dfmt) {
		dd = _ilineptr(dst, rectdst->top) + rectdst->left * 4;
		ss = _ilineptr(src, rectsrc->top) + rectsrc->left * 4;
		retval = ipixel_smooth_resize(dd, ss, dstwidth, srcwidth, dstheight,
			srcheight, dst->pitch, src->pitch);
		return retval;
	}

	if (src->bpp == 32) {
		tmp1 = ibitmap_create(dstwidth, dstheight, 32);
		if (tmp1 == 0) return -20;
		ibitmap_pixfmt_set(tmp1, sfmt);
		dd = _ilineptr(tmp1, 0);
		ss = _ilineptr(src, rectsrc->top) + rectsrc->left * 4;
		retval = ipixel_smooth_resize(dd, ss, dstwidth, srcwidth, 
			dstheight, srcheight, tmp1->pitch, src->pitch);
		if (retval == 0) {
			ibitmap_convert(dst,  rectdst->left, rectdst->top, tmp1, 
				0, 0, dstwidth, dstheight, NULL, 0);
		}
		ibitmap_release(tmp1);
		return 0;
	}

	if (dst->bpp == 32) {
		tmp1 = ibitmap_create(srcwidth, srcheight, 32);
		if (tmp1 == NULL) return -30;
		ibitmap_pixfmt_set(tmp1, dfmt);
		ibitmap_convert(tmp1, 0, 0, src, rectsrc->left, rectsrc->top,
			srcwidth, srcheight, NULL, 0);
		dd = _ilineptr(dst, rectdst->top) + rectdst->left * 4;
		ss = _ilineptr(tmp1, 0);
		retval = ipixel_smooth_resize(dd, ss, dstwidth, srcwidth,
			dstheight, srcheight, dst->pitch, tmp1->pitch);
		ibitmap_release(tmp1);
		return retval;
	}

	tmp1 = ibitmap_create(dstwidth, dstheight, 32);
	tmp2 = ibitmap_create(srcwidth, srcheight, 32);

	if (tmp1 == NULL || tmp2 == NULL) {
		if (tmp1) ibitmap_release(tmp1);
		if (tmp2) ibitmap_release(tmp2);
		return -40;
	}

	ibitmap_pixfmt_set(tmp1, IPIX_FMT_A8R8G8B8);
	ibitmap_pixfmt_set(tmp2, IPIX_FMT_A8R8G8B8);

	ibitmap_convert(tmp2, 0, 0, src, rectsrc->left, rectsrc->top,
			srcwidth, srcheight, NULL, 0);

	dd = _ilineptr(tmp1, 0);
	ss = _ilineptr(tmp2, 0);

	retval = ipixel_smooth_resize(dd, ss, dstwidth, srcwidth,
		dstheight, srcheight, tmp1->pitch, tmp2->pitch);

	if (retval == 0) {
		ibitmap_convert(dst, rectdst->left, rectdst->top, tmp1, 
			0, 0, dstwidth, dstheight, NULL, 0);
	}

	ibitmap_release(tmp1);
	ibitmap_release(tmp2);

	return retval;
}



//=====================================================================
// 位图实用工具
//=====================================================================
int ibitmap_clip_scale(const IBITMAP *dst, IRECT *drect, const IRECT *clip,
	const IBITMAP *src, IRECT *srect)
{
	int x1 = drect->left;
	int y1 = drect->top;
	int w1 = drect->right - x1;
	int h1 = drect->bottom - y1;
	int x2 = srect->left;
	int y2 = srect->top;
	int w2 = srect->right - x2;
	int h2 = srect->bottom - y2;
	int cl, ct, cr, cb;
	int cw, ch;
	if (clip == NULL) {
		cl = 0, ct = 0, cr = dst->w, cb = dst->h;
	}	else {
		cl = clip->left;
		ct = clip->top;
		cr = clip->right;
		cb = clip->bottom;
	}
	cw = cr - cl;
	ch = cb - ct;
	if (w1 <= 0 || h1 <= 0 || w2 <= 0 || h2 <= 0) return -1;
	if (x1 >= cl && y1 >= ct && w1 <= cw && h1 <= ch &&
		x2 >= 0 && y2 >= 0 && w2 <= (int)src->w && h2 <= (int)src->h) {
		return 0;
	}	else {
		float fx = ((float)w2) / w1;
		float fy = ((float)h2) / h1;
		float ix = 1.0f / fx;
		float iy = 1.0f / fy;
		int ds, d;
		if (x1 < cl) {
			d = cl - x1;
			x2 += (int)(d * fx);
			w2 -= (int)(d * fx);
			w1 -= d;
			x1 = cl;
		}
		if (y1 < ct) {
			d = ct - y1;
			y2 += (int)(d * fx);
			h2 -= (int)(d * fx);
			h1 -= (int)d;
			y1 = (int)ct;
		}
		if (x2 < 0) 
			x1 += (int)(-x2 * ix), w1 += (int)(x2 * ix), w2 += x2, x2 = 0;
		if (y2 < 0) 
			y1 += (int)(-y2 * iy), h2 += (int)(y2 * iy), h2 += y2, y2 = 0;
		if (x1 >= cr || y1 >= cb) return -2;
		if (x2 >= (int)src->w || y2 >= (int)src->h) return -3;
		if (w1 <= 0 || h1 <= 0 || w2 <= 0 || h2 <= 0) return -4;
		if (x1 + w1 > cr) 
			ds = x1 + w1 - cr, w1 -= ds, w2 -= (int)(ds * fx);
		if (y1 + h1 > cb)
			ds = y1 + h1 - cb, h1 -= ds, h2 -= (int)(ds * fx);
		if (x2 + w2 > (int)src->w)
			ds = x2 + w2 - (int)src->w, w1 -= (int)(ds * ix), w2 -= ds;
		if (y2 + h2 > (int)src->h)
			ds = y2 + h2 - (int)src->h, h1 -= (int)(ds * iy), h2 -= ds;
		if (w1 <= 0 || h1 <= 0 || w2 <= 0 || h2 <= 0) 
			return -5;
	}
	ipixel_rect_set(drect, x1, y1, x1 + w1, y1 + h1);
	ipixel_rect_set(srect, x2, y2, x2 + w2, y2 + h2);
	return 0;
}


// 缩放绘制：
int ibitmap_stretch(IBITMAP *dst, const IRECT *rectdst, const IBITMAP *src, 
	const IRECT *rectsrc, const IRECT *clip, int flags)
{
	IINT32 dx, dy, dw, dh, sx, sy, sw, sh;
	IRECT drect;
	IRECT srect;

	ipixel_rect_copy(&drect, rectdst);
	ipixel_rect_copy(&srect, rectsrc);

	if (src->bpp != dst->bpp) 
		return -100;

	if (flags & (IBLIT_HFLIP | IBLIT_VFLIP))
		return -200;

	if (ibitmap_clip_scale(dst, &drect, clip, src, &srect) != 0) 
		return -300;

	dx = drect.left;
	dy = drect.top;
	dw = drect.right - dx;
	dh = drect.bottom - dy;
	sx = srect.left;
	sy = srect.top;
	sw = srect.right - sx;
	sh = srect.bottom - sy;

	#define ibitmap_stretch_routine(nbytes, bpp) {	\
			unsigned char *dstrow = (IUINT8*)dst->line[dy] + dx * nbytes; \
			unsigned char *srcrow = (IUINT8*)src->line[sy] + sx * nbytes; \
			long srcpitch = src->pitch;	\
			long dstpitch = dst->pitch; \
			int dstwidth = dw; \
			int dstheight = dh; \
			int dstwidth2 = dw * 2; \
			int dstheight2 = dh * 2; \
			int srcwidth2 = sw * 2; \
			int srcheight2 = sh * 2; \
			int werr = 0; \
			int herr = srcheight2 - dstheight2; \
			int loopw = 0; \
			int looph = 0; \
			IUINT32 mask, c; \
			mask = src->mask; \
			for (looph = 0; looph < dstheight; looph++) { \
				unsigned char *dstpix = dstrow; \
				unsigned char *srcpix = srcrow; \
				werr = srcwidth2 - dstwidth2; \
				if ((flags & IBLIT_MASK) == 0) { \
					for (loopw = dstwidth; loopw > 0; loopw--) { \
						c = _ipixel_fetch(bpp, srcpix, 0); \
						_ipixel_store(bpp, dstpix, 0, c); \
						dstpix += nbytes; \
						while (werr >= 0) { \
							srcpix += nbytes, werr -= dstwidth2; \
						}	\
						werr += srcwidth2; \
					} \
				}	else { \
					for (loopw = dstwidth; loopw > 0; loopw--) { \
						c = _ipixel_fetch(bpp, srcpix, 0); \
						if (c != mask) { \
							_ipixel_store(bpp, dstpix, 0, c); \
						}	\
						dstpix += nbytes; \
						while (werr >= 0) { \
							srcpix += nbytes, werr -= dstwidth2; \
						}	\
						werr += srcwidth2; \
					} \
				}	\
				while (herr >= 0) { \
					srcrow += srcpitch, herr -= dstheight2; \
				}	\
				dstrow += dstpitch; \
				herr += srcheight2; \
			}	\
		}

	switch (src->bpp) 
	{
	case 8: 
		ibitmap_stretch_routine(1, 8); 
		break;
	case 15:
	case 16:
		ibitmap_stretch_routine(2, 16);
		break;
	case 24:
		ibitmap_stretch_routine(3, 24);
		break;
	case 32:
		ibitmap_stretch_routine(4, 32); 
		break;
	}

	#undef ibitmap_stretch_routine

	return 0;
}


// 重新采样
IBITMAP *ibitmap_resample(const IBITMAP *src, const IRECT *bound, 
	int newwidth, int newheight, int mode)
{
	IBITMAP *bitmap;
	IRECT srect;
	IRECT drect;

	bitmap = ibitmap_create(newwidth, newheight, src->bpp);

	if (bitmap == NULL)
		return NULL;

	ibitmap_pixfmt_set(bitmap, ibitmap_pixfmt_guess(src));

	if (bound == NULL) {
		bound = &srect;
		srect.left = 0;
		srect.top = 0;
		srect.right = (int)src->w;
		srect.bottom = (int)src->h;
	}

	drect.left = 0;
	drect.top = 0;
	drect.right = newwidth;
	drect.bottom = newheight;

	if (mode == 0) {
		ibitmap_smooth_resize(bitmap, &drect, src, &srect);
	}	
	else {
		ibitmap_stretch(bitmap, &drect, src, &srect, NULL, 0);
	}

	return bitmap;
}


//! flag: -O3

// ibmtrap.h
