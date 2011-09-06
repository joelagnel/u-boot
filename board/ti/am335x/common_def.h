/*
 * common_def.h
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
#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#define __raw_readl(a)		(*(volatile unsigned int *)(a))
#define __raw_writel(v, a)	(*(volatile unsigned int *)(a) = (v))
#define __raw_readw(a)		(*(volatile unsigned short *)(a))
#define __raw_writew(v, a)	(*(volatile unsigned short *)(a) = (v))

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define NO_OF_DAUGHTER_BOARDS	3

/* EVM type */
#define GP_BOARD	0
#define IA_BOARD	1
#define IPP_BOARD	2
#define BASE_BOARD	3

/* Profiles */
#define PROFILE_NONE	0x0
#define PROFILE_0	(1 << 0)
#define PROFILE_1	(1 << 1)
#define PROFILE_2	(1 << 2)
#define PROFILE_3	(1 << 3)
#define PROFILE_4	(1 << 4)
#define PROFILE_5	(1 << 5)
#define PROFILE_6	(1 << 6)
#define PROFILE_7	(1 << 7)
#define PROFILE_ALL	0xFF

extern void pll_init(void);
extern void enable_ddr_clocks(void);

extern void enable_i2c0_pin_mux(void);
extern void configure_evm_pin_mux(unsigned char daughter_board_id,
					unsigned short daughter_board_profile,
					unsigned char daughter_board_flag);

#endif/*__COMMON_DEF_H__ */
