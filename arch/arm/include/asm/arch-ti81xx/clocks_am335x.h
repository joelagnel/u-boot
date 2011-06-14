/*
 * clocks_am335x.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef _CLOCKS_AM335X_H_
#define _CLOCKS_AM335X_H_

/* Put the pll config values over here */

/* Core PLL Fdll = 1 GHZ, */
#define COREPLL_M      125     /* 125 * n */
#define COREPLL_N      5       /* (n -1 ) */

#define COREPLL_M4     4       /* CORE_CLKOUTM4 = 200 MHZ */
#define COREPLL_M5     3       /* CORE_CLKOUTM5 = 250 MHZ */
#define COREPLL_M6     1       /* CORE_CLKOUTM6 = 500 MHZ */

/*
 * USB PHY clock is 960 MHZ. Since, this comes directly from Fdll, Fdll
 * frequency needs to be set to 960 MHZ. Hence,
 * For clkout = 192 MHZ, Fdll = 960 MHZ, divider values are given below
 */
#define PERPLL_M       80      /* M = 40 * (N + 1) */
#define PERPLL_N       1
#define PERPLL_M2      5

/* DDR Freq is 200 MHZ */
/* Set Fdll = 400 MHZ , Fdll = M * 2 * CLKINP/ N + 1; clkout = Fdll /(2 * M2) */
#define DDRPLL_M	25	/* M/N + 1 = 25/3 */
#define DDRPLL_N	2
#define DDRPLL_M2	1

#endif	/* endif _CLOCKS_AM335X_H_ */
