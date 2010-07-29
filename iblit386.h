//=====================================================================
//
// iblit386.h - bitmap blitters implemention in i386
// 
// -  bypasse on-chip, write directly into combining buffer
// -  software prefetch instruction
// -  improvement: speed up 200% - 250% vs tradition blitters
// -  compile it with gcc, msvc, borlandc, watcomc, ...
//
// for more information about cache-control instructions please see:
//    The IA-32 Intel Architecture Software Developer's Manual
//    Volume 3: System Programming Guide
// with the URL below:
//    http://developer.intel.com/design/pentium4/manuals/253668.htm
// Using Block Prefetch for Optimized Memory Performance, Mike Wall
//
// history of this file:
// -  Feb.15 2004  skywind  create this file with iblit_386, iblit_mmx
// -  Feb.20 2004  skywind  add most i386 assembler globals
// -  Jun.17 2004  skywind  implement normal blitter with sse
// -  May.05 2005  skywind  implement mask blitter with mmx & sse
// -  Aug.02 2005  skywind  improve blitters with cache-control tech.
// -  Dec.11 2006  skywind  rewrite cpu detection method
// -  Dec.27 2006  skywind  rewrite mask blitters 
// -  Jan.28 2007  skywind  reform this file with new comment
//
//=====================================================================

#ifndef __IBLIT386_H__
#define __IBLIT386_H__


//---------------------------------------------------------------------
// platform detection
//---------------------------------------------------------------------
#if (defined(_WIN32) && !defined(WIN32))
	#define WIN32 _WIN32
#elif (defined(WIN32) && !defined(_WIN32))
	#define _WIN32 WIN32
#endif

#if (defined(_WIN32) && !defined(_MSC_VER))
	#ifndef __i386__
	#define __i386__
	#endif
#elif defined(_MSC_VER)
	#if (defined(_M_IX86) && !defined(__i386__))
	#define __i386__
	#endif
#endif

#ifndef __i386__
	#if (defined(__386__) || defined(__I386__) || _M_IX86)
	#define __i386__
	#endif
#endif

#if (defined(__i386__) && !defined(__I386__))
	#define __I386__
#endif




//=====================================================================
// i386 archtech support
//=====================================================================
#ifdef __i386__


#ifndef ASMCODE
#define ASMCODE
//---------------------------------------------------------------------
// basic inline asm operation code generation                         
//---------------------------------------------------------------------
#ifdef __GNUC__
#define ASMCODE1(a)             ".byte "#a"\n"
#define ASMCODE2(a, b)          ".byte "#a", "#b"\n"
#define ASMCODE3(a, b, c)       ".byte "#a", "#b", "#c"\n"
#define ASMCODE4(a, b, c, d)    ".byte "#a", "#b", "#c", "#d"\n"
#define ASMCODE5(a, b, c, d, e) ".byte "#a", "#b", "#c", "#d", "#e"\n"
#define ASMCODEW(n)             ".short "#n"\n"
#define ASMCODED(n)             ".long "#n"\n"
#define ASMALIGN(n)             ".align "#n", 0x90\n"
#define __ALIGNC__              ASMALIGN(8)
#define __INLINEGNU__
#ifndef __MACH__
#define ASM_BEGIN
#define ASM_ENDUP
#define ASM_REGS                "esi", "edi", "eax", "ebx", "ecx", "edx"
#else
#define ASM_BEGIN               "    pushl %%ebx\n"
#define ASM_ENDUP               "    popl %%ebx\n"
#define ASM_REGS                "esi", "edi", "eax", "ecx", "edx"
#endif

#elif defined(_MSC_VER)
#define ASMCODE1(a)             _asm _emit (a)
#define ASMCODE2(a, b)          ASMCODE1(a) ASMCODE1(b)
#define ASMCODE3(a, b, c)       ASMCODE2(a, b) ASMCODE1(c)
#define ASMCODE4(a, b, c, d)    ASMCODE3(a, b, c) ASMCODE1(d)
#define ASMCODE5(a, b, c, d, e) ASMCODE4(a, b, c, d) ASMCODE1(e)
#define ASMCODEW(n)             ASMCODE2(((n) AND 0xff), ((n) / 0x100))
#define ASMCODED(n)             ASMCODEW(n) ASMCODEW((n) / 0x10000)
#define ASMALIGN(n)             ALIGN n
#define __ALIGNC__              ASMALIGN(8)
#define __INLINEMSC__
#define ASM_BEGIN
#define ASM_ENDUP

#elif (defined(__BORLANDC__) || defined(__WATCOMC__))
#define ASMCODE1(a)             DB a; 
#define ASMCODE2(a, b)          DB a, b; 
#define ASMCODE3(a, b, c)       DB a, b, c; 
#define ASMCODE4(a, b, c, d)    DB a, b, c, d;
#define ASMCODE5(a, b, c, d, e) DB a, b, c, d, e;
#define ASMCODEW(n)             DW n;
#define ASMCODED(n)             DD n;
#define ASMALIGN(n)             ALIGN n
#pragma warn -8002  
#pragma warn -8004  
#pragma warn -8008  
#pragma warn -8012
#pragma warn -8027
#pragma warn -8057  
#pragma warn -8066  
#define __ALIGNC__              ASMALIGN(4)
#define __INLINEMSC__
//#ifdef __WATCOMC__
#undef __ALIGNC__
#define __ALIGNC__
//#endif
#define ASM_BEGIN
#define ASM_ENDUP
#endif
#endif


#ifndef ASM_MMPREFETCH
#define ASM_MMPREFETCH
//---------------------------------------------------------------------
// cache instructions operation code generation - SSE only          
//---------------------------------------------------------------------
#define mm_prefetch_esi      ASMCODE3(0x0f, 0x18, 0x06)
#define mm_prefetch_edi      ASMCODE3(0x0f, 0x18, 0x07)
#define mm_sfence            ASMCODE3(0x0f, 0xae, 0xf8)
#endif


//---------------------------------------------------------------------
// transform instructions operation code generation - SSE only      
//---------------------------------------------------------------------
#define mm_maskmovq(MMA, MMB) ASMCODE3(0x0f, 0xf7, 0xc0 | ((MMA)<<3)|(MMB) )
#define mm_maskmovq_0_2 ASMCODE3(0x0f, 0xf7, 0xC2)
#define mm_maskmovq_1_3 ASMCODE3(0x0f, 0xf7, 0xCB)


#ifndef ASM_MMPREFETCHX
#define ASM_MMPREFETCHX
//---------------------------------------------------------------------
// cache instructions extended operation code generation - SSE only 
//---------------------------------------------------------------------
#define mm_prefetch_esi_n(n) ASMCODE3(0x0f, 0x18, 0x86) ASMCODED(n)
#define mm_prefetch_edi_n(n) ASMCODE3(0x0f, 0x18, 0x87) ASMCODED(n)
#define mm_prefetch_esi_8cx_n(n) ASMCODE4(0x0f,0x18,0x84,0xce) ASMCODED(n)
#define mm_prefetch_esi_8dx ASMCODE4(0x0f, 0x18, 0x04, 0xd6)
#define mm_prefetch_edi_8dx ASMCODE4(0x0f, 0x18, 0x04, 0xd7)
#ifdef __WATCOMC__
#undef mm_prefetch_esi_n
#undef mm_prefetch_edi_n
#undef mm_prefetch_esi_8cx_n
#define mm_prefetch_esi_n(n) DD 0x86180f90, n
#define mm_prefetch_edi_n(n) DD 0x86180f90, n
#define mm_prefetch_esi_8cx_n(n) DD 0xCE84180f, n
#endif
#endif

