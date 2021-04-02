/*
 *  armboot - Startup Code for ARM920 CPU-core
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <common.h>
#include <config.h>

/*
 *************************************************************************
 *
 * Startup Code (called from the ARM reset exception vector)
 *
 * do important init only if we don't start from memory!
 * relocate armboot to ram
 * setup stack
 * jump to second stage
 *
 *************************************************************************
 */
	.globl	reset

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r0, r0, #0xd3
	msr	cpsr, r0

	/* turn off the watchdog */

# if defined(CONFIG_S3C2400)
#  define pWTCON	0x15300000
#  define INTMSK	0x14400008	/* Interrupt-Controller base addresses */
#  define CLKDIVN	0x14800014	/* clock divisor register */
#elif defined(CONFIG_S3C2440)
#  define pWTCON	0x53000000
#  define INTMSK	0x4A000008	/* Interrupt-Controller base addresses */
#  define INTSUBMSK	0x4A00001C
#  define CLKDIVN	0x4C000014	/* clock divisor register */
# endif

#define CLKCON              0x4c00000c 
#define GPIO_CLK_OFFSET     13
#define GPBCON              0x56000010
#define GPBDAT              0x56000014
#define GPBUP               0x56000018

	ldr	r0, =pWTCON
	mov	r1, #0x0
	str	r1, [r0]

	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 */
	ldr	r1, =0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]

    /* duanshuai add start
     */
    ldr r1, =0x7fff
    ldr r0, =INTSUBMSK
    str r1, [r0]
    /* duanshuai add end*/

    /*duanshuai add start*/
# if defined(CONFIG_S3C2440)
#   define MPLLCON 0x4c000004
#   define UPLLCON 0x4c000008
//#   define S3C2440_MPLL_400M   ((92 << 12) | (1 << 4) | (1))
#   define S3C2440_MPLL_400M   ((127 << 12) | (2 << 4) | (1))
#   define S3C2440_UPLL_48M    ((56 << 12) | (2 << 4) | (2))
    /* FCLK:HCLK:PCLK = 1:4:8 */
    ldr r0, =CLKDIVN
    mov r1, #5
    str r1, [r0]
    /* MMU Set Async Bus Mode */
    mrc p15, 0, r1, c1, c0, 0
    orr r1, r1, #0xC0000000
    mcr p15, 0, r1, c1, c0, 0
    
    /* Set UPLL */
    ldr r0, =UPLLCON
    ldr r1, =S3C2440_UPLL_48M
    str r1, [r0]
 
    /* Set MPLL*/
    ldr r0, =MPLLCON
    ldr r1, =S3C2440_MPLL_400M
    str r1, [r0]

    
    //设置GPB5，6，7，8为输出
//    ldr r1, =GPBCON
//    ldr r2, =0x55555555
//    str r2, [r1]
//
//    ldr r1, =GPBDAT
//    ldr r2, =0xffffffff
//    str r2, [r1]
#endif	/* CONFIG_S3C24X0 */

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	cpu_init_crit
#endif

    //准备进行代码重定位
    //mov r0, #0
    //bl led_ctrl_on
    /*****CHECK CODE POSITION START*************/
#    adr r0, _start  /* r0 <- current position of code   */
#ldr r0, =_start  /* r0 <- current position of code   */
#define DUANSHUAI   1
#ifdef DUANSHUAI
    //临时的解决方法，不通用。
    //通过判断固定位置的数值来得知是否程序已经重定位
    //在重定位之后将固定的数值写入到固定的地址中
    //在进入主程序之前需要将该地址的值进行清除，不然reset指令执行后
    //程序会认为已经重定位成功。
    //ldr r0, =0x33FFFF00
    //ldr r1, [r0]
    //cmp r1, #0x78
    //beq gotomain

    //系统启动时程序前4k运行在stepstoning内，reset的地址小于0x30000000
    //当程序copy完成后，跳转到ram中运行时，reset的值大于0x30000000
    ldr r0, =0x30000000
    adr r1, reset
    cmp r0, r1
    blt gotomain
    //adr r0, _start
    //ldr r1, =CONFIG_SYS_TEXT_BASE /* test if we run from flash or RAM */
    //cmp r0, r1          /* don't reloc during debug         */
    //beq gotomain 
    /*****CHECK CODE POSITION END ************** * */

    //mov r0, #1
    //bl led_ctrl_on
    /*******   CHECK BOOT FLASH START *******************/
    //ldr r1,=0x4000003c /* ldr r1,=0x4000003c */
    //mov r0,#0                       /*ADDRESS OF INTERNAL SRAM 0X4000003C*/
    //str r0,[r1]

    //mov r1,#0x3c                    /*ADDRESS OF MEN 0X0000003C*/
    //ldr r0,[r1]
    //cmp r0, #0
    //bne gotomain 

    //ldr r0,=(0xdeadbeef)
    //ldr r1,=0x4000003c
    //str r0,[r1]
    /***********   CHECK BOOT FLASH END  ***************/
    //mov r0, #2
    //bl led_ctrl_on

