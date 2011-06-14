/*
 * evm.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include "common_def.h"

DECLARE_GLOBAL_DATA_PTR;

/* UART Defines */
#define UART_RESET		(0x1 << 1)
#define UART_CLK_RUNNING_MASK	0x1
#define UART_SMART_IDLE_EN	(0x1 << 0x3)

/* Timer Defines */
#define TSICR_REG		0x54
#define TIOCP_CFG_REG		0x10
#define TCLR_REG		0x38

void init_timer(void)
{
	/* Reset the Timer */
	__raw_writel(0x2, (DM_TIMER2_BASE + TSICR_REG));

	/* Wait until the reset is done */
	while (__raw_readl(DM_TIMER2_BASE + TIOCP_CFG_REG) & 1);

	/* Start the Timer */
	__raw_writel(0x1, (DM_TIMER2_BASE + TCLR_REG));
}

int dram_init(void)
{
	gd->ram_size = PHYS_DRAM_1_SIZE;

	return 0;
}

void dram_init_banksize (void)
{
	/* Fill up board info */
	gd->bd->bi_dram[0].start = PHYS_DRAM_1;
	gd->bd->bi_dram[0].size = PHYS_DRAM_1_SIZE;
}

int misc_init_r(void)
{
#ifdef CONFIG_AM335X_MIN_CONFIG
	printf("The 2nd stage U-Boot will now be auto-loaded\n");
	printf("Please do not interrupt the countdown till TIAM335X_EVM prompt \
		if 2nd stage is already flashed\n");
#endif

	return 0;
}

#ifdef CONFIG_AM335X_CONFIG_DDR
static void config_am335x_ddr(void)
{
	enable_ddr_clocks();

	__raw_writel(__raw_readl(VTP0_CTRL_REG) | VTP_CTRL_ENABLE,
		 VTP0_CTRL_REG);
	__raw_writel(__raw_readl(VTP0_CTRL_REG) & (~VTP_CTRL_START_EN),
		 VTP0_CTRL_REG);
	__raw_writel(__raw_readl(VTP0_CTRL_REG) | VTP_CTRL_START_EN,
		 VTP0_CTRL_REG);

	/* Poll for READY */
	while ((__raw_readl(VTP0_CTRL_REG) & VTP_CTRL_READY) != VTP_CTRL_READY);

	__raw_writel(__raw_readl(EMIF4_0_IODFT_TLGC) | DDR_PHY_RESET,
			EMIF4_0_IODFT_TLGC); /* Reset DDRr PHY */
	/* Wait while PHY is ready  */
	while ((__raw_readl(EMIF4_0_SDRAM_STATUS) & DDR_PHY_READY) == 0);

	__raw_writel(__raw_readl(EMIF4_0_IODFT_TLGC) | DDR_FUNCTIONAL_MODE_EN,
			EMIF4_0_IODFT_TLGC); /* Start Functional Mode */

	/*Program EMIF0 CFG Registers*/
	__raw_writel(EMIF_PHYCFG, EMIF4_0_DDR_PHY_CTRL_1);
	__raw_writel(EMIF_PHYCFG, EMIF4_0_DDR_PHY_CTRL_1_SHADOW);
	__raw_writel(EMIF_TIM1, EMIF4_0_SDRAM_TIM_1);
	__raw_writel(EMIF_TIM1, EMIF4_0_SDRAM_TIM_1_SHADOW);
	__raw_writel(EMIF_TIM2, EMIF4_0_SDRAM_TIM_2);
	__raw_writel(EMIF_TIM2, EMIF4_0_SDRAM_TIM_2_SHADOW);
	__raw_writel(EMIF_TIM3, EMIF4_0_SDRAM_TIM_3);
	__raw_writel(EMIF_TIM3, EMIF4_0_SDRAM_TIM_3_SHADOW);
	__raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL);
	__raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL_SHD);
	__raw_writel(EMIF_SDREF, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(EMIF_SDREF, EMIF4_0_SDRAM_REF_CTRL_SHADOW);
	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG);
}
#endif

/*
 * early system init of muxing and clocks.
 */
void s_init(u32 in_ddr)
{
	/* Can be removed as A8 comes up with L2 enabled */
	l2_cache_enable();
	/* Setup the PLLs and the clocks for the peripherals */
#ifdef CONFIG_SETUP_PLL
	pll_init();
#endif

#ifdef CONFIG_AM335X_CONFIG_DDR
	if (!in_ddr)
		config_am335x_ddr(); /* Do DDR settings */
#endif
}

/*
 * Basic board specific setup
 */
int board_init(void)
{
	u32 regVal;

	/* Enable UART0 Pin Mux */
	enable_uart_pin_mux();

	/* Enable EMIF DDR Mux */
	enable_emif_pin_mux();

	/* Initialize the Timer */
	init_timer();

	/* UART softreset */
	regVal = __raw_readl(UART_SYSCFG);
	regVal |= UART_RESET;
	__raw_writel(regVal, UART_SYSCFG);
	while ((__raw_readl(UART_SYSSTS) & UART_CLK_RUNNING_MASK) !=
		UART_CLK_RUNNING_MASK);

	/* Disable smart idle */
	regVal = __raw_readl(UART_SYSCFG);
	regVal |= UART_SMART_IDLE_EN;
	__raw_writel(regVal, UART_SYSCFG);

	/* mach type passed to kernel */
	gd->bd->bi_arch_number = MACH_TYPE_AM335XEVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_DRAM_1 + 0x100;

	gpmc_init();

	return 0;
}
