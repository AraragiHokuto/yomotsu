/* k_list.h -- Linked-list implementation */
/* XXX should move to OSRT */

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

#ifndef __RENZAN_K_LIST_H__
#define __RENZAN_K_LIST_H__

#include <osrt/types.h>

struct list_node_s {
        struct list_node_s *prev, *next;
};

typedef struct list_node_s list_node_t;

static inline void
list_head_init(list_node_t *node)
{
        node->prev = node;
        node->next = node;
}

static inline void
list_insert(list_node_t *node, list_node_t *prev)
{
        list_node_t *next = prev->next;
        node->next        = next;
        node->prev        = prev;

        prev->next = node;
        next->prev = node;
}

static inline void
list_remove(list_node_t *node)
{
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->prev = node->next = node;
}

static inline boolean
list_is_empty(list_node_t *head)
{
        return head->next == head;
}

#define OFFSET_OF(__type, __member) __builtin_offsetof(__type, __member)

#define CONTAINER_OF(__ptr, __type, __member) \
        ((__type *)((byte *)(__ptr)-OFFSET_OF(__type, __member)))

#define LIST_FOREACH(__head, __ptr)                              \
        for (list_node_t *__ptr = __head.next; __ptr != &__head; \
             __ptr              = __ptr->next)

#define LIST_FOREACH_MUT(__head, __ptr, __nextptr)                         \
        for (list_node_t *__ptr = (__head).next, *__nextptr = __ptr->next; \
             __ptr != &(__head);                                           \
             __ptr = __nextptr, __nextptr = __nextptr->next)

#endif /* __RENZAN_LIST_H__ */
