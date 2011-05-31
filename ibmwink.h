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


//---------------------------------------------------------------------
// ������Ч
//---------------------------------------------------------------------
IBITMAP *ibitmap_effect_drop_shadow(const IBITMAP *src, int dir, int level);

void ibitmap_stackblur(IBITMAP *src, int rx, int ry, const IRECT *bound);


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
	int ntraps, IBITMAP *alpha, IUINT32 color, int isadditive, 
	const IRECT *clip, struct CVECTOR *scratch);

// ���ƶ����
int ipixel_render_polygon(IBITMAP *dst, const ipixel_point_fixed_t *pts,
	int npts, IBITMAP *alpha, IUINT32 color, int isadditive,
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
	cvector_t scratch;
	cvector_t points;
	cvector_t pointf;
};


//---------------------------------------------------------------------
// ��ͼ�ӿ�
//---------------------------------------------------------------------
ipaint_t *ipaint_create(IBITMAP *image);

void ipaint_destroy(ipaint_t *paint);

int ipaint_set_image(ipaint_t *paint, IBITMAP *image);


void ipaint_set_color(ipaint_t *paint, IUINT32 color);

void ipaint_set_clip(ipaint_t *paint, const IRECT *clip);

void ipaint_text_color(ipaint_t *paint, IUINT32 color);

void ipaint_text_background(ipaint_t *paint, IUINT32 color);

void ipaint_anti_aliasing(ipaint_t *paint, int level);


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


#ifdef __cplusplus
}
#endif

#endif


