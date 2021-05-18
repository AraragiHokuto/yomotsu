/* errno.h -- Errors */
#ifndef RENZAN_CSTD_ERRNO_H_
#define RENZAN_CSTD_ERRNO_H_

#include <osrt/error.h>

#define __ERROR_OFFSET(no) (__OSRT_ERROR_END + no)

/* kernel -> POSIX error code mapping */
#define _OK       __OSRT_OK
#define EINVAL    __OSRT_ERROR_INVAL
#define ENOENT    __OSRT_ERROR_NOENT
#define ENOMEM    __OSRT_ERROR_NOMEM
#define EPERM     __OSRT_ERROR_DENIED

/* stdc error code */
#define EDOM      __ERROR_OFFSET(1)
#define EILSEQ    __ERROR_OFFSET(2)
#define ERANGE    __ERROR_OFFSET(3)
#define EOVERFLOW __ERROR_OFFSET(4)

/* TODO: generate POSIX error codes */

int *__get_errno(void);

#define errno (*__get_errno())

#endif /* RENZAN_CSTD_ERRNO_H_ */
