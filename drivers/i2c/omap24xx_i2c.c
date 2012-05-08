/*
 * Basic I2C functions
 *
 * Copyright (c) 2004 Texas Instruments
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author: Jian Zhang jzhang@ti.com, Texas Instruments
 *
 * Copyright (c) 2003 Wolfgang Denk, wd@denx.de
 * Rewritten to fit into the current U-Boot framework
 *
 * Adapted for OMAP2420 I2C, r-woodruff2@ti.com
 *
 */

#include <common.h>

#include <asm/arch/i2c.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_TI81XX
/* HACK */
#define I2C_REV_OFS	(0x00)
#define I2C_SYSC_OFS	(0x10)
#define I2C_IE_OFS	(0x2C)
#define I2C_STAT_OFS	(0x28)
#define I2C_SYSS_OFS	(0x90)
#define I2C_BUF_OFS	(0x94)
#define I2C_CNT_OFS	(0x98)
#define I2C_DATA_OFS	(0x9c)
#define I2C_CON_OFS	(0xA4)
#define I2C_OA_OFS	(0xA8)
#define I2C_SA_OFS	(0xAC)
#define I2C_PSC_OFS	(0xB0)
#define I2C_SCLL_OFS	(0xB4)
#define I2C_SCLH_OFS	(0xB8)
#define I2C_SYSTEST_OFS	(0xBc)

#define I2C_TIMEOUT   ( 1 << 31)
static u32 wait_for_bb (void);
static u32 wait_for_status_mask(u16 mask);
static void flush_fifo(void);
static u32 i2c_base = (u32)I2C_DEFAULT_BASE;

static unsigned int bus_initialized[I2C_BUS_MAX];
static unsigned int current_bus;

void i2c_init(int speed, int slaveadd)
{
	int psc, fsscll, fssclh;
	int hsscll = 0, hssclh = 0;
	u32 scll, sclh;

	/* Only handle standard, fast and high speeds */
	if ((speed != OMAP_I2C_STANDARD) &&
	    (speed != OMAP_I2C_FAST_MODE) &&
	    (speed != OMAP_I2C_HIGH_SPEED)) {
		return;
	}

	psc = I2C_IP_CLK / I2C_INTERNAL_SAMPLING_CLK;
	psc -= 1;
	if (psc < I2C_PSC_MIN) {
		return;
	}

	if (speed == OMAP_I2C_HIGH_SPEED) {
		/* High speed */

		/* For first phase of HS mode */
		fsscll = fssclh = I2C_INTERNAL_SAMPLING_CLK /
			(2 * OMAP_I2C_FAST_MODE);

		fsscll -= I2C_HIGHSPEED_PHASE_ONE_SCLL_TRIM;
		fssclh -= I2C_HIGHSPEED_PHASE_ONE_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			return;
		}

		/* For second phase of HS mode */
		hsscll = hssclh = I2C_INTERNAL_SAMPLING_CLK / (2 * speed);

		hsscll -= I2C_HIGHSPEED_PHASE_TWO_SCLL_TRIM;
		hssclh -= I2C_HIGHSPEED_PHASE_TWO_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			return;
		}

		scll = (unsigned int)hsscll << 8 | (unsigned int)fsscll;
		sclh = (unsigned int)hssclh << 8 | (unsigned int)fssclh;

	} else {
		/* Standard and fast speed */
		fsscll = fssclh = I2C_INTERNAL_SAMPLING_CLK / (2 * speed);

		fsscll -= I2C_FASTSPEED_SCLL_TRIM;
		fssclh -= I2C_FASTSPEED_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			return;
		}

		scll = (unsigned int)fsscll;
		sclh = (unsigned int)fssclh;
	}

	if (gd->flags & GD_FLG_RELOC)
		bus_initialized[current_bus] = 1;

	if (readw ((i2c_base + I2C_CON_OFS)) & I2C_CON_EN) {
		writew (0, (i2c_base + I2C_CON_OFS));
		udelay (50000);
	}

	writew(psc, (i2c_base + I2C_PSC_OFS));
	writew(scll, (i2c_base + I2C_SCLL_OFS));
	writew(sclh, (i2c_base + I2C_SCLH_OFS));

	/* own address */
	writew (slaveadd, (i2c_base + I2C_OA_OFS));
	writew (I2C_CON_EN, (i2c_base + I2C_CON_OFS));

	/* have to enable intrrupts or OMAP i2c module doesn't work */
	writew (I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE |
		I2C_IE_NACK_IE | I2C_IE_AL_IE, (i2c_base + I2C_IE_OFS));
	udelay (1000);
	flush_fifo();
	writew (0xFFFF, (i2c_base + I2C_STAT_OFS));
	writew (0, (i2c_base + I2C_CNT_OFS));
}

