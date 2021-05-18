#ifndef RENZAN_SIMD_H__
#define RENZAN_SIMD_H__

#ifdef _KERNEL

void  simd_init(void);
void *simd_alloc_state_area(void);
void  simd_free_state_area(void *state);
void  simd_save_state(void *state);
void  simd_load_state(void *state);

#endif /* _KERNEL */

#endif /* RENZAN_SIMD_H__ */
