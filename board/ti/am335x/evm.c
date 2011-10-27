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
#include <asm/omap_common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/arch/nand.h>
#include <asm/arch/clock.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <net.h>
#include <miiphy.h>
#include <netdev.h>
#include <spi_flash.h>
#include "common_def.h"
#include "pmic.h"
#include "tps65217.h"
#include <i2c.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

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
#define RMII_MODE_ENABLE	0x5
#define MII_MODE_ENABLE		0x0

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

#define NO_OF_MAC_ADDR          3
#define ETH_ALEN		6

struct am335x_baseboard_id {
	unsigned int  magic;
	char name[8];
	char version[4];
	char serial[12];
	char config[32];
	char mac_addr[NO_OF_MAC_ADDR][ETH_ALEN];
};

static struct am335x_baseboard_id header;
extern void cpsw_eth_set_mac_addr(const u_int8_t *addr);
static unsigned char daughter_board_connected;
static volatile int board_id = BASE_BOARD;

/*
 * dram_init:
 * At this point we have initialized the i2c bus and can read the
 * EEPROM which will tell us what board and revision we are on.
 */
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

#ifdef CONFIG_SPL_BUILD
static void Data_Macro_Config(int dataMacroNum)
{
	u32 BaseAddrOffset = 0x00;;

	if (dataMacroNum == 1)
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

/*  void DDR2_EMIF_Config(void); */
static void config_am335x_ddr(void)
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

static void init_timer(void)
{
	/* Reset the Timer */
	__raw_writel(0x2, (DM_TIMER2_BASE + TSICR_REG));

	/* Wait until the reset is done */
	while (__raw_readl(DM_TIMER2_BASE + TIOCP_CFG_REG) & 1);

	/* Start the Timer */
	__raw_writel(0x1, (DM_TIMER2_BASE + TCLR_REG));
}
#endif

/*
 * Read header information from EEPROM into global structure.
 */
int read_eeprom(void)
{
        /* Check if baseboard eeprom is available */
	if (i2c_probe(I2C_BASE_BOARD_ADDR)) {
		printf("Could not probe the EEPROM; something fundamentally "
			"wrong on the I2C bus.\n");
		return 1;
	}

	/* read the eeprom using i2c */
	if (i2c_read(I2C_BASE_BOARD_ADDR, 0, 2, (uchar *)&header,
							sizeof(header))) {
		printf("Could not read the EEPROM; something fundamentally"
			" wrong on the I2C bus.\n");
		return 1;
	}

	if (header.magic != 0xEE3355AA) {
		/* read the eeprom using i2c again, but use only a 1 byte address */
		if (i2c_read(I2C_BASE_BOARD_ADDR, 0, 1, (uchar *)&header,
								sizeof(header))) {
			printf("Could not read the EEPROM; something fundamentally"
				" wrong on the I2C bus.\n");
			return 1;
		}

		if (header.magic != 0xEE3355AA) {
			printf("Incorrect magic number in EEPROM\n");
			return 1;
		}
	}
	return 0;
}

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_BOARD_INIT)
/**
 *  tps65217_reg_write() - Generic function that can write a TPS65217 PMIC
 *                         register or bit field regardless of protection
 *                         level.
 *
 *  @prot_level:        Register password protection.
 *                      use PROT_LEVEL_NONE, PROT_LEVEL_1, or PROT_LEVEL_2
 *  @dest_reg:          Register address to write.
 *  @dest_val:          Value to write.
 *  @mask:              Bit mask (8 bits) to be applied.  Function will only
 *                      change bits that are set in the bit mask.
 *
 *  @return:            0 for success, 1 for failure.
 */
int tps65217_reg_write(uchar prot_level, uchar dest_reg,
        uchar dest_val, uchar mask)
{
        uchar read_val;
        uchar xor_reg;

        /* if we are affecting only a bit field, read dest_reg and apply the mask */
        if (mask != MASK_ALL_BITS) {
                if (i2c_read(TPS65217_CHIP_PM, dest_reg, 1, &read_val, 1))
                        return 1;
                read_val &= (~mask);
                read_val |= (dest_val & mask);
                dest_val = read_val;
        }

        if (prot_level > 0) {
                xor_reg = dest_reg ^ PASSWORD_UNLOCK;
                if (i2c_write(TPS65217_CHIP_PM, PASSWORD, 1, &xor_reg, 1))
                        return 1;
        }

        if (i2c_write(TPS65217_CHIP_PM, dest_reg, 1, &dest_val, 1))
                return 1;

        if (prot_level == PROT_LEVEL_2) {
                if (i2c_write(TPS65217_CHIP_PM, PASSWORD, 1, &xor_reg, 1))
                        return 1;

                if (i2c_write(TPS65217_CHIP_PM, dest_reg, 1, &dest_val, 1))
                        return 1;
        }

        return 0;
}

