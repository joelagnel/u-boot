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

/* Daughter Card type */
#define GP_DAUGHTER_BOARD	0
#define IA_DAUGHTER_BOARD	1
#define IPP_DAUGHTER_BOARD	2
#define BASE_BOARD_ONLY		3

/* Profiles */
#define PROFILE_0	(0x1 << 0)
#define PROFILE_1	(0x1 << 1)
#define PROFILE_2	(0x1 << 2)
#define PROFILE_3	(0x1 << 3)
#define PROFILE_4	(0x1 << 4)
#define PROFILE_5	(0x1 << 5)
#define PROFILE_6	(0x1 << 6)
#define PROFILE_7	(0x1 << 7)

extern void pll_init(void);
extern void enable_ddr_clocks(void);

extern void enable_uart0_pin_mux(void);
extern void enable_emif_pin_mux(void);
extern void enable_nand_pin_mux(void);

extern unsigned char get_daughter_board_id(void);
#endif/*__COMMON_DEF_H__ */
