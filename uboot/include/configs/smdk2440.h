/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <garyj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * Configuation settings for the SAMSUNG SMDK2410 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#undef CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_ARM920T
/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_S3C24X0		/* This is a SAMSUNG S3C24x0-type SoC */
#define CONFIG_S3C2440		/* specifically a SAMSUNG S3C2410 SoC */
#define CONFIG_SMDK2440		/* on a SAMSUNG SMDK2410 Board */

/* 该宏使uboot从通过这个配置文件的字符串中配置uboot的各项参数*/
#if 0
#define CONFIG_DELAY_ENVIRONMENT
#endif
#define CONFIG_USE_ARCH_MEMSET
#define CONFIG_USE_ARCH_MEMCPY
/*因为sdram只有64MB空间,也就是从0x3000_0000 - 0x3400_0000 */
/* sdram 单片具有256Mbits空间,通信宽度16bit,两片并连获得32bit通信宽度,存储空间为256*2=512Mbits=64MB*/
/* 在mini2440中偏选线是nGCS6所以地址空间是从0x3000_0000开始的*/
#define CONFIG_SYS_TEXT_BASE	0x33000000


/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
/* NorFlash */
#if 1
#define CONFIG_SYS_NO_FLASH
#endif
#define PHYS_FLASH_1		0x00000000 /* Flash Bank #0 */
#define CONFIG_SYS_FLASH_BASE	PHYS_FLASH_1

#define CONFIG_SYS_FLASH_CFI
#ifndef CONFIG_SYS_NO_FLASH
#if 1
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_FLASH_CFI_LEGACY
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT 1
#endif
#define CONFIG_SYS_FLASH_LEGACY_1Mx16
#define CONFIG_FLASH_SHOW_PROGRESS	45
#define CONFIG_SYS_FLASH_PROTECTION
#endif

#define CONFIG_SYS_FLASH_BANKS_LIST     { CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_SECT	(35)


#define CONFIG_SYS_ARM_CACHE_WRITETHROUGH

/* input clock of PLL (the SMDK2410 has 12MHz input clock) */
#define CONFIG_SYS_CLK_FREQ	12000000

/*
 * select serial console configuration
 */
#define CONFIG_S3C24X0_SERIAL
#define CONFIG_SERIAL1		1	/* we use SERIAL 1 on SMDK2410 */
#define CONFIG_BAUDRATE		115200

/************************************************************
 * USB support (currently only works with D-cache off)
 ************************************************************/
#if 0
#define CONFIG_USB_KEYBOARD
#define CONFIG_USB_STORAGE
#define CONFIG_DOS_PARTITION
#endif

#if 1
#define CONFIG_USB_DEVICE
#define CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ    0x400
#define CONFIG_STACKSIZE_FIQ    0x400
#endif

#ifdef CONFIG_CMD_USB
#define CONFIG_USB
#define CONFIG_USB_OHCI
#define CONFIG_USB_OHCI_S3C24XX
#endif

/************************************************************
 * RTC
 ************************************************************/
#if 0
#define CONFIG_RTC_S3C24X0
#endif

/*
 * BOOTP options
 */
#if 0
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#endif

/*
 * Command line configuration.
 */
#if 0
#define CONFIG_CMD_BSP
#define CONFIG_CMD_NAND
#define CONFIG_CMD_REGINFO
#define CONFIG_CMDLINE_EDITING
#endif

/* autoboot */
#define CONFIG_BOOTDELAY	5
#define CONFIG_BOOT_RETRY_TIME	-1
#define CONFIG_RESET_TO_RETRY
#undef  CONFIG_AUTOBOOT_KEYED
#define CONFIG_ZERO_BOOTDELAY_CHECK

#if 0
#define CONFIG_BOOTCOMMAND "nand read 0x30008000 0xa0000 0x400000;bootm 0x30008000"
#endif


/*
 *  NET 
 */
#if 0
#define CONFIG_DM9000
#endif

