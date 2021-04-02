#uboot移植问题总结

> 交叉编译链的生成
>> 在configure中出现configure: error: could not find GNU libtool >= 1.5.26  
>> 使用sudo apt-get install libtool发现电脑上已经安装了更新的版本  
>> 使用whereis libtool 就能够发现该库所在的位置。
>> 然后在configure的时候带上--with-libtool=/usr/share/libtool就能够正确找到库的位置了   
>> ./configure --prefix=/home/duan/s3c2440/crosstool/ct-ng --with-libtool=/usr/share/libtool    
>> --prefix 指定ct-ng工具生成的目录  
>> 正确执行之后执行make, make install    

> 执行make发现出现错误：zconf.hash.c:167:1: error: conflicting types for ‘kconf_id_lookup’  
>> vi kconfig/zconf.hash.c   
>> 167 `//kconf_id_lookup (register const char *str, register size_t len)    `
>> 168 `kconf_id_lookup (register const char *str, register unsigned int len)  `

> 成功后在/home/duan/s3c2440/crosstool/ct-ng目录下会生成bin,lib,share三个文件夹，在bin文件夹内有ct-ng的工具，将该目录加入到~/.bashrc 中。  
>> 在crosstool-ng的源码目录外创建文件夹build,src,tool.  
>> build用来编译交叉编译链，src用来保存下载的源码，tool用来保存编译生成的交叉编译链。  
>> 在build目录下将crosstool-ng/samples/arm-unknown-linux-gnueabi/crosstool.config文件复制到build目录下，执行ct-ng menuconfig进行配置。
>> 
>>  Paths and misc options ─   
>>  (/home/duan/s3c2440/crosstool/src) Local tarballs directory   
>>  (/home/duan/s3c2440/crosstool/build) Working directory      
>>  (/home/duan/s3c2440/crosstool/tool) Prefix directory  
>>  Target options -  
>>  (arm9tdmi) Emit assembly for CPU   
>>  Toolchain options -  
>>  (s3c2440) Tuple's vendor string  
>>  Operating System ─    
>> (/home/duan/s3c2440/crosstool/kernel/3.6.6/linux-3.6.6) Path to custom source, tarball or direc  
>> 还有很多其他的配置根据需要进行配置。配置完成后执行ct-ng build.该过程漫长且兼具，需要网络很好，有时候还需要手动下载所需要的资源到/home/duan/s3c2440/crosstool/src目录中，注意下载的版本一定要跟配置的版本匹配。
>> 
>>   
>> 编译过程中出现错误：  
>> Installing final gcc compiler 阶段出现的错误。
>> cfns.gperf:101:1: error: 'const char* libc_name_p(const char*, unsigned int)' redeclared inline with  
>> 通过百度搜索到一篇文章https://gcc.gnu.org/git/?p=gcc.git;a=commitdiff;h=ec1cc0263f156f70693a62cf17b254a0029f4852，按照其中的修改，修改完成后能够编译通过final gcc。



### uboot支持烧写yaffs系统
> https://blog.csdn.net/sinat_24088685/article/details/52304466  
> https://www.cnblogs.com/jetli-/p/5361807.html



* 生成反汇编文件

	`arm-none-linux-gnueabi-objdump -S u-boot > filename`

> 本次移植主要参考了这个作者的经历
>>	s3c2440 uboot

>>	1)https://gaomf.cn/2016/06/19/U-Boot%202016.05%20在S3C2440上的移植（1）——基本运行/

>>	2)https://gaomf.cn/2016/06/24/U-Boot%202016.05%20在S3C2440上的移植（2）——Nor%20Flash/
	
>>	3)https://gaomf.cn/2016/06/25/U-Boot%202016.05%20在S3C2440上的移植（3）——Nand%20Flash/
	
>>	4)https://gaomf.cn/2016/06/25/U-Boot%202016.05%20在S3C2440上的移植（4）——DM9000网卡/
	
