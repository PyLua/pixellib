//=====================================================================
//
// ibmfame.h - alpha blending
//
// NOTE:
// Ĭ��Alpha ��ɫ������ģ���ڲ�ʹ�ô����궨�������ٻ���ʱ���
// ��֧�жϣ����ݲ�ͬ����ɫ��ʽ�����Ʒ�ʽ���ú궨�������ٶλ�
// �ƴ��룬��Բ�ͬ��������ⲿ����ͨ�����������ķ�ʽ������ĳ
// ���岿�ֵĴ��룬����ʹ�û����д�����������١����ڶ�����
// �ϰٸ�ģ�飬��˸��Ż�ģʽ�±����ٶȺ�������Ҫ������֮�ã�
// ����ģ��������Ľӿڣ�һ�㲻����̫��仯��
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
// ��ɫ��������
//---------------------------------------------------------------------

// Alpha Blending ��ɫ���ⲿ����������
// dst    - Ŀ���ڴ�
// dpitch - Ŀ���ڴ�һ��ɨ������ռ���ֽ���
// src    - Դ�ڴ�
// spitch - Դ�ڴ�һ��ɨ������ռ���ֽ���
// w      - ����������
// h      - �������򳤶�
// color  - ������ȥ����ɫ(0xffffffffΪ�����ӣ�0x80ffffffΪ��͸����)
// dfmt   - Ŀ�����ظ�ʽ����ibmcols.h ����� IPIX_FMT_XX
// sfmt   - Դ���ظ�ʽ����ibmcols.h ����� IPIX_FMT_XX
// mask   - ʹ�õĹؼ�ɫ
// flags  - �Ƿ��У���ת(IBLIT_HFLIP, IBLIT_VFLIP), ͸��ɫ(IBLIT_MASK)
//
// ����ɹ���ɻ��ƣ��򷵻ؿ�� w�����ʧ���򷵻�0���������
// (0, w) ��������֣���ô˵��ֻ������һ���ֿ�ȣ�ʣ�µĿ�Ƚ���
// Ĭ�ϵ�C������ɻ��ƣ����������һ���Դ���4�����أ���ô����
// ���Ϊw=103�����ص�ͼƬ�Ļ���������д����������ֻ���ƶ����˵�
// 100�����أ�������100����ʣ�µ�3�����ؽ���Ĭ��C������ɻ���
typedef int (*IBLEND_PROC)(unsigned char *dst, long dpitch, 
	const unsigned char *src, long spitch, int w, int h, ICOLORD color,
	int dfmt, int sfmt, ICOLORD mask, int flags);

extern IBLEND_PROC iblend_proc[24][24];

//---------------------------------------------------------------------
// �ӿڶ���
//---------------------------------------------------------------------

#define IBLEND_DEFAULT		512			// ����Ĭ�ϵ� C��ɫ��

// Alpha Blending �����
// dst      - Ŀ�� bitmap
// dx, dy   - Ŀ��λ��
// src      - Դ bitmap
// sx, sy   - Դλ��
// w, h     - ͼ��Ŀ�͸�
// color    - ������ȥ����ɫ(0xffffffffΪ�����ӣ�0x80ffffffΪ��͸����)
// flags    - IBLIT_HFLIP, IBLIT_VFLIP, IBLIT_MASK, IBLEND_DEFAULT
int iblend_blit(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h, ICOLORD color, int flags);


// ת�� ARGB32 -> PARGB32
// Ԥ���������ص� Alphaֵ
int iblend_pmul_convert(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h);


// PARGB32�� AlphaBlend
// src ��Ҫ���� iblend_pmul_convert��ARGB32ת������
int iblend_pmul_blit(IBITMAP *dst, int dx, int dy, IBITMAP *src, 
	int sx, int sy, int w, int h, int alpha);


// ���MMX�����֧�ֵĻ������Զ����Ļ�������
// �ɹ�����1�����ɹ�����0
int iblend_mmx_detect(void);


// �������ȡ�õ�
ICOLORD iblend_getpixel(IBITMAP *bmp, int x, int y);

// ����������Ƶ�
void iblend_putpixel(IBITMAP *bmp, int x, int y, ICOLORD c);

// ����������Ƶ㣨�ӷ���ʽ��
void iblend_addpixel(IBITMAP *bmp, int x, int y, ICOLORD c);

// ��������������Ƶ㣨ÿ�������ɫ��ͬ��
void iblend_setpixel(IBITMAP *bmp, const int *xy, int n, ICOLORD c, int add);

// ��������������Ƶ㣨ÿ�������ɫ��ͬ��
void iblend_dispixel(IBITMAP *bmp, const int *xy, const ICOLORD *colors, 
	int count, int additive);


#ifdef __cplusplus
}
#endif


#endif



