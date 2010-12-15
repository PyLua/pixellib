//=====================================================================
//
// ibmfast.h - ibitmap fast operation
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================
#ifndef __IBMFAST_H__
#define __IBMFAST_H__

#include <stddef.h>
#include <math.h>

#include "ibitmap.h"
#include "ibitmapm.h"
#include "ibmcols.h"


//---------------------------------------------------------------------
// fast call in gcc
//---------------------------------------------------------------------
#ifndef IFASTCALL
	#if defined(__i386__) && defined(__GNUC__)
		#define IFASTCALL	__attribute__((regparm(3)))
	#else
		#define IFASTCALL
	#endif
#endif


//=====================================================================
// INDEX COLOR LUT
//=====================================================================
struct INDEXCOLOR
{
	int color;
	IUINT32 rgba[256];
	unsigned char ent[32768];
};

typedef struct INDEXCOLOR iColorIndex;


//=====================================================================
// PROCEDURE DECLARE
//=====================================================================
typedef IFASTCALL void (*iFetchProc)(const void *bits, int x, 
	int w, IUINT32 *buffer, const iColorIndex *idx);

typedef IFASTCALL void (*iStoreProc)(void *bits, 
	const IUINT32 *buffer, int x, int w, const iColorIndex *idx);

typedef IFASTCALL IUINT32 (*iFetchPixelProc)(const void *bits, 
	int offset, const iColorIndex *idx);



#ifdef __cplusplus
extern "C" {
#endif

//=====================================================================
// INTERFACES
//=====================================================================
iFetchProc iFetchGetProc(int fmt);
iStoreProc iStoreGetProc(int fmt);

iFetchPixelProc iFetchPixelGetProc(int fmt);


#ifdef __cplusplus
}
#endif


#endif



