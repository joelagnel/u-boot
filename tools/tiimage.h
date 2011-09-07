/*
 * tiimage.h
 *
 * mkimage extension for Texas Instruments
 *
 * Copyright (C) 2010, Texas Instruments, Incorporated
 *
 * Author:
 * Mansoor Ahamed  <mansoor.ahamed@ti.com>
 *
 * Referenced tools/imximage.h
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef _TIIMAGE_H_
#define _TIIMAGE_H_

#if defined(CONFIG_AM335X)
struct ti_header {
#if !defined(CONFIG_AM335X_PERIPHERAL_BOOT) && defined(CONFIG_AM335X_MIN_CONFIG)
	uint32_t image_size;
	uint32_t load_addr;
#endif
};
#else
struct ti_header {
	uint32_t image_size;
	uint32_t load_addr;
};
#endif

#endif /* _TIIMAGE_H_ */
