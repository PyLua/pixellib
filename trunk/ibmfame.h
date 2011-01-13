//=====================================================================
//
// ibmfame.h - alpha blending
//
// NOTE:
// 默认Alpha 混色器，本模块内部使用大量宏定义来减少绘制时候的
// 分支判断，根据不同的颜色格式，绘制方式，用宏定义了数百段绘
// 制代码，针对不同的情况。外部可以通过设置驱动的方式来定义某
// 具体部分的代码，比如使用汇编重写填充代码来加速。由于定义了
// 上百个模块，因此该优化模式下编译速度很慢，需要数分钟之久，
// 而本模块所定义的接口，一般不会有太大变化。
//
//=====================================================================
#ifndef __IBMFAME_H__
#define __IBMFAME_H__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ibitmap.h"
#include "ibmcols.h"
#include "iblit386.h"


//---------------------------------------------------------------------
// inline definition
//---------------------------------------------------------------------
#ifndef INLINE
#ifdef __GNUC__

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif

#elif (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#ifndef inline
#define inline INLINE
#endif


#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------
// 混色驱动定义
//---------------------------------------------------------------------

// Alpha Blending 混色的外部驱动声明：
// dst    - 目标内存
// dpitch - 目标内存一条扫描线所占的字节数
// src    - 源内存
// spitch - 源内存一条扫描线所占的字节数
// w      - 绘制区域宽度
// h      - 绘制区域长度
// color  - 叠加上去的颜色(0xffffffff为不叠加，0x80ffffff为半透明等)
// dfmt   - 目标像素格式，见ibmcols.h 里面的 IPIX_FMT_XX
// sfmt   - 源像素格式，见ibmcols.h 里面的 IPIX_FMT_XX
// mask   - 使用的关键色
// flags  - 是否有：反转(IBLIT_HFLIP, IBLIT_VFLIP), 透明色(IBLIT_MASK)
//
// 如果成功完成绘制，则返回宽度 w，如果失败则返回0，如果返回
// (0, w) 区间的数字，那么说明只绘制了一部分宽度，剩下的宽度将由
// 默认的C代码完成绘制，比如汇编代码一次性处理4个像素，那么绘制
// 宽度为w=103个像素的图片的话，汇编代码写的驱动可以只绘制对齐了的
// 100个像素（即返回100），剩下的3个像素将由默认C代码完成绘制
typedef int (*IBLEND_PROC)(unsigned char *dst, long dpitch, 
	const unsigned char *src, long spitch, int w, int h, ICOLORD color,
	int dfmt, int sfmt, ICOLORD mask, int flags);

extern IBLEND_PROC iblend_proc[24][24];

//---------------------------------------------------------------------
// 接口定义
//---------------------------------------------------------------------

#define IBLEND_DEFAULT		512			// 是用默认的 C混色器

// Alpha Blending 主入口
// dst      - 目标 bitmap
// dx, dy   - 目标位置
// src      - 源 bitmap
// sx, sy   - 源位置
// w, h     - 图块的宽和高
// color    - 叠加上去的颜色(0xffffffff为不叠加，0x80ffffff为半透明等)
// flags    - IBLIT_HFLIP, IBLIT_VFLIP, IBLIT_MASK, IBLEND_DEFAULT
int iblend_blit(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h, ICOLORD color, int flags);


// 转换 ARGB32 -> PARGB32
// 预乘所有像素的 Alpha值
int iblend_pmul_convert(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h);


// PARGB32的 AlphaBlend
// src 需要先用 iblend_pmul_convert从ARGB32转换而来
int iblend_pmul_blit(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h, int alpha);


// 检测MMX，如果支持的话，将自动更改绘制驱动
// 成功返回1，不成功返回0
int iblend_mmx_detect(void);


// 点操作：取得点
ICOLORD iblend_getpixel(IBITMAP *bmp, int x, int y);

// 点操作：绘制点
void iblend_putpixel(IBITMAP *bmp, int x, int y, ICOLORD c);

// 点操作：绘制点（加法方式）
void iblend_addpixel(IBITMAP *bmp, int x, int y, ICOLORD c);

// 点操作：批量绘制点（每个点的颜色相同）
void iblend_setpixel(IBITMAP *bmp, const int *xy, int n, ICOLORD c, int add);

// 点操作：批量绘制点（每个点的颜色不同）
void iblend_dispixel(IBITMAP *bmp, const int *xy, const ICOLORD *colors, 
	int count, int additive);


#ifdef __cplusplus
}
#endif


#endif



