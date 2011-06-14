/*
 * Copyright (C) 2009, Texas Instruments, Incorporated
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __TI81XX_HARDWARE_H
#define __TI81XX_HARDWARE_H

/* The objective is to keep only the overall memory map here
 * The break-up of the memory map for individual modules registers should
 * in a diff file like cpu.h so that when this is the only place
 * where change is needed for new SoCs when the ip is the same
 */
#define SRAM0_START			0x40300000

#ifdef CONFIG_AM335X
#define UART0_BASE			0x44E09000
#else
#define UART0_BASE			0x48020000
#endif
#define UART1_BASE			0x48022000
#define UART2_BASE			0x48024000

/* DM Timer base addresses */
#define DM_TIMER0_BASE			0x4802C000
#define DM_TIMER1_BASE			0x4802E000
#define DM_TIMER2_BASE			0x48040000
#define DM_TIMER3_BASE			0x48042000
#define DM_TIMER4_BASE			0x48044000
#define DM_TIMER5_BASE			0x48046000
#define DM_TIMER6_BASE			0x48048000
#define DM_TIMER7_BASE			0x4804A000

/* GPIO Base address */
#define GPIO0_BASE			0x48032000
#define GPIO1_BASE			0x4804C000

/* BCH Error Location Module */
#define ELM_BASE			0x48080000

/* Watchdog Timer */
#define WDT_BASE			0x480C2000

/* Control Module Base Address */
#ifdef CONFIG_AM335X
#define CTRL_BASE			0x44E10000
#else
#define CTRL_BASE			0x48140000
#endif

/* PRCM Base Address */
#ifdef CONFIG_AM335X
#define PRCM_BASE			0x44E00000
#else
#define PRCM_BASE			0x48180000
#endif

/* PLL Subsystem Base Address */
#ifdef CONFIG_TI814X
#define PLL_SUBSYS_BASE			0x481C5000
#endif

/* EMIF Base address */
#define EMIF4_0_CFG_BASE		0x4C000000
#define EMIF4_1_CFG_BASE		0x4D000000
#define DMM_BASE			0x4E000000

#ifdef CONFIG_TI816X
#define DDRPHY_0_CONFIG_BASE		0x48198000
#define DDRPHY_1_CONFIG_BASE		0x4819a000
#define DDRPHY_CONFIG_BASE 		((emif == 0) ? DDRPHY_0_CONFIG_BASE:DDRPHY_1_CONFIG_BASE)
#endif

#ifdef CONFIG_TI814X
#define DDRPHY_0_CONFIG_BASE		(CTRL_BASE + 0x1400)
#define DDRPHY_1_CONFIG_BASE		(CTRL_BASE + 0x1500)
#define DDRPHY_CONFIG_BASE 		((emif == 0) ? DDRPHY_0_CONFIG_BASE:DDRPHY_1_CONFIG_BASE)
#endif

#ifdef CONFIG_AM335X
#define DDRPHY_0_CONFIG_BASE		(CTRL_BASE + 0x1400)
#define DDRPHY_CONFIG_BASE		DDRPHY_0_CONFIG_BASE
#endif

/* GPMC Base address */
#define GPMC_BASE			0x50000000

/* CPSW Config space */
#define TI814X_CPSW_BASE               0x4A100000
#define TI814X_CPSW_MDIO_BASE          0x4A100800

#endif /* __TI81XX_HARDWARE_H */

