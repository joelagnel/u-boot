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

#define CONFIG_AM335X_HSMMC_INSTANCE	0	/* 0 - MMC0, 1 - MMC1 */

/* In the 1st stage we have just 110K, so cut down wherever possible */
#ifdef CONFIG_AM335X_MIN_CONFIG
#define CONFIG_CMD_MEMORY	/* for mtest */
#undef CONFIG_GZIP
#undef CONFIG_ZLIB

#undef CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADY	/* loady */
#define CONFIG_SETUP_PLL
#define CONFIG_AM335X_CONFIG_DDR
#define CONFIG_AM335X_EVM_IS_13x13	0	/* 1 = 13x13, 0 = 15x15 */
#define CONFIG_ENV_SIZE			0x400
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (8 * 1024))
#define CONFIG_SYS_PROMPT		"TI-MIN#"

/* set to negative value for no autoboot */
#define CONFIG_BOOTDELAY		3

# if defined(CONFIG_SPI_BOOT)
#  define CONFIG_SPI			1
#  define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0" \
	"spi_bus_no=0\0" \
	"bootcmd=sf probe ${spi_bus_no}:0;sf read 0x81000000 0x20000 0x40000; \
	go 0x81000000\0" \

# elif defined(CONFIG_NAND_BOOT)
#define CONFIG_NAND		1
#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0" \
	"bootcmd=nand read 0x81000000 0x80000 0x40000; go 0x81000000\0"
#endif

#if defined(CONFIG_NOR_BOOT)
#define CONFIG_NOR
#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0" \
	"bootcmd=cp.b 0x8020000 0x81000000 0x40000; go 0x81000000\0"
#endif

#if defined(CONFIG_SD_BOOT)		/* Autoload the 2nd stage from SD */
#define CONFIG_MMC			1
#define CONFIG_EXTRA_ENV_SETTINGS \
	 "verify=yes\0" \
	"bootcmd=mmc init 0; fatload mmc 0 0x80800000 u-boot.bin; go 0x80800000\0"
#endif

#else /* u-boot Full / Second stage u-boot */

#include <config_cmd_default.h>

/* 1st stage would have done the basic init */
#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (32 * 1024))
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"AM335X_EVM#"
/* Use HUSH parser to allow command parsing */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_TAG		1 /* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1 /* Required for ramdisk support */

/* set to negative value for no autoboot */
#define CONFIG_BOOTDELAY		3

#define CONFIG_MMC			1

#ifndef CONFIG_NOR_BOOT
#define CONFIG_NAND
#else
#define CONFIG_NOR
#endif

#define CONFIG_SPI			1

#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0" \
	"bootfile=uImage\0" \
	"brd_mem=62M\0" \
	"loadaddr=0x81000000\0" \
	"script_addr=0x80900000\0" \
	"console=ttyO0,115200n8\0" \
	"mmc_root=/dev/mmcblk0p2 rw\0" \
	"nand_root=/dev/mtdblock4 rw\0" \
	"spi_root=/dev/mtdblock4 rw\0" \
	"nor_root=/dev/mtdblock3 rw\0" \
	"mmc_root_fs_type=ext2 rootwait\0" \
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
	"ip_method=dhcp\0" \
	"loadbootscript=fatload mmc 0 ${script_addr} boot.scr\0" \
	"bootscript= echo Running bootscript from MMC/SD to set the ENV...; " \
		"source ${script_addr}\0" \
	"mmc_load_uimage=fatload mmc 0 ${loadaddr} ${bootfile}\0" \
	"bootargs_defaults=setenv bootargs " \
		"console=${console} " \
		 "mem=${brd_mem}\0" \
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
	"if mmc init 0; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run mmc_load_uimage; then " \
				"run mmc_boot; " \
			"else " \
				"run nand_boot; " \
			"fi; " \
		"fi; " \
	"else " \
		"run nand_boot; " \
	"fi"



#endif /* CONFIG_AM335X_MIN_CONFIG */

#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for
						   initial data */
#define CONFIG_MISC_INIT_R		1
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

#undef  CONFIG_SYS_CLKS_IN_HZ		/* everything, incl board info, in Hz */
#define CONFIG_SYS_LOAD_ADDR		0x81000000 /* Default load address */
#define CONFIG_SYS_HZ			1000 /* 1ms clock */

 /* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1		/*  1 bank of DRAM */
