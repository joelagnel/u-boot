/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _CLOCKS_TI814X_H_
#define _CLOCKS_TI814X_H_

/* Put the pll config values over here */
#define MODENA_N	1
#define MODENA_M	0x3C
#define MODENA_M2	1
#define MODENA_CLKCTRL	0x1

#define L3_N		19
#define L3_M		800
#define L3_M2		4
#define L3_CLKCTRL	0x801

#define DDR_N		19
#define DDR_M		666
#define DDR_M2		2
#define DDR_CLKCTRL	0x801

#define USB_N		19
#define USB_M		960
#define USB_M2		1
#define USB_CLKCTRL	0x200a0801
#endif	/* endif _CLOCKS_TI814X_H_ */