//---------------------------------------------------------------------
// data instructions operation code generation - SSE only           
//---------------------------------------------------------------------
#define mm_movntq_edi_0_m0  ASMCODE3(0x0f, 0xe7, 0x07)
#define mm_movntq_edi_8_m1  ASMCODE4(0x0f, 0xe7, 0x4f, 0x08)
#define mm_movntq_edi_16_m2 ASMCODE4(0x0f, 0xe7, 0x57, 0x10)
#define mm_movntq_edi_24_m3 ASMCODE4(0x0f, 0xe7, 0x5f, 0x18)
#define mm_movntq_edi_32_m4 ASMCODE4(0x0f, 0xe7, 0x67, 0x20)
#define mm_movntq_edi_40_m5 ASMCODE4(0x0f, 0xe7, 0x6f, 0x28)
#define mm_movntq_edi_48_m6 ASMCODE4(0x0f, 0xe7, 0x77, 0x30)
#define mm_movntq_edi_56_m7 ASMCODE4(0x0f, 0xe7, 0x7f, 0x38)


//---------------------------------------------------------------------
// data instructions operation code generation - SSE only           
//---------------------------------------------------------------------
#define mm_movntq_edi_8cx_0_m0  ASMCODE4(0x0f, 0xe7, 0x04, 0xcf)
#define mm_movntq_edi_8cx_8_m1  ASMCODE5(0x0f, 0xe7, 0x4c, 0xcf, 0x08)
#define mm_movntq_edi_8cx_16_m2 ASMCODE5(0x0f, 0xe7, 0x54, 0xcf, 0x10)
#define mm_movntq_edi_8cx_24_m3 ASMCODE5(0x0f, 0xe7, 0x5c, 0xcf, 0x18)
#define mm_movntq_edi_8cx_32_m4 ASMCODE5(0x0f, 0xe7, 0x64, 0xcf, 0x20)
#define mm_movntq_edi_8cx_40_m5 ASMCODE5(0x0f, 0xe7, 0x6c, 0xcf, 0x28)
#define mm_movntq_edi_8cx_48_m6 ASMCODE5(0x0f, 0xe7, 0x74, 0xcf, 0x30)
#define mm_movntq_edi_8cx_56_m7 ASMCODE5(0x0f, 0xe7, 0x7c, 0xcf, 0x38)


#define _MMX_FEATURE_BIT        0x00800000
#define _SSE_FEATURE_BIT        0x02000000
#define _SSE2_FEATURE_BIT       0x04000000
#define _3DNOW_FEATURE_BIT      0x80000000

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
// X86 FEATURE DEFINITION
//---------------------------------------------------------------------
#define X86_FEATURE_FPU		(0*32+ 0) /* Onboard FPU */
#define X86_FEATURE_VME		(0*32+ 1) /* Virtual Mode Extensions */
#define X86_FEATURE_DE		(0*32+ 2) /* Debugging Extensions */
#define X86_FEATURE_TSC		(0*32+ 4) /* Time Stamp Counter */
#define X86_FEATURE_MSR		(0*32+ 5) /* Model-Specific Registers */
#define X86_FEATURE_PAE		(0*32+ 6) /* Physical Address Extensions */
#define X86_FEATURE_MCE		(0*32+ 7) /* Machine Check Architecture */
#define X86_FEATURE_CX8		(0*32+ 8) /* CMPXCHG8 instruction */
#define X86_FEATURE_MCA		(0*32+14) /* Machine Check Architecture */
#define X86_FEATURE_CMOV	(0*32+15) /* CMOV (FCMOVCC/FCOMI too if FPU) */
#define X86_FEATURE_PAT		(0*32+16) /* Page Attribute Table */
#define X86_FEATURE_PSE36	(0*32+17) /* 36-bit PSEs */
#define X86_FEATURE_CLFLSH	(0*32+19) /* Supports the CLFLUSH instruction */
#define X86_FEATURE_MMX		(0*32+23) /* Multimedia Extensions */
#define X86_FEATURE_XMM		(0*32+25) /* Streaming SIMD Extensions */
#define X86_FEATURE_XMM2	(0*32+26) /* Streaming SIMD Extensions-2 */
#define X86_FEATURE_SELFSNOOP	(0*32+27) /* CPU self snoop */
#define X86_FEATURE_HT		(0*32+28) /* Hyper-Threading */
#define X86_FEATURE_ACC		(0*32+29) /* Automatic clock control */
#define X86_FEATURE_IA64	(0*32+30) /* IA-64 processor */

