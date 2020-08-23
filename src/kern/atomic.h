#ifndef IZANAMI_ATOMIC_H_
#define IZANAMI_ATOMIC_H_

#include <kern/boolean.h>
#include <kern/types.h>

#ifdef _KERNEL

        /* memory orders */
        #define MEMORY_ORDER_RELAXED __ATOMIC_RELAXED
        #define MEMORY_ORDER_CONSUME __ATOMIC_CONSUME
        #define MEMORY_ORDER_RELEASE __ATOMIC_RELEASE
        #define MEMORY_ORDER_ACQUIRE __ATOMIC_ACQUIRE
        #define MEMORY_ORDER_ACQ_REL __ATOMIC_ACQ_REL
        #define MEMORY_ORDER_SEQ_CST __ATOMIC_SEQ_CST

typedef sint    atomic_sint;
typedef uint    atomic_uint;
typedef boolean atomic_boolean;

typedef u8 atomic_u8;

typedef void *atomic_ptr;

        #define __ATOMIC_LOAD_IMPL(dst, order) __atomic_load_n(&dst, order)
        #define __ATOMIC_STORE_IMPL(dst, val, order) \
                __atomic_store_n(&dst, val, order)
        #define __ATOMIC_XCHG_IMPL(dst, val, order) \
                __atomic_exchange_n(&dst, val, order)
        #define __ATOMIC_ADD_FETCH_IMPL(dst, val, order) \
                __atomic_add_fetch(&dst, val, order)
        #define __ATOMIC_SUB_FETCH_IMPL(dst, val, order) \
                __atomic_sub_fetch(&dst, val, order)
        #define __ATOMIC_FETCH_ADD_IMPL(dst, val, order) \
                __atomic_fetch_add(&dst, val, order)
        #define __ATOMIC_FETCH_SUB_IMPL(dst, val, order) \
                __atomic_fetch_sub(&dst, val, order)
        #define __ATOMIC_INC_FETCH_IMPL(dst, order) \
                __atomic_add_fetch(&dst, 1, order)
        #define __ATOMIC_DEC_FETCH_IMPL(dst, order) \
                __atomic_sub_fetch(&dst, 1, order)
        #define __ATOMIC_FETCH_INC_IMPL(dst, order) \
                __atomic_fetch_add(&dst, 1, order)
        #define __ATOMIC_FETCH_DEC_IMPL(dst, order) \
                __atomic_fetch_dec(&dst, 1, order)
        #define __ATOMIC_CMPXCHG_WEAK_IMPL(                          \
            dst, expected, desired, order_success, order_failed)     \
                __atomic_compare_exchange_n(                         \
                    &dst, &expected, desired, B_TRUE, order_success, \
                    order_failed)
        #define __ATOMIC_CMPXCHG_STRONG_IMPL(                         \
            dst, expected, desired, order_success, order_failed)      \
                __atomic_compare_exchange_n(                          \
                    &dst, &expected, desired, B_FALSE, order_success, \
                    order_failed)

        #define atomic_load_sint(dst, order) __ATOMIC_LOAD_IMPL(val, order)
        #define atomic_store_sint(dst, val, order) \
                __ATOMIC_STORE_IMPL(dst, val, order)
        #define atomic_xchg_sint(dst, val, order) \
                __ATOMIC_XCHG_IMPL(dst, val, order)
        #define atomic_add_fetch_sint(dst, val, order) \
                __ATOMIC_ADD_FETCH_IMPL(dst, val, order)
        #define atomic_sub_fetch_sint(dst, val, order) \
                __ATOMIC_SUB_FETCH_IMPL(dst, val, order)
        #define atomic_fetch_add_sint(dst, val, order) \
                __ATOMIC_FETCH_ADD_IMPL(dst, val, order)
        #define atomic_fetch_sub_sint(dst, val, order) \
                __ATOMIC_FETCH_SUB_IMPL(dst, val, order)
        #define atomic_inc_fetch_sint(dst, order) \
                __ATOMIC_INC_FETCH_IMPL(dst, order)
        #define atomic_dec_fetch_sint(dst, order) \
                __ATOMIC_DEC_FETCH_IMPL(dst, order)
        #define atomic_fetch_inc_sint(dst, order) \
                __ATOMIC_FETCH_INC_IMPL(dst, order)
        #define atomic_fetch_dec_sint(dst, order) \
                __ATOMIC_FETCH_DEC_IMPL(dst, order)
        #define atomic_cmpxchg_weak_sint(                        \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_WEAK_IMPL(                      \
                    dst, expected, desired, order_success, order_failed)
        #define atomic_cmpxchg_strong_sint(                      \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_STRONG_IMPL(                    \
                    dst, expected, desired, order_success, order_failed)

        #define atomic_load_uint(dst, order) __ATOMIC_LOAD_IMPL(dst, order)
        #define atomic_store_uint(dst, val, order) \
                __ATOMIC_STORE_IMPL(dst, val, order)
        #define atomic_xchg_uint(dst, val, order) \
                __ATOMIC_XCHG_IMPL(dst, val, order)
        #define atomic_add_fetch_uint(dst, val, order) \
                __ATOMIC_ADD_FETCH_IMPL(dst, val, order)
        #define atomic_sub_fetch_uint(dst, val, order) \
                __ATOMIC_SUB_FETCH_IMPL(dst, val, order)
        #define atomic_fetch_add_uint(dst, val, order) \
                __ATOMIC_FETCH_ADD_IMPL(dst, val, order)
        #define atomic_fetch_sub_uint(dst, val, order) \
                __ATOMIC_FETCH_SUB_IMPL(dst, val, order)
        #define atomic_inc_fetch_uint(dst, order) \
                __ATOMIC_INC_FETCH_IMPL(dst, order)
        #define atomic_dec_fetch_uint(dst, order) \
                __ATOMIC_DEC_FETCH_IMPL(dst, order)
        #define atomic_fetch_inc_uint(dst, order) \
                __ATOMIC_FETCH_INC_IMPL(dst, order)
        #define atomic_fetch_dec_uint(dst, order) \
                __ATOMIC_FETCH_DEC_IMPL(dst, order)
        #define atomic_cmpxchg_weak_uint(                        \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_WEAK_IMPL(                      \
                    dst, expected, desired, order_success, order_failed)
        #define atomic_cmpxchg_strong_uint(                      \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_STRONG_IMPL(                    \
                    dst, expected, desired, order_success, order_failed)

        #define atomic_load_boolean(dst, order) __ATOMIC_LOAD_IMPL(dst, order)
        #define atomic_store_boolean(dst, val, order) \
                __ATOMIC_STORE_IMPL(dst, val, order)
        #define atomic_xchg_boolean(dst, val, order) \
                __ATOMIC_XCHG_IMPL(dst, val, order)
        #define atomic_cmpxchg_weak_boolean(                     \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_WEAK_IMPL(                      \
                    dst, expected, desired, order_success, order_failed)
        #define atomic_cmpxchg_strong_boolean(                   \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_STRONG_IMPL(                    \
                    dst, expected, desired, order_success, order_failed)

        #define atomic_load_u8(dst, order) __ATOMIC_LOAD_IMPL(dst, order)
        #define atomic_store_u8(dst, val, order) \
                __ATOMIC_STORE_IMPL(dst, val, order)
        #define atomic_xchg_u8(dst, val, order) \
                __ATOMIC_XCHG_UINT(dst, val, order)
        #define atomic_add_fetch_u8(dst, val, order) \
                __ATOMIC_ADD_FETCH_IMPL(dst, val, order)
        #define atomic_sub_fetch_u8(dst, val, order) \
                __ATOMIC_SUB_FETCH_IMPL(dst, val, order)
        #define atomic_fetch_add_u8(dst, val, order) \
                __ATOMIC_FETCH_ADD_IMPL(dst, val, order)
        #define atomic_fetch_sub_u8(dst, val, order) \
                __ATOMIC_FETCH_SUB_IMPL(dst, val, order)
        #define atomic_inc_fetch_u8(dst, order) \
                __ATOMIC_INC_FETCH_IMPL(dst, order)
        #define atomic_dec_fetch_u8(dst, order) \
                __ATOMIC_DEC_FETCH_IMPL(dst, order)
        #define atomic_fetch_inc_u8(dst, order) \
                __ATOMIC_FETCH_INC_IMPL(dst, order)
        #define atomic_fetch_dec_u8(dst, order) \
                __ATOMIC_FETCH_DEC_IMPL(dst, order)
        #define atomic_cmpxchg_weak_u8(                          \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_WEAK_IMPL(                      \
                    dst, expected, desired, order_success, order_failed)
        #define atomic_cmpxchg_strong_u8(                        \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_STRONG_IMPL(                    \
                    dst, expected, desired, order_success, order_failed)

        #define atomic_load_u64(dst, order) __ATOMIC_LOAD_IMPL(dst, order)
        #define atomic_store_u64(dst, val, order) \
                __ATOMIC_STORE_IMPL(dst, val, order)
        #define atomic_xchg_u64(dst, val, order) \
                __ATOMIC_XCHG_UINT(dst, val, order)
        #define atomic_add_fetch_u64(dst, val, order) \
                __ATOMIC_ADD_FETCH_IMPL(dst, val, order)
        #define atomic_sub_fetch_u64(dst, val, order) \
                __ATOMIC_SUB_FETCH_IMPL(dst, val, order)
        #define atomic_fetch_add_u64(dst, val, order) \
                __ATOMIC_FETCH_ADD_IMPL(dst, val, order)
        #define atomic_fetch_sub_u64(dst, val, order) \
                __ATOMIC_FETCH_SUB_IMPL(dst, val, order)
        #define atomic_inc_fetch_u64(dst, order) \
                __ATOMIC_INC_FETCH_IMPL(dst, order)
        #define atomic_dec_fetch_u64(dst, order) \
                __ATOMIC_DEC_FETCH_IMPL(dst, order)
        #define atomic_fetch_inc_u64(dst, order) \
                __ATOMIC_FETCH_INC_IMPL(dst, order)
        #define atomic_fetch_dec_u64(dst, order) \
                __ATOMIC_FETCH_DEC_IMPL(dst, order)
        #define atomic_cmpxchg_weak_u64(                         \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_WEAK_IMPL(                      \
                    dst, expected, desired, order_success, order_failed)
        #define atomic_cmpxchg_strong_u64(                       \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_STRONG_IMPL(                    \
                    dst, expected, desired, order_success, order_failed)

        #define atomic_load_ptr(dst, order) __ATOMIC_LOAD_IMPL(dst, order)
        #define atomic_store_ptr(dst, val, order) \
                __ATOMIC_STORE_IMPL(dst, val, order)
        #define atomic_xchg_ptr(dst, val, order) \
                __ATOMIC_XCHG_IMPL(dst, val, order)
        #define atomic_cmpxchg_weak_ptr(                         \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_WEAK_IMPL(                      \
                    dst, expected, desired, order_success, order_failed)
        #define atomic_cmpxchg_strong_ptr(                       \
            dst, expected, desired, order_success, order_failed) \
                __ATOMIC_CMPXCHG_STRONG_IMPL(                    \
                    dst, expected, desired, order_success, order_failed)

#endif /* _KERNEL */

#endif /* IZANAMI_ATOMIC_H_ */
