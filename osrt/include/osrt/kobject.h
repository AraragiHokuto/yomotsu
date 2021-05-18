/* XXX Will be removed soon */

#ifndef __RENZAN_OSRT_KOBJECT_H__
#define __RENZAN_OSRT_KOBJECT_H__

#include <osrt/types.h>

typedef __osrt_size_t __osrt_kobject_handler_t;

#ifdef __RZ_KERNEL

typedef __osrt_kobject_handler_t kobject_handler_t;

#else  /* __RZ_KERNEL */

typedef __osrt_kobject_handler_t kobject_t;

#endif	/* __RZ_KERNEL */

#endif	/* __RENZAN_OSRT_KOBJECT_H__ */
