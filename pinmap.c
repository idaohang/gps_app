/******************************************************************************
 ** File Name:      pinmap.c                                                  *
 ** Author:         Xia.Yang                                                  *
 ** DATE:           04/13/2004                                                *
 ** Copyright:      2004 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the pin usage and initiate value of     *
 **                 D-phone_5100b_b7                                          *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 04/13/2004     Xia.Yang         Create.                                   *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "arm_reg.h"
#include "pinmap.h"

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C"
    {
#endif

/**---------------------------------------------------------------------------*
 **                         Data                                              *
 **---------------------------------------------------------------------------*/
const PM_PINFUNC_T pm_func[]=
{
	//  ***************************************************************************
	//  CONST settings
	//  ***************************************************************************
	//  Note: The settings below should not be modified
	{0x8c0000d4, BIT_2		},	// add keyin4 WPU
	{0x8c0000ec, BIT_2   	},	// add gpio46 WPU
	//  ***************************************************************************
	//  CONST settings end
	//  ***************************************************************************
	    
	//  ***************************************************************************
	// Customize setting
	//  ***************************************************************************
	// Note: Please add your configuration below
	// choose U2TXD as lcd spi tx signal
	
#ifdef _TRACK
	{0x8c000000, BIT_8 | BIT_9						}, //配置为GPIO42
#else
    {0x8c000000, BIT_9									}, 
#endif
    // enable rfctl13 bit1
    {0x8c000008, BIT_1  								}, 
    // enable GPIO6 bit12|bit13
    {0x8c00000c, BIT_12	| BIT_13						}, 
	// enable GPIO8 bit2|bit3,GPIO9 bit4|bit5,gpio10 bit8|bit9 ,gpio11 bit10|bit11
    {0x8c000010, BIT_2	| BIT_3 | BIT_4  | BIT_5  |  BIT_8 | BIT_9 | BIT_10 | BIT_11 },	
    //piaotq added for pbint
    {0x8c000014, BIT_6  | BIT_7	},	
    //gpio26 is set enable in 8c000014 ~bit8 ~bit9 at default
    // enable rfctl14 bit5, rfctrl15 bit7
    {0x8c000018, BIT_5  | BIT_7	},
    #ifdef _DUART
    // enable GPIO3 bit13
    {0x8c000038,  BIT_13	},
    #else
	// enable GPIO3 bit12|bit13	
	{0x8c000038, BIT_12  | BIT_13	},    
    #endif

    // enable GPIO1 bit8|bit9 GPIO3 bit12|bit13, GPIO4 bit14|bit15
    {0x8c00003C, BIT_8  | BIT_9 | BIT_10 | BIT_11 | BIT_14 | BIT_15 },	
    // enable gpio18(jtag tck) bit0|bit1 , and will set to 0 if use JTAG gpio17 bit2|bit3 gpio16 bit4|bit5 gpio19 bit6|bit7 gpio20 bit8|bit9
    {0x8c000044, BIT_0  | BIT_1	| BIT_2  | BIT_3  | BIT_4 | BIT_5 | BIT_6 | BIT_7 | BIT_8 | BIT_9 },	
	// enable gpio46 bit4|bit5
    {0x8c000048, BIT_4  | BIT_5 },	 			
	//  ***************************************************************************
	//  Customize setting end   
	//  ***************************************************************************
	{0x8C00009C, (BIT_5 | BIT_4 | BIT_1 | BIT_0)},   // 设置串口驱动能力

	{0x8c0000e4, (BIT_0	| BIT_2 | BIT_6) },          // Add the UART0 RXD and UART1 RXD wpu  
                                                     // 设置串口上拉电阻WPU
	{0x8C0000C8, (BIT_4)},                           // pwma wpu for SPI_DO									
	{0x8C0000AC, (BIT_2 | BIT_3)},                   // 加大SIMDA的驱动能力									
	
	{0x8c00006C, 0x3        }, //  driven strength
    
    {0xffffffff, 0xffffffff }
};

const PM_PINFUNC_T pm_default_global_map[]=
{
    {0x80000028, 0x2000     }, // Enable FIQ interrupt
    {0x8b00002c, 0x7401e	}, // Enable Gpio 0~15, and 16~31
    {0x20000008, 0x0803eeef			},	// NF
    {0x2000000c, 0x0803eeef			},	// NF
    {0x20000010, 0x0803eeef			},	// NF    
    {0x20000014, 0x0803eeef			},	// NF        
    {0x20000018, 0x0803eeef			},	// NF
    {0x2000001c, 0x0803eeef			},	// NF   
    {0xffffffff, 0xffffffff}
};

