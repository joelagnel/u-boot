/*
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef __CONFIG_AM335X_EVM_H
#define __CONFIG_AM335X_EVM_H

#define CONFIG_AM335X
#define CONFIG_TI81XX
#define CONFIG_SYS_NO_FLASH
#define CONFIG_NAND_ENV

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/hardware.h>

#define CONFIG_AM335X_HSMMC_INSTANCE	0	/* 0 - MMC0, 1 - MMC1 */

#include <config_cmd_default.h>

#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (32 * 1024))
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"U-Boot# "
/* Use HUSH parser to allow command parsing */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG		/* Required for ramdisk support */

/* set to negative value for no autoboot */
#define CONFIG_BOOTDELAY		1

#define CONFIG_MMC
#define CONFIG_NAND
#define CONFIG_SPI

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootfile=uImage\0" \
	"loadaddr=0x82000000\0" \
	"script_addr=0x81900000\0" \
	"console=ttyO0,115200n8\0" \
	"mmc_dev=0\0" \
	"mmc_root=/dev/mmcblk0p2 ro\0" \
	"nand_root=/dev/mtdblock4 rw\0" \
	"spi_root=/dev/mtdblock4 rw\0" \
	"nor_root=/dev/mtdblock3 rw\0" \
	"mmc_root_fs_type=ext3 rootwait\0" \
	"nand_root_fs_type=jffs2\0" \
	"spi_root_fs_type=jffs2\0" \
	"nor_root_fs_type=jffs2\0" \
	"nand_src_addr=0x2e0000\0" \
	"spi_src_addr=0x62000\0" \
	"nor_src_addr=0x08080000\0" \
	"nand_img_siz=0x440000\0" \
	"spi_img_siz=0x280000\0" \
	"nor_img_siz=0x280000\0" \
	"spi_bus_no=0\0" \
	"rootpath=/export/rootfs\0" \
	"nfsopts=nolock\0" \
	"static_ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}" \
			"::off\0" \
	"ip_method=none\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=fatload mmc ${mmc_dev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t $loadaddr $filesize\0" \
	"mmc_load_uimage_fat=fatload mmc ${mmc_dev} ${loadaddr} ${bootfile}\0" \
	"mmc_load_uimage=ext2load mmc 0:2 ${loadaddr} /boot/${bootfile}\0" \
	"bootargs_defaults=setenv bootargs " \
		"console=${console}\0" \
	"mmc_args=run bootargs_defaults;" \
		"setenv bootargs ${bootargs} " \
		"root=${mmc_root} " \
		"rootfstype=${mmc_root_fs_type} ip=${ip_method}\0" \
	"nand_args=run bootargs_defaults;" \
		"setenv bootargs ${bootargs} " \
		"root=${nand_root} noinitrd " \
		"rootfstype=${nand_root_fs_type} ip=${ip_method}\0" \
	"spi_args=run bootargs_defaults;" \
		"setenv bootargs ${bootargs} " \
		"root=${spi_root} " \
		"rootfstype=${spi_root_fs_type} ip=${ip_method}\0" \
	"nor_args=run bootargs_defaults;" \
		"setenv bootargs ${bootargs} " \
		"root={nor_root} " \
		"rootfstype=${nor_root_fs_type} ip=${ip_method}\0" \
	"net_args=run bootargs_defaults;" \
		"setenv bootargs ${bootargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} rw " \
		"ip=${ip_method}\0" \
	"mmc_boot=run mmc_args; " \
		"run mmc_load_uimage; " \
		"bootm ${loadaddr}\0" \
	"nand_boot=echo Booting from nand ...; " \
		"run nand_args; " \
		"nand read.i ${loadaddr} ${nand_src_addr} ${nand_img_siz}; " \
		"bootm ${loadaddr}\0" \
	"spi_boot=echo Booting from spi ...; " \
		"run spi_args; " \
		"sf probe ${spi_bus_no}:0; " \
		"sf read ${loadaddr} ${spi_src_addr} ${spi_img_siz}; " \
		"bootm ${loadaddr}\0" \
	"nor_boot=echo Booting from NOR ...; " \
		"run nor_args; " \
		"cp.b ${0x08080000} ${loadaddr} ${nor_img_siz}; " \
		"bootm ${loadaddr}\0" \
	"net_boot=echo Booting from network ...; " \
		"setenv autoload no; " \
		"dhcp; " \
		"tftp ${loadaddr} ${bootfile}; " \
		"run net_args; " \
		"bootm ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"if mmc rescan; then " \
		"echo SD/MMC found on device ${mmc_dev};" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv};" \
			"run importbootenv;" \
		"fi;" \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...;" \
			"run uenvcmd;" \
		"fi;" \
		"if run mmc_load_uimage; then " \
			"run mmc_args;" \
			"bootm ${loadaddr};" \
		"fi;" \
	"fi;" \
	"run nand_boot;" \

