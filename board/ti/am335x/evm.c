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
#include <spi_flash.h>
#include "common_def.h"
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

/* UART Defines */
#define UART_RESET		(0x1 << 1)
#define UART_CLK_RUNNING_MASK	0x1
#define UART_SMART_IDLE_EN	(0x1 << 0x3)

/* Timer Defines */
#define TSICR_REG		0x54
#define TIOCP_CFG_REG		0x10
#define TCLR_REG		0x38

/* CPLD registers */
#define CFG_REG			0x10

/*
 * I2C Address of various board
 */
#define I2C_BASE_BOARD_ADDR	0x50
#define I2C_DAUGHTER_BOARD_ADDR 0x51
#define I2C_LCD_BOARD_ADDR	0x52

#define I2C_CPLD_ADDR		0x35

/* RGMII mode define */
#define RGMII_MODE_ENABLE	0xA

/* TLK110 PHY registers */
#define TLK110_COARSEGAIN_REG	0x00A3
#define TLK110_LPFHPF_REG	0x00AC
#define TLK110_SPAREANALOG_REG	0x00B9
#define TLK110_VRCR_REG		0x00D0
#define TLK110_SETFFE_REG	0x0107
#define TLK110_FTSP_REG		0x0154
#define TLK110_ALFATPIDL_REG	0x002A
#define TLK110_PSCOEF21_REG	0x0096
#define TLK110_PSCOEF3_REG	0x0097
#define TLK110_ALFAFACTOR1_REG	0x002C
#define TLK110_ALFAFACTOR2_REG	0x0023
#define TLK110_CFGPS_REG	0x0095
#define TLK110_FTSPTXGAIN_REG	0x0150
#define TLK110_SWSCR3_REG	0x000B
#define TLK110_SCFALLBACK_REG	0x0040
#define TLK110_PHYRCR_REG	0x001F

/* TLK110 register writes values */
#define TLK110_COARSEGAIN_VAL	0x0000
#define TLK110_LPFHPF_VAL	0x8000
#define TLK110_SPAREANALOG_VAL	0x0000
#define TLK110_VRCR_VAL		0x0008
#define TLK110_SETFFE_VAL	0x0605
#define TLK110_FTSP_VAL		0x0255
#define TLK110_ALFATPIDL_VAL	0x7998
#define TLK110_PSCOEF21_VAL	0x3A20
#define TLK110_PSCOEF3_VAL	0x003F
#define TLK110_ALFAFACTOR1_VAL	0xFF80
#define TLK110_ALFAFACTOR2_VAL	0x021C
#define TLK110_CFGPS_VAL	0x0000
#define TLK110_FTSPTXGAIN_VAL	0x6A88
#define TLK110_SWSCR3_VAL	0x0000
#define TLK110_SCFALLBACK_VAL	0xC11D
#define TLK110_PHYRCR_VAL	0x4000
#define TLK110_PHYIDR1		0x2000
#define TLK110_PHYIDR2		0xA201

struct dghtr_brd_id_hdr {
	unsigned int  header;
	char board_name[8];
	char version[4];
	char serial_num[12];
	unsigned char config[32];
};

#define NO_OF_MAC_ADDR          3
#define ETH_ALEN		6
struct basebrd_id_hdr {
	unsigned int  header;
	char board_name[8];
	char version[4];
	char serial_num[12];
	unsigned char config[32];
	unsigned char mac_addr[NO_OF_MAC_ADDR][ETH_ALEN];
};

static struct basebrd_id_hdr brd_id_hdr;

static struct dghtr_brd_id_hdr db_header[NO_OF_DAUGHTER_BOARDS] = {
	{
	.header = 0xAA5533EE,
	.board_name = "A335GPBD", /* General purpose daughter board */
	.version = "1.0A",
	},
	{
	.header = 0xAA5533EE,
	.board_name = "A335IAMC", /* Industrial automation board */
	.version = "1.0A",

	},
	{
	.header = 0xAA5533EE,
	.board_name = "A335IPPH", /* IP Phone daughter board */
	.version = "1.0A",
	}
};

