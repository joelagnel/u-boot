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

#define I2C_TIMEOUT   ( 1 << 31)
static u32 wait_for_bb (void);
static u32 wait_for_status_mask(u16 mask);
static void flush_fifo(void);

static struct i2c *i2c_base = (struct i2c *)I2C_DEFAULT_BASE;

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

	if (readw (I2C_CON) & I2C_CON_EN) {
		writew (0, I2C_CON);
		udelay (50000);
	}

	writew(psc, I2C_PSC);
	writew(scll, I2C_SCLL);
	writew(sclh, I2C_SCLH);

	/* own address */
	writew (slaveadd, I2C_OA);
	writew (I2C_CON_EN, I2C_CON);

	/* have to enable intrrupts or OMAP i2c module doesn't work */
	writew (I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE |
		I2C_IE_NACK_IE | I2C_IE_AL_IE, I2C_IE);
	udelay (1000);
	flush_fifo();
	writew (0xFFFF, I2C_STAT);
	writew (0, I2C_CNT);
}

static void flush_fifo(void)
{	u16 stat;

	/* note: if you try and read data when its not there or ready
	 * you get a bus error
	 */
	while(1){
		stat = readw(I2C_STAT);
		if(stat == I2C_STAT_RRDY){
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_TI81XX)
			readb(I2C_DATA);
#else
			readw(I2C_DATA);
#endif
			writew(I2C_STAT_RRDY,I2C_STAT);
			udelay(1000);
		} else
			break;
	}
}

int i2c_probe(uchar chip)
{
	int res = 1; /* default = fail */
	u32 status;

	if (chip == readw (I2C_OA))
		return res;

	/* wait until bus not busy */
	status = wait_for_bb();

	/* exiting on BUS busy */
	if (status & I2C_TIMEOUT)
		return res;

	/* try to read one byte */
	writew (1, I2C_CNT);
	/* set slave address */
	writew (chip, I2C_SA);
	/* stop bit needed here */
	writew (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP, I2C_CON);
	/* enough delay for the NACK bit set */
	udelay (50000);

	if (!(readw (I2C_STAT) & I2C_STAT_NACK)) {
		res = 0;      /* success case */
		flush_fifo();
		writew(0xFFFF, I2C_STAT);
	} else {
		writew(0xFFFF, I2C_STAT);	 /* failue, clear sources*/
		/* finish up xfer */
		writew(readw(I2C_CON) | I2C_CON_STP, I2C_CON);
		udelay(20000);
		wait_for_bb();
		status = wait_for_bb();

		/* exiting on BUS busy */
		if (status & I2C_TIMEOUT)
			return res;
	}
	flush_fifo();
	/* don't allow any more data in...we don't want it.*/
	writew(0, I2C_CNT);
	writew(0xFFFF, I2C_STAT);
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
	writew((alen & 0xFF), I2C_CNT);
	/* set slave address */
	writew(chip, I2C_SA);
	/* Clear the Tx & Rx FIFOs */
	writew((readw(I2C_BUF) | I2C_RXFIFO_CLEAR | I2C_TXFIFO_CLEAR), I2C_BUF);
	/* no stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_TRX | I2C_CON_STT, I2C_CON);

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
				writew(((addr >> 8) & 0xFF), I2C_DATA);

				/* Clearing XRDY event */
				writew((status & I2C_STAT_XRDY), I2C_STAT);
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
				writew((addr & 0xFF), I2C_DATA);
#else
				/* Send address Short word */
				writew((addr & 0xFFFF), I2C_DATA);
#endif
				/* Clearing XRDY event */
				writew((status & I2C_STAT_XRDY), I2C_STAT);
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
		writew(chip, I2C_SA);
		/* read one byte from slave */
		writew((len & 0xFF), I2C_CNT);
		/* Clear the Tx & Rx FIFOs */
		writew((readw(I2C_BUF) | I2C_RXFIFO_CLEAR |
					I2C_TXFIFO_CLEAR), I2C_BUF);
		/* need stop bit here */
		writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP,
				I2C_CON);

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
				buffer[i] = readb(I2C_DATA);
