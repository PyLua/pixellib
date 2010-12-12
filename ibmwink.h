//=====================================================================
//
// ibmwink.h - 
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================

#ifndef __IBMWINK_H__
#define __IBMWINK_H__

#include <stddef.h>
#include <math.h>

#include "ibitmap.h"
#include "ibitmapm.h"
#include "ibmcols.h"
#include "ibmfast.h"


//---------------------------------------------------------------------
// ���νṹ
//---------------------------------------------------------------------
struct IPOINT2D
{
	float x;
	float y;
};

struct IRECT2D
{
	float x0;
	float y0;
	float x1;
	float y1;
};

typedef struct IRECT2D IRECT2D;
typedef struct IPOINT2D IPOINT2D;
typedef IPOINT2D IVECTOR2D;


//---------------------------------------------------------------------
// ����������
//---------------------------------------------------------------------




#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------
// Simple Drawing
//---------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif


