/*
 * Copyright (C) 2009, Texas Instruments, Incorporated
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

#ifndef __CONFIG_TI816X_EVM_H
#define __CONFIG_TI816X_EVM_H

#define CONFIG_ARMV7

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/hardware.h>

/* U-Boot default commands */
#include <config_cmd_default.h>

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (32 * 1024))
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for
						   initial data */

/* Only one the following two options (DDR3/DDR2) should be enabled */
//#define CONFIG_TI816X_EVM_DDR3			/* Configure DDR3 in U-Boot */
#define CONFIG_TI816X_EVM_DDR2				/* Configure DDR2 in U-Boot */
#define CONFIG_TI816X_TWO_EMIF		1
#define CONFIG_MISC_INIT_R		1
#define CONFIG_TI816X_ASCIIART		1	/* The eye */

#define CONFIG_CMDLINE_TAG        	1	/* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS  	1
#define CONFIG_INITRD_TAG	  	1	/* Required for ramdisk support */

#define CONFIG_CMD_ASKENV
#define CONFIG_VERSION_VARIABLE
#define CONFIG_BOOTDELAY		3	/* set to negative value for no autoboot */
#define CONFIG_SYS_AUTOLOAD		"no"
#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0" \
	"bootfile=uImage\0" \
	"ramdisk_file=ramdisk.gz\0" \

/*
 * Miscellaneous configurable options
 */
/* allow overwriting serial config and ethaddr */
#define CONFIG_ENV_OVERWRITE
/* Undef to save memory */
#define CONFIG_SYS_LONGHELP
/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT		"TI8168_EVM#"
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		512
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)
/* max number of command args */
#define CONFIG_SYS_MAXARGS		32
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
/* memtest works on 8 MB in DRAM after skipping 32MB from start addr of ram disk*/
#define CONFIG_SYS_MEMTEST_START	(PHYS_DRAM_1 + (64 *1024 *1024))
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START \
					+ (8 * 1024 * 1024))

#undef  CONFIG_SYS_CLKS_IN_HZ				/* everything, incl board info, in Hz */
#define CONFIG_SYS_LOAD_ADDR		0x81000000  	/* Default load address */

/* Hardware related */

/**
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS		2		/* we have 2 banks of DRAM */
#define PHYS_DRAM_1			0x80000000	/* DRAM Bank #1 */
#define PHYS_DRAM_1_SIZE		0x40000000	/* 1 GB */
#define PHYS_DRAM_2			0xC0000000	/* DRAM Bank #2 */
#define PHYS_DRAM_2_SIZE		0x40000000	/* 1 GB */


/**
 * Clock related defines
 */
#define V_OSCK				27000000
#define V_SCLK				(V_OSCK >> 1)
#define CONFIG_SYS_TIMERBASE		0x4802E000
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000        	/* 1ms clock */

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		(48000000)
#define CONFIG_SYS_NS16550_COM1		0x48024000	/* Base EVM has UART2 */

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 110, 300, 600, 1200, 2400, \
4800, 9600, 14400, 19200, 28800, 38400, 56000, 57600, 115200 }

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1			1
#define CONFIG_CONS_INDEX		1

#if defined(CONFIG_NO_ETH)
# undef CONFIG_CMD_NET
#else
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_PING
#endif

#if defined(CONFIG_SYS_NO_FLASH)
#define CONFIG_ENV_IS_NOWHERE
#endif

/* Unsupported features */
#undef CONFIG_USE_IRQ

/* additions for new relocation code, must added to all boards */
#undef CONFIG_SYS_ARM_WITHOUT_RELOC /* This board is tested with relocation support */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		((SRAM0_START + SRAM0_SIZE - SRAM_GPMC_STACK_SIZE) - CONFIG_SYS_GBL_DATA_SIZE)

#endif	  /* ! __CONFIG_TI816X_EVM_H */

