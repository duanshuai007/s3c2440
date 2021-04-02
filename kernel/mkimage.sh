#!/bin/sh

DIR=arch/arm/boot
#mkimage -n "linux366" -A arm -O linux -T kernel -C none -a 0x31ffffc0 -e 0x32000000 -d ${DIR}/zImage ${DIR}/uImage
mkimage -n "linux366" -A arm -O linux -T kernel -C none -a 0x30007fc0 -e 0x30008000 -d ${DIR}/zImage ${DIR}/uImage
