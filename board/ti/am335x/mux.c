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
#include <asm/io.h>
#include "common_def.h"
#include "mux.h"

#define PAD_CONTROL_BASE	0x44E10000
#define MUX_CFG(offset, value)	\
	__raw_writel(value, PAD_CONTROL_BASE + offset);

void enable_uart0_pin_mux(void)
{
	MUX_CFG(UART0_CTSN_PAD_CTRL_OFFSET, MODE(0) | PULLUDEN);
	MUX_CFG(UART0_RTSN_PAD_CTRL_OFFSET, MODE(0) | RXACTIVE | PULLUDEN);
	MUX_CFG(UART0_RXD_PAD_CTRL_OFFSET, MODE(0) | RXACTIVE | PULLUDEN);
	MUX_CFG(UART0_TXD_PAD_CTRL_OFFSET, MODE(0) | PULLUDEN);
}

void enable_emif_pin_mux(void)
{
	int i;

	if (DDR_NO_OF_PINS != sizeof(ddr_mux_val)) {
		printf("EMIF Muxing failed: Mismatch in the no of \
			registers and the values!!!");
		return;
	}

	for (i = 0; i < sizeof(ddr_mux_val); i++)
		MUX_CFG(DDR_RESETN_PAD_CTRL_OFFSET + i * 4, ddr_mux_val[i]);
}

void enable_nand_pin_mux(void)
{
	MUX_CFG(GPMC_AD0_PAD_CTRL_OFFSET, (MODE(0) | RXACTIVE));
	MUX_CFG(GPMC_AD1_PAD_CTRL_OFFSET, (MODE(0) | RXACTIVE));
	MUX_CFG(GPMC_AD2_PAD_CTRL_OFFSET, (MODE(0) | RXACTIVE));
	MUX_CFG(GPMC_AD3_PAD_CTRL_OFFSET, (MODE(0) | RXACTIVE));
	MUX_CFG(GPMC_AD4_PAD_CTRL_OFFSET, (MODE(0) | RXACTIVE));
	MUX_CFG(GPMC_AD5_PAD_CTRL_OFFSET, (MODE(0) | RXACTIVE));
	MUX_CFG(GPMC_AD6_PAD_CTRL_OFFSET, (MODE(0) | RXACTIVE));
	MUX_CFG(GPMC_WAIT0_PAD_CTRL_OFFSET, (MODE(0) | RXACTIVE | PULLUP_EN));
	MUX_CFG(GPMC_WPN_PAD_CTRL_OFFSET, (MODE(0) | PULLUP_EN));
	MUX_CFG(GPMC_CSN0_PAD_CTRL_OFFSET, (MODE(0) | PULLUP_EN));
	MUX_CFG(GPMC_ADVN_ALE_PAD_CTRL_OFFSET, (MODE(0) | PULLUP_EN));
	MUX_CFG(GPMC_OEN_REN_ALE_PAD_CTRL_OFFSET, (MODE(0) | PULLUP_EN));
	MUX_CFG(GPMC_ADVN_ALE_PAD_CTRL_OFFSET, (MODE(0) | PULLUP_EN));
	MUX_CFG(GPMC_OEN_REN_ALE_PAD_CTRL_OFFSET, (MODE(0) | PULLUP_EN));
}

void enable_i2c0_pin_mux(void)
{
	MUX_CFG(I2C0_SDA_PAD_CTRL_OFFSET, MODE(0) | RXACTIVE | PULLUDEN);
	MUX_CFG(I2C0_CLK_PAD_CTRL_OFFSET, MODE(0) | RXACTIVE | PULLUDEN);
}
