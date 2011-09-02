//=====================================================================
//
// ibmwink.h - animation
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================
#ifndef __IBMWINK_H__
#define __IBMWINK_H__

#include "ibitmap.h"
#include "ibmbits.h"
#include "ibmcols.h"
#include "ibmdata.h"


#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------
// �˲�����ͨ������
//---------------------------------------------------------------------

// ʹ���˲���
// filter��һ����Ϊ9�����飬�ú�������ÿ������Ϊ����3x3��9�����еĸ���
// ��������filter�ж�Ӧ��ֵ����ӣ��ٳ���256����Ϊ�õ����ɫ����
int ibitmap_filter(IBITMAP *dst, const short *filter);


// ȡ��channel
// dst������8λ��λͼ��filter��0,1,2,3����ȡ��src�е�r,g,b,a����
int ibitmap_channel_get(IBITMAP *dst, int dx, int dy, const IBITMAP *src,
	int sx, int sy, int sw, int sh, int channel);

// ����channel
// src������8λ��λͼ��filter��0,1,2,3��������dst�е�r,g,b,a����
int ibitmap_channel_set(IBITMAP *dst, int dx, int dy, const IBITMAP *src,
	int sx, int sy, int sw, int sh, int channel);


// ͼ����º�����
// ���أ�����0����ɹ���1����д�أ�С����Ϊ����
typedef int (*iBitmapUpdate)(int x, int y, int w, IUINT32 *card, void *user);


// ͼ����£����մ��ϵ���˳��ÿ��ɨ���ߵ���һ��updater
int ibitmap_update(IBITMAP *dst, const IRECT *bound, 
	iBitmapUpdate updater, int readonly, void *user);


//---------------------------------------------------------------------
// ������Ч
//---------------------------------------------------------------------

// ͼ��ģ��
void ibitmap_stackblur(IBITMAP *src, int rx, int ry, const IRECT *bound);

// ��AA����ֱ��
int ibitmap_put_line(IBITMAP *dst, int x1, int y1, int x2, int y2,
	IUINT32 color, int additive, const IRECT *clip);

// ��AA����Բ��
void ibitmap_put_circle(IBITMAP *dst, int x0, int y0, int r, int fill,
	const IRECT *clip, IUINT32 color, int additive);

// ɫ�ʱ�ã����� 5 x 5����
void ibitmap_color_transform(IBITMAP *dst, const IRECT *b, const float *t);

// ɫ�ʱ�ã��ӷ�
void ibitmap_color_add(IBITMAP *dst, const IRECT *b, IUINT32 color);

// ɫ�ʱ�ã�����
void ibitmap_color_sub(IBITMAP *dst, const IRECT *b, IUINT32 color);

// ɫ�ʱ�ã��˷�
void ibitmap_color_mul(IBITMAP *dst, const IRECT *b, IUINT32 color);


// ������Ӱ
IBITMAP *ibitmap_drop_shadow(const IBITMAP *src, int rx, int ry);

// ����Բ��
IBITMAP *ibitmap_round_rect(const IBITMAP *src, int radius, int style);



//---------------------------------------------------------------------
// �������
//---------------------------------------------------------------------
struct CVECTOR
{
	unsigned char *data;
	size_t size;
	size_t block;
};

// ��ʼ������
void cvector_init(struct CVECTOR *vector);

// ���ٻ���
void cvector_destroy(struct CVECTOR *vector);

// �ı仺���С
int cvector_resize(struct CVECTOR *vector, size_t size);

// ������ݵ�ĩβ
int cvector_push(struct CVECTOR *vector, const void *data, size_t size);


//---------------------------------------------------------------------
// ԭʼ��ͼ
//---------------------------------------------------------------------

// �����������
int ipixel_render_traps(IBITMAP *dst, const ipixel_trapezoid_t *traps, 
	int ntraps, IBITMAP *alpha, const ipixel_source_t *src, int isadditive,
	const IRECT *clip, struct CVECTOR *scratch);

// ���ƶ����
int ipixel_render_polygon(IBITMAP *dst, const ipixel_point_fixed_t *pts,
	int npts, IBITMAP *alpha, const ipixel_source_t *src, int isadditive,
	const IRECT *clip, struct CVECTOR *scratch);



//---------------------------------------------------------------------
// ��ͼ����
//---------------------------------------------------------------------
typedef struct CVECTOR cvector_t;
typedef struct IPAINT  ipaint_t;

