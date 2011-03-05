/******************************************************************************
 ** File Name:      main.c                                                    *
 ** Author:         Richard Yang                                              *
 ** DATE:           22/08/2001                                                *
 ** Copyright:      2001 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 22/08/2001     Richard.Yang     Create.                                   *
 ** 16/09/2003     Xueliang.Wang    Modify for SM5100B module.                *
 **				    CR4013				      *
 ******************************************************************************/

#define _MAIN_ENTRY
/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

#include "os_api.h"
#include "sci_types.h"
#include "tb_hal.h"

/**---------------------------------------------------------------------------*
 **                         Debugging Flag                                    *
 **---------------------------------------------------------------------------*/
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C" 
    {
#endif


//@Daniel.Ding,CR:MS00005266,11/12/2003,begin
#ifdef USE_NUCLEUS
   
#else
    #ifndef WIN32
    int main(void)
    {
        SCI_KernelEnter();
    }
    #endif
#endif    
//Daniel.end.
/*****************************************************************************/
// Description :    This function initialize application tasks and resources.
// Global resource dependence : 
// Author :         Xueliang.Wang
// Note :
/*****************************************************************************/
//@Daniel.Ding,CR:MS00005266,11/12/2003,begin
#ifdef USE_NUCLEUS
    #ifdef HTW_INTEGRATION     //use this macro to buile this file for  .
        //NULL
    #else
        PUBLIC void SCI_InitApplication(uint32 mode)
        {
            //@Daniel.Ding,CR:MS00006039,12/4/2003,begin
            SCI_InitAtc();
            // Do nothing.
        }
    #endif
#else   //ThreadX
    PUBLIC void SCI_InitApplication(uint32 mode)
    {
        // User could add initialization of application sources 
        // and create their tasks.
        
		BOOLEAN       power_button_state = CheckPowerButtonState();
   		#ifndef _ATC_ONLY 
   		// Create Application task. Such as MMI task.
			if (!power_button_state && !REFPARAM_GetAutoPowerOnFlag())

			{

			      HAL_TurnOffPower();

			}
   			SCI_InitAppTasks();
			
 			#ifndef WIN32 
    	    // @wind.deng   12/2/2003
			// can not start GMMI and ATC task at the same time for the memory limited.
				SCI_InitAtc();  
    				
			#endif
    	#else
    		// Create AT task. 
    		;//SCI_InitAtc();   
    	#endif
    }
#endif
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    } 
#endif
