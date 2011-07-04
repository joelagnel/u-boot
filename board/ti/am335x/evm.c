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
#include <asm/arch/mem.h>
#include <asm/arch/nand.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <net.h>
#include <miiphy.h>
#include <netdev.h>
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

/*
 * I2C Address of eeprom on Daughter cards
 * I2C Addresses are not confirmed yet and
 * will be modifed later
 */
#define I2C_GP_DB_EEPROM_ADDR	0x51
#define I2C_IA_DB_EEPROM_ADDR	0x52
#define I2C_IPP_DB_EEPROM_ADDR	0x52
#define I2C_BB_EEPROM_ADDR	0x50

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
	/* Wait while PHY is ready */
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


static unsigned char daughter_board_id = BASE_BOARD_ONLY;
/*
 * Daughter board detection: All daughter boards have I2C EEPROM in all the
 * profiles. Hence, We probe for the EEPROM to find the daughter board type.
 */
static void detect_daughter_board(void)
{
	/* Configure the i2c0 pin mux */
	enable_i2c0_pin_mux();

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	/* Probe for EEPROM on DBs starting from GP Daughter Board */
	if (i2c_probe(I2C_GP_DB_EEPROM_ADDR) == 0)
		daughter_board_id = GP_DAUGHTER_BOARD;
	else if (i2c_probe(I2C_IA_DB_EEPROM_ADDR) == 0)
		daughter_board_id = IA_DAUGHTER_BOARD;
	else if (i2c_probe(I2C_IPP_DB_EEPROM_ADDR) == 0)
		daughter_board_id = IPP_DAUGHTER_BOARD;
}

unsigned char get_daughter_board_id(void)
{
	return daughter_board_id;
}

unsigned short get_gp_profile(void)
{
	/*
	* TODO : update with CPLD/DIN switch read and profile
	* detection logic
	*/
	return PROFILE_0;
}

unsigned short get_ia_profile(void)
{
	/*
	* TODO : update with CPLD/DIN switch read and profile
	* detection logic
	*/
	return PROFILE_0;
}

/*
 * Basic board specific setup
 */
