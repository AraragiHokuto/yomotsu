/* errno.h -- Errors */
#ifndef RENZAN_CSTD_ERRNO_H_
#define RENZAN_CSTD_ERRNO_H_

#include <os/kernel/error.h>

#define __ERROR_OFFSET(no) (KERN_ERROR_END + no)

/* kernel -> POSIX error code mapping */
#define _OK       KERN_OK
#define ENOMEM    KERN_ERROR_NOMEM
#define EPERM     KERN_ERROR_UNAUTH
#define EINVAL    KERN_ERROR_INVAL
#define ENOENT    KERN_ERROR_NOENT
#define ETIMEDOUT KERN_ERROR_TIMEOUT

/* stdc error code */
#define EDOM      __ERROR_OFFSET(1)
#define EILSEQ    __ERROR_OFFSET(2)
#define ERANGE    __ERROR_OFFSET(3)
#define EOVERFLOW __ERROR_OFFSET(4)

/* TODO: generate POSIX error codes */

int *__get_errno(void);

#define errno (*__get_errno())

#endif /* RENZAN_CSTD_ERRNO_H_ */