#define PHYS_DRAM_1			0x80000000	/* DRAM Bank #1 */
#define PHYS_DRAM_1_SIZE		0x10000000 /*(0x80000000 / 8) 256 MB */

 /* Platform/Board specific defs */
#define CONFIG_SYS_CLK_FREQ		24000000
#define CONFIG_SYS_TIMERBASE		0x48040000	/* Use Timer2 */

/* NS16550 Configuration */
#define CONFIG_SERIAL_MULTI     	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		(48000000)
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CONFIG_SYS_NS16550_COM4		0x481A6000	/* UART3 on IA BOard */

#define CONFIG_BAUDRATE		115200
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
#define CONFIG_ENV_IS_IN_NAND		1
#define CONFIG_SYS_MAX_FLASH_SECT	520 /* max no of sectors in a chip */
#define CONFIG_SYS_MAX_FLASH_BANKS	2 /* max no of flash banks */
#define CONFIG_SYS_MONITOR_LEN	(256 << 10) /* Reserve 2 sectors */
#define CONFIG_SYS_FLASH_BASE		boot_flash_base
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define MNAND_ENV_OFFSET		0x260000 /* environment starts here */
#define CONFIG_SYS_ENV_SECT_SIZE	boot_flash_sec
#define CONFIG_ENV_OFFSET		boot_flash_off
#define CONFIG_ENV_ADDR			MNAND_ENV_OFFSET

#ifndef __ASSEMBLY__
extern unsigned int boot_flash_base;
extern volatile unsigned int boot_flash_env_addr;
extern unsigned int boot_flash_off;
extern unsigned int boot_flash_sec;
extern unsigned int boot_flash_type;
#endif
#endif /* NAND support */

/*
 * NOR Size = 16 MB
 * No.Of Sectors/Blocks = 128
 * Sector Size = 128 KB
 * Word lenght = 16 bits
 */
#if defined(CONFIG_NOR)
# undef CONFIG_ENV_IS_NOWHERE
# ifdef CONFIG_SYS_MALLOC_LEN
#  undef CONFIG_SYS_MALLOC_LEN
# endif
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
#define BOARD_LATE_INIT               1
#define CONFIG_OMAP3_SPI
#define CONFIG_MTD_DEVICE
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_SPEED	(75000000)
#endif

/* ENV in SPI */
#if defined(CONFIG_SPI_ENV)
# undef CONFIG_ENV_IS_NOWHERE
# define CONFIG_ENV_IS_IN_SPI_FLASH   1
# define CONFIG_SYS_FLASH_BASE       (0)
# define SPI_FLASH_ERASE_SIZE        (4 * 1024) /* sector size of SPI flash */
# define CONFIG_SYS_ENV_SECT_SIZE    (2 * SPI_FLASH_ERASE_SIZE) /* env size */
# define CONFIG_ENV_SECT_SIZE        (CONFIG_SYS_ENV_SECT_SIZE)
# define CONFIG_ENV_OFFSET           (96 * SPI_FLASH_ERASE_SIZE)
# define CONFIG_ENV_ADDR             (CONFIG_ENV_OFFSET)
# define CONFIG_SYS_MAX_FLASH_SECT   (1024) /* no of sectors in SPI flash */
# define CONFIG_SYS_MAX_FLASH_BANKS  (1)
#endif /* SPI support */

/* I2C */
# define CONFIG_I2C
# define CONFIG_CMD_I2C
# define CONFIG_HARD_I2C		1
# define CONFIG_I2C_MULTI_BUS		1
# define CONFIG_SYS_I2C_SPEED		100000
# define CONFIG_SYS_I2C_SLAVE		1
# define CONFIG_SYS_I2C_BUS		0
# define CONFIG_SYS_I2C_BUS_SELECT	1
# define CONFIG_DRIVER_TI81XX_I2C	1

/* HSMMC support */
#ifdef CONFIG_MMC
#if (CONFIG_AM335X_HSMMC_INSTANCE == 0)
# define CONFIG_AM335X_HSMMC_BASE    0x48060100
#else
# define CONFIG_AM335X_HSMMC_BASE    0x481D8100
#endif
# define CONFIG_OMAP3_MMC	1
# define CONFIG_CMD_MMC		1
# define CONFIG_DOS_PARTITION	1
# define CONFIG_CMD_FAT		1
#endif

/* Unsupported features */
#undef CONFIG_USE_IRQ


 #endif	/* ! __CONFIG_AM335X_EVM_H */
