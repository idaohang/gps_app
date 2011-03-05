/******************************************************************************
 ** File Name:      mem_cfg.c                                                 *
 ** Author:         Xia.Yang                                                  *
 ** DATE:           04/22/2004                                                *
 ** Copyright:      2004 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the memory config of D-phone_5100b_b7   *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 04/22/2004     Xia.Yang         Create.                                   *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "arm_reg.h"
#include "sc6600d_reg.h"
#include "os_api.h"
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C"
    {
#endif

//-----------------------------------------------------------------------------
//
//  The initialized frequency when system start up.
//
//-----------------------------------------------------------------------------
#define EXT_MCU13M_ECS0_VALUE       		0x0802110e 	// for mirrorbit flash
#define EXT_MCU39M_ECS0_VALUE       		0x0802330e  // for mirrorbit flash
#define EXT_MCU78M_ECS0_VALUE       		0x0802770e  // for mirrorbit flash
//#define EXT_MCU39M_ECS1_VALUE       		0x08022208  // for sst flash
#define EXT_MCU39M_ECS1_VALUE       		0x08023308    // for sst flash

// PLL = 78MHz, ARM = 13MHz, DSP = 78MHz when start up.
const uint32    g_pll_mn_value = PLL_MN_CLK_P78M;
const uint32    g_clk_value = PCTL_CLK_P78M_D78M_A39M;
const uint32    g_cs0_value = EXT_MCU39M_ECS0_VALUE;
const uint32    g_cs1_value = EXT_MCU39M_ECS1_VALUE;

const uint32  FLASH_FileSystemNum   = 2;
    
// Record assert information address define.
const uint32  record_flash_base = 0x330000;
const uint32  record_flash_size = (3 * 65536);

const uint32 dump_mem_begin_addr = 0x02000000;
const uint32 dump_mem_size       = (0);  


// Table for each memory cs control register setting in different arm clock
// Should be updated according to hardware pcb layout
// All ARM clock type {13, 26, 39, 52, 71.5, 78, 104}(MHz)
const uint32 g_emcs_clk_tble[ECS_MAX_SUPPORT_NUMBER][7] =
{
	// CS0: norflash
	//{EXT_MCU13M_ECS0_VALUE, MCU26M_ECS0_VALUE, EXT_MCU39M_ECS0_VALUE, MCU52M_ECS0_VALUE, MCU71M_ECS0_VALUE, EXT_MCU78M_ECS0_VALUE, MCU104M_ECS0_VALUE},
	{DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE},
	// CS1: sram
	//{MCU13M_ECS1_VALUE, MCU26M_ECS1_VALUE, MCU39M_ECS1_VALUE, MCU52M_ECS1_VALUE, MCU71M_ECS1_VALUE, MCU78M_ECS1_VALUE, MCU104M_ECS1_VALUE},
	{DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE},
	// CS2: NF
	{DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE},
	// CS3: NF
	{DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE},	
	// CS4: NF
	{DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE},	
	// CS5: NF
	{DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE},	
	// CS6: NF
	{DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE},	
	// CS7: NF
	{DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE, DEFAULT_ECS_VALUE}	
};


// Each block is 1024byte(defined in efs_config.h), so the total file sytem space is 
//  1024 * g_file_system_data_block_num
// temporarily we use SRAM instead of flash, this two parameter is just for compiling
const uint32 g_file_system_data_block_num = 256;//20

// The max file number that support.
const uint32 g_file_system_file_num       = 16;//8

// flag indicates if we use nvitem cache
const uint32 g_nvitem_cache_enabled = 0;

const uint32 	g_fiq_stack_base = 0x4000fdf0;  

// Pool Config
// Be careful, the array below must be 4 bytes alignment.
// Byte heap size

#ifdef WIN32
	#define BYTE_HEAP_SIZE          (150 * 1024)
#else
	#define BYTE_HEAP_SIZE           (200 * 1024)//(61 * 1024)  // 61K
#endif

// Pool 52
#ifndef SCI_MEMORY_DEBUG
    #define BLOCK_52_SIZE       52
#else
    #define BLOCK_52_SIZE       (52 + sizeof(MEM_HEADER_T))
#endif

#define MAX_NUM_OF_52_POOLS     800 //200
#define POOL_52_SIZE            (MAX_NUM_OF_52_POOLS * (BLOCK_52_SIZE + sizeof(void *)))


// Pool 100
#ifndef SCI_MEMORY_DEBUG
    #define BLOCK_100_SIZE      100
#else
    #define BLOCK_100_SIZE      124
#endif

#define MAX_NUM_OF_100_POOLS    400
#define POOL_100_SIZE           (MAX_NUM_OF_100_POOLS * (BLOCK_100_SIZE + sizeof(void *)))

// Pool 300
#ifndef SCI_MEMORY_DEBUG
    #define BLOCK_300_SIZE      300
#else
    #define BLOCK_300_SIZE      324
#endif

#define MAX_NUM_OF_300_POOLS    150//25 //35
#define POOL_300_SIZE           (MAX_NUM_OF_300_POOLS * (BLOCK_300_SIZE + sizeof(void *)))

// Pool 600
#ifndef SCI_MEMORY_DEBUG
    #define BLOCK_600_SIZE      600
#else
    #define BLOCK_600_SIZE      624
#endif
#define MAX_NUM_OF_600_POOLS    50//20 //25
#define POOL_600_SIZE           (MAX_NUM_OF_600_POOLS * (BLOCK_600_SIZE + sizeof(void *)))

// Pool 1600
#ifndef SCI_MEMORY_DEBUG
    #define BLOCK_1600_SIZE     1600
#else
    #define BLOCK_1600_SIZE     1624
#endif
#define MAX_NUM_OF_1600_POOLS   20//12 //10
#define POOL_1600_SIZE          (MAX_NUM_OF_1600_POOLS * (BLOCK_1600_SIZE + sizeof(void *)))

// Memory area to create memory pool.

const uint32 block_52_size   = BLOCK_52_SIZE;
const uint32 block_100_size  = BLOCK_100_SIZE;
const uint32 block_300_size  = BLOCK_300_SIZE;
const uint32 block_600_size  = BLOCK_600_SIZE;
const uint32 block_1600_size = BLOCK_1600_SIZE;

const uint32 byte_head_size  = BYTE_HEAP_SIZE;
const uint32 pool_52_size    = POOL_52_SIZE;
const uint32 pool_100_size   = POOL_100_SIZE;
const uint32 pool_300_size   = POOL_300_SIZE;
const uint32 pool_600_size   = POOL_600_SIZE;
const uint32 pool_1600_size  = POOL_1600_SIZE;

uint32 BYTE_HEAP_ADDR[ byte_head_size >> 2 ];
uint32 POOL_52_ADDR  [ pool_52_size   >> 2 ];
uint32 POOL_100_ADDR [ pool_100_size  >> 2 ];
uint32 POOL_300_ADDR [ pool_300_size  >> 2 ];
uint32 POOL_600_ADDR [ pool_600_size  >> 2 ];
uint32 POOL_1600_ADDR[ pool_1600_size >> 2 ];

// Flash Config
const uint32  FLASH_SectorNum       = 15;       // From 0x70_0000 ==> 0x7E_FFFF     //7;        // From 0x38_0000 ==> 0x3E_FFFF
const uint32  FLASH_SectorSize      = 0x10000;  // 64K                              //0x10000;  // 64K
const uint32  FLASH_StartAddrr      = 0x0;
const uint32  FLASH_BankAddr        = 0x700000; // the bank managed by EFS          //0x380000; // the bank managed by EFS
const uint32  FLASH_Size            = 0x800000;                                     //0x400000;

const uint32  FLASH_FixedNvitemAddr = 0x6F0000;                                     //0x370000;
const uint32  FLASH_ProductInfoAddr = 0x7FE000; // the last 8K ( for some flash has the 8K sector)  //0x3FE000;

// The offset of the sram that be used to simulator
const uint32  FLASH_EmuOffsetAddr    = 0x81000;

// SRAM config
const uint32 g_fiq_stack_size        = 512;
const uint32 g_irq_stack_size        = 2048;
const uint32 g_undefine_stack_size   = 512;
const uint32 g_abort_stack_size      = 8;
const uint32 g_sys_stack_size        = 8;
const uint32 g_timer_stack_size      = 2048;

//the system memory allocation define,the allocation map defeine is different when
//chip,module and the flash chip used in the module is different for the production
const uint32 const_StackLimitOffset  =0x1FDBFC;  // = 0xFDBFC   // 6408
                                                // = 0x1FDBFC  // 6416
                                                // = 0x7DBFC   // 3204 

const uint32 const_RAM_LimitOffset   =0x1FC000;  // = 0xFC000   // 6408
                                                // = 0x1FC000  // 6416
                                                // = 0x7C000   // 3204

const uint32 g_svc_stack_limit_add = EXTM_XCSN1_START + const_StackLimitOffset;

//the ZW and ZI data should not excced the RAM LIMIT,and RAM after RAM_LIMIT
//is used as stack of exceptions as FIQ,IRQ,ABORT and so on
const uint32 g_mem_limit = EXTM_XCSN1_START + const_RAM_LimitOffset;

const uint32 g_sram_size =  1024 * 1024*2;//512 * 1024;
#define	VB_DecodeOuputBufSIZ		(160*16)		// in doubleword
uint32 VBDecodeOuputBuf[VB_DecodeOuputBufSIZ];
uint32 vb_decode_output_buffer_size = VB_DecodeOuputBufSIZ;


//
// the status be located in the space that wouldn't be changed while ARM booting )
//#define FLASH_STORE_OPTR_STATUS_ADDR   	0x7FFF0 
const uint32 flash_store_optr_address = 0x041F0000;

//uint32 flash_store_optr_address = 0x0407FFF0;

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif
