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

#include <common.h>
#include <asm/cache.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

#define __raw_readl(a)		(*(volatile unsigned int *)(a))
#define __raw_writel(v, a)	(*(volatile unsigned int *)(a) = (v))
#define __raw_readw(a)		(*(volatile unsigned short *)(a))
#define __raw_writew(v, a)	(*(volatile unsigned short *)(a) = (v))

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SETUP_PLL
static void pll_config(u32, u32, u32, u32, u32);
static void pcie_pll_config(void);
static void sata_pll_config(void);
static void modena_pll_config(void);
static void l3_pll_config(void);
static void ddr_pll_config(void);
static void usb_pll_config(void);
#endif

static void unlock_pll_control_mmr(void);
/*
 * spinning delay to use before udelay works
 */
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b" : "=r" (loops) : "0"(loops));
}

/*
 * Basic board specific setup
 */
int board_init(void)
{
	u32 regVal;

	/* Get Timer and UART out of reset */

	/* UART softreset */
	regVal = __raw_readl(UART_SYSCFG);
	regVal |= 0x2;
	__raw_writel(regVal, UART_SYSCFG);
	while( (__raw_readl(UART_SYSSTS) & 0x1) != 0x1);

	/* Disable smart idle */
	regVal = __raw_readl(UART_SYSCFG);
	regVal |= (1<<3);
	__raw_writel(regVal, UART_SYSCFG);

	gd->bd->bi_arch_number = MACH_TYPE_TI8168EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_DRAM_1 + 0x100;

	return 0;
}

/*
 * Configure DRAM banks
 *
 * Description: sets uboots idea of sdram size
 */
int dram_init(void)
{
	/* Fill up board info */
	gd->bd->bi_dram[0].start = PHYS_DRAM_1;
	gd->bd->bi_dram[0].size = PHYS_DRAM_1_SIZE;

	gd->bd->bi_dram[1].start = PHYS_DRAM_2;
	gd->bd->bi_dram[1].size = PHYS_DRAM_2_SIZE;

	return 0;
}


/*************************************************************
 *  get_device_type(): tell if GP/HS/EMU/TST
 *************************************************************/
u32 get_device_type(void)
{
	int mode;
	mode = __raw_readl(CONTROL_STATUS) & (DEVICE_MASK);
	return(mode >>= 8);
}

/************************************************
 * get_sysboot_value(void) - return SYS_BOOT[4:0]
 ************************************************/
u32 get_sysboot_value(void)
{
	int mode;
	mode = __raw_readl(CONTROL_STATUS) & (SYSBOOT_MASK);
	return mode;
}

#ifdef CONFIG_TI814X_EVM_DDR2
/* assume delay is aprox at least 1us */
void ddr_delay(int d)
{  /*
   * in interactive mode use a user response
   * printf("Pause... Press enter to continue\n");
   * getchar();
   */
  int i;

  /*
   * read a control module register.
   * this is a bit more delay and cannot be optimized by the compiler
   * assuming one read takes 200 cycles and A8 is runing 1 GHz
   * somewhat conservative setting
   */
  for(i=0; i<50*d; i++)
    RD_MEM_32(CONTROL_STATUS);
}

