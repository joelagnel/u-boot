/*
 *
 * Common board functions for AM33XX based boards.
 *
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 *
 * Derived from arch/arm/cpu/armv7/omap3/board.c by
 *      Sunil Kumar <sunilsaini05@gmail.com>
 *      Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_common.h>

#ifdef CONFIG_SPL_BUILD
/*
* We use static variables because global data is not ready yet.
* Initialized data is available in SPL right from the beginning.
* We would not typically need to save these parameters in regular
* U-Boot. This is needed only in SPL at the moment.
*/
u32 ti81xx_boot_device = BOOT_DEVICE_NAND;

/* auto boot mode detection is not possible for AM335X - hard code */
u32 omap_boot_mode(void)
{
	switch (omap_boot_device()) {
	case BOOT_DEVICE_MMC2:
		return MMCSD_MODE_RAW;
	case BOOT_DEVICE_MMC1:
		return MMCSD_MODE_FAT;
		break;
	case BOOT_DEVICE_NAND:
		return NAND_MODE_HW_ECC;
		break;
	default:
		puts("spl: ERROR:  unknown device - can't select boot mode\n");
		hang();
	}
}

u32 omap_boot_device(void)
{
	return ti81xx_boot_device;
}
#endif /* CONFIG_SPL_BUILD */