>>	5)https://gaomf.cn/2016/06/26/U-Boot%202016.05%20在S3C2440上的移植——杂项/

***


> 1.问题`ORR r0, r0, #R1_nF:OR:R1_iA`问题的解答？
>>		
	这是在start.S中
		/* MMU Set Async Bus Mode */
		mrc p15, 0, r1, c1, c0, 0
		orr r1, r1, #0xC0000000
		mcr p15, 0, r1, c1, c0, 0
	0xC0000000的值是怎么得来的?
	https://blog.csdn.net/fengsheng301/article/details/25918963


> 2.怎么让`lowlevel_init和nand_read`能运行在前4K代码中。
>>	方法：在2016-05版本中需要修改Makefile，将对应文件的obj-y:xxx.o修改为extra-y:xxx.o

>>	然后修改arch/arm/cpu/u-boot.lds，在CPUDIR/start.o (.text*)下面添加board/samsung/smdk2440/lowlevel_init.o (.text*) arch/arm/cpu/arm920t/nand_read.o (.text*) 这样再编译就不会出现重复定义的错误了。

> 3.程序卡在lowlevel_init中
>> 	1.在我调试的时候是在start.S中定义了一个led控制函数，然后在进入lowlevel_init之后调用这个函数之后就卡死了。然后我在lowlevel_init函数内直接操作寄存器控制led，程序就能正常进行，原因应该是因为调用了link。

>>	2.修改了第一个问题之后发现程序依然不能顺利进行，查看反汇编代码，发现调用memset的函数已经超出了前4K的范围。所以肯定是不回成功的。因为s3c2440的stepstoning是只能把nandflash的前4K代码复制过来。所以修改start.S内的重定位代码，在拷贝结束之后让程序直接跳到CONFIG_SYS_TEXT_BASE处执行代码，在这里又遇到一个问题，在执行adr r0, _start时出现了_start未定义的错误，重复尝试了几次没有较好的方法，所以用设置标志位的方式来。。。


> 4.网卡移植 
>>	使能DM9000的相关宏以后就不能启动了，通过打印发现，关闭cache就能够正常启动了dm9000连接在bank4上,BASEADDRESS=0x2000_0000,IO=BASEADDRESS,DATA=BASEADDRESS+4
		
> 5.代码重定位
>>	在uboot-2016.05中如果是norflash启动的话代码可以不修改直接启动(没有亲自验证).但是如果使用nandflash启动就需要我们来把代码从nandflash里读到stepstoning中了。

##NANDFLASH移植

> 芯片型号

>> K9F2G08U0C  
>>	Memory Cell Array: (256M + 8M) * 8bit  
	Data Register: (2K + 64) * 8bit  
	Page Program: (2K + 64) Byte  
	Block Erase: (128K + 4K) Byte  
	Page Size: (2K + 64) Byte  
	这几个参数前面是容量，后面的是用来保存存储块信息的空间。  
	可以看出总内存大小是256M（256M*8bit = 2Gbit）

***
> 我们需要的主要参数

>>	NFCONF主要用到了TACLS、TWRPH0、TWRPH1这三个变量用于配置nandflash的时序。  
>> 	s3c2440的数据手册没有详细说明这三个变量的具体含义，但通过nandflash芯片手册所给出的时序图和s3c2440的nandflash存储器时序图，我们可以看出，TACLS为CLE/ALE有效到nWE有效之间的持续时间，TWRPH0为nWE的有效持续时间，TWRPH1为nWE无效到CLE/ALE无效之间的持续时间，这些时间都是以HCLK为单位的（本文程序中的HCLK=100MHz）。
>>	通过查阅K9F2G08U0A的数据手册，在4.1节的Command Latch Cycle可以看到nandflash的时序图。在nandflash数据手册的2.9节能够找到芯片的各个状态所需的时间。tCLS=12ns,tWP=12ns,tCLH=5ns。

