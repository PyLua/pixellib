//=====================================================================
//
// iblit386.c - bitmap blitters implemention in i386
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iblit386.h"
#include "ibitmapm.h"

#ifdef __i386__


//---------------------------------------------------------------------
// global definition
//---------------------------------------------------------------------
unsigned long _cpu_feature[4] = { 0, 0, 0, 0 };
unsigned long _cpu_cachesize;
int _cpu_level = -1;
int _cpu_device = -1;
int _cpu_vendor = X86_VENDOR_UNKNOWN;
char _cpu_vendor_name[14] = "unknow";

int _cpu_cache_l1i = 0;
int _cpu_cache_l1d = 0;
int _cpu_cache_l2 = 0;


//---------------------------------------------------------------------
// _x86_cpu_types
//---------------------------------------------------------------------
struct _x86_cpu_types 
{
	int vendor;
	char ident[14];
	char name[14];
};

//---------------------------------------------------------------------
// _x86_cpu_ident
//---------------------------------------------------------------------
static struct _x86_cpu_types _x86_cpu_ident[] = {
	{ X86_VENDOR_INTEL,     "GenuineIntel", "Intel" }, 
	{ X86_VENDOR_CYRIX,     "CyrixInstead", "Cyrix" }, 
	{ X86_VENDOR_AMD,       "AuthenticAMD", "AMD" },        
	{ X86_VENDOR_UMC,       "UMC UMC UMC ", "UMC" }, 
	{ X86_VENDOR_NEXGEN,    "NexGenDriven", "NexGen" }, 
	{ X86_VENDOR_CENTAUR,   "CentaurHauls", "Centaur" }, 
	{ X86_VENDOR_RISE,      "RiseRiseRise", "Rise" }, 
	{ X86_VENDOR_TRANSMETA, "GenuineTMx86", "Transmeta" }, 
	{ X86_VENDOR_TRANSMETA, "TransmetaCPU", "Transmeta" }, 
	{ X86_VENDOR_NSC,       "Geode by NSC", "NSC" }, 
	{ X86_VENDOR_SIS,       "SiS SiS SiS ", "SiS" },
	{ -1, "", "Unknow" }
};


//---------------------------------------------------------------------
// _x86_cpuid
//---------------------------------------------------------------------
void _x86_cpuid(int op, int *eax, int *ebx, int *ecx, int *edx)
{
	int a = op, b = 0, c = 0, d = 0;
	
	#define mm_cpuid ASMCODE2(0x0F, 0xA2)	
	#ifdef __INLINEGNU__
	__asm__ __volatile__(
		"	pushal\n"
		"	movl %0, %%eax\n"
		mm_cpuid
		"	movl %%eax, %0\n"
		"	movl %%ebx, %1\n"
		"	movl %%ecx, %2\n"
		"	movl %%edx, %3\n"
		"	popal\n"
		: "=m" (a), "=m" (b), "=m" (c), "=m" (d) 
		: "a" (op): "memory");
	#elif defined(__INLINEMSC__)
	_asm {
		pushad
		mov eax, op
		mm_cpuid
		mov a, eax
		mov b, ebx
		mov c, ecx
		mov d, edx
		popad
	}
	#endif
	#undef mm_cpuid
	if (eax) *eax = a;
	if (ebx) *ebx = b;
	if (ecx) *ecx = c;
	if (edx) *edx = d;
}

//---------------------------------------------------------------------
// _flag_changeable
//---------------------------------------------------------------------
static int _flag_changeable(unsigned long flag) 
{ 
	unsigned long f1 = 0, f2 = 0;
	#ifdef __INLINEGNU__
	__asm__ __volatile__ ( 
		"pushfl\n" 
		"pushfl\n" 
		"popl %0\n" 
		"movl %0,%1\n" 
		"xorl %2,%0\n" 
		"pushl %0\n" 
		"popfl\n" 
		"pushfl\n" 
		"popl %0\n" 
		"popfl\n" 
		: "=&r" (f1), "=&r" (f2) 
		: "ir" (flag) : "memory"); 
	#elif defined(__INLINEMSC__)
	_asm {
		pushfd
		pushfd
		pop eax
		mov f1, eax
		mov f2, eax
		xor eax, flag
		push eax
		popfd
		pushfd
		pop eax
		mov f1, eax
		popfd
	}
	#endif
	return ((f1^f2) & flag) != 0; 
} 

//---------------------------------------------------------------------
// _x86_detect_cpu
//---------------------------------------------------------------------
static void _x86_detect_cpu(void)
{
	unsigned long test = 0;
	int eax, edx, i;

	// setup default value
	_cpu_vendor = X86_VENDOR_UNKNOWN;
	_cpu_vendor_name[0] = 0;
	_cpu_device = -1;
	_cpu_level = -1;

	// check weather have cpuid instruction
	if (!_flag_changeable(0x00200000)) {	// have not cpuid	
		if (_flag_changeable(0x00040000)) 
			_cpu_device = 0x00000400;	// 486
		else 
			_cpu_device = 0x00000300;	// 386

		if (_cpu_device == 0x00000400) {
			#ifdef __INLINEGNU__
			__asm__ __volatile__ (
				"pushl %%ebx\n"
				"movl $5, %%eax\n"
				"movl $2, %%ebx\n"
				"sahf\n"
				"div %%ebx\n"
				"lahf\n"
				"movl %%eax, %0\n"
				"popl %%ebx\n"
				:"=m"(test): :"memory");
			#elif defined(__INLINEMSC__)
			unsigned long flag = 0;
			_asm {
				mov eax, 5
				mov ebx, 2
				sahf
				div ebx
				lahf
				mov flag, eax
			}
			test = flag;
			#endif
			if ((test & 0xFF00) == 0x0200) 
				memcpy(_cpu_vendor_name, "CyrixInstead", 13);
		}
		if (_cpu_vendor_name[0] == 0) {
			#ifdef __INLINEGNU__
			__asm__ __volatile__ ( 
				"movl $0x5555, %%eax\n" 
				"xorw %%dx,%%dx\n" 
				"movw $2, %%cx\n" 
				"divw %%cx\n" 
				"xorl %%eax, %%eax\n" 
				"jnz x86_detect_cpu_l2\n" 
				"movl $1, %%eax\n" 
				"x86_detect_cpu_l2:\n" 
				: "=a" (test) : : "cx", "dx" ); 
			#elif defined(__INLINEMSC__)
			unsigned long flag = 0;
			_asm {
				mov eax, 0x5555
				xor dx, dx
				mov cx, 2
				div cx
				xor eax, eax
				jnz L1
				mov eax, 1
			L1:
				mov flag, eax
			}
			test = flag;
			#endif
			if (test) memcpy(_cpu_vendor_name, "NexGenDriven", 13);
		}
	}	else {		// have cpuid
		int regs[4];
		char *p = (char*)regs;
		_x86_cpuid(0, regs, regs + 1, regs + 2, regs + 3);
		_cpu_level = regs[0];
		for (i = 0; i < 12; i++) _cpu_vendor_name[i] = p[i + 4];
		_cpu_vendor_name[i] = 0;

		if (_cpu_level >= 1) {
			*(int*)(&_cpu_device) = 0;
			_x86_cpuid(1, &_cpu_device, 0, 0, 0);
		}	else {
			_cpu_device = 0x00000400;
		}
	}

	for (i = 0; _x86_cpu_ident[i].vendor >= 0; i++) {
		if (memcmp(_cpu_vendor_name, _x86_cpu_ident[i].ident, 12) == 0) {
			_cpu_vendor = _x86_cpu_ident[i].vendor;
			break;
		}
	}
	for (i = 0; i < 4; i++) 
		_cpu_feature[i] = 0;

	if (_cpu_level >= 1) {
		_x86_cpuid(1, 0, 0, 0, &edx);
		_cpu_feature[0] = (unsigned long)edx;
		_x86_cpuid(0x80000000uL, &eax, 0, 0, 0);
		if ((unsigned long)eax != 0x80000000ul) {
			_x86_cpuid(0x80000001uL, 0, 0, 0, &edx);
			_cpu_feature[1] = (unsigned long)edx;
		}
	}
}


//---------------------------------------------------------------------
// _x86_detect_cpu
//---------------------------------------------------------------------
void _x86_detect(void)
{
	static int flag = 0;
	if (flag) return;
	_x86_detect_cpu();
	flag = 1;
}


//---------------------------------------------------------------------
// _x86_choose_blitter
//---------------------------------------------------------------------
void _x86_choose_blitter(void)
{
	_x86_detect();
	ibitmap_funcset(0, (void*)iblit_386);
	ibitmap_funcset(1, NULL);
	if (X86_FEATURE(X86_FEATURE_MMX)) {
		if (X86_FEATURE(X86_FEATURE_XMM)) 
			ibitmap_funcset(0, (void*)iblit_mix);
		else
			ibitmap_funcset(0, (void*)iblit_mmx);
	}
	if (X86_FEATURE(X86_FEATURE_MMX)) {
		if (X86_FEATURE(X86_FEATURE_XMM)) {
			if (X86_FEATURE(X86_FEATURE_XMM2))
				ibitmap_funcset(1, (void*)iblit_mask_xmm);
			else
				ibitmap_funcset(1, (void*)iblit_mask_mix);
		}	else {
			ibitmap_funcset(1, (void*)iblit_mask_mmx);
		}
	}
}


//---------------------------------------------------------------------
// iblit_386 - 386 normal blitter
// this routine is design to support the platform elder than 80486
//---------------------------------------------------------------------
int iblit_386(char *dst, long pitch1, const char *src, 
	int w, int h, long pitch2, int pixelbyte, long linesize)
{
	long linebytes = linesize;
	long c1, c2;

	if (pixelbyte == 1) linebytes = w;
	c1 = pitch1 - linesize;
	c2 = pitch2 - linesize;

	#ifdef __INLINEGNU__
	__asm__ __volatile__ ("\n"
			ASM_BEGIN
		"	movl %1, %%esi\n"
		"	movl %2, %%edi\n"
		"	movl %3, %%eax\n"
		"	movl %4, %%ebx\n"
		"	movl %5, %%edx\n"
		"	cld\n"
		__ALIGNC__
		".blit_386n_loopline:\n"
		"	movl %%eax, %%ecx\n"
		"	shrl $2, %%ecx\n"
		"	rep movsl\n"
		"	movl %%eax, %%ecx\n"
		"	andl $3, %%ecx\n"
		"	rep movsb\n"
		"	addl %%ebx, %%edi\n"
		"	addl %%edx, %%esi\n"
		"	decl %0\n"
		"	jnz .blit_386n_loopline\n"
			ASM_ENDUP
		:"=m"(h)
		:"m"(src), "m"(dst), "m"(linebytes), "m"(c1), "m"(c2)
		:"memory", ASM_REGS);
	#elif defined(__INLINEMSC__)
	_asm {
		mov esi, src
		mov edi, dst
		mov eax, linebytes
		mov ebx, c1
		mov edx, c2
		cld
	blit_386n_loopline:
		mov ecx, eax
		shr ecx, 2
		rep movsd
		mov ecx, eax
		and ecx, 3
		rep movsb
		add edi, ebx
		add esi, edx
		dec h
		jnz blit_386n_loopline
	}
	#else
	for (; h; h--) {
		memcpy(dst, src, linebytes);
		dst += linebytes + c1;
		src += linebytes + c2;
	}
	#endif

	return 0;
}