static void flush_fifo(void)
{	u16 stat;

	/* note: if you try and read data when its not there or ready
	 * you get a bus error
	 */
	while(1){
		stat = readw((i2c_base + I2C_STAT_OFS));
		if(stat == I2C_STAT_RRDY){
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_TI81XX)
			readb((i2c_base + I2C_DATA_OFS));
#else
			readw((i2c_base + I2C_DATA_OFS));
#endif
			writew(I2C_STAT_RRDY,(i2c_base + I2C_STAT_OFS));
			udelay(1000);
		} else
			break;
	}
}

int i2c_probe(uchar chip)
{
	int res = 1; /* default = fail */
	u32 status;

	if (chip == readw ((i2c_base + I2C_OA_OFS)))
		return res;

	/* wait until bus not busy */
	status = wait_for_bb();

	/* exiting on BUS busy */
	if (status & I2C_TIMEOUT)
		return res;

	/* try to read one byte */
	writew (1, (i2c_base + I2C_CNT_OFS));
	/* set slave address */
	writew (chip, (i2c_base + I2C_SA_OFS));
	/* stop bit needed here */
	writew (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP, (i2c_base + I2C_CON_OFS));
	/* enough delay for the NACK bit set */
	udelay (50000);

	if (!(readw ((i2c_base + I2C_STAT_OFS)) & I2C_STAT_NACK)) {
		res = 0;      /* success case */
		flush_fifo();
		writew(0xFFFF, (i2c_base + I2C_STAT_OFS));
	} else {
		writew(0xFFFF, (i2c_base + I2C_STAT_OFS));	 /* failue, clear sources*/
		/* finish up xfer */
		writew(readw((i2c_base + I2C_CON_OFS)) | I2C_CON_STP, (i2c_base + I2C_CON_OFS));
		udelay(20000);
		wait_for_bb();
		status = wait_for_bb();

		/* exiting on BUS busy */
		if (status & I2C_TIMEOUT)
			return res;
	}
	flush_fifo();
	/* don't allow any more data in...we don't want it.*/
	writew(0, (i2c_base + I2C_CNT_OFS));
	writew(0xFFFF, (i2c_base + I2C_STAT_OFS));
	return res;
}

