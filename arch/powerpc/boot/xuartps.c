#include <stdarg.h>
#include <stddef.h>
#include "types.h"
#include "string.h"
#include "stdio.h"
#include "io.h"
#include "ops.h"
#include "of.h"

/* Xilinx UART PS register definitions */
#define CDNS_UART_CR			0x00  /* Control Register */
#define CDNS_UART_SR			0x2C  /* Channel Status */
#define CDNS_UART_FIFO			0x30  /* FIFO */
#define CDNS_UART_SR_TXFULL		0x00000010 /* TX FIFO full */
#define CDNS_UART_SR_TXEMPTY		0x00000008 /* TX FIFO empty */
#define CDNS_UART_SR_RXEMPTY		0x00000002 /* RX FIFO empty */
#define CDNS_UART_CR_TX_DIS		0x00000020  /* TX disabled. */
#define CDNS_UART_CR_TX_EN		0x00000010  /* TX enabled */
#define CDNS_UART_CR_TXRST		0x00000002  /* TX logic reset */
#define CDNS_UART_CR_RXRST		0x00000001  /* RX logic reset */

static unsigned char *reg_base;
static uint32_t reg_base_addr;

static int xuartps_open(void)
{
	u8 ctrl;
	
	/* Reset TX & RX */
	ctrl = readl(reg_base_addr + CDNS_UART_CR);
	ctrl |= CDNS_UART_CR_TXRST | CDNS_UART_CR_RXRST;
	writeb(ctrl, reg_base_addr + CDNS_UART_CR);

	while (readl(reg_base_addr + CDNS_UART_CR) & (CDNS_UART_CR_TXRST | CDNS_UART_CR_RXRST));

	/* Enable TX */
	ctrl = readl(reg_base_addr + CDNS_UART_CR);
	ctrl &= ~CDNS_UART_CR_TX_DIS;
	ctrl |= CDNS_UART_CR_TX_EN;
	writeb(ctrl, reg_base_addr + CDNS_UART_CR);

	return 0;
}

static void xuartps_putc(unsigned char ch)
{
	while (readl(reg_base_addr + CDNS_UART_SR) & CDNS_UART_SR_TXFULL);

	writeb(ch, reg_base_addr + CDNS_UART_FIFO);

	while (!(readl(reg_base_addr + CDNS_UART_SR) & CDNS_UART_SR_TXEMPTY));
}

static unsigned char xuartps_getc(void)
{
	while (readl(reg_base_addr + CDNS_UART_SR) & CDNS_UART_SR_RXEMPTY);

	return (unsigned char)readl(reg_base_addr + CDNS_UART_FIFO);
}

static unsigned char xuartps_tstc(void)
{
	return !(readl(reg_base_addr + CDNS_UART_SR) & CDNS_UART_SR_RXEMPTY);
}

int xuartps_console_init(void *devp, struct serial_console_data *scd)
{
	if (dt_get_virtual_reg(devp, (void **)&reg_base, 1) < 1) {
		printf("virt reg parse fail...\r\n");
		return -1;
	}
	reg_base_addr = (unsigned long)(uint32_t *)reg_base;
	
	scd->open = xuartps_open;
	scd->putc = xuartps_putc;
	scd->getc = xuartps_getc;
	scd->tstc = xuartps_tstc;
	scd->close = NULL;

	return 0;
}