#else
				*((u16 *)&buffer[i]) = readw(I2C_DATA) & 0xFFFF;
				i++;
#endif
				writew((status & I2C_STAT_RRDY), I2C_STAT);
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
		writew(0, I2C_CON);
		return 1;
	}

	if (!i2c_error) {
		writew(I2C_CON_EN, I2C_CON);

		while (readw(I2C_STAT)
				|| (readw(I2C_CON) & I2C_CON_MST)) {
			udelay(10000);
			writew(0xFFFF, I2C_STAT);
		}
	}

	writew(I2C_CON_EN, I2C_CON);
	flush_fifo();
	writew(0xFFFF, I2C_STAT);
	writew(0, I2C_CNT);

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
	writew((writelen & 0xFFFF), I2C_CNT);
	/* Clear the Tx & Rx FIFOs */
	writew((readw(I2C_BUF) | I2C_RXFIFO_CLEAR | I2C_TXFIFO_CLEAR), I2C_BUF);
	/* set slave address */
	writew(chip, I2C_SA);
	/* stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX |
		I2C_CON_STP, I2C_CON);

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
				writeb(((addr >> 8) & 0xFF), I2C_DATA);
#else
				writeb((addr  & 0xFFFF), I2C_DATA);
				break;
#endif
				/* Clearing XRDY event */
				writew((status & I2C_STAT_XRDY), I2C_STAT);
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
				writeb((addr  & 0xFF), I2C_DATA);
#else
				writew(((bufer[0] << 8) | (addr & 0xFF)),
						I2C_DATA);
#endif
			}

			/* Clearing XRDY event */
			writew((status & I2C_STAT_XRDY), I2C_STAT);
		}

		/* waiting for Transmit ready condition */
		status = wait_for_status_mask(I2C_STAT_XRDY | I2C_STAT_NACK);

		if (status & (I2C_STAT_NACK | I2C_TIMEOUT))
			i2c_error = 1;

		if (!i2c_error) {
			for (i = ((alen > 1) ? 0 : 1); i < len; i++) {

				if (status & I2C_STAT_XRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) \
			       || defined(CONFIG_TI81XX)
					writeb((buffer[i] & 0xFF), I2C_DATA);
#else
					writew((((bufer[i] << 8) | buffer[i + 1]) &
								0xFFFF) ,
							I2C_DATA);
					i++;
#endif
					} else
						i2c_error = 1;

					/* Clearing XRDY event */
					writew((status & I2C_STAT_XRDY),
							I2C_STAT);
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
		writew(0, I2C_CON);
		return 1;
	}

	if (!i2c_error) {
		int eout = 200;

		writew(I2C_CON_EN, I2C_CON);
		while ((status = readw(I2C_STAT)) ||
				(readw(I2C_CON) & I2C_CON_MST)) {
			udelay(1000);
			/* have to read to clear intrrupt */
			writew(0xFFFF, I2C_STAT);

			if (--eout == 0)
				/* better leave with error than hang */
				break;
		}
	}

	flush_fifo();
	writew(0xFFFF, I2C_STAT);
	writew(0, I2C_CNT);
	return 0;
}

static u32 wait_for_bb (void)
{
	int timeout = 10;
	u32 stat;

	writew(0xFFFF, I2C_STAT);	 /* clear current interruts...*/
	while ((stat = readw (I2C_STAT) & I2C_STAT_BB) && timeout--) {
		writew (stat, I2C_STAT);
		udelay (50000);
	}

	if (timeout <= 0) {
			stat |= I2C_TIMEOUT;
	}
	writew(0xFFFF, I2C_STAT);	 /* clear delayed stuff*/
	return stat;
}

static u32 wait_for_status_mask(u16 mask)
{
	u32 status;
	int timeout = 10;

	do {
		udelay(1000);
		status = readw(I2C_STAT);
	} while (!(status & mask) && timeout--);

	if (timeout <= 0) {
		writew(0xFFFF, I2C_STAT);
		status |= I2C_TIMEOUT;
	}
	return status;
}

int i2c_set_bus_num(unsigned int bus)
{
	if ((bus < 0) || (bus >= I2C_BUS_MAX)) {
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
