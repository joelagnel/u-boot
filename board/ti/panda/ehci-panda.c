/*
 * OMAP4 EHCI port, copied from linux/drivers/usb/host/ehci-omap.c
 *
 * Copyright (C) 2007-2010 Texas Instruments, Inc.
 *	Author: Vikram Pandita <vikram.pandita@ti.com>
 *	Author: Anand Gadiyar <gadiyar@ti.com>
 */

#include <asm/arch/gpio.h>
#include <asm/arch/ehci.h>

#define GPIO_HUB_POWER		1
#define GPIO_HUB_NRESET		62

int ehci_hcd_init(void)
{
	unsigned long base = get_timer(0);
	unsigned reg = 0, port = 0;
	int rc;

	/* disable the power to the usb hub prior to init */
	rc = omap_request_gpio(GPIO_HUB_POWER);
	if (rc < 0) {
		printf("Could not request gpio %d\n", GPIO_HUB_POWER);
		return rc;
	}

	rc = omap_request_gpio(GPIO_HUB_NRESET);
	if (rc < 0) {
		printf("Could not request gpio %d\n", GPIO_HUB_NRESET);
		omap_free_gpio(GPIO_HUB_POWER);
		return rc;
	}

	omap_set_gpio_direction(GPIO_HUB_POWER, 0);
	omap_set_gpio_dataout(GPIO_HUB_POWER, 0);
	omap_set_gpio_direction(GPIO_HUB_NRESET, 0);
	omap_set_gpio_dataout(GPIO_HUB_NRESET, 0);
	omap_set_gpio_dataout(GPIO_HUB_NRESET, 1);

	rc = omap4_ehci_hcd_init();

	if (rc < 0)
		return rc;

	omap_set_gpio_dataout(GPIO_HUB_POWER, 1);

	return 0;
}

int ehci_hcd_stop(void)
{
	omap4_ehci_hcd_stop();

	omap_set_gpio_dataout(GPIO_HUB_POWER, 0);
	omap_set_gpio_dataout(GPIO_HUB_NRESET, 0);
	omap_free_gpio(GPIO_HUB_POWER);
	omap_free_gpio(GPIO_HUB_NRESET);
}
