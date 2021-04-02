#!/bin/sh
echo "---Create rootfs directions start...---"
mkdir rootfs
cd rootfs
echo "---Create root,dev...---"
mkdir bin boot dev etc home lib mnt proc root sbin sys tmp usr var
mkdir etc/init.d etc/rc.d etc/sysconfig
mkdir usr/bin usr/sbin usr/lib usr/modules
echo "---make node in dev/console dev/null---"
mknod -m 666 dev/console c 5 1
mknod -m 666 dev/null c 1 3
mkdir mnt/etc mnt/jffs2 mnt/yaffs mnt/data mnt/temp
mkdir var/lib var/lock var/run var/tmp
chmod 1777 tmp 
chmod 1777 var/tmp
echo "------make direction done------"