//---------------------------------------------------------------------
// iblit_mmx - mmx normal blitter
// this routine is design to support the platform with MMX feature
//---------------------------------------------------------------------
int iblit_mmx(char *dst, long pitch1, const char *src, 
	int w, int h, long pitch2, int pixelbyte, long linesize)
{
	long c1, c2, m, n1, n2;

	if (pixelbyte == 1) linesize = w;

	c1 = pitch1 - linesize;
	c2 = pitch2 - linesize;
	m = linesize >> 6;
	n1 = (linesize & 63) >> 2;
	n2 = (linesize & 63) & 3;

#if defined(__INLINEGNU__)
	__asm__ __volatile__ ( "\n"
			ASM_BEGIN
		"	movl %1, %%esi\n"
		"	movl %2, %%edi\n"
		"	movl %3, %%eax\n"
		"	movl %4, %%ebx\n"
		"	movl %5, %%edx\n"
		"	cld\n"
		__ALIGNC__
		".mmxn_loopline:\n"
		"	movl %%eax, %%ecx\n"
		"	testl %%eax, %%eax\n"
		"	jz .mmxn_last\n"
		__ALIGNC__
		".mmxn_looppixel:\n"
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm1\n"
		"	movq 16(%%esi), %%mm2\n"
		"	movq 24(%%esi), %%mm3\n"
		"	movq 32(%%esi), %%mm4\n"
		"	movq 40(%%esi), %%mm5\n"
		"	movq 48(%%esi), %%mm6\n"
		"	movq 56(%%esi), %%mm7\n"
		"	movq %%mm0, 0(%%edi)\n"
		"	movq %%mm1, 8(%%edi)\n"
		"	movq %%mm2, 16(%%edi)\n"
		"	movq %%mm3, 24(%%edi)\n"
		"	movq %%mm4, 32(%%edi)\n"
		"	movq %%mm5, 40(%%edi)\n"
		"	movq %%mm6, 48(%%edi)\n"
		"	movq %%mm7, 56(%%edi)\n"
		"	addl $64, %%esi\n"
		"	addl $64, %%edi\n"
		"	decl %%ecx\n"
		"	jnz .mmxn_looppixel\n"
		__ALIGNC__
		".mmxn_last:\n"
		"	movl %%ebx, %%ecx\n"
		"	rep movsl\n"
		"	movl %%edx, %%ecx\n"
		"	rep movsb\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz .mmxn_loopline\n"
		"	emms\n"
			ASM_ENDUP
		:"=m"(h)
		:"m"(src),"m"(dst),"m"(m),"m"(n1),"m"(n2),"m"(c1),"m"(c2),"m"(h)
		:"memory", ASM_REGS);

#elif defined(__INLINEMSC__)
	_asm {
		mov esi, src
		mov edi, dst
		mov eax, m
		mov ebx, n1
		mov edx, n2
		cld
	__ALIGNC__
	loop_line:
		mov ecx, eax
		test eax, eax
		jz jmp_next
	__ALIGNC__
	loop_pixel:
		movq mm0, [esi + 0]
		movq mm1, [esi + 8]
		movq mm2, [esi + 16]
		movq mm3, [esi + 24]
		movq mm4, [esi + 32]
		movq mm5, [esi + 40]
		movq mm6, [esi + 48]
		movq mm7, [esi + 56]
		movq [edi + 0], mm0
		movq [edi + 8], mm1
		movq [edi + 16], mm2
		movq [edi + 24], mm3
		movq [edi + 32], mm4
		movq [edi + 40], mm5
		movq [edi + 48], mm6
		movq [edi + 56], mm7
		add esi, 64
		add edi, 64
		dec ecx
		jnz loop_pixel
	__ALIGNC__
	jmp_next:
		mov ecx, ebx
		rep movsd
		mov ecx, edx
		rep movsb
		add edi, c1
		add esi, c2
		dec h
		jnz loop_line
		emms
	}
#else
	return -1;
#endif

	return 0;
}


//---------------------------------------------------------------------
// iblit_sse - mmx normal blitter
// this routine is design to support the platform with sse feature,
// which has more than 256KB L2 cache
//---------------------------------------------------------------------
int iblit_sse(char *dst, long pitch1, const char *src, 
	int w, int h, long pitch2, int pixelbyte, long linesize)
{
	long c1, c2, m, n1, n2;

	if (pixelbyte == 1) linesize = w;
	c1 = pitch1 - linesize;
	c2 = pitch2 - linesize;
	m = linesize >> 6;
	n1 = (linesize & 63) >> 2;
	n2 = (linesize & 63) & 3;

#if defined(__INLINEGNU__)
	__asm__ __volatile__ ( "\n"
			ASM_BEGIN
		"	movl %1, %%esi\n"
		"	movl %2, %%edi\n"
		"	movl %3, %%eax\n"
		"	movl %4, %%ebx\n"
		"	movl %5, %%edx\n"
			mm_prefetch_esi_n(0)
		"	shll $3, %%eax\n"
		"	cld\n"
		__ALIGNC__
		".ssen_loop_line:\n"
			mm_prefetch_esi_n(32)
		"	movl %%eax, %%ecx\n"
		"	negl %%ecx\n"
		"	leal 0(%%esi, %%eax, 8), %%esi\n"
		"	leal 0(%%edi, %%eax, 8), %%edi\n"
		"	testl %%eax, %%eax\n"
		"	jz .ssen_next\n"
			mm_prefetch_esi_8cx_n(0)
		__ALIGNC__
		".ssen_loop_pixel:\n"
			mm_prefetch_esi_8cx_n(256)
		"	movq 0(%%esi, %%ecx, 8), %%mm0\n"
		"	movq 8(%%esi, %%ecx, 8), %%mm1\n"
		"	movq 16(%%esi, %%ecx, 8), %%mm2\n"
		"	movq 24(%%esi, %%ecx, 8), %%mm3\n"
		"	movq 32(%%esi, %%ecx, 8), %%mm4\n"
		"	movq 40(%%esi, %%ecx, 8), %%mm5\n"
		"	movq 48(%%esi, %%ecx, 8), %%mm6\n"
		"	movq 56(%%esi, %%ecx, 8), %%mm7\n"
			mm_prefetch_esi_8cx_n(128)
			mm_movntq_edi_8cx_0_m0
			mm_movntq_edi_8cx_8_m1
			mm_movntq_edi_8cx_16_m2
			mm_movntq_edi_8cx_24_m3
			mm_movntq_edi_8cx_32_m4
			mm_movntq_edi_8cx_40_m5
			mm_movntq_edi_8cx_48_m6
			mm_movntq_edi_8cx_56_m7
		"	addl $8, %%ecx\n"
			mm_prefetch_esi_8cx_n(64)
		"	jnz .ssen_loop_pixel\n"
		__ALIGNC__
		".ssen_next:\n"
			mm_prefetch_esi_n(0)
		"	movl %%ebx, %%ecx\n"
		"	rep movsl\n"
		"	movl %%edx, %%ecx\n"
		"	rep movsb\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
			mm_prefetch_esi_n(0)
		"	decl %0\n"
		"	jnz .ssen_loop_line\n"
			mm_sfence
		"	emms\n"
			ASM_ENDUP
		:"=m"(h)
		:"m"(src),"m"(dst),"m"(m),"m"(n1),"m"(n2),"m"(c1),"m"(c2),"m"(h)
		:"memory", ASM_REGS);
	#elif defined(__INLINEMSC__)
	_asm {
		mov esi, src
		mov edi, dst
		mm_prefetch_esi_n(64)
		mm_prefetch_edi_n(64)
		mm_prefetch_esi
		mm_prefetch_edi
		mov eax, m
		shl eax, 3
		mov ebx, n1
		mov edx, n2
		cld
	__ALIGNC__
	ssen_loop_line:
		mm_prefetch_esi
		mm_prefetch_esi_n(64)
		mm_prefetch_esi_n(128)
		mm_prefetch_esi_n(192)
		mov ecx, eax
		neg ecx
		lea esi, [esi + eax * 8]
		lea edi, [edi + eax * 8]
		test eax, eax
		jz ssen_jmp_next
	__ALIGNC__
	ssen_loop_pixel:
		mm_prefetch_esi_8cx_n(256)
		mm_prefetch_esi_8cx_n(128)
		movq mm0, [esi + ecx * 8 +  0]
		movq mm1, [esi + ecx * 8 +  8]
		movq mm2, [esi + ecx * 8 + 16]
		movq mm3, [esi + ecx * 8 + 24]
		movq mm4, [esi + ecx * 8 + 32]
		movq mm5, [esi + ecx * 8 + 40]
		movq mm6, [esi + ecx * 8 + 48]
		movq mm7, [esi + ecx * 8 + 56]
		mm_movntq_edi_8cx_0_m0  
		mm_movntq_edi_8cx_8_m1  
		mm_movntq_edi_8cx_16_m2 
		mm_movntq_edi_8cx_24_m3 
		mm_movntq_edi_8cx_32_m4 
		mm_movntq_edi_8cx_40_m5 
		mm_movntq_edi_8cx_48_m6 
		mm_movntq_edi_8cx_56_m7 
		add ecx, 8
		jnz ssen_loop_pixel
	__ALIGNC__
	ssen_jmp_next:
		mm_prefetch_esi
		mov ecx, ebx
		rep movsd
		mov ecx, edx
		rep movsb
		add edi, c1
		add esi, c2
		mm_prefetch_esi
		dec h
		jnz ssen_loop_line
		mm_sfence
		emms
	}
	#else
	return -1;
	#endif

	return 0;
}


//---------------------------------------------------------------------
// iblit_mix - mix normal blitter
// this routine is design to support the platform with MMX feature
//---------------------------------------------------------------------
int iblit_mix(char *dst, long pitch1, const char *src, 
	int w, int h, long pitch2, int pixelbyte, long linesize)
{
	long c1, c2, m, n1, n2;

	if (pixelbyte == 1) linesize = w;

	c1 = pitch1 - linesize;
	c2 = pitch2 - linesize;
	m = linesize >> 6;
	n1 = (linesize & 63) >> 2;
	n2 = (linesize & 63) & 3;

#if defined(__INLINEGNU__)
	__asm__ __volatile__ ( "\n"
			ASM_BEGIN
		"	movl %1, %%esi\n"
		"	movl %2, %%edi\n"
		"	movl %3, %%eax\n"
		"	movl %4, %%ebx\n"
		"	movl %5, %%edx\n"
		"	cld\n"
		__ALIGNC__
		".mixn_loopline:\n"
		"	movl %%eax, %%ecx\n"
		"	testl %%eax, %%eax\n"
		"	jz .mixn_last\n"
		__ALIGNC__
		".mixn_looppixel:\n"
			mm_prefetch_esi_n(512)
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm1\n"
		"	movq 16(%%esi), %%mm2\n"
		"	movq 24(%%esi), %%mm3\n"
		"	movq 32(%%esi), %%mm4\n"
		"	movq 40(%%esi), %%mm5\n"
		"	movq 48(%%esi), %%mm6\n"
		"	movq 56(%%esi), %%mm7\n"
		"	movq %%mm0, 0(%%edi)\n"
		"	movq %%mm1, 8(%%edi)\n"
		"	movq %%mm2, 16(%%edi)\n"
		"	movq %%mm3, 24(%%edi)\n"
		"	movq %%mm4, 32(%%edi)\n"
		"	movq %%mm5, 40(%%edi)\n"
		"	movq %%mm6, 48(%%edi)\n"
		"	movq %%mm7, 56(%%edi)\n"
		"	addl $64, %%esi\n"
		"	addl $64, %%edi\n"
		"	decl %%ecx\n"
		"	jnz .mixn_looppixel\n"
		__ALIGNC__
		".mixn_last:\n"
		"	movl %%ebx, %%ecx\n"
		"	rep movsl\n"
		"	movl %%edx, %%ecx\n"
		"	rep movsb\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
			mm_prefetch_esi_n(64)
		"	decl %0\n"
		"	jnz .mixn_loopline\n"
		"	emms\n"
			ASM_ENDUP
		:"=m"(h)
		:"m"(src),"m"(dst),"m"(m),"m"(n1),"m"(n2),"m"(c1),"m"(c2),"m"(h)
		:"memory", ASM_REGS);

#elif defined(__INLINEMSC__)
	_asm {
		mov esi, src
		mov edi, dst
		mov eax, m
		mov ebx, n1
		mov edx, n2
		cld
	__ALIGNC__
	mixn_loop_line:
		mov ecx, eax
		test eax, eax
		jz mixn_jmp_next
	__ALIGNC__
	mixn_loop_pixel:
		mm_prefetch_esi_n(512)
		movq mm0, [esi + 0]
		movq mm1, [esi + 8]
		movq mm2, [esi + 16]
		movq mm3, [esi + 24]
		movq mm4, [esi + 32]
		movq mm5, [esi + 40]
		movq mm6, [esi + 48]
		movq mm7, [esi + 56]
		movq [edi + 0], mm0
		movq [edi + 8], mm1
		movq [edi + 16], mm2
		movq [edi + 24], mm3
		movq [edi + 32], mm4
		movq [edi + 40], mm5
		movq [edi + 48], mm6
		movq [edi + 56], mm7
		add esi, 64
		add edi, 64
		dec ecx
		jnz mixn_loop_pixel
	__ALIGNC__
	mixn_jmp_next:
		mov ecx, ebx
		rep movsd
		mov ecx, edx
		rep movsb
		add edi, c1
		add esi, c2
		mm_prefetch_esi_n(64)
		dec h
		jnz mixn_loop_line
		emms
	}
#else
	return -1;
#endif

	return 0;
}



//---------------------------------------------------------------------
// iblit_mask_mmx - mmx mask blitter 
// this routine is designed to support mmx mask blit
//---------------------------------------------------------------------
int iblit_mask_mmx(char *dst, long pitch1, const char *src, int w, int h, 
		long pitch2, int pixelbyte, long linesize, unsigned long ckey)
{
	long linebytes = linesize;
	long c1, c2, m, n, pb;
	unsigned long mask;

	switch (pixelbyte) {
	case 1:
		m = w >> 4;
		n = w & 15;
		mask = (ckey | (ckey << 8)) & 0xffff;
		mask = mask | (mask << 16);
		break;
	case 2: 
		linebytes = (((long)w) << 1); 
		m = w >> 3;
		n = w & 7;
		mask = ckey | (ckey << 16);
		break;
	case 3: 
		linebytes = (((long)w) << 1) + w; 
		m = 0;
		n = w;
		mask = ckey;
		break;
	case 4: 
		linebytes = (((long)w) << 2); 
		m = w >> 2;
		n = w & 3;
		mask = ckey;
		break;
	default: 
		linebytes = -1; 
		break;
	}

	if (linebytes < 0) return -1;

	c1 = pitch1 - linebytes;
	c2 = pitch2 - linebytes;
	pb = pixelbyte;

#if defined(__INLINEGNU__)
	__asm__ __volatile__ ("\n"
			ASM_BEGIN
		"	movl %1, %%esi\n"
		"	movl %2, %%edi\n"
		"	movd %3, %%mm7\n"
		"	movd %3, %%mm6\n"
		"	movl %3, %%ebx\n"
		"	psllq $32, %%mm7\n"
		"	por %%mm6, %%mm7\n"
		"	cld\n"
		"	movl %8, %%eax\n"
		"	cmpl $2, %%eax\n"
		"	jz bmaskmmx16_loopline\n"
		"	cmpl $4, %%eax\n"
		"	jz bmaskmmx32_loopline\n"
		"	cmpl $1, %%eax\n"
		"	jz bmaskmmx8_loopline\n"
		"	jmp bmaskmmx24_loopline\n"
		__ALIGNC__		/* bmaskmmx8_loopline */
		"bmaskmmx8_loopline:\n"
		"	movl %4, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmmx8_last\n"
		__ALIGNC__
		"bmaskmmx8_looppixel:\n"
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm4\n"
		"	movq %%mm0, %%mm1\n"
		"	movq %%mm4, %%mm5\n"
		"	pcmpeqb %%mm7, %%mm0\n"
		"	pcmpeqb %%mm7, %%mm4\n"
		"	addl $16, %%esi\n"
		"	addl $16, %%edi\n"
		"	movq %%mm0, %%mm2\n"
		"	movq %%mm4, %%mm6\n"
		"	pandn %%mm1, %%mm0\n"
		"	pandn %%mm5, %%mm4\n"
		"	pand -16(%%edi), %%mm2\n"
		"	pand -8(%%edi), %%mm6\n"
		"	por %%mm0, %%mm2\n"
		"	por %%mm4, %%mm6\n"
		"	movq %%mm2, -16(%%edi)\n"
		"	movq %%mm6, -8(%%edi)\n"
		"	decl %%ecx\n"
		"	jnz bmaskmmx8_looppixel\n"
		"bmaskmmx8_last:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmmx8_linend\n"
		"	movd %%mm7, %%ebx\n"
		"bmaskmmx8_l2:\n"
		"	lodsb\n"
		"	cmpb %%al, %%bl\n"
		"	jz bmaskmmx8_l3\n"
		"	movb %%al, (%%edi)\n"
		"bmaskmmx8_l3:\n"
		"	incl %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskmmx8_l2\n"
		"bmaskmmx8_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskmmx8_loopline\n"
		"	jmp bmaskmmx_endup\n"
		__ALIGNC__		/* bmaskmmx16_loopline */
		"bmaskmmx16_loopline:\n"
		"	movl %4, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmmx16_last\n"
		__ALIGNC__
		"bmaskmmx16_looppixel:\n"
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm4\n"
		"	movq %%mm0, %%mm1\n"
		"	movq %%mm4, %%mm5\n"
		"	pcmpeqw %%mm7, %%mm0\n"
		"	pcmpeqw %%mm7, %%mm4\n"
		"	addl $16, %%esi\n"
		"	addl $16, %%edi\n"
		"	movq %%mm0, %%mm2\n"
		"	movq %%mm4, %%mm6\n"
		"	pandn %%mm1, %%mm0\n"
		"	pandn %%mm5, %%mm4\n"
		"	pand -16(%%edi), %%mm2\n"
		"	pand -8(%%edi), %%mm6\n"
		"	por %%mm0, %%mm2\n"
		"	por %%mm4, %%mm6\n"
		"	movq %%mm2, -16(%%edi)\n"
		"	movq %%mm6, -8(%%edi)\n"
		"	decl %%ecx\n"
		"	jnz bmaskmmx16_looppixel\n"
		"bmaskmmx16_last:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmmx16_linend\n"
		"	movd %%mm7, %%ebx\n"
		"bmaskmmx16_l2:\n"
		"	lodsw\n"
		"	cmpw %%ax, %%bx\n"
		"	jz bmaskmmx16_l3\n"
		"	movw %%ax, (%%edi)\n"
		"bmaskmmx16_l3:\n"
		"	addl $2, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskmmx16_l2\n"
		"bmaskmmx16_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskmmx16_loopline\n"
		"	jmp bmaskmmx_endup\n"
		__ALIGNC__		/* bmaskmmx32_loopline */
		"bmaskmmx32_loopline:\n"
		"	movl %4, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmmx32_last\n"
		__ALIGNC__
		"bmaskmmx32_looppixel:\n"
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm4\n"
		"	movq %%mm0, %%mm1\n"
		"	movq %%mm4, %%mm5\n"
		"	pcmpeqd %%mm7, %%mm0\n"
		"	pcmpeqd %%mm7, %%mm4\n"
		"	addl $16, %%esi\n"
		"	addl $16, %%edi\n"
		"	movq %%mm0, %%mm2\n"
		"	movq %%mm4, %%mm6\n"
		"	pandn %%mm1, %%mm0\n"
		"	pandn %%mm5, %%mm4\n"
		"	pand -16(%%edi), %%mm2\n"
		"	pand -8(%%edi), %%mm6\n"
		"	por %%mm0, %%mm2\n"
		"	por %%mm4, %%mm6\n"
		"	movq %%mm2, -16(%%edi)\n"
		"	movq %%mm6, -8(%%edi)\n"
		"	decl %%ecx\n"
		"	jnz bmaskmmx32_looppixel\n"
		"bmaskmmx32_last:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmmx32_linend\n"
		"	movd %%mm7, %%ebx\n"
		"bmaskmmx32_l2:\n"
		"	lodsl\n"
		"	cmpl %%eax, %%ebx\n"
		"	jz bmaskmmx32_l3\n"
		"	movl %%eax, (%%edi)\n"
		"bmaskmmx32_l3:\n"
		"	addl $4, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskmmx32_l2\n"
		"bmaskmmx32_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskmmx32_loopline\n"
		"	jmp bmaskmmx_endup\n"
		__ALIGNC__		/* bmaskmmx24_loopline */
		"bmaskmmx24_loopline:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmmx24_linend\n"
		"	andl $0xffffff, %%ebx\n"
		"bmaskmmx24_looppixel:\n"
		"	movb 2(%%esi), %%al\n"
		"	movw 0(%%esi), %%dx\n"
		"	shll $16, %%eax\n"
		"	addl $3, %%esi\n"
		"	movw %%dx, %%ax\n"
		"	andl $0xffffff, %%eax\n"
		"	cmpl %%ebx, %%eax\n"
		"	jz bmaskmmx24_skip\n"
		"	movw %%ax, (%%edi)\n"
		"	shrl $16, %%eax\n"
		"	movb %%al, 2(%%edi)\n"
		"bmaskmmx24_skip:\n"
		"	addl $3, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskmmx24_looppixel\n"
		"bmaskmmx24_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskmmx24_loopline\n"
		"	jmp bmaskmmx_endup\n"
		__ALIGNC__		/* bmaskmmx_endup */
		"bmaskmmx_endup:\n"
		"	emms\n"
			ASM_ENDUP
		:"=m"(h)
		:"m"(src),"m"(dst),"m"(mask),"m"(m),"m"(n),"m"(c1),"m"(c2),"m"(pb)
		:"memory", ASM_REGS);

#elif defined(__INLINEMSC__)
	_asm {
		mov esi, src
		mov edi, dst
		movd mm7, mask
		movd mm6, mask
		mov ebx, mask
		psllq mm7, 32
		por mm7, mm6
		cld
		mov eax, pb
		cmp eax, 1
		jnz bmaskmmx16_check
	__ALIGNC__		/* bmaskmmx8_loopline */
	bmaskmmx8_loopline:
		mov ecx, m
		test ecx, ecx
		jz bmaskmmx8_last
	__ALIGNC__
	bmaskmmx8_looppixel:
		movq mm0, [esi + 0]
		movq mm4, [esi + 8]
		movq mm1, mm0
		movq mm5, mm4
		pcmpeqb mm0, mm7
		pcmpeqb mm4, mm7
		add esi, 16
		add edi, 16
		movq  mm2, mm0
		movq  mm6, mm4
		pandn mm0, mm1
		pandn mm4, mm5
		pand  mm2, [edi - 16]
		pand  mm6, [edi - 8]
		por  mm2, mm0
		por  mm6, mm4
		movq [edi - 16], mm2
		movq [edi - 8], mm6
		dec ecx
		jnz bmaskmmx8_looppixel
	bmaskmmx8_last:
		mov ecx, n
		test ecx, ecx
		jz bmaskmmx8_linend
	bmaskmmx8_l2:
		lodsb
		cmp al, bl
		jz bmaskmmx8_l3
		mov [edi], al
	bmaskmmx8_l3:
		inc edi
		dec ecx
		jnz bmaskmmx8_l2
	bmaskmmx8_linend:
		add edi, c1
		add esi, c2
		dec h
		jnz bmaskmmx8_loopline
	bmaskmmx16_check:
		mov eax, pb
		cmp eax, 2
		jnz bmaskmmx32_check
	__ALIGNC__		/* bmaskmmx16_loopline */
	bmaskmmx16_loopline:
		mov ecx, m
		test ecx, ecx
		jz bmaskmmx16_last
	__ALIGNC__
	bmaskmmx16_looppixel:
		movq mm0, [esi + 0]
		movq mm4, [esi + 8]
		movq mm1, mm0
		movq mm5, mm4
		pcmpeqw mm0, mm7
		pcmpeqw mm4, mm7
		add esi, 16
		add edi, 16
		movq  mm2, mm0
		movq  mm6, mm4
		pandn mm0, mm1
		pandn mm4, mm5
		pand  mm2, [edi - 16]
		pand  mm6, [edi - 8]
		por  mm2, mm0
		por  mm6, mm4
		movq [edi - 16], mm2
		movq [edi - 8], mm6
		dec ecx
		jnz bmaskmmx16_looppixel
	bmaskmmx16_last:
		mov ecx, n
		test ecx, ecx
		jz bmaskmmx16_linend
	bmaskmmx16_l2:
		lodsw
		cmp ax, bx
		jz bmaskmmx16_l3
		mov [edi], ax
	bmaskmmx16_l3:
		add edi, 2
		dec ecx
		jnz bmaskmmx16_l2
	bmaskmmx16_linend:
		add edi, c1
		add esi, c2
		dec h
		jnz bmaskmmx16_loopline
	bmaskmmx32_check:
		mov eax, pb
		cmp eax, 4
		jnz bmaskmmx24_check
	__ALIGNC__		/* bmaskmmx32_loopline */
	bmaskmmx32_loopline:
		mov ecx, m
		test ecx, ecx
		jz bmaskmmx32_last
	__ALIGNC__
	bmaskmmx32_looppixel:
		movq mm0, [esi + 0]
		movq mm4, [esi + 8]
		movq mm1, mm0
		movq mm5, mm4
		pcmpeqd mm0, mm7
		pcmpeqd mm4, mm7
		add esi, 16
		add edi, 16
		movq  mm2, mm0
		movq  mm6, mm4
		pandn mm0, mm1
		pandn mm4, mm5
		pand  mm2, [edi - 16]
		pand  mm6, [edi - 8]
		por  mm2, mm0
		por  mm6, mm4
		movq [edi - 16], mm2
		movq [edi - 8], mm6
		dec ecx
		jnz bmaskmmx32_looppixel
	bmaskmmx32_last:
		mov ecx, n
		test ecx, ecx
		jz bmaskmmx32_linend
	bmaskmmx32_l2:
		lodsd
		cmp eax, ebx
		jz bmaskmmx32_l3
		mov [edi], eax
	bmaskmmx32_l3:
		add edi, 4
		dec ecx
		jnz bmaskmmx32_l2
	bmaskmmx32_linend:
		add edi, c1
		add esi, c2
		dec h
		jnz bmaskmmx32_loopline
		jmp bmaskmmx_endup
	bmaskmmx24_check:
		mov eax, pb
		cmp eax, 3
		jnz bmaskmmx_endup
	__ALIGNC__		/* maskmmx24_loopline */
	bmaskmmx24_loopline:
		mov ecx, n
		test ecx, ecx
		jz bmaskmmx24_linend
		and ebx, 0xffffff
	__ALIGNC__
	bmaskmmx24_looppixel:
		lodsd
		dec esi
		and eax, 0xffffff
		cmp eax, ebx
		jz bmaskmmx24_skip
		mov [edi], ax
		shr eax, 16
		mov [edi + 2], al
	bmaskmmx24_skip:
		add edi, 3
		dec ecx
		jnz bmaskmmx24_looppixel
	bmaskmmx24_linend:
		add edi, c1
		add esi, c2
		dec h
		jnz bmaskmmx24_loopline
	__ALIGNC__
	bmaskmmx_endup:
		emms
	}
#else
	return -1;
#endif
	return 0;
}


