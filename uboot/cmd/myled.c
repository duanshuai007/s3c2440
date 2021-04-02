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
#include <asm/arch-s3c24x0/gpio.h>

extern int gpio_set_value(unsigned gpio, int value);

int do_led (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    if (argc < 3) {
        return CMD_RET_USAGE;
    }

    int led;

    //printf("argv[0] = %s\n", argv[0]);
    //printf("argv[1] = %s\n", argv[1]);
    //printf("argv[2] = %s\n", argv[2]);

    int ledno = simple_strtoul(argv[1], NULL, 10);
    int onoff = simple_strtoul(argv[2], NULL, 10);

    switch(ledno) {
        case 0:
            led = GPB5;
            break;
        case 1:
            led = GPB6;
            break;
        case 2:
            led = GPB7;
            break;
        case 3:
            led = GPB8;
            break;
        default:
            return CMD_RET_USAGE;
    }

    if (gpio_set_value(led, onoff)) {
        printf("led ctrl error\n");
        return 1;
    }

	return 0;
}

U_BOOT_CMD(
	led, 3, 1, do_led,
	"led [0/1/2/3] [0(on)/1(off)]",
	"led on/off"
);