int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int i2c_error = 0, i;
	u32 status;

	if ((alen > 2) || (alen < 0)) {
		return 1;
	}

	if (addr + len > 0xFFFF) {
		return 1;
	}

	/* wait until bus not busy */
	status = wait_for_bb();

	/* exiting on BUS busy */
	if (status & I2C_TIMEOUT)
		return 1;

	/* one byte only */
	writew((alen & 0xFF), (i2c_base + I2C_CNT_OFS));
	/* set slave address */
	writew(chip, (i2c_base + I2C_SA_OFS));
	/* Clear the Tx & Rx FIFOs */
	writew((readw((i2c_base + I2C_BUF_OFS)) | I2C_RXFIFO_CLEAR | I2C_TXFIFO_CLEAR), (i2c_base + I2C_BUF_OFS));
	/* no stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_TRX | I2C_CON_STT, (i2c_base + I2C_CON_OFS));

	/* waiting for Transmit ready condition */
	status = wait_for_status_mask(I2C_STAT_XRDY | I2C_STAT_NACK);

	if (status & (I2C_STAT_NACK | I2C_TIMEOUT))
		i2c_error = 1;

	if (!i2c_error) {
		if (status & I2C_STAT_XRDY) {
			switch (alen) {
			case 2:
				/* Send address MSByte */
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX)\
		|| defined(CONFIG_TI81XX)
				writew(((addr >> 8) & 0xFF), (i2c_base + I2C_DATA_OFS));

				/* Clearing XRDY event */
				writew((status & I2C_STAT_XRDY), (i2c_base + I2C_STAT_OFS));
				/*waiting for Transmit ready * condition */
				status = wait_for_status_mask(I2C_STAT_XRDY |
						I2C_STAT_NACK);

				if (status & (I2C_STAT_NACK |
							I2C_TIMEOUT)) {
					i2c_error = 1;
					break;
				}
#else
#endif
			case 1:
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX)\
		|| defined(CONFIG_TI81XX)
				/* Send address LSByte */
				writew((addr & 0xFF), (i2c_base + I2C_DATA_OFS));
#else
				/* Send address Short word */
				writew((addr & 0xFFFF), (i2c_base + I2C_DATA_OFS));
#endif
				/* Clearing XRDY event */
				writew((status & I2C_STAT_XRDY), (i2c_base + I2C_STAT_OFS));
				/*waiting for Transmit ready * condition */
				status = wait_for_status_mask(I2C_STAT_ARDY |
						I2C_STAT_NACK);

				if (status & (I2C_STAT_NACK |
								I2C_TIMEOUT)) {
					i2c_error = 1;
					break;
				}
			}
		} else
			i2c_error = 1;
	}

	/* Waiting for ARDY to set */
	status = wait_for_status_mask(I2C_STAT_ARDY | I2C_STAT_NACK
			| I2C_STAT_AL);

	if (!i2c_error) {
		/* set slave address */
		writew(chip, (i2c_base + I2C_SA_OFS));
		/* read one byte from slave */
		writew((len & 0xFF), (i2c_base + I2C_CNT_OFS));
		/* Clear the Tx & Rx FIFOs */
		writew((readw((i2c_base + I2C_BUF_OFS)) | I2C_RXFIFO_CLEAR |
					I2C_TXFIFO_CLEAR), (i2c_base + I2C_BUF_OFS));
		/* need stop bit here */
		writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP,
				(i2c_base + I2C_CON_OFS));

		for (i = 0; i < len; i++) {
			/* waiting for Receive condition */
			status = wait_for_status_mask(I2C_STAT_RRDY |
					I2C_STAT_NACK);

			if (status & (I2C_STAT_NACK | I2C_TIMEOUT)) {
				i2c_error = 1;
				break;
			}

			if (status & I2C_STAT_RRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) \
				|| defined(CONFIG_TI81XX)
				buffer[i] = readb((i2c_base + I2C_DATA_OFS));
#else
				*((u16 *)&buffer[i]) = readw((i2c_base + I2C_DATA_OFS)) & 0xFFFF;
				i++;
#endif
				writew((status & I2C_STAT_RRDY), (i2c_base + I2C_STAT_OFS));
				udelay(1000);
			} else {
				i2c_error = 1;
			}
		}
	}

	/* Waiting for ARDY to set */
	status = wait_for_status_mask(I2C_STAT_ARDY | I2C_STAT_NACK
			| I2C_STAT_AL);

	if (i2c_error) {
		writew(0, (i2c_base + I2C_CON_OFS));
		return 1;
	}

	if (!i2c_error) {
		writew(I2C_CON_EN, (i2c_base + I2C_CON_OFS));

		while (readw((i2c_base + I2C_STAT_OFS))
				|| (readw((i2c_base + I2C_CON_OFS)) & I2C_CON_MST)) {
			udelay(10000);
			writew(0xFFFF, (i2c_base + I2C_STAT_OFS));
		}
	}

	writew(I2C_CON_EN, (i2c_base + I2C_CON_OFS));
	flush_fifo();
	writew(0xFFFF, (i2c_base + I2C_STAT_OFS));
	writew(0, (i2c_base + I2C_CNT_OFS));

	return 0;
}

