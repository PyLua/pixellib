//=====================================================================
//
// ibmfast.c - ibitmap fast operation
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ibmcols.h"
#include "ibitmapm.h"
#include "ibmfast.h"


int ibitmap_blitclip(const IBITMAP *dst, int *dx, int *dy, 
	const IBITMAP *src, int *sx, int *sy, int *sw, int *sh, int flags);


