/******************************************************************************
 ** File Name:      gpio.c                                                    *
 ** Author:         Xia.Yang                                                  *
 ** DATE:           04/13/2004                                                *
 ** Copyright:      2004 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the pin usage and initiate value of     *
 **                 D-phone_5100b_b7                                          *
 **                 GPIO used information((SM5100B+SC6600B7))                 *
 **                     GPIO_18   Charge Control(Output)                       *
 **                     GPIO_26   Backlight LED(Output)                        *
 **                     GPIO_04   HandFree Control(Output)                     *
 **                     GPIO_46   Hook(Input)                                  *
 **                     GPIO_03   Handfree LED(Output)                         *
 **                     GPIO_10   Charge In(Input)                             *
 **                     GPIO_11   Status Led(Output)                             *
 **                                                                           *
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
#include "tb_comm.h"
#include "tb_dal.h"
//added by ptq
#include "sc6600d_reg.h"
#include "deep_sleep.h"
/**---------------------------------------------------------------------------*
 **                         Debugging Flag                                    *
 **---------------------------------------------------------------------------*/
#define GPIO_DRV_DEBUG

#ifdef GPIO_DRV_DEBUG
    #define GPIO_PRINT      SCI_TRACE_LOW
#else
    #define GPIO_PRINT   
#endif

#define GPIO_CHG_CTRL			18
#define GPIO_BK_LED_CTRL 		26
#define GPIO_HF_AMPLIFY_CTRL 	04
#define GPIO_HOOK			 	46
#define GPIO_HF_LED_CTRL	 	03
#define GPIO_CHARGE_IN		 	10
#define GPIO_STATUS_LED_CTRL	11