int i2c_write(uchar chip, uint addr, int alen,
		uchar *buffer, int len)
{
	int i, i2c_error = 0;
	u16 writelen;
	u32 status;

	if (alen > 2) {
		return 1;
	}

	if (addr + len > 0xFFFF) {
		return 1;
	}

	/* wait until bus not busy */
	status = wait_for_bb();

	/* exiting on BUS busy */
	if (status & I2C_TIMEOUT)
		return 1;

	writelen = (len & 0xFFFF) + alen;

	/* two bytes */
	writew((writelen & 0xFFFF), (i2c_base + I2C_CNT_OFS));
	/* Clear the Tx & Rx FIFOs */
	writew((readw((i2c_base + I2C_BUF_OFS)) | I2C_RXFIFO_CLEAR | I2C_TXFIFO_CLEAR), (i2c_base + I2C_BUF_OFS));
	/* set slave address */
	writew(chip, (i2c_base + I2C_SA_OFS));
	/* stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX |
		I2C_CON_STP, (i2c_base + I2C_CON_OFS));

	/* waiting for Transmit ready condition */
	status = wait_for_status_mask(I2C_STAT_XRDY | I2C_STAT_NACK);

	if (status & (I2C_STAT_NACK | I2C_TIMEOUT))
		i2c_error = 1;

	if (!i2c_error) {
		if (status & I2C_STAT_XRDY) {
			switch (alen) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX)\
		|| defined(CONFIG_TI81XX)
			case 2:
				/* send out MSB byte */
				writeb(((addr >> 8) & 0xFF), (i2c_base + I2C_DATA_OFS));
#else
				writeb((addr  & 0xFFFF), (i2c_base + I2C_DATA_OFS));
				break;
#endif
				/* Clearing XRDY event */
				writew((status & I2C_STAT_XRDY), (i2c_base + I2C_STAT_OFS));
				/*waiting for Transmit ready * condition */
				status = wait_for_status_mask(I2C_STAT_XRDY |
						I2C_STAT_NACK);

				if (status & (I2C_STAT_NACK | I2C_TIMEOUT)) {
					i2c_error = 1;
					break;
				}
			case 1:
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX)\
		|| defined(CONFIG_TI81XX)
				/* send out MSB byte */
				writeb((addr  & 0xFF), (i2c_base + I2C_DATA_OFS));
#else
				writew(((bufer[0] << 8) | (addr & 0xFF)),
						(i2c_base + I2C_DATA_OFS));
#endif
			}

			/* Clearing XRDY event */
			writew((status & I2C_STAT_XRDY), (i2c_base + I2C_STAT_OFS));
		}

		/* waiting for Transmit ready condition */
		status = wait_for_status_mask(I2C_STAT_XRDY | I2C_STAT_NACK);

		if (status & (I2C_STAT_NACK | I2C_TIMEOUT))
			i2c_error = 1;

		if (!i2c_error) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) \
			|| defined(CONFIG_TI81XX)
			for (i =  0; i < len; i++) {
#else
			for (i = ((alen > 1) ? 0 : 1); i < len; i++) {
#endif
				if (status & I2C_STAT_XRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) \
			       || defined(CONFIG_TI81XX)
					writeb((buffer[i] & 0xFF), (i2c_base + I2C_DATA_OFS));
#else
					writew((((bufer[i] << 8) | buffer[i + 1]) &
								0xFFFF) ,
							(i2c_base + I2C_DATA_OFS));
					i++;
#endif
					} else
						i2c_error = 1;

					/* Clearing XRDY event */
					writew((status & I2C_STAT_XRDY),
							(i2c_base + I2C_STAT_OFS));
					/* waiting for XRDY condition */
					status = wait_for_status_mask(
							I2C_STAT_XRDY |
							I2C_STAT_ARDY |
							I2C_STAT_NACK);

					if (status & (I2C_STAT_NACK | I2C_TIMEOUT)) {
						i2c_error = 1;
						break;
					}

					if (status & I2C_STAT_ARDY)
						break;
			}
		}

	}

	status = wait_for_status_mask(I2C_STAT_ARDY | I2C_STAT_NACK |
			I2C_STAT_AL);

	if (status & (I2C_STAT_NACK | I2C_TIMEOUT))
		i2c_error = 1;

	if (i2c_error) {
		writew(0, (i2c_base + I2C_CON_OFS));
		return 1;
	}

	if (!i2c_error) {
		int eout = 200;

		writew(I2C_CON_EN, (i2c_base + I2C_CON_OFS));
		while ((status = readw((i2c_base + I2C_STAT_OFS))) ||
				(readw((i2c_base + I2C_CON_OFS)) & I2C_CON_MST)) {
			udelay(1000);
			/* have to read to clear intrrupt */
			writew(0xFFFF, (i2c_base + I2C_STAT_OFS));

			if (--eout == 0)
				/* better leave with error than hang */
				break;
		}
	}

	flush_fifo();
	writew(0xFFFF, (i2c_base + I2C_STAT_OFS));
	writew(0, (i2c_base + I2C_CNT_OFS));
	return 0;
}