extern void cpsw_eth_set_mac_addr(const u_int8_t *addr);

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
 * Daughter board detection: All boards have i2c based EEPROM in all the
 * profiles. Base boards and daughter boards are assigned unique I2C Address.
 * We probe for daughter card and then if sucessful, read the EEPROM to find
 * daughter card type.
 */
static void detect_daughter_board(void)
{
	struct dghtr_brd_id_hdr st_dghtr_brd_id_hdr;
	unsigned char db_board_id = GP_DAUGHTER_BOARD;

	/* Check if daughter board is conneted */
	if (i2c_probe(I2C_DAUGHTER_BOARD_ADDR))
		return;

	/* read the eeprom using i2c */
	if (i2c_read(I2C_DAUGHTER_BOARD_ADDR, 0, 1, (uchar *)
		(&st_dghtr_brd_id_hdr), sizeof(struct dghtr_brd_id_hdr)))
		return;

	do {
		if (!strncmp(db_header[db_board_id].board_name,
				st_dghtr_brd_id_hdr.board_name, 8)) {
			daughter_board_id = db_board_id;
			break;
		}
	} while (++db_board_id < NO_OF_DAUGHTER_BOARDS);
}

unsigned char get_daughter_board_id(void)
{
	return daughter_board_id;
}

static unsigned char evm_profile = PROFILE_0;
static void detect_daughter_board_profile(void)
{
	unsigned short val;

	if (i2c_probe(I2C_CPLD_ADDR))
		return;

	if (i2c_read(I2C_CPLD_ADDR, CFG_REG, 1, (unsigned char *)(&val), 2))
		return;

	evm_profile = 1 << (val & 0x7);
}

unsigned char get_daughter_board_profile(void)
{
	return evm_profile;
}

/*
 * Basic board specific setup
 */
#ifdef CONFIG_AM335X_MIN_CONFIG
int board_min_init(void)
{
	u32 regVal;

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

	/* Initialize the Timer */
	init_timer();

	return 0;
}
#else
int board_evm_init(void)
{
	/* mach type passed to kernel */
	gd->bd->bi_arch_number = MACH_TYPE_TIAM335EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_DRAM_1 + 0x100;

	return 0;
}
#endif

int board_init(void)
{
	char sku_config[7];
	unsigned int dghtr_brd_valid;

	/* Configure the i2c0 pin mux */
	enable_i2c0_pin_mux();

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	/* Check if baseboard eeprom is available */
	if (i2c_probe(I2C_BASE_BOARD_ADDR))
		return 1; /* something is seriously wrong */

	/* read the eeprom using i2c */
	if (i2c_read(I2C_BASE_BOARD_ADDR, 0, 1, (uchar *)
		(&brd_id_hdr), sizeof(struct basebrd_id_hdr)))
		return 1;	/* something is seriously wrong */

	detect_daughter_board();

	if ((daughter_board_id == GP_DAUGHTER_BOARD) ||
				(daughter_board_id == IA_DAUGHTER_BOARD))
		detect_daughter_board_profile();

	memcpy(sku_config, brd_id_hdr.config, 6);
	sku_config[6] = '\0';

	/*
	* If any daughter board is already detected, daughter board detection
	* logic	detect_daughter_board() would have changed
	* daughter_board_id to appropriate evm id. If not set means no daughter
	* board is detected.
	*/
	if (daughter_board_id != BASE_BOARD_ONLY)
		dghtr_brd_valid = 1;

	if (!strncmp("SKU#00", sku_config, 6) &&
			(daughter_board_id == BASE_BOARD_ONLY))
		configure_evm_pin_mux(BASE_BOARD_ONLY, PROFILE_NONE,
							dghtr_brd_valid);
	else if (!strncmp("SKU#01", sku_config, 6) &&
			(daughter_board_id == GP_DAUGHTER_BOARD))
		configure_evm_pin_mux(GP_DAUGHTER_BOARD, evm_profile,
							dghtr_brd_valid);
	else if (!strncmp("SKU#02", sku_config, 6) &&
			(daughter_board_id == IA_DAUGHTER_BOARD))
		configure_evm_pin_mux(IA_DAUGHTER_BOARD, evm_profile,
							dghtr_brd_valid);
	else if (!strncmp("SKU#03", sku_config, 6) &&
			(daughter_board_id == IPP_DAUGHTER_BOARD))
		configure_evm_pin_mux(IPP_DAUGHTER_BOARD, PROFILE_NONE,
							dghtr_brd_valid);
	else
		/*
		printf("AM335X: Invalid configuration. Board %s, sku %s, "
				"evm Id %d\n", brd_name, sku_config,
				daughter_board_id);
		*/

#ifdef CONFIG_AM335X_MIN_CONFIG
	board_min_init();
#else
	board_evm_init();
#endif

	gpmc_init();

	return 0;
}