//---------------------------------------------------------------------
// iblit_mask_sse - sse mask blitter 
// this routine is designed to support mmx mask blit
//---------------------------------------------------------------------
int iblit_mask_sse(char *dst, long pitch1, const char *src, int w, int h, 
		long pitch2, int pixelbyte, long linesize, unsigned long ckey)
{
	long linebytes = linesize;
	long c1, c2, m, n, pb;
	unsigned long mask;

	switch (pixelbyte) {
	case 1:
		m = w >> 4;
		n = w & 15;
		mask = (ckey | (ckey << 8)) & 0xffff;
		mask = mask | (mask << 16);
		break;
	case 2: 
		linebytes = (((long)w) << 1); 
		m = w >> 3;
		n = w & 7;
		mask = ckey | (ckey << 16);
		break;
	case 3: 
		linebytes = (((long)w) << 1) + w; 
		m = 0;
		n = w;
		mask = ckey;
		break;
	case 4: 
		linebytes = (((long)w) << 2); 
		m = w >> 2;
		n = w & 3;
		mask = ckey;
		break;
	default: 
		linebytes = -1; 
		break;
	}

	if (linebytes < 0) return -1;

	c1 = pitch1 - linebytes;
	c2 = pitch2 - linebytes;
	pb = pixelbyte;

#if defined(__INLINEGNU__)
	__asm__ __volatile__ ("\n"
			ASM_BEGIN
		"	movl %1, %%esi\n"
		"	movl %2, %%edi\n"
		"	movd %3, %%mm7\n"
		"	movd %3, %%mm6\n"
		"	movl %3, %%ebx\n"
		"	psllq $32, %%mm7\n"
		"	por %%mm6, %%mm7\n"
		"	pcmpeqw %%mm6, %%mm6\n"
		"	movl %8, %%eax\n"
		"	cmpl $2, %%eax\n"
		"	jz bmasksse16_loopline\n"
		"	cmpl $4, %%eax\n"
		"	jz bmasksse32_loopline\n"
		"	cmpl $1, %%eax\n"
		"	jz bmasksse8_loopline\n"
		"	jmp bmasksse24_loopline\n"
		"	cld\n"
		__ALIGNC__		/* bmasksse8_loopline */
		"bmasksse8_loopline:\n"
		"	movl %4, %%ecx\n"
			mm_prefetch_esi_n(32)
		"	testl %%ecx, %%ecx\n"
		"	jz bmasksse8_last\n"
		__ALIGNC__
		"bmasksse8_looppixel:\n"
			mm_prefetch_esi_n(512)
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm1\n"
		"	movq %%mm7, %%mm2\n"
		"	movq %%mm7, %%mm3\n"
			mm_prefetch_esi_n(128)
		"	pcmpeqb %%mm0, %%mm2\n"
		"	pcmpeqb %%mm1, %%mm3\n"
		"	addl $16, %%esi\n"
		"	pxor %%mm6, %%mm2\n"
		"	pxor %%mm6, %%mm3\n"
			mm_prefetch_esi_n(96)
			mm_maskmovq(0, 2)
		"	addl $8, %%edi\n"
			mm_prefetch_esi_n(64)
			mm_maskmovq(1, 3)
		"	addl $8, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmasksse8_looppixel\n"
		"bmasksse8_last:\n"
			mm_prefetch_esi_n(0)
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmasksse8_linend\n"
		"	movd %%mm7, %%ebx\n"
		"bmasksse8_l1:\n"
		"	lodsb\n"
		"	cmpb %%bl, %%al\n"
		"	jz bmasksse8_l2\n"
		"	movb %%al, (%%edi)\n"
		"bmasksse8_l2:\n"
		"	incl %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmasksse8_l1\n"
		"bmasksse8_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
			mm_prefetch_esi_n(0)
		"	decl %0\n"
		"	jnz bmasksse8_loopline\n"
		"	jmp bmasksse_endup\n"
		__ALIGNC__		/* bmasksse16_loopline */
		"bmasksse16_loopline:\n"
		"	movl %4, %%ecx\n"
			mm_prefetch_esi_n(32)
		"	testl %%ecx, %%ecx\n"
		"	jz bmasksse16_last\n"
		__ALIGNC__
		"bmasksse16_looppixel:\n"
			mm_prefetch_esi_n(160)
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm1\n"
		"	movq %%mm7, %%mm2\n"
		"	movq %%mm7, %%mm3\n"
			mm_prefetch_esi_n(128)
		"	pcmpeqw %%mm0, %%mm2\n"
		"	pcmpeqw %%mm1, %%mm3\n"
		"	addl $16, %%esi\n"
		"	pxor %%mm6, %%mm2\n"
		"	pxor %%mm6, %%mm3\n"
			mm_prefetch_esi_n(96)
			mm_maskmovq(0, 2)
		"	addl $8, %%edi\n"
			mm_prefetch_esi_n(64)
			mm_maskmovq(1, 3)
		"	addl $8, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmasksse16_looppixel\n"
		"bmasksse16_last:\n"
			mm_prefetch_esi_n(0)
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmasksse16_linend\n"
		"	movd %%mm7, %%ebx\n"
		"bmasksse16_l1:\n"
		"	lodsw\n"
		"	cmpw %%bx, %%ax\n"
		"	jz bmasksse16_l2\n"
		"	movw %%ax, (%%edi)\n"
		"bmasksse16_l2:\n"
		"	addl $2, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmasksse16_l1\n"
		"bmasksse16_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
			mm_prefetch_esi_n(0)
		"	decl %0\n"
		"	jnz bmasksse16_loopline\n"
		"	jmp bmasksse_endup\n"
		__ALIGNC__		/* bmasksse32_loopline */
		"bmasksse32_loopline:\n"
		"	movl %4, %%ecx\n"
			mm_prefetch_esi_n(32)
		"	testl %%ecx, %%ecx\n"
		"	jz bmasksse32_last\n"
		__ALIGNC__
		"bmasksse32_looppixel:\n"
			mm_prefetch_esi_n(160)
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm1\n"
		"	movq %%mm7, %%mm2\n"
		"	movq %%mm7, %%mm3\n"
			mm_prefetch_esi_n(128)
		"	pcmpeqd %%mm0, %%mm2\n"
		"	pcmpeqd %%mm1, %%mm3\n"
		"	addl $16, %%esi\n"
		"	pxor %%mm6, %%mm2\n"
		"	pxor %%mm6, %%mm3\n"
			mm_prefetch_esi_n(96)
			mm_maskmovq(0, 2)
		"	addl $8, %%edi\n"
			mm_prefetch_esi_n(64)
			mm_maskmovq(1, 3)
		"	addl $8, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmasksse32_looppixel\n"
		__ALIGNC__
		"bmasksse32_last:\n"
			mm_prefetch_esi_n(0)
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmasksse32_linend\n"
		"	movd %%mm7, %%ebx\n"
		"bmasksse32_l1:\n"
		"	lodsl\n"
		"	cmpl %%ebx, %%eax\n"
		"	jz bmasksse32_l2\n"
		"	movl %%eax, (%%edi)\n"
		"bmasksse32_l2:\n"
		"	addl $4, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmasksse32_l1\n"
		"bmasksse32_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
			mm_prefetch_esi_n(0)
		"	decl %0\n"
		"	jnz bmasksse32_loopline\n"
		"	jmp bmasksse_endup\n"
		__ALIGNC__		/* bmasksse24_loopline */
		"bmasksse24_loopline:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmasksse24_linend\n"
		"	andl $0xffffff, %%ebx\n"
		"bmasksse24_looppixel:\n"
		"	movb 2(%%esi), %%al\n"
		"	movw 0(%%esi), %%dx\n"
		"	shll $16, %%eax\n"
		"	addl $3, %%esi\n"
		"	movw %%dx, %%ax\n"
			mm_prefetch_esi_n(64)
		"	andl $0xffffff, %%eax\n"
		"	cmpl %%ebx, %%eax\n"
		"	jz bmasksse24_skip\n"
		"	movw %%ax, (%%edi)\n"
		"	shrl $16, %%eax\n"
		"	movb %%al, 2(%%edi)\n"
		"bmasksse24_skip:\n"
		"	addl $3, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmasksse24_looppixel\n"
		"bmasksse24_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmasksse24_loopline\n"
		"	jmp bmasksse_endup\n"
		"bmasksse_endup:\n"
			mm_sfence
		"	emms\n"
			ASM_ENDUP
		:"=m"(h)
		:"m"(src),"m"(dst),"m"(mask),"m"(m),"m"(n),"m"(c1),"m"(c2),"m"(pb)
		:"memory", ASM_REGS);

#elif defined(__INLINEMSC__)
	_asm {
		mov esi, src
		mov edi, dst
		mov ebx, mask
		movd mm7, mask
		movd mm6, mask
		psllq mm7, 32
		por mm7, mm6
		movq mm6, mm7
		pcmpeqw mm6, mm6
		mov edx, 8
		cld
		mov eax, pb
		cmp eax, 1
		jnz bmasksse16_check
	__ALIGNC__			/* bmasksse16_loopline */
	bmasksse8_loopline:	
		mov ecx, m
		test ecx, ecx
		jz bmasksse8_last
	__ALIGNC__
	bmasksse8_looppixel:
		mm_prefetch_esi_n(190)
		movq mm0, [esi + 0]
		movq mm1, [esi + 8]
		movq mm2, mm7
		movq mm3, mm7
		mm_prefetch_esi_n(128)
		pcmpeqb mm2, mm0
		pcmpeqb mm3, mm1
		add esi, 16
		pxor mm2, mm6
		pxor mm3, mm6
		mm_maskmovq_0_2
		add edi, edx
		mm_prefetch_esi_n(64)
		mm_maskmovq_1_3
		add edi, edx
		dec ecx
		jnz bmasksse8_looppixel
	bmasksse8_last:
		mov ecx, n
		test ecx, ecx
		jz bmasksse8_linend
		movd ebx, mm7
	bmasksse8_l1:
		lodsb
		cmp al, bl
		jz bmasksse8_l2
		mov [edi], al
	bmasksse8_l2:
		inc edi
		dec ecx
		jnz bmasksse8_l1
	bmasksse8_linend:
		add edi, c1
		add esi, c2
		mm_prefetch_esi_n(0)
		dec h
		jnz bmasksse8_loopline
	bmasksse16_check:
		mov eax, pb
		cmp al, 2
		jnz bmasksse32_check
	__ALIGNC__			/* bmasksse16_loopline */
	bmasksse16_loopline:	
		mov ecx, m
		test ecx, ecx
		jz bmasksse16_last
	__ALIGNC__
	bmasksse16_looppixel:
		mm_prefetch_esi_n(190)
		movq mm0, [esi + 0]
		movq mm1, [esi + 8]
		movq mm2, mm7
		movq mm3, mm7
		pcmpeqw mm2, mm0
		pcmpeqw mm3, mm1
		add esi, 16
		pxor mm2, mm6
		pxor mm3, mm6
		mm_maskmovq_0_2
		add edi, edx
		mm_prefetch_esi_n(64)
		mm_maskmovq_1_3
		add edi, edx
		dec ecx
		jnz bmasksse16_looppixel
	bmasksse16_last:
		mov ecx, n
		test ecx, ecx
		jz bmasksse16_linend
		movd ebx, mm7
	bmasksse16_l1:
		lodsw
		cmp ax, bx
		jz bmasksse16_l2
		mov [edi], ax
	bmasksse16_l2:
		add edi, 2
		dec ecx
		jnz bmasksse16_l1
	bmasksse16_linend:
		add edi, c1
		add esi, c2
		mm_prefetch_esi_n(0)
		dec h
		jnz bmasksse16_loopline
	bmasksse32_check:
		mov eax, pb
		cmp eax, 4
		jnz bmasksse24_check
	__ALIGNC__			/* bmasksse32_loopline */
	bmasksse32_loopline:	
		mov ecx, m
		test ecx, ecx
		jz bmasksse32_last
	__ALIGNC__
	bmasksse32_looppixel:
		mm_prefetch_esi_n(190)
		movq mm0, [esi + 0]
		movq mm1, [esi + 8]
		movq mm2, mm7
		movq mm3, mm7
		pcmpeqd mm2, mm0
		pcmpeqd mm3, mm1
		add esi, 16
		pxor mm2, mm6
		pxor mm3, mm6
		mm_maskmovq_0_2
		add edi, edx
		mm_prefetch_esi_n(64)
		mm_maskmovq_1_3
		add edi, edx
		dec ecx
		jnz bmasksse32_looppixel
	bmasksse32_last:
		mov ecx, n
		test ecx, ecx
		jz bmasksse32_linend
		movd ebx, mm7
	bmasksse32_l1:
		lodsd
		cmp eax, ebx
		jz bmasksse32_l2
		mov [edi], eax
	bmasksse32_l2:
		add edi, 4
		dec ecx
		jnz bmasksse32_l1
	bmasksse32_linend:
		add edi, c1
		add esi, c2
		mm_prefetch_esi_n(0)
		dec h
		jnz bmasksse32_loopline
	bmasksse24_check:
		mov eax, pb
		cmp eax, 3
		jnz bmasksse_endup
	__ALIGNC__		/* masksse24_loopline */
	bmasksse24_loopline:
		mov ecx, n
		test ecx, ecx
		jz bmasksse24_linend
		and ebx, 0xffffff
	__ALIGNC__
	bmasksse24_looppixel:
		lodsd
		dec esi
		and eax, 0xffffff
		mm_prefetch_esi_n(0)
		cmp eax, ebx
		jz bmasksse24_skip
		mov [edi], ax
		shr eax, 16
		mov [edi + 2], al
	bmasksse24_skip:
		add edi, 3
		dec ecx
		jnz bmasksse24_looppixel
	bmasksse24_linend:
		add edi, c1
		add esi, c2
		dec h
		jnz bmasksse24_loopline
	bmasksse_endup:
		mm_sfence
		emms
	}
#else
	return -1;
#endif
	return 0;
}