static void ddr_init_settings(int emif)
{
	/* DLL Lockdiff */
	__raw_writel(0xF, (DDRPHY_CONFIG_BASE + 0x028));
	__raw_writel(0xF, (DDRPHY_CONFIG_BASE + 0x05C));
	__raw_writel(0xF, (DDRPHY_CONFIG_BASE + 0x090));
	__raw_writel(0xF, (DDRPHY_CONFIG_BASE + 0x138));
	__raw_writel(0xF, (DDRPHY_CONFIG_BASE + 0x1DC));
	__raw_writel(0xF, (DDRPHY_CONFIG_BASE + 0x280));
	__raw_writel(0xF, (DDRPHY_CONFIG_BASE + 0x324));

	/* setup rank delays */
	__raw_writel(0x1, (DDRPHY_CONFIG_BASE + 0x134));
	__raw_writel(0x1, (DDRPHY_CONFIG_BASE + 0x1D8));
	__raw_writel(0x1, (DDRPHY_CONFIG_BASE + 0x27C));
	__raw_writel(0x1, (DDRPHY_CONFIG_BASE + 0x320));


	__raw_writel(INVERT_CLK_OUT, (DDRPHY_CONFIG_BASE + 0x02C));	/* invert_clk_out cmd0 */
	__raw_writel(INVERT_CLK_OUT, (DDRPHY_CONFIG_BASE + 0x060));	/* invert_clk_out cmd0 */
	__raw_writel(INVERT_CLK_OUT, (DDRPHY_CONFIG_BASE + 0x094));	/* invert_clk_out cmd0 */


	__raw_writel(CMD_SLAVE_RATIO, (DDRPHY_CONFIG_BASE + 0x01C));	/* cmd0 slave ratio */
	__raw_writel(CMD_SLAVE_RATIO, (DDRPHY_CONFIG_BASE + 0x050));	/* cmd0 slave ratio */
	__raw_writel(CMD_SLAVE_RATIO, (DDRPHY_CONFIG_BASE + 0x084));	/* cmd0 slave ratio */

	__raw_writel(DQS_GATE_BYTE_LANE0, (DDRPHY_CONFIG_BASE + 0x108));
	__raw_writel(0x00000000, (DDRPHY_CONFIG_BASE + 0x10C));
	__raw_writel(DQS_GATE_BYTE_LANE1, (DDRPHY_CONFIG_BASE + 0x1AC));
	__raw_writel(0x00000000, (DDRPHY_CONFIG_BASE + 0x1B0));
	__raw_writel(DQS_GATE_BYTE_LANE2, (DDRPHY_CONFIG_BASE + 0x250));
	__raw_writel(0x00000000, (DDRPHY_CONFIG_BASE + 0x254));
	__raw_writel(DQS_GATE_BYTE_LANE3, (DDRPHY_CONFIG_BASE + 0x2F4));
	__raw_writel(0x00000000, (DDRPHY_CONFIG_BASE + 0x2F8));

	__raw_writel(WR_DQS_RATIO_BYTE_LANE0, (DDRPHY_CONFIG_BASE + 0x0DC));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x0E0));
	__raw_writel(WR_DQS_RATIO_BYTE_LANE1, (DDRPHY_CONFIG_BASE + 0x180));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x184));
	__raw_writel(WR_DQS_RATIO_BYTE_LANE2, (DDRPHY_CONFIG_BASE + 0x224));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x228));
	__raw_writel(WR_DQS_RATIO_BYTE_LANE3, (DDRPHY_CONFIG_BASE + 0x2C8));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x2CC));

	__raw_writel(WR_DATA_RATIO_BYTE_LANE0, (DDRPHY_CONFIG_BASE + 0x120));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x124));
	__raw_writel(WR_DATA_RATIO_BYTE_LANE1, (DDRPHY_CONFIG_BASE + 0x1C4));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x1C8));
	__raw_writel(WR_DATA_RATIO_BYTE_LANE2, (DDRPHY_CONFIG_BASE + 0x268));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x26C));
	__raw_writel(WR_DATA_RATIO_BYTE_LANE3, (DDRPHY_CONFIG_BASE + 0x30C));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x310));

	__raw_writel(RD_DQS_RATIO, (DDRPHY_CONFIG_BASE + 0x0C8));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x0CC));
	__raw_writel(RD_DQS_RATIO, (DDRPHY_CONFIG_BASE + 0x16C));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x170));
	__raw_writel(RD_DQS_RATIO, (DDRPHY_CONFIG_BASE + 0x210));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x214));
	__raw_writel(RD_DQS_RATIO, (DDRPHY_CONFIG_BASE + 0x2B4));
	__raw_writel(0x0, (DDRPHY_CONFIG_BASE + 0x2B8));

	__raw_writel(0x5, (DDRPHY_CONFIG_BASE + 0x00C));
	__raw_writel(0x5, (DDRPHY_CONFIG_BASE + 0x010));
	__raw_writel(0x5, (DDRPHY_CONFIG_BASE + 0x040));
	__raw_writel(0x5, (DDRPHY_CONFIG_BASE + 0x044));
	__raw_writel(0x5, (DDRPHY_CONFIG_BASE + 0x074));
	__raw_writel(0x5, (DDRPHY_CONFIG_BASE + 0x078));

	__raw_writel(0x4, (DDRPHY_CONFIG_BASE + 0x0A8));
	__raw_writel(0x4, (DDRPHY_CONFIG_BASE + 0x0AC));
	__raw_writel(0x4, (DDRPHY_CONFIG_BASE + 0x14C));
	__raw_writel(0x4, (DDRPHY_CONFIG_BASE + 0x150));
	__raw_writel(0x4, (DDRPHY_CONFIG_BASE + 0x1F0));
	__raw_writel(0x4, (DDRPHY_CONFIG_BASE + 0x1F4));
	__raw_writel(0x4, (DDRPHY_CONFIG_BASE + 0x294));
	__raw_writel(0x4, (DDRPHY_CONFIG_BASE + 0x298));

	__raw_writel(0x5, (DDRPHY_CONFIG_BASE + 0x338));
	__raw_writel(0x5, (DDRPHY_CONFIG_BASE + 0x340));
	__raw_writel(0x5, (DDRPHY_CONFIG_BASE + 0x348));
	__raw_writel(0x5, (DDRPHY_CONFIG_BASE + 0x350));

}

