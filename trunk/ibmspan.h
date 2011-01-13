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
// SPAN��Ⱦ(spanָһ��ɨ���ߣ����ڶ�������)
//---------------------------------------------------------------------
typedef struct ISPAN
{
	int x;							// ɨ������� X����
	int y;							// ɨ������� Y����
	int w;							// ɨ���߳���
	int alpha;						// ���ص��Ƿ����alpha
	ICOLORD color1;					// ɨ����������ɫ
	ICOLORD color2;					// ɨ�����յ����ɫ
	const unsigned char *cover;		// ���飺ÿ����� alphaֵ(��ΪNULL��
	const unsigned char *pixel;		// ���飺ARGB32��ʽ������
}	ISPAN;


#define ISPAN_MODE_FLAT_FILL			0		// FLAT���
#define ISPAN_MODE_FLAT_BLEND			1		// FLAT��ɫ
#define ISPAN_MODE_FLAT_COVER_FILL		2		// FLAT�� COVER���
#define ISPAN_MODE_FLAT_COVER_BLEND		3		// FLAT�� COVER��ɫ
#define ISPAN_MODE_GOURAUD_FILL			4		// ���޵����
#define ISPAN_MODE_GOURAUD_BLEND		5		// ���޵»�ɫ
#define ISPAN_MODE_GOURAUD_COVER_FILL	6		// ���޵´� COVER���
#define ISPAN_MODE_GOURAUD_COVER_BLEND	7		// ���޵´� COVER��ɫ
#define ISPAN_MODE_FLAT_COPY			8		// FLAT����
#define ISPAN_MODE_FLAT_BLIT			9		// FLAT��ɫ
#define ISPAN_MODE_FLAT_COVER_COPY		10		// FLAT�� COVER����
#define ISPAN_MODE_FLAT_COVER_BLIT		11		// FLAT�� COVER��ɫ
#define ISPAN_MODE_GOURAUD_COPY			12		// ���޵¿���
#define ISPAN_MODE_GOURAUD_BLIT			13		// ���޵»�ɫ
#define ISPAN_MODE_GOURAUD_COVER_COPY	14		// ���޵´� COVER����
#define ISPAN_MODE_GOURAUD_COVER_BLIT	15		// ���޵´� COVER��ɫ


// ɨ���������ⲿ�������ڲ��Ѿ���Cʵ�֣��ⲿ���������ⲿ���ʵ�֣�
// ����0����ɹ������ط�0����ʧ�ܣ�ʧ��������Ĭ��C�����룩
typedef int (*ISPAN_PROC)(unsigned char *ptr, const ISPAN *span, 
	int pixfmt, int additive);

// ɨ���������б�, ispan_proc[pixfmt][ISPAN_MODE_*]
extern ISPAN_PROC ispan_proc[24][16];


#define ISPAN_FLAG_ADDITIVE		1				// ʹ�üӷ�ģʽ��ɫ
#define ISPAN_FLAG_DEFAULT		2				// ʹ��Ĭ�ϵ�C�����



#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
// INTERFACES
//---------------------------------------------------------------------

// �����߶�������
void ispan_draw(IBITMAP *dst, const ISPAN *spans, int count, int flags);


// �߶�����������������ߣ�С�������ұ�
void ispan_shrink(ISPAN *span, int width);


#ifdef __cplusplus
}
#endif


#endif



