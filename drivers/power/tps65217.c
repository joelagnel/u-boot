/*
 * tps65217.c
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
#include <tps65217.h>

/**
 * tps65217_reg_read() - Generic function that can read a TPS65217 register
 * @src_reg:          Source register address
 * @src_val:          Address of destination variable
 */

unsigned char tps65217_reg_read(uchar src_reg, uchar *src_val)
{
	if (i2c_read(TPS65217_CHIP_PM, src_reg, 1, src_val, 1))
		return 1;
	return 0;
}

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
