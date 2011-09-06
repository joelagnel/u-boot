/*
 * mux.c
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
#include <config.h>
#include "common_def.h"
#include <asm/arch/hardware.h>

#define MUX_CFG(value, offset)	\
	__raw_writel(value, (CTRL_BASE + offset));

/* PAD Control Fields */
#define SLEWCTRL	(0x1 << 6)
#define	RXACTIVE	(0x1 << 5)
#define	PULLUP_EN	(0x1 << 4) /* Pull UP Selection */
#define PULLUDEN	(0x0 << 3) /* Pull up enabled */
#define PULLUDDIS	(0x1 << 3) /* Pull up disabled */
#define MODE(val)	val

/*
 * PAD CONTROL OFFSETS
 * Field names corresponds to the pad signal name
 */
struct pad_signals {
	int gpmc_ad0;
	int gpmc_ad1;
	int gpmc_ad2;
	int gpmc_ad3;
	int gpmc_ad4;
	int gpmc_ad5;
	int gpmc_ad6;
	int gpmc_ad7;
	int gpmc_ad8;
	int gpmc_ad9;
	int gpmc_ad10;
	int gpmc_ad11;
	int gpmc_ad12;
	int gpmc_ad13;
	int gpmc_ad14;
	int gpmc_ad15;
	int gpmc_a0;
	int gpmc_a1;
	int gpmc_a2;
	int gpmc_a3;
	int gpmc_a4;
	int gpmc_a5;
	int gpmc_a6;
	int gpmc_a7;
	int gpmc_a8;
	int gpmc_a9;
	int gpmc_a10;
	int gpmc_a11;
	int gpmc_wait0;
	int gpmc_wpn;
	int gpmc_be1n;
	int gpmc_csn0;
	int gpmc_csn1;
	int gpmc_csn2;
	int gpmc_csn3;
	int gpmc_clk;
	int gpmc_advn_ale;
	int gpmc_oen_ren;
	int gpmc_wen;
	int gpmc_be0n_cle;
	int lcd_data0;
	int lcd_data1;
	int lcd_data2;
	int lcd_data3;
	int lcd_data4;
	int lcd_data5;
	int lcd_data6;
	int lcd_data7;
	int lcd_data8;
	int lcd_data9;
	int lcd_data10;
	int lcd_data11;
	int lcd_data12;
	int lcd_data13;
	int lcd_data14;
	int lcd_data15;
	int lcd_vsync;
	int lcd_hsync;
	int lcd_pclk;
	int lcd_ac_bias_en;
	int mmc0_dat3;
	int mmc0_dat2;
	int mmc0_dat1;
	int mmc0_dat0;
	int mmc0_clk;
	int mmc0_cmd;
	int mii1_col;
	int mii1_crs;
	int mii1_rxerr;
	int mii1_txen;
	int mii1_rxdv;
	int mii1_txd3;
	int mii1_txd2;
	int mii1_txd1;
	int mii1_txd0;
	int mii1_txclk;
	int mii1_rxclk;
	int mii1_rxd3;
	int mii1_rxd2;
	int mii1_rxd1;
	int mii1_rxd0;
	int rmii1_refclk;
	int mdio_data;
	int mdio_clk;
	int spi0_sclk;
	int spi0_d0;
	int spi0_d1;
	int spi0_cs0;
	int spi0_cs1;
	int ecap0_in_pwm0_out;
	int uart0_ctsn;
	int uart0_rtsn;
	int uart0_rxd;
	int uart0_txd;
	int uart1_ctsn;
	int uart1_rtsn;
	int uart1_rxd;
	int uart1_txd;
	int i2c0_sda;
	int i2c0_scl;
	int mcasp0_aclkx;
	int mcasp0_fsx;
	int mcasp0_axr0;
	int mcasp0_ahclkr;
	int mcasp0_aclkr;
	int mcasp0_fsr;
	int mcasp0_axr1;
	int mcasp0_ahclkx;
	int xdma_event_intr0;
	int xdma_event_intr1;
	int nresetin_out;
	int porz;
	int nnmi;
	int osc0_in;
	int osc0_out;
	int rsvd1;
	int tms;
	int tdi;
	int tdo;
	int tck;
	int ntrst;
	int emu0;
	int emu1;
	int osc1_in;
	int osc1_out;
	int pmic_power_en;
	int rtc_porz;
	int rsvd2;
	int ext_wakeup;
	int enz_kaldo_1p8v;
	int usb0_dm;
	int usb0_dp;
	int usb0_ce;
	int usb0_id;
	int usb0_vbus;
	int usb0_drvvbus;
	int usb1_dm;
	int usb1_dp;
	int usb1_ce;
	int usb1_id;
	int usb1_vbus;
	int usb1_drvvbus;
	int ddr_resetn;
	int ddr_csn0;
	int ddr_cke;
	int ddr_ck;
	int ddr_nck;
	int ddr_casn;
	int ddr_rasn;
	int ddr_wen;
	int ddr_ba0;
	int ddr_ba1;
	int ddr_ba2;
	int ddr_a0;
	int ddr_a1;
	int ddr_a2;
	int ddr_a3;
	int ddr_a4;
	int ddr_a5;
	int ddr_a6;
	int ddr_a7;
	int ddr_a8;
	int ddr_a9;
	int ddr_a10;
	int ddr_a11;
	int ddr_a12;
	int ddr_a13;
	int ddr_a14;
	int ddr_a15;
	int ddr_odt;
	int ddr_d0;
	int ddr_d1;
	int ddr_d2;
	int ddr_d3;
	int ddr_d4;
	int ddr_d5;
	int ddr_d6;
	int ddr_d7;
	int ddr_d8;
	int ddr_d9;
	int ddr_d10;
	int ddr_d11;
	int ddr_d12;
	int ddr_d13;
	int ddr_d14;
	int ddr_d15;
	int ddr_dqm0;
	int ddr_dqm1;
	int ddr_dqs0;
	int ddr_dqsn0;
	int ddr_dqs1;
	int ddr_dqsn1;
	int ddr_vref;
	int ddr_vtp;
	int ddr_strben0;
	int ddr_strben1;
	int ain7;
	int ain6;
	int ain5;
	int ain4;
	int ain3;
	int ain2;
	int ain1;
	int ain0;
	int vrefp;
	int vrefn;
};

