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
// 几何部分
//======================================================================
typedef struct ipixel_point_fixed ipixel_point_fixed_t;
typedef struct ipixel_line_fixed  ipixel_line_fixed_t;
typedef struct ipixel_vector      ipixel_vector_t;
typedef struct ipixel_transform   ipixel_transform_t;
typedef struct ipixel_point       ipixel_point_t;
typedef struct ipixel_matrix      ipixel_matrix_t;

// 点的定义
struct ipixel_point_fixed
{
	cfixed x;
	cfixed y;
};

// 直线定义
struct ipixel_line_fixed
{
	ipixel_point_fixed_t p1;
	ipixel_point_fixed_t p2;
};

// 矢量定义
struct ipixel_vector
{
	cfixed vector[3];
};

// 矩阵定义
struct ipixel_transform
{
	cfixed matrix[3][3];
};

// 浮点数点
struct ipixel_point
{
	double x;
	double y;
};

// 浮点矩阵
struct ipixel_matrix
{
	double m[3][3];
};


//---------------------------------------------------------------------
// 矩阵操作
//---------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// 矩阵同点相乘: matrix * vector
int ipixel_transform_point(const ipixel_transform_t *matrix,
						     struct ipixel_vector *vector);

// 齐次化矢量: [x/w, y/w, 1]
int ipixel_transform_homogeneous(struct ipixel_vector *vector);

// 矩阵相乘：dst=l*r
int ipixel_transform_multiply(ipixel_transform_t *dst, 
							const ipixel_transform_t *l,
							const ipixel_transform_t *r);

// 初始化单位矩阵
int ipixel_transform_init_identity(ipixel_transform_t *matrix);

// 初始化位移矩阵
int ipixel_transform_init_translate(ipixel_transform_t *matrix, 
							cfixed x,
							cfixed y);

// 初始化旋转矩阵
int ipixel_transform_init_rotate(ipixel_transform_t *matrix,
							cfixed cos,
							cfixed sin);

// 初始化缩放矩阵
int ipixel_transform_init_scale(ipixel_transform_t *matrix,
							cfixed sx,
							cfixed sy);

// 初始化透视矩阵
int ipixel_transform_init_perspective(ipixel_transform_t *matrix,
							const struct ipixel_point_fixed *src,
							const struct ipixel_point_fixed *dst);

// 初始化仿射矩阵
int ipixel_transform_init_affine(ipixel_transform_t *matrix,
							const struct ipixel_point_fixed *src,
							const struct ipixel_point_fixed *dst);


// 检测是否是单位矩阵：成功返回非零，否则为零
int ipixel_transform_is_identity(const ipixel_transform_t *matrix);

// 检测是否是缩放矩阵：成功返回非零，否则为零
int ipixel_transform_is_scale(const ipixel_transform_t *matrix);

// 检测是否是整数平移矩阵：成功返回非零，否则为零
int ipixel_transform_is_int_translate(const ipixel_transform_t *matrix);

// 检测是否为平移+缩放
int ipixel_transform_is_scale_translate(const ipixel_transform_t *matrix);

// 检测是否为普通平移矩阵：
int ipixel_transform_is_translate(const ipixel_transform_t *matrix);


// 浮点数矩阵到定点数矩阵
int ipixel_transform_from_matrix(ipixel_transform_t *t, 
	const ipixel_matrix_t *m);

// 定点数矩阵到浮点数矩阵
int ipixel_transform_to_matrix(const ipixel_transform_t *t, 
	ipixel_matrix_t *m);

// 浮点数求逆矩阵
int ipixel_matrix_invert(ipixel_matrix_t *dst, const ipixel_matrix_t *src);

// 定点数矩阵求逆
int ipixel_transform_invert(ipixel_transform_t *dst, 
	const ipixel_transform_t *src);

// 浮点数矩阵乘法
int ipixel_matrix_point(const ipixel_matrix_t *matrix, double *vec);

// 初始化单位矩阵
int ipixel_matrix_init_identity(ipixel_matrix_t *matrix);

// 初始化位移矩阵
int ipixel_matrix_init_translate(ipixel_matrix_t *matrix, 
							double x,
							double y);

// 初始化旋转矩阵
int ipixel_matrix_init_rotate(ipixel_matrix_t *matrix,
							double cos,
							double sin);

// 初始化缩放矩阵
int ipixel_matrix_init_scale(ipixel_matrix_t *matrix,
							double sx,
							double sy);

#ifdef __cplusplus
}
#endif


//=====================================================================
// 光栅化部分
//=====================================================================
typedef struct ipixel_trapezoid   ipixel_trapezoid_t;
typedef struct ipixel_span        ipixel_span_t;

// 梯形定义
struct ipixel_trapezoid
{
	cfixed top, bottom;
	ipixel_line_fixed_t left, right;
};

// 扫描线定义
struct ipixel_span
{
	int x, y, w;
};


//---------------------------------------------------------------------
// 光栅化基础
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

// 光栅化梯形
void ipixel_raster_trapezoid(IBITMAP *image, const ipixel_trapezoid_t *trap,
		int x_off, int y_off, const IRECT *clip);

