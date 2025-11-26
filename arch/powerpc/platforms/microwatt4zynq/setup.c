/*
 * Microwatt4Zynq Zynq-based SoC platform setup code.
 *
 * Derived from arch/powerpc/platforms/microwatt/setup.c, which is:
 * Copyright 2020 Paul Mackerras (paulus@ozlabs.org), IBM Corp.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_platform.h>

#include <asm/machdep.h>
#include <asm/time.h>
#include <asm/xics.h>
#include <asm/udbg.h>

#include "microwatt4zynq.h"

static void __init microwatt4zynq_init_IRQ(void)
{
	xics_init();
}

static int __init microwatt4zynq_populate(void)
{
	return of_platform_default_populate(NULL, NULL, NULL);
}
machine_arch_initcall(microwatt4zynq, microwatt4zynq_populate);

static int __init microwatt4zynq_probe(void)
{
	/* Main reason for having this is to start the other CPU(s) */
	if (IS_ENABLED(CONFIG_SMP))
		microwatt4zynq_init_smp();
	return 1;
}

static void __init microwatt4zynq_setup_arch(void)
{
	microwatt4zynq_rng_init();
}

static void microwatt4zynq_idle(void)
{
	if (!prep_irq_for_idle_irqsoff())
		return;

	__asm__ __volatile__ ("wait");
}

define_machine(microwatt4zynq) {
	.name			= "microwatt4zynq",
	.compatible		= "microwatt4zynq-soc",
	.probe			= microwatt4zynq_probe,
	.init_IRQ		= microwatt4zynq_init_IRQ,
	.setup_arch		= microwatt4zynq_setup_arch,
	.progress		= udbg_progress,
	.power_save		= microwatt4zynq_idle,
};
