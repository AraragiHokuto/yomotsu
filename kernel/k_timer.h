/* k_timer.h -- Timer implementation */
/* XXX part of this should move to HAL/Ig */

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

#ifndef __RENZAN_K_TIMER_H__
#define __RENZAN_K_TIMER_H__

#include <osrt/types.h>

typedef u64 timer_deadline_t;  /* deadline in ms */
typedef u64 timer_uduration_t; /* duration in ms */

typedef void (*timer_callback_t)(void *data);

struct timer_heap_element_s {
        timer_deadline_t deadline;

        timer_callback_t callback;
        void *           data;
};

typedef struct timer_heap_element_s timer_heap_element_t;

void timer_init_bsp(void);
void timer_init_ap(void);

void timer_spin_wait(timer_uduration_t);
void timer_set_timeout(
    timer_uduration_t timeout, timer_callback_t callback, void *data);
u64 timer_get_timestamp(void);

#endif /* __RENZAN_K_TIMER_H__ */
