//=====================================================================
//
// ibmdata.h - ibitmap raster code
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================
#ifndef __IBMDATA_H__
#define __IBMDATA_H__

#include "ibitmap.h"
#include "ibmbits.h"
#include "ibmcols.h"


//======================================================================
// ���β���
//======================================================================
typedef struct ipixel_point_fixed ipixel_point_fixed_t;
typedef struct ipixel_line_fixed  ipixel_line_fixed_t;
typedef struct ipixel_vector      ipixel_vector_t;
typedef struct ipixel_transform   ipixel_transform_t;
typedef struct ipixel_point       ipixel_point_t;
typedef struct ipixel_matrix      ipixel_matrix_t;

// ��Ķ���
struct ipixel_point_fixed
{
	cfixed x;
	cfixed y;
};

// ֱ�߶���
struct ipixel_line_fixed
{
	ipixel_point_fixed_t p1;
	ipixel_point_fixed_t p2;
};

// ʸ������
struct ipixel_vector
{
	cfixed vector[3];
};

// ������
struct ipixel_transform
{
	cfixed matrix[3][3];
};

// ��������
struct ipixel_point
{
	double x;
	double y;
};

// �������
struct ipixel_matrix
{
	double m[3][3];
};


//---------------------------------------------------------------------
// �������
//---------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// ����ͬ�����: matrix * vector
int ipixel_transform_point(const ipixel_transform_t *matrix,
						     struct ipixel_vector *vector);

// ��λ�ʸ��: [x/w, y/w, 1]
int ipixel_transform_homogeneous(struct ipixel_vector *vector);

// ������ˣ�dst=l*r
int ipixel_transform_multiply(ipixel_transform_t *dst, 
							const ipixel_transform_t *l,
							const ipixel_transform_t *r);

// ��ʼ����λ����
int ipixel_transform_init_identity(ipixel_transform_t *matrix);

// ��ʼ��λ�ƾ���
int ipixel_transform_init_translate(ipixel_transform_t *matrix, 
							cfixed x,
							cfixed y);

// ��ʼ����ת����
int ipixel_transform_init_rotate(ipixel_transform_t *matrix,
							cfixed cos,
							cfixed sin);

// ��ʼ�����ž���
int ipixel_transform_init_scale(ipixel_transform_t *matrix,
							cfixed sx,
							cfixed sy);

// ��ʼ��͸�Ӿ���
int ipixel_transform_init_perspective(ipixel_transform_t *matrix,
							const struct ipixel_point_fixed *src,
							const struct ipixel_point_fixed *dst);

// ��ʼ���������
int ipixel_transform_init_affine(ipixel_transform_t *matrix,
							const struct ipixel_point_fixed *src,
							const struct ipixel_point_fixed *dst);


// ����Ƿ��ǵ�λ���󣺳ɹ����ط��㣬����Ϊ��
int ipixel_transform_is_identity(const ipixel_transform_t *matrix);

// ����Ƿ������ž��󣺳ɹ����ط��㣬����Ϊ��
int ipixel_transform_is_scale(const ipixel_transform_t *matrix);

// ����Ƿ�������ƽ�ƾ��󣺳ɹ����ط��㣬����Ϊ��
int ipixel_transform_is_int_translate(const ipixel_transform_t *matrix);

// ����Ƿ�Ϊƽ��+����
int ipixel_transform_is_scale_translate(const ipixel_transform_t *matrix);

// ����Ƿ�Ϊ��ͨƽ�ƾ���
int ipixel_transform_is_translate(const ipixel_transform_t *matrix);


// ���������󵽶���������
int ipixel_transform_from_matrix(ipixel_transform_t *t, 
	const ipixel_matrix_t *m);

// ���������󵽸���������
int ipixel_transform_to_matrix(const ipixel_transform_t *t, 
	ipixel_matrix_t *m);

// �������������
int ipixel_matrix_invert(ipixel_matrix_t *dst, const ipixel_matrix_t *src);

// ��������������
int ipixel_transform_invert(ipixel_transform_t *dst, 
	const ipixel_transform_t *src);

// ����������˷�
int ipixel_matrix_point(const ipixel_matrix_t *matrix, double *vec);

// ��ʼ����λ����
int ipixel_matrix_init_identity(ipixel_matrix_t *matrix);

// ��ʼ��λ�ƾ���
int ipixel_matrix_init_translate(ipixel_matrix_t *matrix, 
							double x,
							double y);

// ��ʼ����ת����
int ipixel_matrix_init_rotate(ipixel_matrix_t *matrix,
							double cos,
							double sin);