/**
 *  tps65217_voltage_update() - Controls output voltage setting for the DCDC1,
 *       DCDC2, or DCDC3 control registers in the PMIC.
 *
 *  @dc_cntrl_reg:      DCDC Control Register address.
 *                      Must be DEFDCDC1, DEFDCDC2, or DEFDCDC3.
 *  @volt_sel:          Register value to set.  See PMIC TRM for value set.
 *
 *  @return:            0 for success, 1 for failure.
 */
int tps65217_voltage_update(unsigned char dc_cntrl_reg, unsigned char volt_sel)
{
        if ((dc_cntrl_reg != DEFDCDC1) && (dc_cntrl_reg != DEFDCDC2)
                && (dc_cntrl_reg != DEFDCDC3))
                return 1;

        /* set voltage level */
        if (tps65217_reg_write(PROT_LEVEL_2, dc_cntrl_reg, volt_sel, MASK_ALL_BITS))
                return 1;

        /* set GO bit to initiate voltage transition */
        if (tps65217_reg_write(PROT_LEVEL_2, DEFSLEW, DCDC_GO, DCDC_GO))
                return 1;

        return 0;
}

/*
 * MPU voltage switching for MPU frequency switching.
 */
int mpu_voltage_update(unsigned char vdd1_op_vol_sel)
{
	uchar buf[4];

	/* Select VDD1 OP   */
	if (i2c_read(PMIC_SR_I2C_ADDR, PMIC_VDD1_OP_REG, 1, buf, 1))
		return 1;

	buf[0] &= ~PMIC_OP_REG_CMD_MASK;

	if (i2c_write(PMIC_SR_I2C_ADDR, PMIC_VDD1_OP_REG, 1, buf, 1))
		return 1;

	/* Configure VDD1 OP  Voltage */
	if (i2c_read(PMIC_SR_I2C_ADDR, PMIC_VDD1_OP_REG, 1, buf, 1))
		return 1;

	buf[0] &= ~PMIC_OP_REG_SEL_MASK;
	buf[0] |= vdd1_op_vol_sel;

	if (i2c_write(PMIC_SR_I2C_ADDR, PMIC_VDD1_OP_REG, 1, buf, 1))
		return 1;

	if (i2c_read(PMIC_SR_I2C_ADDR, PMIC_VDD1_OP_REG, 1, buf, 1))
		return 1;

	if ((buf[0] & PMIC_OP_REG_SEL_MASK ) != vdd1_op_vol_sel)
		return 1;

	return 0;
}

void spl_board_init(void)
{
	/* Configure the i2c0 pin mux */
	enable_i2c0_pin_mux();

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	if (read_eeprom()) {
		printf("read_eeprom() failure\n");
		return;
	}

	if (!strncmp("A335BONE", header.name, 8)) {
		/* BeagleBone PMIC Code */
		if (i2c_probe(TPS65217_CHIP_PM))
			return;

		/* Only perform PMIC configurations if board rev > A1 */
		if (!strncmp(header.version, "00A1", 4))
			return;

		/* Set LDO3, LDO4 output voltage to 3.3V */
		if (tps65217_reg_write(PROT_LEVEL_2, DEFLS1,
				       LDO_VOLTAGE_OUT_3_3, LDO_MASK))
			printf("tps65217_reg_write failure\n");

		if (tps65217_reg_write(PROT_LEVEL_2, DEFLS2,
				       LDO_VOLTAGE_OUT_3_3, LDO_MASK))
			printf("tps65217_reg_write failure\n");

		/* Increase USB current limit to 1300mA */
		if (tps65217_reg_write(PROT_LEVEL_NONE, POWER_PATH,
				       USB_INPUT_CUR_LIMIT_1300MA,
				       USB_INPUT_CUR_LIMIT_MASK))
			printf("tps65217_reg_write failure\n");

		/* Set DCDC2 (MPU) voltage to 1.275V */
		if (!tps65217_voltage_update(DEFDCDC2,
					     DCDC_VOLT_SEL_1275MV)) {
			/* Set MPU Frequency to 720MHz */
			mpu_pll_config(MPUPLL_M_720);
		} else {
			printf("tps65217_voltage_update failure\n");
		}
	} else {
		/* 
		 * EVM PMIC code.  PMIC voltage is configuring for frequency
		 * scaling.
		 */
	        if (!i2c_probe(PMIC_SR_I2C_ADDR)) {
		        if (!mpu_voltage_update(PMIC_OP_REG_SEL_1_2_6)) {
			        /* Frequency switching for OPP 120 */
			        mpu_pll_config(MPUPLL_M_720);
		        }
	        }
	}
}
#endif

