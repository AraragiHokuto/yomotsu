#ifndef KAGUYA_BOOLEAN_H_
#define KAGUYA_BOOLEAN_H_

#include <os/kernel/types.h>

typedef __u64 __boolean;

#define _B_TRUE  1
#define _B_FALSE 0

#ifdef _KERNEL

typedef __boolean boolean;
#define B_TRUE  _B_TRUE
#define B_FALSE _B_FALSE

#endif /* _KERNEL */

#endif /* KAGUYA_BOOLEAN_H_ */
