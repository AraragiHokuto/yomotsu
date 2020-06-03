#ifndef	YOMOTSU_TIMER_H_
#define YOMOTSU_TIMER_H_

#include <kern/types.h>

typedef u64	timer_deadline_t; /* deadline in ms */
typedef u64	timer_uduration_t; /* duration in ms */

typedef void (*timer_callback_t)(void *data);

struct timer_heap_element_s {
	timer_deadline_t	deadline;

	timer_callback_t	callback;
	void*	data;
};

typedef struct timer_heap_element_s	timer_heap_element_t;

void	timer_init_bsp(void);
void	timer_init_ap(void);

void	timer_spin_wait(timer_uduration_t);
void	timer_set_timeout(timer_uduration_t timeout,
			  timer_callback_t callback, void *data);
u64	timer_get_timestamp(void);

#endif	/* YOMOTSU_TIMER_H_ */