// ���ƵĽṹ��
struct IPAINT
{
	IBITMAP *image;
	IBITMAP *alpha;
	IRECT clip;
	IUINT32 color;
	IUINT32 text_color;
	IUINT32 text_backgrnd;
	int additive;
	double line_width;
	ipixel_point_t *pts;
	int npts;
	ipixel_source_t *current;
	ipixel_source_t source;
	cvector_t scratch;
	cvector_t points;
	cvector_t pointf;
	cvector_t gradient;
};


//---------------------------------------------------------------------
// ��ͼ�ӿ�
//---------------------------------------------------------------------

// ����ͼ�ζ���
ipaint_t *ipaint_create(IBITMAP *image);

// ����ͼ�ζ���
void ipaint_destroy(ipaint_t *paint);

// ����Ŀ��λͼ
int ipaint_set_image(ipaint_t *paint, IBITMAP *image);


// ɫ��Դ�����õ�ǰɫ��Դ
void ipaint_source_set(ipaint_t *paint, ipixel_source_t *source);

// ���õ�ǰ��ɫ
void ipaint_set_color(ipaint_t *paint, IUINT32 color);

// ���òü�����
void ipaint_set_clip(ipaint_t *paint, const IRECT *clip);

// ����������ɫ
void ipaint_text_color(ipaint_t *paint, IUINT32 color);

// �������屳��
void ipaint_text_background(ipaint_t *paint, IUINT32 color);

// ���ÿ����
void ipaint_anti_aliasing(ipaint_t *paint, int level);


// ���ƶ����
int ipaint_draw_polygon(ipaint_t *paint, const ipixel_point_t *pts, int n);

int ipaint_draw_line(ipaint_t *paint, double x1, double y1, double x2, 
	double y2);

void ipaint_line_width(ipaint_t *paint, double width);

int ipaint_draw_circle(ipaint_t *paint, double x, double y, double r);

int ipaint_draw_ellipse(ipaint_t *paint, double x, double y, double rx,
	double ry);

void ipaint_fill(ipaint_t *paint, const IRECT *rect, IUINT32 color);

void ipaint_cprintf(ipaint_t *paint, int x, int y, const char *fmt, ...);

void ipaint_sprintf(ipaint_t *paint, int x, int y, const char *fmt, ...);

int ipaint_raster(ipaint_t *paint, const ipixel_point_t *pts, 
	const IBITMAP *image, const IRECT *rect, IUINT32 color, int flag);

int ipaint_raster_draw(ipaint_t *paint, double x, double y, 
	const IBITMAP *src, const IRECT *rect, double off_x, double off_y, 
	double scale_x, double scale_y, double angle, IUINT32 color);

int ipaint_raster_draw_3d(ipaint_t *paint, double x, double y, double z, 
	const IBITMAP *src, const IRECT *rect, double off_x, double off_y,
	double scale_x, double scale_y, double angle_x, double angle_y,
	double angle_z, IUINT32 color);

int ipaint_draw(ipaint_t *paint, int x, int y, const IBITMAP *src, 
	const IRECT *bound, IUINT32 color, int flags);



//---------------------------------------------------------------------
// ɫ�ʿռ�
//---------------------------------------------------------------------
typedef struct IYCRCB
{
	unsigned char Y, Cr, Cb;
	unsigned char reserved;
}	IYCrCb;

typedef struct IHSV
{
	float H, S, V;
	unsigned char reserved;
}	IHSV;


void iconv_RGB_to_YCrCb(const IRGB *input, long size, IYCrCb *output);

void iconv_YCrCb_to_RGB(const IYCrCb *input, long size, IRGB *output);

void iconv_RGB_to_HSV(const IRGB *input, long size, IHSV *output);

void iconv_HSV_to_RGB(const IHSV *input, long size, IRGB *output);




//---------------------------------------------------------------------
// ��Ч����
//---------------------------------------------------------------------

// ������Ч
IBITMAP *ibitmap_glossy_make(IBITMAP *bmp, int radius, int border, int light,
	int shadow, int shadow_pos);

// ����ɫ��
void ibitmap_adjust_hsv(IBITMAP *bmp, float hue, float saturation, 
	float value, const IRECT *bound);

// ����ɫ��
void ibitmap_adjust_hsl(IBITMAP *bmp, float hue, float saturation, 
	float lightness, const IRECT *bound);

// Android: patch 9 ����
IBITMAP *ibitmap_patch_nine(const IBITMAP *src, int nw, int nh, int *code);

// Android: ȡ�ÿͻ���
int ibitmap_patch_client(const IBITMAP *src, IRECT *client);


#ifdef __cplusplus
}
#endif

#endif


