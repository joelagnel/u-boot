/*
 * pll.c
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
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include "common_def.h"

#define PRCM_MOD_EN		0x2
#define	PRCM_FORCE_WAKEUP	0x2

#define PRCM_EMIF_CLK_ACTIVITY	(0x1 << 2)
#define PRCM_L3_GCLK_ACTIVITY	(0x1 << 4)

#define PLL_BYPASS_MODE	0x4
#define PLL_LOCK_MODE	0x7
#define PLL_MULTIPLIER_SHIFT	 8

void interface_clocks_enable(void)
{
	/* Enable all the Interconnect Modules */
	__raw_writel(PRCM_MOD_EN, CM_PER_L3_CLKCTRL);
	while (__raw_readl(CM_PER_L3_CLKCTRL) != PRCM_MOD_EN);

	__raw_writel(PRCM_MOD_EN, CM_PER_L4LS_CLKCTRL);
	while (__raw_readl(CM_PER_L4LS_CLKCTRL) != PRCM_MOD_EN);

	__raw_writel(PRCM_MOD_EN, CM_PER_L4FW_CLKCTRL);
	while (__raw_readl(CM_PER_L4FW_CLKCTRL) != PRCM_MOD_EN);

	__raw_writel(PRCM_MOD_EN, CM_WKUP_L4WKUP_CLKCTRL);
	while (__raw_readl(CM_WKUP_L4WKUP_CLKCTRL) != PRCM_MOD_EN);

	__raw_writel(PRCM_MOD_EN, CM_PER_L3_INSTR_CLKCTRL);
	while (__raw_readl(CM_PER_L3_INSTR_CLKCTRL) != PRCM_MOD_EN);

	__raw_writel(PRCM_MOD_EN, CM_PER_L4HS_CLKCTRL);
	while (__raw_readl(CM_PER_L4HS_CLKCTRL) != PRCM_MOD_EN);
}

void power_domain_transition_enable(void)
{
	/*
	 * Force power domain wake up transition
	 * Ensure that the corresponding interface clock is active before
	 * using the peripheral
	 */
	__raw_writel(PRCM_FORCE_WAKEUP, CM_PER_L3_CLKSTCTRL);

	__raw_writel(PRCM_FORCE_WAKEUP, CM_PER_L4LS_CLKSTCTRL);

	__raw_writel(PRCM_FORCE_WAKEUP, CM_WKUP_CLKSTCTRL);

	__raw_writel(PRCM_FORCE_WAKEUP, CM_PER_L4FW_CLKSTCTRL);

	__raw_writel(PRCM_FORCE_WAKEUP, CM_PER_L3S_CLKSTCTRL);
}

/*
 * Enable the module clock and the power domain for required peripherals
 */
void per_clocks_enable(void)
{
	/* Enable the module clock */
	__raw_writel(PRCM_MOD_EN, CM_PER_TIMER2_CLKCTRL);
	while (__raw_readl(CM_PER_TIMER2_CLKCTRL) != PRCM_MOD_EN);

	/* Select the Master osc 24 MHZ as Timer2 clock source */
	__raw_writel(0x1, CLKSEL_TIMER2_CLK);

	/* UART0 */
	__raw_writel(PRCM_MOD_EN, CM_WKUP_UART0_CLKCTRL);
	while (__raw_readl(CM_WKUP_UART0_CLKCTRL) != PRCM_MOD_EN);

	/* GPMC */
	__raw_writel(PRCM_MOD_EN, CM_PER_GPMC_CLKCTRL);
	while (__raw_readl(CM_PER_GPMC_CLKCTRL) != PRCM_MOD_EN);

	/* ELM */
	__raw_writel(PRCM_MOD_EN, CM_PER_ELM_CLKCTRL);
	while (__raw_readl(CM_PER_ELM_CLKCTRL) != PRCM_MOD_EN);

	/* i2c0 */
	__raw_writel(PRCM_MOD_EN, CM_WKUP_I2C0_CLKCTRL);
	while (__raw_readl(CM_WKUP_I2C0_CLKCTRL) != PRCM_MOD_EN);

	/* Ethernet */
	__raw_writel(PRCM_MOD_EN, CM_PER_CPGMAC0_CLKCTRL);
	__raw_writel(PRCM_MOD_EN, CM_PER_CPSW_CLKSTCTRL);
	while ((__raw_readl(CM_PER_CPGMAC0_CLKCTRL) & 0x30000) != 0x0);
}

