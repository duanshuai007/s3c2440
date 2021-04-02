#include <common.h>
#include <command.h>
#include <asm/byteorder.h>

#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>

#include <2440usb/usb.h>
#include <2440usb/2440usb.h>
#include <2440usb/usblib.h>

extern int usb_init_slave(void);

#ifdef CONFIG_USE_IRQ
    #define    FREE_RAM_SIZE    0x02000000
#else
    #define    FREE_RAM_SIZE    0x02000000
#endif

#define CLR_EP3_OUT_PKT_READY() writeb(((out_csr3 & (~ EPO_WR_BITS)) & (~EPO_OUT_PKT_READY)) , &usbdevregs->out_csr1_reg)
#define SET_EP1_IN_PKT_READY()  writeb(((in_csr1 & (~EPI_WR_BITS)) | EPI_IN_PKT_READY ), &usbdevregs->ep0_csr_in_csr1_reg)

extern FASTBOOT_PACKET fb_packet;

void easy_send(char *str) 
{
    struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();
    U8 in_csr1;

    //printf("--->easy_send:%s\n", str);
    writeb(1, &usbdevregs->index_reg);
    in_csr1 = readb(&usbdevregs->ep0_csr_in_csr1_reg);

    WrPktEp1(str, strlen(str));
    SET_EP1_IN_PKT_READY(); 
}

int do_fastboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    char buff[64];

    memset(&fb_packet, 0, sizeof(fb_packet));
    usb_init_slave();

    while(1) {
        switch(fb_packet.type) {
            case FB_UBOOT:
                fb_packet.which = FLASH_UBOOT;
                fb_packet.type = FB_UNKNOW;
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "%s", "OKAYuboot");
                easy_send(buff);
                break;
            case FB_KERNEL:
                fb_packet.which = FLASH_KERNEL;
                fb_packet.type = FB_UNKNOW;
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "%s", "OKAYkernel");
                easy_send(buff);
                break;
            case FB_ROOTFS:
                fb_packet.which = FLASH_ROOTFS;
                fb_packet.type = FB_UNKNOW;
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "%s", "OKAYrootfs");
                easy_send(buff);       
                break;
            case GET_MAX_DL_SIZE:
                fb_packet.type = FB_UNKNOW;
                memset(buff, 0, sizeof(buff));
                if ( fb_packet.which == FLASH_UBOOT ) {
                    sprintf(buff, "%s%08d", "DATA", 0x80000);
                } else if (fb_packet.which == FLASH_KERNEL ) {
                    sprintf(buff, "%s%08d", "DATA", 0x200000);
                } else if (fb_packet.which == FLASH_ROOTFS ) {
                    sprintf(buff, "%s%08d", "DATA", 0xfd60000);
                } else {
                    sprintf(buff, "%s", "FAIL");
                }
                easy_send(buff);
                break;
            case GET_DOWNLOAD_SIZE:
                fb_packet.type = FB_UNKNOW;
                memset(buff, 0, sizeof(buff));
                switch(fb_packet.which) {
                    case FLASH_UBOOT:
                        if (fb_packet.image_size > 0x80000) {
                            sprintf(buff, "%s", "FAIL");
                        } else {
                            sprintf(buff, "%s%08x", "DATA", fb_packet.image_size);
                        }
                        break;
                    case FLASH_KERNEL:
                        if (fb_packet.image_size > 0x400000) {
                            sprintf(buff, "%s", "FAIL");
                        } else {
                            sprintf(buff, "%s%08x", "DATA", fb_packet.image_size);
                        }
                        break;
                    case FLASH_ROOTFS:
                        if (fb_packet.image_size > 0xfd60000) {
                            sprintf(buff, "%s", "FAIL");
                        } else {
                            sprintf(buff, "%s%08x", "DATA", fb_packet.image_size);
                        }
                        break;
                    default:
                        sprintf(buff, "%s", "FAIL");
                        break;
                } 
                easy_send(buff);
                break;
            case FILE_RECV_OK:
                fb_packet.type = FB_UNKNOW;
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "%s", "OKAY");
                easy_send(buff);
                break;
            case FB_FLASH_UBOOT:
                fb_packet.type = FB_UNKNOW;
                memset(buff, 0, sizeof(buff));
                run_command("nand erase.part u-boot", 0);
                sprintf(buff, "%s %x", "nand write 32000000 0", fb_packet.image_size);
                run_command(buff, 0);
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "%s", "OKAY");
                easy_send(buff);
                //break;
                return 0;

            case FB_FLASH_KERNEL:
                fb_packet.type = FB_UNKNOW;
                memset(buff, 0, sizeof(buff));
                run_command("nand erase.part kernel", 0);
                int kernel_nandaddr_offser = 0xa0000;
                sprintf(buff, "%s %x %x", "nand write 32000000", kernel_nandaddr_offser, fb_packet.image_size);
                run_command(buff, 0);
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "%s", "OKAY");
                easy_send(buff);

                return 0;

            case FB_FLASH_ROOTFS:
                fb_packet.type = FB_UNKNOW;
                memset(buff, 0, sizeof(buff));
                run_command("nand erase.part rootfs", 0);
                int rootfs_nandaddr_offset = 0x4a0000;
                sprintf(buff, "%s %x %x", "nand write.yaffs2 32000000", rootfs_nandaddr_offset, fb_packet.image_size);
                run_command(buff, 0);
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "%s", "OKAY");
                easy_send(buff);

                return 0;

            default:
                break;
        }
        udelay(1000);   //必要的延时，可以根据需要加长或减少
    }

    return 0;
}

U_BOOT_CMD(
        fastboot, 3, 0, do_fastboot,
        "fastboot - flash file by usb fastboot tool",
        "   fastboot\n"
        "   fastboot hahah"
        );
