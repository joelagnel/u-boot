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
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include "common_def.h"
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

#undef EVM_EEPROM_DEBUG

/*define above EVM_EEPROM_DEBUG to get debug printf's for evm config display */
#ifdef	EVM_EEPROM_DEBUG
#define PRINTD(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTD(fmt,args...)
#endif

#ifndef	CONFIG_AM335X_MIN_CONFIG
#define PRINT_FULL(fmt,args...)	printf (fmt ,##args)
#else
#define PRINT_FULL(fmt,args...)
#endif

/* UART Defines */
#define UART_SYSCFG_OFFSET	(0x54)
#define UART_SYSSTS_OFFSET	(0x58)

#define UART_RESET		(0x1 << 1)
#define UART_CLK_RUNNING_MASK	0x1
#define UART_SMART_IDLE_EN	(0x1 << 0x3)

/* Timer Defines */
#define TSICR_REG		0x54
#define TIOCP_CFG_REG		0x10
#define TCLR_REG		0x38

static volatile int board_id = GP_BOARD;
static unsigned char daughter_board_connected = FALSE;
static unsigned char profile = PROFILE_NONE;

static void init_timer(void)
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

static void Data_Macro_Config(int dataMacroNum)
{
	u32 BaseAddrOffset;

	if (dataMacroNum == 0)
		BaseAddrOffset = 0x00;
	else if (dataMacroNum == 1)
		BaseAddrOffset = 0xA4;

	__raw_writel(((DDR2_RD_DQS<<30)|(DDR2_RD_DQS<<20)
			|(DDR2_RD_DQS<<10)|(DDR2_RD_DQS<<0)),
			(DATA0_RD_DQS_SLAVE_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR2_RD_DQS>>2,
			(DATA0_RD_DQS_SLAVE_RATIO_1 + BaseAddrOffset));
	__raw_writel(((DDR2_WR_DQS<<30)|(DDR2_WR_DQS<<20)
			|(DDR2_WR_DQS<<10)|(DDR2_WR_DQS<<0)),
			(DATA0_WR_DQS_SLAVE_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR2_WR_DQS>>2,
			(DATA0_WR_DQS_SLAVE_RATIO_1 + BaseAddrOffset));
	__raw_writel(((DDR2_PHY_WRLVL<<30)|(DDR2_PHY_WRLVL<<20)
			|(DDR2_PHY_WRLVL<<10)|(DDR2_PHY_WRLVL<<0)),
			(DATA0_WRLVL_INIT_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR2_PHY_WRLVL>>2,
			(DATA0_WRLVL_INIT_RATIO_1 + BaseAddrOffset));
	__raw_writel(((DDR2_PHY_GATELVL<<30)|(DDR2_PHY_GATELVL<<20)
			|(DDR2_PHY_GATELVL<<10)|(DDR2_PHY_GATELVL<<0)),
			(DATA0_GATELVL_INIT_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR2_PHY_GATELVL>>2,
			(DATA0_GATELVL_INIT_RATIO_1 + BaseAddrOffset));
	__raw_writel(((DDR2_PHY_FIFO_WE<<30)|(DDR2_PHY_FIFO_WE<<20)
			|(DDR2_PHY_FIFO_WE<<10)|(DDR2_PHY_FIFO_WE<<0)),
			(DATA0_FIFO_WE_SLAVE_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR2_PHY_FIFO_WE>>2,
			(DATA0_FIFO_WE_SLAVE_RATIO_1 + BaseAddrOffset));
	__raw_writel(((DDR2_PHY_WR_DATA<<30)|(DDR2_PHY_WR_DATA<<20)
			|(DDR2_PHY_WR_DATA<<10)|(DDR2_PHY_WR_DATA<<0)),
			(DATA0_WR_DATA_SLAVE_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR2_PHY_WR_DATA>>2,
			(DATA0_WR_DATA_SLAVE_RATIO_1 + BaseAddrOffset));
	__raw_writel(PHY_DLL_LOCK_DIFF,
			(DATA0_DLL_LOCK_DIFF_0 + BaseAddrOffset));
}

static void Cmd_Macro_Config(void)
{
	__raw_writel(DDR2_RATIO, CMD0_CTRL_SLAVE_RATIO_0);
	__raw_writel(CMD_FORCE, CMD0_CTRL_SLAVE_FORCE_0);
	__raw_writel(CMD_DELAY, CMD0_CTRL_SLAVE_DELAY_0);
	__raw_writel(DDR2_DLL_LOCK_DIFF, CMD0_DLL_LOCK_DIFF_0);
	__raw_writel(DDR2_INVERT_CLKOUT, CMD0_INVERT_CLKOUT_0);

	__raw_writel(DDR2_RATIO, CMD1_CTRL_SLAVE_RATIO_0);
	__raw_writel(CMD_FORCE, CMD1_CTRL_SLAVE_FORCE_0);
	__raw_writel(CMD_DELAY, CMD1_CTRL_SLAVE_DELAY_0);
	__raw_writel(DDR2_DLL_LOCK_DIFF, CMD1_DLL_LOCK_DIFF_0);
	__raw_writel(DDR2_INVERT_CLKOUT, CMD1_INVERT_CLKOUT_0);

	__raw_writel(DDR2_RATIO, CMD2_CTRL_SLAVE_RATIO_0);
	__raw_writel(CMD_FORCE, CMD2_CTRL_SLAVE_FORCE_0);
	__raw_writel(CMD_DELAY, CMD2_CTRL_SLAVE_DELAY_0);
	__raw_writel(DDR2_DLL_LOCK_DIFF, CMD2_DLL_LOCK_DIFF_0);
	__raw_writel(DDR2_INVERT_CLKOUT, CMD2_INVERT_CLKOUT_0);
}

static void config_vtp(void)
{
	__raw_writel(__raw_readl(VTP0_CTRL_REG) | VTP_CTRL_ENABLE,
			VTP0_CTRL_REG);
	__raw_writel(__raw_readl(VTP0_CTRL_REG) & (~VTP_CTRL_START_EN),
			VTP0_CTRL_REG);
	__raw_writel(__raw_readl(VTP0_CTRL_REG) | VTP_CTRL_START_EN,
			VTP0_CTRL_REG);

	/* Poll for READY */
	while ((__raw_readl(VTP0_CTRL_REG) & VTP_CTRL_READY) != VTP_CTRL_READY);
}

static void config_emif(void)
{
	u32 i;

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
	__raw_writel(EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_1);
	__raw_writel(EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_1_SHADOW);
	__raw_writel(EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_2);
	__raw_writel(EMIF_TIM1, EMIF4_0_SDRAM_TIM_1);
	__raw_writel(EMIF_TIM1, EMIF4_0_SDRAM_TIM_1_SHADOW);
	__raw_writel(EMIF_TIM2, EMIF4_0_SDRAM_TIM_2);
	__raw_writel(EMIF_TIM2, EMIF4_0_SDRAM_TIM_2_SHADOW);
	__raw_writel(EMIF_TIM3, EMIF4_0_SDRAM_TIM_3);
	__raw_writel(EMIF_TIM3, EMIF4_0_SDRAM_TIM_3_SHADOW);

	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG);
	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG2);

	/* __raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL);
	__raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL_SHD); */
	__raw_writel(0x00004650, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(0x00004650, EMIF4_0_SDRAM_REF_CTRL_SHADOW);

	for(i = 0; i<10000; i++)
	{

	}

	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG);
	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG2);

	/* __raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL);
	__raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL_SHD); */
	__raw_writel(EMIF_SDREF,EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(EMIF_SDREF, EMIF4_0_SDRAM_REF_CTRL_SHADOW);
}

static void config_emif_ddr2(void)
{
	u32 i;

	/*Program EMIF0 CFG Registers*/
	__raw_writel(EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_1);
	__raw_writel(EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_1_SHADOW);
	__raw_writel(EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_2);
	__raw_writel(EMIF_TIM1, EMIF4_0_SDRAM_TIM_1);
	__raw_writel(EMIF_TIM1, EMIF4_0_SDRAM_TIM_1_SHADOW);
	__raw_writel(EMIF_TIM2, EMIF4_0_SDRAM_TIM_2);
	__raw_writel(EMIF_TIM2, EMIF4_0_SDRAM_TIM_2_SHADOW);
	__raw_writel(EMIF_TIM3, EMIF4_0_SDRAM_TIM_3);
	__raw_writel(EMIF_TIM3, EMIF4_0_SDRAM_TIM_3_SHADOW);

	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG);
	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG2);

	/* __raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL);
	__raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL_SHD); */
	__raw_writel(0x00004650, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(0x00004650, EMIF4_0_SDRAM_REF_CTRL_SHADOW);

	for (i = 0; i < 5000; i++) {

	}

	/* __raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL);
	__raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL_SHD); */
	__raw_writel(EMIF_SDREF, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(EMIF_SDREF, EMIF4_0_SDRAM_REF_CTRL_SHADOW);

	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG);
	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG2);
}

static void config_am335x_mddr(void)
{
	int data_macro_0 = 0;
	int data_macro_1 = 1;

	enable_ddr_clocks();

	Cmd_Macro_Config();

	Data_Macro_Config(data_macro_0);
	Data_Macro_Config(data_macro_1);

	__raw_writel(PHY_RANK0_DELAY, DATA0_RANK0_DELAYS_0);
	__raw_writel(PHY_RANK0_DELAY, DATA1_RANK0_DELAYS_0);

	/* set IO control registers */
	__raw_writel(DDR_IOCTRL_VALUE, DDR_CMD0_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_CMD1_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_CMD2_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_DATA0_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_DATA1_IOCTRL);

	__raw_writel(__raw_readl(DDR_IO_CTRL) | 0x10000000, DDR_IO_CTRL);
	__raw_writel(__raw_readl(DDR_CKE_CTRL) | 0x00000001, DDR_CKE_CTRL);

	config_emif();	/* vtp enable is here */
}

/*  void DDR2_EMIF_Config(void); */
static void config_am335x_ddr2(void)
{
	int data_macro_0 = 0;
	int data_macro_1 = 1;

	enable_ddr_clocks();

	config_vtp();

	Cmd_Macro_Config();

	Data_Macro_Config(data_macro_0);
	Data_Macro_Config(data_macro_1);

	__raw_writel(PHY_RANK0_DELAY, DATA0_RANK0_DELAYS_0);
	__raw_writel(PHY_RANK0_DELAY, DATA1_RANK0_DELAYS_0);

	__raw_writel(DDR_IOCTRL_VALUE, DDR_CMD0_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_CMD1_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_CMD2_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_DATA0_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_DATA1_IOCTRL);

	__raw_writel(__raw_readl(DDR_IO_CTRL) & 0xefffffff, DDR_IO_CTRL);
	__raw_writel(__raw_readl(DDR_CKE_CTRL) | 0x00000001, DDR_CKE_CTRL);

	config_emif_ddr2();
}

static void config_am335x_ddr(void)
{
#if	(CONFIG_AM335X_EVM_IS_13x13 ==1)
	config_am335x_mddr(); /* Do DDR settings for 13x13 */
#else
	config_am335x_ddr2();
#endif
}

#endif

/*
 * early system init of muxing and clocks.
 */
void s_init(u32 in_ddr)
{
	/* Can be removed as A8 comes up with L2 enabled */
	l2_cache_enable();

	/* WDT1 is already running when the bootloader gets control
	 * Disable it to avoid "random" resets
	 */
	__raw_writel(0xAAAA, WDT_WSPR);
	while(__raw_readl(WDT_WWPS) != 0x0);
	__raw_writel(0x5555, WDT_WSPR);
	while(__raw_readl(WDT_WWPS) != 0x0);

	/* Setup the PLLs and the clocks for the peripherals */
#ifdef CONFIG_SETUP_PLL
	pll_init();
#endif

#ifdef CONFIG_AM335X_CONFIG_DDR
	if (!in_ddr)
		config_am335x_ddr();
#endif
}

/*
 * Basic board specific setup
 */
#ifdef CONFIG_AM335X_MIN_CONFIG
int board_min_init(void)
{
	u32 regVal;
	u32 uart_base = DEFAULT_UART_BASE;

	/* UART softreset */
	regVal = __raw_readl(uart_base + UART_SYSCFG_OFFSET);
	regVal |= UART_RESET;
	__raw_writel(regVal, (uart_base + UART_SYSCFG_OFFSET) );
	while ((__raw_readl(uart_base + UART_SYSSTS_OFFSET) &
			UART_CLK_RUNNING_MASK) != UART_CLK_RUNNING_MASK);

	/* Disable smart idle */
	regVal = __raw_readl((uart_base + UART_SYSCFG_OFFSET));
	regVal |= UART_SMART_IDLE_EN;
	__raw_writel(regVal, (uart_base + UART_SYSCFG_OFFSET));

	/* Initialize the Timer */
	init_timer();

	return 0;
}
#else
int board_evm_init(void)
{
	/* mach type passed to kernel */
	if (board_id == IA_BOARD)
		gd->bd->bi_arch_number = MACH_TYPE_TIAM335IAEVM;
	else
		gd->bd->bi_arch_number = MACH_TYPE_TIAM335EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_DRAM_1 + 0x100;

	return 0;
}
#endif

int board_init(void)
{

	configure_evm_pin_mux(board_id, profile, daughter_board_connected);

#ifdef CONFIG_AM335X_MIN_CONFIG
	board_min_init();
#else
	board_evm_init();
#endif

	gpmc_init();

	return 0;
}

/* Display the board info */
int checkboard(void)
{
	return 0;
}

/*
 * Reset the CPU
 */
void reset_cpu(ulong addr)
{
	addr = __raw_readl(PRM_DEVICE_RSTCTRL);
	addr &= ~BIT(1);
	addr |= BIT(1);
	__raw_writel(addr, PRM_DEVICE_RSTCTRL);
}