>>	我们可以找到并计算该nandflash与s3c2440相对应的时序：K9F2G08U0A中的tWP与TWRPH0相对应，tCLH与TWRPH1相对应，（tCLS－tWP）与TACLS相对应。K9F2G08U0A给出的都是最小时间，
>> 	s3c2440只要满足它的最小时间即可，因此TACLS、TWRPH0、TWRPH1。这三个变量取值大一些会更保险。在这里，这三个值分别取1，2和0。
		
>>	TACLS,TWRPH0,TWRPH1的值需要参考s3c2440手册和nandflash的时序图来进行确认。通过s3c2440的手册<图6-3> 可以得出，TACLS等于CLE/ALE开始有效到nWE开始有效到时间值，twrph0的值就是nWE的持续时间值，twrph1的值就是nWE开始无效到CLE/ALE开始无效到时间值。通过nandflash的4.1 Command Latch Cycle可以得出,TACLS=tCLS-tWP, TWRPH0=tWP, TWRPH1=tCLH.因为在nandflash手册中tcls和twp数值相等，所以TACLS值可以为0。TWRPH=12，TWRPH1=5.
		
>	clk=1/100MHz = 10ns		
>	最终得到TACLS=0,TWRPH0=1,TWRPH1=0  
>	**但是,在s3c2440_nand.c中是需要将tw的两个值减1才写入寄存器，所以在定义该值时需要加1.所以最终定义的值为**  
>	<font color=blue>
> 	**`#define CONFIG_S3C24XX_TACLS        0`**  
> 	**`#define CONFIG_S3C24XX_TWRPH0       2`**  
> 	**`#define CONFIG_S3C24XX_TWRPH1       1`**</font>

***

> s3c2440 nandflash 驱动与2410的驱动大同小异，差别主要在寄存的定义上和一些读写配置的寄存器上。

> nand在uboot下操作方法：<font color=red>在写入前必须先擦除对应的nandflash地址区域</font>
>> 
	nand erase a0000 80             
	//擦出nandflash内a0000偏移地址开始的0x80个字节
	md.l 30008000
	nand write 30008000 a0000 80    
	//从内存30008000处的数据写入到nandflash的便宜0xa0000开始的0x80个字节长度
	nand read 30000000 a0000 80     
	//将nandflash的0xa0000偏移地址开始的0x80个字节读取到内存的30000000地址处
	md.l 30000000
	cmp.l 30000000 30008000 80

> 重定位

>
>>	在代码重定位时需要从nand读取代码，这个时候nand驱动还没安装好，所以是直接操作寄存器来使用nand来读取代码。
	通过在`arch/arm/cpu/arm920t/`目录下添加文件`nand_read.c`，修改Makefile，使用`extra-y = nand_read.o`来编译，同时还需要修改`arch/arm/cpu/u-boot.lds`,在`CPUDIR/start.o (.text*)`下面添加`arch/arm/cpu/arm920t/nand_read.o (.text*)`

> **<font color=red>重要</font>**
>>	在`s3c2440_nand.c`中，驱动代码有问题。需要修改代码。除了其他几个函数需要修改对应的寄存器以外，在`s3c24x0_hwcontrol`函数中需要进行改正，改正后的代码如下
>>

```
static void s3c24x0_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
    struct nand_chip *chip = mtd->priv;
    struct s3c24x0_nand *nand = s3c24x0_get_base_nand();

    debug("hwcontrol(): 0x%02x 0x%02x\n", cmd, ctrl);

    if (ctrl & NAND_CTRL_CHANGE) {
        ulong IO_ADDR_W = (ulong)nand;

        if (!(ctrl & NAND_CLE)) 
            IO_ADDR_W |= S3C2440_ADDR_NCLE;
        if (!(ctrl & NAND_ALE))
            IO_ADDR_W |= S3C2440_ADDR_NALE;
		//新加的代码，如果不加这一判断就会无法写入
        if (cmd == NAND_CMD_NONE)
            IO_ADDR_W = (ulong)&nand->nfdata;

        chip->IO_ADDR_W = (void *)IO_ADDR_W;

        if (ctrl & NAND_NCE)
            writel(readl(&nand->nfcont) & ~S3C2440_NFCONT_nFCE,
                   &nand->nfcont);
        else
            writel(readl(&nand->nfcont) | S3C2440_NFCONT_nFCE,
                   &nand->nfcont);
    }   

    if (cmd != NAND_CMD_NONE)
        writeb(cmd, chip->IO_ADDR_W);
}
```

