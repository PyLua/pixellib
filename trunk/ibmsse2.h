//=====================================================================
//
// ibmsse2.h - sse2 & mmx optimize
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================

#ifndef __IBMSSE2_H__
#define __IBMSSE2_H__

#include "ibmbits.h"
#include "iblit386.h"

//---------------------------------------------------------------------
// PLATFORM DETECT
//---------------------------------------------------------------------
#if defined(__GNUC__)
	#if defined(__MMX__)
		#define __ARCH_MMX__
	#endif
	#if defined(__SSE__) 
		#define __ARCH_SSE__
	#endif
	#if defined(__SSE2__)
		#define __ARCH_SSE2__
	#endif
#elif defined(_MSC_VER) && defined(__x86__)
	#if !(defined(WIN64) || defined(_WIN64) || defined(__x86_64))
		#if _MSC_VER >= 1200
			#define __ARCH_MMX__
		#elif _MSC_VER >= 1300
			#define __ARCH_SSE__
		#elif _MSC_VER >= 1400
			#define __ARCH_SSE2__
		#endif
	#endif
#endif


//---------------------------------------------------------------------
// pixel load and store
//---------------------------------------------------------------------



//---------------------------------------------------------------------
// INTERFACES
//---------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

int pixellib_mmx_init(void);

int pixellib_xmm_init(void);

#ifdef __cplusplus
}
#endif



#endif