//---------------------------------------------------------------------
// iblit_mask_mix - mmx & sse mixed mask blitter 
// this routine is designed to support mmx mask blit
//---------------------------------------------------------------------
int iblit_mask_mix(char *dst, long pitch1, const char *src, int w, int h, 
		long pitch2, int pixelbyte, long linesize, unsigned long ckey)
{
	long linebytes = (long)linesize;
	long c1, c2, m, n, pb;
	unsigned long mask;

	switch (pixelbyte) {
	case 1:
		m = w >> 4;
		n = w & 15;
		mask = (ckey | (ckey << 8)) & 0xffff;
		mask = mask | (mask << 16);
		break;
	case 2: 
		linebytes = (((long)w) << 1); 
		m = w >> 3;
		n = w & 7;
		mask = ckey | (ckey << 16);
		break;
	case 3: 
		linebytes = (((long)w) << 1) + w; 
		m = 0;
		n = w;
		mask = ckey;
		break;
	case 4: 
		linebytes = (((long)w) << 2); 
		m = w >> 2;
		n = w & 3;
		mask = ckey;
		break;
	default: 
		linebytes = -1; 
		break;
	}

	if (linebytes < 0) return -1;

	c1 = pitch1 - linebytes;
	c2 = pitch2 - linebytes;
	pb = pixelbyte;

#if defined(__INLINEGNU__)
	__asm__ __volatile__ ("\n"
			ASM_BEGIN
		"	movl %1, %%esi\n"
		"	movl %2, %%edi\n"
		"	movd %3, %%mm7\n"
		"	movd %3, %%mm6\n"
		"	movl %3, %%ebx\n"
		"	psllq $32, %%mm7\n"
		"	por %%mm6, %%mm7\n"
		"	cld\n"
		"	movl %8, %%eax\n"
		"	cmpl $2, %%eax\n"
		"	jz bmaskmix16_loopline\n"
		"	cmpl $4, %%eax\n"
		"	jz bmaskmix32_loopline\n"
		"	cmpl $1, %%eax\n"
		"	jz bmaskmix8_loopline\n"
		"	jmp bmaskmix24_loopline\n"
		__ALIGNC__		/* bmaskmix8_loopline */
		"bmaskmix8_loopline:\n"
		"	movl %4, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmix8_last\n"
		__ALIGNC__
		"bmaskmix8_looppixel:\n"
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm4\n"
			mm_prefetch_esi_n(256)
			mm_prefetch_edi_n(256)
		"	movq %%mm0, %%mm1\n"
		"	movq %%mm4, %%mm5\n"
		"	pcmpeqb %%mm7, %%mm0\n"
		"	pcmpeqb %%mm7, %%mm4\n"
		"	addl $16, %%esi\n"
		"	addl $16, %%edi\n"
		"	movq %%mm0, %%mm2\n"
		"	movq %%mm4, %%mm6\n"
		"	pandn %%mm1, %%mm0\n"
		"	pandn %%mm5, %%mm4\n"
		"	pand -16(%%edi), %%mm2\n"
		"	pand -8(%%edi), %%mm6\n"
			mm_prefetch_esi_n(128)
			mm_prefetch_edi_n(128)
		"	por %%mm0, %%mm2\n"
		"	por %%mm4, %%mm6\n"
		"	movq %%mm2, -16(%%edi)\n"
		"	movq %%mm6, -8(%%edi)\n"
		"	decl %%ecx\n"
		"	jnz bmaskmix8_looppixel\n"
		"bmaskmix8_last:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmix8_linend\n"
		"	movd %%mm7, %%ebx\n"
		"bmaskmix8_l2:\n"
		"	lodsb\n"
		"	cmpb %%al, %%bl\n"
		"	jz bmaskmix8_l3\n"
		"	movb %%al, (%%edi)\n"
		"bmaskmix8_l3:\n"
		"	incl %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskmix8_l2\n"
		"bmaskmix8_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskmix8_loopline\n"
		"	jmp bmaskmix_endup\n"
		__ALIGNC__		/* bmaskmix16_loopline */
		"bmaskmix16_loopline:\n"
		"	movl %4, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmix16_last\n"
		__ALIGNC__
		"bmaskmix16_looppixel:\n"
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm4\n"
			mm_prefetch_esi_n(256)
			mm_prefetch_edi_n(256)
		"	movq %%mm0, %%mm1\n"
		"	movq %%mm4, %%mm5\n"
		"	pcmpeqw %%mm7, %%mm0\n"
		"	pcmpeqw %%mm7, %%mm4\n"
		"	addl $16, %%esi\n"
		"	addl $16, %%edi\n"
		"	movq %%mm0, %%mm2\n"
		"	movq %%mm4, %%mm6\n"
		"	pandn %%mm1, %%mm0\n"
		"	pandn %%mm5, %%mm4\n"
		"	pand -16(%%edi), %%mm2\n"
		"	pand -8(%%edi), %%mm6\n"
			mm_prefetch_esi_n(128)
			mm_prefetch_edi_n(128)
		"	por %%mm0, %%mm2\n"
		"	por %%mm4, %%mm6\n"
		"	movq %%mm2, -16(%%edi)\n"
		"	movq %%mm6, -8(%%edi)\n"
		"	decl %%ecx\n"
		"	jnz bmaskmix16_looppixel\n"
		"bmaskmix16_last:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmix16_linend\n"
		"	movd %%mm7, %%ebx\n"
		"bmaskmix16_l2:\n"
		"	lodsw\n"
		"	cmpw %%ax, %%bx\n"
		"	jz bmaskmix16_l3\n"
		"	movw %%ax, (%%edi)\n"
		"bmaskmix16_l3:\n"
		"	addl $2, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskmix16_l2\n"
		"bmaskmix16_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskmix16_loopline\n"
		"	jmp bmaskmix_endup\n"
		__ALIGNC__		/* bmaskmix32_loopline */
		"bmaskmix32_loopline:\n"
		"	movl %4, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmix32_last\n"
		__ALIGNC__
		"bmaskmix32_looppixel:\n"
		"	movq 0(%%esi), %%mm0\n"
		"	movq 8(%%esi), %%mm4\n"
			mm_prefetch_esi_n(256)
			mm_prefetch_edi_n(256)
		"	movq %%mm0, %%mm1\n"
		"	movq %%mm4, %%mm5\n"
		"	pcmpeqd %%mm7, %%mm0\n"
		"	pcmpeqd %%mm7, %%mm4\n"
		"	addl $16, %%esi\n"
		"	addl $16, %%edi\n"
		"	movq %%mm0, %%mm2\n"
		"	movq %%mm4, %%mm6\n"
		"	pandn %%mm1, %%mm0\n"
		"	pandn %%mm5, %%mm4\n"
		"	pand -16(%%edi), %%mm2\n"
		"	pand -8(%%edi), %%mm6\n"
			mm_prefetch_esi_n(128)
			mm_prefetch_edi_n(128)
		"	por %%mm0, %%mm2\n"
		"	por %%mm4, %%mm6\n"
		"	movq %%mm2, -16(%%edi)\n"
		"	movq %%mm6, -8(%%edi)\n"
		"	decl %%ecx\n"
		"	jnz bmaskmix32_looppixel\n"
		"bmaskmix32_last:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmix32_linend\n"
		"	movd %%mm7, %%ebx\n"
		"bmaskmix32_l2:\n"
		"	lodsl\n"
		"	cmpl %%eax, %%ebx\n"
		"	jz bmaskmix32_l3\n"
		"	movl %%eax, (%%edi)\n"
		"bmaskmix32_l3:\n"
		"	addl $4, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskmix32_l2\n"
		"bmaskmix32_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskmix32_loopline\n"
		"	jmp bmaskmix_endup\n"
		__ALIGNC__		/* bmaskmix24_loopline */
		"bmaskmix24_loopline:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskmix24_linend\n"
		"	andl $0xffffff, %%ebx\n"
		"bmaskmix24_looppixel:\n"
		"	movb 2(%%esi), %%al\n"
		"	movw 0(%%esi), %%dx\n"
		"	shll $16, %%eax\n"
		"	addl $3, %%esi\n"
		"	movw %%dx, %%ax\n"
			mm_prefetch_esi_n(64)
		"	andl $0xffffff, %%eax\n"
		"	cmpl %%ebx, %%eax\n"
		"	jz bmaskmix24_skip\n"
		"	movw %%ax, (%%edi)\n"
		"	shrl $16, %%eax\n"
		"	movb %%al, 2(%%edi)\n"
		"bmaskmix24_skip:\n"
		"	addl $3, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskmix24_looppixel\n"
		"bmaskmix24_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskmix24_loopline\n"
		"	jmp bmaskmix_endup\n"
		__ALIGNC__		/* bmaskmix_endup */
		"bmaskmix_endup:\n"
		"	emms\n"
			ASM_ENDUP
		:"=m"(h)
		:"m"(src),"m"(dst),"m"(mask),"m"(m),"m"(n),"m"(c1),"m"(c2),"m"(pb)
		:"memory", ASM_REGS);

#elif defined(__INLINEMSC__)
	_asm {
		mov esi, src
		mov edi, dst
		movd mm7, mask
		movd mm6, mask
		mov ebx, mask
		psllq mm7, 32
		por mm7, mm6
		cld
		mov edx, 16
		mov eax, pb
		cmp eax, 1
		jnz bmaskmix16_check
	__ALIGNC__		/* bmaskmix8_loopline */
	bmaskmix8_loopline:
		mov ecx, m
		test ecx, ecx
		jz bmaskmix8_last
	__ALIGNC__
	bmaskmix8_looppixel:
		movq mm0, [esi + 0]
		movq mm4, [esi + 8]
		mm_prefetch_esi_n(256)
		movq mm1, mm0
		movq mm5, mm4
		pcmpeqb mm0, mm7
		pcmpeqb mm4, mm7
		add esi, edx
		add edi, edx
		movq  mm2, mm0
		movq  mm6, mm4
		pandn mm0, mm1
		pandn mm4, mm5
		pand  mm2, [edi - 16]
		pand  mm6, [edi - 8]
		mm_prefetch_edi_n(256)
		por  mm2, mm0
		por  mm6, mm4
		movq [edi - 16], mm2
		movq [edi - 8], mm6
		dec ecx
		jnz bmaskmix8_looppixel
	bmaskmix8_last:
		mov ecx, n
		test ecx, ecx
		jz bmaskmix8_linend
	bmaskmix8_l2:
		lodsb
		cmp al, bl
		jz bmaskmix8_l3
		mov [edi], al
	bmaskmix8_l3:
		inc edi
		dec ecx
		jnz bmaskmix8_l2
	bmaskmix8_linend:
		add edi, c1
		add esi, c2
		dec h
		jnz bmaskmix8_loopline
	bmaskmix16_check:
		mov eax, pb
		cmp eax, 2
		jnz bmaskmix32_check
	__ALIGNC__		/* bmaskmix16_loopline */
	bmaskmix16_loopline:
		mov ecx, m
		test ecx, ecx
		jz bmaskmix16_last
	__ALIGNC__
	bmaskmix16_looppixel:
		movq mm0, [esi]
		movq mm4, [esi + 8]
		mm_prefetch_esi_8dx
		movq mm1, mm0
		movq mm5, mm4
		pcmpeqw mm0, mm7
		pcmpeqw mm4, mm7
		add esi, edx
		add edi, edx
		movq  mm2, mm0
		movq  mm6, mm4
		pandn mm0, mm1
		pandn mm4, mm5
		pand  mm2, [edi - 16]
		pand  mm6, [edi - 8]
		mm_prefetch_edi_8dx
		por  mm2, mm0
		por  mm6, mm4
		movq [edi - 16], mm2
		movq [edi - 8], mm6
		dec ecx
		jnz bmaskmix16_looppixel
	bmaskmix16_last:
		mov ecx, n
		test ecx, ecx
		jz bmaskmix16_linend
	bmaskmix16_l2:
		lodsw
		cmp ax, bx
		jz bmaskmix16_l3
		mov [edi], ax
	bmaskmix16_l3:
		add edi, 2
		dec ecx
		jnz bmaskmix16_l2
	bmaskmix16_linend:
		add edi, c1
		add esi, c2
		dec h
		jnz bmaskmix16_loopline
	bmaskmix32_check:
		mov eax, pb
		cmp eax, 4
		jnz bmaskmix24_check
	__ALIGNC__		/* bmaskmix32_loopline */
	bmaskmix32_loopline:
		mov ecx, m
		test ecx, ecx
		jz bmaskmix32_last
	__ALIGNC__
	bmaskmix32_looppixel:
		movq mm0, [esi + 0]
		movq mm4, [esi + 8]
		mm_prefetch_esi_8dx
		movq mm1, mm0
		movq mm5, mm4
		pcmpeqd mm0, mm7
		pcmpeqd mm4, mm7
		add esi, edx
		add edi, edx
		movq  mm2, mm0
		movq  mm6, mm4
		pandn mm0, mm1
		pandn mm4, mm5
		pand  mm2, [edi - 16]
		pand  mm6, [edi - 8]
		mm_prefetch_edi_8dx
		por  mm2, mm0
		por  mm6, mm4
		movq [edi - 16], mm2
		movq [edi - 8], mm6
		dec ecx
		jnz bmaskmix32_looppixel
	bmaskmix32_last:
		mov ecx, n
		test ecx, ecx
		jz bmaskmix32_linend
	bmaskmix32_l2:
		lodsd
		cmp eax, ebx
		jz bmaskmix32_l3
		mov [edi], eax
	bmaskmix32_l3:
		add edi, 4
		dec ecx
		jnz bmaskmix32_l2
	bmaskmix32_linend:
		add edi, c1
		add esi, c2
		dec h
		jnz bmaskmix32_loopline
		jmp bmaskmix_endup
	bmaskmix24_check:
		mov eax, pb
		cmp eax, 3
		jnz bmaskmix_endup
	__ALIGNC__		/* bmaskmix24_loopline */
	bmaskmix24_loopline:
		mov ecx, n
		test ecx, ecx
		jz bmaskmix24_linend
		and ebx, 0xffffff
	__ALIGNC__
	bmaskmix24_looppixel:
		lodsd
		dec esi
		and eax, 0xffffff
		cmp eax, ebx
		jz bmaskmix24_skip
		mov [edi], ax
		shr eax, 16
		mov [edi + 2], al
	__ALIGNC__
	bmaskmix24_skip:
		add edi, 3
		dec ecx
		jnz bmaskmix24_looppixel
	__ALIGNC__
	bmaskmix24_linend:
		add edi, c1
		add esi, c2
		dec h
		jnz bmaskmix24_loopline
	__ALIGNC__
	bmaskmix_endup:
		emms
	}
#else
	return -1;
#endif
	return 0;
}


//---------------------------------------------------------------------
// iblit_mask_xmm - mmx & sse mixed mask blitter 
// this routine is designed to support mmx mask blit
//---------------------------------------------------------------------
int iblit_mask_xmm(char *dst, long pitch1, const char *src, int w, int h, 
		long pitch2, int pixelbyte, long linesize, unsigned long ckey)
{
	long linebytes = (long)linesize;
	long c1, c2, m, n, pb;
	unsigned long mask;

	switch (pixelbyte) {
	case 1:
		m = w >> 5;
		n = w & 31;
		mask = (ckey | (ckey << 8)) & 0xffff;
		mask = mask | (mask << 16);
		break;
	case 2: 
		linebytes = (((long)w) << 1); 
		m = w >> 4;
		n = w & 15;
		mask = ckey | (ckey << 16);
		break;
	case 3:
		linebytes = (((long)w) << 1) + w; 
		m = 0;
		n = w;
		mask = ckey;
		break;
	case 4: 
		linebytes = (((long)w) << 2); 
		m = w >> 3;
		n = w & 7;
		mask = ckey;
		break;
	default: 
		linebytes = -1; 
		break;
	}

	if (linebytes < 0) return -1;

	c1 = pitch1 - linebytes;
	c2 = pitch2 - linebytes;
	pb = pixelbyte;

#if defined(__INLINEGNU__)
	__asm__ __volatile__ ("\n"
			ASM_BEGIN
		"	movl %1, %%esi\n"
		"	movl %2, %%edi\n"
		"	movd %3, %%mm7\n"
		"	movd %3, %%mm6\n"
		"	movl %3, %%ebx\n"
		"	psllq $32, %%mm7\n"
		"	por %%mm6, %%mm7\n"
			ASMCODE4(0xf3, 0x0f, 0xd6, 0xff)	// movq2dq %mm7, %xmm7
			ASMCODE4(0xf3, 0x0f, 0xd6, 0xf7)	// movq2dq %mm7, %xmm6
			ASMCODE5(0x66, 0x0f, 0x73, 0xff, 0x08)	// pslldq $8, %xmm7
			ASMCODE4(0x66, 0x0f, 0xeb, 0xfe)	// por %xmm6, %xmm7
		"	cld\n"
		"	movl %8, %%eax\n"
		"	cmpl $2, %%eax\n"
		"	jz bmaskxmm16_loopline\n"
		"	cmpl $4, %%eax\n"
		"	jz bmaskxmm32_loopline\n"
		"	cmpl $3, %%eax\n"
		"	jz bmaskxmm24_loopline\n"
		"	jmp bmaskxmm8_loopline\n"
		__ALIGNC__		/* bmaskxmm8_loopline */
		"bmaskxmm8_loopline:\n"
		"	movl %4, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskxmm8_last\n"
		__ALIGNC__
		"bmaskxmm8_looppixel:\n"
			ASMCODE4(0xf3, 0x0f, 0x6f, 0x06)	// movdqu xmm0, [esi]
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x66, 0x10)	// movdqu xmm4, [esi+16]
			mm_prefetch_esi_n(256)				
			ASMCODE4(0x66, 0x0f, 0x6f, 0xc8)	// movdqa xmm1, xmm0
			ASMCODE4(0x66, 0x0f, 0x6f, 0xec)	// movdqa xmm5, xmm4
			ASMCODE4(0x66, 0x0f, 0x74, 0xc7)	// pcmpeqb xmm0, xmm7
			ASMCODE4(0x66, 0x0f, 0x74, 0xe7)	// pcmpeqb xmm4, xmm7
		"	addl $32, %%esi\n"
		"	addl $32, %%edi\n"
			ASMCODE4(0x66, 0x0f, 0x6f, 0xd0)	// movdqa xmm2, xmm0
			ASMCODE4(0x66, 0x0f, 0x6f, 0xf4)	// movdqa xmm6, xmm4
			ASMCODE4(0x66, 0x0f, 0xdf, 0xc1)	// pandn xmm0, xmm1
			ASMCODE4(0x66, 0x0f, 0xdf, 0xe5)	// pandn xmm4, xmm5
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x4f, 0xe0)	// movdqu xmm1, [edi-32]
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x6f, 0xf0)	// movdqu xmm5, [edi-16]
			mm_prefetch_esi_n(128)	
			ASMCODE4(0x66, 0x0f, 0xdb, 0xd1)	// pand xmm2, xmm1
			ASMCODE4(0x66, 0x0f, 0xdb, 0xf5)	// pand xmm6, xmm5
			ASMCODE4(0x66, 0x0f, 0xeb, 0xd0)	// por xmm2, xmm0
			ASMCODE4(0x66, 0x0f, 0xeb, 0xf4)	// por xmm6, xmm4
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x57, 0xe0)	// movdqu [edi-32], xmm2
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x77, 0xf0)	// movdqu [edi-16], xmm6
		"	decl %%ecx\n"
		"	jnz bmaskxmm8_looppixel\n"
		"bmaskxmm8_last:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskxmm8_linend\n"
		"	movd %%mm7, %%ebx\n"
		__ALIGNC__
		"bmaskxmm8_l2:\n"
		"	lodsb\n"
		"	cmpb %%al, %%bl\n"
		"	jz bmaskxmm8_l3\n"
		"	movb %%al, (%%edi)\n"
		__ALIGNC__
		"bmaskxmm8_l3:\n"
		"	incl %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskxmm8_l2\n"
		__ALIGNC__
		"bmaskxmm8_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskxmm8_loopline\n"
		"	jmp bmaskxmm_endup\n"
		__ALIGNC__		/* bmaskxmm16_loopline */
		"bmaskxmm16_loopline:\n"
		"	movl %4, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskxmm16_last\n"
		__ALIGNC__
		"bmaskxmm16_looppixel:\n"
			ASMCODE4(0xf3, 0x0f, 0x6f, 0x06)	// movdqu xmm0, [esi]
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x66, 0x10)	// movdqu xmm4, [esi+16]
			mm_prefetch_esi_n(256)		
			ASMCODE4(0x66, 0x0f, 0x6f, 0xc8)	// movdqa xmm1, xmm0
			ASMCODE4(0x66, 0x0f, 0x6f, 0xec)	// movdqa xmm5, xmm4
			ASMCODE4(0x66, 0x0f, 0x75, 0xc7)	// pcmpeqw xmm0, xmm7
			ASMCODE4(0x66, 0x0f, 0x75, 0xe7)	// pcmpeqw xmm4, xmm7
		"	addl $32, %%esi\n"
		"	addl $32, %%edi\n"
			ASMCODE4(0x66, 0x0f, 0x6f, 0xd0)	// movdqa xmm2, xmm0
			ASMCODE4(0x66, 0x0f, 0x6f, 0xf4)	// movdqa xmm6, xmm4
			ASMCODE4(0x66, 0x0f, 0xdf, 0xc1)	// pandn xmm0, xmm1
			ASMCODE4(0x66, 0x0f, 0xdf, 0xe5)	// pandn xmm4, xmm5
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x4f, 0xe0)	// movdqu xmm1, [edi-32]
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x6f, 0xf0)	// movdqu xmm5, [edi-16]
			mm_prefetch_esi_n(128)	
			ASMCODE4(0x66, 0x0f, 0xdb, 0xd1)	// pand xmm2, xmm1
			ASMCODE4(0x66, 0x0f, 0xdb, 0xf5)	// pand xmm6, xmm5
			ASMCODE4(0x66, 0x0f, 0xeb, 0xd0)	// por xmm2, xmm0
			ASMCODE4(0x66, 0x0f, 0xeb, 0xf4)	// por xmm6, xmm4
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x57, 0xe0)	// movdqu [edi-32], xmm2
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x77, 0xf0)	// movdqu [edi-16], xmm6
		"	decl %%ecx\n"
		"	jnz bmaskxmm16_looppixel\n"
		"bmaskxmm16_last:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskxmm16_linend\n"
		"	movd %%mm7, %%ebx\n"
		__ALIGNC__
		"bmaskxmm16_l2:\n"
		"	lodsw\n"
		"	cmpw %%ax, %%bx\n"
		"	jz bmaskxmm16_l3\n"
		"	movw %%ax, (%%edi)\n"
		__ALIGNC__
		"bmaskxmm16_l3:\n"
		"	addl $2, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskxmm16_l2\n"
		__ALIGNC__
		"bmaskxmm16_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskxmm16_loopline\n"
		"	jmp bmaskxmm_endup\n"
		__ALIGNC__		/* bmaskxmm32_loopline */
		"bmaskxmm32_loopline:\n"
		"	movl %4, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskxmm32_last\n"
		__ALIGNC__
		"bmaskxmm32_looppixel:\n"
			ASMCODE4(0xf3, 0x0f, 0x6f, 0x06)
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x66, 0x10)	
			mm_prefetch_esi_n(256)				
			ASMCODE4(0x66, 0x0f, 0x6f, 0xc8)	
			ASMCODE4(0x66, 0x0f, 0x6f, 0xec)	// movdqa xmm5, xmm4
			ASMCODE4(0x66, 0x0f, 0x76, 0xc7)	// pcmpeqd xmm0, xmm7
			ASMCODE4(0x66, 0x0f, 0x76, 0xe7)	// pcmpeqd xmm4, xmm7
		"	addl $32, %%esi\n"
		"	addl $32, %%edi\n"
			ASMCODE4(0x66, 0x0f, 0x6f, 0xd0)	// movdqa xmm2, xmm0
			ASMCODE4(0x66, 0x0f, 0x6f, 0xf4)	// movdqa xmm6, xmm4
			ASMCODE4(0x66, 0x0f, 0xdf, 0xc1)	// pandn xmm0, xmm1
			ASMCODE4(0x66, 0x0f, 0xdf, 0xe5)	// pandn xmm4, xmm5
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x4f, 0xe0)	// movdqu xmm1, [edi-32]
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x6f, 0xf0)	// movdqu xmm5, [edi-16]
			mm_prefetch_esi_n(128)	
			ASMCODE4(0x66, 0x0f, 0xdb, 0xd1)	// pand xmm2, xmm1
			ASMCODE4(0x66, 0x0f, 0xdb, 0xf5)	// pand xmm6, xmm5
			ASMCODE4(0x66, 0x0f, 0xeb, 0xd0)	// por xmm2, xmm0
			ASMCODE4(0x66, 0x0f, 0xeb, 0xf4)	// por xmm6, xmm4
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x57, 0xe0)	// movdqu [edi-32], xmm2
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x77, 0xf0)	// movdqu [edi-16], xmm6
		"	decl %%ecx\n"
		"	jnz bmaskxmm32_looppixel\n"
		"bmaskxmm32_last:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskxmm32_linend\n"
		"	movd %%mm7, %%ebx\n"
		__ALIGNC__
		"bmaskxmm32_l2:\n"
		"	lodsl\n"
		"	cmpl %%eax, %%ebx\n"
		"	jz bmaskxmm32_l3\n"
		"	movl %%eax, (%%edi)\n"
		__ALIGNC__
		"bmaskxmm32_l3:\n"
		"	addl $4, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskxmm32_l2\n"
		__ALIGNC__
		"bmaskxmm32_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskxmm32_loopline\n"
		"	jmp bmaskxmm_endup\n"
		__ALIGNC__		/* bmaskxmm24_loopline */
		"bmaskxmm24_loopline:\n"
		"	movl %5, %%ecx\n"
		"	testl %%ecx, %%ecx\n"
		"	jz bmaskxmm24_linend\n"
		"	andl $0xffffff, %%ebx\n"
		"bmaskxmm24_looppixel:\n"
		"	movb 2(%%esi), %%al\n"
		"	movw 0(%%esi), %%dx\n"
		"	shll $16, %%eax\n"
		"	addl $3, %%esi\n"
		"	movw %%dx, %%ax\n"
			mm_prefetch_esi_n(64)
		"	andl $0xffffff, %%eax\n"
		"	cmpl %%ebx, %%eax\n"
		"	jz bmaskxmm24_skip\n"
		"	movw %%ax, (%%edi)\n"
		"	shrl $16, %%eax\n"
		"	movb %%al, 2(%%edi)\n"
		__ALIGNC__
		"bmaskxmm24_skip:\n"
		"	addl $3, %%edi\n"
		"	decl %%ecx\n"
		"	jnz bmaskxmm24_looppixel\n"
		__ALIGNC__
		"bmaskxmm24_linend:\n"
		"	addl %6, %%edi\n"
		"	addl %7, %%esi\n"
		"	decl %0\n"
		"	jnz bmaskxmm24_loopline\n"
		"	jmp bmaskxmm_endup\n"
		__ALIGNC__		/* bmaskxmm_endup */
		"bmaskxmm_endup:\n"
			mm_sfence
		"	emms\n"
			ASM_ENDUP
		:"=m"(h)
		:"m"(src),"m"(dst),"m"(mask),"m"(m),"m"(n),"m"(c1),"m"(c2),"m"(pb)
		:"memory", ASM_REGS);