***
>  `/*用来保存参数的地址空间为128k*/`

>>
```
#define CONFIG_ENV_SIZE         0x20000
/*因为前512k地址被划分给uboot，所以这里偏移值需要让出uboot地址空间512k*/
#define CONFIG_ENV_OFFSET       0x00080000
#define CONFIG_ENV_ADDR         CONFIG_ENV_OFFSET
```
***
>>
	nandflash内的空间分配
	device nand0 <nand>, # parts = 4
	#: name                size            offset          mask_flags
	0: u-boot              0x00080000      0x00000000      0
	1: params              0x00020000      0x00080000      0
	2: kernel              0x00200000      0x000a0000      0
	3: rootfs              0x0fd60000      0x002a0000      0
>>	
	通过bdinfo可以看到以下信息
	SMDK2440 # bdinfo
	arch_number = 0x000007CF
	boot_params = 0x30001000
	DRAM bank   = 0x00000000
	-> start    = 0x30000000
	-> size     = 0x04000000
	eth0name    = dm9000ls
>>
	ethaddr     = (not set)
	current eth = dm9000
	ip_addr     = 192.168.1.100
	baudrate    = 115200 bps
	TLB addr    = 0x33FF0000
	relocaddr   = 0x33FC0000
	reloc off   = 0x03FB8000
	irq_sp      = 0x33F5FEF0
	sp start    = 0x33F5FEE0
	可以看到relocaddr地址为33fc0000该地址是uboot自身重定向之后的地址，
	所以实际的运行地址是这里。预留给uboot的空间为256K。
	根据reloc off也能看出，我们设置的CONFIG_SYS_TEXT_BASE地址
	为30008000，他俩相加都等于relocaddr
***


##NorFlash

> Uboot 下操作norflash的命令是flinfo,md,cp等
>>	使能 menuconfig cmdline flinfo

> 芯片型号
>>	通过查看电路板上的芯片可以看到芯片型号是:
>>	s29al016j70tfi02（该芯片兼容AM29LV160DB/SST39VF1601）最后的02的意思是bottom boot sector device (CFI Support)
	
**<font color=red>根据数据手册第三章的3.1图，可以看到BYTE#引脚是47引脚，再看mini2440原理图，看到47引脚被连接到VDD可以知道是16位的word接法。</font>**

* 根据数据手册中的参数`2M*8bit/1M*16bit`，在查看原理图，原理图中数据线是16位，所以nandflash是2Mbit
* 拥有1个16Kbyte，2个8Kbyte，1个32KByte和31个64Kbyte
* 在数据手册6.1中可以通过芯片型号得知01代表的是bottom boot sector device(CFI Support),然后在10.10节Command Definitions Table中的Device ID寻找bottom boot block对应word的是0x2249，Byte的是0x49
* 这里建议使用word类型的值进行填写，因为word类型的也能够兼容byte类型，但是byte却不能兼容word类型。
* 在drivers/mtd/jedec_flash.c中添加芯片ID的定义#define S29AL016J   0x2249

