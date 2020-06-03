#include <kern/simd.h>
#include <kern/types.h>
#include <kern/boolean.h>
#include <kern/asm.h>
#include <kern/console.h>
#include <kern/macrodef.h>
#include <kern/memory.h>

static boolean
have_sse2(u32 cpuid_edx)
{
	return !!(cpuid_edx & (1 << 26));
}

static boolean
have_sse3(u32 cpuid_ecx)
{
	return !!(cpuid_ecx & 1);
}

/* notice: this is not for SSE3 */
static boolean
have_ssse3(u32 cpuid_ecx)
{
	return !!(cpuid_ecx & (1 << 9));
}

static boolean
have_sse41(u32 cpuid_ecx)
{
	return !!(cpuid_ecx & (1 << 19));
}

static boolean
have_sse42(u32 cpuid_ecx)
{
	return !!(cpuid_ecx & (1 << 20));
}

static boolean
have_sse4a(u32 cpuid_ecx)
{
	return !!(cpuid_ecx & (1 << 6));
}

static boolean
have_xop(u32 cpuid_ecx)
{
	return !!(cpuid_ecx & (1 << 11));
}

static boolean
have_fma4(u32 cpuid_ecx)
{
	return !!(cpuid_ecx & (1 << 16));
}

static boolean
have_cvt16(u32 cpuid_ecx)
{
	return !!(cpuid_ecx & (1 << 29));
}

static boolean
have_avx(u32 cpuid_ecx)
{
	return !!(cpuid_ecx & (1 << 28));
}

static boolean
have_avx512(cpuid_eax_0d)
{
	return !!(cpuid_eax_0d & (7 << 5)); /* check bits 5-7 */
}

static boolean
have_xsave(u32 cpuid_ecx)
{
	return !!(cpuid_ecx & (1 << 26));
}

static boolean use_avx;
static boolean use_avx512;
static boolean use_xsave;

static size_t state_size;
static size_t state_alignment;

static void
check_simd(void)
{
	kprintf("Checking for SIMD features...");

	u32 eax, ebx, ecx, edx;
	cpuid(1, &eax, &ebx, &ecx, &edx);

	use_avx	= have_avx(ecx);
	use_xsave	= have_xsave(ecx);

	kprintf(have_sse2(edx) ? "SSE2 " : "sse2 ");
	VERIFY(have_sse2(edx), "All AMD64 enabled cpu should have SSE2");

	kprintf(have_sse3(ecx) ? "SSE3 " : "sse3 ");
	kprintf(have_ssse3(ecx) ? "SSSE3 " : "ssse3 ");
	kprintf(have_sse41(ecx) ? "SSE4.1 " : "sse4.1 ");
	kprintf(have_sse42(ecx) ? "SSE4.2 " : "sse4.2 ");
	kprintf(have_sse4a(ecx) ? "SSE4A " : "sse4a ");
	kprintf(have_xop(ecx) ? "XOP " : "xop ");
	kprintf(have_cvt16(ecx) ? "CVT16 " : "cvt16 ");
	kprintf(have_fma4(ecx) ? "FMA4 " : "fma4 ");

	kprintf(use_avx ? "AVX " : "avx ");
	kprintf(use_xsave ? "XSAVE " : "xsave ");

	cpuid(0x0d, &eax, &ebx, &ecx, &edx);
	use_avx512	= have_avx512(eax);

	kprintf(use_avx512 ? "AVX512 " : "avx512 ");
	kprintf("\n");

	if (use_xsave) {
		/* for xsave/xrstor */
		state_size	= ecx;
		state_alignment	= 64;
	} else {
		/* for fxsave/fxrstor */
		state_size	= 512;
		state_alignment	= 16;
	}
}

void
simd_init(void)
{
	check_simd();

	kprintf("SIMD: Enabling SSE\n");
	u64 cr_val;

	/* clear CR0.EM; set CR0.MP */
	asm volatile ("movq %%cr0, %0": "=r"(cr_val));
	cr_val	&= 0xfffffffb;
	cr_val	|= 0x2;
	asm volatile ("movq %0, %%cr0" :: "r"(cr_val));

	/* set CR4.OSFXSR and CR4.OSXMMEXCPT */
	asm volatile ("movq %%cr4, %0" : "=r"(cr_val));
	cr_val	|= 3 << 9;
	asm volatile ("movq %%cr4, %0" :: "r"(cr_val));

	if (!use_avx) {
		return;
	}

	kprintf("SIMD: Enabling AVX\n");

	u64 xcr0;
	asm volatile(
		"xorq	%%rcx, %%rcx\n"
		"xgetbv\n"
		: "=a"(xcr0) :: "rcx");

	/* AVX, SSE, X87 */
	xcr0	|=	7;

	if (use_avx512) {
		kprintf("SIMD: Enabling AVX512\n");
		/* OPMASK, ZMM_Hi256, Hi16_ZMM */
		xcr0	|= 7 << 5;
	}
	asm volatile ("xsetbv" :: "a"(xcr0));
}

void *
simd_alloc_state_area(void)
{
	return kmem_alloc_aligned(state_size, state_alignment);
}

void
simd_free_state_area(void *state)
{
	return kmem_free(state);
}

void
simd_save_state(void *state)
{
	if (use_xsave) {
		asm volatile ("xsave (%0)" :: "r" (state) : "memory");
	} else {
		asm volatile ("fxsave (%0)" :: "r" (state) : "memory");
	}
}

void
simd_load_state(void *state)
{
	if (use_xsave) {
		asm volatile ("xrstor (%0)" :: "r" (state) : "memory");
	} else {
		asm volatile ("fxrstor (%0)" :: "r" (state) : "memory");
	}
}
