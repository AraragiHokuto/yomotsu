/* k_timer.c -- Timer implementation */
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

#include <hal_percpu.h>
#include <k_asm.h>
#include <k_cdefs.h>
#include <k_console.h>
#include <k_int.h>
#include <k_memory.h>
#include <k_string.h>
#include <k_timer.h>

#define HEAP_INIT_SIZE 32

/* timeout heap */
static void
timer_heap_init(void)
{
        ASSERT(!percpu()->timer_heap);
        ASSERT(percpu()->timer_heap_size == 0);

        percpu()->timer_heap = kmem_alloc(sizeof(timer_heap_element_t) * 64);
        percpu()->timer_heap_end  = percpu()->timer_heap + 1;
        percpu()->timer_heap_size = HEAP_INIT_SIZE;

        percpu()->timer_heap->deadline = 0;
        percpu()->timer_heap->callback = NULL;
        percpu()->timer_heap->data     = NULL;
}

static void
timer_heap_expand(void)
{
        ASSERT(percpu()->timer_heap);
        ASSERT(percpu()->timer_heap_size != 0);

        size_t                new_size = percpu()->timer_heap_size * 2;
        timer_heap_element_t *new_heap = kmem_realloc(
            percpu()->timer_heap, new_size * sizeof(timer_heap_element_t));

        if (!new_heap) {
                /* XXX: how to handle OOM here? */
                PANIC("OOM in timer_heap_expand()");
        }

        size_t occupied = percpu()->timer_heap_end - percpu()->timer_heap;

        percpu()->timer_heap      = new_heap;
        percpu()->timer_heap_end  = new_heap + occupied;
        percpu()->timer_heap_size = new_size;
}

static boolean
timer_heap_empty(void)
{
        return percpu()->timer_heap_end - percpu()->timer_heap == 1;
}

static timer_heap_element_t *
timer_heap_top(void)
{
        return &percpu()->timer_heap[1];
}

static timer_heap_element_t *
timer_heap_parent(timer_heap_element_t *self)
{
        timer_heap_element_t *root = percpu()->timer_heap;
        u64                   i    = self - root;
        return &root[i / 2];
}

static timer_heap_element_t *
timer_heap_left_child(timer_heap_element_t *self)
{
        timer_heap_element_t *root = percpu()->timer_heap;
        u64                   i    = self - root;
        timer_heap_element_t *ret  = &root[2 * i];
        return ret >= percpu()->timer_heap_end ? NULL : ret;
}

static timer_heap_element_t *
timer_heap_right_child(timer_heap_element_t *self)
{
        timer_heap_element_t *root = percpu()->timer_heap;
        u64                   i    = self - root;
        timer_heap_element_t *ret  = &root[2 * i + 1];
        return ret >= percpu()->timer_heap_end ? NULL : ret;
}

static void
timer_heap_insert(
    timer_deadline_t deadline, timer_callback_t callback, void *data)
{
        if (percpu()->timer_heap_end - percpu()->timer_heap
            >= percpu()->timer_heap_size)
                timer_heap_expand();

        ASSERT(
            percpu()->timer_heap_end - percpu()->timer_heap
            < percpu()->timer_heap_size);

        timer_heap_element_t *ptr = percpu()->timer_heap_end++;
        ptr->deadline             = deadline;
        ptr->callback             = callback;
        ptr->data                 = data;

        while (timer_heap_parent(ptr)->deadline > ptr->deadline) {
                timer_heap_element_t  tmp;
                timer_heap_element_t *parent = timer_heap_parent(ptr);
                kmemcpy(&tmp, ptr, sizeof(tmp));
                kmemcpy(ptr, parent, sizeof(tmp));
                kmemcpy(parent, &tmp, sizeof(tmp));

                ptr = parent;
        }
}

