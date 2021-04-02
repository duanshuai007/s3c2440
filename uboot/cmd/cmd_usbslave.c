#include <common.h>
#include <command.h>
#include <asm/byteorder.h>

#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>

#include <2440usb/usb.h>
#include <2440usb/2440usb.h>
#include <2440usb/usblib.h>

extern int usb_init_slave(void);

#ifdef CONFIG_USB_DEVICE

#ifdef CONFIG_USE_IRQ
    //#define IRQ_STACK_START    (_start - CONFIG_SYS_MALLOC_LEN - CONFIG_SYS_GBL_DATA_SIZE - 4)
    //#define FIQ_STACK_START    (IRQ_STACK_START - CONFIG_STACKSIZE_IRQ)
    //#define FREE_RAM_END        (FIQ_STACK_START - CONFIG_STACKSIZE_FIQ - CONFIG_STACKSIZE)
    //#define FREE_RAM_SIZE        (FREE_RAM_END - PHYS_SDRAM_1)
    #define    FREE_RAM_SIZE    0x02000000
#else
    //#define    FREE_RAM_END    (_start - CONFIG_SYS_MALLOC_LEN - CONFIG_SYS_GBL_DATA_SIZE - 4 - CONFIG_STACKSIZE)
    //#define    FREE_RAM_SIZE    (FREE_RAM_END - PHYS_SDRAM_1)
    #define    FREE_RAM_SIZE    0x02000000
#endif

//int g_bUSBWait = 1;
//u32 g_dwDownloadLen = 0;

//extern int download_run;
//extern volatile unsigned int dwUSBBufBase;
//extern volatile unsigned int dwUSBBufSize;

//extern __u32 usb_receive(char *buf, size_t len, unsigned int wait);

#if 0
int do_usbslave (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int i;
    size_t len = ~0UL;
    char buf[32];

    if (argc < 3) {
        printf("paramter too small\n");
        return CMD_RET_USAGE;
    }

    /* download_run为1时表示将文件保存在USB Host发送工具dnw指定的位置
     * download_run为0时表示将文件保存在参数argv[2]指定的位置
     * 要下载程序到内存，然后直接运行时，要设置download_run=1，这也是这个参数名字的来由
     */
    download_run = 1;
    switch (argc) {
        case 1:
        {
            break;
        }
        case 2:
        {
            g_bUSBWait = (int)simple_strtoul(argv[1], NULL, 16);
            break;
        }

        case 3:
        {
            g_bUSBWait = (int)simple_strtoul(argv[1], NULL, 16);
            load_addr = simple_strtoul(argv[2], NULL, 16);
            download_run = 0;
            break;
        }
        
        default:
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
            return 1;
        }
    }

    dwUSBBufBase = load_addr;
    dwUSBBufSize = (FREE_RAM_SIZE&(~(0x80000-1)));
    if (g_bUSBWait)
        len = FREE_RAM_SIZE;

    g_dwDownloadLen = usb_receive(dwUSBBufBase, len, g_bUSBWait);
    sprintf(buf, "%X", g_dwDownloadLen);
    setenv("filesize", buf);
    
    return 0;
}
#else

#define CLR_EP3_OUT_PKT_READY() writeb(((out_csr3 & (~ EPO_WR_BITS)) & (~EPO_OUT_PKT_READY)) , &usbdevregs->out_csr1_reg)
#define SET_EP1_IN_PKT_READY()  writeb(((in_csr1 & (~EPI_WR_BITS)) | EPI_IN_PKT_READY ), &usbdevregs->ep0_csr_in_csr1_reg)

USB_PACKET usbPacket;

