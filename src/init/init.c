#include <libos/port.h>
#include <libos/process.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char *argv[] = {
    "hell", "o wo", "rld eh he he aaaaaaaaaaaa", "aaaaaaaa ",
    "aaaaaaaaaaaaaaaaaaaaa"};

extern uint8_t _binary_stub_proc_bin_start[];

/* stub symbol for crt */
typedef void (*__crt_func_ptr_t)(void);
__crt_func_ptr_t __preinit_array_start = NULL;
__crt_func_ptr_t __preinit_array_end = NULL;
__crt_func_ptr_t __init_array_start = NULL;
__crt_func_ptr_t __init_array_end = NULL;
__crt_func_ptr_t __fini_array_start = NULL;
__crt_func_ptr_t __fini_array_end = NULL;

int
__init_main(void)
{
        printf("init start\n");

        port_t port = port_create("/stubinit/testport");
        if (port < 0) {
                perror("port_create()");
                process_exit(-1);
        }

        pid_t pid = process_spawn_from_memory(
            _binary_stub_proc_bin_start, sizeof(argv) / sizeof(char *), argv);

        if (pid < 0) {
                perror("process_spawn_from_memory()");
        } else {
                printf("process spawned: pid = %ld\n", pid);
        }

        for (int i = 0; i < 10; ++i) {
                port_request_t buffer;

		printf("receiving\n");
                kobject_t req = port_receive(port, &buffer, NULL, 0);
		if (req < 0) {
                        perror("port_receive()");
                        process_exit(-1);
                }

                printf(
                    "received request from port by %ld: value %lu\n",
                    buffer.sender_pid, buffer.val_small);

		if (port_response(req, buffer.val_small, NULL, 0) != 0) {
			perror("port_response()");
			break;
		}
        }

	port_close(port);

        int64_t retval;
        if (!process_wait(pid, &retval)) {
                perror("process_wait()");
        } else if ((int)retval != 0xdeadbeef) {
                printf("expecting 0xdeadbeef; got 0x%x\n", (int)retval);
        } else {
		printf("received retval: 0x%x\n", (int)retval);
	}

        printf("exiting init to trigger a panic\n");
        process_exit(0);
}

int
main(void)
{
        /* this should not be called */
        printf("main in stub_init called\n");
}