static void emif4p_init(u32 TIM1, u32 TIM2, u32 TIM3, u32 SDREF, u32 SDCFG, u32 RL)
{
	/*Program EMIF0 CFG Registers*/
	__raw_writel(TIM1, EMIF4_0_SDRAM_TIM_1);
	__raw_writel(TIM1, EMIF4_0_SDRAM_TIM_1_SHADOW);
	__raw_writel(TIM2, EMIF4_0_SDRAM_TIM_2);
	__raw_writel(TIM2, EMIF4_0_SDRAM_TIM_2_SHADOW);
	__raw_writel(TIM3, EMIF4_0_SDRAM_TIM_3);
	__raw_writel(TIM3, EMIF4_0_SDRAM_TIM_3_SHADOW);
	__raw_writel(SDCFG, EMIF4_0_SDRAM_CONFIG);
	//__raw_writel(SDREF, EMIF4_0_SDRAM_REF_CTRL);
	//__raw_writel(SDREF, EMIF4_0_SDRAM_REF_CTRL_SHADOW);
	__raw_writel(RL, EMIF4_0_DDR_PHY_CTRL_1);
	__raw_writel(RL, EMIF4_0_DDR_PHY_CTRL_1_SHADOW);

	if (CONFIG_TI816X_TWO_EMIF){
	__raw_writel(TIM1, EMIF4_1_SDRAM_TIM_1);
	__raw_writel(TIM1, EMIF4_1_SDRAM_TIM_1_SHADOW);
	__raw_writel(TIM2, EMIF4_1_SDRAM_TIM_2);
	__raw_writel(TIM2, EMIF4_1_SDRAM_TIM_2_SHADOW);
	__raw_writel(TIM3, EMIF4_1_SDRAM_TIM_3);
	__raw_writel(TIM3, EMIF4_1_SDRAM_TIM_3_SHADOW);
	__raw_writel(SDCFG, EMIF4_1_SDRAM_CONFIG);
	//__raw_writel(SDREF, EMIF4_0_SDRAM_REF_CTRL);
	//__raw_writel(SDREF, EMIF4_0_SDRAM_REF_CTRL_SHADOW);
	__raw_writel(RL, EMIF4_1_DDR_PHY_CTRL_1);
	__raw_writel(RL, EMIF4_1_DDR_PHY_CTRL_1_SHADOW);
	}

}

