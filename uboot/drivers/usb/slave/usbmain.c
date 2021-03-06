/****************************************************************
 NAME: usbmain.c
 DESC: endpoint interrupt handler
       USB init jobs
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 ****************************************************************/
#include <common.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>

#include <2440usb/2440usb.h>
#include <2440usb/usbmain.h>
#include <2440usb/usblib.h>
#include <2440usb/usbsetup.h>
#include <2440usb/usbout.h>
#include <2440usb/usbin.h>

#define BIT_USBD		(0x1<<25)
extern void ClearPending_my(int bit); 
    
/**************************
    Some PrepareEp1Fifo() should be deleted
 **************************/   
void UsbdMain(void)
{
    //ChangeUPllValue(0x38,2,1);	// UCLK=96Mhz     
    //ChangeUPllValue(0x38,2,2);	// UCLK=48Mhz     
    InitDescriptorTable();
    //ResetUsbd();
    ConfigUsbd(); 
    //PrepareEp1Fifo(); 
#if 0    
    while(1)
    {
    	if(DbgPrintfLoop())continue;
    	
    	Delay(5000);
    	if((i++%2)==0)Led_Display(0x8);
    	else Led_Display(0x0);
    }
#endif    
}

void IsrUsbd(void)
{
    struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();
    U8 usbdIntpnd,epIntpnd;

    U8 saveIndexReg = readb(&usbdevregs->index_reg);
    
    usbdIntpnd = readb(&usbdevregs->usb_int_reg);
    epIntpnd = readb(&usbdevregs->ep_int_reg);
    //DbgPrintf( "[INT:EP_I=%x,USBI=%x]",epIntpnd,usbdIntpnd );

    if(usbdIntpnd & SUSPEND_INT)
    {
        writeb(SUSPEND_INT, &usbdevregs->usb_int_reg);
        //    	DbgPrintf( "<SUS]\n");
    }
    if(usbdIntpnd & RESUME_INT)
    {
        writeb(RESUME_INT, &usbdevregs->usb_int_reg);
        //    	DbgPrintf("<RSM]\n");
    }
    if(usbdIntpnd & RESET_INT)
    {
        //    	DbgPrintf( "<RST] ReconfigUsbd\n");  
        //ResetUsbd();
        ReconfigUsbd();
        writeb(RESET_INT, &usbdevregs->usb_int_reg); //RESET_INT should be cleared after ResetUsbd().
        //PrepareEp1Fifo(); 
    }

    if(epIntpnd & EP0_INT)
    {
        //printf("---> %s %s goto Ep0Handler\n", __FILE__, __func__);
        writeb(EP0_INT, &usbdevregs->ep_int_reg);
        Ep0Handler();
    }
    if(epIntpnd & EP1_INT)
    {
        //printf("---> %s %s goto Ep1Handler\n", __FILE__, __func__);
        writeb(EP1_INT, &usbdevregs->ep_int_reg);
        //Ep1Handler();
    }

    if(epIntpnd & EP2_INT)
    {
        writeb(EP2_INT, &usbdevregs->ep_int_reg);
        //   	DbgPrintf("<2:TBD]\n");   //not implemented yet	
        //Ep2Handler();
    }

    if(epIntpnd & EP3_INT)
    {
        writeb(EP3_INT, &usbdevregs->ep_int_reg);
        Ep3Handler();
    }

    if(epIntpnd & EP4_INT)
    {
        writeb(EP4_INT, &usbdevregs->ep_int_reg);
        //   	DbgPrintf("<4:TBD]\n");   //not implemented yet	
        //Ep4Handler();
    }

    ClearPending_my((int)BIT_USBD);	 
    writeb(saveIndexReg, &usbdevregs->index_reg);    
}

/******************* Consol printf for debug *********************/
#define DBGSTR_LENGTH (0x1000)
U8 dbgStrFifo[DBGSTR_LENGTH];
volatile U32 dbgStrRdPt=0;
volatile U32 dbgStrWrPt=0;

void _WrDbgStrFifo(U8 c)
{
    dbgStrFifo[dbgStrWrPt++]=c;
    if(dbgStrWrPt==DBGSTR_LENGTH)dbgStrWrPt=0;
}


#if 0

#if 0
void PrintEp0Pkt(U8 *pt)
{
    int i;
    DbgPrintf("[RCV:");
    for(i=0;i<EP0_PKT_SIZE;i++)
        DbgPrintf("%x,",pt[i]);
    DbgPrintf("]\n");
}
#endif

void DbgPrintf(char *fmt,...)
{
    int i,slen;
    va_list ap;
    char string[256];

    va_start(ap,fmt);
    vsprintf(string,fmt,ap);
    
//    slen=strlen(string);
    
//    for(i=0;i<slen;i++)
//    	_WrDbgStrFifo(string[i]);
    
    va_end(ap);
    puts(string);
}
#else
void DbgPrintf(char *fmt,...)
{
}
#endif