int board_init(void)
{
	u32 regVal;
	unsigned short profile = PROFILE_NONE;

	detect_daughter_board();

	switch (daughter_board_id) {
	case GP_DAUGHTER_BOARD:
		profile = get_gp_profile();
		break;

	case IA_DAUGHTER_BOARD:
		profile = get_ia_profile();
		break;

	case IPP_DAUGHTER_BOARD:
		profile = PROFILE_0;
		break;
	};

	configure_evm_pin_mux(daughter_board_id, profile);

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

/* Display the board info */
int checkboard(void)
{
	if (daughter_board_id == GP_DAUGHTER_BOARD)
		printf("board: General purpose daughter card connected");
	else if (daughter_board_id == IA_DAUGHTER_BOARD)
		printf("board: IA motor control daughter card connected");
	else if (daughter_board_id == IPP_DAUGHTER_BOARD)
		printf("board: IPP daughter card connected");
	else
		printf("board: No daughter card connected");
}

#ifdef CONFIG_DRIVER_TI_CPSW
/* TODO : Check for the board specific PHY */
static void evm_phy_init(char *name, int addr)
{
	unsigned short val;
	unsigned int cntr = 0;

	/* Enable PHY to clock out TX_CLK */
	/* TODO: PHY config register is not present need to check for TX_CLK
	 * behavior */

	/* Enable Autonegotiation */
	if (miiphy_read(name, addr, MII_BMCR, &val) != 0) {
		printf("failed to read bmcr\n");
		return;
	}
	val |= BMCR_FULLDPLX | BMCR_ANENABLE | BMCR_SPEED100;
	if (miiphy_write(name, addr, MII_BMCR, val) != 0) {
		printf("failed to write bmcr\n");
		return;
	}
	miiphy_read(name, addr, MII_BMCR, &val);

	/* Setup GIG advertisement */
	miiphy_read(name, addr, MII_CTRL1000, &val);
	val |= PHY_1000BTCR_1000FD;
	val &= ~PHY_1000BTCR_1000HD;
	miiphy_write(name, addr, MII_CTRL1000, val);
	miiphy_read(name, addr, MII_CTRL1000, &val);

	/* Setup general advertisement */
	if (miiphy_read(name, addr, MII_ADVERTISE, &val) != 0) {
		printf("failed to read anar\n");
		return;
	}
	val |= (LPA_10HALF | LPA_10FULL | LPA_100HALF | LPA_100FULL);
	if (miiphy_write(name, addr, MII_ADVERTISE, val) != 0) {
		printf("failed to write anar\n");
		return;
	}
	miiphy_read(name, addr, MII_ADVERTISE, &val);

	/* Restart auto negotiation*/
	miiphy_read(name, addr, MII_BMCR, &val);
	val |= BMCR_ANRESTART;
	miiphy_write(name, addr, MII_BMCR, val);

	/*check AutoNegotiate complete - it can take upto 3 secs*/
	do {
		udelay(40000);
		cntr++;
		if (!miiphy_read(name, addr, MII_BMSR, &val)) {
			if (val & BMSR_ANEGCOMPLETE)
				break;
		}
	} while (cntr < 250);

	if (cntr >= 250)
		printf("Auto negotitation failed\n");

	return;
}

static void cpsw_control(int enabled)
{
	/* nothing for now */
	/* TODO : VTP was here before */
	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_id		= 0,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_id		= 1,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= AM335X_CPSW_MDIO_BASE,
	.cpsw_base		= AM335X_CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 2,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.mac_control		= (1 << 5) /* MIIEN */,
	.control		= cpsw_control,
	.phy_init		= evm_phy_init,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

int board_eth_init(bd_t *bis)
{
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;

	if (!eth_getenv_enetaddr("ethaddr", mac_addr)) {
		printf("<ethaddr> not set. Reading from E-fuse\n");
		/* try reading mac address from efuse */
		mac_lo = __raw_readl(MAC_ID0_LO);
		mac_hi = __raw_readl(MAC_ID0_HI);
		mac_addr[0] = mac_hi & 0xFF;
		mac_addr[1] = (mac_hi & 0xFF00) >> 8;
		mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
		mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
		mac_addr[4] = mac_lo & 0xFF;
		mac_addr[5] = (mac_lo & 0xFF00) >> 8;
		/* set the ethaddr variable with MACID detected */
		eth_setenv_enetaddr("ethaddr", mac_addr);
	} else {
		printf("Caution:using static MACID!! Set <ethaddr> variable\n");
	}

	/* set mii mode to rgmii in in device configure register */
	__raw_writel(0x2, MAC_MII_SEL);

	return cpsw_register(&cpsw_data);
}
#endif

#ifdef CONFIG_NAND_TI81XX
/******************************************************************************
 * Command to switch between NAND HW and SW ecc
 *****************************************************************************/
extern void ti81xx_nand_switch_ecc(nand_ecc_modes_t hardware, int32_t mode);
static int do_switch_ecc(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int type = 0;
	if (argc < 2)
		goto usage;

	if (strncmp(argv[1], "hw", 2) == 0) {
		if (argc == 3)
			type = simple_strtoul(argv[2], NULL, 10);
		ti81xx_nand_switch_ecc(NAND_ECC_HW, type);
	}
	else if (strncmp(argv[1], "sw", 2) == 0)
		ti81xx_nand_switch_ecc(NAND_ECC_SOFT, 0);
	else
		goto usage;

	return 0;

usage:
	printf("Usage: nandecc %s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	nandecc, 3, 1,	do_switch_ecc,
	"Switch NAND ECC calculation algorithm b/w hardware and software",
	"[sw|hw <hw_type>] \n"
	"   [sw|hw]- Switch b/w hardware(hw) & software(sw) ecc algorithm\n"
	"   hw_type- 0 for Hamming code\n"
	"            1 for bch4\n"
	"            2 for bch8\n"
	"            3 for bch16\n"
);

#endif /* CONFIG_NAND_TI81XX */