static void ddrsetup(void)
{
	/* setup a small control period */
	__raw_writel(0x0000613B, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(0x1000613B, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(0x10000C30, EMIF4_0_SDRAM_REF_CTRL);

	__raw_writel(EMIF_PHYCFG, EMIF4_0_DDR_PHY_CTRL_1);
	__raw_writel(EMIF_PHYCFG, EMIF4_0_DDR_PHY_CTRL_1_SHADOW);

	if (CONFIG_TI816X_TWO_EMIF){
	/* setup a small control period */
	__raw_writel(0x0000613B, EMIF4_1_SDRAM_REF_CTRL);
	__raw_writel(0x1000613B, EMIF4_1_SDRAM_REF_CTRL);
	__raw_writel(0x10000C30, EMIF4_1_SDRAM_REF_CTRL);

	__raw_writel(EMIF_PHYCFG, EMIF4_1_DDR_PHY_CTRL_1);
	__raw_writel(EMIF_PHYCFG, EMIF4_1_DDR_PHY_CTRL_1_SHADOW);
	}

}

static void update_dqs(int emif)
{
}

static void ddr_pll_config()
{
	pll_config(DDR_PLL_BASE,
			DDR_N, DDR_M,
			DDR_M2, DDR_CLKCTRL);
}

/*
 * configure individual ADPLLJ
 */
static void pll_config(u32 base, u32 n, u32 m, u32 m2, u32 clkctrl_val)
{
	__raw_writel(0x2, CM_DEFAULT_L3_FAST_CLKSTCTRL);			/*Enable the Power Domain Transition of L3 Fast Domain Peripheral*/
	__raw_writel(0x2, CM_DEFAULT_EMIF_0_CLKCTRL);				/*Enable EMIF0 Clock*/
	__raw_writel(0x2, CM_DEFAULT_EMIF_1_CLKCTRL); 				/*Enable EMIF1 Clock*/
	while((__raw_readl(CM_DEFAULT_L3_FAST_CLKSTCTRL) & 0x300) != 0x300);	/*Poll for L3_FAST_GCLK  & DDR_GCLK  are active*/
	while((__raw_readl(CM_DEFAULT_EMIF_0_CLKCTRL)) != 0x2);	/*Poll for Module is functional*/
	while((__raw_readl(CM_DEFAULT_EMIF_1_CLKCTRL)) != 0x2);	/*Poll for Module is functional*/

	ddr_init_settings(0);

	if (CONFIG_TI816X_TWO_EMIF){
		ddr_init_settings(1);
	}

	__raw_writel(0x2, CM_DEFAULT_DMM_CLKCTRL); 				/*Enable EMIF1 Clock*/
	while((__raw_readl(CM_DEFAULT_DMM_CLKCTRL)) != 0x2);		/*Poll for Module is functional*/

	/*Program the DMM to Access EMIF0*/
	__raw_writel(0x80640300, DMM_LISA_MAP__0);
	__raw_writel(0xC0640320, DMM_LISA_MAP__1);

	/*Program the DMM to Access EMIF1*/
	__raw_writel(0x80640300, DMM_LISA_MAP__2);
	__raw_writel(0xC0640320, DMM_LISA_MAP__3);

	/*Enable Tiled Access*/
	__raw_writel(0x80000000, DMM_PAT_BASE_ADDR);

	emif4p_init(EMIF_TIM1, EMIF_TIM2, EMIF_TIM3, EMIF_SDREF & 0xFFFFFFFF, EMIF_SDCFG, 0x10B);
	ddrsetup();
	update_dqs(0);
	update_dqs(1);
}
#endif

/*******************************************************
 * Routine: misc_init_r
 ********************************************************/
int misc_init_r (void)
{
	return 0;
}

/*****************************************************************
 * Routine: peripheral_enable
 * Description: Enable the clks & power for perifs (TIMER1, UART0,...)
 *
 ******************************************************************/
static void peripheral_enable(void)
{
	/* DMTimers */
	__raw_writel(0x2, CM_ALWON_L3_SLOW_CLKSTCTRL);

	/* First we need to enable the modules and setup the clk path
	 * Then the timers need to be configured by writing to their registers
	 * To access the timer registers we need the module to be
	 * enabled which is what we do in the first step
	 */

	/* TIMER 1 */
	__raw_writel(0x2, CM_ALWON_TIMER_1_CLKCTRL);

	/* Selects CLKIN (27MHz) */
	__raw_writel(0x2, CM_TIMER1_CLKSEL);

	while(((__raw_readl(CM_ALWON_L3_SLOW_CLKSTCTRL) & (0x80000<<1)) >> (19+1)) != 1);

	while(((__raw_readl(CM_ALWON_TIMER_1_CLKCTRL) & 0x30000)>>16) !=0);


	__raw_writel(0x2,(DM_TIMER1_BASE + 0x54));
	while(__raw_readl(DM_TIMER1_BASE + 0x10) & 1);

	__raw_writel(0x1,(DM_TIMER1_BASE + 0x38));

	/* UARTs */
	__raw_writel(0x2, CM_ALWON_UART_0_CLKCTRL);
	while(__raw_readl(CM_ALWON_UART_0_CLKCTRL) != 0x2);

	__raw_writel(0x2, CM_ALWON_UART_1_CLKCTRL);
	while(__raw_readl(CM_ALWON_UART_1_CLKCTRL) != 0x2);

	__raw_writel(0x2, CM_ALWON_UART_2_CLKCTRL);
	while(__raw_readl(CM_ALWON_UART_2_CLKCTRL) != 0x2);

	while((__raw_readl(CM_ALWON_L3_SLOW_CLKSTCTRL) & 0x2100) != 0x2100);

	/* SPI */
	__raw_writel(0x2, CM_ALWON_SPI_CLKCTRL);
	while(__raw_readl(CM_ALWON_SPI_CLKCTRL) != 0x2);

	/* I2C0 */
	__raw_writel(0x2, CM_ALWON_I2C_0_CLKCTRL);
	while(__raw_readl(CM_ALWON_I2C_0_CLKCTRL) != 0x2);

	/* Ethernet */
	__raw_writel(0x2, CM_ETHERNET_CLKSTCTRL);
	__raw_writel(0x2, CM_ALWON_ETHERNET_0_CLKCTRL);
	__raw_writel(0x2, CM_ALWON_ETHERNET_1_CLKCTRL);



}

/******************************************************************************
 * prcm_init() - inits clocks for PRCM as defined in clocks.h
 *****************************************************************************/
void prcm_init(u32 in_ddr)
{
	/* For future */
	u32 clk_index = 0, sil_index = 0;

	/* Enable the control module */
	__raw_writel(0x2, CM_ALWON_CONTROL_CLKCTRL);

	/* pll init functions come here */

	/*  With clk freqs setup to desired values, enable the required peripherals */
	peripheral_enable();
}

static void unlock_pll_control_mmr(void)
{
	/* ??? */
	__raw_writel(0x1EDA4C3D, 0x481C5040);
	__raw_writel(0x2FF1AC2B, 0x48140060);
	__raw_writel(0xF757FDC0, 0x48140064);
	__raw_writel(0xE2BC3A6D, 0x48140068);
	__raw_writel(0x1EBF131D, 0x4814006c);
	__raw_writel(0x6F361E05, 0x48140070);

}


void cpsw_pad_config(u32 instance)
{
#define PADCTRL_BASE 0x48140000
#define PAD232_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B9C))
#define PAD233_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BA0))
#define PAD234_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BA4))
#define PAD235_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BA8))
#define PAD236_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BAC))
#define PAD237_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BB0))
#define PAD238_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BB4))
#define PAD239_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BB8))
#define PAD240_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BBC))
#define PAD241_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BC0))
#define PAD242_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BC4))
#define PAD243_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BC8))
#define PAD244_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BCC))
#define PAD245_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BD0))
#define PAD246_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BD4))
#define PAD247_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BD8))
#define PAD248_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BDC))
#define PAD249_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BE0))
#define PAD250_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BE4))
#define PAD251_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BE8))
#define PAD252_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BEC))
#define PAD253_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BF0))
#define PAD254_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BF4))
#define PAD255_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BF8))
#define PAD256_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BFC))
#define PAD257_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0C00))
#define PAD258_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0C04))

	if (instance == 0) {
		volatile u32 val = 0;
		val = PAD232_CNTRL;
		PAD232_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD233_CNTRL;
		PAD233_CNTRL = (volatile unsigned int) (BIT(19) | BIT(17) | BIT(0));
		val = PAD234_CNTRL;
		PAD234_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(17) | BIT(0));
		val = PAD235_CNTRL;
		PAD235_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(0));
		val = PAD236_CNTRL;
		PAD236_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(0));
		val = PAD237_CNTRL;
		PAD237_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(0));
		val = PAD238_CNTRL;
		PAD238_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(0));
		val = PAD239_CNTRL;
		PAD239_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(0));
		val = PAD240_CNTRL;
		PAD240_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD241_CNTRL;
		PAD241_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD242_CNTRL;
		PAD242_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD243_CNTRL;
		PAD243_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD244_CNTRL;
		PAD244_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD245_CNTRL;
		PAD245_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD246_CNTRL;
		PAD246_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD247_CNTRL;
		PAD247_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD248_CNTRL;
		PAD248_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD249_CNTRL;
		PAD249_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD250_CNTRL;
		PAD250_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD251_CNTRL;
		PAD251_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD252_CNTRL;
		PAD252_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD253_CNTRL;
		PAD253_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD254_CNTRL;
		PAD254_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD255_CNTRL;
		PAD255_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD256_CNTRL;
		PAD256_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD257_CNTRL;
		PAD257_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD258_CNTRL;
		PAD258_CNTRL = (volatile unsigned int) (BIT(0));
	}
}


/*
 * early system init of muxing and clocks.
 */
void s_init(u32 in_ddr)
{
	l2_cache_enable();		/* Can be removed as A8 comes up with L2 enabled */
	prcm_init(in_ddr);		/* Setup the PLLs and the clocks for the peripherals */
#if CONFIG_TI814X_CONFIG_DDR
	if (!in_ddr)
		config_ti814x_sdram_ddr();	/* Do DDR settings */
#endif
}

/* optionally do something like blinking LED */
void board_hang (void)
{ while (0) {};}

/* Reset the board */
void reset_cpu (ulong addr)
{
	addr = __raw_readl(PRM_DEVICE_RSTCTRL);
	addr &= ~BIT(1);
	addr |= BIT(1);
	__raw_writel(addr, PRM_DEVICE_RSTCTRL);
}