/*
 * early system init of muxing and clocks.
 */
void s_init(void)
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

#ifdef CONFIG_SPL_BUILD
	/* Setup the PLLs and the clocks for the peripherals */
	pll_init();

	/* UART softreset */
	u32 regVal;
	u32 uart_base = DEFAULT_UART_BASE;

	enable_uart0_pin_mux();
	/* IA Motor Control Board has default console on UART3*/
	/* XXX: This is before we've probed / set board_id */
	if (board_id == IA_BOARD) {
		uart_base = UART3_BASE;
	}

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

	preloader_console_init();

	config_am335x_ddr();
#endif
}

static void detect_daughter_board(void)
{
	/* Check if daughter board is conneted */
	if (i2c_probe(I2C_DAUGHTER_BOARD_ADDR)) {
		printf("No daughter card present\n");
		return;
	} else {
		printf("Found a daughter card connected\n");
		daughter_board_connected = 1;
	}
}

static unsigned char profile = PROFILE_0;
static void detect_daughter_board_profile(void)
{
	unsigned short val;

	if (i2c_probe(I2C_CPLD_ADDR))
		return;

	if (i2c_read(I2C_CPLD_ADDR, CFG_REG, 1, (unsigned char *)(&val), 2))
		return;

	profile = 1 << (val & 0x7);
}

/*
 * Basic board specific setup
 */
#ifndef CONFIG_SPL_BUILD
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

#if 0
struct serial_device *default_serial_console(void)
{

	if (board_id != IA_BOARD) {
		return &eserial1_device;	/* UART0 */
	} else {
		return &eserial4_device;	/* UART3 */
	}
}
#endif

int board_init(void)
{
	/* Configure the i2c0 pin mux */
	enable_i2c0_pin_mux();

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	if (read_eeprom())
		goto err_out;

	detect_daughter_board();

	if (!strncmp("SKU#01", header.config, 6)) {
		board_id = GP_BOARD;
		detect_daughter_board_profile();
	} else if (!strncmp("SKU#02", header.config, 6)) {
		board_id = IA_BOARD;
		detect_daughter_board_profile();
	} else if (!strncmp("SKU#03", header.config, 6)) {
		board_id = IPP_BOARD;
	} else if (!strncmp("A335BONE", header.name, 8)) {
		board_id = BONE_BOARD;
		profile = 1;	/* profile 0 is internally considered as 1 */
		daughter_board_connected = 0;
	} else {
		printf("Did not find a recognized configuration, "
			"assuming General purpose EVM in Profile 0 with "
			"Daughter board\n");
		board_id = GP_BOARD;
		profile = 1;	/* profile 0 is internally considered as 1 */
		daughter_board_connected = 1;
	}

	configure_evm_pin_mux(board_id, header.version, profile, daughter_board_connected);

#ifndef CONFIG_SPL_BUILD
	board_evm_init();
#endif

	gpmc_init();

	return 0;

err_out:
	/*
	 * When we cannot use the EEPROM to determine what board we
	 * are we assume BeagleBone currently as we have not yet
	 * programmed the EEPROMs.
	 */
	board_id = BONE_BOARD;
	profile = 1;	/* profile 0 is internally considered as 1 */
	daughter_board_connected = 1;
	configure_evm_pin_mux(board_id, header.version, profile, daughter_board_connected);

#ifndef CONFIG_SPL_BUILD
	board_evm_init();
#endif

	gpmc_init();

	return 0;
}

int misc_init_r(void)
{
#ifdef DEBUG
	unsigned int cntr;
	unsigned char *valPtr;

	debug("EVM Configuration - ");
	debug("\tBoard id %x, profile %x, db %d\n", board_id, profile,
						daughter_board_connected);
	debug("Base Board EEPROM Data\n");
	valPtr = (unsigned char *)&header;
	for(cntr = 0; cntr < sizeof(header); cntr++) {
		if(cntr % 16 == 0)
			debug("\n0x%02x :", cntr);
		debug(" 0x%02x", (unsigned int)valPtr[cntr]);
	}
	debug("\n\n");

	debug("Board identification from EEPROM contents:\n");
	debug("\tBoard name   : %.8s\n", header.name);
	debug("\tBoard version: %.4s\n", header.version);
	debug("\tBoard serial : %.12s\n", header.serial);
	debug("\tBoard config : %.6s\n\n", header.config);
#endif
	return 0;
}