int do_usbslave (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();
    U8 in_csr1;
    u8 send_buff[64] = {0};

    if (argc == 2) {

        if (strncmp(argv[1], "get", 3) == 0) {
            printf("*********************\n");
            printf("image len = %d\n", usbPacket.image_len);
            printf("recv image len = %d\n", usbPacket.recv_image_len);
            printf("*********************\n");
        } else {
            printf("/tparamter error\n");
        }
    } else {

        usb_init_slave();


        while(1) {

            writeb(1, &usbdevregs->index_reg);
            in_csr1 = readb(&usbdevregs->ep0_csr_in_csr1_reg);

            switch(usbPacket.send_step) {
                case 1:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    send_buff[0] = 0x65;
                    WrPktEp1(send_buff, 1);
                    SET_EP1_IN_PKT_READY();        
                    break;
                case 2:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    memcpy(send_buff, " Superboot-2440 Version: 1.5(20150414)", 38);
                    WrPktEp1(send_buff, 38);
                    SET_EP1_IN_PKT_READY();        
                    break;
                case 3:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    memcpy(send_buff, " CPU: S3C2440 400MHz", 20);
                    WrPktEp1(send_buff, 20);
                    SET_EP1_IN_PKT_READY();        
                    break;
                case 4:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    memcpy(send_buff, " RAM: 64MB", 10);
                    WrPktEp1(send_buff, 10);
                    SET_EP1_IN_PKT_READY();        
                    break;
                case 5:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    memcpy(send_buff, " NAND: 256MB(SIC) ID:ECDA1095", 29);
                    WrPktEp1(send_buff, 29);
                    SET_EP1_IN_PKT_READY();        
                    break;
                case 6:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    send_buff[0] = 0x20;
                    WrPktEp1(send_buff, 1);
                    SET_EP1_IN_PKT_READY();        
                    break;
                case 7:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    memcpy(send_buff, "1 Unknown command", 17);
                    WrPktEp1(send_buff, 17);
                    SET_EP1_IN_PKT_READY();        
                    break;
                case 8:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    send_buff[0] = 0x31;
                    send_buff[1] = 0x20;
                    WrPktEp1(send_buff, 2);
                    SET_EP1_IN_PKT_READY();        
                    break;
                case 9:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    usbPacket.start_recv_image = true;
                    usbPacket.recv_image_len = 0;
                    send_buff[0] = 0x4f;
                    WrPktEp1(send_buff, 1);
                    SET_EP1_IN_PKT_READY();        
                    break;
                case 10:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    send_buff[0] = 0x4f;
                    WrPktEp1(send_buff, 1);
                    SET_EP1_IN_PKT_READY();        
                    break;
                case 11:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    //usbPacket.recv_image_len = 0;
                    send_buff[0] = 0x6f;
                    WrPktEp1(send_buff, 1);
                    SET_EP1_IN_PKT_READY();  
                    break;
                case 12:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    memcpy(send_buff, "0 Installing bootloader...", 26);
                    WrPktEp1(send_buff, 26);
                    SET_EP1_IN_PKT_READY();

                    char cmd_buff[64] = {0};
                    run_command("nand erase.part u-boot", 0);
                    sprintf(cmd_buff, "%s %x", "nand write 32000000 0", usbPacket.image_len);
                    printf("%s", cmd_buff);
                    run_command(cmd_buff, 0);
                    usbPacket.startflash = false;
                    break;
                case 13:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    send_buff[0] = 0x30;
                    send_buff[1] = 0x20;
                    WrPktEp1(send_buff, 2);
                    SET_EP1_IN_PKT_READY();
                    break;
                case 14:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    memcpy(send_buff, "1 Installing bootloader succeed", 31);
                    WrPktEp1(send_buff, 31);
                    SET_EP1_IN_PKT_READY();
                    break;
                case 15:
                    usbPacket.last_step = usbPacket.send_step;
                    usbPacket.send_step = 0;
                    send_buff[0] = 0x31;
                    send_buff[1] = 0x20;
                    WrPktEp1(send_buff, 2);
                    SET_EP1_IN_PKT_READY();
                    break;
                    //return 0;
                default:
                    break;
            }
        }

    }

    return 0;
}
#endif

U_BOOT_CMD(
    usbslave, 3, 0, do_usbslave,
    "usbslave - recvive uboot file from PC minitools",
    "usbslave get\n"
    "   get usb recvive uboot size\n"
    "usbslave\n"
    "   start recvive uboot"
);

#endif
