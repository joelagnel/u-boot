#
# config.mk
#
# Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation version 2.
#
# This program is distributed "as is" WITHOUT ANY WARRANTY of any
# kind, whether express or implied; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#

#sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp
ifdef CONFIG_AM335X_MIN_CONFIG
CONFIG_PRELOADER = 1
CONFIG_SYS_TEXT_BASE = 0x80700000
else
CONFIG_SYS_TEXT_BASE = 0x80800000
endif

TI_IMAGE = u-boot.min.uart
# This will be used by mkimage extension to select header for image
TI_DEVICE = ti81xx
# ROM code will load u-boot to this address
TI_LOAD_ADDR = 0x402f0400
# TEXT_BASE for the 2 stages is different and comes from the Makefile
CROSS_COMPILE:= arm-none-linux-gnueabi-
#LDSCRIPT:=board/ti/am335x/u-boot.lds
