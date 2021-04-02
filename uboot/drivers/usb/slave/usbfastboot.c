/****************************************************************
 NAME: usbout.c
 DESC: USB bulk-OUT operation related functions
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 ****************************************************************/
#include <common.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>

#include <2440usb/def.h>
#include <2440usb/2440usb.h>
#include <2440usb/usbmain.h>
#include <2440usb/usb.h>
#include <2440usb/usblib.h>
#include <2440usb/usbsetup.h>
#include <2440usb/usbout.h>
#include <2440usb/usbinit.h>

#define BIT_USBD		(0x1<<25)
#define BIT_DMA2		(0x1<<19)

FASTBOOT_PACKET fb_packet;

// ===================================================================
// All following commands will operate in case 
// - out_csr3 is valid.
// ===================================================================

#define CLR_EP3_OUT_PKT_READY() writeb(((out_csr3 & (~ EPO_WR_BITS)) & (~EPO_OUT_PKT_READY)) , &usbdevregs->out_csr1_reg)
//#define SET_EP3_SEND_STALL()	usbdevregs->OUT_CSR1_REG= ((out_csr3 & (~EPO_WR_BITS)) | EPO_SEND_STALL) 
#define CLR_EP3_SENT_STALL()	writeb(((out_csr3 & (~EPO_WR_BITS)) &(~EPO_SENT_STALL)), &usbdevregs->out_csr1_reg)
//#define FLUSH_EP3_FIFO() 	usbdevregs->OUT_CSR1_REG= ((out_csr3 & (~EPO_WR_BITS)) |EPO_FIFO_FLUSH) 

// ***************************
// *** VERY IMPORTANT NOTE ***
// ***************************
// Prepare for the packit size constraint!!!

// EP3 = OUT end point. 

static U8 ep3Buf[EP3_PKT_SIZE] = {0};

void Ep3Handler(void)
{
#if 0
	struct s3c24x0_interrupt * intregs = s3c24x0_get_base_interrupt();
#endif
	struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();
    U8 out_csr3;
    int fifoCnt;
    int len;
    int i;
    unsigned char ch;

	writeb(3, &usbdevregs->index_reg);
	out_csr3 = readb(&usbdevregs->out_csr1_reg);
    
//    DbgPrintf("<3:%x]",out_csr3);

    if(out_csr3 & EPO_OUT_PKT_READY)
    {   
        //	fifoCnt=usbdevregs->OUT_FIFO_CNT1_REG; 
        fifoCnt = readb(&usbdevregs->out_fifo_cnt1_reg);
        //PrintEpoPkt(ep3Buf,fifoCnt);

        if (fb_packet.start_recv_image == true) {
            memset(ep3Buf, 0, EP3_PKT_SIZE);
            //printf("fifocnt=%d\n", fifoCnt);
            if (fifoCnt > EP3_PKT_SIZE) {
                len = EP3_PKT_SIZE;    
            } else {
                len = fifoCnt;
            }

            for (i = 0; i < len; i++) {
                ch = readb(&usbdevregs->fifo[3].ep_fifo_reg);
                (*(volatile unsigned char *)(0x32000000 + i + fb_packet.recv_image_len)) = ch;
            }

            fb_packet.recv_image_len += len;

            if (fb_packet.recv_image_len == fb_packet.image_size) {
                fb_packet.start_recv_image = false;
                fb_packet.type = FILE_RECV_OK;
            }

        } else {
            memset(ep3Buf, 0, EP3_PKT_SIZE);
            RdPktEp3(ep3Buf, fifoCnt);
            //SKIPINFO
            //printf("recv:%s\n", ep3Buf);

            if (strncmp((char *)ep3Buf, "getvar:", 7) == 0) {
                if (strncmp((char *)ep3Buf + 7, "has-slot:", 9) == 0) {
                    if (strncmp((char *)ep3Buf + 7 + 9, "uboot", 5) == 0) {
                        //printf("recv:uboot\n");
                        fb_packet.type = FB_UBOOT;
                    } else if (strncmp((char *)ep3Buf + 7 + 9, "kernel", 6) == 0) {
                        //printf("recv:kernel\n");
                        fb_packet.type = FB_KERNEL;
                    } else if (strncmp((char *)ep3Buf + 7 + 9, "rootfs", 6) == 0) {
                        //printf("recv:rootfs\n");
                        fb_packet.type = FB_ROOTFS;
                    } else {
                        //wrong
                        fb_packet.type = FB_UNKNOW;
                    }
                } else if (strncmp((char *)ep3Buf + 7, "max-download-size", strlen("max-download-size")) == 0) {
                    fb_packet.type = GET_MAX_DL_SIZE;
                } 
            } else if (strncmp((char *)ep3Buf, "download:", strlen("download:")) == 0) {
                //解析将要接收的文件大小
                int size = 0;
                int pos = 0;
                while (1) {
                    size <<= 4;
                    ch = ep3Buf[9 + pos];
                    if (ch >= 'a' && ch <= 'z') {
                        size |= (ch - 'a' + 0xa);
                    } else if (ch >= 'A' && ch <= 'Z') {
                        size |= (ch - 'A' + 0xa);
                    } else if (ch >= '0' && ch <= '9') {
                        size |= (ch - '0');
                    }
                    
                    pos++;

                    if(pos == 8)
                        break;
                }
                fb_packet.image_size = size;
                fb_packet.start_recv_image = true;
                fb_packet.type = GET_DOWNLOAD_SIZE;
            } else if (strncmp((char *)ep3Buf, "flash:", 6) == 0) {
                if (strncmp((char *)ep3Buf + 6, "uboot", 5) == 0) {
                    fb_packet.type = FB_FLASH_UBOOT;
                } else if (strncmp((char *)ep3Buf + 6, "kernel", 6) == 0) {
                    fb_packet.type = FB_FLASH_KERNEL; 
                } else if (strncmp((char *)ep3Buf + 6, "rootfs", 6) == 0) {
                    fb_packet.type = FB_FLASH_ROOTFS;
                }
            }
        }

        CLR_EP3_OUT_PKT_READY();

        return;
    }
    
        //I think that EPO_SENT_STALL will not be set to 1.
    if(out_csr3 & EPO_SENT_STALL)
    {   
        DbgPrintf("[STALL]");
        CLR_EP3_SENT_STALL();
        return;
    }	
}

#if 0
void PrintEpoPkt(U8 *pt,int cnt)
{
    int i;
    DbgPrintf("[BOUT:%d:",cnt);
    for(i=0;i<cnt;i++)
    	DbgPrintf("%x,",pt[i]);
    DbgPrintf("]");
}
#endif
