/*
 * mux.h
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
#ifndef __MUX_H
#define __MUX_H

/* PAD Control Fields */
#define SLEWCTRL	(0x1 << 6)
#define	RXACTIVE	(0x1 << 5)
#define	PULLUP_EN	(0x1 << 4) /* Pull UP Selection */
#define PULLUDEN	(0x1 << 3) /* Pull up Disbled */
#define MODE(val)	val

/* PAD Register offsets */
/* UART0 */
#define UART0_CTSN_PAD_CTRL_OFFSET			0x0968
#define UART0_RTSN_PAD_CTRL_OFFSET			0x096C
#define UART0_RXD_PAD_CTRL_OFFSET			0x0970
#define UART0_TXD_PAD_CTRL_OFFSET			0x0974

/* EMIF/DDR */
#define DDR_RESETN_PAD_CTRL_OFFSET			0x0A38
#define DDR_CSN0_PAD_CTRL_OFFSET			0x0A3C
#define DDR_CKE_PAD_CTRL_OFFSET				0x0A40
#define DDR_CK_PAD_CTRL_OFFSET				0x0A44
#define DDR_CKN_PAD_CTRL_OFFSET				0x0A48
#define DDR_CASN_PAD_CTRL_OFFSET			0x0A4C
#define DDR_RASN_PAD_CTRL_OFFSET			0x0A50
#define DDR_WEN_PAD_CTRL_OFFSET				0x0A54
#define DDR_BA0_PAD_CTRL_OFFSET				0x0A58
#define DDR_BA1_PAD_CTRL_OFFSET				0x0A5C
#define DDR_BA2_PAD_CTRL_OFFSET				0x0A60
#define DDR_A0_PAD_CTRL_OFFSET				0x0A64
#define DDR_A1_PAD_CTRL_OFFSET				0x0A68
#define DDR_A2_PAD_CTRL_OFFSET				0x0A6C
#define DDR_A3_PAD_CTRL_OFFSET				0x0A70
#define DDR_A4_PAD_CTRL_OFFSET				0x0A74
#define DDR_A5_PAD_CTRL_OFFSET				0x0A78
#define DDR_A6_PAD_CTRL_OFFSET				0x0A7C
#define DDR_A7_PAD_CTRL_OFFSET				0x0A80
#define DDR_A8_PAD_CTRL_OFFSET				0x0A84
#define DDR_A9_PAD_CTRL_OFFSET				0x0A88
#define DDR_A10_PAD_CTRL_OFFSET				0x0A8C
#define DDR_A11_PAD_CTRL_OFFSET				0x0A90
#define DDR_A12_PAD_CTRL_OFFSET				0x0A94
#define DDR_A13_PAD_CTRL_OFFSET				0x0A98
#define DDR_A14_PAD_CTRL_OFFSET				0x0A9C
#define DDR_A15_PAD_CTRL_OFFSET				0x0AA0
#define DDR_ODT_PAD_CTRL_OFFSET				0x0AA4
#define DDR_D0_PAD_CTRL_OFFSET				0x0AA8
#define DDR_D1_PAD_CTRL_OFFSET				0x0AAC
#define DDR_D2_PAD_CTRL_OFFSET				0x0AB0
#define DDR_D3_PAD_CTRL_OFFSET				0x0AB4
#define DDR_D4_PAD_CTRL_OFFSET				0x0AB8
#define DDR_D5_PAD_CTRL_OFFSET				0x0ABC
#define DDR_D6_PAD_CTRL_OFFSET				0x0AC0
#define DDR_D7_PAD_CTRL_OFFSET				0x0AC4
#define DDR_D8_PAD_CTRL_OFFSET				0x0AC8
#define DDR_D9_PAD_CTRL_OFFSET				0x0ACC
#define DDR_D10_PAD_CTRL_OFFSET				0x0AD0
#define DDR_D11_PAD_CTRL_OFFSET				0x0AD4
#define DDR_D12_PAD_CTRL_OFFSET				0x0AD8
#define DDR_D13_PAD_CTRL_OFFSET				0x0ADC
#define DDR_D14_PAD_CTRL_OFFSET				0x0AE0
#define DDR_D15_PAD_CTRL_OFFSET				0x0AE4
#define DDR_DQM0_PAD_CTRL_OFFSET			0x0AE8
#define DDR_DQM1_PAD_CTRL_OFFSET			0x0AEC
#define DDR_DQS0_PAD_CTRL_OFFSET			0x0AF0
#define DDR_DQSN0_PAD_CTRL_OFFSET			0x0AF4
#define DDR_DQS1_PAD_CTRL_OFFSET			0x0AF8
#define DDR_DQSN1_PAD_CTRL_OFFSET			0x0AFC
#define DDR_VREF_PAD_CTRL_OFFSET			0x0B00
#define DDR_VTP_PAD_CTRL_OFFSET				0x0B04

