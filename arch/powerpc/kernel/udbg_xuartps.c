// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * udbg for xuartps compatible serial ports
 *
 * Copyright (C) 2025 Mohammad A. Nili <manili.devteam@gmail.com>
 *
 * Derived from arch/powerpc/kernel/udbg_16550.c, which is: 
 * Copyright (C) 2001-2005 PPC 64 Team, IBM Corp
 */
#include <linux/types.h>
#include <asm/udbg.h>
#include <asm/io.h>
#include <asm/early_ioremap.h>

static u8 (*udbg_uart_in)(unsigned int reg);
static void (*udbg_uart_out)(unsigned int reg, u8 data);

static void udbg_uart_flush(void)
{

}

static void udbg_uart_putc(char c)
{
	if (!udbg_uart_out)
		return;

	if (c == '\n')
		udbg_uart_putc('\r');
	udbg_uart_flush();
	while (udbg_uart_in(0x2c) & (1 << 4));
	udbg_uart_out(0x30, c);
}

static int udbg_uart_getc(void)
{
	if (!udbg_uart_in)
		return -1;

	/* wait for char */
	while (udbg_uart_in(0x2c) & (1 << 1))
		cpu_relax();

	return udbg_uart_in(0x30);
}

static int udbg_uart_getc_poll(void)
{
	if (!udbg_uart_in)
		return -1;

	if (udbg_uart_in(0x2c) & (1 << 1))
		return -1;
	
	return udbg_uart_in(0x30);
}

static void __init udbg_use_uart(void)
{
	udbg_flush     = udbg_uart_flush;
	udbg_putc      = udbg_uart_putc;
	udbg_getc      = udbg_uart_getc;
	udbg_getc_poll = udbg_uart_getc_poll;
}

void __init udbg_uart_setup(unsigned int speed, unsigned int clock)
{
	
}

unsigned int __init udbg_probe_uart_speed(unsigned int clock)
{
	return 115200;
}

static union {
	unsigned char __iomem *mmio_base;
	unsigned long pio_base;
} udbg_uart;

static unsigned int udbg_uart_stride = 1;

static u8 udbg_uart_in_mmio(unsigned int reg)
{
	return in_8(udbg_uart.mmio_base + (reg * udbg_uart_stride));
}

static void udbg_uart_out_mmio(unsigned int reg, u8 data)
{
	out_8(udbg_uart.mmio_base + (reg * udbg_uart_stride), data);
}


void __init udbg_uart_init_mmio(void __iomem *addr, unsigned int stride)
{
	if (!addr)
		return;
	udbg_uart.mmio_base = addr;
	udbg_uart_stride = stride;
	udbg_uart_in = udbg_uart_in_mmio;
	udbg_uart_out = udbg_uart_out_mmio;
	udbg_use_uart();
}
