#include <libos/process.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

char *argv[] = {
	"hell",
	"o wo",
	"rld eh he he aaaaaaaaaaaa",
	"aaaaaaaa ",
	"aaaaaaaaaaaaaaaaaaaaa"
};

extern uint8_t _binary_stub_proc_bin_start[];

int
__init_main(void)
{
        printf("init start\n");

        pid_t pid = process_spawn_from_memory(_binary_stub_proc_bin_start,
				  sizeof(argv)/sizeof(char*), argv);

	if (pid < 0) {
		perror("process_spawn_from_memory()");
	} else {
		printf("process spawned: pid = %ld\n", pid);
	}

	int64_t retval;
	if (!process_wait(pid, &retval)) {
		perror("process_wait()");
	} else if ((int)retval != 0xdeadbeef) {
		printf("expecting 0xdeadbeef; got 0x%x\n", (int)retval);
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
