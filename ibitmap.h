/**********************************************************************
 *
 * ibitmap.h - the basic definition of the bitmap
 *
 * IBITMAP is designed under the desire that it can exactly describe  
 * one basic bitmap, which is considered as simple and pure.
 *
 **********************************************************************/

#ifndef __IBITMAP_H__
#define __IBITMAP_H__


/**********************************************************************
 * Definition of struct IBITMAP
 **********************************************************************/

struct IBITMAP
{
    unsigned long w;         /* width of the bitmap   */
    unsigned long h;         /* height of the bitmap  */
    unsigned long bpp;       /* color depth of bitmap */
    unsigned long pitch;     /* pitch of the bitmap   */
    unsigned long mask;      /* bitmap bit flags      */
    unsigned long code;      /* bitmap class code     */
    unsigned long mode;      /* additional mode data  */
    void *pixel;             /* pixels data in bitmap */
    void *extra;             /* extra data structure  */
    void **line;             /* pointer to each line in bitmap */
};



#endif



