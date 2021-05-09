/* error.h -- kernel error code definitinons */

#ifndef __RENZAN_ERROR_H__
#define __RENZAN_ERROR_H__

enum KERN_ERRCODE {
        KERN_OK          = 0,
        KERN_ERROR_NOMEM = 1,
        KERN_ERROR_UNAUTH,
        KERN_ERROR_INVAL,
        KERN_ERROR_NOENT,
        KERN_ERROR_TIMEOUT,

        /* port */
        KERN_ERROR_PORT_CLOSED = 0x100,
        KERN_ERROR_PORT_NAME_OCCUPIED,
        KERN_ERROR_PORT_REJECTED,
        KERN_ERROR_PORT_DATA_TOO_LONG,
	KERN_ERROR_PORT_CANCELED,

        KERN_ERROR_END = 0x1000
};

#endif /* __RENZAN_ERROR_H__ */