#define CONFIG_MISC_INIT_R
#define CONFIG_SYS_AUTOLOAD		"yes"
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_ECHO

/* max number of command args */
#define CONFIG_SYS_MAXARGS		32
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		512
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
/*
 * memtest works on 8 MB in DRAM after skipping 32MB from
 * start addr of ram disk
 */
#define CONFIG_SYS_MEMTEST_START	(PHYS_DRAM_1 + (64 * 1024 * 1024))
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START \
					+ (8 * 1024 * 1024))

#define CONFIG_SYS_LOAD_ADDR		0x81000000 /* Default load address */

 /* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1		/* 1 bank of DRAM */
#define PHYS_DRAM_1			0x80000000	/* DRAM Bank #1 */
#define PHYS_DRAM_1_SIZE		0x10000000	/* 256 MiB */

#define CONFIG_SYS_SDRAM_BASE		PHYS_DRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	SRAM0_START
#define CONFIG_SYS_INIT_RAM_SIZE	SRAM0_SIZE
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

/* Defines for SPL */
#define CONFIG_SPL
#define CONFIG_SPL_TEXT_BASE		0x402F0400
#define CONFIG_SPL_MAX_SIZE		(46 * 1024)
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.img"
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_YMODEM_SUPPORT
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/omap-common/u-boot-spl.lds"

/* NAND boot config */
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14

#define CONFIG_SYS_NAND_ECCSTEPS	4
#define	CONFIG_SYS_NAND_ECCTOTAL	(CONFIG_SYS_NAND_ECCBYTES * \
						CONFIG_SYS_NAND_ECCSTEPS)

#define	CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80800000 should not be used for any
 * other needs.
 */
#define CONFIG_SYS_TEXT_BASE		0x80800000 /* 0x80100000 */

/* Since SPL did all of this for us, we don't need to do it twice. */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/**
 * Clock related defines
 */
#define V_OSCK				24000000 /* Clock output from T2 */
#define V_SCLK				(V_OSCK >> 1)
#define CONFIG_SYS_TIMERBASE		0x48040000	/* Use Timer2 */
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		(48000000)
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CONFIG_SYS_NS16550_COM4		0x481A6000	/* UART3 on IA BOard */

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 110, 300, 600, 1200, 2400, \
4800, 9600, 14400, 19200, 28800, 38400, 56000, 57600, 115200 }

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1			1
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_CONSOLE_INFO_QUIET

#if defined(CONFIG_NO_ETH)
# undef CONFIG_CMD_NET
#else
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_PING
#endif

#if defined(CONFIG_CMD_NET)
# define CONFIG_DRIVER_TI_CPSW
# define CONFIG_MII
# define CONFIG_BOOTP_DEFAULT
# define CONFIG_BOOTP_DNS
# define CONFIG_BOOTP_DNS2
# define CONFIG_BOOTP_SEND_HOSTNAME
# define CONFIG_BOOTP_GATEWAY
# define CONFIG_BOOTP_SUBNETMASK
# define CONFIG_NET_RETRY_COUNT		10
# define CONFIG_NET_MULTI
# define CONFIG_PHY_GIGE
#endif

#if defined(CONFIG_SYS_NO_FLASH)
# define CONFIG_ENV_IS_NOWHERE
#endif

