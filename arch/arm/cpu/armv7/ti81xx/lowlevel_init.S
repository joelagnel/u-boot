/*
 * lowlevel_init.S
 *
 * TI81XX low level initialization.
 *
 * Copyright (C) 2010, Texas Instruments, Incorporated
 *
 * Initial Code by:
 * Mansoor Ahamed  <mansoor.ahamed@ti.com>
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

#include <config.h>
#include <asm/arch/hardware.h>
#include <asm/arch/cpu.h>

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE	/* Load address (RAM) */

/*****************************************************************************
 * lowlevel_init: - Platform low level init.
 * Corrupted Register : r0, r1, r2, r3, r4, r5, r6
 ****************************************************************************/
.globl lowlevel_init
lowlevel_init:

	/* The link register is saved in ip by start.S */
	mov r6, ip
	/* check if we are already running from RAM */
	ldr r2, _lowlevel_init
	ldr r3, _TEXT_BASE
	sub r4, r2, r3
	sub r0, pc, r4
	/* require dummy instr or subtract pc by 4 instead i'm doing stack init */
	ldr sp, SRAM_STACK
mark1:
	ldr r5, _mark1
	sub r5, r5, r2 /* bytes between mark1 and lowlevel_init */
	sub r0, r0, r5 /* r0 <- _start w.r.t current place of execution */
	mov r10, #0x0 /* r10 has in_ddr used by s_init() */

	ands r0, r0, #0xC0000000 /* MSB 2 bits <> 0 then we are in ocmc or DDR */
	cmp r0, #0x80000000
	bne s_init_start
	mov r10, #0x01
	b s_init_start

s_init_start:
	mov r0, r10 /* passing in_ddr in r0 */
	bl s_init
	/* back to arch calling code */
	mov pc, r6
	/* the literal pools origin */
	.ltorg

_mark1:
	.word mark1
_lowlevel_init:
	.word lowlevel_init
_s_init_start:
	.word s_init_start

SRAM_STACK:
	.word (SRAM0_START + SRAM0_SIZE - SRAM_GPMC_STACK_SIZE)