#ifdef BOARD_LATE_INIT
int board_late_init(void)
{
	if (board_id == IA_BOARD) {
		/*
		* SPI bus number is switched to in case Industrial Automation
		* motor control EVM.
		*/
		setenv("spi_bus_no", "1");
		/* Change console to tty03 for IA Motor Control EVM */
		setenv("console", "ttyO3,115200n8");
	}

	return 0;
}
#endif

#ifdef CONFIG_DRIVER_TI_CPSW
/* TODO : Check for the board specific PHY */
static void evm_phy_init(char *name, int addr)
{
	unsigned short val;
	unsigned int cntr = 0;
	unsigned short phyid1, phyid2;
	int bone_pre_a3 = 0;

	if (board_id == BONE_BOARD && (!strncmp(header.version, "00A1", 4) ||
		    !strncmp(header.version, "00A2", 4)))
		bone_pre_a3 = 1;

	/*
	 * This is done as a workaround to support TLK110 rev1.0 PHYs.
	 * We can only perform these reads on these PHYs (currently
	 * only found on the IA EVM).
	 */
	if ((miiphy_read(name, addr, MII_PHYSID1, &phyid1) != 0) ||
			(miiphy_read(name, addr, MII_PHYSID2, &phyid2) != 0))
		return;

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

	if (bone_pre_a3) {
		val &= ~(BMCR_FULLDPLX | BMCR_ANENABLE | BMCR_SPEED100);
		val |= BMCR_FULLDPLX;
	} else
		val |= BMCR_FULLDPLX | BMCR_ANENABLE | BMCR_SPEED100;

	if (miiphy_write(name, addr, MII_BMCR, val) != 0) {
		printf("failed to write bmcr\n");
		return;
	}
	miiphy_read(name, addr, MII_BMCR, &val);

	/* TODO: Disable GIG advertisement for the time being */
	if (board_id != IA_BOARD && board_id != BONE_BOARD) {
		miiphy_read(name, addr, MII_CTRL1000, &val);
		val &= ~PHY_1000BTCR_1000FD;
		val &= ~PHY_1000BTCR_1000HD;
		miiphy_write(name, addr, MII_CTRL1000, val);
		miiphy_read(name, addr, MII_CTRL1000, &val);
	}

	/* Setup general advertisement */
	if (miiphy_read(name, addr, MII_ADVERTISE, &val) != 0) {
		printf("failed to read anar\n");
		return;
	}

	if (bone_pre_a3)
		val |= (LPA_10HALF | LPA_10FULL);
	else
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
		debug("<ethaddr> not set. Reading from E-fuse\n");
		/* try reading mac address from efuse */
		mac_lo = __raw_readl(MAC_ID0_LO);
		mac_hi = __raw_readl(MAC_ID0_HI);
		mac_addr[0] = mac_hi & 0xFF;
		mac_addr[1] = (mac_hi & 0xFF00) >> 8;
		mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
		mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
		mac_addr[4] = mac_lo & 0xFF;
		mac_addr[5] = (mac_lo & 0xFF00) >> 8;

		if (!is_valid_ether_addr(mac_addr)) {
			debug("Did not find a valid mac address in e-fuse. "
					"Trying the one present in EEPROM\n");

			for (i = 0; i < ETH_ALEN; i++)
				mac_addr[i] = header.mac_addr[0][i];
		}

		if (is_valid_ether_addr(mac_addr))
			eth_setenv_enetaddr("ethaddr", mac_addr);
		else {
			printf("Caution: Using hardcoded mac address. "
				"Set <ethaddr> variable to overcome this.\n");
		}
	}

	if (board_id == BONE_BOARD) {
		/* For beaglebone > Rev A2 , enable MII mode, for others enable RMII */
		if (!strncmp(header.version, "00A1", 4) ||
		    !strncmp(header.version, "00A2", 4))
			__raw_writel(RMII_MODE_ENABLE, MAC_MII_SEL);
		else
			__raw_writel(MII_MODE_ENABLE, MAC_MII_SEL);
	} else if (board_id == IA_BOARD) {
		cpsw_slaves[0].phy_id = 30;
		cpsw_slaves[1].phy_id = 0;
	} else {
		/* set mii mode to rgmii in in device configure register */
		__raw_writel(RGMII_MODE_ENABLE, MAC_MII_SEL);
	}

	return cpsw_register(&cpsw_data);
}
#endif

#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0);
	return 0;
}
#endif

#ifdef CONFIG_NAND_TI81XX
/******************************************************************************
 * Command to switch between NAND HW and SW ecc
 *****************************************************************************/
extern void ti81xx_nand_switch_ecc(nand_ecc_modes_t hardware, int32_t mode);
static int do_switch_ecc(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
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
#endif /* CONFIG_SPL_BUILD */