static u32 wait_for_bb (void)
{
	int timeout = 10;
	u32 stat;

	writew(0xFFFF, (i2c_base + I2C_STAT_OFS));	 /* clear current interruts...*/
	while ((stat = readw ((i2c_base + I2C_STAT_OFS)) & I2C_STAT_BB) && timeout--) {
		writew (stat, (i2c_base + I2C_STAT_OFS));
		udelay (50000);
	}

	if (timeout <= 0) {
		stat |= I2C_TIMEOUT;
	}
	writew(0xFFFF, (i2c_base + I2C_STAT_OFS));	 /* clear delayed stuff*/
	return stat;
}

static u32 wait_for_status_mask(u16 mask)
{
	u32 status;
	int timeout = 10;

	do {
		udelay(1000);
		status = readw((i2c_base + I2C_STAT_OFS));
	} while (!(status & mask) && timeout--);

	if (timeout <= 0) {
		writew(0xFFFF, (i2c_base + I2C_STAT_OFS));
		status |= I2C_TIMEOUT;
	}
	return status;
}

#if defined(CONFIG_I2C_MULTI_BUS)
/*
* Functions for multiple I2C bus handling
*/

/**
* i2c_get_bus_num - returns index of active I2C bus
*/
unsigned int i2c_get_bus_num(void)
{
	return current_bus;
}

/**
* i2c_set_bus_num - change active I2C bus
*	@bus: bus index, zero based
*	@returns: 0 on success, non-0 on failure
*/
int i2c_set_bus_num(unsigned int bus)
{
	if ((bus < 0) || (bus >= I2C_BUS_MAX)) {
		return -1;
	}

#if I2C_BUS_MAX == 3
	if (bus == 2)
		i2c_base = (u32)I2C_BASE3;
	else
#endif
	if (bus == 1)
		i2c_base = (u32)I2C_BASE2;
	else
		i2c_base = (u32)I2C_BASE1;

	current_bus = bus;

	if (!bus_initialized[current_bus])
		i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	return 0;
}
#endif
#else
#include "omap24xx_i2c.h"

#define I2C_TIMEOUT	1000

static void wait_for_bb(void);
static u16 wait_for_pin(void);
static void flush_fifo(void);