> ### <font color=red>重要 </font>
>>	在调试的过程中发现在cfi_flash.c中定义了DEBUG宏之后发现<br>
>>	fwc addr 00000000 cmd f0 00f0 16bit x 16 bit<br>
>>	fwc addr 0000aaaa cmd aa 00aa 16bit x 16 bit<br>
>> 	fwc addr 00005554 cmd 55 0055 16bit x 16 bit<br>
>>	fwc addr 0000aaaa cmd 90 0090 16bit x 16 bit<br>
>>	fwc addr 00000000 cmd f0 00f0 16bit x 16 bit<br>
>>	JEDEC PROBE: ID f0 ea00 0<br>
>>	manuid=f0, devid=ea00，这与手册所写的不相符。但是如果按照这个值填写下方的结构体后，就能够正常的找到设备。

<font color=red>在正常测试时因为我使通过mini2440提供的superboot来烧uboot的，所以都是通过nand方式启动,如果想要测试norflash功能，就得启动后把开关扳到norflash启动到一边，注意这里不要重新启动，然后就能测试norflash驱动功能了。</font>

```
在static const struct amd_flash_info jedec_table[] = {}
的CONFIG_SYS_FLASH_LEGACY_512Kx16的分支中
中添加
    {
        #.mfr_id		= (u16)AMD_MANUFACT,
        #.dev_id		= S29AL016J,
        .mfr_id     = 0xf0,
        .dev_id     = 0xea00,
        .name		= "S29AL016J",
        .uaddr		= {
            [1] = MTD_UADDR_0x0555_0x02AA /* x16 */
        },
        .DevSize		= SIZE_2MiB,
        .CmdSet			= CFI_CMDSET_AMD_LEGACY,
        .NumEraseRegions	= 4,
        .regions		= {
            ERASEINFO(0x04000, 1),
            ERASEINFO(0x02000, 2),
            ERASEINFO(0x08000, 1),
            ERASEINFO(0x10000, 31),
        }
    },
    
```

>>	其中mfr_id是根据数据手册中的Manufacturer ID的Fourth也就是Device ID等于0x2249的那一列的值，是0x01，AMD_MANUFACT的值就是0x01。<br>
>>	dev_id是上面定义的宏S29AL016J。name随便写。uaddr的参数是固定可选的，根据10.10的表能看出来Device ID的First是555，Second是2AA，所以需要选择MTD_UADDR_0x0555_0x02AA。<br>
>>	DevSize就是芯片容量，8位宽对应2MiB。<br>
>>	CmdSet对应CFI_CMDSET_AMD_LEGACY,因为该芯片跟AMD的芯片是兼容的，所以选择这个。<br>
>>	NumEraseRegions代表一共有几种类型的内存块。<br>
>>	regions对应的每种类型的内存块的大小和数量，分别是16K-1个，8K-2个，32K-1个，64K-31个。


> 测试方法
		
>>		flinfo
		
>>		Bank # 1: S29AL016J flash (16 x 16)  Size: 2 MB in 35 Sectors
>>	  	AMD Legacy command set, Manufacturer ID: 0xF0, Device ID: 0xEA00
>>	  	Erase timeout: 30000 ms, write timeout: 100 ms
	
>>	  	Sector Start Addresses:
>>	  	00000000   RO   00004000   RO   00006000   RO   00008000   RO   00010000   RO 
>>	  	00020000   RO   00030000        00040000        00050000        00060000      
>>	  	00070000        00080000        00090000        000A0000        000B0000      
>>	  	000C0000        000D0000        000E0000        000F0000        00100000      
>>	  	00110000        00120000        00130000        00140000        00150000      
>>	  	00160000        00170000        00180000        00190000        001A0000      
>>	  	001B0000        001C0000        001D0000        001E0000        001F0000 
>>	  
	erase 1:34  		//擦除最后一块内存,地址对应001F0000
	md.l 001F0000 20	//查看内存001F0000开始的0x20个字符
	md.l 30000000 20	//同上
	cp.l 30000000 001F0000 20	//把30000000开始的0x20个字符复制到001F0000地址处
	md.l 001F0000 20 	//查看内存
	cmp.l 30000000 001F0000 20	//比较内存