#ifdef BOARD_LATE_INIT
/*
 * SPI bus number is switched to in case Industrial Automation
 * motor control EVM.
 */
void set_spi_bus_on_board_detect(void){
	if (daughter_board_id == IA_DAUGHTER_BOARD)
		setenv("spi_bus_no", "1");
}

int board_late_init(void){
		set_spi_bus_on_board_detect();
		return 0;
}
#endif

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

#ifdef CONFIG_AM335X_MIN_CONFIG
#ifdef CONFIG_NAND
	if ((daughter_board_id == GP_DAUGHTER_BOARD) &&
		((evm_profile  == PROFILE_2) ||
			(evm_profile == PROFILE_3)))
		printf("NAND boot: Profile setting is wrong!!");
#endif
#ifdef CONFIG_NOR
	if ((daughter_board_id != GP_DAUGHTER_BOARD) ||
		(evm_profile != PROFILE_3))
		printf("NOR boot: Profile setting is wrong!!");
#endif
#endif
	return 0;
}

#ifdef CONFIG_DRIVER_TI_CPSW
/* TODO : Check for the board specific PHY */
static void evm_phy_init(char *name, int addr)
{
	unsigned short val;
	unsigned int cntr = 0;
	unsigned short phyid1, phyid2;

	/* This is done as a workaround to support TLK110 rev1.0 phy */
	/* Reading phy identification register 1 */
	if (miiphy_read(name, addr, MII_PHYSID1, &phyid1) != 0) {
		printf("failed to read phyid1\n");
		return;
	}

	/* Reading phy identification register 2 */
	if (miiphy_read(name, addr, MII_PHYSID2, &phyid2) != 0) {
		printf("failed to read phyid2\n");
		return;
	}
	if ((phyid1 == TLK110_PHYIDR1) && (phyid2 == TLK110_PHYIDR2)) {
		miiphy_read(name, addr, TLK110_COARSEGAIN_REG, &val);
		val |= TLK110_COARSEGAIN_VAL;
		miiphy_write(name, addr, TLK110_COARSEGAIN_REG, val);

		miiphy_read(name, addr, TLK110_LPFHPF_REG, &val);
		val |= TLK110_LPFHPF_VAL;
		miiphy_write(name, addr, TLK110_LPFHPF_REG, val);

		miiphy_read(name, addr, TLK110_SPAREANALOG_REG, &val);
		val |= TLK110_SPAREANALOG_VAL;
		miiphy_write(name, addr, TLK110_SPAREANALOG_REG, val);

		miiphy_read(name, addr, TLK110_VRCR_REG, &val);
		val |= TLK110_VRCR_VAL;
		miiphy_write(name, addr, TLK110_VRCR_REG, val);

		miiphy_read(name, addr, TLK110_SETFFE_REG, &val);
		val |= TLK110_SETFFE_VAL;
		miiphy_write(name, addr, TLK110_SETFFE_REG, val);

		miiphy_read(name, addr, TLK110_FTSP_REG, &val);
		val |= TLK110_FTSP_VAL;
		miiphy_write(name, addr, TLK110_FTSP_REG, val);

		miiphy_read(name, addr, TLK110_ALFATPIDL_REG, &val);
		val |= TLK110_ALFATPIDL_VAL;
		miiphy_write(name, addr, TLK110_ALFATPIDL_REG, val);

		miiphy_read(name, addr, TLK110_PSCOEF21_REG, &val);
		val |= TLK110_PSCOEF21_VAL;
		miiphy_write(name, addr, TLK110_PSCOEF21_REG, val);

		miiphy_read(name, addr, TLK110_PSCOEF3_REG, &val);
		val |= TLK110_PSCOEF3_VAL;
		miiphy_write(name, addr, TLK110_PSCOEF3_REG, val);

		miiphy_read(name, addr, TLK110_ALFAFACTOR1_REG, &val);
		val |= TLK110_ALFAFACTOR1_VAL;
		miiphy_write(name, addr, TLK110_ALFAFACTOR1_REG, val);

		miiphy_read(name, addr, TLK110_ALFAFACTOR2_REG, &val);
		val |= TLK110_ALFAFACTOR2_VAL;
		miiphy_write(name, addr, TLK110_ALFAFACTOR2_REG, val);

		miiphy_read(name, addr, TLK110_CFGPS_REG, &val);
		val |= TLK110_CFGPS_VAL;
		miiphy_write(name, addr, TLK110_CFGPS_REG, val);

		miiphy_read(name, addr, TLK110_FTSPTXGAIN_REG, &val);
		val |= TLK110_FTSPTXGAIN_VAL;
		miiphy_write(name, addr, TLK110_FTSPTXGAIN_REG, val);

		miiphy_read(name, addr, TLK110_SWSCR3_REG, &val);
		val |= TLK110_SWSCR3_VAL;
		miiphy_write(name, addr, TLK110_SWSCR3_REG, val);

		miiphy_read(name, addr, TLK110_SCFALLBACK_REG, &val);
		val |= TLK110_SCFALLBACK_VAL;
		miiphy_write(name, addr, TLK110_SCFALLBACK_REG, val);

		miiphy_read(name, addr, TLK110_PHYRCR_REG, &val);
		val |= TLK110_PHYRCR_VAL;
		miiphy_write(name, addr, TLK110_PHYRCR_REG, val);
	}

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

	/* Setup GIG advertisement only if it is not IA board */
	if (daughter_board_id != IA_DAUGHTER_BOARD) {
		miiphy_read(name, addr, MII_CTRL1000, &val);
		val |= PHY_1000BTCR_1000FD;
		val &= ~PHY_1000BTCR_1000HD;
		miiphy_write(name, addr, MII_CTRL1000, val);
		miiphy_read(name, addr, MII_CTRL1000, &val);
	}

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
	u_int32_t i;

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
	}

	if (!is_valid_ether_addr(mac_addr)) {
		/* Read MACID from eeprom if MACID in eFuse is not valid */
		for (i = 0; i < ETH_ALEN; i++)
			mac_addr[i] = brd_id_hdr.mac_addr[0][i];
	}

	if (is_valid_ether_addr(mac_addr))
		/* set the ethaddr variable with MACID detected */
		eth_setenv_enetaddr("ethaddr", mac_addr);
	else
		printf("Caution:using static MACID!! Set <ethaddr> variable\n");

	/* set mii mode to rgmii in in device configure register */
	if (daughter_board_id != IA_DAUGHTER_BOARD)
		__raw_writel(RGMII_MODE_ENABLE, MAC_MII_SEL);

	if (daughter_board_id == IA_DAUGHTER_BOARD) {
		cpsw_slaves[0].phy_id = 1;
		cpsw_slaves[1].phy_id = 0;
	}

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
