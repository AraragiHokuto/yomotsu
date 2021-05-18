/* misc.h -- general utilities */
#ifndef __RENZAN_LIBOS_MISC_H__
#define __RENZAN_LIBOS_MISC_H__

#ifdef __RENZAN_OSRT

#define ALIGNDOWN(x, a) ((x) / (a) * (a))
#define ALIGNUP(x, a) ((((x) - 1) / (a) + 1) * (a))

#endif	/* __RENZAN_OSRT */

#endif	/* __RENZAN_LIBOS_MISC_H__ */