const PM_GPIO_CTL_T  pm_gpio_default_map[]=
{
#ifdef _DUART
    {19,      0, PM_INPUT,       PM_INVALID_INT  }, // CPU_ACC_IN
	{17,      0, PM_INPUT,       PM_INVALID_INT  }, // CPU_JJQ_IN
	{4,	  	  0, PM_OUTPUT,		 PM_INVALID_INT  }, // WDT
	{10,      0, PM_INPUT,       PM_INVALID_INT  }, // VCAR_PFO 欠压检测输入
    {6,	      0, PM_INPUT,		 PM_INVALID_INT  }, // CPU_SDBJ_IN
    {1,	      0, PM_OUTPUT,		 PM_INVALID_INT  }, // PWREN_GPS
    {9,	      0, PM_OUTPUT,		 PM_INVALID_INT  }, // SUART :NULL DUART:PWR_SB
    {18,      0, PM_OUTPUT,      PM_INVALID_INT  }, // CPU_LED
    {26,      1, PM_OUTPUT,      PM_INVALID_INT  }, // SUART:NULL DUART: PWR_HH
    {16,      1, PM_OUTPUT,      PM_INVALID_INT  }, // CPU_XIHUO_OUT
    {11,      0, PM_INPUT,       PM_INVALID_INT  }, // CPU_CMKG_IN
    {8,       0, PM_INPUT,       PM_INVALID_INT  },  // PIC_ACK I/O
    {20,      0, PM_OUTPUT,      PM_INVALID_INT  }, // PIC_TXD
    {46,      0, PM_OUTPUT,      PM_INVALID_INT  }, // HF_MUTE
    //added by piaotq pbint
    {15,      0, PM_INPUT,       PM_BOTH_EDGE	 }, 
    //added end
    {0xffff,  0, PM_INVALID_DIR, PM_INVALID_INT  }
#endif

#ifdef _SUART
#ifdef _TRACK
	{19,      0, PM_INPUT,       PM_INVALID_INT  }, // CPU_ACC_IN
	{17,      0, PM_INPUT,       PM_INVALID_INT  }, // CPU_JJQ_IN
	{4,	  	  0, PM_OUTPUT,		 PM_INVALID_INT  }, // WDT
	{10,      0, PM_INPUT,       PM_INVALID_INT  }, // VCAR_PFO 欠压检测输入
    {1,	      1, PM_OUTPUT,		 PM_INVALID_INT  }, // PWREN_GPS
    {9,	      0, PM_OUTPUT,		 PM_INVALID_INT  }, // NULL
    {18,      0, PM_OUTPUT,      PM_INVALID_INT  }, // CPU_LED
    {16,      1, PM_OUTPUT,      PM_INVALID_INT  }, // CPU_XIHUO_OUT
    {11,      0, PM_INPUT,       PM_INVALID_INT  }, // CPU_CMKG_IN
    {8,       0, PM_INPUT,       PM_INVALID_INT  },  // PIC_ACK I/O
    {32,       0, PM_OUTPUT,      PM_INVALID_INT  }, // PWR_SB
    {20,      0, PM_OUTPUT,      PM_INVALID_INT  }, // PIC_TXD
	{26,      0, PM_INPUT,      PM_INVALID_INT  }, // CPU_DD_IN
	{42,      0, PM_INPUT,      PM_INVALID_INT  }, // CPU_SDBJ_IN
	{46,      0, PM_INPUT,      PM_INVALID_INT  }, // CPU_LINE_IN
    //added by piaotq pbint
    {15,      0, PM_INPUT,       PM_BOTH_EDGE	 }, 
    //added end
    {0xffff,  0, PM_INVALID_DIR, PM_INVALID_INT  }
#else
    {19,      0, PM_INPUT,       PM_INVALID_INT  }, // CPU_ACC_IN
	{17,      0, PM_INPUT,       PM_INVALID_INT  }, // CPU_JJQ_IN
	{4,	  	  0, PM_OUTPUT,		 PM_INVALID_INT  }, // WDT
	{10,      0, PM_INPUT,       PM_INVALID_INT  }, // VCAR_PFO 欠压检测输入
    {6,	      0, PM_INPUT,		 PM_INVALID_INT  }, // CPU_SDBJ_IN
    {1,	      0, PM_OUTPUT,		 PM_INVALID_INT  }, // PWREN_GPS
    {9,	      0, PM_OUTPUT,		 PM_INVALID_INT  }, // SUART :NULL DUART:PWR_SB
    {18,      0, PM_OUTPUT,      PM_INVALID_INT  }, // CPU_LED
    {26,      0, PM_OUTPUT,      PM_INVALID_INT  }, // SUART:NULL DUART: PWR_HH
    {16,      1, PM_OUTPUT,      PM_INVALID_INT  }, // CPU_XIHUO_OUT
    {11,      0, PM_INPUT,       PM_INVALID_INT  }, // CPU_CMKG_IN
    {8,       0, PM_INPUT,       PM_INVALID_INT  },  // PIC_ACK I/O
    {32,       0, PM_OUTPUT,      PM_INVALID_INT  }, // PWR_SB
    {20,      0, PM_OUTPUT,      PM_INVALID_INT  }, // PIC_TXD
    {46,      0, PM_OUTPUT,      PM_INVALID_INT  }, // HF_MUTE
    //added by piaotq pbint
    {15,      0, PM_INPUT,       PM_BOTH_EDGE	 }, 
    //added end
    {0xffff,  0, PM_INVALID_DIR, PM_INVALID_INT  }
#endif
#endif

};

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif
