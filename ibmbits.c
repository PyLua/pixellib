/**********************************************************************
 *
 * ibmbits.c - bitmap bits accessing
 *
 * NOTE:
 * for more information, please see the readme file
 *
 **********************************************************************/

#include "ibmbits.h"


/**********************************************************************
 * GLOBAL VARIABLES
 **********************************************************************/

/* pixel format declare */
const struct IPIXELFMT ipixelfmt[64] =
{
    { IPIX_FMT_A8R8G8B8, 32, 1, 0, 4, "IPIX_FMT_A8R8G8B8",
      0xff0000, 0xff00, 0xff, 0xff000000, 16, 8, 0, 24, 0, 0, 0, 0 },
    { IPIX_FMT_A8B8G8R8, 32, 1, 0, 4, "IPIX_FMT_A8B8G8R8",
      0xff, 0xff00, 0xff0000, 0xff000000, 0, 8, 16, 24, 0, 0, 0, 0 },
    { IPIX_FMT_R8G8B8A8, 32, 1, 0, 4, "IPIX_FMT_R8G8B8A8",
      0xff000000, 0xff0000, 0xff00, 0xff, 24, 16, 8, 0, 0, 0, 0, 0 },
    { IPIX_FMT_B8G8R8A8, 32, 1, 0, 4, "IPIX_FMT_B8G8R8A8",
      0xff00, 0xff0000, 0xff000000, 0xff, 8, 16, 24, 0, 0, 0, 0, 0 },
    { IPIX_FMT_X8R8G8B8, 32, 0, 1, 4, "IPIX_FMT_X8R8G8B8",
      0xff0000, 0xff00, 0xff, 0x0, 16, 8, 0, 0, 0, 0, 0, 8 },
    { IPIX_FMT_X8B8G8R8, 32, 0, 1, 4, "IPIX_FMT_X8B8G8R8",
      0xff, 0xff00, 0xff0000, 0x0, 0, 8, 16, 0, 0, 0, 0, 8 },
    { IPIX_FMT_R8G8B8X8, 32, 0, 1, 4, "IPIX_FMT_R8G8B8X8",
      0xff000000, 0xff0000, 0xff00, 0x0, 24, 16, 8, 0, 0, 0, 0, 8 },
    { IPIX_FMT_B8G8R8X8, 32, 0, 1, 4, "IPIX_FMT_B8G8R8X8",
      0xff00, 0xff0000, 0xff000000, 0x0, 8, 16, 24, 0, 0, 0, 0, 8 },
    { IPIX_FMT_P8R8G8B8, 32, 1, 5, 4, "IPIX_FMT_P8R8G8B8",
      0xff0000, 0xff00, 0xff, 0x0, 16, 8, 0, 0, 0, 0, 0, 8 },
    { IPIX_FMT_R8G8B8, 24, 0, 1, 3, "IPIX_FMT_R8G8B8",
      0xff0000, 0xff00, 0xff, 0x0, 16, 8, 0, 0, 0, 0, 0, 8 },
    { IPIX_FMT_B8G8R8, 24, 0, 1, 3, "IPIX_FMT_B8G8R8",
      0xff, 0xff00, 0xff0000, 0x0, 0, 8, 16, 0, 0, 0, 0, 8 },
    { IPIX_FMT_R5G6B5, 16, 0, 1, 2, "IPIX_FMT_R5G6B5",
      0xf800, 0x7e0, 0x1f, 0x0, 11, 5, 0, 0, 3, 2, 3, 8 },
    { IPIX_FMT_B5G6R5, 16, 0, 1, 2, "IPIX_FMT_B5G6R5",
      0x1f, 0x7e0, 0xf800, 0x0, 0, 5, 11, 0, 3, 2, 3, 8 },
    { IPIX_FMT_X1R5G5B5, 16, 0, 1, 2, "IPIX_FMT_X1R5G5B5",
      0x7c00, 0x3e0, 0x1f, 0x0, 10, 5, 0, 0, 3, 3, 3, 8 },
    { IPIX_FMT_X1B5G5R5, 16, 0, 1, 2, "IPIX_FMT_X1B5G5R5",
      0x1f, 0x3e0, 0x7c00, 0x0, 0, 5, 10, 0, 3, 3, 3, 8 },
    { IPIX_FMT_R5G5B5X1, 16, 0, 1, 2, "IPIX_FMT_R5G5B5X1",
      0xf800, 0x7c0, 0x3e, 0x0, 11, 6, 1, 0, 3, 3, 3, 8 },
    { IPIX_FMT_B5G5R5X1, 16, 0, 1, 2, "IPIX_FMT_B5G5R5X1",
      0x3e, 0x7c0, 0xf800, 0x0, 1, 6, 11, 0, 3, 3, 3, 8 },
    { IPIX_FMT_A1R5G5B5, 16, 1, 0, 2, "IPIX_FMT_A1R5G5B5",
      0x7c00, 0x3e0, 0x1f, 0x8000, 10, 5, 0, 15, 3, 3, 3, 7 },
    { IPIX_FMT_A1B5G5R5, 16, 1, 0, 2, "IPIX_FMT_A1B5G5R5",
      0x1f, 0x3e0, 0x7c00, 0x8000, 0, 5, 10, 15, 3, 3, 3, 7 },
    { IPIX_FMT_R5G5B5A1, 16, 1, 0, 2, "IPIX_FMT_R5G5B5A1",
      0xf800, 0x7c0, 0x3e, 0x1, 11, 6, 1, 0, 3, 3, 3, 7 },
    { IPIX_FMT_B5G5R5A1, 16, 1, 0, 2, "IPIX_FMT_B5G5R5A1",
      0x3e, 0x7c0, 0xf800, 0x1, 1, 6, 11, 0, 3, 3, 3, 7 },
    { IPIX_FMT_X4R4G4B4, 16, 0, 1, 2, "IPIX_FMT_X4R4G4B4",
      0xf00, 0xf0, 0xf, 0x0, 8, 4, 0, 0, 4, 4, 4, 8 },
    { IPIX_FMT_X4B4G4R4, 16, 0, 1, 2, "IPIX_FMT_X4B4G4R4",
      0xf, 0xf0, 0xf00, 0x0, 0, 4, 8, 0, 4, 4, 4, 8 },
    { IPIX_FMT_R4G4B4X4, 16, 0, 1, 2, "IPIX_FMT_R4G4B4X4",
      0xf000, 0xf00, 0xf0, 0x0, 12, 8, 4, 0, 4, 4, 4, 8 },
    { IPIX_FMT_B4G4R4X4, 16, 0, 1, 2, "IPIX_FMT_B4G4R4X4",
      0xf0, 0xf00, 0xf000, 0x0, 4, 8, 12, 0, 4, 4, 4, 8 },
    { IPIX_FMT_A4R4G4B4, 16, 1, 0, 2, "IPIX_FMT_A4R4G4B4",
      0xf00, 0xf0, 0xf, 0xf000, 8, 4, 0, 12, 4, 4, 4, 4 },
    { IPIX_FMT_A4B4G4R4, 16, 1, 0, 2, "IPIX_FMT_A4B4G4R4",
      0xf, 0xf0, 0xf00, 0xf000, 0, 4, 8, 12, 4, 4, 4, 4 },
    { IPIX_FMT_R4G4B4A4, 16, 1, 0, 2, "IPIX_FMT_R4G4B4A4",
      0xf000, 0xf00, 0xf0, 0xf, 12, 8, 4, 0, 4, 4, 4, 4 },
    { IPIX_FMT_B4G4R4A4, 16, 1, 0, 2, "IPIX_FMT_B4G4R4A4",
      0xf0, 0xf00, 0xf000, 0xf, 4, 8, 12, 0, 4, 4, 4, 4 },
    { IPIX_FMT_C8, 8, 0, 2, 1, "IPIX_FMT_C8",
      0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 8, 8, 8, 8 },
    { IPIX_FMT_G8, 8, 0, 3, 1, "IPIX_FMT_G8",
      0x0, 0xff, 0x0, 0x0, 0, 0, 0, 0, 8, 0, 8, 8 },
    { IPIX_FMT_A8, 8, 1, 4, 1, "IPIX_FMT_A8",
      0x0, 0x0, 0x0, 0xff, 0, 0, 0, 0, 8, 8, 8, 0 },
    { IPIX_FMT_R3G3B2, 8, 0, 1, 1, "IPIX_FMT_R3G3B2",
      0xe0, 0x1c, 0x3, 0x0, 5, 2, 0, 0, 5, 5, 6, 8 },
    { IPIX_FMT_B2G3R3, 8, 0, 1, 1, "IPIX_FMT_B2G3R3",
      0x7, 0x38, 0xc0, 0x0, 0, 3, 6, 0, 5, 5, 6, 8 },
    { IPIX_FMT_X2R2G2B2, 8, 0, 1, 1, "IPIX_FMT_X2R2G2B2",
      0x30, 0xc, 0x3, 0x0, 4, 2, 0, 0, 6, 6, 6, 8 },
    { IPIX_FMT_X2B2G2R2, 8, 0, 1, 1, "IPIX_FMT_X2B2G2R2",
      0x3, 0xc, 0x30, 0x0, 0, 2, 4, 0, 6, 6, 6, 8 },
    { IPIX_FMT_R2G2B2X2, 8, 0, 1, 1, "IPIX_FMT_R2G2B2X2",
      0xc0, 0x30, 0xc, 0x0, 6, 4, 2, 0, 6, 6, 6, 8 },
    { IPIX_FMT_B2G2R2X2, 8, 0, 1, 1, "IPIX_FMT_B2G2R2X2",
      0xc, 0x30, 0xc0, 0x0, 2, 4, 6, 0, 6, 6, 6, 8 },
    { IPIX_FMT_A2R2G2B2, 8, 1, 0, 1, "IPIX_FMT_A2R2G2B2",
      0x30, 0xc, 0x3, 0xc0, 4, 2, 0, 6, 6, 6, 6, 6 },
    { IPIX_FMT_A2B2G2R2, 8, 1, 0, 1, "IPIX_FMT_A2B2G2R2",
      0x3, 0xc, 0x30, 0xc0, 0, 2, 4, 6, 6, 6, 6, 6 },
    { IPIX_FMT_R2G2B2A2, 8, 1, 0, 1, "IPIX_FMT_R2G2B2A2",
      0xc0, 0x30, 0xc, 0x3, 6, 4, 2, 0, 6, 6, 6, 6 },
    { IPIX_FMT_B2G2R2A2, 8, 1, 0, 1, "IPIX_FMT_B2G2R2A2",
      0xc, 0x30, 0xc0, 0x3, 2, 4, 6, 0, 6, 6, 6, 6 },
    { IPIX_FMT_X4C4, 8, 0, 2, 1, "IPIX_FMT_X4C4",
      0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 8, 8, 8, 8 },
    { IPIX_FMT_X4G4, 8, 0, 3, 1, "IPIX_FMT_X4G4",
      0x0, 0xf, 0x0, 0x0, 0, 0, 0, 0, 8, 4, 8, 8 },
    { IPIX_FMT_X4A4, 8, 1, 4, 1, "IPIX_FMT_X4A4",
      0x0, 0x0, 0x0, 0xf, 0, 0, 0, 0, 8, 8, 8, 4 },
    { IPIX_FMT_C4X4, 8, 0, 2, 1, "IPIX_FMT_C4X4",
      0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 8, 8, 8, 8 },
    { IPIX_FMT_G4X4, 8, 0, 3, 1, "IPIX_FMT_G4X4",
      0x0, 0xf0, 0x0, 0x0, 0, 4, 0, 0, 8, 4, 8, 8 },
    { IPIX_FMT_A4X4, 8, 1, 4, 1, "IPIX_FMT_A4X4",
      0x0, 0x0, 0x0, 0xf0, 0, 0, 0, 4, 8, 8, 8, 4 },
    { IPIX_FMT_C4, 4, 0, 2, 0, "IPIX_FMT_C4",
      0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 8, 8, 8, 8 },
    { IPIX_FMT_G4, 4, 0, 3, 0, "IPIX_FMT_G4",
      0x0, 0xf, 0x0, 0x0, 0, 0, 0, 0, 8, 4, 8, 8 },
    { IPIX_FMT_A4, 4, 1, 4, 0, "IPIX_FMT_A4",
      0x0, 0x0, 0x0, 0xf, 0, 0, 0, 0, 8, 8, 8, 4 },
    { IPIX_FMT_R1G2B1, 4, 0, 1, 0, "IPIX_FMT_R1G2B1",
      0x8, 0x6, 0x1, 0x0, 3, 1, 0, 0, 7, 6, 7, 8 },
    { IPIX_FMT_B1G2R1, 4, 0, 1, 0, "IPIX_FMT_B1G2R1",
      0x1, 0x6, 0x8, 0x0, 0, 1, 3, 0, 7, 6, 7, 8 },
    { IPIX_FMT_A1R1G1B1, 4, 1, 0, 0, "IPIX_FMT_A1R1G1B1",
      0x4, 0x2, 0x1, 0x8, 2, 1, 0, 3, 7, 7, 7, 7 },
    { IPIX_FMT_A1B1G1R1, 4, 1, 0, 0, "IPIX_FMT_A1B1G1R1",
      0x1, 0x2, 0x4, 0x8, 0, 1, 2, 3, 7, 7, 7, 7 },
    { IPIX_FMT_R1G1B1A1, 4, 1, 0, 0, "IPIX_FMT_R1G1B1A1",
      0x8, 0x4, 0x2, 0x1, 3, 2, 1, 0, 7, 7, 7, 7 },
    { IPIX_FMT_B1G1R1A1, 4, 1, 0, 0, "IPIX_FMT_B1G1R1A1",
      0x2, 0x4, 0x8, 0x1, 1, 2, 3, 0, 7, 7, 7, 7 },
    { IPIX_FMT_X1R1G1B1, 4, 0, 1, 0, "IPIX_FMT_X1R1G1B1",
      0x4, 0x2, 0x1, 0x0, 2, 1, 0, 0, 7, 7, 7, 8 },
    { IPIX_FMT_X1B1G1R1, 4, 0, 1, 0, "IPIX_FMT_X1B1G1R1",
      0x1, 0x2, 0x4, 0x0, 0, 1, 2, 0, 7, 7, 7, 8 },
    { IPIX_FMT_R1G1B1X1, 4, 0, 1, 0, "IPIX_FMT_R1G1B1X1",
      0x8, 0x4, 0x2, 0x0, 3, 2, 1, 0, 7, 7, 7, 8 },
    { IPIX_FMT_B1G1R1X1, 4, 0, 1, 0, "IPIX_FMT_B1G1R1X1",
      0x2, 0x4, 0x8, 0x0, 1, 2, 3, 0, 7, 7, 7, 8 },
    { IPIX_FMT_C1, 1, 0, 2, 0, "IPIX_FMT_C1",
      0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 8, 8, 8, 8 },
    { IPIX_FMT_G1, 1, 0, 3, 0, "IPIX_FMT_G1",
      0x0, 0x1, 0x0, 0x0, 0, 0, 0, 0, 8, 7, 8, 8 },
    { IPIX_FMT_A1, 1, 1, 4, 0, "IPIX_FMT_A1",
      0x0, 0x0, 0x0, 0x1, 0, 0, 0, 0, 8, 8, 8, 7 },
};


/* lookup table for scaling 1 bit colors up to 8 bits */
const int _ipixel_scale_1[2] = { 0, 255 };

/* lookup table for scaling 2 bit colors up to 8 bits */
const int _ipixel_scale_2[4] = { 0, 85, 170, 255 };

/* lookup table for scaling 3 bit colors up to 8 bits */
const int _ipixel_scale_3[8] = { 0, 36, 72, 109, 145, 182, 218, 255 };

/* lookup table for scaling 4 bit colors up to 8 bits */
const int _ipixel_scale_4[16] = 
{
    0, 16, 32, 49, 65, 82, 98, 115, 
    139, 156, 172, 189, 205, 222, 238, 255
};

/* lookup table for scaling 5 bit colors up to 8 bits */
const int _ipixel_scale_5[32] =
{
   0,   8,   16,  24,  32,  41,  49,  57,
   65,  74,  82,  90,  98,  106, 115, 123,
   131, 139, 148, 156, 164, 172, 180, 189,
   197, 205, 213, 222, 230, 238, 246, 255
};

/* lookup table for scaling 6 bit colors up to 8 bits */
const int _ipixel_scale_6[64] =
{
   0,   4,   8,   12,  16,  20,  24,  28,
   32,  36,  40,  44,  48,  52,  56,  60,
   64,  68,  72,  76,  80,  85,  89,  93,
   97,  101, 105, 109, 113, 117, 121, 125,
   129, 133, 137, 141, 145, 149, 153, 157,
   161, 165, 170, 174, 178, 182, 186, 190,
   194, 198, 202, 206, 210, 214, 218, 222,
   226, 230, 234, 238, 242, 246, 250, 255
};

/* default color index */
static iColorIndex _ipixel_static_index[2];

iColorIndex *_ipixel_src_index = &_ipixel_static_index[0];
iColorIndex *_ipixel_dst_index = &_ipixel_static_index[1];

/* default palette */
IRGB _ipaletted[256];

/* 8 bits min/max saturation table */
const unsigned char IMINMAX256[770] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,
   14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,
   29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,
   44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
   59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,
   74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,
   89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103,
  104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118,
  119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
  134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
  149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
  164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178,
  179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193,
  194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
  209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
  224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238,
  239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253,
  254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255,
};

const unsigned char *iclip256 = &IMINMAX256[256];

unsigned char ipixel_blend_lut[2048 * 2];


#if defined(__BORLANDC__) && !defined(__MSDOS__)
	#pragma warn -8004  
	#pragma warn -8057
#endif


/**********************************************************************
 * 256 PALETTE INTERFACE
 **********************************************************************/

/* find best fit color */
int ibestfit_color(const IRGB *pal, int r, int g, int b, int palsize)
{ 
	static IUINT32 diff_lookup[512 * 3] = { 0 };
	long lowest = 0x7FFFFFFF, bestfit = 0;
	long coldiff, i, k;
	IRGB *rgb;

	/* calculate color difference lookup table:
	 * COLOR DIFF TABLE
	 * table1: diff_lookup[i | i = 256->511, n=0->(+255)] = (n * 30) ^ 2
	 * table2: diff_lookup[i | i = 256->1,   n=0->(-255)] = (n * 30) ^ 2
	 * result: f(n) = (n * 30) ^ 2 = diff_lookup[256 + n]
	 */
	if (diff_lookup[0] == 0) {
		for (i = 0; i < 256; i++) {
			k = i * i;
			diff_lookup[ 256 + i] = diff_lookup[ 256 - i] = k * 30 * 30;
			diff_lookup[ 768 + i] = diff_lookup[ 768 - i] = k * 59 * 59;
			diff_lookup[1280 + i] = diff_lookup[1280 - i] = k * 11 * 11;
		}
		diff_lookup[0] = 1;
	}

	/* range correction */
	r = r & 255;
	g = g & 255;
	b = b & 255;

	/*
	 * vector:   c1 = [r1, g1, b1], c2 = [r2, g2, b2]
	 * distance: dc = length(c1 - c2)
	 * coldiff:  dc^2 = (r1 - r2)^2 + (g1 - g2)^2 + (b1 - b2)^2
	 */
	for (i = palsize, rgb = (IRGB*)pal; i > 0; rgb++, i--) {
		coldiff  = diff_lookup[ 768 + rgb->g - g];
		if (coldiff >= lowest) continue;

		coldiff += diff_lookup[ 256 + rgb->r - r];
		if (coldiff >= lowest) continue;

		coldiff += diff_lookup[1280 + rgb->b - b];
		if (coldiff >= lowest) continue;

		bestfit = (int)(rgb - (IRGB*)pal); /* faster than `bestfit = i;' */
		if (coldiff == 0) return bestfit;
		lowest = coldiff;
	}

	return bestfit;
} 

/* convert palette to index */
int ipalette_to_index(iColorIndex *index, const IRGB *pal, int palsize)
{
	IUINT32 r, g, b, a;
	int i;
	index->color = palsize;
	for (i = 0; i < palsize; i++) {
		r = pal[i].r;
		g = pal[i].g;
		b = pal[i].b;
		a = 255;
		index->rgba[i] = IRGBA_TO_PIXEL(A8R8G8B8, r, g, b, a);
	}
	for (i = 0; i < 0x8000; i++) {
		IRGBA_FROM_PIXEL(X1R5G5B5, i, r, g, b, a);
		index->ent[i] = ibestfit_color(pal, r, g, b, palsize);
	}
	return 0;
}


/* get raw color */
IUINT32 ipixel_assemble(int pixfmt, int r, int g, int b, int a)
{
	IUINT32 c;
	IRGBA_ASSEMBLE(pixfmt, c, r, g, b, a);
	return c;
}

/* get r, g, b, a from color */
void ipixel_desemble(int pixfmt, IUINT32 c, IINT32 *r, IINT32 *g, 
	IINT32 *b, IINT32 *a)
{
	IINT32 R, G, B, A;
	IRGBA_DISEMBLE(pixfmt, c, R, G, B, A);
	*r = R;
	*g = G;
	*b = B;
	*a = A;
}


/**********************************************************************
 * Fast 1/2 byte -> 4 byte
 **********************************************************************/
void ipixel_lut_2_to_4(int sfmt, int dfmt, IUINT32 *table)
{
	IUINT32 c;

	if (ipixelfmt[sfmt].pixelbyte != 2 || ipixelfmt[dfmt].pixelbyte != 4) {
		assert(0);
		return;
	}

	for (c = 0; c < 256; c++) {
		IUINT32 r1, g1, b1, a1;
		IUINT32 r2, g2, b2, a2;
		IUINT32 c1, c2;
#if IWORDS_BIG_ENDIAN
		c1 = c << 8;
		c2 = c;
#else
		c1 = c;
		c2 = c << 8;
#endif
		IRGBA_DISEMBLE(sfmt, c1, r1, g1, b1, a1);
		IRGBA_DISEMBLE(sfmt, c2, r2, g2, b2, a2);
		IRGBA_ASSEMBLE(dfmt, c1, r1, g1, b1, a1);
		IRGBA_ASSEMBLE(dfmt, c2, r2, g2, b2, a2);
		table[c +   0] = c1;
		table[c + 256] = c2;
	}
}

void ipixel_lut_1_to_4(int sfmt, int dfmt, IUINT32 *table)
{
	IUINT32 c;

	if (ipixelfmt[sfmt].pixelbyte != 1 || ipixelfmt[dfmt].pixelbyte != 4) {
		assert(0);
		return;
	}

	for (c = 0; c < 256; c++) {
		IUINT32 c1, r1, g1, b1, a1;
		c1 = c;
		IRGBA_DISEMBLE(sfmt, c1, r1, g1, b1, a1);
		IRGBA_ASSEMBLE(dfmt, c1, r1, g1, b1, a1);
		table[c] = c1;
	}
}

void ipixel_lut_conv_2(IUINT32 *dst, const IUINT16 *src, int w, 
	const IUINT32 *lut)
{
	const IUINT8 *input = (const IUINT8*)src;
	IUINT32 c1, c2, c3, c4;
	ILINS_LOOP_DOUBLE(
		{
			c1 = lut[*input++ +   0];
			c2 = lut[*input++ + 256];
			*dst++ = c1 | c2;
		},
		{
			c1 = lut[*input++ +   0];
			c2 = lut[*input++ + 256];
			c3 = lut[*input++ +   0];
			c4 = lut[*input++ + 256];
			c1 |= c2;
			c3 |= c4;
			*dst++ = c1;
			*dst++ = c3;
		},
		w);
}

void ipixel_lut_conv_1(IUINT32 *dst, const IUINT16 *src, int w, 
	const IUINT32 *lut)
{
	const IUINT8 *input = (const IUINT8*)src;
	IUINT32 c1, c2;
	ILINS_LOOP_DOUBLE(
		{
			*dst++ = lut[*input++];
		},
		{
			c1 = lut[*input++];
			c2 = lut[*input++];
			*dst++ = c1;
			*dst++ = c2;
		},
		w);
}


/**********************************************************************
 * DEFAULT FETCH PROC
 **********************************************************************/
#define IFETCH_PROC(fmt, bpp) \
static void _ifetch_proc_##fmt(const void *bits, int x, \
    int w, IUINT32 *buffer, const iColorIndex *_ipixel_src_index) \
{ \
    IUINT32 c1, r1, g1, b1, a1, c2, r2, g2, b2, a2; \
    int y = x + 1; \
    ILINS_LOOP_DOUBLE( \
        { \
            c1 = _ipixel_fetch(bpp, bits, x); \
            x++; \
            y++; \
            IRGBA_FROM_PIXEL(fmt, c1, r1, g1, b1, a1); \
            c1 = IRGBA_TO_PIXEL(A8R8G8B8, r1, g1, b1, a1); \
            *buffer++ = c1; \
        }, \
        { \
            c1 = _ipixel_fetch(bpp, bits, x); \
            c2 = _ipixel_fetch(bpp, bits, y); \
            x += 2; \
            y += 2; \
            IRGBA_FROM_PIXEL(fmt, c1, r1, g1, b1, a1); \
            IRGBA_FROM_PIXEL(fmt, c2, r2, g2, b2, a2); \
            c1 = IRGBA_TO_PIXEL(A8R8G8B8, r1, g1, b1, a1); \
            c2 = IRGBA_TO_PIXEL(A8R8G8B8, r2, g2, b2, a2); \
            *buffer++ = c1; \
            *buffer++ = c2; \
        }, \
        w); \
    _ipixel_src_index = _ipixel_src_index; \
}


/**********************************************************************
 * DEFAULT STORE PROC
 **********************************************************************/
#define ISTORE_PROC(fmt, bpp) \
static void _istore_proc_##fmt(void *bits, const IUINT32 *values, \
    int x, int w, const iColorIndex *_ipixel_dst_index) \
{ \
    IUINT32 c1, r1, g1, b1, a1, c2, r2, g2, b2, a2; \
    int y = x + 1; \
    ILINS_LOOP_DOUBLE( \
        { \
            c1 = *values++; \
            IRGBA_FROM_PIXEL(A8R8G8B8, c1, r1, g1, b1, a1); \
            c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
            _ipixel_store(bpp, bits, x, c1); \
            x++; \
            y++; \
        }, \
        { \
            c1 = *values++; \
            c2 = *values++; \
            IRGBA_FROM_PIXEL(A8R8G8B8, c1, r1, g1, b1, a1); \
            IRGBA_FROM_PIXEL(A8R8G8B8, c2, r2, g2, b2, a2); \
            c1 = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
            c2 = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
            _ipixel_store(bpp, bits, x, c1); \
            _ipixel_store(bpp, bits, y, c2); \
            x += 2; \
            y += 2; \
        }, \
        w); \
    _ipixel_dst_index = _ipixel_dst_index; \
}


/**********************************************************************
 * DEFAULT PIXEL FETCH PROC
 **********************************************************************/
#define IFETCH_PIXEL(fmt, bpp) \
static IUINT32 _ifetch_pixel_##fmt(const void *bits, \
    int offset, const iColorIndex *_ipixel_src_index) \
{ \
    IUINT32 c, r, g, b, a; \
    c = _ipixel_fetch(bpp, bits, offset); \
    IRGBA_FROM_PIXEL(fmt, c, r, g, b, a); \
    return IRGBA_TO_PIXEL(A8R8G8B8, r, g, b, a); \
}



/**********************************************************************
 * FETCHING PROCEDURES
 **********************************************************************/
static void _ifetch_proc_A8R8G8B8(const void *bits, int x, 
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	memcpy(buffer, (const IUINT32*)bits + x, w * sizeof(IUINT32));
}

static void _ifetch_proc_A8B8G8R8(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT32 *pixel = (const IUINT32*)bits + x;
	ILINS_LOOP_DOUBLE( 
		{
			*buffer++ = ((*pixel & 0xff00ff00) |
				((*pixel >> 16) & 0xff) | ((*pixel & 0xff) << 16));
			pixel++;
		},
		{
			*buffer++ = ((*pixel & 0xff00ff00) |
				((*pixel >> 16) & 0xff) | ((*pixel & 0xff) << 16));
			pixel++;
			*buffer++ = ((*pixel & 0xff00ff00) |
				((*pixel >> 16) & 0xff) | ((*pixel & 0xff) << 16));
			pixel++;
		},
		w);
}

static void _ifetch_proc_R8G8B8A8(const void *bits, int x, 
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT32 *pixel = (const IUINT32*)bits + x;
	ILINS_LOOP_DOUBLE( 
		{
			*buffer++ = ((*pixel & 0xff) << 24) |
				((*pixel >> 8) & 0xffffff);
			pixel++;
		},
		{
			*buffer++ = ((*pixel & 0xff) << 24) |
				((*pixel >> 8) & 0xffffff);
			pixel++;
			*buffer++ = ((*pixel & 0xff) << 24) |
				((*pixel >> 8) & 0xffffff);
			pixel++;
		},
		w);
}

IFETCH_PROC(B8G8R8A8, 32)

static void _ifetch_proc_X8R8G8B8(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT32 *pixel = (const IUINT32*)bits + x;
	ILINS_LOOP_DOUBLE( 
		{
			*buffer++ = *pixel++ | 0xff000000;
		},
		{
			*buffer++ = *pixel++ | 0xff000000;
			*buffer++ = *pixel++ | 0xff000000;
		},
		w);
}

IFETCH_PROC(X8B8G8R8, 32)

static void _ifetch_proc_R8G8B8X8(const void *bits, int x, 
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT32 *pixel = (const IUINT32*)bits + x;
	ILINS_LOOP_DOUBLE( 
		{
			*buffer++ = (0xff000000) |
				((*pixel >> 8) & 0xffffff);
			pixel++;
		},
		{
			*buffer++ = (0xff000000) |
				((*pixel >> 8) & 0xffffff);
			pixel++;
			*buffer++ = (0xff000000) |
				((*pixel >> 8) & 0xffffff);
			pixel++;
		},
		w);
}

IFETCH_PROC(B8G8R8X8, 32)
IFETCH_PROC(P8R8G8B8, 32)

static void _ifetch_proc_R8G8B8(const void *bits, int x,
	int w, IUINT32 *buffer, const iColorIndex *idx)
{
	const IUINT8 *pixel = (const IUINT8*)bits + x * 3;
	ILINS_LOOP_DOUBLE( 
		{
			*buffer++ = IFETCH24(pixel) | 0xff000000;
			pixel += 3;
		},
		{
			*buffer++ = IFETCH24(pixel) | 0xff000000;
			pixel += 3;
			*buffer++ = IFETCH24(pixel) | 0xff000000;
			pixel += 3;
		},
		w);
}


IFETCH_PROC(B8G8R8, 24)

IFETCH_PROC(R5G6B5, 16)
IFETCH_PROC(B5G6R5, 16)
IFETCH_PROC(X1R5G5B5, 16)
IFETCH_PROC(X1B5G5R5, 16)
IFETCH_PROC(R5G5B5X1, 16)
IFETCH_PROC(B5G5R5X1, 16)
IFETCH_PROC(A1R5G5B5, 16)
IFETCH_PROC(A1B5G5R5, 16)
IFETCH_PROC(R5G5B5A1, 16)
IFETCH_PROC(B5G5R5A1, 16)
IFETCH_PROC(X4R4G4B4, 16)
IFETCH_PROC(X4B4G4R4, 16)
IFETCH_PROC(R4G4B4X4, 16)
IFETCH_PROC(B4G4R4X4, 16)
IFETCH_PROC(A4R4G4B4, 16)
IFETCH_PROC(A4B4G4R4, 16)
IFETCH_PROC(R4G4B4A4, 16)
IFETCH_PROC(B4G4R4A4, 16)
IFETCH_PROC(C8, 8)
IFETCH_PROC(G8, 8)
IFETCH_PROC(A8, 8)
IFETCH_PROC(R3G3B2, 8)
IFETCH_PROC(B2G3R3, 8)
IFETCH_PROC(X2R2G2B2, 8)
IFETCH_PROC(X2B2G2R2, 8)
IFETCH_PROC(R2G2B2X2, 8)
IFETCH_PROC(B2G2R2X2, 8)
IFETCH_PROC(A2R2G2B2, 8)
IFETCH_PROC(A2B2G2R2, 8)
IFETCH_PROC(R2G2B2A2, 8)
IFETCH_PROC(B2G2R2A2, 8)
IFETCH_PROC(X4C4, 8)
IFETCH_PROC(X4G4, 8)
IFETCH_PROC(X4A4, 8)
IFETCH_PROC(C4X4, 8)
IFETCH_PROC(G4X4, 8)
IFETCH_PROC(A4X4, 8)
IFETCH_PROC(C4, 4)
IFETCH_PROC(G4, 4)
IFETCH_PROC(A4, 4)
IFETCH_PROC(R1G2B1, 4)
IFETCH_PROC(B1G2R1, 4)
IFETCH_PROC(A1R1G1B1, 4)
IFETCH_PROC(A1B1G1R1, 4)
IFETCH_PROC(R1G1B1A1, 4)
IFETCH_PROC(B1G1R1A1, 4)
IFETCH_PROC(X1R1G1B1, 4)
IFETCH_PROC(X1B1G1R1, 4)
IFETCH_PROC(R1G1B1X1, 4)
IFETCH_PROC(B1G1R1X1, 4)
IFETCH_PROC(C1, 1)
IFETCH_PROC(G1, 1)
IFETCH_PROC(A1, 1)


/**********************************************************************
 * STORING PROCEDURES
 **********************************************************************/
static void _istore_proc_A8R8G8B8(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	memcpy(((IUINT32*)bits) + x, values, w * sizeof(IUINT32));
}

static void _istore_proc_A8B8G8R8(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT32 *pixel = (IUINT32*)bits + x;
	ILINS_LOOP_DOUBLE( 
		{
			*pixel++ = (values[0] & 0xff00ff00) |
				((values[0] >> 16) & 0xff) | ((values[0] & 0xff) << 16);
			values++;
		},
		{
			*pixel++ = (values[0] & 0xff00ff00) |
				((values[0] >> 16) & 0xff) | ((values[0] & 0xff) << 16);
			values++;
			*pixel++ = (values[0] & 0xff00ff00) |
				((values[0] >> 16) & 0xff) | ((values[0] & 0xff) << 16);
			values++;
		},
		w);
}

static void _istore_proc_R8G8B8A8(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT32 *pixel = (IUINT32*)bits + x;
	ILINS_LOOP_DOUBLE( 
		{
			*pixel++ = ((values[0] & 0xffffff) << 8) |
				((values[0] & 0xff000000) >> 24);
			values++;
		},
		{
			*pixel++ = ((values[0] & 0xffffff) << 8) |
				((values[0] & 0xff000000) >> 24);
			values++;
			*pixel++ = ((values[0] & 0xffffff) << 8) |
				((values[0] & 0xff000000) >> 24);
			values++;
		},
		w);
}

ISTORE_PROC(B8G8R8A8, 32)

static void _istore_proc_X8R8G8B8(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT32 *pixel = (IUINT32*)bits + x;
	ILINS_LOOP_DOUBLE( 
		{
			*pixel++ = values[0] & 0xffffff;
			values++;
		},
		{
			*pixel++ = values[0] & 0xffffff;
			values++;
			*pixel++ = values[0] & 0xffffff;
			values++;
		},
		w);
}

ISTORE_PROC(X8B8G8R8, 32)

static void _istore_proc_R8G8B8X8(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT32 *pixel = (IUINT32*)bits + x;
	ILINS_LOOP_DOUBLE( 
		{
			*pixel++ = ((values[0] & 0xffffff) << 8);
			values++;
		},
		{
			*pixel++ = ((values[0] & 0xffffff) << 8);
			values++;
			*pixel++ = ((values[0] & 0xffffff) << 8);
			values++;
		},
		w);
}

ISTORE_PROC(B8G8R8X8, 32)
ISTORE_PROC(P8R8G8B8, 32)


static void _istore_proc_R8G8B8(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x * 3;
	IUINT32 c;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISTORE24(pixel, c);
		pixel += 3;
	}
}

static void _istore_proc_B8G8R8(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT8 *pixel = (IUINT8*)bits + x * 3;
	IUINT32 c;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		c = (c & 0x00ff00) | ((c >> 16) & 0xff) | ((c << 16) & 0xff0000);
		ISTORE24(pixel, c);
		pixel += 3;
	}
}

static void _istore_proc_R5G6B5(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c1, c2, r1, g1, b1, r2, g2, b2;
	ILINS_LOOP_DOUBLE( 
		{
			c1 = *values++;
			ISPLIT_RGB(c1, r1, g1, b1);
			*pixel++ = (IUINT16) (((IUINT16)(r1 & 0xf8) << 8) |
								  ((IUINT16)(g1 & 0xfc) << 3) |
								  ((IUINT16)(b1 & 0xf8) >> 3));
		},
		{
			c1 = *values++;
			c2 = *values++;
			ISPLIT_RGB(c1, r1, g1, b1);
			ISPLIT_RGB(c2, r2, g2, b2);
			*pixel++ = (IUINT16) (((IUINT16)(r1 & 0xf8) << 8) |
								  ((IUINT16)(g1 & 0xfc) << 3) |
								  ((IUINT16)(b1 & 0xf8) >> 3));
			*pixel++ = (IUINT16) (((IUINT16)(r2 & 0xf8) << 8) |
								  ((IUINT16)(g2 & 0xfc) << 3) |
								  ((IUINT16)(b2 & 0xf8) >> 3));
		},
		w);
}

static void _istore_proc_B5G6R5(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c1, c2, r1, g1, b1, r2, g2, b2;
	ILINS_LOOP_DOUBLE( 
		{
			c1 = *values++;
			ISPLIT_RGB(c1, r1, g1, b1);
			*pixel++ = (IUINT16) (((IUINT16)(b1 & 0xf8) << 8) |
								  ((IUINT16)(g1 & 0xfc) << 3) |
								  ((IUINT16)(r1 & 0xf8) >> 3));
		},
		{
			c1 = *values++;
			c2 = *values++;
			ISPLIT_RGB(c1, r1, g1, b1);
			ISPLIT_RGB(c2, r2, g2, b2);
			*pixel++ = (IUINT16) (((IUINT16)(b1 & 0xf8) << 8) |
								  ((IUINT16)(g1 & 0xfc) << 3) |
								  ((IUINT16)(r1 & 0xf8) >> 3));
			*pixel++ = (IUINT16) (((IUINT16)(b2 & 0xf8) << 8) |
								  ((IUINT16)(g2 & 0xfc) << 3) |
								  ((IUINT16)(r2 & 0xf8) >> 3));
		},
		w);
}

static void _istore_proc_X1R5G5B5(void *bits, 
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c1, c2, r1, g1, b1, r2, g2, b2;
	ILINS_LOOP_DOUBLE( 
		{
			c1 = *values++;
			ISPLIT_RGB(c1, r1, g1, b1);
			*pixel++ = (IUINT16) (((IUINT16)(r1 & 0xf8) << 7) |
								  ((IUINT16)(g1 & 0xf8) << 2) |
								  ((IUINT16)(b1 & 0xf8) >> 3));
		},
		{
			c1 = *values++;
			c2 = *values++;
			ISPLIT_RGB(c1, r1, g1, b1);
			ISPLIT_RGB(c2, r2, g2, b2);
			*pixel++ = (IUINT16) (((IUINT16)(r1 & 0xf8) << 7) |
								  ((IUINT16)(g1 & 0xf8) << 2) |
								  ((IUINT16)(b1 & 0xf8) >> 3));
			*pixel++ = (IUINT16) (((IUINT16)(r2 & 0xf8) << 7) |
								  ((IUINT16)(g2 & 0xf8) << 2) |
								  ((IUINT16)(b2 & 0xf8) >> 3));
		},
		w);
}

ISTORE_PROC(X1B5G5R5, 16)
ISTORE_PROC(R5G5B5X1, 16)
ISTORE_PROC(B5G5R5X1, 16)

ISTORE_PROC(A1R5G5B5, 16)
ISTORE_PROC(A1B5G5R5, 16)
ISTORE_PROC(R5G5B5A1, 16)
ISTORE_PROC(B5G5R5A1, 16)
ISTORE_PROC(X4R4G4B4, 16)
ISTORE_PROC(X4B4G4R4, 16)
ISTORE_PROC(R4G4B4X4, 16)
ISTORE_PROC(B4G4R4X4, 16)

ISTORE_PROC(A4R4G4B4, 16)
ISTORE_PROC(A4B4G4R4, 16)

static void _istore_proc_R4G4B4A4(void *bits,
	const IUINT32 *values, int x, int w, const iColorIndex *idx)
{
	IUINT16 *pixel = (IUINT16*)bits + x;
	IUINT32 c, a, r, g, b;
	int i;
	for (i = w; i > 0; i--) {
		c = *values++;
		ISPLIT_ARGB(c, a, r, g, b);
		*pixel++ = ((IUINT16)( 
			((IUINT16)((r) & 0xf0) << 8) | 
			((IUINT16)((g) & 0xf0) << 4) | 
			((IUINT16)((b) & 0xf0) >> 0) | 
			((IUINT16)((a) & 0xf0) >> 4)));
	}
}

ISTORE_PROC(B4G4R4A4, 16)
ISTORE_PROC(C8, 8)
ISTORE_PROC(G8, 8)
ISTORE_PROC(A8, 8)
ISTORE_PROC(R3G3B2, 8)
ISTORE_PROC(B2G3R3, 8)
ISTORE_PROC(X2R2G2B2, 8)
ISTORE_PROC(X2B2G2R2, 8)
ISTORE_PROC(R2G2B2X2, 8)
ISTORE_PROC(B2G2R2X2, 8)
ISTORE_PROC(A2R2G2B2, 8)
ISTORE_PROC(A2B2G2R2, 8)
ISTORE_PROC(R2G2B2A2, 8)
ISTORE_PROC(B2G2R2A2, 8)
ISTORE_PROC(X4C4, 8)
ISTORE_PROC(X4G4, 8)
ISTORE_PROC(X4A4, 8)
ISTORE_PROC(C4X4, 8)
ISTORE_PROC(G4X4, 8)
ISTORE_PROC(A4X4, 8)
ISTORE_PROC(C4, 4)
ISTORE_PROC(G4, 4)
ISTORE_PROC(A4, 4)
ISTORE_PROC(R1G2B1, 4)
ISTORE_PROC(B1G2R1, 4)
ISTORE_PROC(A1R1G1B1, 4)
ISTORE_PROC(A1B1G1R1, 4)
ISTORE_PROC(R1G1B1A1, 4)
ISTORE_PROC(B1G1R1A1, 4)
ISTORE_PROC(X1R1G1B1, 4)
ISTORE_PROC(X1B1G1R1, 4)
ISTORE_PROC(R1G1B1X1, 4)
ISTORE_PROC(B1G1R1X1, 4)
ISTORE_PROC(C1, 1)
ISTORE_PROC(G1, 1)
ISTORE_PROC(A1, 1)


/**********************************************************************
 * PIXEL FETCHING PROCEDURES
 **********************************************************************/

static IUINT32 _ifetch_pixel_A8R8G8B8(const void *bits, 
    int offset, const iColorIndex *_ipixel_src_index) 
{ 
    return _ipixel_fetch(32, bits, offset); 
}

static IUINT32 _ifetch_pixel_A8B8G8R8(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 pixel = ((const IUINT32*)(bits))[offset];
	return ((pixel & 0xff00ff00) | ((pixel >> 16) & 0xff) | 
		((pixel & 0xff) << 16));
}

static IUINT32 _ifetch_pixel_R8G8B8A8(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 pixel = ((const IUINT32*)(bits))[offset];
	return ((pixel & 0xff) << 24) | ((pixel >> 8) & 0xffffff);
}

static IUINT32 _ifetch_pixel_B8G8R8A8(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 pixel = ((const IUINT32*)(bits))[offset];
	return		((pixel & 0x000000ff) << 24) |
				((pixel & 0x0000ff00) <<  8) |
				((pixel & 0x00ff0000) >>  8) |
				((pixel & 0xff000000) >> 24);
}


static IUINT32 _ifetch_pixel_X8R8G8B8(const void *bits,
	int offset, const iColorIndex *idx)
{
	return ((const IUINT32*)(bits))[offset] | 0xff000000;
}

static IUINT32 _ifetch_pixel_X8B8G8R8(const void *bits,
	int offset, const iColorIndex *idx)
{
	IUINT32 pixel = ((const IUINT32*)(bits))[offset];
	return ((pixel & 0x0000ff00) | ((pixel >> 16) & 0xff) | 
		((pixel & 0xff) << 16)) | 0xff000000;
}

IFETCH_PIXEL(R8G8B8X8, 32)
IFETCH_PIXEL(B8G8R8X8, 32)
IFETCH_PIXEL(P8R8G8B8, 32)

IFETCH_PIXEL(R8G8B8, 24)
IFETCH_PIXEL(B8G8R8, 24)
IFETCH_PIXEL(R5G6B5, 16)
IFETCH_PIXEL(B5G6R5, 16)
IFETCH_PIXEL(X1R5G5B5, 16)
IFETCH_PIXEL(X1B5G5R5, 16)
IFETCH_PIXEL(R5G5B5X1, 16)
IFETCH_PIXEL(B5G5R5X1, 16)
IFETCH_PIXEL(A1R5G5B5, 16)
IFETCH_PIXEL(A1B5G5R5, 16)
IFETCH_PIXEL(R5G5B5A1, 16)
IFETCH_PIXEL(B5G5R5A1, 16)
IFETCH_PIXEL(X4R4G4B4, 16)
IFETCH_PIXEL(X4B4G4R4, 16)
IFETCH_PIXEL(R4G4B4X4, 16)
IFETCH_PIXEL(B4G4R4X4, 16)
IFETCH_PIXEL(A4R4G4B4, 16)
IFETCH_PIXEL(A4B4G4R4, 16)
IFETCH_PIXEL(R4G4B4A4, 16)
IFETCH_PIXEL(B4G4R4A4, 16)
IFETCH_PIXEL(C8, 8)
IFETCH_PIXEL(G8, 8)
IFETCH_PIXEL(A8, 8)
IFETCH_PIXEL(R3G3B2, 8)
IFETCH_PIXEL(B2G3R3, 8)
IFETCH_PIXEL(X2R2G2B2, 8)
IFETCH_PIXEL(X2B2G2R2, 8)
IFETCH_PIXEL(R2G2B2X2, 8)
IFETCH_PIXEL(B2G2R2X2, 8)
IFETCH_PIXEL(A2R2G2B2, 8)
IFETCH_PIXEL(A2B2G2R2, 8)
IFETCH_PIXEL(R2G2B2A2, 8)
IFETCH_PIXEL(B2G2R2A2, 8)
IFETCH_PIXEL(X4C4, 8)
IFETCH_PIXEL(X4G4, 8)
IFETCH_PIXEL(X4A4, 8)
IFETCH_PIXEL(C4X4, 8)
IFETCH_PIXEL(G4X4, 8)
IFETCH_PIXEL(A4X4, 8)
IFETCH_PIXEL(C4, 4)
IFETCH_PIXEL(G4, 4)
IFETCH_PIXEL(A4, 4)
IFETCH_PIXEL(R1G2B1, 4)
IFETCH_PIXEL(B1G2R1, 4)
IFETCH_PIXEL(A1R1G1B1, 4)
IFETCH_PIXEL(A1B1G1R1, 4)
IFETCH_PIXEL(R1G1B1A1, 4)
IFETCH_PIXEL(B1G1R1A1, 4)
IFETCH_PIXEL(X1R1G1B1, 4)
IFETCH_PIXEL(X1B1G1R1, 4)
IFETCH_PIXEL(R1G1B1X1, 4)
IFETCH_PIXEL(B1G1R1X1, 4)
IFETCH_PIXEL(C1, 1)
IFETCH_PIXEL(G1, 1)
IFETCH_PIXEL(A1, 1)



/**********************************************************************
 * FETCHING STORING LOOK UP TABLE
 **********************************************************************/
struct iPixelAccessProc
{
    iFetchProc fetch, fetch_default;
    iStoreProc store, store_default;
    iFetchPixelProc fetchpixel, fetchpixel_default;
};

#define ITABLE_ITEM(fmt) { \
		_ifetch_proc_##fmt, _ifetch_proc_##fmt, \
		_istore_proc_##fmt, _istore_proc_##fmt, \
		_ifetch_pixel_##fmt, _ifetch_pixel_##fmt }


/* builtin look table */
static struct iPixelAccessProc ipixel_access_proc[IPIX_FMT_COUNT] =
{
	ITABLE_ITEM(A8R8G8B8),
	ITABLE_ITEM(A8B8G8R8),
	ITABLE_ITEM(R8G8B8A8),
	ITABLE_ITEM(B8G8R8A8),
	ITABLE_ITEM(X8R8G8B8),
	ITABLE_ITEM(X8B8G8R8),
	ITABLE_ITEM(R8G8B8X8),
	ITABLE_ITEM(B8G8R8X8),
	ITABLE_ITEM(P8R8G8B8),
	ITABLE_ITEM(R8G8B8),
	ITABLE_ITEM(B8G8R8),
	ITABLE_ITEM(R5G6B5),
	ITABLE_ITEM(B5G6R5),
	ITABLE_ITEM(X1R5G5B5),
	ITABLE_ITEM(X1B5G5R5),
	ITABLE_ITEM(R5G5B5X1),
	ITABLE_ITEM(B5G5R5X1),
	ITABLE_ITEM(A1R5G5B5),
	ITABLE_ITEM(A1B5G5R5),
	ITABLE_ITEM(R5G5B5A1),
	ITABLE_ITEM(B5G5R5A1),
	ITABLE_ITEM(X4R4G4B4),
	ITABLE_ITEM(X4B4G4R4),
	ITABLE_ITEM(R4G4B4X4),
	ITABLE_ITEM(B4G4R4X4),
	ITABLE_ITEM(A4R4G4B4),
	ITABLE_ITEM(A4B4G4R4),
	ITABLE_ITEM(R4G4B4A4),
	ITABLE_ITEM(B4G4R4A4),
	ITABLE_ITEM(C8),
	ITABLE_ITEM(G8),
	ITABLE_ITEM(A8),
	ITABLE_ITEM(R3G3B2),
	ITABLE_ITEM(B2G3R3),
	ITABLE_ITEM(X2R2G2B2),
	ITABLE_ITEM(X2B2G2R2),
	ITABLE_ITEM(R2G2B2X2),
	ITABLE_ITEM(B2G2R2X2),
	ITABLE_ITEM(A2R2G2B2),
	ITABLE_ITEM(A2B2G2R2),
	ITABLE_ITEM(R2G2B2A2),
	ITABLE_ITEM(B2G2R2A2),
	ITABLE_ITEM(X4C4),
	ITABLE_ITEM(X4G4),
	ITABLE_ITEM(X4A4),
	ITABLE_ITEM(C4X4),
	ITABLE_ITEM(G4X4),
	ITABLE_ITEM(A4X4),
	ITABLE_ITEM(C4),
	ITABLE_ITEM(G4),
	ITABLE_ITEM(A4),
	ITABLE_ITEM(R1G2B1),
	ITABLE_ITEM(B1G2R1),
	ITABLE_ITEM(A1R1G1B1),
	ITABLE_ITEM(A1B1G1R1),
	ITABLE_ITEM(R1G1B1A1),
	ITABLE_ITEM(B1G1R1A1),
	ITABLE_ITEM(X1R1G1B1),
	ITABLE_ITEM(X1B1G1R1),
	ITABLE_ITEM(R1G1B1X1),
	ITABLE_ITEM(B1G1R1X1),
	ITABLE_ITEM(C1),
	ITABLE_ITEM(G1),
	ITABLE_ITEM(A1),
};


#undef ITABLE_ITEM
#undef IFETCH_PROC
#undef ISTORE_PROC
#undef IFETCH_PIXEL


/* set procedure */
void ipixel_set_proc(int pixfmt, int type, void *proc)
{
	assert(pixfmt >= 0 && pixfmt < IPIX_FMT_COUNT);
	if (pixfmt < 0 || pixfmt >= IPIX_FMT_COUNT) return;
	if (type == IPIXEL_PROC_TYPE_FETCH) {
		ipixel_access_proc[pixfmt].fetch = (iFetchProc)proc;
	}
	else if (type == IPIXEL_PROC_TYPE_STORE) {
		ipixel_access_proc[pixfmt].store = (iStoreProc)proc;
	}
	else if (type == IPIXEL_PROC_TYPE_FETCHPIXEL) {
		ipixel_access_proc[pixfmt].fetchpixel = (iFetchPixelProc)proc;
	}
}



/**********************************************************************
 * FETCHING STORING LOOK UP TABLE
 **********************************************************************/
#define IFETCH_LUT_2(sfmt) \
static IUINT32 _ipixel_lut_##sfmt[256 * 2]; \
static void _ifetch_proc_lut_##sfmt(const void *bits, int x, \
    int w, IUINT32 *buffer, const iColorIndex *idx) \
{ \
	const IUINT8 *input = (const IUINT8*)bits + (x << 1); \
	IUINT32 c1, c2, c3, c4; \
	ILINS_LOOP_DOUBLE( \
		{ \
			c1 = _ipixel_lut_##sfmt[*input++ +   0]; \
			c2 = _ipixel_lut_##sfmt[*input++ + 256]; \
			*buffer++ = c1 | c2; \
		}, \
		{ \
			c1 = _ipixel_lut_##sfmt[*input++ +   0]; \
			c2 = _ipixel_lut_##sfmt[*input++ + 256]; \
			c3 = _ipixel_lut_##sfmt[*input++ +   0]; \
			c4 = _ipixel_lut_##sfmt[*input++ + 256]; \
			c1 |= c2; \
			c3 |= c4; \
			*buffer++ = c1; \
			*buffer++ = c3; \
		}, \
		w); \
} \
static IUINT32 _ifetch_pixel_lut_##sfmt(const void *bits, \
    int offset, const iColorIndex *idx) \
{ \
	const IUINT8 *input = (const IUINT8*)bits + (offset << 1); \
	IUINT32 c1, c2; \
	c1 = _ipixel_lut_##sfmt[*input++ +   0]; \
	c2 = _ipixel_lut_##sfmt[*input++ + 256]; \
	return c1 | c2; \
}


#define IFETCH_LUT_1(sfmt) \
static IUINT32 _ipixel_lut_##sfmt[256]; \
static void _ifetch_proc_lut_##sfmt(const void *bits, int x, \
    int w, IUINT32 *buffer, const iColorIndex *idx) \
{ \
	const IUINT8 *input = (const IUINT8*)bits + x; \
	IUINT32 c1, c2; \
	ILINS_LOOP_DOUBLE( \
		{ \
			c1 = _ipixel_lut_##sfmt[*input++]; \
			*buffer++ = c1; \
		}, \
		{ \
			c1 = _ipixel_lut_##sfmt[*input++]; \
			c2 = _ipixel_lut_##sfmt[*input++]; \
			*buffer++ = c1; \
			*buffer++ = c2; \
		}, \
		w); \
} \
static IUINT32 _ifetch_pixel_lut_##sfmt(const void *bits, \
    int offset, const iColorIndex *idx) \
{ \
	const IUINT8 *input = (const IUINT8*)bits + offset; \
	return _ipixel_lut_##sfmt[*input]; \
}

#define IFETCH_LUT_MAIN(sfmt, nbytes) \
	IFETCH_LUT_##nbytes(sfmt)


IFETCH_LUT_MAIN(R5G6B5, 2)
IFETCH_LUT_MAIN(B5G6R5, 2)
IFETCH_LUT_MAIN(X1R5G5B5, 2)
IFETCH_LUT_MAIN(X1B5G5R5, 2)
IFETCH_LUT_MAIN(R5G5B5X1, 2)
IFETCH_LUT_MAIN(B5G5R5X1, 2)
IFETCH_LUT_MAIN(A1R5G5B5, 2)
IFETCH_LUT_MAIN(A1B5G5R5, 2)
IFETCH_LUT_MAIN(R5G5B5A1, 2)
IFETCH_LUT_MAIN(B5G5R5A1, 2)
IFETCH_LUT_MAIN(X4R4G4B4, 2)
IFETCH_LUT_MAIN(X4B4G4R4, 2)
IFETCH_LUT_MAIN(R4G4B4X4, 2)
IFETCH_LUT_MAIN(B4G4R4X4, 2)
IFETCH_LUT_MAIN(A4R4G4B4, 2)
IFETCH_LUT_MAIN(A4B4G4R4, 2)
IFETCH_LUT_MAIN(R4G4B4A4, 2)
IFETCH_LUT_MAIN(B4G4R4A4, 2)
IFETCH_LUT_MAIN(R3G3B2, 1)
IFETCH_LUT_MAIN(B2G3R3, 1)
IFETCH_LUT_MAIN(X2R2G2B2, 1)
IFETCH_LUT_MAIN(X2B2G2R2, 1)
IFETCH_LUT_MAIN(R2G2B2X2, 1)
IFETCH_LUT_MAIN(B2G2R2X2, 1)
IFETCH_LUT_MAIN(A2R2G2B2, 1)
IFETCH_LUT_MAIN(A2B2G2R2, 1)
IFETCH_LUT_MAIN(R2G2B2A2, 1)
IFETCH_LUT_MAIN(B2G2R2A2, 1)


struct iPixelAccessLutTable
{
	int fmt;
	iFetchProc fetch;
	iFetchPixelProc pixel;
};


#define ITABLE_ITEM(fmt) \
	{ IPIX_FMT_##fmt, _ifetch_proc_lut_##fmt, _ifetch_pixel_lut_##fmt }

static struct iPixelAccessLutTable ipixel_access_lut[28] =
{
	ITABLE_ITEM(R5G6B5),
	ITABLE_ITEM(B5G6R5),
	ITABLE_ITEM(X1R5G5B5),
	ITABLE_ITEM(X1B5G5R5),
	ITABLE_ITEM(R5G5B5X1),
	ITABLE_ITEM(B5G5R5X1),
	ITABLE_ITEM(A1R5G5B5),
	ITABLE_ITEM(A1B5G5R5),
	ITABLE_ITEM(R5G5B5A1),
	ITABLE_ITEM(B5G5R5A1),
	ITABLE_ITEM(X4R4G4B4),
	ITABLE_ITEM(X4B4G4R4),
	ITABLE_ITEM(R4G4B4X4),
	ITABLE_ITEM(B4G4R4X4),
	ITABLE_ITEM(A4R4G4B4),
	ITABLE_ITEM(A4B4G4R4),
	ITABLE_ITEM(R4G4B4A4),
	ITABLE_ITEM(B4G4R4A4),
	ITABLE_ITEM(R3G3B2),
	ITABLE_ITEM(B2G3R3),
	ITABLE_ITEM(X2R2G2B2),
	ITABLE_ITEM(X2B2G2R2),
	ITABLE_ITEM(R2G2B2X2),
	ITABLE_ITEM(B2G2R2X2),
	ITABLE_ITEM(A2R2G2B2),
	ITABLE_ITEM(A2B2G2R2),
	ITABLE_ITEM(R2G2B2A2),
	ITABLE_ITEM(B2G2R2A2),
};

static const int ipixel_access_lut_fmt[IPIX_FMT_COUNT] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 
	9, 10, 11, 12, 13, 14, 15, 16, 17, -1, -1, -1, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1 
};


#undef ITABLE_ITEM
#undef IFETCH_LUT_2
#undef IFETCH_LUT_1
#undef IFETCH_LUT_MAIN

static int ipixel_lut_inited = 0;

void ipixel_lut_init(void)
{
	int i;

	if (ipixel_lut_inited != 0) return;

	#define IPIXEL_LUT_INIT(fmt, nbytes) \
		ipixel_lut_##nbytes##_to_4(IPIX_FMT_##fmt, IPIX_FMT_A8R8G8B8, \
			_ipixel_lut_##fmt)

	IPIXEL_LUT_INIT(R5G6B5, 2);
	IPIXEL_LUT_INIT(B5G6R5, 2);
	IPIXEL_LUT_INIT(X1R5G5B5, 2);
	IPIXEL_LUT_INIT(X1B5G5R5, 2);
	IPIXEL_LUT_INIT(R5G5B5X1, 2);
	IPIXEL_LUT_INIT(B5G5R5X1, 2);
	IPIXEL_LUT_INIT(A1R5G5B5, 2);
	IPIXEL_LUT_INIT(A1B5G5R5, 2);
	IPIXEL_LUT_INIT(R5G5B5A1, 2);
	IPIXEL_LUT_INIT(B5G5R5A1, 2);
	IPIXEL_LUT_INIT(X4R4G4B4, 2);
	IPIXEL_LUT_INIT(X4B4G4R4, 2);
	IPIXEL_LUT_INIT(R4G4B4X4, 2);
	IPIXEL_LUT_INIT(B4G4R4X4, 2);
	IPIXEL_LUT_INIT(A4R4G4B4, 2);
	IPIXEL_LUT_INIT(A4B4G4R4, 2);
	IPIXEL_LUT_INIT(R4G4B4A4, 2);
	IPIXEL_LUT_INIT(B4G4R4A4, 2);
	IPIXEL_LUT_INIT(R3G3B2, 1);
	IPIXEL_LUT_INIT(B2G3R3, 1);
	IPIXEL_LUT_INIT(X2R2G2B2, 1);
	IPIXEL_LUT_INIT(X2B2G2R2, 1);
	IPIXEL_LUT_INIT(R2G2B2X2, 1);
	IPIXEL_LUT_INIT(B2G2R2X2, 1);
	IPIXEL_LUT_INIT(A2R2G2B2, 1);
	IPIXEL_LUT_INIT(A2B2G2R2, 1);
	IPIXEL_LUT_INIT(R2G2B2A2, 1);
	IPIXEL_LUT_INIT(B2G2R2A2, 1);

	#undef IPIXEL_LUT_INIT
	
	for (i = 0; i < 2048; i++) {
		IUINT32 da = _ipixel_scale_6[i >> 6];
		IUINT32 sa = _ipixel_scale_6[i & 63];
		IUINT32 FA = da + ((255 - da) * sa) / 255;
		IUINT32 SA = (FA != 0)? ((sa * 255) / FA) : 0;
		ipixel_blend_lut[i * 2 + 0] = (unsigned char)SA;
		ipixel_blend_lut[i * 2 + 1] = (unsigned char)FA;
	}

	ipixel_lut_inited = 1;
}