static void
timer_heap_extract(void)
{
        --percpu()->timer_heap_end;
        if (timer_heap_top() == percpu()->timer_heap_end) { return; }

        kmemcpy(
            timer_heap_top(), percpu()->timer_heap_end,
            sizeof(timer_heap_element_t));

        timer_heap_element_t *ptr = timer_heap_top();

        while (B_TRUE) {
                timer_heap_element_t *left  = timer_heap_left_child(ptr);
                timer_heap_element_t *right = timer_heap_right_child(ptr);

                if ((!left || ptr->deadline < left->deadline)
                    && (!right || ptr->deadline < right->deadline)) {
                        break;
                }

                timer_heap_element_t tmp;
                if (left && left->deadline < right->deadline) {
                        kmemcpy(&tmp, left, sizeof(tmp));
                        kmemcpy(left, ptr, sizeof(tmp));
                        kmemcpy(ptr, &tmp, sizeof(tmp));

                        ptr = left;
                } else if (right) {
                        kmemcpy(&tmp, right, sizeof(tmp));
                        kmemcpy(right, ptr, sizeof(tmp));
                        kmemcpy(ptr, &tmp, sizeof(tmp));

                        ptr = right;
                }
        }
}

static void
timer_exec_callbacks(void)
{
        while (1) {
                if (timer_heap_empty()) { return; }

                if (timer_heap_top()->deadline > timer_get_timestamp()) {
                        return;
                }

                timer_callback_t callback = timer_heap_top()->callback;
                void *           data     = timer_heap_top()->data;

                timer_heap_extract();

                callback(data);
        }
}

static void
timer_irq_handler(u64 vector)
{
        DONTCARE(vector);

        if (!timer_heap_empty()) { timer_exec_callbacks(); }
}

static volatile boolean pit_flag = B_TRUE;

static void
pit_handler(u64 vector)
{
        ASSERT(vector == 32);
        pit_flag = B_FALSE;
}

static u64
__do_rdtscp(void)
{
        u64 l, h;
        asm volatile("rdtscp" : "=a"(l), "=d"(h)::"rcx");
        return l | (h << 32);
}

static u64 apic_timer_count = 0;
static u64 tsc_count        = 0;

static void
timer_calibrate(void)
{
        asm volatile("cli");

        /* set divider to 16 */
        apic_write_reg(APIC_REG_TIMER_DIV_CFG, 0x3);

        /* unmask PIT */
        outb(0x21, 0xfe);

        /* prepare PIT for 10ms wait */
        interrupt_set_handler(32, pit_handler);

        outb(0x43, 48);

        apic_write_reg(APIC_REG_TIMER_INIT_CNT, 0xFFFFFFFF);
        u64 tsc_begin = __do_rdtscp();

        /* 0x2E9C -- 11932 ticks, or 10ms */
        outb(0x40, 0x9c);
        outb(0x40, 0x2e);

        asm volatile("sti");

        /* wait until pic fire */
        while (pit_flag == B_TRUE) { asm volatile("pause"); }

        /* mask apic timer */
        asm volatile("cli");
        apic_write_reg(APIC_REG_TIMER_LVT, 1 << 16);

        interrupt_clear_handler(32);

        /* mask pit again */
        outb(0x21, 0xff);
        asm volatile("sti");

        /* read and return value */
        u64 tsc_end = __do_rdtscp();
        u32 val     = apic_read_reg(APIC_REG_TIMER_CURR_CNT);

        apic_timer_count = 0xffffffff - val;
        tsc_count        = tsc_end - tsc_begin;
}

static void
init_apic_timer(void)
{
        /* set divider to 16 */
        apic_write_reg(APIC_REG_TIMER_DIV_CFG, 0x3);

        /* XXX: hardcoded to 1ms */
        u32 cnt = apic_timer_count / 10;

        /* start receiving irq at vector 48 */
        apic_write_reg(APIC_REG_TIMER_INIT_CNT, cnt);
        apic_write_reg(APIC_REG_TIMER_LVT, 48 | 0x20000);
}

void
timer_init_bsp(void)
{
        timer_heap_init();

        timer_calibrate();

        interrupt_set_handler(48, timer_irq_handler);
        init_apic_timer();
}

void
timer_init_ap(void)
{
        timer_heap_init();
        init_apic_timer();
        interrupt_set_handler(48, timer_irq_handler);
}

void
timer_spin_wait(timer_uduration_t wait_ms)
{
        interrupt_enable_preemption();

        u64 deadline = timer_get_timestamp() + wait_ms;

        while (timer_get_timestamp() < deadline) { asm volatile("pause"); }
}

void
timer_set_timeout(
    timer_uduration_t timeout, timer_callback_t callback, void *data)
{
        timer_heap_insert(timer_get_timestamp() + timeout, callback, data);
}

u64
timer_get_timestamp(void)
{
        return __do_rdtscp() / tsc_count * 10;
}
