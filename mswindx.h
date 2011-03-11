//=====================================================================
//
// mswindx.h - Microsoft Window DirectDraw7 Easy Interface
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================

#ifndef __MSWINDX_H__
#define __MSWINDX_H__

#if (defined(_WIN32) || defined(WIN32))

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


//---------------------------------------------------------------------
// Structure Reference
//---------------------------------------------------------------------
struct CSURFACE;
typedef struct CSURFACE CSURFACE;


#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------
// Interfaces
//---------------------------------------------------------------------

// ��ʼ�� DirectDraw
int DDrawInit(void);

// ����
void DDrawDestroy(void);

// ���ظ�ʽ
#define PIXFMT_DEFAULT		0
#define PIXFMT_8			1
#define PIXFMT_RGB15		2
#define PIXFMT_RGB16		3
#define PIXFMT_RGB24		4
#define PIXFMT_RGB32		5
#define PIXFMT_ARGB32		6

// ģʽ
#define DDSM_SYSTEMMEM		0
#define DDSM_VIDEOMEM		1

// ��������
// fmt Ϊ���ظ�ʽ���� DDPF_xx
// modeΪ DDSM_SYSTEMMEM���� DDSM_VIDEOMEM
CSURFACE *DDrawSurfaceCreate(int w, int h, int fmt, int mode, HRESULT *hr);

// ɾ������
void DDrawSurfaceRelease(CSURFACE *surface);

// ���� DirectDraw��Ļ
// ��� bppΪ0�Ļ��򴴽�����ģʽ��w,hʧЧ������ȫ��Ļ
CSURFACE *DDrawCreateScreen(HWND hWnd, int w, int h, int bpp, HRESULT *hr);


// ȡ�ñ�����Ϣ
int DDrawSurfaceInfo(const CSURFACE *s, int *w, int *h, int *bpp, int *fmt);


#define DDBLIT_MASK		1		// ʹ�ùؼ�ɫ
#define DDBLIT_NOWAIT	2		// �첽BLIT
#define DDBLIT_NOCLIP	3		// ���ü�

// BLIT: 
// (dx, dy, dw, dh)  -  Ŀ����������
// (sx, sy, sw, sh)  -  Դ���������
// flags��������������Щ DDBLIT_* ��ͷ�ĺ�
// ���� 0��ʾ�ɹ�������ֵ��ʾ DirectDrawSurface7::Blt�ķ���
int DDrawSurfaceBlit(CSURFACE *dst, int dx, int dy, int dw, int dh, 
	CSURFACE *src, int sx, int sy, int sw, int sh, int flags);


#define DDLOCK_NOWAIT		1	// ����Ȼ�����Ϸ���
#define DDLOCK_OREAD		2	// ֻ��
#define DDLOCK_OWRITE		4	// ֻд


// �������棺���������д���ݣ�flagsֵ���������������
// ����    0 �ɹ���
// ����    1 ��ʾ���ڻ��ƣ��޷�����(û��NOWAITʱ)
// ����   -1 ��ʾ�Ѿ��������ˣ��޷��ٴ�����
// ����   -2 ����ʱ���������������� 
int DDrawSurfaceLock(CSURFACE *dst, void **bits, long *pitch, int flags);


// �������棺��������ܵ���BLIT
// ����    0 �ɹ���
// ����   -1 ��ʾ�Ѿ��������ˣ��޷��ٴ�����
// ����   -2 ����ʱ���������������� 
int DDrawSurfaceUnlock(CSURFACE *dst);


// ���ùؼ�ɫ
int DDrawSurfaceSetMask(CSURFACE *dst, unsigned long mask);


// ��ֱͬ��: 0��ӦDDWAITVB_BLOCKBEGIN, ����ΪDDWAITVB_BLOCKEND 
int DDrawWaitForVerticalBlank(int mode);


#ifdef __cplusplus
}
#endif


#endif

#endif