#ifdef  CONFIG_DM9000
#define CONFIG_DRIVER_DM9000
/*BAND4*/
#define CONFIG_DM9000_BASE  0x20000000
#define DM9000_IO           (CONFIG_DM9000_BASE)
#define DM9000_DATA         (CONFIG_DM9000_BASE + 0x4)
#define CONFIG_DM9000_NO_SROM
#define CONFIG_ETHADDR      {0x19, 0x32, 0xab, 0x9f, 0x6a, 0x44}
/* support rand_r function*/
#ifndef CONFIG_ETHADDR
/*如果使用随机的mac地址则需要是能这两项*/
#define CONFIG_LIB_RAND
#define CONFIG_NET_RANDOM_ETHADDR
#endif
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.3.100
#define CONFIG_SERVERIP		192.168.3.1
#define CONFIG_GATEWAYIP    192.168.3.1
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port */
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_CBSIZE	256
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
				sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

#if 0
#define CONFIG_DISPLAY_CPUINFO				/* Display cpu info */
#define CONFIG_SYS_MEMTEST_START	0x30000000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x33F00000	/* 63 MB in DRAM */
#endif

/* support additional compression methods */
#if 0
#define CONFIG_BZIP2
#define CONFIG_LZO
#define CONFIG_LZMA
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 *  SDRAM
 */
#define CONFIG_NR_DRAM_BANKS	1          /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x30000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */

#ifdef CONFIG_CMD_NAND
#define CONFIG_ENV_IS_IN_NAND   1
#endif

/*用来保存参数的地址空间为128k*/
#define CONFIG_ENV_SIZE         0x20000
/*因为前512k地址被划分给uboot，所以这里偏移值需要让出uboot地址空间512k*/
#define CONFIG_ENV_OFFSET       0x00080000
#define CONFIG_ENV_ADDR         CONFIG_ENV_OFFSET
/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE       /* needed for mtdparts commands */
#define MTDIDS_DEFAULT      "nand0=nand"
#define MTDPARTS_DEFAULT    "mtdparts=nand:512k(u-boot),"  \
                            "128k(params)," \
                            "4m(kernel),"  \
                            "-(rootfs)"
/* 使用tftp命令时，将image下载到这个地址*/
#if 1
#define CONFIG_SYS_LOAD_ADDR		0x30008000
#endif

/*
 * Size of malloc() pool
 * BZIP2 / LZO / LZMA need a lot of RAM
 */
#if 1
#define CONFIG_SYS_MALLOC_LEN	(4 * 1024 * 1024)
#else
#define CONFIG_SYS_MALLOC_LEN   (CONFIG_ENV_SIZE + 128*1024)
#endif

#define CONFIG_SYS_MONITOR_LEN	(448 * 1024)
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE

/*
 * NAND configuration
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_S3C2440
#if 0
#define CONFIG_S3C2440_NAND_HWECC
#endif
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x4E000000
#define CONFIG_S3C2440_NAND_SKIP_BAD
#define CONFIG_SYS_NAND_PAGE_SIZE   2048
#define CONFIG_SYS_NAND_ECCSIZE     CONFIG_SYS_NAND_PAGE_SIZE
#define CONFIG_SYS_NAND_ECCBYTES    4
#endif

#ifdef CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_S3C24X0
#define CONFIG_SYS_I2C_S3C24X0_SLAVE    0xa0
#define PINMUX_FLAG_HS_MODE             (1 << 1)
#define CONFIG_SYS_I2C_SPEED 10000
#define CONFIG_SYS_I2C_S3C24X0_SPEED    10000
#endif
/*
 * File system
 */
#if 0
#define CONFIG_YAFFS2
#define CONFIG_MTD_PARTITIONS
#endif
/* additions for new relocation code, must be added to all boards */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x1000 - \
				GENERATED_GBL_DATA_SIZE)

#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_S3C2440_GPIO

/*
   bootm 启动方式
 */
#define CONFIG_CMDLINE_TAG
#if 0
#define BOOTM_ENABLE_SERIAL_TAG
#define BOOTM_ENABLE_REVISION_TAG
#define BOOTM_ENABLE_MEMORY_TAGS
#define BOOTM_ENABLE_INITRD_TAG
#endif

#define CONFIG_CMD_NAND_YAFFS
#define CONFIG_BOOTARGS "noinitrd root=/dev/mtdblock3 rootfstype=ext4 console=ttySAC0,115200 init=/linuxrc mem=64M"

#endif /* __CONFIG_H */