/*
 * For SPL boot some boards need i2c before SDRAM is initialised so force
 * variables to live in SRAM
 */
static struct i2c __attribute__((section (".data"))) *i2c_base =
					(struct i2c *)I2C_DEFAULT_BASE;
static unsigned int __attribute__((section (".data"))) bus_initialized[I2C_BUS_MAX] =
					{ [0 ... (I2C_BUS_MAX-1)] = 0 };
static unsigned int __attribute__((section (".data"))) current_bus = 0;

void i2c_init(int speed, int slaveadd)
{
	int psc, fsscll, fssclh;
	int hsscll = 0, hssclh = 0;
	u32 scll, sclh;
	int timeout = I2C_TIMEOUT;

	/* Only handle standard, fast and high speeds */
	if ((speed != OMAP_I2C_STANDARD) &&
	    (speed != OMAP_I2C_FAST_MODE) &&
	    (speed != OMAP_I2C_HIGH_SPEED)) {
		printf("Error : I2C unsupported speed %d\n", speed);
		return;
	}

	psc = I2C_IP_CLK / I2C_INTERNAL_SAMPLING_CLK;
	psc -= 1;
	if (psc < I2C_PSC_MIN) {
		printf("Error : I2C unsupported prescalar %d\n", psc);
		return;
	}

	if (speed == OMAP_I2C_HIGH_SPEED) {
		/* High speed */

		/* For first phase of HS mode */
		fsscll = fssclh = I2C_INTERNAL_SAMPLING_CLK /
			(2 * OMAP_I2C_FAST_MODE);

		fsscll -= I2C_HIGHSPEED_PHASE_ONE_SCLL_TRIM;
		fssclh -= I2C_HIGHSPEED_PHASE_ONE_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			printf("Error : I2C initializing first phase clock\n");
			return;
		}

		/* For second phase of HS mode */
		hsscll = hssclh = I2C_INTERNAL_SAMPLING_CLK / (2 * speed);

		hsscll -= I2C_HIGHSPEED_PHASE_TWO_SCLL_TRIM;
		hssclh -= I2C_HIGHSPEED_PHASE_TWO_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			printf("Error : I2C initializing second phase clock\n");
			return;
		}

		scll = (unsigned int)hsscll << 8 | (unsigned int)fsscll;
		sclh = (unsigned int)hssclh << 8 | (unsigned int)fssclh;

	} else {
		/* Standard and fast speed */
		fsscll = fssclh = I2C_INTERNAL_SAMPLING_CLK / (2 * speed);

		fsscll -= I2C_FASTSPEED_SCLL_TRIM;
		fssclh -= I2C_FASTSPEED_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			printf("Error : I2C initializing clock\n");
			return;
		}

		scll = (unsigned int)fsscll;
		sclh = (unsigned int)fssclh;
	}

	if (readw(&i2c_base->con) & I2C_CON_EN) {
		writew(0, &i2c_base->con);
		udelay(50000);
	}

	writew(0x2, &i2c_base->sysc); /* for ES2 after soft reset */
	udelay(1000);

	writew(I2C_CON_EN, &i2c_base->con);
	while (!(readw(&i2c_base->syss) & I2C_SYSS_RDONE) && timeout--) {
		if (timeout <= 0) {
			printf("ERROR: Timeout in soft-reset\n");
			return;
		}
		udelay(1000);
	}

	writew(0, &i2c_base->con);
	writew(psc, &i2c_base->psc);
	writew(scll, &i2c_base->scll);
	writew(sclh, &i2c_base->sclh);

	/* own address */
	writew(slaveadd, &i2c_base->oa);
	writew(I2C_CON_EN, &i2c_base->con);

	/* have to enable intrrupts or OMAP i2c module doesn't work */
	writew(I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE |
		I2C_IE_NACK_IE | I2C_IE_AL_IE, &i2c_base->ie);
	udelay(1000);
	flush_fifo();
	writew(0xFFFF, &i2c_base->stat);
	writew(0, &i2c_base->cnt);

	if (gd->flags & GD_FLG_RELOC)
		bus_initialized[current_bus] = 1;
}

static int i2c_read_byte(u8 devaddr, u8 regoffset, u8 *value)
{
	int i2c_error = 0;
	u16 status;

	/* wait until bus not busy */
	wait_for_bb();

	/* one byte only */
	writew(1, &i2c_base->cnt);
	/* set slave address */
	writew(devaddr, &i2c_base->sa);
	/* no stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT |
	      I2C_CON_TRX, &i2c_base->con);

	/* send register offset */
	while (1) {
		status = wait_for_pin();
		if (status == 0 || status & I2C_STAT_NACK) {
			i2c_error = 1;
			goto read_exit;
		}
		if (status & I2C_STAT_XRDY) {
			/* Important: have to use byte access */
			writeb(regoffset, &i2c_base->data);
			writew(I2C_STAT_XRDY, &i2c_base->stat);
		}
		if (status & I2C_STAT_ARDY) {
			writew(I2C_STAT_ARDY, &i2c_base->stat);
			break;
		}
	}

	/* set slave address */
	writew(devaddr, &i2c_base->sa);
	/* read one byte from slave */
	writew(1, &i2c_base->cnt);
	/* need stop bit here */
	writew(I2C_CON_EN | I2C_CON_MST |
		I2C_CON_STT | I2C_CON_STP,
		&i2c_base->con);

	/* receive data */
	while (1) {
		status = wait_for_pin();
		if (status == 0 || status & I2C_STAT_NACK) {
			i2c_error = 1;
			goto read_exit;
		}
		if (status & I2C_STAT_RRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || \
	defined(CONFIG_OMAP44XX)
			*value = readb(&i2c_base->data);
#else
			*value = readw(&i2c_base->data);
#endif
			writew(I2C_STAT_RRDY, &i2c_base->stat);
		}
		if (status & I2C_STAT_ARDY) {
			writew(I2C_STAT_ARDY, &i2c_base->stat);
			break;
		}
	}

read_exit:
	flush_fifo();
	writew(0xFFFF, &i2c_base->stat);
	writew(0, &i2c_base->cnt);
	return i2c_error;
}

static void flush_fifo(void)
{	u16 stat;

	/* note: if you try and read data when its not there or ready
	 * you get a bus error
	 */
	while (1) {
		stat = readw(&i2c_base->stat);
		if (stat == I2C_STAT_RRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || \
	defined(CONFIG_OMAP44XX)
			readb(&i2c_base->data);
#else
			readw(&i2c_base->data);
#endif
			writew(I2C_STAT_RRDY, &i2c_base->stat);
			udelay(1000);
		} else
			break;
	}
}

int i2c_probe(uchar chip)
{
	u16 status;
	int res = 1; /* default = fail */

	if (chip == readw(&i2c_base->oa))
		return res;

	/* wait until bus not busy */
	wait_for_bb();

	/* try to write one byte */
	writew(1, &i2c_base->cnt);
	/* set slave address */
	writew(chip, &i2c_base->sa);
	/* stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX |
	       I2C_CON_STP, &i2c_base->con);

	status = wait_for_pin();

	/* check for ACK (!NAK) */
	if (!(status & I2C_STAT_NACK))
		res = 0;

	/* abort transfer (force idle state) */
	writew(0, &i2c_base->con);

	flush_fifo();
	/* don't allow any more data in... we don't want it. */
	writew(0, &i2c_base->cnt);
	writew(0xFFFF, &i2c_base->stat);
	return res;
}

