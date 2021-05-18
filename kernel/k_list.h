#ifndef RENZAN_LIST_H__
#define RENZAN_LIST_H__




#ifdef _KERNEL

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

#endif /* _KERNEL */

#endif /* RENZAN_LIST_H__ */
