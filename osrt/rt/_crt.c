/* rt/_crt.c -- CRI ABI */

/*
 * Copyright 2021 Mosakuji Hokuto <shikieiki@yamaxanadu.org>.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <osrt/process.h>
#include <osrt/syscall.h>

#include <stdint.h>

typedef void (*__crt_func_ptr_t)(void);

extern __crt_func_ptr_t __preinit_array_start[];
extern __crt_func_ptr_t __preinit_array_end[];
extern __crt_func_ptr_t __init_array_start[];
extern __crt_func_ptr_t __init_array_end[];
extern __crt_func_ptr_t __fini_array_start[];
extern __crt_func_ptr_t __fini_array_end[];

void
__crt_preinit(void)
{
	for (__crt_func_ptr_t *f = __preinit_array_start;
	     f != __preinit_array_end; ++f) {
		(*f)();
	}
}

void
__crt_init(void)
{
	for (__crt_func_ptr_t* f = __init_array_start;
	     f != __init_array_end; ++f) {
		(*f)();
	}
}

void
__crt_fini(void)
{
	for (__crt_func_ptr_t *f = __fini_array_start;
	     f != __fini_array_end; ++f) {
		(*f)();
	}
}

__attribute__((noreturn)) void
__exit(int64_t retval)
{
	__crt_fini();
        syscall_process_exit(retval);
}

int main(int argc, char **argv);

void
__main_wrapper(void)
{
        int ret = main(__INIT_DATA->argc, __INIT_DATA->argv);
        __exit(ret);
}
