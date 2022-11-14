/* arch/amd64/hal_serial.c -- Serial port driver for debugging purpose */

/*
 * Copyright 2022 Tenhouin Youkou <youkou@tenhou.in>.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "hal_asm.h"
#include <hal_serial.h>

#include <osrt/types.h>

#ifdef _KDEBUG

const static u16 COM_PORTS[] = {0x3f8, 0x2f8, 0x3e8, 0x2e8,
                                0x5f8, 0x4f8, 0x5e8, 0x4e8};

enum {
        REG_DATA         = 0,
        REG_INT          = 1,
        REG_FIFO         = 2,
        REG_LINE_CTL     = 3,
        REG_MODEM_CTL    = 4,
        REG_LINE_STATUS  = 5,
        REG_MODEM_STATUS = 6,
        REG_SCRATCH      = 7,
};

static u16 base_port = 0;

static boolean
probe_port(u16 port)
{
        outb(port + REG_INT, 0x00);      /* disable interrupts */
        outb(port + REG_LINE_CTL, 0x80); /* enable DLAB */
        outb(port + 0, 0x03);            /* divisor: 3 */
        outb(port + 1, 0x00);
        outb(port + REG_LINE_CTL, 0x03);  /* 8 bits, no parity, one stop bit */
        outb(port + REG_FIFO, 0XC7);      /* enable FIFO */
        outb(port + REG_MODEM_CTL, 0x0b); /* set RTS/DSR */
        outb(port + REG_MODEM_CTL, 0x1e); /* set loopback mode */
        outb(port + REG_DATA, 0xae);      /* send 0xae */

        if (inb(port + REG_DATA) != 0xae) {
                /* faulty */
                return B_FALSE;
        }

        /* set to normal operation mode */
        outb(port + REG_MODEM_CTL, 0x0f);
        return B_TRUE;
}

static void
write_reg(u16 reg, u8 data)
{
        ASSERT(base_port);

        outb(base_port + reg, data);
}

static u8
read_reg(u16 reg)
{
        ASSERT(base_port);

        return inb(base_port + reg);
}

void
serial_init(void)
{
        for (size_t i = 0; i < sizeof(COM_PORTS) / sizeof(COM_PORTS[0]); ++i) {
                if (!probe_port(COM_PORTS[i])) { continue; }

                base_port = COM_PORTS[i];
                break;
        }
}

static u8
is_transmit_empty()
{
        return inb(REG_LINE_STATUS) & 0x20;
}

void
serial_write(char a)
{
	if (!base_port) return;

	/* wait until transmit empty */
	while (is_transmit_empty() == 0);

	write_reg(REG_DATA, a);
}

#endif /* _KDEBUG */