#define DDR_NO_OF_PINS	((DDR_VTP_PAD_CTRL_OFFSET - \
	DDR_RESETN_PAD_CTRL_OFFSET) / 4 + 1)

#define	GPMC_AD0_PAD_CTRL_OFFSET			0x800
#define	GPMC_AD1_PAD_CTRL_OFFSET			0x804
#define	GPMC_AD2_PAD_CTRL_OFFSET			0x808
#define	GPMC_AD3_PAD_CTRL_OFFSET			0x80C
#define	GPMC_AD4_PAD_CTRL_OFFSET			0x810
#define	GPMC_AD5_PAD_CTRL_OFFSET			0x814
#define	GPMC_AD6_PAD_CTRL_OFFSET			0x818
#define	GPMC_AD7_PAD_CTRL_OFFSET			0x81C
#define	GPMC_WAIT0_PAD_CTRL_OFFSET			0x870
#define	GPMC_WPN_PAD_CTRL_OFFSET			0x874
#define	GPMC_CSN0_PAD_CTRL_OFFSET			0x87C
#define	GPMC_ADVN_ALE_PAD_CTRL_OFFSET			0x890
#define	GPMC_OEN_REN_ALE_PAD_CTRL_OFFSET		0x894
#define	GPMC_WEN_ALE_PAD_CTRL_OFFSET			0x898
#define	GPMC_BE0N_CLE_PAD_CTRL_OFFSET			0x89C

static char ddr_mux_val[] = {
	MODE(0),		/* reset */
	MODE(0) | PULLUP_EN,	/* csn0 */
	MODE(0),		/* cke */
	MODE(0),		/* ckn */
	MODE(0) | PULLUP_EN,	/* casn */
	MODE(0) | PULLUP_EN,	/* rasn */
	MODE(0) | PULLUP_EN,	/* wen */
	MODE(0) | PULLUP_EN,	/* ba0 */
	MODE(0) | PULLUP_EN,	/* ba1 */
	MODE(0) | PULLUP_EN,	/* ba2 */
	MODE(0) | PULLUP_EN,	/* ba3 */
	MODE(0) | PULLUP_EN,	/* a0 */
	MODE(0) | PULLUP_EN,	/* a1 */
	MODE(0) | PULLUP_EN,	/* a2 */
	MODE(0) | PULLUP_EN,	/* a3 */
	MODE(0) | PULLUP_EN,	/* a4 */
	MODE(0) | PULLUP_EN,	/* a5 */
	MODE(0) | PULLUP_EN,	/* a6 */
	MODE(0) | PULLUP_EN,	/* a7 */
	MODE(0) | PULLUP_EN,	/* a8 */
	MODE(0) | PULLUP_EN,	/* a9 */
	MODE(0) | PULLUP_EN,	/* a10 */
	MODE(0) | PULLUP_EN,	/* a11 */
	MODE(0) | PULLUP_EN,	/* a12 */
	MODE(0) | PULLUP_EN,	/* a13 */
	MODE(0) | PULLUP_EN,	/* a14 */
	MODE(0) | PULLUP_EN,	/* a15 */
	MODE(0) | RXACTIVE,	/* odt */
	MODE(0) | RXACTIVE,	/* d0 */
	MODE(0) | RXACTIVE,	/* d1 */
	MODE(0) | RXACTIVE,	/* d2 */
	MODE(0) | RXACTIVE,	/* d3 */
	MODE(0) | RXACTIVE,	/* d4 */
	MODE(0) | RXACTIVE,	/* d5 */
	MODE(0) | RXACTIVE,	/* d6 */
	MODE(0) | RXACTIVE,	/* d7 */
	MODE(0) | RXACTIVE,	/* d8 */
	MODE(0) | RXACTIVE,	/* d9 */
	MODE(0) | RXACTIVE,	/* d10 */
	MODE(0) | RXACTIVE,	/* d11 */
	MODE(0) | RXACTIVE,	/* d12 */
	MODE(0) | RXACTIVE,	/* d13 */
	MODE(0) | RXACTIVE,	/* d14 */
	MODE(0) | RXACTIVE,	/* d15 */
	MODE(0) | PULLUP_EN,	/* dqm0 */
	MODE(0) | PULLUP_EN,	/* dqm1 */
	MODE(0) | RXACTIVE,	/* dqs0 */
	MODE(0) | PULLUP_EN | RXACTIVE,	/* dqsn0 */
	MODE(0) | RXACTIVE,	/* dqs1 */
	MODE(0) | PULLUP_EN | RXACTIVE,	/* dqsn1 */
	MODE(0) | RXACTIVE,	/* vref */
	MODE(0) | RXACTIVE,	/* vtp */
};

#endif /* __MUX_H */
