/****************************************************************
 NAME: usbin.c
 DESC: usb bulk-IN operation
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
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
#include <2440usb/usbin.h>

//static void PrintEpiPkt(U8 *pt,int cnt);

// ===================================================================
// All following commands will operate in case 
// - in_csr1 is valid.
// ===================================================================

//#define SET_EP1_IN_PKT_READY()  usbdevregs->ep0_csr_in_csr1_reg= ((in_csr1 & (~EPI_WR_BITS)) | EPI_IN_PKT_READY )
#define SET_EP1_IN_PKT_READY()  writeb(((in_csr1 & (~EPI_WR_BITS)) | EPI_IN_PKT_READY ), &usbdevregs->ep0_csr_in_csr1_reg);	 
// #define SET_EP1_SEND_STALL()	usbdevregs->ep0_csr_in_csr1_reg= ((in_csr1 & (~EPI_WR_BITS)) | EPI_SEND_STALL )
//#define CLR_EP1_SENT_STALL()	usbdevregs->ep0_csr_in_csr1_reg= ((in_csr1 & (~EPI_WR_BITS)) & (~EPI_SENT_STALL) )
#define CLR_EP1_SENT_STALL()	writeb(((in_csr1 & (~EPI_WR_BITS)) & (~EPI_SENT_STALL) ), &usbdevregs->ep0_csr_in_csr1_reg);
// #define FLUSH_EP1_FIFO() 	usbdevregs->ep0_csr_in_csr1_reg= ((in_csr1 & (~EPI_WR_BITS)) | EPI_FIFO_FLUSH )


// ***************************
// *** VERY IMPORTANT NOTE ***
// ***************************
// Prepare the code for the packit size constraint!!!

// EP1 = IN end point. 

U8 ep1Buf[EP1_PKT_SIZE];

void PrepareEp1Fifo(void) 
{
#if 0
	struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();
	//int i;
	U8 in_csr1;
	
	writeb(1, &usbdevregs->index_reg);
//	usbdevregs->index_reg=1;
	in_csr1 = readb(&usbdevregs->ep0_csr_in_csr1_reg);
//	in_csr1=usbdevregs->ep0_csr_in_csr1_reg;
    
	for(i = 0; i < EP1_PKT_SIZE; i++) {
		ep1Buf[i] = (U8)(transferIndex+i);
        ep1Buf[i] = (U8)(i);
    }


    WrPktEp1(ep1Buff, EP1_PKT_SIZE);
    SET_EP1_IN_PKT_READY(); 
#endif
}

void Ep1Handler(void)
{
	U8 in_csr1;
	struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();
	
    writeb(1, &usbdevregs->index_reg);
//	usbdevregs->index_reg=1;
	in_csr1 = readb(&usbdevregs->ep0_csr_in_csr1_reg);
//	in_csr1 = usbdevregs->ep0_csr_in_csr1_reg;
    
//	DbgPrintf("<1:%x]",in_csr1);

	//I think that EPI_SENT_STALL will not be set to 1.
	if(in_csr1 & EPI_SENT_STALL)
	{   
//		DbgPrintf("[STALL]");
		CLR_EP1_SENT_STALL();
   		return;
	}	

    //printf("--->Ep1Handler\n");
	//IN_PKT_READY is cleared
	//The data transfered was ep1Buf[] which was already configured 

	//PrintEpiPkt(ep1Buf,EP1_PKT_SIZE); 
    
	//transferIndex++;
	PrepareEp1Fifo(); 
    
    //IN_PKT_READY is set   
    //This packit will be used for next IN packit.	

    return;
}


#if 0
void PrintEpiPkt(U8 *pt,int cnt)
{
	int i;
	DbgPrintf("[B_IN:%d:",cnt);
	for(i=0;i<cnt;i++) {
		DbgPrintf("%x,",pt[i]);
	}
	DbgPrintf("]");
}
#endif