// 光栅化清除梯形
void ipixel_raster_clear(IBITMAP *image, const ipixel_trapezoid_t *trap,
		int x_off, int y_off, const IRECT *clip);

// 光栅化扫描线
int ipixel_raster_spans(IBITMAP *image, const ipixel_trapezoid_t *trap,
		int x_off, int y_off, const IRECT *clip, ipixel_span_t *spans);

// 光栅化三角形
void ipixel_raster_triangle(IBITMAP *image, const ipixel_point_fixed_t *p1,
		const ipixel_point_fixed_t *p2, const ipixel_point_fixed_t *p3, 
		int x_off, int y_off, const IRECT *clip);

// 批量梯形光栅化
void ipixel_raster_traps(IBITMAP *image, const ipixel_trapezoid_t *traps,
		int count, int x_off, int y_off, const IRECT *clip);


//---------------------------------------------------------------------
// 几何基础
//---------------------------------------------------------------------

// 线段与y轴相交的x坐标，ceil为是否向上去整
static inline cfixed ipixel_line_fixed_x(const ipixel_line_fixed_t *l,
						cfixed y, int ceil)
{
	cfixed dx = l->p2.x - l->p1.x;
	IINT64 ex = ((IINT64)(y - l->p1.y)) * dx;
	cfixed dy = l->p2.y - l->p1.y;
	if (ceil) ex += (dy - 1);
	return l->p1.x + (cfixed)(ex / dy);
}

// 梯形在当前扫描线的X轴的覆盖区域
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

// 梯形的绑定区域
void ipixel_trapezoid_bound(const ipixel_trapezoid_t *t, int n, IRECT *rect);

// 很多梯形在当前扫描线的X轴的覆盖区域
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


//---------------------------------------------------------------------
// 梯形基础
//---------------------------------------------------------------------

// 将三角形转化为trap，并返trap形个数，0-2个
int ipixel_traps_from_triangle(ipixel_trapezoid_t *trap, 
	const ipixel_point_fixed_t *p1, const ipixel_point_fixed_t *p2, 
	const ipixel_point_fixed_t *p3);


// 多边形转化为trap, 并返回trap的个数, [0, n * 2] 个
// 需要提供工作内存，大小为 sizeof(ipixel_point_fixed_t) * n
int ipixel_traps_from_polygon(ipixel_trapezoid_t *trap,
	const ipixel_point_fixed_t *pts, int n, int clockwise, void *workmem);


// 简易多边形转化为trap，不需要额外提供内存，在栈上分配了，参数相同
int ipixel_traps_from_polygon_ex(ipixel_trapezoid_t *trap,
	const ipixel_point_fixed_t *pts, int n, int clockwise);


//---------------------------------------------------------------------
// 像素读取
//---------------------------------------------------------------------
int ipixel_span_fetch(const IBITMAP *image, int offset, int line,
	int width, IUINT32 *card, const ipixel_transform_t *t,
	iBitmapFetchProc proc, const IRECT *clip);

iBitmapFetchProc ipixel_span_get_proc(const IBITMAP *image, 
	const ipixel_transform_t *t);



//---------------------------------------------------------------------
// 位图低层次光栅化：透视/仿射变换
//---------------------------------------------------------------------
#define IBITMAP_RASTER_FLAG_PERSPECTIVE		0	// 透视变换
#define IBITMAP_RASTER_FLAG_AFFINE			1	// 反射变换

#define IBITMAP_RASTER_FLAG_OVER			0	// 绘制OVER
#define IBITMAP_RASTER_FLAG_ADD				4	// 绘制ADD


// 低层次光栅化位图
int ibitmap_raster_low(IBITMAP *dst, const ipixel_point_fixed_t *pts, 
	const IBITMAP *src, const IRECT *rect, IUINT32 color, int flags,
	const IRECT *clip, void *workmem);

// 低层次光栅化：不需要工作内存，在栈上分配了
int ibitmap_raster_base(IBITMAP *dst, const ipixel_point_fixed_t *pts, 
	const IBITMAP *src, const IRECT *rect, IUINT32 color, int flags,
	const IRECT *clip);

// 低层次光栅化：浮点参数
int ibitmap_raster_float(IBITMAP *dst, const ipixel_point_t *pts, 
	const IBITMAP *src, const IRECT *rect, IUINT32 color, int flags,
	const IRECT *clip);



//---------------------------------------------------------------------
// 高层次光栅化
//---------------------------------------------------------------------

// 旋转/缩放绘制
int ibitmap_raster_draw(IBITMAP *dst, double x, double y, const IBITMAP *src,
	const IRECT *rect, int offset_x, int offset_y, double scale_x, 
	double scale_y, double angle, IUINT32 color, const IRECT *clip);

// 三维旋转/缩放绘制
int ibitmap_raster_draw_3d(IBITMAP *dst, double x, double y, double z,
	const IBITMAP *src, const IRECT *rect, int offset_x, int offset_y,
	double scale_x, double scale_y, double angle_x, double angle_y, 
	double angle_z, IUINT32 color, const IRECT *clip);


#ifdef __cplusplus
}
#endif

#endif



