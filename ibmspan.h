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
#include "ibitmapm.h"
#include "ibmcols.h"


//---------------------------------------------------------------------
// SPAN��Ⱦ(spanָһ��ɨ���ߣ����ڶ�������)
//---------------------------------------------------------------------
typedef struct ISPAN
{
	int x;							// ɨ������� X����
	int y;							// ɨ������� Y����
	int w;							// ɨ���߳���
	ICOLORD color1;					// ɨ����������ɫ
	ICOLORD color2;					// ɨ�����յ����ɫ
	const unsigned char *cover;		// ���飺ÿ����� alphaֵ(��ΪNULL��
}	ISPAN;


#define ISPAN_MODE_FLAT_FILL			0		// FLAT���
#define ISPAN_MODE_FLAT_BLEND			1		// FLAT��ɫ
#define ISPAN_MODE_FLAT_COVER_FILL		2		// FLAT�� COVER���
#define ISPAN_MODE_FLAT_COVER_BLEND		3		// FLAT�� COVER��ɫ
#define ISPAN_MODE_GOURAUD_FILL			4		// ���޵����
#define ISPAN_MODE_GOURAUD_BLEND		5		// ���޵»�ɫ
#define ISPAN_MODE_GOURAUD_COVER_FILL	6		// ���޵´� COVER���
#define ISPAN_MODE_GOURAUD_COVER_BLEND	7		// ���޵´� COVER��ɫ


// ɨ���������ⲿ�������ڲ��Ѿ���Cʵ�֣��ⲿ���������ⲿ���ʵ�֣�
// ����0����ɹ������ط�0����ʧ�ܣ�ʧ��������Ĭ��C�����룩
typedef int (*ISPAN_PROC)(unsigned char *ptr, int w, ICOLORD color1, 
	ICOLORD color2, const unsigned char *cover, int pixfmt, int additive);

// ɨ���������б�, ispan_proc[pixfmt][ISPAN_MODE_*]
extern ISPAN_PROC ispan_proc[24][8];


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


#ifdef __cplusplus
}
#endif


#endif



