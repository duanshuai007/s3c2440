/*
 * (C) Copyright 2010
 * Jason Kridner <jkridner@beagleboard.org>
 *
 * Based on cmd_led.c patch from:
 * http://www.mail-archive.com/u-boot@lists.denx.de/msg06873.html
 * (C) Copyright 2008
 * Ulf Samuelsson <ulf.samuelsson@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <command.h>

int do_mybootm (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    //if (argc < 3) {
    //    return CMD_RET_USAGE;
    //}

    //int addr = simple_strtoul(argv[1], NULL, 16);
    //int size = simple_strtoul(argv[2], NULL, 16);
    char buff[64] = {0};
    
    sprintf(buff, "nand read 30008000 a0000 400000");
    run_command(buff, 0);

    memset(buff, 0, sizeof(buff));
    sprintf(buff, "bootm 30008000");
    run_command(buff, 0);

	return 0;
}

U_BOOT_CMD(
	mybootm, 3, 1, do_mybootm,
	"mybootm",
	"boot kernel"
);