#elif defined(__INLINEMSC__)
	if (pb == 1) {	// 8 bits pixel
		_asm {
			mov esi, src
			mov edi, dst
			movd mm7, mask
			movd mm6, mask
			mov ebx, mask
			psllq mm7, 32
			por mm7, mm6
			ASMCODE4(0xf3, 0x0f, 0xd6, 0xff)
			ASMCODE4(0xf3, 0x0f, 0xd6, 0xf7)
			ASMCODE5(0x66, 0x0f, 0x73, 0xff, 0x08)
			ASMCODE4(0x66, 0x0f, 0xeb, 0xfe)
			cld
		__ALIGNC__		/* bmaskxmm8_loopline */
		bmaskxmm8_loopline:
			mov ecx, m
			test ecx, ecx
			jz bmaskxmm8_last
		__ALIGNC__
		bmaskxmm8_looppixel:
			ASMCODE4(0xf3, 0x0f, 0x6f, 0x06)
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x66, 0x10)
			mm_prefetch_esi_n(256)				
			ASMCODE4(0x66, 0x0f, 0x6f, 0xc8)
			ASMCODE4(0x66, 0x0f, 0x6f, 0xec)
			ASMCODE4(0x66, 0x0f, 0x74, 0xc7)
			ASMCODE4(0x66, 0x0f, 0x74, 0xe7)
			add esi, 32	
			add edi, 32	
			ASMCODE4(0x66, 0x0f, 0x6f, 0xd0)
			ASMCODE4(0x66, 0x0f, 0x6f, 0xf4)
			ASMCODE4(0x66, 0x0f, 0xdf, 0xc1)
			ASMCODE4(0x66, 0x0f, 0xdf, 0xe5)	
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x4f, 0xe0)
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x6f, 0xf0)
			mm_prefetch_esi_n(128)
			ASMCODE4(0x66, 0x0f, 0xdb, 0xd1)
			ASMCODE4(0x66, 0x0f, 0xdb, 0xf5)
			ASMCODE4(0x66, 0x0f, 0xeb, 0xd0)
			ASMCODE4(0x66, 0x0f, 0xeb, 0xf4)
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x57, 0xe0)
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x77, 0xf0)
			dec ecx
			jnz bmaskxmm8_looppixel
		__ALIGNC__
		bmaskxmm8_last:
			mov ecx, n
			test ecx, ecx
			jz bmaskxmm8_linend
		bmaskxmm8_l2:
			lodsb
			cmp al, bl
			jz bmaskxmm8_l3
			mov [edi], al
		bmaskxmm8_l3:
			inc edi
			dec ecx
			jnz bmaskxmm8_l2
		__ALIGNC__
		bmaskxmm8_linend:
			add edi, c1
			add esi, c2
			dec h
			jnz bmaskxmm8_loopline
			mm_sfence
			emms
		}
	}	else
	if (pb == 2) {	// 16 bits pixel
		_asm {
			mov esi, src
			mov edi, dst
			movd mm7, mask
			movd mm6, mask
			mov ebx, mask
			psllq mm7, 32
			por mm7, mm6
			ASMCODE4(0xf3, 0x0f, 0xd6, 0xff)
			ASMCODE4(0xf3, 0x0f, 0xd6, 0xf7)
			ASMCODE5(0x66, 0x0f, 0x73, 0xff, 0x08)
			ASMCODE4(0x66, 0x0f, 0xeb, 0xfe)
			cld
		__ALIGNC__		/* bmaskxmm16_loopline */
		bmaskxmm16_loopline:
			mov ecx, m
			test ecx, ecx
			jz bmaskxmm16_last
		__ALIGNC__
		bmaskxmm16_looppixel:
			ASMCODE4(0xf3, 0x0f, 0x6f, 0x06)
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x66, 0x10)
			mm_prefetch_esi_n(256)				
			ASMCODE4(0x66, 0x0f, 0x6f, 0xc8)
			ASMCODE4(0x66, 0x0f, 0x6f, 0xec)
			ASMCODE4(0x66, 0x0f, 0x75, 0xc7)
			ASMCODE4(0x66, 0x0f, 0x75, 0xe7)
			add esi, 32	
			add edi, 32	
			ASMCODE4(0x66, 0x0f, 0x6f, 0xd0)
			ASMCODE4(0x66, 0x0f, 0x6f, 0xf4)
			ASMCODE4(0x66, 0x0f, 0xdf, 0xc1)
			ASMCODE4(0x66, 0x0f, 0xdf, 0xe5)
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x4f, 0xe0)
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x6f, 0xf0)
			mm_prefetch_esi_n(128)	
			ASMCODE4(0x66, 0x0f, 0xdb, 0xd1)
			ASMCODE4(0x66, 0x0f, 0xdb, 0xf5)
			ASMCODE4(0x66, 0x0f, 0xeb, 0xd0)
			ASMCODE4(0x66, 0x0f, 0xeb, 0xf4)
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x57, 0xe0)
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x77, 0xf0)
			dec ecx
			jnz bmaskxmm16_looppixel
		__ALIGNC__
		bmaskxmm16_last:
			mov ecx, n
			test ecx, ecx
			jz bmaskxmm16_linend
		bmaskxmm16_l2:
			lodsw
			cmp ax, bx
			jz bmaskxmm16_l3
			mov [edi], ax
		bmaskxmm16_l3:
			add edi, 2
			dec ecx
			jnz bmaskxmm16_l2
		__ALIGNC__
		bmaskxmm16_linend:
			add edi, c1
			add esi, c2
			dec h
			jnz bmaskxmm16_loopline
			mm_sfence
			emms
		}
	}	else
	if (pb == 4) {	// 32 bits pixel
		_asm {
			mov esi, src
			mov edi, dst
			movd mm7, mask
			movd mm6, mask
			mov ebx, mask
			psllq mm7, 32
			por mm7, mm6
			ASMCODE4(0xf3, 0x0f, 0xd6, 0xff)
			ASMCODE4(0xf3, 0x0f, 0xd6, 0xf7)
			ASMCODE5(0x66, 0x0f, 0x73, 0xff, 0x08)
			ASMCODE4(0x66, 0x0f, 0xeb, 0xfe)
			cld
		__ALIGNC__		/* bmaskxmm32_loopline */
		bmaskxmm32_loopline:
			mov ecx, m
			test ecx, ecx
			jz bmaskxmm32_last
		__ALIGNC__
		bmaskxmm32_looppixel:
			ASMCODE4(0xf3, 0x0f, 0x6f, 0x06)
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x66, 0x10)
			mm_prefetch_esi_n(256)				
			ASMCODE4(0x66, 0x0f, 0x6f, 0xc8)
			ASMCODE4(0x66, 0x0f, 0x6f, 0xec)
			ASMCODE4(0x66, 0x0f, 0x76, 0xc7)
			ASMCODE4(0x66, 0x0f, 0x76, 0xe7)
			add esi, 32
			add edi, 32
			ASMCODE4(0x66, 0x0f, 0x6f, 0xd0)
			ASMCODE4(0x66, 0x0f, 0x6f, 0xf4)
			ASMCODE4(0x66, 0x0f, 0xdf, 0xc1)
			ASMCODE4(0x66, 0x0f, 0xdf, 0xe5)
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x4f, 0xe0)
			ASMCODE5(0xf3, 0x0f, 0x6f, 0x6f, 0xf0)
			mm_prefetch_esi_n(128)	
			ASMCODE4(0x66, 0x0f, 0xdb, 0xd1)
			ASMCODE4(0x66, 0x0f, 0xdb, 0xf5)
			ASMCODE4(0x66, 0x0f, 0xeb, 0xd0)
			ASMCODE4(0x66, 0x0f, 0xeb, 0xf4)
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x57, 0xe0)
			ASMCODE5(0xf3, 0x0f, 0x7f, 0x77, 0xf0)
			dec ecx
			jnz bmaskxmm32_looppixel
		__ALIGNC__
		bmaskxmm32_last:
			mov ecx, n
			test ecx, ecx
			jz bmaskxmm32_linend
		bmaskxmm32_l2:
			lodsd
			cmp eax, ebx
			jz bmaskxmm32_l3
			mov [edi], eax
		bmaskxmm32_l3:
			add edi, 4
			dec ecx
			jnz bmaskxmm32_l2
		__ALIGNC__
		bmaskxmm32_linend:
			add edi, c1
			add esi, c2
			dec h
			jnz bmaskxmm32_loopline
			mm_sfence
			emms
		}
	}	else
	if (pb == 3) {	// 24 bits pixel
		_asm {
			mov esi, src
			mov edi, dst
		__ALIGNC__		/* bmaskmix24_loopline */
		bmaskxmm24_loopline:
			mov ecx, n
			test ecx, ecx
			jz bmaskxmm24_linend
			and ebx, 0xffffff
		__ALIGNC__
		bmaskxmm24_looppixel:
			mm_prefetch_esi_n(256)
			lodsd
			dec esi
			and eax, 0xffffff
			cmp eax, ebx
			jz bmaskxmm24_skip
			mov [edi], ax
			shr eax, 16
			mov [edi + 2], al
		__ALIGNC__
		bmaskxmm24_skip:
			add edi, 3
			dec ecx
			jnz bmaskxmm24_looppixel
		__ALIGNC__
		bmaskxmm24_linend:
			add edi, c1
			add esi, c2
			dec h
			jnz bmaskxmm24_loopline
			emms
		}
	}
#else
	return -1;
#endif
	return 0;
}


#endif

