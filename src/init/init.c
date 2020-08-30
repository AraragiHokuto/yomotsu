#include "calls.h"
#include <kern/error.h>
#include <kern/kobject.h>
#include <kern/port.h>

void
server(void)
{
        char buf[256];

        long port = port_create("test", 4);

        if (port < 0) {
                print("port_create() error\n");
                return;
        } else {
                print("port created\n");
        }

        while (1) {
                port_request_t req;
                print("dequeueing\n");
                long req_handler = port_receive(port, &req, buf, 255);
                print("dequeue\n");

                if (req.type == PORT_REQ_TYPE_OPEN) {
                        print("open request received\n");
                        port_response(
                            req_handler, PORT_REQ_OPEN_ACCEPT, (void *)0, 0);
                        print("open approved\n");
                        continue;
                }

                if (req.type == PORT_REQ_TYPE_CLOSE) {
                        print("close request received\n");
                        port_response(req_handler, 0, (void *)0, 0);
                        print("close responsed\n");
                        process_exit(0xc0ffee);
                        print("should be impossible to reach here\n");
                        asm volatile("xchgw %bx, %bx");
                }

                buf[req.data_size] = '\0';
                print(buf);
                print("\n");

                if (port_response(req_handler, 0xdeadbeef, (void *)0, 0)
                    != KERN_OK) {
                        print("port_response() error\n");
                        break;
                }
        }
}

extern void  __process_spawn_entry(void);
extern void *_entry_func;
extern void *_entry_arg;

int
main(void)
{
        print("init start\n");
        _entry_func          = server;
        kobject_handler_t as = as_clone(0);
        pid_t server_pid = process_spawn(as, (uintptr_t)__process_spawn_entry);

        long port;
        while ((port = port_open("test", 4)) < 0) {
                print("port_open() failed; retry\n");
        }

        print("port open\n");

        port_request_t req;
        req.type = PORT_REQ_TYPE_CUSTOM_START + 1;

        req.data_size    = 4;
        req.val_small = 0xdeadbaba;

        req.data_addr = "abcd";
        print("request\n");
        port_request(port, &req);
        print("request complete\n");
        if (req.val_small != 0xdeadbeef) {
                print("invalid retval\n");
                return -1;
        }
        req.val_small = 0xdeadbaba;

        req.data_addr = "efgh";
        print("request\n");
        port_request(port, &req);
        print("request complete\n");
        if (req.val_small != 0xdeadbeef) {
                print("invalid retval\n");
                return -1;
        }
        req.val_small = 0xdeadbaba;

        req.data_addr = "hijk";
        print("request\n");
        port_request(port, &req);
        print("request complete\n");
        if (req.val_small != 0xdeadbeef) {
                print("invalid retval\n");
                return -1;
        }

        print("closing port\n");
        port_close(port);
        print("port closed\n");

        process_state_t state;
        process_wait(-1, &state);

        print("wait returned\n");
        if (state.retval != 0xc0ffee) {
                print("not exactly what I was expecting for\n");
        }

        print("exiting init to trigger a panic\n");
        return 0;
}