int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int i;

	if (alen > 1) {
		printf("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		printf("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		if (i2c_read_byte(chip, addr + i, &buffer[i])) {
			printf("I2C read: I/O error\n");
			i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int i;
	u16 status;
	int i2c_error = 0;

	if (alen > 1) {
		printf("I2C write: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		printf("I2C write: address 0x%x + 0x%x out of range\n",
				addr, len);
		return 1;
	}

	/* wait until bus not busy */
	wait_for_bb();

	/* start address phase - will write regoffset + len bytes data */
	/* TODO consider case when !CONFIG_OMAP243X/34XX/44XX */
	writew(alen + len, &i2c_base->cnt);
	/* set slave address */
	writew(chip, &i2c_base->sa);
	/* stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX |
		I2C_CON_STP, &i2c_base->con);

	/* Send address byte */
	status = wait_for_pin();

	if (status == 0 || status & I2C_STAT_NACK) {
		i2c_error = 1;
		printf("error waiting for i2c address ACK (status=0x%x)\n",
		      status);
		goto write_exit;
	}

	if (status & I2C_STAT_XRDY) {
		writeb(addr & 0xFF, &i2c_base->data);
		writew(I2C_STAT_XRDY, &i2c_base->stat);
	} else {
		i2c_error = 1;
		printf("i2c bus not ready for transmit (status=0x%x)\n",
		      status);
		goto write_exit;
	}

	/* address phase is over, now write data */
	for (i = 0; i < len; i++) {
		status = wait_for_pin();

		if (status == 0 || status & I2C_STAT_NACK) {
			i2c_error = 1;
			printf("i2c error waiting for data ACK (status=0x%x)\n",
					status);
			goto write_exit;
		}

		if (status & I2C_STAT_XRDY) {
			writeb(buffer[i], &i2c_base->data);
			writew(I2C_STAT_XRDY, &i2c_base->stat);
		} else {
			i2c_error = 1;
			printf("i2c bus not ready for Tx (i=%d)\n", i);
			goto write_exit;
		}
	}

write_exit:
	flush_fifo();
	writew(0xFFFF, &i2c_base->stat);
	return i2c_error;
}

static void wait_for_bb(void)
{
	int timeout = I2C_TIMEOUT;
	u16 stat;

	writew(0xFFFF, &i2c_base->stat);	/* clear current interrupts...*/
	while ((stat = readw(&i2c_base->stat) & I2C_STAT_BB) && timeout--) {
		writew(stat, &i2c_base->stat);
		udelay(1000);
	}

	if (timeout <= 0) {
		printf("timed out in wait_for_bb: I2C_STAT=%x\n",
			readw(&i2c_base->stat));
	}
	writew(0xFFFF, &i2c_base->stat);	 /* clear delayed stuff*/
}

static u16 wait_for_pin(void)
{
	u16 status;
	int timeout = I2C_TIMEOUT;

	do {
		udelay(1000);
		status = readw(&i2c_base->stat);
	} while (!(status &
		   (I2C_STAT_ROVR | I2C_STAT_XUDF | I2C_STAT_XRDY |
		    I2C_STAT_RRDY | I2C_STAT_ARDY | I2C_STAT_NACK |
		    I2C_STAT_AL)) && timeout--);

	if (timeout <= 0) {
		printf("timed out in wait_for_pin: I2C_STAT=%x\n",
			readw(&i2c_base->stat));
		writew(0xFFFF, &i2c_base->stat);
		status = 0;
	}

	return status;
}

int i2c_set_bus_num(unsigned int bus)
{
	if ((bus < 0) || (bus >= I2C_BUS_MAX)) {
		printf("Bad bus: %d\n", bus);
		return -1;
	}

#if I2C_BUS_MAX == 3
	if (bus == 2)
		i2c_base = (struct i2c *)I2C_BASE3;
	else
#endif
	if (bus == 1)
		i2c_base = (struct i2c *)I2C_BASE2;
	else
		i2c_base = (struct i2c *)I2C_BASE1;

	current_bus = bus;

	if (!bus_initialized[current_bus])
		i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	return 0;
}

int i2c_get_bus_num(void)
{
	return (int) current_bus;
}
#endif