##SDRAM:

> 连接方式
> > mini2440上sdram连接到nGS6上，通过查询s3c2440的手册可以看到对应的内存地址是0x3000_0000。
> > 
> > 该板上的SDRAM芯片em63a165ts-6g * 2片，单片的容量256Mbits，16位宽，两片容量就是512Mbits，32位宽。
> > 
> > 所以对应的sdram地址空间是0x3000_0000 -> 0x3400_0000

> 移植需要修改的地方
> >
> > board/samsung/smdk2440/lowlevel_init.S 中的SDRAM参数需要参考sdram芯片和s3c2440芯片的参数和寄存器
> > 
> > 在em63a165ts的第19页可以看到trp的取值范围是15/20 ns,trc取值范围60/63 ns,tref的时间是7.8us。
> > 
* 一个clk时间是多少？hclk=100mhz,1clk=10ns
* trp，trc的取值需要参考s3c2440的REFRESH的具体设置，可以得到trp=0x2,对应3个始终,trc=0x3,对应7个时钟.
* REFCNT计算公式：2048 + 1 - HCLK * tREF = 1268

***

##其他

```
在arch/arm/include/asm/mach-types.h中添加版号的定义
#define MACH_TYPE_SMDK2440             1999
在文件下方的SMDK2410下方添加定义
#ifdef CONFIG_ARCH_SMDK2440
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_SMDK2440
# endif
# define machine_is_smdk2440()	(machine_arch_type == MACH_TYPE_SMDK2440)
#else
# define machine_is_smdk2440()	(0)
#endif
该宏在board/samsung/smdk2440/smdk2440.c 中被使用，在board_init中gd->bd->bi_arch_number = MACH_TYPE_SMDK2440;

```

##I2C

板子上的芯片是atmlh151,没有找到有关的资料信息。

* 在`inclde/configs/smdk2440.h`中添加以下宏定义。

		#ifdef CONFIG_CMD_I2C
		#define CONFIG_SYS_I2C
		#define CONFIG_SYS_I2C_S3C24X0
		#define CONFIG_SYS_I2C_S3C24X0_SLAVE    0xa0
		#define PINMUX_FLAG_HS_MODE             (1 << 1)
		#define CONFIG_SYS_I2C_SPEED 10000
		#define CONFIG_SYS_I2C_S3C24X0_SPEED    10000
		#endif

在menuconfig->command line interface->Devices access commands->i2c使能。

* 测试i2c功能
	*	probe
	
		输入i2c probe 会自动扫描可用的i2c设备。在我这个板子上扫描结果是
		
		```
		BIGF # i2c probe
		
		Valid chip addresses: 50 51 52 53
		```
		
	* 	i2c mw [dev] [addr] [value]

		i2c mw 50 00 ab 该命令将在50设备的00地址处写入0xab值
		
	* 	i2c md [dev] [addr]
	
		i2c md 50 01	该命令将50设备的01地址处的值读出来
		
##Uboot saveenv 导致系统复位

```
	通过追踪代码发现代码在执行到lib/hashtable.c 文件中的hexport_r函数内的
	672     if (*resp) {
	673         /* yes; clear it */
	674         //printf("test: 2 up\n");
	675         res = *resp;
	676         memset(res, '\0', size);
	677     } else {
	第676行处memset时产生了复位，通过工程内搜索memset的定义发现没有定义CONFIG_USE_ARCH_MEMSET宏，在include/configs/smdk2440.h中添加了该宏后重新编译后，功能正常了。
```


##在bootm指令加载内核的过程中不能memmove内核
	
* 在执行memmove时发现产生了系统复位，寻找原因，最后发现是cache使能导致指令没能及时的执行。
* 因为我在调试的时候在memmove里面的while循环内加入打印信息后，发现结果就正常了。所以猜想可能会是cache使能所导致的。