/* NAND support */
#ifdef CONFIG_NAND
#define CONFIG_CMD_NAND
#define CONFIG_NAND_TI81XX
#define GPMC_NAND_ECC_LP_x16_LAYOUT	1
#define NAND_BASE			(0x08000000)
#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access nand at */
							/* CS0 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */
#endif							/* devices */

/* ENV in NAND */
#if defined(CONFIG_NAND_ENV)
#undef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_SYS_MAX_FLASH_SECT	520 /* max no of sectors in a chip */
#define CONFIG_SYS_MAX_FLASH_BANKS	2 /* max no of flash banks */
#define CONFIG_SYS_MONITOR_LEN		(256 << 10) /* Reserve 2 sectors */
#define CONFIG_SYS_FLASH_BASE		PISMO1_NAND_BASE
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define MNAND_ENV_OFFSET		0x260000 /* environment starts here */
#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		MNAND_ENV_OFFSET
#define CONFIG_ENV_ADDR			MNAND_ENV_OFFSET
#endif /* NAND support */

/*
 * NOR Size = 16 MB
 * No.Of Sectors/Blocks = 128
 * Sector Size = 128 KB
 * Word lenght = 16 bits
 */
#if defined(CONFIG_NOR)
# undef CONFIG_ENV_IS_NOWHERE
# undef CONFIG_SYS_MALLOC_LEN
# define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1
# define CONFIG_SYS_MALLOC_LEN		(0x100000)
# define CONFIG_SYS_FLASH_CFI
# define CONFIG_FLASH_CFI_DRIVER
# define CONFIG_FLASH_CFI_MTD
# define CONFIG_SYS_MAX_FLASH_SECT	128
# define CONFIG_SYS_MAX_FLASH_BANKS	1
# define CONFIG_SYS_FLASH_BASE		(0x08000000)
# define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
# define CONFIG_ENV_IS_IN_FLASH	1
# define NOR_SECT_SIZE			(128 * 1024)
# define CONFIG_SYS_ENV_SECT_SIZE	(NOR_SECT_SIZE)
# define CONFIG_ENV_SECT_SIZE		(NOR_SECT_SIZE)
# define CONFIG_ENV_OFFSET		(8 * NOR_SECT_SIZE) /* After 1 MB */
# define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + \
	 CONFIG_ENV_OFFSET)
# define CONFIG_MTD_DEVICE
#endif	/* NOR support */

/* SPI support */
#ifdef CONFIG_SPI
#define BOARD_LATE_INIT
#define CONFIG_OMAP3_SPI
#define CONFIG_MTD_DEVICE
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_SPEED		(75000000)
#endif

/* ENV in SPI */
#if defined(CONFIG_SPI_ENV)
# undef CONFIG_ENV_IS_NOWHERE
# define CONFIG_ENV_IS_IN_SPI_FLASH
# define CONFIG_SYS_FLASH_BASE		(0)
# define SPI_FLASH_ERASE_SIZE		(4 * 1024) /* sector size */
# define CONFIG_SYS_ENV_SECT_SIZE	(2 * SPI_FLASH_ERASE_SIZE)
# define CONFIG_ENV_SECT_SIZE		(CONFIG_SYS_ENV_SECT_SIZE)
# define CONFIG_ENV_OFFSET		(96 * SPI_FLASH_ERASE_SIZE)
# define CONFIG_ENV_ADDR		(CONFIG_ENV_OFFSET)
# define CONFIG_SYS_MAX_FLASH_SECT	(1024) /* # of sectors in SPI flash */
# define CONFIG_SYS_MAX_FLASH_BANKS	(1)
#endif /* SPI support */

/* I2C */
#define CONFIG_I2C
#define CONFIG_CMD_I2C
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_DRIVER_TI81XX_I2C

/* HSMMC support */
#ifdef CONFIG_MMC
#if (CONFIG_AM335X_HSMMC_INSTANCE == 0)
#define CONFIG_AM335X_HSMMC_BASE	0x48060100
#else
#define CONFIG_AM335X_HSMMC_BASE	0x481D8100
#endif
#define CONFIG_GENERIC_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_CMD_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#endif

/* Unsupported features */
#undef CONFIG_USE_IRQ

#endif	/* ! __CONFIG_AM335X_EVM_H */
