/******************************************************************************
 ** File Name:      dummy_func.c                                              *
 ** Author:         Xia.yang                                                  *
 ** DATE:           29/04/2004                                                *
 ** Copyright:      2004 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 29/04/2004     Xia.yang         Create.                                   *
 ******************************************************************************/


/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "sci_types.h"
#include "sc6600d_reg.h"
#include "production_test.h"
#include "lcd.h"
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C"
    {
#endif

//
// Added by lin.liu for EFS test(2004-05-14).
// These function defined in efs_rpc.c, please add this file to ref_base.a
//
void EFS_RegisterDiagCmdRoutine( void );
void EFS_UnRegisterDiagCmdRoutine( void );


// This is called by diag task to register the callback routine.
void SCI_RegisterDiagCmdRoutine( void )
{
    EFS_RegisterDiagCmdRoutine();
}

// This is called by diag task to un-register the callback routine.
void SCI_UnRegisterDiagCmdRoutine( void )
{
    EFS_UnRegisterDiagCmdRoutine();
}

void PowerButton_SetMode( BOOLEAN  bGPIO_Mode )
{

}

void SCI_BeforeEnterDeepSleep( void )
{

}

void SCI_AfterExitDeepSleep( void )
{

}

void CHR_TX_Handler( void )
{

}

//@lin.liu. CR11595
void SCI_AfterChangeClk( uint32  clk )
{

}

//
// This function will be called when try close the LDO.
//
void SCI_PrePowerOff( void )
{

}

/* @jim.zhang CR:MS11459 2004-07-28 */
void LCD_PointerSaveInAssertResetMode(void)
{

}


PUBLIC void LCD_InitInAssertRestMode(void)
{

}
/* end CR:MS11459 */
PUBLIC void PROD_Init_StartUp(void)
{
	
}
void PROD_Init()
{

}

void MP3_Write_Buffer(void)
{
	return;
}

void FFS_RegisterDiagCmdRoutine( void )
{
	return;
}

//rtc bug is fixed in d5,so this interface will be set to null in 5210.
void GPIO_RTC_Reset(void)
{
	return;
}

//record function is not used in 5210
void AUD_StopRecord(void)
{
	return;
}

//it will be set to null in 5210,there is no temperature offset.
int16 ADC_GetPaTemperature(void)
{
	return 0;
}

void SCI_BeforeAssertReset( void )
{
	//need do nothing in dphone when assert
	return ;
}

void FFS_Task(uint32 argc, void * argv)
{
	return;
}

//delete lcd.c in product driver lib ,and dummy lcd in driver common lib 
LCD_OPERATIONS_T *SubLCD_GetOperations(void)
{
	return NULL;
}

LCD_OPERATIONS_T *LCD_GetOperations(void)
{
	return NULL;
}

void *LCD_GetLCDBuffer(void)
{
	return NULL;
}

void	LCD_DisplayOn(void)
{
	return;
}

BOOLEAN LCD_IsLCDPointerValid(LCD_ID_E lcd_id, LCD_OPERATIONS_T* lcdpointer_ptr)
{
	return FALSE;
}

void GPIO_SetLcdReset(void)
{
	return;
}

const uint16 lcd_width = 124;		// width of main lcd.
const uint16 lcd_height = 60;		// height of main lcd.

//dummy product test command
uint32 SCI_ProductTest_Extend( uint8 *src_ptr, uint32 src_len, uint8 *rsp_ptr, uint32 max_rsp_len )
{
    return 0;
}

void Register_MMI_Test_CmdRoutine( void )
{    
    SCI_TRACE_LOW( "Production test dummy function: Register_MMI_Test_CmdRoutine" );
    
    return;
}

// Register 
void Register_Current_Test_CmdRoutine( void )
{    
    SCI_TRACE_LOW( "Production test dummy function: Register_Current_Test_CmdRoutine" );
    
    return;
}

void ATC_ExInit(void)
{

}
//need to rewrite : the function is used to extend AT: AT## command
uint32 ATC_ExtCmd(
				  uint8 *cmd,  // Point to the command infomation buffer
				  uint32 len   // The length of command infomation buffer
				  )
{
	uint8   com_res_str[2][10]={{"OK\r\n"},{"ERROR\r\n"}};
	
	return 0;
	
}

			
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif

/* diag_routine_sp7100.c  */