/* get color fetching procedure */
iFetchProc ipixel_get_fetch(int pixfmt, int access_mode)
{
	assert(pixfmt >= 0 && pixfmt < IPIX_FMT_COUNT);
	if (pixfmt < 0 || pixfmt >= IPIX_FMT_COUNT) return NULL;
	if (access_mode == IPIXEL_ACCESS_MODE_NORMAL) {
		int id = ipixel_access_lut_fmt[pixfmt];
		if (ipixel_lut_inited == 0) ipixel_lut_init();
		if (id >= 0) return ipixel_access_lut[id].fetch;
		return ipixel_access_proc[pixfmt].fetch;
	}
	if (access_mode == IPIXEL_ACCESS_MODE_ACCURATE) {
		return ipixel_access_proc[pixfmt].fetch;
	}
	return ipixel_access_proc[pixfmt].fetch_default;
}

/* get color storing procedure */
iStoreProc ipixel_get_store(int pixfmt, int access_mode)
{
	assert(pixfmt >= 0 && pixfmt < IPIX_FMT_COUNT);
	if (pixfmt < 0 || pixfmt >= IPIX_FMT_COUNT) return NULL;
	if (access_mode == IPIXEL_ACCESS_MODE_NORMAL) {
		if (ipixel_lut_inited == 0) ipixel_lut_init();
		return ipixel_access_proc[pixfmt].store;
	}
	if (access_mode == IPIXEL_ACCESS_MODE_ACCURATE) {
		return ipixel_access_proc[pixfmt].store;
	}
	return ipixel_access_proc[pixfmt].store_default;
}

/* get color pixel fetching procedure */
iFetchPixelProc ipixel_get_fetchpixel(int pixfmt, int access_mode)
{
	assert(pixfmt >= 0 && pixfmt < IPIX_FMT_COUNT);
	if (pixfmt < 0 || pixfmt >= IPIX_FMT_COUNT) return NULL;
	if (access_mode == IPIXEL_ACCESS_MODE_NORMAL) {
		int id = ipixel_access_lut_fmt[pixfmt];
		if (ipixel_lut_inited == 0) ipixel_lut_init();
		if (id >= 0) return ipixel_access_lut[id].pixel;
		return ipixel_access_proc[pixfmt].fetchpixel;
	}
	if (access_mode == IPIXEL_ACCESS_MODE_ACCURATE) {
		return ipixel_access_proc[pixfmt].fetchpixel;
	}
	return ipixel_access_proc[pixfmt].fetchpixel_default;
}


/* returns pixel format names */
const char *ipixelfmt_name(int fmt)
{
	return ipixelfmt[fmt].name;
}


/**********************************************************************
 * SPAN DRAWING
 **********************************************************************/