struct module_pin_mux {
	short reg_offset;
	unsigned char val;
};

struct evm_pin_mux {
	struct module_pin_mux *mod_pin_mux;
	unsigned short profile;
/*
* If the device is required on both baseboard & daughter board (ex i2c),
* specify DEV_ON_BASEBOARD
*/
#define DEV_ON_BASEBOARD       0
#define DEV_ON_DGHTR_BRD       1
	unsigned short device_on;
};

#define PAD_CTRL_BASE	0x800
#define OFFSET(x)	(unsigned int) (&((struct pad_signals *) \
				(PAD_CTRL_BASE))->x)

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		/* UART0_TXD */
	{-1},
};

static struct module_pin_mux uart3_pin_mux[] = {
	{OFFSET(spi0_cs1), (MODE(1) | PULLUDEN | RXACTIVE)},	/* UART3_RXD */
	{OFFSET(ecap0_in_pwm0_out), (MODE(1) | PULLUDEN)},	/* UART3_TXD */
	{-1},
};

/*
 * Update the structure with the modules present in the general purpose
 * board and the profiles in which the modules are present.
 * If the module is physically present but if it is not available
 * in any of the profile, then do not update it.
 * For eg, nand is avialable only in the profiles 0 and 1, whereas
 * UART0  is available in all the profiles.
 */
static struct evm_pin_mux general_purpose_evm_pin_mux[] = {
	{uart0_pin_mux, PROFILE_ALL, DEV_ON_BASEBOARD},
	{0},
};

/* Base board has single profile */
static struct evm_pin_mux low_cost_evm_pin_mux[] = {
	{uart0_pin_mux,	PROFILE_NONE, DEV_ON_BASEBOARD},
	{0},
};

static struct evm_pin_mux *am335x_evm_pin_mux[] = {
	general_purpose_evm_pin_mux,
	low_cost_evm_pin_mux,
};

/*
 * Configure the pin mux for the module
 */
static void configure_module_pin_mux(struct module_pin_mux *mod_pin_mux)
{
	int i;

	if (!mod_pin_mux)
		return;

	for (i = 0; mod_pin_mux[i].reg_offset != -1; i++)
		MUX_CFG(mod_pin_mux[i].val, mod_pin_mux[i].reg_offset);
}

/*
 * Check each module in the daughter board(first argument) whether it is
 * available in the selected profile(second argument). If the module is not
 * available in the selected profile, skip the corresponding configuration.
 */
static void set_evm_pin_mux(struct evm_pin_mux *pin_mux,
			char prof, unsigned char dghtr_brd_flg)
{
	int i;

	if (!pin_mux)
		return;

	/*
	* Only General Purpose & Industrial Auto Motro Control
	* EVM has profiles. So check if this evm has profile.
	* If not, ignore the profile comparison
	*/

	/*
	* If the device is on baseboard, directly configure it. Else (device on
	* Daughter board), check if the daughter card is detected.
	*/

	for (i = 0; pin_mux[i].mod_pin_mux != 0; i++)  {
		if ((pin_mux[i].profile & prof) ||
					(prof == PROFILE_NONE)) {
			if (pin_mux->device_on == DEV_ON_BASEBOARD)
				configure_module_pin_mux(pin_mux[i].
								mod_pin_mux);
			else if (dghtr_brd_flg == TRUE)
					configure_module_pin_mux(pin_mux[i].
								mod_pin_mux);
		}
	}
}

void configure_evm_pin_mux(unsigned char dghtr_brd_id, unsigned short
		profile, unsigned char daughter_board_flag)
{
	if (dghtr_brd_id > BASE_BOARD)
		return;

	set_evm_pin_mux(am335x_evm_pin_mux[dghtr_brd_id], profile,
							daughter_board_flag);
}