#define X86_FEATURE_MP		(1*32+19) /* MP Capable. */
#define X86_FEATURE_NX		(1*32+20) /* Execute Disable */
#define X86_FEATURE_MMXEXT	(1*32+22) /* AMD MMX extensions */
#define X86_FEATURE_LM		(1*32+29) /* Long Mode (x86-64) */
#define X86_FEATURE_3DNOWEXT	(1*32+30) /* AMD 3DNow! extensions */
#define X86_FEATURE_3DNOW	(1*32+31) /* 3DNow! */

#define X86_FEATURE_CXMMX	(3*32+ 0) /* Cyrix MMX extensions */
#define X86_FEATURE_K6_MTRR	(3*32+ 1) /* AMD K6 nonstandard MTRRs */
#define X86_FEATURE_CYRIX_ARR	(3*32+ 2) /* Cyrix ARRs (= MTRRs) */
#define X86_FEATURE_CENTAUR_MCR	(3*32+ 3) /* Centaur MCRs (= MTRRs) */
#define X86_FEATURE_K8		(3*32+ 4) /* Opteron, Athlon64 */
#define X86_FEATURE_K7		(3*32+ 5) /* Athlon */
#define X86_FEATURE_P3		(3*32+ 6) /* P3 */
#define X86_FEATURE_P4		(3*32+ 7) /* P4 */
#define X86_FEATURE_EST		(4*32+ 7) /* Enhanced SpeedStep */
#define X86_FEATURE_MWAIT	(4*32+ 3) /* Monitor/Mwait support */

#define X86_VENDOR_INTEL 0
#define X86_VENDOR_CYRIX 1
#define X86_VENDOR_AMD 2
#define X86_VENDOR_UMC 3
#define X86_VENDOR_NEXGEN 4
#define X86_VENDOR_CENTAUR 5
#define X86_VENDOR_RISE 6
#define X86_VENDOR_TRANSMETA 7
#define X86_VENDOR_NSC 8
#define X86_VENDOR_NUM 9
#define X86_VENDOR_SIS 10
#define X86_VENDOR_UNKNOWN 0xff


//---------------------------------------------------------------------
// cpu global information
//---------------------------------------------------------------------
extern unsigned long _cpu_feature[];
extern unsigned long _cpu_cachesize;
extern int _cpu_level;
extern int _cpu_device;
extern int _cpu_vendor;
extern char _cpu_vendor_name[];
extern int _cpu_cache_l1i;
extern int _cpu_cache_l1d;
extern int _cpu_cache_l2;

#define _TEST_BIT(p, x) (((unsigned long*)(p))[(x) >> 5] & (1 << ((x) & 31)))
#define X86_FEATURE(x) _TEST_BIT(_cpu_feature, x)

#ifndef IMASK32
#define IMASK32 unsigned long
#endif

#ifndef ISRCPTR
#define ISRCPTR const char *
#endif

void _x86_cpuid(int op, int *eax, int *ebx, int *ecx, int *edx);
void _x86_detect(void);

void _x86_choose_blitter(void);

int iblit_386(char*, long, const char*, int, int, long, int, long);
int iblit_mmx(char*, long, const char*, int, int, long, int, long);
int iblit_sse(char*, long, const char*, int, int, long, int, long);
int iblit_mix(char*, long, const char*, int, int, long, int, long);

int iblit_mask_mmx(char*, long, ISRCPTR, int, int, long, int, long, IMASK32);
int iblit_mask_sse(char*, long, ISRCPTR, int, int, long, int, long, IMASK32);
int iblit_mask_mix(char*, long, ISRCPTR, int, int, long, int, long, IMASK32);
int iblit_mask_xmm(char*, long, ISRCPTR, int, int, long, int, long, IMASK32);



#ifdef __cplusplus
}
#endif


#endif

#endif