void core_pll_config(void)
{
	/* Set the PLL to bypass Mode */
	__raw_writel(PLL_BYPASS_MODE, CM_CLKMODE_DPLL_CORE);

	/* Set the divider values */
	__raw_writel(COREPLL_M << PLL_MULTIPLIER_SHIFT | COREPLL_N,
		CM_CLKSEL_DPLL_CORE);

	/* Set the HSdiv Values */
	__raw_writel(COREPLL_M4, CM_DIV_M4_DPLL_CORE);

	__raw_writel(COREPLL_M5, CM_DIV_M5_DPLL_CORE);

	__raw_writel(COREPLL_M6, CM_DIV_M6_DPLL_CORE);

	/* Set the PLL to Lock Mode */
	__raw_writel(PLL_LOCK_MODE, CM_CLKMODE_DPLL_CORE);
}

void per_pll_config(void)
{
	/* Set the PLL to bypass Mode */
	__raw_writel(PLL_BYPASS_MODE, CM_CLKMODE_DPLL_PER);

	/* Set the divider values */
	__raw_writel(PERPLL_M << PLL_MULTIPLIER_SHIFT | PERPLL_N,
		CM_CLKSEL_DPLL_PER);

	/* Set the M2 divider value */
	__raw_writel(PERPLL_M2, CM_DIV_M2_DPLL_PER);

	/* Set the PLL to Lock Mode */
	__raw_writel(PLL_LOCK_MODE, CM_CLKMODE_DPLL_PER);
}

void ddr_pll_config(void)
{
	/* Set the PLL to bypass Mode */
	__raw_writel(PLL_BYPASS_MODE, CM_CLKMODE_DPLL_DDR);

	/* Set the divider values */
	__raw_writel((DDRPLL_M << PLL_MULTIPLIER_SHIFT) | DDRPLL_N,
		CM_CLKSEL_DPLL_DDR);

	/* Set the M2 divider value */
	__raw_writel(DDRPLL_M2, CM_DIV_M2_DPLL_DDR);

	/* Set the PLL to Lock Mode */
	__raw_writel(PLL_LOCK_MODE, CM_CLKMODE_DPLL_DDR);
}

void enable_ddr_clocks(void)
{
	/* Enable the  EMIF_FW Functional clock */
	__raw_writel(PRCM_MOD_EN, CM_PER_EMIF_FW_CLKCTRL);
	/* Enable EMIF0 Clock */
	__raw_writel(PRCM_MOD_EN, CM_PER_EMIF_CLKCTRL);
	/* Poll for emif_gclk  & L3_G clock  are active */
	while ((__raw_readl(CM_PER_L3_CLKSTCTRL) & (PRCM_EMIF_CLK_ACTIVITY |
		PRCM_L3_GCLK_ACTIVITY)) != (PRCM_EMIF_CLK_ACTIVITY |
		PRCM_L3_GCLK_ACTIVITY));
	/* Poll if module is functional */
	while ((__raw_readl(CM_PER_EMIF_CLKCTRL)) != PRCM_MOD_EN);

}

/*
 * Configure the PLL/PRCM for necessary peripherals
 */
void pll_init()
{
	/* Enable the control module */
	__raw_writel(PRCM_MOD_EN, CM_ALWON_CONTROL_CLKCTRL);

	core_pll_config();
	per_pll_config();
	ddr_pll_config();
	/* Enable the required interconnect clocks */
	interface_clocks_enable();
	/* Enable power domain transition */
	power_domain_transition_enable();
	/* Enable the required peripherals */
	per_clocks_enable();
}