// ��ʼ�����ž���
int ipixel_matrix_init_scale(ipixel_matrix_t *matrix,
							double sx,
							double sy);

#ifdef __cplusplus
}
#endif


//=====================================================================
// ��դ������
//=====================================================================
typedef struct ipixel_trapezoid   ipixel_trapezoid_t;
typedef struct ipixel_span        ipixel_span_t;

// ���ζ���
struct ipixel_trapezoid
{
	cfixed top, bottom;
	ipixel_line_fixed_t left, right;
};

// ɨ���߶���
struct ipixel_span
{
	int x, y, w;
};


//---------------------------------------------------------------------
// ��դ������
//---------------------------------------------------------------------
#define ipixel_trapezoid_valid(t) \
		((t)->left.p1.y != (t)->left.p2.y && \
		 (t)->right.p1.y != (t)->right.p2.y && \
		 (int)((t)->bottom - (t)->top) > 0)


#define IPIXEL_SUBPIXEL_8		0
#define IPIXEL_SUBPIXEL_4		1
#define IPIXEL_SUBPIXEL_1		2


#ifdef __cplusplus
extern "C" {
#endif

// ��դ������
void ipixel_raster_trapezoid(IBITMAP *image, const ipixel_trapezoid_t *trap,
		int x_off, int y_off, const IRECT *clip);

// ��դ��ɨ����
int ipixel_raster_spans(ipixel_span_t *spans, const ipixel_trapezoid_t *trap,
		int subpixel, int x_off, int y_off, const IRECT *clip);

// ��դ��������
void ipixel_raster_triangle(IBITMAP *image, const ipixel_point_fixed_t *p1,
		const ipixel_point_fixed_t *p2, const ipixel_point_fixed_t *p3, 
		int x_off, int y_off, const IRECT *clip);

// �������ι�դ��
void ipixel_raster_traps(IBITMAP *image, const ipixel_trapezoid_t *traps,
		int count, int x_off, int y_off, const IRECT *clip);


//---------------------------------------------------------------------
// ���λ���
//---------------------------------------------------------------------

// �߶���y���ཻ��x���꣬ceilΪ�Ƿ�����ȥ��
static inline cfixed ipixel_line_fixed_x(const ipixel_line_fixed_t *l,
						cfixed y, int ceil)
{
	cfixed dx = l->p2.x - l->p1.x;
	IINT64 ex = ((IINT64)(y - l->p1.y)) * dx;
	cfixed dy = l->p2.y - l->p1.y;
	if (ceil) ex += (dy - 1);
	return l->p1.x + (cfixed)(ex / dy);
}

// �����ڵ�ǰɨ���ߵ�X��ĸ�������
static inline int ipixel_trapezoid_span_bound(const ipixel_trapezoid_t *t, 
		int y, int *lx, int *rx)
{
	cfixed x1, x2, y1, y2;
	int yt, yb;
	if (!ipixel_trapezoid_valid(t)) return -1;
	yt = cfixed_to_int(t->top);
	yb = cfixed_to_int(cfixed_ceil(t->bottom));
	if (y < yt || y >= yb) return -2;
	y1 = cfixed_from_int(y);
	y2 = cfixed_from_int(y) + cfixed_const_1_m_e;
	x1 = ipixel_line_fixed_x(&t->left, y1, 0);
	x2 = ipixel_line_fixed_x(&t->left, y2, 0);
	*lx = cfixed_to_int((x1 < x2)? x1 : x2);
	x1 = cfixed_ceil(ipixel_line_fixed_x(&t->right, y1, 1));
	x2 = cfixed_ceil(ipixel_line_fixed_x(&t->right, y2, 1));
	*rx = cfixed_to_int((x1 > x2)? x1 : x2);
	return 0;
}

// ���εİ����򣬷��غϷ����εĸ���
int ipixel_trapezoid_bound(const ipixel_trapezoid_t *t, int n, IRECT *rect);

// �ܶ������ڵ�ǰɨ���ߵ�X��ĸ�������
static inline int ipixel_trapezoid_line_bound(const ipixel_trapezoid_t *t,
	int n, int y, int *lx, int *rx)
{
	int xmin = 0x7fff, xmax = -0x7fff;
	int xl, xr, retval = -1;
	for (; n > 0; t++, n--) {
		if (ipixel_trapezoid_span_bound(t, y, &xl, &xr) == 0) {
			if (xl < xmin) xmin = xl;
			if (xr > xmax) xmax = xr;
			retval = 0;
		}
	}
	if (retval == 0) *lx = xmin, *rx = xmax;
	return retval;
}


// ȡ��traps��ɨ���ߣ���Ҫ����spans��bound�ĸߵ�������С��spans��ȥ
int ipixel_trapezoid_spans(const ipixel_trapezoid_t *t, int n, int subpixel,
	ipixel_span_t *spans, int x_off, int y_off, const IRECT *clip);



//---------------------------------------------------------------------
// ���λ���
//---------------------------------------------------------------------

// ��������ת��Ϊtrap������trap�θ�����0-2��
int ipixel_traps_from_triangle(ipixel_trapezoid_t *trap, 
	const ipixel_point_fixed_t *p1, const ipixel_point_fixed_t *p2, 
	const ipixel_point_fixed_t *p3);


// �����ת��Ϊtrap, ������trap�ĸ���, [0, n * 2] ��
// ��Ҫ�ṩ�����ڴ棬��СΪ sizeof(ipixel_point_fixed_t) * n
int ipixel_traps_from_polygon(ipixel_trapezoid_t *trap,
	const ipixel_point_fixed_t *pts, int n, int clockwise, void *workmem);


// ���׶����ת��Ϊtrap������Ҫ�����ṩ�ڴ棬��ջ�Ϸ����ˣ�������ͬ
int ipixel_traps_from_polygon_ex(ipixel_trapezoid_t *trap,
	const ipixel_point_fixed_t *pts, int n, int clockwise);


//---------------------------------------------------------------------
// ���ض�ȡ
//---------------------------------------------------------------------
int ipixel_span_fetch(const IBITMAP *image, int offset, int line,
	int width, IUINT32 *card, const ipixel_transform_t *t,
	iBitmapFetchProc proc, const IUINT8 *mask, const IRECT *clip);

iBitmapFetchProc ipixel_span_get_proc(const IBITMAP *image, 
	const ipixel_transform_t *t);



//---------------------------------------------------------------------
// λͼ�Ͳ�ι�դ����͸��/����任
//---------------------------------------------------------------------
#define IBITMAP_RASTER_FLAG_PERSPECTIVE		0	// ͸�ӱ任
#define IBITMAP_RASTER_FLAG_AFFINE			1	// ����任

#define IBITMAP_RASTER_FLAG_OVER			0	// ����OVER
#define IBITMAP_RASTER_FLAG_ADD				4	// ����ADD
#define IBITMAP_RASTER_FLAG_COPY			8	// ����COPY


// �Ͳ�ι�դ��λͼ
int ibitmap_raster_low(IBITMAP *dst, const ipixel_point_fixed_t *pts, 
	const IBITMAP *src, const IRECT *rect, IUINT32 color, int flags,
	const IRECT *clip, void *workmem);

// �Ͳ�ι�դ��������Ҫ�����ڴ棬��ջ�Ϸ�����
int ibitmap_raster_base(IBITMAP *dst, const ipixel_point_fixed_t *pts, 
	const IBITMAP *src, const IRECT *rect, IUINT32 color, int flags,
	const IRECT *clip);

// �Ͳ�ι�դ�����������
int ibitmap_raster_float(IBITMAP *dst, const ipixel_point_t *pts, 
	const IBITMAP *src, const IRECT *rect, IUINT32 color, int flags,
	const IRECT *clip);



//---------------------------------------------------------------------
// �߲�ι�դ��
//---------------------------------------------------------------------

// ��ת/���Ż���
int ibitmap_raster_draw(IBITMAP *dst, double x, double y, const IBITMAP *src,
	const IRECT *rect, double offset_x, double offset_y, double scale_x, 
	double scale_y, double angle, IUINT32 color, const IRECT *clip);

// ��ά��ת/���Ż���
int ibitmap_raster_draw_3d(IBITMAP *dst, double x, double y, double z,
	const IBITMAP *src, const IRECT *rect, double offset_x, double offset_y,
	double scale_x, double scale_y, double angle_x, double angle_y, 
	double angle_z, IUINT32 color, const IRECT *clip);



//=====================================================================
// ɫ���ݶȲ���
//=====================================================================
typedef struct ipixel_gradient          ipixel_gradient_t;
typedef struct ipixel_gradient_stop     ipixel_gradient_stop_t;
typedef struct ipixel_gradient_walker   ipixel_gradient_walker_t;


struct ipixel_gradient
{
	int overflow;
	IUINT32 transparent;
	int nstops;
	ipixel_gradient_stop_t *stops;
};

struct ipixel_gradient_stop
{
	cfixed x;
	IUINT32 color;
};

struct ipixel_gradient_walker
{
	IUINT32 left_ag;
	IUINT32 left_rb;
	IUINT32 right_ag;
	IUINT32 right_rb;
	IUINT32 left_c;
	IUINT32 right_c;
	IINT64 left_x;
	IINT64 right_x;
	IINT32 stepper;
	IUINT32 transparent;
	int nstops;
	ipixel_gradient_stop_t *stops;
	int overflow;
	int need_reset;
};


void ipixel_gradient_walker_init(ipixel_gradient_walker_t *walker, 
	const ipixel_gradient_t *gradient);

IUINT32 ipixel_gradient_walker_pixel(ipixel_gradient_walker_t *walker,
	IINT64 pos_fixed_48_16);


//---------------------------------------------------------------------
// ɫ��Դʵ��
//---------------------------------------------------------------------
typedef struct ipixel_gradient_linear   ipixel_gradient_linear_t;
typedef struct ipixel_gradient_radial   ipixel_gradient_radial_t;
typedef struct ipixel_gradient_conical  ipixel_gradient_conical_t;
typedef struct ipixel_solid_color       ipixel_solid_color_t;
typedef struct ipixel_source            ipixel_source_t;

// �����ݶ�
struct ipixel_gradient_linear
{
	ipixel_gradient_t gradient;
	ipixel_point_fixed_t p1;
	ipixel_point_fixed_t p2;
};

// �����ݶ�
struct ipixel_gradient_radial
{
	ipixel_gradient_t gradient;
	cfixed x1, y1, r1;
	cfixed x2, y2, r2;
	cfixed xd, yd, rd;
	double a;
	double inva;
	double mindr;
};

// ׶���ݶ�
struct ipixel_gradient_conical
{
	ipixel_gradient_t gradient;
	ipixel_point_fixed_t center;
	double angle;
};

// �̶���ɫ
struct ipixel_solid_color
{
	IUINT32 color;
};

// ����Դ������
union ipixel_source_union
{
	IBITMAP *bitmap;
	ipixel_gradient_linear_t linear;
	ipixel_gradient_radial_t radial;
	ipixel_gradient_conical_t conical;
	ipixel_solid_color_t solid;
};

// ����Դ����
enum IPIXELSOURCE
{
	IPIXEL_SOURCE_BITMAP	= 0,
	IPIXEL_SOURCE_SOLID		= 1,
	IPIXEL_SOURCE_LINEAR	= 2,
	IPIXEL_SOURCE_RADIAL	= 3,
	IPIXEL_SOURCE_CONICAL	= 4,
};

// ����Դ����
struct ipixel_source
{
	enum IPIXELSOURCE type;
	ipixel_transform_t *transform;
	int overflow;
	IUINT32 transparent;
	iBitmapFetchProc fetch;
	IRECT srect;
	union ipixel_source_union source;
};


// ��ʼ��λͼԴ
void ipixel_source_init_bitmap(ipixel_source_t *source, IBITMAP *bmp);

// ��ʼ���̶���ɫԴ
void ipixel_source_init_solid(ipixel_source_t *source, IUINT32 color);

// ��ʼ�������ݶ�Դ
void ipixel_source_init_gradient_linear(ipixel_source_t *source,
	const ipixel_point_fixed_t *p1, const ipixel_point_fixed_t *p2,
	const ipixel_gradient_stop_t *stops, int nstops);

// ��ʼ������Դ
void ipixel_source_init_gradient_radial(ipixel_source_t *source,
	const ipixel_point_fixed_t *inner, const ipixel_point_fixed_t *outer,
	cfixed inner_radius, cfixed outer_radius, 
	const ipixel_gradient_stop_t *stops, int nstops);

// ��ʼ��׶��Դ
void ipixel_source_init_gradient_conical(ipixel_source_t *source,
	const ipixel_point_fixed_t *center, cfixed angle, 
	const ipixel_gradient_stop_t *stops, int nstops);


// ���ñ任���󣺾���ָ��仯�����ݲ���ʱҲҪ����
void ipixel_source_set_transform(ipixel_source_t *source,
	const ipixel_transform_t *t);

// ����Խ�緽ʽ
void ipixel_source_set_overflow(ipixel_source_t *source,
	enum IBOM overflow, IUINT32 transparent);

// ���ù�����
void ipixel_source_set_filter(ipixel_source_t *source,
	enum IPIXELFILTER filter);

// ���òü�����
void ipixel_source_set_bound(ipixel_source_t *source,
	const IRECT *bound);


// ȡ��ɨ����
int ipixel_source_fetch(const ipixel_source_t *source, int offset, int line,
	int width, IUINT32 *card, const IUINT8 *mask);


#ifdef __cplusplus
}
#endif

#endif



