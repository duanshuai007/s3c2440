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

extern volatile U32 dwUSBBufReadPtr;
extern volatile U32 dwUSBBufWritePtr;
extern volatile U32 dwWillDMACnt;
extern volatile U32 bDMAPending;
extern volatile U32 dwUSBBufBase;
extern volatile U32 dwUSBBufSize;
extern void ClearPending_my(int bit); 
//static void PrintEpoPkt(U8 *pt,int cnt);
//static void RdPktEp3_CheckSum(U8 *buf,int num);


extern USB_PACKET usbPacket;

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
static int total = 0;

void Ep3Handler(void)
{
#if 0
	struct s3c24x0_interrupt * intregs = s3c24x0_get_base_interrupt();
#endif
	struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();
    U8 out_csr3;
    int fifoCnt;
    int len, read_num;
    int i;
    unsigned char ch;

	writeb(3, &usbdevregs->index_reg);
	out_csr3 = readb(&usbdevregs->out_csr1_reg);
    
//    DbgPrintf("<3:%x]",out_csr3);

    if(out_csr3 & EPO_OUT_PKT_READY)
    {   
        //	fifoCnt=usbdevregs->OUT_FIFO_CNT1_REG; 
        fifoCnt = readb(&usbdevregs->out_fifo_cnt1_reg);
#if 1
        //PrintEpoPkt(ep3Buf,fifoCnt);

        if (usbPacket.start_recv_image == true) {
            memset(ep3Buf, 0, EP3_PKT_SIZE);
            if (fifoCnt > EP3_PKT_SIZE) {
                len = EP3_PKT_SIZE;    
            } else {
                len = fifoCnt;
            }

            //for (i = 0; i < len; i++) {
            read_num = 0;
            i = 0;

            while(1) {
                ch = readb(&usbdevregs->fifo[3].ep_fifo_reg);
                (*(volatile unsigned char *)(0x32000000 + i + usbPacket.recv_image_len)) = ch;

                if (total % 0xffff != 0) {
                    i++;
                }
                
                read_num++; //当前循环内读取到的有效字节数
                
                total++;
                
                if (read_num == len)
                    break;
            }

            usbPacket.recv_image_len += i;
            
            if ((total % 0xffff == 0) && (total > 0)) {
                usbPacket.send_step = 10;
            }

            if (usbPacket.recv_image_len == (usbPacket.image_len)) {
                usbPacket.start_recv_image = false;
                usbPacket.send_step = 11; 
                usbPacket.startflash = true;
                total = 0;
            }

        } else {
            memset(ep3Buf, 0, EP3_PKT_SIZE);
            RdPktEp3(ep3Buf, fifoCnt);
            //SKIPINFO
            if (strncmp((char *)ep3Buf, "SKIPINFO", 8) == 0) {
                usbPacket.send_step = 1;
            } 
            else if (strncmp((char *)ep3Buf, "GETHW 0", 7) == 0) {
                usbPacket.send_step = 2;
            }
            else if (strncmp((char *)ep3Buf, "GETHW 1", 7) == 0) {
                usbPacket.send_step = 3;
            }
            else if (strncmp((char *)ep3Buf, "GETHW 2", 7) == 0) {
                usbPacket.send_step = 4;
            }
            else if (strncmp((char *)ep3Buf, "GETHW 3", 7) == 0) {
                usbPacket.send_step = 5;
            }
            else if (strncmp((char *)ep3Buf, "GETHW 4", 7) == 0) {
                usbPacket.send_step = 6;
            }
            else if (strncmp((char *)ep3Buf, "GETINFO", 7) == 0) {
                if (usbPacket.last_step  == 6) {
                    usbPacket.send_step = 7;
                    //bfirsttime = false;
                } else if (usbPacket.last_step == 7) {
                    usbPacket.send_step = 8;
                } else if (usbPacket.last_step == 8) {
                    usbPacket.send_step = 8;
                }
                else if (usbPacket.last_step == 11) {
                    usbPacket.send_step = 12;   //跳转到开始安装uboot
                } else if (usbPacket.last_step == 12) {
                    if (usbPacket.startflash == true) {
                        usbPacket.send_step = 13;  //正在安装uboot
                    }
                } else if (usbPacket.last_step == 13) {
                    //正在安装uboot
                    if (usbPacket.startflash == true)
                        usbPacket.send_step = 13;
                    else
                        usbPacket.send_step = 14;
                } else if (usbPacket.last_step == 14) {
                    usbPacket.send_step = 15;
                } else if (usbPacket.last_step == 15) {
                    usbPacket.send_step = 15;
                }
                //if (usbPacket.startflash == true) {
                //    usbPacket.send_step = 12;
                //}
            }
            else if (strncmp((char *)ep3Buf, "FARM spbt", 9) == 0) {
                usbPacket.send_step = 9;
                //char size[6] = {0};
                int image_len = 0;
                char ch; 
                int i = 0;
                while(1) {
                    //size[0 + i] = ep3Buf[10 + i];
                    image_len <<= 4;
                    ch = ep3Buf[10 + i];
                    if ((ch >= '0') && (ch <= '9')) {
                        image_len |= (ch - '0');
                    } else if ((ch >= 'a') && (ch <= 'z')) {
                        image_len |= (ch - 'a' + 0xa);
                    }

                    i++;

                    if (i == 6)
                        break;
                    if (ep3Buf[10 + i] == 0)
                        break;
                }
                usbPacket.image_len = image_len;
                //printf("len = %d\n", usbPacket.image_len);
            } 
        }
#else

	if( downloadFileSize == 0)
	{
   	    RdPktEp3((U8 *)downPt,8); 	
   	    
   	    if(download_run==0)
   	    {
		    downloadAddress = tempDownloadAddress;
	    }
	    else
	    {
	    	downloadAddress=
	    		*((U8 *)(downPt + 0))+
			(*((U8 *)(downPt + 1)) << 8)+
			(*((U8 *)(downPt + 2)) << 16)+
			(*((U8 *)(downPt + 3)) << 24);
            
            dwUSBBufReadPtr = downloadAddress;
            dwUSBBufWritePtr = downloadAddress;
	    }
	    downloadFileSize=
	    	*((U8 *)(downPt + 4))+
		(*((U8 *)(downPt + 5)) << 8)+
		(*((U8 *)(downPt + 6)) << 16)+
		(*((U8 *)(downPt + 7)) << 24);
	    checkSum = 0;
	    downPt = (U8 *)downloadAddress;

  	    RdPktEp3_CheckSum((U8 *)downPt, fifoCnt-8); //The first 8-bytes are deleted.	    
  	    downPt += fifoCnt-8;  
  	    
  	#if USBDMA
     	    //CLR_EP3_OUT_PKT_READY() is not executed. 
     	    //So, USBD may generate NAK until DMA2 is configured for USB_EP3;
		writel((readl(&intregs->intmsk) | BIT_USBD), &intregs->intmsk);
      	    return;	
  	#endif	
	}
	else
	{
	#if USBDMA    	
	    printf("<ERROR>");
	#endif    
	    RdPktEp3_CheckSum((U8 *)downPt,fifoCnt); 	    
	    downPt+=fifoCnt;  //fifoCnt=64
	}
#endif

   	CLR_EP3_OUT_PKT_READY();

#if 0
       if(((rOUT_CSR1_REG&0x1)==1) && ((rEP_INT_REG & 0x8)==0))
  		{
  		fifoCnt=rOUT_FIFO_CNT1_REG; 
		RdPktEp3_CheckSum((U8 *)downPt,fifoCnt); 	    
	       downPt+=fifoCnt;  //fifoCnt=64
	       CLR_EP3_OUT_PKT_READY();
		}
#endif
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

#if 0
void RdPktEp3_CheckSum(U8 *buf,int num)
{
    int i;
	struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();    
	
    for(i=0;i<num;i++)
    {
        buf[i] = readb(&usbdevregs->fifo[3].ep_fifo_reg);
        checkSum += buf[i];
    }
}
#endif
void IsrDma2(void)
{
	struct s3c24x0_interrupt * intregs = s3c24x0_get_base_interrupt();
	struct s3c24x0_usb_device * const usbdevregs	= s3c24x0_get_base_usb_device();
    U8 out_csr3;
    U32 dwEmptyCnt;
	U8 saveIndexReg = readb(&usbdevregs->index_reg);
	writeb(3, &usbdevregs->index_reg);
	out_csr3 = readb(&usbdevregs->out_csr1_reg);

    ClearPending_my((int)BIT_DMA2);	    

    printf("--->IsrDma2\n");
    /* thisway.diy, 2006.06.22 
     * When the first DMA interrupt happened, it has received max (0x80000 + EP3_PKT_SIZE) bytes data from PC
     */
    if (!totalDmaCount) 
        totalDmaCount = dwWillDMACnt + EP3_PKT_SIZE;
    else
        totalDmaCount+=dwWillDMACnt;

//    dwUSBBufWritePtr = ((dwUSBBufWritePtr + dwWillDMACnt - USB_BUF_BASE) % USB_BUF_SIZE) + USB_BUF_BASE; /* thisway.diy, 2006.06.21 */
    dwUSBBufWritePtr = ((dwUSBBufWritePtr + dwWillDMACnt - dwUSBBufBase) % dwUSBBufSize) + dwUSBBufBase;

    if(totalDmaCount>=downloadFileSize)// is last?
    {
    	totalDmaCount=downloadFileSize;
	
    	ConfigEp3IntMode();	

    	if(out_csr3& EPO_OUT_PKT_READY)
    	{
       	    CLR_EP3_OUT_PKT_READY();
	    }
		writel(((readl(&intregs->intmsk) | BIT_DMA2) & ~(BIT_USBD)), &intregs->intmsk);
    }
    else
    {
    	if((totalDmaCount+0x80000)<downloadFileSize)	
    	{
    	    dwWillDMACnt = 0x80000;
	    }
    	else
    	{
    	    dwWillDMACnt = downloadFileSize - totalDmaCount;
    	}

        // dwEmptyCnt = (dwUSBBufReadPtr - dwUSBBufWritePtr - 1 + USB_BUF_SIZE) % USB_BUF_SIZE; /* thisway.diy, 2006.06.21 */
        dwEmptyCnt = (dwUSBBufReadPtr - dwUSBBufWritePtr - 1 + dwUSBBufSize) % dwUSBBufSize;
        if (dwEmptyCnt >= dwWillDMACnt)
        {
    	    ConfigEp3DmaMode(dwUSBBufWritePtr, dwWillDMACnt);
        }
        else
        {
            bDMAPending = 1;
        }
    }
	writeb(saveIndexReg, &usbdevregs->index_reg);
}


void ClearEp3OutPktReady(void)
{
	struct s3c24x0_usb_device * const usbdevregs	= s3c24x0_get_base_usb_device();
    U8 out_csr3;
	writeb(3, &usbdevregs->index_reg);
	out_csr3 = readb(&usbdevregs->out_csr1_reg);
    CLR_EP3_OUT_PKT_READY();
}