/* ***********  NAND BOOT  ********************/
#define LENGTH_UBOOT 0x40000
#define NAND_CTL_BASE 0x4e000000
#ifdef CONFIG_S3C2440
#define oNFCONF 0x0
#define oNFCONT 0x4
#define oNFCMD  0x8
#define oNFSTAT 0x20

    mov r1,#NAND_CTL_BASE 
    ldr r2,=((7<<12)|(7<<8)|(7<<4)|(0<<0))
    str r2,[r1,#oNFCONF]
    ldr r2,[r1,#oNFCONF]

    ldr r2,=((1<<4)|(0<<1)|(1<<0))          @ACTIVE LOW CE CONTROL
    str r2,[r1,#oNFCONT]
    ldr r2,[r1,#oNFCONT]

    //ldr r2,=(1<<2)                           @RNB CLEAR
    ldr r2,=(0x6)                           @RNB CLEAR
    str r2,[r1,#oNFSTAT]
    ldr r2,[r1,#oNFSTAT]

    mov r2,#0xff                            @RESET COMMAND
    strb r2,[r1,#oNFCMD]

    mov r3,#0                               @WAI
nand1: 
    add r3,r3,#0x1
    cmp r3,#0xa
    blt nand1

nand2:
    ldr r2,[r1,#oNFSTAT]                    @WAIT READY 
    tst r2,#0x4
    //tst r2,#0x1
    beq nand2

    ldr r2,[r1,#oNFCONT]
    orr r2,r2,#0x2                          @FLASH MEMORY CHIP DISABLE
    str r2,[r1,#oNFCONT]

    ldr sp, =CONFIG_SYS_INIT_SP_ADDR          @SETUP STACK POINTER
    mov fp,#0
    /*******   COPY UBOOT TO RAM   **********/
    ldr r0,=CONFIG_SYS_TEXT_BASE /* TEXT_BASE = 0x30800000 */
    mov r1,#0
    mov r2,#LENGTH_UBOOT
    bl nand_read_ll
    tst r0,#0
    beq ok_nand_read   

bad_nand_read:
    //mov r0, #0
    //bl led_ctrl_on
loop2:
    b loop2

ok_nand_read:
    mov r0,#0
    ldr r1,=CONFIG_SYS_TEXT_BASE
    mov r2,#0x400
go_next:
    ldr r3,[r0],#4
    ldr r4,[r1],#4
    teq r3,r4
    bne notmatch
    subs r2,r2,#4
    //beq stack_setup
    bne go_next
    //mov r0, #0
    //bl led_ctrl_on
    //ldr r0, =0x33FFFF00
    //ldr r1, =0x78
    //str r1, [r0]
    ldr pc, =CONFIG_SYS_TEXT_BASE
#endif
#endif

gotomain:
    //清除重定位标志，否则reset指令会无法使程序正确重启
    //ldr r0, =0x33FFFF00
    //mov r1, #0
    //str r1, [r0]
	bl	_main

notmatch:                                                                                             
    //mov r0, #1
    //bl led_ctrl_on
loop3:                                                                                                
    b loop3 

//#define STACK_BASE 0x33f00000
//#define STACK_SIZE 0x10000
//    .align 2
//    DW_STACK_START: .word STACK_BASE+STACK_SIZE-4
/*------------------------------------------------------------------------------*/

	.globl	c_runtime_cpu_setup
c_runtime_cpu_setup:

	mov	pc, lr

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */
//参数是led编号0-3
//.global led_ctrl_on
//led_ctrl_on:
//    ldr r2, =GPBDAT
//    ldr r3, [r2]
//    cmp r0, #0
//    beq led0
//    cmp r0, #1
//    beq led1
//    cmp r0, #2
//    beq led2
//    cmp r0, #3
//    beq led3
//led0:
//    bic r3, r3, #(1<<5)
//    str r3, [r2]
//    mov pc, lr
//led1:
//    bic r3, r3, #(1<<6)
//    str r3, [r2]
//    mov pc, lr
//led2:
//    bic r3, r3, #(1<<7)
//    str r3, [r2]
//    mov pc,lr
//led3:
//    bic r3, r3, #(1<<8)
//    str r3, [r2]
//    mov pc, lr
//
//led_ctrl_off:
//    ldr r2, =GPBDAT
//    mov r3, #0
//    str r3, [r2]
//    mov pc, lr
    

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (A) Align
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependend, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	mov	ip, lr

	bl	lowlevel_init

	mov	lr, ip
	mov	pc, lr
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */
