#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <osrt/port.h>
#include <osrt/process.h>

#define PS2_PORT_DATA    0x60
#define PS2_PORT_STATUS  0x64
#define PS2_PORT_COMMAND 0x64

void __attribute__((constructor)) __constructor1(void)
{
        printf("constructor1\n");
}

void __attribute__((constructor)) __constructor2(void)
{
        printf("constructor2\n");
}

void __attribute__((destructor)) __destructor1(void)
{
        printf("destructor1\n");
}

void __attribute__((destructor)) __destructor2(void)
{
        printf("destructor2\n");
}

void
outb(uint16_t port, uint8_t val)
{
        asm volatile("outb %0, %1" ::"a"(val), "Nd"(port));
}

uint8_t
inb(uint16_t port)
{
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}

void
ps2_send_command(uint8_t command)
{
        outb(PS2_PORT_COMMAND, command);
}

uint8_t
ps2_recv_status(void)
{
        return inb(PS2_PORT_STATUS);
}

void
ps2_send_data(uint8_t data)
{
        while (true) {
                uint8_t status = ps2_recv_status();
                if (status & (1 << 1)) {
                        __builtin_ia32_pause();
                        continue;
                }
                outb(PS2_PORT_DATA, data);
                break;
        }
}

uint8_t
ps2_recv_data(void)
{
        /* TODO: interrupt */
        while (true) {
                uint8_t status = ps2_recv_status();
                if (!(status & 1)) {
                        __builtin_ia32_pause();
                        continue;
                }
                return inb(PS2_PORT_DATA);
        }
}

void
init_ps2_keyboard(void)
{
        /* disable devices */
        ps2_send_command(0xad);
        ps2_send_command(0xa7);

        /* flush output buffer */
        inb(PS2_PORT_DATA);

        /* set controller configuration byte */
        ps2_send_command(0x20);
        uint8_t conf_byte = ps2_recv_data();
        /* disable IRQs and translation */
        conf_byte &= 0xbc;
        ps2_send_command(0x60);
        ps2_send_data(conf_byte);

        /* perform controller self test */
        ps2_send_command(0xaa);
        uint8_t self_test_byte = ps2_recv_data();
        assert(self_test_byte == 0x55);

        /* restore configuration byte just in case */
        ps2_send_command(0x60);
        ps2_send_data(conf_byte);

        /* enable first port */
        ps2_send_command(0xae);
        /* conf_byte |= 1; */
        /* ps2_send_command(0x60); */
        /* ps2_send_data(conf_byte); */

        /* reset device on first port */
        ps2_send_data(0xff);
        self_test_byte = ps2_recv_data();
        assert(self_test_byte == 0xfa);
        self_test_byte = ps2_recv_data();
        assert(self_test_byte == 0xaa);
}

uint8_t
getc(void)
{
        return ps2_recv_data();
}

int
main(int argc, char **argv)
{
        printf("stub process spawned\n");
        printf("argc = %d, argv at %p\n", argc, argv);

        for (int i = 0; i < argc; ++i) {
                printf("argv[%d] = %s\n", i, argv[i]);
        }

        port_t port = port_open("/stubinit/testport");
        if (port < 0) {
                perror("port_open()");
                return -1;
        }

        init_ps2_keyboard();
        printf("ps2 controller initialized\n");
        while (true) {
                uint8_t data = getc();
                printf("received ps2 data: %d\n", data);

                port_request_t req;
                req.data_addr = NULL;
                req.data_size = 0;
                req.val_small = data;

                if (port_request(port, &req) < 0) {
                        perror("port_request()");
                        break;
                }

                if (req.val_small != data) {
                        printf(
                            "invalid response: %lu vs %d", req.val_small, data);
                }
        }

        return 0xdeadbeef;
}