#define IO_ENABLE 1

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C" 
    {
#endif

/*****************************************************************************/
//  Description:    This function set BackLight.
//                  b_light = SCI_TRUE   turn on LED
//                  b_light = SCI_FALSE  turn off LED
//  Author:         Xueliang.Wang
//  Note:           
/*****************************************************************************/
PUBLIC void GPIO_SetBackLight(BOOLEAN b_light);

/*****************************************************************************/
//  Description:    This function set enable/disable Amplifier device.
//                  b_aux_device = SCI_TRUE   Enable Amplifier device
//                  b_aux_device = SCI_FALSE  Disable Amplifier device
//  Author:         Xueliang.Wang
//  Note:           
/*****************************************************************************/
PUBLIC void GPIO_EnableAmplifier(BOOLEAN b_aux_device);

/*****************************************************************************/
//  Description:    This function set LED of hand free.
//                  b_light = SCI_TRUE   turn on LED
//                  b_light = SCI_FALSE  turn off LED
//  Author:         Xueliang.Wang
//  Note:           
/*****************************************************************************/
PUBLIC void GPIO_SetHandFreeLed(BOOLEAN b_light);

void PbintDSlp_Handler(uint32 gpio_id, uint32 gpio_state);
void PbintSlp_Init(void);
/**---------------------------------------------------------------------------*
 **                         Function Definitions                              *
 **---------------------------------------------------------------------------*/
PUBLIC void GPIO_CustomizeInit(void)
{
#if IO_ENABLE
#ifndef _TRACK
    GPIO_AddToIntTable(
        GPIO_HOOK,                  //gpio46
        SCI_TRUE,                   //is NOT checked mode
        SCI_TRUE,                   //need deshaking
        50,                         //50ms
        TB_KPD_PRESSED,
        SCI_GPIO_SIG2,              //HOOK key
        TB_KPD_RELEASED,
        SCI_GPIO_SIG2               //HOOK key
        );
#endif
    GPIO_AddCallbackToIntTable(
        15,                  		
        SCI_TRUE,                   
        50,                         
        PbintDSlp_Handler
        );
//    GPIO_SetBackLight(SCI_FALSE);
    
//    GPIO_EnableAmplifier(SCI_FALSE);
    
//    GPIO_SetHandFreeLed(SCI_FALSE);
    //added by piaotq 20060303
    PbintSlp_Init();
    //added end
#endif

}

/*****************************************************************************/
//  Description:    This function enable/disable charge
//  Author:         lin.liu
//  Note:           
/*****************************************************************************/
PUBLIC void GPIO_EnableCharge( BOOLEAN bEnable )
{    
    //GPIO_PRINT("GPIO_EnableCharge: EnableCharge = %d \n", bEnable);
 //   GPIO_SetValue(GPIO_CHG_CTRL, bEnable);
}

/*****************************************************************************/
//  Description:    This function set BackLight.
//                  b_light = SCI_TRUE   turn on LED
//                  b_light = SCI_FALSE  turn off LED
//  Author:         Xueliang.Wang
//  Note:           
/*****************************************************************************/
PUBLIC void GPIO_SetBackLight(BOOLEAN b_light)
{
    //GPIO_PRINT("GPIO_SetBackLight: SetBackLight = %d \n", b_light);
 //   GPIO_SetValue(GPIO_BK_LED_CTRL, b_light);
}

/*****************************************************************************/
//  Description:    This function set enable/disable Amplifier device.
//                  b_aux_device = SCI_TRUE   Enable Amplifier device
//                  b_aux_device = SCI_FALSE  Disable Amplifier device
//  Author:         Xueliang.Wang
//  Note:           
/*****************************************************************************/
PUBLIC void GPIO_EnableAmplifier(BOOLEAN b_aux_device)
{
    //GPIO_PRINT("GPIO_EnableAmplifier: EnableAmplifier = %d \n", b_aux_device);
 //   GPIO_SetValue(GPIO_HF_AMPLIFY_CTRL, b_aux_device);
}

/*****************************************************************************/
//  Description:    This function return hook state(Pick up or put down).
//  Author:         Xueliang.Wang
//  Note:           SCI_TRUE :  Hook is Picked up
//                  SCI_FALSE:  Hook is Putted down
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_GetHookState(void)
{
#if 0
    if (GPIO_GetValue(GPIO_HOOK))
    {
        GPIO_PRINT("GPIO_GetHookState: HookState = 0 \n");
        return (SCI_FALSE);
    }
    else
    {
        GPIO_PRINT("GPIO_GetHookState: HookState = 1 \n");
        return (SCI_TRUE);
    }
#endif
 
}

/*****************************************************************************/
//  Description:    This function set LED of hand free.
//                  b_light = SCI_TRUE   turn on LED
//                  b_light = SCI_FALSE  turn off LED
//  Author:         Xueliang.Wang
//  Note:           
/*****************************************************************************/
PUBLIC void GPIO_SetHandFreeLed(BOOLEAN b_light)
{    
    //GPIO_PRINT("GPIO_SetHandFreeLed: HandFreeLed = %d \n", b_light);
//    GPIO_SetValue(GPIO_HF_LED_CTRL, b_light);
}

/*****************************************************************************/
//  Description:    This function set LED of power.
//                  b_light = SCI_TRUE   turn on LED
//                  b_light = SCI_FALSE  turn off LED
//  Author:         Xueliang.Wang
//  Note:           
/*****************************************************************************/
LOCAL void GPIO_SetPowerLed(BOOLEAN b_light)
{
    //GPIO_PRINT("GPIO_SetPowerLed: PowerLed = %d \n", b_light);
 //   GPIO_SetValue(GPIO_STATUS_LED_CTRL, b_light);
}

/*****************************************************************************/
//  Description:    This function check the charge has been connected?
//  Author:         lin.liu
//  Note:           
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_IsChargeConnected( void )
{
#if 0
    //GPIO_PRINT("GPIO_IsChargeConnected: IsChargeConnected = %d \n", GPIO_GetValue(GPIO_CHARGE_IN));
    if (GPIO_GetValue(GPIO_CHARGE_IN))
    {
	    GPIO_SetPowerLed(SCI_TRUE);
    }
    else
    {
	    GPIO_SetPowerLed(SCI_FALSE);
    }


    return GPIO_GetValue(GPIO_CHARGE_IN);
#endif
}

/*****************************************************************************/
//  Description:    This function set LED1.
//  Author:         Xueliang.Wang
//  Note:           
/*****************************************************************************/
PUBLIC void GPIO_SetLed1(BOOLEAN b_light)
{
}

/*****************************************************************************/
//  Description:    This function prehandle extra keypad.
//  Author:         Alex li
//  Note:           if the scan_code be not changed, then return scan_code
/*****************************************************************************/
uint16 SCI_PreHandleKeypadInterrupt( uint16  scan_code )
{
	return scan_code;
}
//added by piaotq 20060303
SCI_TIMER_PTR g_PbintDSlpTmr_ptr = NULL;

void PbintDSlpTmr_isr_handler(uint32 state)
{
#if IO_ENABLE
	if (GPIO_GetValue(15))
	{
		SCI_TRACE_LOW("PbintDSlpTmr_isr_handler disable");
		SCI_ChangeTimer(g_PbintDSlpTmr_ptr, PbintDSlpTmr_isr_handler, 1000);
	    SCI_ActiveTimer(g_PbintDSlpTmr_ptr);
	    SCI_MMI_EnableDeepSleep(DISABLE_DEEP_SLEEP);		
	}
	else
	{
		SCI_TRACE_LOW("PbintDSlpTmr_isr_handler enable");
	    SCI_DeactiveTimer(g_PbintDSlpTmr_ptr);
	    SCI_MMI_EnableDeepSleep(ENABLE_DEEP_SLEEP);    	
	}
#endif
}


PUBLIC void PbintDSlp_Handler(uint32 gpio_id, uint32 gpio_state)
{
#if IO_ENABLE
	SCI_TRACE_LOW("PbintDSlp_Handler");
    SCI_ChangeTimer(g_PbintDSlpTmr_ptr, PbintDSlpTmr_isr_handler, 1000);
    SCI_ActiveTimer(g_PbintDSlpTmr_ptr);
    SCI_MMI_EnableDeepSleep(DISABLE_DEEP_SLEEP);
#endif
}

void PbintSlp_Init(void)
{
#if IO_ENABLE
	if (NULL == g_PbintDSlpTmr_ptr)
    {
        char TmrName[15] = "PbintDSlpTmr";
    	g_PbintDSlpTmr_ptr = SCI_CreateTimer(TmrName,
    	                      	            PbintDSlpTmr_isr_handler,
    	                                    0,
    	                                    1000,
    	                                    SCI_NO_ACTIVATE);
	}	
#endif
}
//added end

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif
