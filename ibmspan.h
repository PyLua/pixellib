//=====================================================================
// 
// ibmspan.h - 
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================
#ifndef __IBMSPAN_H__
#define __IBMSPAN_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ibitmap.h"
#include "ibmcols.h"


//---------------------------------------------------------------------
// SPAN渲染(span指一段扫描线，用于多边形填充)
//---------------------------------------------------------------------
typedef struct ISPAN
{
	int x;							// 扫描线起点 X坐标
	int y;							// 扫描线起点 Y坐标
	int w;							// 扫描线长度
	int alpha;						// 像素点是否带有alpha
	ICOLORD color1;					// 扫描线起点的颜色
	ICOLORD color2;					// 扫描线终点的颜色
	const unsigned char *cover;		// 数组：每个点的 alpha值(可为NULL）
	const unsigned char *pixel;		// 数组：ARGB32格式的像素
}	ISPAN;


#define ISPAN_MODE_FLAT_FILL			0		// FLAT填充
#define ISPAN_MODE_FLAT_BLEND			1		// FLAT混色
#define ISPAN_MODE_FLAT_COVER_FILL		2		// FLAT带 COVER填充
#define ISPAN_MODE_FLAT_COVER_BLEND		3		// FLAT带 COVER混色
#define ISPAN_MODE_GOURAUD_FILL			4		// 高罗德填充
#define ISPAN_MODE_GOURAUD_BLEND		5		// 高罗德混色
#define ISPAN_MODE_GOURAUD_COVER_FILL	6		// 高罗德带 COVER填充
#define ISPAN_MODE_GOURAUD_COVER_BLEND	7		// 高罗德带 COVER混色
#define ISPAN_MODE_FLAT_COPY			8		// FLAT拷贝
#define ISPAN_MODE_FLAT_BLIT			9		// FLAT混色
#define ISPAN_MODE_FLAT_COVER_COPY		10		// FLAT带 COVER拷贝
#define ISPAN_MODE_FLAT_COVER_BLIT		11		// FLAT带 COVER混色
#define ISPAN_MODE_GOURAUD_COPY			12		// 高罗德拷贝
#define ISPAN_MODE_GOURAUD_BLIT			13		// 高罗德混色
#define ISPAN_MODE_GOURAUD_COVER_COPY	14		// 高罗德带 COVER拷贝
#define ISPAN_MODE_GOURAUD_COVER_BLIT	15		// 高罗德带 COVER混色


// 扫描线填充的外部驱动（内部已经有C实现，外部驱动用于外部汇编实现）
// 返回0代表成功，返回非0代表失败（失败则启用默认C填充代码）
typedef int (*ISPAN_PROC)(unsigned char *ptr, const ISPAN *span, 
	int pixfmt, int additive);

// 扫描线驱动列表, ispan_proc[pixfmt][ISPAN_MODE_*]
extern ISPAN_PROC ispan_proc[24][16];


#define ISPAN_FLAG_ADDITIVE		1				// 使用加法模式混色
#define ISPAN_FLAG_DEFAULT		2				// 使用默认的C填充器



#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
// INTERFACES
//---------------------------------------------------------------------

// 绘制线段主函数
void ispan_draw(IBITMAP *dst, const ISPAN *spans, int count, int flags);


// 线段收缩，大于零是左边，小于零是右边
void ispan_shrink(ISPAN *span, int width);


#ifdef __cplusplus
}
#endif


#endif



