/* osrt/cdefs.h -- Miscellaneous definitions */

/*
 * Copyright 2022 Tenhouin Youkou <youkou@tenhou.in>.
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

#ifndef __RENZAN_OSRT_CDEFS_H__
#define __RENZAN_OSRT_CDEFS_H__

/*
 * Names should be prefixed with __OSRT_ outside kernel and osrt
 * to avoid namespace pollution into user programs
 */
#if defined(__RZ_KERNEL) || defined(__RZ_OSRT)

#define __OSRT_PF_MNAME(name) name
#define __OSRT_PF_TNAME(name) name

#else

#define __OSRT_PF_MNAME(name) __OSRT_##name
#define __OSRT_PF_TNAME(name) __osrt_##name

#endif /* __RZ_KERNEL || __RZ_OSRT */

#endif /* __RENZAN_OSRT_CDEFS_H__ */
