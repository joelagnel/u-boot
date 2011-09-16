/*
 * tiimage.c
 *
 * mkimage extension for Texas Instruments processors
 *
 * Copyright (C) 2010, Texas Instruments, Incorporated
 *
 * Author:
 * Mansoor Ahamed  <mansoor.ahamed@ti.com>
 *
 * Referenced tools/imximage.c
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

/* Required to obtain the getline prototype from stdio.h */
#define _GNU_SOURCE

/* TODO: Instead of device macros we should use -n option to get device info
 * and APIs associated with that TI device.
 */

#ifndef __ASSEMBLY__
#define __ASSEMBLY__
#endif
#define __ASM_STUB_PROCESSOR_H__
#include <config.h>
#undef CONFIG_LMB
#undef __ASSEMBLY__

#include "mkimage.h"
#include <image.h>
#include "tiimage.h"

static struct ti_header tiimage_header;

static int tiimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_TIIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int tiimage_verify_header(uint8_t *ptr, int image_size,
			struct mkimage_params *params)
{
	return 0;
}

static void ti81xximage_print_header(const void *ptr)
{
#if !defined(CONFIG_AM335X_PERIPHERAL_BOOT) && \
	!defined(CONFIG_TI_DUMMY_HEADER)
	struct ti_header *ti_hdr = (struct ti_header *) ptr;
	printf("Image Type:   Texas Instruments ti81xx Boot Image\n");
	printf("Image Size:   ");
	genimg_print_size(ti_hdr->image_size);
	printf("Load Address: %08x\n", ti_hdr->load_addr);
	printf("Entry Point:  %08x\n", ti_hdr->load_addr);
#endif
}

#if defined(CONFIG_AM335X) && defined(CONFIG_AM335X_MIN_CONFIG)
static void am335ximage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct mkimage_params *params)
{
	struct ti_header *hdr = (struct ti_header *)ptr;
#if !defined(CONFIG_AM335X_PERIPHERAL_BOOT)
	FILE *data_fp = NULL;
	uint32_t data_size = 0;

	/* Set default offset */
	hdr->load_addr = params->ep;

	if ((data_fp = fopen(params->datafile, "r")) == NULL) {
		printf("Data FILE [%s] open error.\n", params->datafile);
		return;
	}

	/* calculate image size */
	fseek(data_fp, 0, SEEK_END);
	data_size = ftell(data_fp);
	hdr->image_size = data_size;
	fclose(data_fp);
#endif
	/* spi image has to be endian swapped */
#if defined(CONFIG_TI81XX_SPI_BOOT)
	if (strstr(params->imagefile, "min.spi") != NULL) {
		/* generate spi image */
		ti81xximage_spi(hdr, sizeof(struct ti_header),
			params->datafile, "u-boot.min.spi");
	}
#endif
}
#else
static void ti_dummy_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct mkimage_params *params)
{
	/* Dummy function for set header */
}
#endif

int tiimage_check_params(struct mkimage_params *params)
{
	if (!params || !params->dflag || !params->eflag)
		return -1;

	if (!strlen(params->datafile)) {
		fprintf(stderr, "Error: %s - Input Image file not specified, "
			"it is needed for ti image generation\n",
			params->cmdname);
		return -1;
	}
	return 0;
}

/*
 * tiimage parameters
 */
static struct image_type_params tiimage_params = {
	.name		= "Texas Instruments Boot Image support",
	.header_size	= sizeof(struct ti_header),
	.hdr			= (void *)&tiimage_header,
	.check_image_type = tiimage_check_image_types,
	.verify_header	= tiimage_verify_header,
	.print_header	= ti81xximage_print_header,
#if defined(CONFIG_AM335X) && defined(CONFIG_AM335X_MIN_CONFIG)
	.set_header		= am335ximage_set_header,
#else
	.set_header		= ti_dummy_set_header,
#endif
	.check_params	= tiimage_check_params,
};

void init_ti_image_type(void)
{
	mkimage_register(&tiimage_params);
}