/* span blending for 8, 16, 24, 32 bits */
#define IPIXEL_SPAN_DRAW_PROC_N(fmt, bpp, nbytes, mode) \
static void ipixel_span_draw_proc_##fmt##_0(void *bits, \
	int offset, int w, const IUINT32 *card, const IUINT8 *cover, \
	const iColorIndex *_ipixel_src_index) \
{ \
	unsigned char *dst = ((unsigned char*)bits) + offset * nbytes; \
	IUINT32 cc, r1, g1, b1, a1, r2, g2, b2, a2, inc; \
	if (cover == NULL) { \
		for (inc = w; inc > 0; inc--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			if (a1 == 255) { \
				cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			else if (a1 > 0) { \
				cc = _ipixel_fetch(bpp, dst, 0); \
				IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
				IBLEND_##mode(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			card++; \
			dst += nbytes; \
		} \
	}	else { \
		for (inc = w; inc > 0; inc--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			cc = *cover++; \
			r2 = a1 + cc; \
			if (r2 == 510) { \
				cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			else if (r2 > 0 && cc > 0) { \
				a1 = _imul_y_div_255(a1, cc); \
				cc = _ipixel_fetch(bpp, dst, 0); \
				IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
				IBLEND_##mode(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			card++; \
			dst += nbytes; \
		} \
	} \
} \
static void ipixel_span_draw_proc_##fmt##_1(void *bits, \
	int offset, int w, const IUINT32 *card, const IUINT8 *cover, \
	const iColorIndex *_ipixel_src_index) \
{ \
	unsigned char *dst = ((unsigned char*)bits) + offset * nbytes; \
	IUINT32 cc, r1, g1, b1, a1, r2, g2, b2, a2, inc; \
	if (cover == NULL) { \
		for (inc = w; inc > 0; inc--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			if (a1 > 0) { \
				cc = _ipixel_fetch(bpp, dst, 0); \
				IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
				IBLEND_ADDITIVE(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			card++; \
			dst += nbytes; \
		} \
	}	else { \
		for (inc = w; inc > 0; inc--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			cc = *cover++; \
			if (a1 > 0 && cc > 0) { \
				a1 = _imul_y_div_255(a1, cc); \
				cc = _ipixel_fetch(bpp, dst, 0); \
				IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
				IBLEND_ADDITIVE(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			card++; \
			dst += nbytes; \
		} \
	} \
}

/* span blending for 8 bits without palette */
#define IPIXEL_SPAN_DRAW_PROC_1(fmt, bpp, nbytes, mode) \
static void ipixel_span_draw_proc_##fmt##_0(void *bits, \
	int offset, int w, const IUINT32 *card, const IUINT8 *cover, \
	const iColorIndex *_ipixel_src_index) \
{ \
	unsigned char *dst = ((unsigned char*)bits) + (offset); \
	IUINT32 cc, r1, g1, b1, a1, r2, g2, b2, a2, inc; \
	if (cover == NULL) { \
		for (inc = w; inc > 0; inc--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			if (a1 == 255) { \
				cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			else if (a1 > 0) { \
				r1 = dst[0]; \
				cc = _ipixel_lut_##fmt[r1]; \
				IRGBA_FROM_PIXEL(A8R8G8B8, cc, r2, g2, b2, a2); \
				IBLEND_##mode(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			card++; \
			dst++; \
		} \
	}	else { \
		for (inc = w; inc > 0; inc--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			cc = *cover++; \
			r2 = a1 + cc; \
			if (r2 == 510) { \
				cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			else if (r2 > 0 && cc > 0) { \
				a1 = _imul_y_div_255(a1, cc); \
				r1 = dst[0]; \
				cc = _ipixel_lut_##fmt[r1]; \
				IRGBA_FROM_PIXEL(A8R8G8B8, cc, r2, g2, b2, a2); \
				IBLEND_##mode(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			card++; \
			dst++; \
		} \
	} \
} \
static void ipixel_span_draw_proc_##fmt##_1(void *bits, \
	int offset, int w, const IUINT32 *card, const IUINT8 *cover, \
	const iColorIndex *_ipixel_src_index) \
{ \
	unsigned char *dst = ((unsigned char*)bits) + (offset); \
	IUINT32 cc, r1, g1, b1, a1, r2, g2, b2, a2, inc; \
	if (cover == NULL) { \
		for (inc = w; inc > 0; inc--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			if (a1 > 0) { \
				r1 = dst[0]; \
				cc = _ipixel_lut_##fmt[r1]; \
				IRGBA_FROM_PIXEL(A8R8G8B8, cc, r2, g2, b2, a2); \
				IBLEND_ADDITIVE(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			card++; \
			dst++; \
		} \
	}	else { \
		for (inc = w; inc > 0; inc--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			cc = *cover++; \
			if (a1 > 0 && cc > 0) { \
				a1 = _imul_y_div_255(a1, cc); \
				r1 = dst[0]; \
				cc = _ipixel_lut_##fmt[r1]; \
				IRGBA_FROM_PIXEL(A8R8G8B8, cc, r2, g2, b2, a2); \
				IBLEND_ADDITIVE(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
			card++; \
			dst++; \
		} \
	} \
}

/* span blending for 8/4/1 bits with or without palette */
#define IPIXEL_SPAN_DRAW_PROC_X(fmt, bpp, nbytes, mode, init) \
static void ipixel_span_draw_proc_##fmt##_0(void *bits, \
	int offset, int w, const IUINT32 *card, const IUINT8 *cover, \
	const iColorIndex *_ipixel_src_index) \
{ \
	IUINT32 cc, r1, g1, b1, a1, r2, g2, b2, a2, inc; \
	unsigned char *dst = (unsigned char*)bits; \
	init; \
	if (cover == NULL) { \
		for (inc = offset; w > 0; inc++, w--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			if (a1 == 255) { \
				cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
				_ipixel_store(bpp, dst, inc, cc); \
			} \
			else if (a1 > 0) { \
				cc = _ipixel_fetch(bpp, dst, inc); \
				IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
				IBLEND_##mode(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, inc, cc); \
			} \
			card++; \
		} \
	}	else { \
		for (inc = offset; w > 0; inc++, w--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			cc = *cover++; \
			r2 = a1 + cc; \
			if (r2 == 510) { \
				cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
				_ipixel_store(bpp, dst, inc, cc); \
			} \
			else if (r2 > 0 && cc > 0) { \
				a1 = _imul_y_div_255(a1, cc); \
				cc = _ipixel_fetch(bpp, dst, inc); \
				IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
				IBLEND_##mode(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, inc, cc); \
			} \
			card++; \
		} \
	} \
} \
static void ipixel_span_draw_proc_##fmt##_1(void *bits, \
	int offset, int w, const IUINT32 *card, const IUINT8 *cover, \
	const iColorIndex *_ipixel_src_index) \
{ \
	IUINT32 cc, r1, g1, b1, a1, r2, g2, b2, a2, inc; \
	unsigned char *dst = (unsigned char*)bits; \
	init; \
	if (cover == NULL) { \
		for (inc = offset; w > 0; inc++, w--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			if (a1 > 0) { \
				cc = _ipixel_fetch(bpp, dst, inc); \
				IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
				IBLEND_ADDITIVE(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, inc, cc); \
			} \
			card++; \
		} \
	}	else { \
		for (inc = offset; w > 0; inc++, w--) { \
			_ipixel_load_card(card, r1, g1, b1, a1); \
			cc = *cover++; \
			if (r1 > 0 && cc > 0) { \
				a1 = _imul_y_div_255(a1, cc); \
				cc = _ipixel_fetch(bpp, dst, inc); \
				IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
				IBLEND_ADDITIVE(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, inc, cc); \
			} \
			card++; \
		} \
	} \
}

/* span blending for 4/1 bits without palette */
#define IPIXEL_SPAN_DRAW_PROC_BITS(fmt, bpp, nbytes, mode) \
		IPIXEL_SPAN_DRAW_PROC_X(fmt, bpp, nbytes, mode, {}) 

/* span blending for 8/4/1 bits with palette */
#define IPIXEL_SPAN_DRAW_PROC_PAL(fmt, bpp, nbytes, mode) \
		IPIXEL_SPAN_DRAW_PROC_X(fmt, bpp, nbytes, mode, \
			const iColorIndex *_ipixel_dst_index = _ipixel_src_index)

/* span blending main */
#define IPIXEL_SPAN_DRAW_MAIN(type, fmt, bpp, nbytes, mode) \
	IPIXEL_SPAN_DRAW_PROC_##type(fmt, bpp, nbytes, mode) 

/* span blending procedures declare */
IPIXEL_SPAN_DRAW_MAIN(N, A8R8G8B8, 32, 4, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, A8B8G8R8, 32, 4, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, R8G8B8A8, 32, 4, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, B8G8R8A8, 32, 4, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, X8R8G8B8, 32, 4, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, X8B8G8R8, 32, 4, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, R8G8B8X8, 32, 4, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, B8G8R8X8, 32, 4, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, P8R8G8B8, 32, 4, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, R8G8B8, 24, 3, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, B8G8R8, 24, 3, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, R5G6B5, 16, 2, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, B5G6R5, 16, 2, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, X1R5G5B5, 16, 2, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, X1B5G5R5, 16, 2, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, R5G5B5X1, 16, 2, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, B5G5R5X1, 16, 2, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, A1R5G5B5, 16, 2, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, A1B5G5R5, 16, 2, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, R5G5B5A1, 16, 2, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, B5G5R5A1, 16, 2, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, X4R4G4B4, 16, 2, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, X4B4G4R4, 16, 2, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, R4G4B4X4, 16, 2, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, B4G4R4X4, 16, 2, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, A4R4G4B4, 16, 2, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, A4B4G4R4, 16, 2, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, R4G4B4A4, 16, 2, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(N, B4G4R4A4, 16, 2, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(PAL, C8, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, G8, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, A8, 8, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(1, R3G3B2, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(1, B2G3R3, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(1, X2R2G2B2, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(1, X2B2G2R2, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(1, R2G2B2X2, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(1, B2G2R2X2, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(1, A2R2G2B2, 8, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(1, A2B2G2R2, 8, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(1, R2G2B2A2, 8, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(1, B2G2R2A2, 8, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(PAL, X4C4, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, X4G4, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, X4A4, 8, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(PAL, C4X4, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, G4X4, 8, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(N, A4X4, 8, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(PAL, C4, 4, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(BITS, G4, 4, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(BITS, A4, 4, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(BITS, R1G2B1, 4, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(BITS, B1G2R1, 4, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(BITS, A1R1G1B1, 4, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(BITS, A1B1G1R1, 4, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(BITS, R1G1B1A1, 4, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(BITS, B1G1R1A1, 4, 1, NORMAL_FAST)
IPIXEL_SPAN_DRAW_MAIN(BITS, X1R1G1B1, 4, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(BITS, X1B1G1R1, 4, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(BITS, R1G1B1X1, 4, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(BITS, B1G1R1X1, 4, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(PAL, C1, 1, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(BITS, G1, 1, 1, STATIC)
IPIXEL_SPAN_DRAW_MAIN(BITS, A1, 1, 1, NORMAL_FAST)

#undef IPIXEL_SPAN_DRAW_MAIN
#undef IPIXEL_SPAN_DRAW_PROC_N
#undef IPIXEL_SPAN_DRAW_PROC_X
#undef IPIXEL_SPAN_DRAW_PROC_1
#undef IPIXEL_SPAN_DRAW_PROC_BITS
#undef IPIXEL_SPAN_DRAW_PROC_PAL

struct iPixelSpanDrawProc
{
	iSpanDrawProc blend, additive, blend_default, additive_default;
};

#define ITABLE_ITEM(fmt) { \
	ipixel_span_draw_proc_##fmt##_0, ipixel_span_draw_proc_##fmt##_1, \
	ipixel_span_draw_proc_##fmt##_0, ipixel_span_draw_proc_##fmt##_1 }

static struct iPixelSpanDrawProc ipixel_span_proc_list[IPIX_FMT_COUNT] =
{
	ITABLE_ITEM(A8R8G8B8),
	ITABLE_ITEM(A8B8G8R8),
	ITABLE_ITEM(R8G8B8A8),
	ITABLE_ITEM(B8G8R8A8),
	ITABLE_ITEM(X8R8G8B8),
	ITABLE_ITEM(X8B8G8R8),
	ITABLE_ITEM(R8G8B8X8),
	ITABLE_ITEM(B8G8R8X8),
	ITABLE_ITEM(P8R8G8B8),
	ITABLE_ITEM(R8G8B8),
	ITABLE_ITEM(B8G8R8),
	ITABLE_ITEM(R5G6B5),
	ITABLE_ITEM(B5G6R5),
	ITABLE_ITEM(X1R5G5B5),
	ITABLE_ITEM(X1B5G5R5),
	ITABLE_ITEM(R5G5B5X1),
	ITABLE_ITEM(B5G5R5X1),
	ITABLE_ITEM(A1R5G5B5),
	ITABLE_ITEM(A1B5G5R5),
	ITABLE_ITEM(R5G5B5A1),
	ITABLE_ITEM(B5G5R5A1),
	ITABLE_ITEM(X4R4G4B4),
	ITABLE_ITEM(X4B4G4R4),
	ITABLE_ITEM(R4G4B4X4),
	ITABLE_ITEM(B4G4R4X4),
	ITABLE_ITEM(A4R4G4B4),
	ITABLE_ITEM(A4B4G4R4),
	ITABLE_ITEM(R4G4B4A4),
	ITABLE_ITEM(B4G4R4A4),
	ITABLE_ITEM(C8),
	ITABLE_ITEM(G8),
	ITABLE_ITEM(A8),
	ITABLE_ITEM(R3G3B2),
	ITABLE_ITEM(B2G3R3),
	ITABLE_ITEM(X2R2G2B2),
	ITABLE_ITEM(X2B2G2R2),
	ITABLE_ITEM(R2G2B2X2),
	ITABLE_ITEM(B2G2R2X2),
	ITABLE_ITEM(A2R2G2B2),
	ITABLE_ITEM(A2B2G2R2),
	ITABLE_ITEM(R2G2B2A2),
	ITABLE_ITEM(B2G2R2A2),
	ITABLE_ITEM(X4C4),
	ITABLE_ITEM(X4G4),
	ITABLE_ITEM(X4A4),
	ITABLE_ITEM(C4X4),
	ITABLE_ITEM(G4X4),
	ITABLE_ITEM(A4X4),
	ITABLE_ITEM(C4),
	ITABLE_ITEM(G4),
	ITABLE_ITEM(A4),
	ITABLE_ITEM(R1G2B1),
	ITABLE_ITEM(B1G2R1),
	ITABLE_ITEM(A1R1G1B1),
	ITABLE_ITEM(A1B1G1R1),
	ITABLE_ITEM(R1G1B1A1),
	ITABLE_ITEM(B1G1R1A1),
	ITABLE_ITEM(X1R1G1B1),
	ITABLE_ITEM(X1B1G1R1),
	ITABLE_ITEM(R1G1B1X1),
	ITABLE_ITEM(B1G1R1X1),
	ITABLE_ITEM(C1),
	ITABLE_ITEM(G1),
	ITABLE_ITEM(A1),
};

#undef ITABLE_ITEM

iSpanDrawProc ipixel_get_span_proc(int fmt, int isadditive, int usedefault)
{
	assert(fmt >= 0 && fmt < IPIX_FMT_COUNT);
	if (fmt < 0 || fmt >= IPIX_FMT_COUNT) {
		abort();
		return NULL;
	}
	if (ipixel_lut_inited == 0) ipixel_lut_init();
	if (usedefault) {
		if (isadditive == 0) return ipixel_span_proc_list[fmt].blend_default;
		else return ipixel_span_proc_list[fmt].additive_default;
	}	else {
		if (isadditive == 0) return ipixel_span_proc_list[fmt].blend;
		else return ipixel_span_proc_list[fmt].additive;
	}
}

void ipixel_set_span_proc(int fmt, int isadditive, iSpanDrawProc proc)
{
	assert(fmt >= 0 && fmt < IPIX_FMT_COUNT);
	if (fmt < 0 || fmt >= IPIX_FMT_COUNT) {
		abort();
		return;
	}
	if (ipixel_lut_inited == 0) ipixel_lut_init();
	if (isadditive == 0) ipixel_span_proc_list[fmt].blend = proc;
	else ipixel_span_proc_list[fmt].additive = proc;
}



/**********************************************************************
 * CARD operations
 **********************************************************************/

/* reverse card */
void ipixel_card_reverse(IUINT32 *card, int size)
{
	IUINT32 *p1, *p2;
	IUINT32 value;
	for (p1 = card, p2 = card + size - 1; p1 < p2; p1++, p2--) {
		value = *p1;
		*p1 = *p2;
		*p2 = value;
	}
}


/* multi card */
void ipixel_card_multi_default(IUINT32 *card, int size, IUINT32 color)
{
	IUINT32 r1, g1, b1, a1, r2, g2, b2, a2, f;
	IRGBA_FROM_A8R8G8B8(color, r1, g1, b1, a1);
	if ((color & 0xffffff) == 0xffffff) f = 1;
	else f = 0;
	if (color == 0xffffffff) {
		return;
	}
	else if (color == 0) {
		memset(card, 0, sizeof(IUINT32) * size);
	}
	else if (f) {
		IUINT8 *src = (IUINT8*)card;
		if (a1 == 0) {
			for (; size > 0; size--) {
			#if IWORDS_BIG_ENDIAN
				src[0] = 0;
			#else
				src[3] = 0;
			#endif
				src += sizeof(IUINT32);
			}
			return;
		}
		a1 = _ipixel_norm(a1);
		for (; size > 0; size--) {
		#if IWORDS_BIG_ENDIAN
			a2 = src[0];
			src[0] = (IUINT8)((a2 * a1) >> 8);
		#else
			a2 = src[3];
			src[3] = (IUINT8)((a2 * a1) >> 8);
		#endif
			src += sizeof(IUINT32);
		}
	}
	else {
		IUINT8 *src = (IUINT8*)card;
		a1 = _ipixel_norm(a1);
		r1 = _ipixel_norm(r1);
		g1 = _ipixel_norm(g1);
		b1 = _ipixel_norm(b1);
		for (; size > 0; src += sizeof(IUINT32), size--) {
			_ipixel_load_card(src, r2, g2, b2, a2);
			r2 = (r1 * r2) >> 8;
			g2 = (g1 * g2) >> 8;
			b2 = (b1 * b2) >> 8;
			a2 = (a1 * a2) >> 8;
			*((IUINT32*)src) = IRGBA_TO_A8R8G8B8(r2, g2, b2, a2);
		}
	}
}


void (*ipixel_card_multi_proc)(IUINT32 *card, int size, IUINT32 color) =
	ipixel_card_multi_default;

/* multi card */
void ipixel_card_multi(IUINT32 *card, int size, IUINT32 color)
{
	ipixel_card_multi_proc(card, size, color);
}


/* mask card */
void ipixel_card_mask_default(IUINT32 *card, int size, const IUINT32 *mask)
{
	IUINT32 r1, g1, b1, a1, r2, g2, b2, a2;
	for (; size > 0; card++, mask++, size--) {
		_ipixel_load_card(mask, r1, g1, b1, a1);
		_ipixel_load_card(card, r2, g2, b2, a2);
		r2 = _imul_y_div_255(r2, r1);
		g2 = _imul_y_div_255(g2, g1);
		b2 = _imul_y_div_255(b2, b1);
		a2 = _imul_y_div_255(a2, a1);
		*card = IRGBA_TO_A8R8G8B8(r2, g2, b2, a2);
	}
}

void (*ipixel_card_mask_proc)(IUINT32 *card, int size, const IUINT32 *mask) =
	ipixel_card_mask_default;

/* mask card */
void ipixel_card_mask(IUINT32 *card, int size, const IUINT32 *mask)
{
	ipixel_card_mask_proc(card, size, mask);
}

/* cover multi */
void ipixel_card_cover_default(IUINT32 *card, int size, const IUINT8 *cover)
{
	IINT32 cc, aa;
	for (; size > 0; card++, size--) {
		cc = *cover++;
		if (cc == 0) {
			((IUINT8*)card)[_ipixel_card_alpha] = 0;
		}
		else {
			aa = ((IUINT8*)card)[_ipixel_card_alpha];
			if (aa == 0) continue;
			aa *= cc;
			((IUINT8*)card)[_ipixel_card_alpha] = (IUINT8)_idiv_255(aa);
		}
	}
}

void (*ipixel_card_cover_proc)(IUINT32 *card, int size, const IUINT8 *cover) 
	= ipixel_card_cover_default;

/* mask cover */
void ipixel_card_cover(IUINT32 *card, int size, const IUINT8 *cover)
{
	ipixel_card_cover_proc(card, size, cover);
}

void ipixel_card_over_default(IUINT32 *dst, int size, const IUINT32 *card,
	const IUINT8 *cover)
{
	IUINT32 *endup = dst + size;
	if (cover == NULL) {
		for (; dst < endup; card++, dst++) {
			IBLEND_PARGB(dst[0], card[0]);
		}
	}	else {
		for (; dst < endup; cover++, card++, dst++) {
			IBLEND_PARGB_COVER(dst[0], card[0], cover[0]);
		}
	}
}

void (*ipixel_card_over_proc)(IUINT32*, int, const IUINT32*, const IUINT8*) =
	ipixel_card_over_default;

/* card composite: src over */
void ipixel_card_over(IUINT32 *dst, int size, const IUINT32 *card, 
	const IUINT8 *cover)
{
	ipixel_card_over_proc(dst, size, card, cover);
}

/* card proc set */
void ipixel_card_set_proc(int id, void *proc)
{
	if (id == 0) {
		if (proc == NULL) ipixel_card_multi_proc = ipixel_card_multi_default;
		else {
			ipixel_card_multi_proc = 
				(void (*)(IUINT32 *, int, IUINT32))proc;
		}
	}
	else if (id == 1) {
		if (proc == NULL) ipixel_card_mask_proc = ipixel_card_mask_default;
		else {
			ipixel_card_mask_proc = 
				(void (*)(IUINT32 *, int, const IUINT32 *))proc;
		}
	}
	else if (id == 2) {
		if (proc == NULL) ipixel_card_cover_proc = ipixel_card_cover_default;
		else {
			ipixel_card_cover_proc = 
				(void (*)(IUINT32 *, int, const IUINT8 *))proc;
		}
	}
	else if (id == 3) {
		if (proc == NULL) ipixel_card_over_proc = ipixel_card_over_default;
		else {
			ipixel_card_over_proc = 
				(void (*)(IUINT32*, int, const IUINT32*, const IUINT8*))proc;
		}
	}
}



/**********************************************************************
 * MACRO: HLINE ROUTINE
 **********************************************************************/
/* hline filling: 8/16/24/32 bits without palette */
#define IPIXEL_HLINE_DRAW_PROC_N(fmt, bpp, nbytes, mode) \
static void ipixel_hline_draw_proc_##fmt##_0(void *bits, \
	int offset, int w, IUINT32 color, const IUINT8 *cover, \
	const iColorIndex *idx) \
{ \
	unsigned char *dst = ((unsigned char*)bits) + offset * nbytes; \
	IUINT32 r1, g1, b1, a1, r2, g2, b2, a2, cc, cx, cz; \
	IRGBA_FROM_A8R8G8B8(color, r1, g1, b1, a1); \
	if (a1 == 0) return; \
	cz = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
	if (cover == NULL) { \
		if (a1 == 255) { \
			_ipixel_fill(bpp, dst, 0, w, cz); \
		} \
		else if (a1 > 0) { \
			for (; w > 0; dst += nbytes, w--) { \
				cc = _ipixel_fetch(bpp, dst, 0); \
				IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
				IBLEND_##mode(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, 0, cc); \
			} \
		} \
	}	else { \
		if (a1 == 255) { \
			for (; w > 0; dst += nbytes, w--) { \
				a1 = *cover++; \
				if (a1 == 255) { \
					_ipixel_store(bpp, dst, 0, cz); \
				} \
				else if (a1 > 0) { \
					cc = _ipixel_fetch(bpp, dst, 0); \
					IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
					IBLEND_##mode(r1, g1, b1, a1, r2, g2, b2, a2); \
					cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
					_ipixel_store(bpp, dst, 0, cc); \
				} \
			} \
		}	\
		else if (a1 > 0) { \
			a1 = _ipixel_norm(a1); \
			for (; w > 0; dst += nbytes, w--) { \
				cx = *cover++; \
				if (cx > 0) { \
					cx = (cx * a1) >> 8; \
					cc = _ipixel_fetch(bpp, dst, 0); \
					IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
					IBLEND_##mode(r1, g1, b1, cx, r2, g2, b2, a2); \
					cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
					_ipixel_store(bpp, dst, 0, cc); \
				} \
			} \
		} \
	} \
} \
static void ipixel_hline_draw_proc_##fmt##_1(void *bits, \
	int offset, int w, IUINT32 color, const IUINT8 *cover, \
	const iColorIndex *idx) \
{ \
	unsigned char *dst = ((unsigned char*)bits) + offset * nbytes; \
	IUINT32 r1, g1, b1, a1, r2, g2, b2, a2, cc, cx; \
	IRGBA_FROM_A8R8G8B8(color, r1, g1, b1, a1); \
	if (a1 == 0) return; \
	if (cover == NULL) { \
		r2 = g2 = b2 = a2 = 0; \
		IBLEND_ADDITIVE(r1, g1, b1, a1, r2, g2, a2, b2); \
		r1 = r2; g1 = g2; b1 = b2; a1 = a2; \
		for (; w > 0; dst += nbytes, w--) { \
			cc = _ipixel_fetch(bpp, dst, 0); \
			IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
			r2 = ICLIP_256(r1 + r2); \
			g2 = ICLIP_256(g1 + g2); \
			b2 = ICLIP_256(b1 + b2); \
			a2 = ICLIP_256(a1 + a2); \
			cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
			_ipixel_store(bpp, dst, 0, cc); \
		} \
	}	else { \
		if (a1 == 255) { \
			for (; w > 0; dst += nbytes, w--) { \
				a1 = *cover++; \
				if (a1 > 0) { \
					cc = _ipixel_fetch(bpp, dst, 0); \
					IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
					IBLEND_ADDITIVE(r1, g1, b1, a1, r2, g2, b2, a2); \
					cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
					_ipixel_store(bpp, dst, 0, cc); \
				} \
			} \
		}	\
		else if (a1 > 0) { \
			a1 = _ipixel_norm(a1); \
			for (; w > 0; dst += nbytes, w--) { \
				cx = *cover++; \
				if (cx > 0) { \
					cx = (cx * a1) >> 8; \
					cc = _ipixel_fetch(bpp, dst, 0); \
					IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
					IBLEND_ADDITIVE(r1, g1, b1, cx, r2, g2, b2, a2); \
					cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
					_ipixel_store(bpp, dst, 0, cc); \
				} \
			} \
		} \
	} \
}

/* hline filling: 8/4/1 bits with or without palette */
#define IPIXEL_HLINE_DRAW_PROC_X(fmt, bpp, nbytes, mode, init) \
static void ipixel_hline_draw_proc_##fmt##_0(void *bits, \
	int offset, int w, IUINT32 col, const IUINT8 *cover, \
	const iColorIndex *_ipixel_src_index) \
{ \
	unsigned char *dst = ((unsigned char*)bits); \
	IUINT32 r1, g1, b1, a1, r2, g2, b2, a2, cc, cx, cz; \
	init; \
	IRGBA_FROM_A8R8G8B8(col, r1, g1, b1, a1); \
	if (a1 == 0) return; \
	cz = IRGBA_TO_PIXEL(fmt, r1, g1, b1, a1); \
	if (cover == NULL) { \
		if (a1 == 255) { \
			_ipixel_fill(bpp, dst, offset, w, cz); \
		} \
		else if (a1 > 0) { \
			for (; w > 0; offset++, w--) { \
				cc = _ipixel_fetch(bpp, dst, offset); \
				IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
				IBLEND_##mode(r1, g1, b1, a1, r2, g2, b2, a2); \
				cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
				_ipixel_store(bpp, dst, offset, cc); \
			} \
		} \
	}	else { \
		if (a1 == 255) { \
			for (; w > 0; offset++, w--) { \
				a1 = *cover++; \
				if (a1 == 255) { \
					_ipixel_store(bpp, dst, offset, cz); \
				} \
				else if (a1 > 0) { \
					cc = _ipixel_fetch(bpp, dst, offset); \
					IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
					IBLEND_##mode(r1, g1, b1, a1, r2, g2, b2, a2); \
					cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
					_ipixel_store(bpp, dst, offset, cc); \
				} \
			} \
		}	\
		else if (a1 > 0) { \
			a1 = _ipixel_norm(a1); \
			for (; w > 0; offset++, w--) { \
				cx = (*cover++ * a1) >> 8; \
				if (cx == 255) { \
					cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
					_ipixel_store(bpp, dst, offset, cc); \
				} \
				else if (cx > 0) { \
					cc = _ipixel_fetch(bpp, dst, offset); \
					IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
					IBLEND_##mode(r1, g1, b1, cx, r2, g2, b2, a2); \
					cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
					_ipixel_store(bpp, dst, offset, cc); \
				} \
			} \
		} \
	} \
} \
static void ipixel_hline_draw_proc_##fmt##_1(void *bits, \
	int offset, int w, IUINT32 col, const IUINT8 *cover, \
	const iColorIndex *_ipixel_src_index) \
{ \
	unsigned char *dst = ((unsigned char*)bits); \
	IUINT32 r1, g1, b1, a1, r2, g2, b2, a2, cc, cx; \
	init; \
	IRGBA_FROM_A8R8G8B8(col, r1, g1, b1, a1); \
	if (a1 == 0) return; \
	if (cover == NULL) { \
		r2 = g2 = b2 = a2 = 0; \
		IBLEND_ADDITIVE(r1, g1, b1, a1, r2, g2, a2, b2); \
		r1 = r2; g1 = g2; b1 = b2; a1 = a2; \
		for (; w > 0; dst += nbytes, w--) { \
			cc = _ipixel_fetch(bpp, dst, offset); \
			IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
			r2 = ICLIP_256(r1 + r2); \
			g2 = ICLIP_256(g1 + g2); \
			b2 = ICLIP_256(b1 + b2); \
			a2 = ICLIP_256(a1 + a2); \
			cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
			_ipixel_store(bpp, dst, offset, cc); \
		} \
	}	else { \
		if (a1 == 255) { \
			for (; w > 0; offset++, w--) { \
				a1 = *cover++; \
				if (a1 > 0) { \
					cc = _ipixel_fetch(bpp, dst, offset); \
					IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
					IBLEND_ADDITIVE(r1, g1, b1, a1, r2, g2, b2, a2); \
					cc = IRGBA_TO_PIXEL(fmt, r2, g2, b2, a2); \
					_ipixel_store(bpp, dst, offset, cc); \
				} \
			} \
		}	\
		else if (a1 > 0) { \
			a1 = _ipixel_norm(a1); \
			for (; w > 0; offset++, w--) { \
				cx = *cover++; \
				if (cx > 0) { \
					cx = (cx * a1) >> 8; \
					cc = _ipixel_fetch(bpp, dst, offset); \
					IRGBA_FROM_PIXEL(fmt, cc, r2, g2, b2, a2); \
					IBLEND_ADDITIVE(r1, g1, b1, cx, r2, g2, b2, a2); \
					cc = IRGBA_TO_PIXEL(fmt, r1, g1, b1, 255); \
					_ipixel_store(bpp, dst, offset, cc); \
				} \
			} \
		} \
	} \
}

/* hline filling: 8/4/1 bits without palette */
#define IPIXEL_HLINE_DRAW_PROC_BITS(fmt, bpp, nbytes, mode) \
		IPIXEL_HLINE_DRAW_PROC_X(fmt, bpp, nbytes, mode, {})

/* hline filling: 8/4/1 bits with palette */
#define IPIXEL_HLINE_DRAW_PROC_PAL(fmt, bpp, nbytes, mode) \
		IPIXEL_HLINE_DRAW_PROC_X(fmt, bpp, nbytes, mode,  \
				const iColorIndex *_ipixel_dst_index = _ipixel_src_index) 

/* hline filling: main macro */
#define IPIXEL_HLINE_DRAW_MAIN(type, fmt, bpp, nbytes, mode) \
	IPIXEL_HLINE_DRAW_PROC_##type(fmt, bpp, nbytes, mode) 


#if 1
IPIXEL_HLINE_DRAW_MAIN(N, A8R8G8B8, 32, 4, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, A8B8G8R8, 32, 4, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, R8G8B8A8, 32, 4, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, B8G8R8A8, 32, 4, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, X8R8G8B8, 32, 4, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, X8B8G8R8, 32, 4, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, R8G8B8X8, 32, 4, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, B8G8R8X8, 32, 4, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, P8R8G8B8, 32, 4, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, R8G8B8, 24, 3, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, B8G8R8, 24, 3, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, R5G6B5, 16, 2, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, B5G6R5, 16, 2, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, X1R5G5B5, 16, 2, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, X1B5G5R5, 16, 2, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, R5G5B5X1, 16, 2, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, B5G5R5X1, 16, 2, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, A1R5G5B5, 16, 2, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, A1B5G5R5, 16, 2, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, R5G5B5A1, 16, 2, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, B5G5R5A1, 16, 2, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, X4R4G4B4, 16, 2, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, X4B4G4R4, 16, 2, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, R4G4B4X4, 16, 2, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, B4G4R4X4, 16, 2, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, A4R4G4B4, 16, 2, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, A4B4G4R4, 16, 2, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, R4G4B4A4, 16, 2, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, B4G4R4A4, 16, 2, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(PAL, C8, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, G8, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, A8, 8, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, R3G3B2, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, B2G3R3, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, X2R2G2B2, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, X2B2G2R2, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, R2G2B2X2, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, B2G2R2X2, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, A2R2G2B2, 8, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, A2B2G2R2, 8, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, R2G2B2A2, 8, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(N, B2G2R2A2, 8, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(PAL, X4C4, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, X4G4, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, X4A4, 8, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(PAL, C4X4, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, G4X4, 8, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(N, A4X4, 8, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(PAL, C4, 4, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(BITS, G4, 4, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(BITS, A4, 4, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(BITS, R1G2B1, 4, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(BITS, B1G2R1, 4, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(BITS, A1R1G1B1, 4, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(BITS, A1B1G1R1, 4, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(BITS, R1G1B1A1, 4, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(BITS, B1G1R1A1, 4, 1, NORMAL_FAST)
IPIXEL_HLINE_DRAW_MAIN(BITS, X1R1G1B1, 4, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(BITS, X1B1G1R1, 4, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(BITS, R1G1B1X1, 4, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(BITS, B1G1R1X1, 4, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(PAL, C1, 1, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(BITS, G1, 1, 1, STATIC)
IPIXEL_HLINE_DRAW_MAIN(BITS, A1, 1, 1, NORMAL_FAST)
#endif

#undef IPIXEL_HLINE_DRAW_MAIN
#undef IPIXEL_HLINE_DRAW_PROC_PAL
#undef IPIXEL_HLINE_DRAW_PROC_BITS
#undef IPIXEL_HLINE_DRAW_PROC_X
#undef IPIXEL_HLINE_DRAW_PROC_N


struct iPixelHLineDrawProc
{
	iHLineDrawProc blend, additive, blend_default, additive_default;
};

#define ITABLE_ITEM(fmt) { \
	ipixel_hline_draw_proc_##fmt##_0, ipixel_hline_draw_proc_##fmt##_1, \
	ipixel_hline_draw_proc_##fmt##_0, ipixel_hline_draw_proc_##fmt##_1 }

static struct iPixelHLineDrawProc ipixel_hline_proc_list[IPIX_FMT_COUNT] =
{
	ITABLE_ITEM(A8R8G8B8),
	ITABLE_ITEM(A8B8G8R8),
	ITABLE_ITEM(R8G8B8A8),
	ITABLE_ITEM(B8G8R8A8),
	ITABLE_ITEM(X8R8G8B8),
	ITABLE_ITEM(X8B8G8R8),
	ITABLE_ITEM(R8G8B8X8),
	ITABLE_ITEM(B8G8R8X8),
	ITABLE_ITEM(P8R8G8B8),
	ITABLE_ITEM(R8G8B8),
	ITABLE_ITEM(B8G8R8),
	ITABLE_ITEM(R5G6B5),
	ITABLE_ITEM(B5G6R5),
	ITABLE_ITEM(X1R5G5B5),
	ITABLE_ITEM(X1B5G5R5),
	ITABLE_ITEM(R5G5B5X1),
	ITABLE_ITEM(B5G5R5X1),
	ITABLE_ITEM(A1R5G5B5),
	ITABLE_ITEM(A1B5G5R5),
	ITABLE_ITEM(R5G5B5A1),
	ITABLE_ITEM(B5G5R5A1),
	ITABLE_ITEM(X4R4G4B4),
	ITABLE_ITEM(X4B4G4R4),
	ITABLE_ITEM(R4G4B4X4),
	ITABLE_ITEM(B4G4R4X4),
	ITABLE_ITEM(A4R4G4B4),
	ITABLE_ITEM(A4B4G4R4),
	ITABLE_ITEM(R4G4B4A4),
	ITABLE_ITEM(B4G4R4A4),
	ITABLE_ITEM(C8),
	ITABLE_ITEM(G8),
	ITABLE_ITEM(A8),
	ITABLE_ITEM(R3G3B2),
	ITABLE_ITEM(B2G3R3),
	ITABLE_ITEM(X2R2G2B2),
	ITABLE_ITEM(X2B2G2R2),
	ITABLE_ITEM(R2G2B2X2),
	ITABLE_ITEM(B2G2R2X2),
	ITABLE_ITEM(A2R2G2B2),
	ITABLE_ITEM(A2B2G2R2),
	ITABLE_ITEM(R2G2B2A2),
	ITABLE_ITEM(B2G2R2A2),
	ITABLE_ITEM(X4C4),
	ITABLE_ITEM(X4G4),
	ITABLE_ITEM(X4A4),
	ITABLE_ITEM(C4X4),
	ITABLE_ITEM(G4X4),
	ITABLE_ITEM(A4X4),
	ITABLE_ITEM(C4),
	ITABLE_ITEM(G4),
	ITABLE_ITEM(A4),
	ITABLE_ITEM(R1G2B1),
	ITABLE_ITEM(B1G2R1),
	ITABLE_ITEM(A1R1G1B1),
	ITABLE_ITEM(A1B1G1R1),
	ITABLE_ITEM(R1G1B1A1),
	ITABLE_ITEM(B1G1R1A1),
	ITABLE_ITEM(X1R1G1B1),
	ITABLE_ITEM(X1B1G1R1),
	ITABLE_ITEM(R1G1B1X1),
	ITABLE_ITEM(B1G1R1X1),
	ITABLE_ITEM(C1),
	ITABLE_ITEM(G1),
	ITABLE_ITEM(A1),
};

#undef ITABLE_ITEM


/* get a hline drawing function with given pixel format */
iHLineDrawProc ipixel_get_hline_proc(int fmt, int isadditive, int usedefault)
{
	assert(fmt >= 0 && fmt < IPIX_FMT_COUNT);
	if (fmt < 0 || fmt >= IPIX_FMT_COUNT) {
		abort();
		return NULL;
	}
	if (ipixel_lut_inited == 0) ipixel_lut_init();
	if (usedefault) {
		if (isadditive == 0) 
			return ipixel_hline_proc_list[fmt].blend_default;
		else 
			return ipixel_hline_proc_list[fmt].additive_default;
	}	else {
		if (isadditive == 0) 
			return ipixel_hline_proc_list[fmt].blend;
		else 
			return ipixel_hline_proc_list[fmt].additive;
	}
}

/* set a hline drawing function */
void ipixel_set_hline_proc(int fmt, int isadditive, iHLineDrawProc proc)
{
	assert(fmt >= 0 && fmt < IPIX_FMT_COUNT);
	if (fmt < 0 || fmt >= IPIX_FMT_COUNT) {
		abort();
		return;
	}
	if (ipixel_lut_inited == 0) ipixel_lut_init();
	if (isadditive == 0) ipixel_hline_proc_list[fmt].blend = proc;
	else ipixel_hline_proc_list[fmt].additive = proc;
}


/* ipixel_blend - blend between two formats 
 * you must provide a working memory pointer to workmem. if workmem eq NULL,
 * this function will do nothing but returns how many bytes needed in workmem
 * dfmt        - dest pixel format
 * dbits       - dest pixel buffer
 * dpitch      - dest row stride
 * dx          - dest x offset
 * sfmt        - source pixel format
 * sbits       - source pixel buffer
 * spitch      - source row stride
 * sx          - source x offset
 * w           - width
 * h           - height
 * color       - const color
 * op          - blending operator (IPIXEL_BLEND_OP_BLEND, ADD, COPY)
 * flip        - flip (IPIXEL_FLIP_NONE, HFLIP, VFLIP)
 * dindex      - dest index
 * sindex      - source index
 * workmem     - working memory
 * this function need some memory to work with. to avoid allocating, 
 * you must provide a memory block whose size is (w * 4) to it.
 */
long ipixel_blend(int dfmt, void *dbits, long dpitch, int dx, int sfmt, 
	const void *sbits, long spitch, int sx, int w, int h, IUINT32 color,
	int op, int flip, const iColorIndex *dindex, 
	const iColorIndex *sindex, void *workmem)
{
	IUINT32 *buffer = (IUINT32*)workmem;
	IUINT8 *dline = (IUINT8*)dbits;
	const IUINT8 *sline = (const IUINT8*)sbits;
	iSpanDrawProc drawspan = NULL;
	iFetchProc fetch;
	iStoreProc store;
	int k;

	if (workmem == NULL) {
		return w * sizeof(IUINT32);
	}

	fetch = ipixel_get_fetch(sfmt, IPIXEL_ACCESS_MODE_NORMAL);
	store = ipixel_get_store(dfmt, IPIXEL_ACCESS_MODE_NORMAL);

	if (op == IPIXEL_BLEND_OP_BLEND) {
		drawspan = ipixel_get_span_proc(dfmt, 0, 0);
	}
	else if (op == IPIXEL_BLEND_OP_ADD) {
		drawspan = ipixel_get_span_proc(dfmt, 1, 0);
	}

	if ((flip & IPIXEL_FLIP_VFLIP) != 0) {
		sline = sline + spitch * (h - 1);
		spitch = -spitch;
	}

	if (sfmt == IPIX_FMT_A8R8G8B8 && (flip & IPIXEL_FLIP_HFLIP) == 0 &&
		color == 0xffffffff) {
		if (drawspan != NULL) {
			for (k = 0; k < h; sline += spitch, dline += dpitch, k++) {
				const IUINT32 *src = ((const IUINT32*)sline) + sx;
				IUINT32 *dst = ((IUINT32*)dline);
				drawspan(dst, dx, w, src, NULL, dindex);
			}
		}	else {
			for (k = 0; k < h; sline += spitch, dline += dpitch, k++) {
				const IUINT32 *src = ((const IUINT32*)sline) + sx;
				IUINT32 *dst = ((IUINT32*)dline);
				store(dst, src, dx, w, dindex);
			}
		}
		return 0;
	}

	#define IPIXEL_BLEND_LOOP(work) do { \
			for (k = 0; k < h; sline += spitch, dline += dpitch, k++) { \
				const IUINT32 *src = ((const IUINT32*)sline); \
				IUINT32 *dst = ((IUINT32*)dline); \
				fetch(src, sx, w, buffer, sindex); \
				work; \
			} \
		}	while (0)

	if ((flip & IPIXEL_FLIP_HFLIP) == 0) {
		if (drawspan != NULL) {
			if (color == 0xffffffff) {
				IPIXEL_BLEND_LOOP( {
					drawspan(dst, dx, w, buffer, NULL, dindex);
				});
			}	else {
				IPIXEL_BLEND_LOOP( {
					ipixel_card_multi_proc(buffer, w, color);
					drawspan(dst, dx, w, buffer, NULL, dindex);
				});
			}
		}	else {
			if (color == 0xffffffff) {
				IPIXEL_BLEND_LOOP( {
					store(dst, buffer, dx, w, dindex);
				});
			}	else {
				IPIXEL_BLEND_LOOP( {
					ipixel_card_multi_proc(buffer, w, color);
					store(dst, buffer, dx, w, dindex);
				});
			}
		}
	}	else {
		if (drawspan != NULL) {
			if (color == 0xffffffff) {
				IPIXEL_BLEND_LOOP( {
					ipixel_card_reverse(buffer, w);
					drawspan(dst, dx, w, buffer, NULL, dindex);
				});
			}	else {
				IPIXEL_BLEND_LOOP( {
					ipixel_card_reverse(buffer, w);
					ipixel_card_multi_proc(buffer, w, color);
					drawspan(dst, dx, w, buffer, NULL, dindex);
				});
			}
		}	else {
			if (color == 0xffffffff) {
				IPIXEL_BLEND_LOOP( {
					ipixel_card_reverse(buffer, w);
					store(dst, buffer, dx, w, dindex);
				});
			}	else {
				IPIXEL_BLEND_LOOP( {
					ipixel_card_reverse(buffer, w);
					ipixel_card_multi_proc(buffer, w, color);
					store(dst, buffer, dx, w, dindex);
				});
			}
		}
	}

	#undef IPIXEL_BLEND_LOOP

	return 0;
}


/* ipixel_convert: convert pixel format 
 * parameters: same as ipixel_blend 
 * it just calls ipixel_blend with 'op=IPIXEL_BLEND_OP_COPY'
 */
long ipixel_convert(int dfmt, void *dbits, long dpitch, int dx, int sfmt, 
	const void *sbits, long spitch, int sx, int w, int h, IUINT32 color,
	int flip, const iColorIndex *dindex, const iColorIndex *sindex, 
	void *workmem)
{
	return ipixel_blend(dfmt, dbits, dpitch, dx, sfmt, sbits, spitch, sx,
		w, h, color, IPIXEL_BLEND_OP_COPY, flip, dindex, sindex, workmem);
}



/**********************************************************************
 * MACRO: BLITING ROUTINE
 **********************************************************************/
/* normal blit in 32/16/8 bits */
#define IPIXEL_BLIT_PROC_N(nbits, nbytes, INTTYPE) \
static void ipixel_blit_proc_##nbits(void *dbits, long dpitch, int dx,  \
	const void *sbits, long spitch, int sx, int w, int h, int flip) \
{ \
	int y, x; \
	if (flip & IPIXEL_FLIP_VFLIP) { \
		sbits = (const IUINT8*)sbits + spitch * (h - 1); \
		spitch = -spitch; \
	} \
	if ((flip & IPIXEL_FLIP_HFLIP) == 0) { \
		long size = w * nbytes; \
		for (y = 0; y < h; y++) { \
			memcpy((INTTYPE*)dbits + dx, (const INTTYPE*)sbits + sx, size); \
			dbits = (IUINT8*)dbits + dpitch; \
			sbits = (const IUINT8*)sbits + spitch; \
		} \
	}	else { \
		for (y = 0; y < h; y++) { \
			const INTTYPE *src = (const INTTYPE*)sbits + sx + w - 1; \
			INTTYPE *dst = (INTTYPE*)dbits + dx; \
			for (x = w; x > 0; x--) *dst++ = *src--; \
			dbits = (IUINT8*)dbits + dpitch; \
			sbits = (const IUINT8*)sbits + spitch; \
		} \
	} \
} 

/* normal blit in 24/4/1 bits */
#define IPIXEL_BLIT_PROC_BITS(nbits) \
static void ipixel_blit_proc_##nbits(void *dbits, long dpitch, int dx, \
	const void *sbits, long spitch, int sx, int w, int h, int flip) \
{ \
	int y, x1, x2, sx0, sxd, endx; \
	if (flip & IPIXEL_FLIP_VFLIP) { \
		sbits = (const IUINT8*)sbits + spitch * (h - 1); \
		spitch = -spitch; \
	} \
	if (flip & IPIXEL_FLIP_HFLIP) { \
		sx0 = sx + w - 1; \
		sxd = -1; \
	}	else { \
		sx0 = sx; \
		sxd = 1; \
	} \
	endx = dx + w; \
	for (y = 0; y < h; y++) { \
		IUINT32 cc; \
		for (x1 = dx, x2 = sx0; x1 < endx; x1++, x2 += sxd) { \
			cc = _ipixel_fetch(nbits, sbits, x2); \
			_ipixel_store(nbits, dbits, x1, cc); \
		} \
		dbits = (IUINT8*)dbits + dpitch; \
		sbits = (const IUINT8*)sbits + spitch; \
	} \
}


/* mask blit in 32/16/8 bits */
#define IPIXEL_BLIT_MASK_PROC_N(nbits, nbytes, INTTYPE) \
static void ipixel_blit_mask_proc_##nbits(void *dbits, long dpitch, \
	int dx, const void *sbits, long spitch, int sx, int w, int h, \
	IUINT32 mask, int flip) \
{ \
	INTTYPE cmask = (INTTYPE)mask; \
	int y; \
	if (flip & IPIXEL_FLIP_VFLIP) { \
		sbits = (const IUINT8*)sbits + spitch * (h - 1); \
		spitch = -spitch; \
	} \
	if ((flip & IPIXEL_FLIP_HFLIP) == 0) { \
		for (y = 0; y < h; y++) { \
			const INTTYPE *src = (const INTTYPE*)sbits + sx; \
			INTTYPE *dst = (INTTYPE*)dbits + dx; \
			INTTYPE *dstend = dst + w; \
			for (; dst < dstend; src++, dst++) { \
				if (src[0] != cmask) dst[0] = src[0]; \
			} \
			dbits = (IUINT8*)dbits + dpitch; \
			sbits = (const IUINT8*)sbits + spitch; \
		} \
	}	else { \
		for (y = 0; y < h; y++) { \
			const INTTYPE *src = (const INTTYPE*)sbits + sx + w - 1; \
			INTTYPE *dst = (INTTYPE*)dbits + dx; \
			INTTYPE *dstend = dst + w; \
			for (; dst < dstend; src--, dst++) { \
				if (src[0] != cmask) dst[0] = src[0]; \
			} \
			dbits = (IUINT8*)dbits + dpitch; \
			sbits = (const IUINT8*)sbits + spitch; \
		} \
	} \
}

/* mask blit in 24/4/1 bits */
#define IPIXEL_BLIT_MASK_PROC_BITS(nbits) \
static void ipixel_blit_mask_proc_##nbits(void *dbits, long dpitch, \
	int dx, const void *sbits, long spitch, int sx, int w, int h, \
	IUINT32 mask, int flip) \
{ \
	int y, x1, x2, sx0, sxd, endx; \
	if (flip & IPIXEL_FLIP_VFLIP) { \
		sbits = (const IUINT8*)sbits + spitch * (h - 1); \
		spitch = -spitch; \
	} \
	if (flip & IPIXEL_FLIP_HFLIP) { \
		sx0 = sx + w - 1; \
		sxd = -1; \
	}	else { \
		sx0 = sx; \
		sxd = 1; \
	} \
	endx = dx + w; \
	for (y = 0; y < h; y++) { \
		IUINT32 cc; \
		for (x1 = dx, x2 = sx0; x1 < endx; x1++, x2 += sxd) { \
			cc = _ipixel_fetch(nbits, sbits, x2); \
			if (cc != mask) _ipixel_store(nbits, dbits, x1, cc); \
		} \
		dbits = (IUINT8*)dbits + dpitch; \
		sbits = (const IUINT8*)sbits + spitch; \
	} \
}


/* normal bliter */
IPIXEL_BLIT_PROC_N(32, 4, IUINT32);
IPIXEL_BLIT_PROC_N(16, 2, IUINT16);
IPIXEL_BLIT_PROC_N(8, 1, IUINT8);

IPIXEL_BLIT_PROC_BITS(24);
IPIXEL_BLIT_PROC_BITS(4);
IPIXEL_BLIT_PROC_BITS(1);

/* mask bliter */
IPIXEL_BLIT_MASK_PROC_N(32, 4, IUINT32);
IPIXEL_BLIT_MASK_PROC_N(16, 2, IUINT16);
IPIXEL_BLIT_MASK_PROC_N(8, 1, IUINT8);

IPIXEL_BLIT_MASK_PROC_BITS(24);
IPIXEL_BLIT_MASK_PROC_BITS(4);
IPIXEL_BLIT_MASK_PROC_BITS(1);


#undef IPIXEL_BLIT_PROC_N
#undef IPIXEL_BLIT_PROC_BITS
#undef IPIXEL_BLIT_MASK_PROC_N
#undef IPIXEL_BLIT_MASK_PROC_BITS



/* blit driver desc */
struct iPixelBlitProc
{
	iBlitNMProc normal, normal_default;
	iBlitMKProc mask, mask_default;
};

#define ITABLE_ITEM(bpp) { \
	ipixel_blit_proc_##bpp, ipixel_blit_proc_##bpp, \
	ipixel_blit_mask_proc_##bpp, ipixel_blit_mask_proc_##bpp }


/* blit procedure look up table */
static struct iPixelBlitProc ipixel_blit_proc_list[6] =
{
	ITABLE_ITEM(32),
	ITABLE_ITEM(24),
	ITABLE_ITEM(16),
	ITABLE_ITEM(8),
	ITABLE_ITEM(4),
	ITABLE_ITEM(1),
};

static const int ipixel_lookup_bpp[33] = {
	-1, 5, -1, -1, 4, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, 
	2, 2, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1,
	-1, -1, 0,
};


/* get normal blit procedure */
iBlitNMProc ipixel_get_blit_normal(int bpp, int isdefault)
{
	int index;
	if (bpp < 0 || bpp > 32) return NULL;
	index = ipixel_lookup_bpp[bpp];
	if (index < 0) return NULL;
	if (isdefault) return ipixel_blit_proc_list[index].normal_default;
	return ipixel_blit_proc_list[index].normal;
}

/* get mask blit procedure */
iBlitMKProc ipixel_get_blit_mask(int bpp, int isdefault)
{
	int index;
	if (bpp < 0 || bpp > 32) return NULL;
	index = ipixel_lookup_bpp[bpp];
	if (index < 0) return NULL;
	if (isdefault) return ipixel_blit_proc_list[index].mask_default;
	return ipixel_blit_proc_list[index].mask;
}

/* set normal blit procedure */
void ipixel_set_blit_proc(int bpp, int type, void *proc)
{
	int index;
	if (bpp < 0 || bpp > 32) return;
	index = ipixel_lookup_bpp[bpp];
	if (index < 0) return;
	switch (type)
	{
	case IPIXEL_BLIT_NORMAL:
		ipixel_blit_proc_list[index].normal = (iBlitNMProc)proc;
		break;
	case IPIXEL_BLIT_MASK:
		ipixel_blit_proc_list[index].mask = (iBlitMKProc)proc;
		break;
	}
}


/* normal blit */
void ipixel_blit_normal(int bpp, void *dbits, long dpitch, int dx, 
	const void *sbits, long spitch, int sx, int w, int h, int flip)
{
	iBlitNMProc bliter;
	bliter = ipixel_get_blit_normal(bpp, 0);
	if (bliter) {
		bliter(dbits, dpitch, dx, sbits, spitch, sx, w, h, flip);
	}
}

/* mask blit */
void ipixel_blit_mask(int bpp, void *dbits, long dpitch, int dx,
	const void *sbits, long spitch, int sx, int w, int h, IUINT32 mask, 
	int flip)
{
	iBlitMKProc bliter;
	bliter = ipixel_get_blit_mask(bpp, 0);
	if (bliter) {
		bliter(dbits, dpitch, dx, sbits, spitch, sx, w, h, mask, flip);
	}
}



/**********************************************************************
 * CLIPING
 **********************************************************************/

/*
 * ipixel_clip - clip the rectangle from the src clip and dst clip then
 * caculate a new rectangle which is shared between dst and src cliprect:
 * clipdst  - dest clip array (left, top, right, bottom)
 * clipsrc  - source clip array (left, top, right, bottom)
 * (x, y)   - dest position
 * rectsrc  - source rect
 * mode     - check IPIXEL_FLIP_HFLIP or IPIXEL_FLIP_VFLIP
 * return zero for successful, return non-zero if there is no shared part
 */
int ipixel_clip(const int *clipdst, const int *clipsrc, int *x, int *y,
    int *rectsrc, int mode)
{
    int dcl = clipdst[0];       /* dest clip: left     */
    int dct = clipdst[1];       /* dest clip: top      */
    int dcr = clipdst[2];       /* dest clip: right    */
    int dcb = clipdst[3];       /* dest clip: bottom   */
    int scl = clipsrc[0];       /* source clip: left   */
    int sct = clipsrc[1];       /* source clip: top    */
    int scr = clipsrc[2];       /* source clip: right  */
    int scb = clipsrc[3];       /* source clip: bottom */
    int dx = *x;                /* dest x position     */
    int dy = *y;                /* dest y position     */
    int sl = rectsrc[0];        /* source rectangle: left   */
    int st = rectsrc[1];        /* source rectangle: top    */
    int sr = rectsrc[2];        /* source rectangle: right  */
    int sb = rectsrc[3];        /* source rectangle: bottom */
    int hflip, vflip;
    int w, h, d;
    
    hflip = (mode & IPIXEL_FLIP_HFLIP)? 1 : 0;
    vflip = (mode & IPIXEL_FLIP_VFLIP)? 1 : 0;

    if (dcr <= dcl || dcb <= dct || scr <= scl || scb <= sct) 
        return -1;

    if (sr <= scl || sb <= sct || sl >= scr || st >= scb) 
        return -2;

    /* check dest clip: left */
    if (dx < dcl) {
        d = dcl - dx;
        dx = dcl;
        if (!hflip) sl += d;
        else sr -= d;
    }

    /* check dest clip: top */
    if (dy < dct) {
        d = dct - dy;
        dy = dct;
        if (!vflip) st += d;
        else sb -= d;
    }

    w = sr - sl;
    h = sb - st;

    if (w < 0 || h < 0) 
        return -3;

    /* check dest clip: right */
    if (dx + w > dcr) {
        d = dx + w - dcr;
        if (!hflip) sr -= d;
        else sl += d;
    }

    /* check dest clip: bottom */
    if (dy + h > dcb) {
        d = dy + h - dcb;
        if (!vflip) sb -= d;
        else st += d;
    }

    if (sl >= sr || st >= sb) 
        return -4;

    /* check source clip: left */
    if (sl < scl) {
        d = scl - sl;
        sl = scl;
        if (!hflip) dx += d;
    }

    /* check source clip: top */
    if (st < sct) {
        d = sct - st;
        st = sct;
        if (!vflip) dy += d;
    }

    if (sl >= sr || st >= sb) 
        return -5;

    /* check source clip: right */
    if (sr > scr) {
        d = sr - scr;
        sr = scr;
        if (hflip) dx += d;
    }

    /* check source clip: bottom */
    if (sb > scb) {
        d = sb - scb;
        sb = scb;
        if (vflip) dy += d;
    }

    if (sl >= sr || st >= sb) 
        return -6;

    *x = dx;
    *y = dy;

    rectsrc[0] = sl;
    rectsrc[1] = st;
    rectsrc[2] = sr;
    rectsrc[3] = sb;

    return 0;
}


