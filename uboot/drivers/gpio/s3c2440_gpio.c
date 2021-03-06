/*
 * Copyright (C) 2012
 * Gabriel Huau <contact@huau-gabriel.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/s3c2440.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <errno.h>

#define GPIO_INPUT  0x0
#define GPIO_OUTPUT 0x1

#define S3C_GPIO_CON	0x0
#define S3C_GPIO_DAT	0x4

static uint32_t s3c_gpio_get_bank_addr(unsigned gpio)
{
	/* There is up to 16 pins per bank, one bank is 0x10 big. */
	//uint32_t addr = gpio & ~0xf;
    //modify by bigfish 20190612-10
    uint32_t addr;

    if (gpio < 32) {
        addr = 0;       //GPIOA
    } else if (gpio < 64) {
        addr = 0x10;    //GPIOB
    } else if (gpio < 96) {
        addr = 0x20;    //GPIOC
    } else if (gpio < 128) {
        addr = 0x30;    //GPIOD
    } else if (gpio < 160) {
        addr = 0x40;    //GPIOE
    } else if (gpio < 192) {
        addr = 0x50;    //GPIOF
    } else if (gpio < 224) {
        addr = 0x60;    //GPIOG
    } else if (gpio < 256) {
        addr = 0x70;    //GPIOH
    } else {
        addr = 0x80;    //GPIOJ
    }

	if (addr >= 0x80 && addr != 0xd0) {	/* Wrong GPIO bank. */
		printf("Invalid GPIO bank (bank %02x)\n", addr);
		return 0xffffffff;
	}

	return addr | S3C24X0_GPIO_BASE;
}

int gpio_set_value(unsigned gpio, int value)
{
	uint32_t addr = s3c_gpio_get_bank_addr(gpio);

    //printf("addr=%08x, gpio=%d\n, value=%d\n", addr, gpio, value);

	if (addr == 0xffffffff)
		return -EINVAL;

	if (value)
		setbits_le32(addr | S3C_GPIO_DAT, 1 << (gpio & 0xf));
        //__arch_putl(1 << (gpio & 0x0f), addr | S3C_GPIO_DAT);
	else
        //__arch_putl(, addr | S3C_GPIO_DAT);
		clrbits_le32(addr | S3C_GPIO_DAT, 1 << (gpio & 0xf));

	return 0;
}

int gpio_get_value(unsigned gpio)
{
	uint32_t addr = s3c_gpio_get_bank_addr(gpio);

	if (addr == 0xffffffff)
		return -EINVAL;

	return !!(readl(addr | S3C_GPIO_DAT) & (1 << (gpio & 0xf)));
}

int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

static int s3c_gpio_direction(unsigned gpio, uint8_t dir)
{
	uint32_t addr = s3c_gpio_get_bank_addr(gpio);
	const uint32_t mask = 0x3 << ((gpio & 0xf) << 1);
	const uint32_t dirm = dir << ((gpio & 0xf) << 1);

	if (addr == 0xffffffff)
		return -EINVAL;

	clrsetbits_le32(addr | S3C_GPIO_CON, mask, dirm);
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	return s3c_gpio_direction(gpio, GPIO_INPUT);
}

int gpio_direction_output(unsigned gpio, int value)
{
	return s3c_gpio_direction(gpio, GPIO_OUTPUT);
}
