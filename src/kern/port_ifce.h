/* port_ifce.h -- port-like interface for various kernel functions */
#ifndef	IZANAMI_PORT_IFCE_H__
#define IZANAMI_PORT_IFCE_H__

#include <kern/process.h>
#include <kern/boolean.h>

#ifdef _KERNEL

typedef sint	(*port_ifce_open_func_t)(void);
typedef u64	(*port_ifce_req_func_t)(void *data, size_t datalen,
					void *reply, size_t replylen);
typedef void	(*port_ifce_close_func_t)(void);

struct port_ifce_s {
	char	*name;
	port_ifce_open_func_t	open_func;
	port_ifce_req_func_t	req_func;
	port_ifce_close_func_t	close_func;
};

typedef struct port_ifce_s	port_ifce_t;

void	port_ifce_init(void);
void	port_ifce_create(port_ifce_t *port);
port_ifce_t *	port_ifce_lookup(const char *name);

#endif	/* _KERNEL */

#endif	/* IZANAMI_PORT_IFCE_H__ */
