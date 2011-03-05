/********************************************************************************
 ** File Name:      Charge.c                                                    *
 ** Author:         Xia.yang                                                    *
 ** DATE:           04/28/2004                                                  *
 ** Copyright:      2004 Spreatrum, Incoporated. All Rights Reserved.           *
 ** Description:    Manager charge (Li-Battery) and Send event to client        *
 **                                                                             *
 ** Physical Model:                                                             *
 **                 SM5100B use two ADC.                                        *
 **                 one for voltage of battery(V_ext).                          *
 **                 one for voltage of module(V_battery).                       *
 **                                                                             *
 **                 If the charge is not connect, only report the capacity to   *
 **                    client( capacity, worning, shutdow )                     *
 **                 If the charge is connect, start charge. and don't           *
 **                    report worning/shutdown message to client.               *
 ** Note :                                                                      *
 **                 The battery couldn't be removed while phone is working      *
 ********************************************************************************

 ********************************************************************************
 **                        Edit History                                         *
 ** -------------------------------------------------------------------------   *
 ** DATE           NAME             DESCRIPTION                                 *
 ** 04/28/2004     Xia.yang         Create.                                     *
 ********************************************************************************/

 /**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "tb_dal.h"
#include "tb_hal.h"
#include "sc6600d_reg.h"
#include "ref_import.h"
#include "tb_comm.h"
#include "dal_chr.h"
//#include "gpio_drv.h"
/**---------------------------------------------------------------------------*
 **                         Debug Flag                                        *
 **---------------------------------------------------------------------------*/
//#define DEBUG_CHARGE

#ifdef  DEBUG_CHARGE
#define DEBUG_PRINT    SCI_TRACE_LOW
#else
#define DEBUG_PRINT
#endif

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C"
    {
#endif

uint32  GPIO_IsBattery(void)
{
	return 1;
}
/**---------------------------------------------------------------------------*
 **             The customization data define                                 *
 **---------------------------------------------------------------------------*/
#define LI_BATTERY

#ifdef  LI_BATTERY
    #define CHR_VOLTAGE_100             4200
    #define CHR_VOLTAGE_80              3800
    #define CHR_VOLTAGE_60              3700
    #define CHR_VOLTAGE_40              3600
    #define CHR_VOLTAGE_20              3500
    #define CHR_VOLTAGE_0               3300
	#define CHR_VOLTAGE_POWERON			3350//used to check the MS can power on ,is more than CHR_VOLTAGE_0 to avoid unexpected power on!
    // for AT command
    // the max level range and default value
    #define CHR_MAX_LEVEL_HIGH          4200
    #define CHR_MAX_LEVEL_LOW           4100
    #define CHR_MAX_LEVEL_DEFAULT       4200
    // the min level range and default value
    #define CHR_MIN_LEVEL_HIGH          3300
    #define CHR_MIN_LEVEL_LOW           3200
    #define CHR_MIN_LEVEL_DEFAULT       3300

    // When the module voltage don't falling in the given time slot, should stop charge.
    #define CHR_MAX_VP_TIME_SLOT        (8 * 3600 * 1000)   // 8 hours

    #define ADC_SOURCE_BATTERY          6 //ADIN_0
    #define VOLATGE_OFFSET				0 					//LI BATTERY  need not the offset 
#else
    #define CHR_VOLTAGE_100             4200
    #define CHR_VOLTAGE_80              3800
    #define CHR_VOLTAGE_60              3700
    #define CHR_VOLTAGE_40              3600
    #define CHR_VOLTAGE_20              3500
    #define CHR_VOLTAGE_0               3300
    #define CHR_VOLTAGE_POWERON			3350//used to check the MS can power on ,is more than CHR_VOLTAGE_0 to avoid unexpected power on!
    // for AT command
    // the max level range and default value
    #define CHR_MAX_LEVEL_HIGH          4200
    #define CHR_MAX_LEVEL_LOW           4100
    #define CHR_MAX_LEVEL_DEFAULT       4200
    // the min level range and default value
    #define CHR_MIN_LEVEL_HIGH          3300
    #define CHR_MIN_LEVEL_LOW           3200
    #define CHR_MIN_LEVEL_DEFAULT       3300

    // When the module voltage don't falling in the given time slot, should stop charge.
    #define CHR_MAX_VP_TIME_SLOT        (4 * 3600 * 1000) // 4 Hour
	// the source of charge.
	// 0x1, 0x2, 0x4, 0x8, 0x10
	// 5-channel analog inputs are connected to a 10-bit ADC
	// through an analog multiplexor.
    #define ADC_SOURCE_BATTERY          ADIN_1
    #define VOLATGE_OFFSET				200 
    
	//check if there is a call currently, and we will reduce shutdown vol threshold in call 
	//this func is used in Ni-Hi battery ,and there is no need to check call vol in li-battery
	BOOLEAN CC_IsInCall(void)
	{
		CALLS_STATE_T  calls_context;
		
		MNCALL_GetCallsState(&calls_context);
		if (calls_context.call_entity_amount == 0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;	
		}
	}    
    
#endif

/**---------------------------------------------------------------------------*
 **             The const data define                                         *
 **---------------------------------------------------------------------------*/
//for adc calibration
#define VOLTAGE_1000MV        1000
#define ADIN1_VOL_BASE        3610
#define ADIN6_VOL_BASE        3610
#define ADIN1_ADC_BASE        394   
#define ADIN6_ADC_BASE        783
#define ADIN1_ADC_OF_1V       97
#define ADIN6_ADC_OF_1V       203
	
static  uint32  adin1_base_vol = ADIN1_VOL_BASE;
static  uint32  adin6_base_vol = ADIN6_VOL_BASE;
static  uint32  adin1_base_adc = ADIN1_ADC_BASE;
static  uint32  adin6_base_adc = ADIN6_ADC_BASE;


#define CHR_INIT_MAGIC              0x43485249

#define SHUTDOWN_BATTERY_VOLTAGE    CHR_VOLTAGE_0
#define WARNING_BATTERY_VOLTAGE     CHR_VOLTAGE_20
#define SHUTDOWN_MODULE_ADC         700
#define SHUTDOWN_MODULE_VOLTAGE     3200

#define CHARGE_ENABLE_BATTERY_OUT	500//define the battery out limit

#define DISPLAY_NO_BATTERY			0

#define CHR_MODULE_ADC_SHIFT_NUM    3
#define CHR_BATT_ADC_SHIFT_NUM      3

#define ADC_BATTERY          0
#define ADC_MODULE           1


//Support  new VBATADC calibration after this calibrtion version
#define NEW_VBATADC_CAL_VER 0xFF05   

//Default VBATADC slope
#define DEFAULT_VBATADC_SLOPE 60

// Support the ADC calibration after this phone module version.
#define CHR_PM_VER_SUPPORT                        0x0201

// The invalid calibration value
#define CHR_INVALID_CALIBRATION                   0xFFFFFFFF


#define enable_charge( _flag )      GPIO_EnableCharge( _flag )

// Calculate the time slot between the two given time
#define get_time_slot( _start, _end ) \
    ( (_end) >= (_start) ) ? ( (_end) - (_start) ) : (0xFFFFFFFF - (_start) + (_end) )

//  Charge State & Event define
typedef enum
{
    CHR_IDLE,                       // Not in charge
    CHR_CHARGING                    // being charging.
}CHR_STATE;

typedef enum
{
    CHR_UPDATE_BATTERY,             // update the battery voltage
    CHR_UPDATE_MODULE,              // update the module voltage
    CHR_DISCONNECT,                 // the charge being disconnect
    CHR_CONNECT                     // the charge being connect.
}CHR_EVENT;


// the current voltage(m-voltage) of battery.
static uint32  current_battery_voltage = CHR_VOLTAGE_100;

// The current adc of module
static uint32  current_module_adc      = 0x3FF;

 uint32 g_module_adc;
 uint32 g_module_voltage;
static CHR_STATE chr_state       = CHR_IDLE;
static uint32  chr_init_flag     = CHR_INIT_MAGIC;
static uint32  bCapacityFull     = FALSE;  //it is not used to indicate MMI but only to be used in the charging plan e.g. need charge judgement 
static uint32  bFirstTime        = TRUE;


static uint32  battery_adc_count;
static uint32  battery_adc_total;
static uint32  battery_adc_max;
static uint32  battery_adc_min;

static uint32  battery_voltage_saved;
static uint32  battery_time_saved;

static BOOLEAN  is_battery_in = TRUE;//the battery is set to hand-in default
/**---------------------------------------------------------------------------*
 **                         Function Definitions                              *
 **---------------------------------------------------------------------------*/
//for read adc calibration data
static void read_adc_calibration(void);

//map the adc to the correspond voltage
static uint32 adc_to_voltage(
							 uint8 adcSource,
							 uint32 adc
							 );

extern BOOLEAN CC_IsInCall(void);

//for read adc calibration data
static void read_adc_calibration(void)
{
	uint32  temp_adin1_base_vol;
	uint32  temp_adin4_base_vol;
	uint32  temp_adin1_base_adc;
	uint32  temp_adin4_base_adc;
	
	//check if the calibration is 0xff05, if true then will do adc calibration else do not support adc calibration
    if	(REF_GetCalibrationParamVersion() >= NEW_VBATADC_CAL_VER)
    {
    	uint32 battery[2];
    	uint32 general_adc[2];
        //Get battery cal infor from calibaration parameters. general_adc is no use.
        REF_GetAdcCalibrationPara(battery, general_adc, CHR_INVALID_CALIBRATION, CHR_PM_VER_SUPPORT);
    	
    	/*
	    if((0 != battery[0]) && (0 != battery[1]))
	    {
			//channel ADIN1
			temp_adin1_base_vol = battery[1] & 0x0000FFFF;
			temp_adin1_base_adc = battery[1] >> 16;
			
			
			adin1_base_vol = temp_adin1_base_vol;
			adin1_base_adc = temp_adin1_base_adc;
			SCI_TRACE_LOW("CHR:ADC1 Calibration: battery[1] = 0x%x",battery[1]);
			SCI_TRACE_LOW("CHR:ADIN1 base adc = %d, base vol = %d", adin1_base_adc, adin1_base_vol );
		
		
			//channel ADIN6
			temp_adin4_base_vol = battery[0] & 0x0000FFFF;
			temp_adin4_base_adc = battery[0] >> 16;
			
			
			adin6_base_vol = temp_adin4_base_vol;
			adin6_base_adc = temp_adin4_base_adc;
			SCI_TRACE_LOW("CHR:ADC4 Calibration: battery[0] = 0x%x",battery[0]);
			SCI_TRACE_LOW("CHR:ADIN4 base adc = %d, base vol = %d", adin6_base_adc, adin6_base_vol );
		
		
			return;
	    }    
	    */
	    
	    if (0 != battery[0])
	    {
			//channel ADIN6
			temp_adin4_base_vol = battery[0] & 0x0000FFFF;
			temp_adin4_base_adc = battery[0] >> 16;
			
			adin6_base_vol = temp_adin4_base_vol;
			adin6_base_adc = temp_adin4_base_adc;
			SCI_TRACE_LOW("CHR:ADC4 Calibration: battery[0] = 0x%x",battery[0]);
			SCI_TRACE_LOW("CHR:ADIN4 base adc = %d, base vol = %d", adin6_base_adc, adin6_base_vol );
	    }
	    else
	    {
    		SCI_TRACE_LOW("CHR:no adc calibration data battery[0], use default data :");
	    }
	    
	    if (0 != battery[1])
	    {
			//channel ADIN1
			temp_adin1_base_vol = battery[1] & 0x0000FFFF;
			temp_adin1_base_adc = battery[1] >> 16;
			
			adin1_base_vol = temp_adin1_base_vol;
			adin1_base_adc = temp_adin1_base_adc;
			SCI_TRACE_LOW("CHR:ADC1 Calibration: battery[1] = 0x%x",battery[1]);
			SCI_TRACE_LOW("CHR:ADIN1 base adc = %d, base vol = %d", adin1_base_adc, adin1_base_vol );
	    }
	    else
	    {
    		SCI_TRACE_LOW("CHR:no adc calibration data battery[1], use default data :");
	    }
    }
    
    //SCI_TRACE_LOW("CHR:no adc calibration data, use default data :");
}


static void reset_battery_adc_var(
            void
            )
{
    battery_adc_count  = 0;
    battery_adc_total  = 0;
    battery_adc_max    = 0x0;
    battery_adc_min    = 0xFFFF;
}

//  check the charge status.
static int is_charge_connect(
            void
            )
{
    return (int)GPIO_IsChargeConnected();
}

// start the charge process and send message to client.
static void start_charge(
            void
            )
{
    // When start charge, need reset the vars, so when charge again, the value is correct.
    enable_charge( 1 );
    SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_CHARGE_START_IND, (void *)NULL );
}

// stop the charge and send message to MMI
static void stop_charge(
            void
            )
{
    battery_time_saved = 0;
    battery_voltage_saved = 0;
    enable_charge( 0 );
    SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_CHARGE_END_IND, (void *)NULL );
}

//   see phycical model in this file header to get more information.
//   if battery voltage is not enough high, or Vprog is not enough low,
//   need charge and return non-zero;
//   otherwise return zero;
static uint32 need_charge(
              void
              )
{
	DEBUG_PRINT("CHR : bCapacityFull = %d; is_battery_in = %d", bCapacityFull, is_battery_in);
    if( is_charge_connect() && !bCapacityFull && is_battery_in)//if battery is hand-in and battery is not full and charge connect ,then need charge
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//map the adc to the correspond voltage
static uint32 adc_to_voltage(
							 uint8 adcSource,
							 uint32 adc
							 )
{
    uint32 voltage_result;
	
	
    if( adc >= 0x3FF )
    {
		//DEBUG_PRINT("CHR:charge.c adc_to_voltage(): invalid adc = %d > 0x3ff", adc);
        return CHR_VOLTAGE_100;
    }
	
    if( adc <= 100 )
    {
       	//DEBUG_PRINT("CHR:charge.c adc_to_voltage(): invalid adc = %d < 100", adc);
        return 0;
    }
	
	switch (adcSource)
	{
	case ADC_BATTERY:
		if(adc >= adin1_base_adc)
		{
			voltage_result = ((adc-adin1_base_adc)*VOLTAGE_1000MV/ADIN1_ADC_OF_1V) + adin1_base_vol;
		}
		else
		{
			voltage_result = adin1_base_vol - ((adin1_base_adc-adc)*VOLTAGE_1000MV/ADIN1_ADC_OF_1V);
		}
		break;
		
	case ADC_MODULE:
		if(adc >= adin6_base_adc)
		{
			voltage_result = ((adc-adin6_base_adc)*VOLTAGE_1000MV/ADIN6_ADC_OF_1V) + adin6_base_vol;
		}
		else
		{
			voltage_result = adin6_base_vol - ((adin6_base_adc-adc)*VOLTAGE_1000MV/ADIN6_ADC_OF_1V);
		}
		break;
		
	default:
		//DEBUG_PRINT("CHR:charge.c adc_to_voltage(): invalid adc channel");
		return 0;
		break;
	}
	
	return voltage_result;
	
}


//map the voltage to the correspond capacity display in MMI
static uint32 voltage_to_percentum(
              uint32 voltage
              )
{
    if( voltage >= CHR_VOLTAGE_80 )
    {
        return 4;
    }
    else if( voltage >= CHR_VOLTAGE_60 )
    {
        return 3;
    }
    else if( voltage >= CHR_VOLTAGE_40 )
    {
        return 2;
    }
    else if( voltage >= CHR_VOLTAGE_20 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static uint32  read_adc_result( uint32 adc_src )
{
    volatile uint32  result;
    
    // clear ADC interrupt
	*(volatile uint32 *)(GR_ICLR) = 0x10;
	
	// Select ADC source	
	*(volatile uint32 *)(GR_ADCC) = adc_src;
	
	// wait ADC completed
    while( !(*(volatile uint32 *)(INT_IRQ_RAW_STS) & (INTCTL_ADC_IRQ) ) )
    {  ;  }
	
	// Read ADC result
	result  = *(volatile uint32 *)(GR_ADCR);
	
	DEBUG_PRINT("CHR:read_adc_result result is %d",result); 
	// Clear ADC interrupt
	*(volatile uint32 *)(GR_ICLR) = 0x10;
	
	return  (result & 0xFFFF);
}

static uint32 calculate_adc_average( ADC_E adcSource, uint32 adc_result )
{
	int i;
	uint32 max_adc = adc_result;
	uint32 min_adc = adc_result;
	uint32 adc, adc_total = 0;
	
	adc_total += adc_result;
	
    for (i = 0; i < 5; i++)
    {
       adc = read_adc_result(  1 << adcSource );//read adc continuously 
       DEBUG_PRINT("CHR:Check Power adc is %d",adc);
       
       if( adc > max_adc )
       {
           max_adc = adc;
       }

       if( adc < min_adc )
       {
           min_adc = adc;
       }
       
       adc_total += adc;
    }
        
    adc_total -= (min_adc + max_adc);  //avoid the incorrect adc value
    adc_total >>= 2;	
    return adc_total;
}
// charge state machine
static void charge_state_machine(
            CHR_EVENT event
            )
{
    uint32  tick, time_slot;
    static uint32  warning_count  = 0;
	uint32 shutdown_battery_vol = 0;
	uint32 current_module_vol = 0;
	uint32 warning_battery_vol = 0;
	uint32 capacity_full_batt_vol = 0;//this variable is used to check whether the battery is full.The battery vol will ramp down ,when in call ,so we will not use current_battery_voltage 
	BOOLEAN call_state = FALSE;
	static BOOLEAN call_state_last = FALSE;
	
    if( CHR_INIT_MAGIC != chr_init_flag )
    {
        return;
    }

    DEBUG_PRINT("CHR : chr_stm(%d : %d)", chr_state, event);

    switch( chr_state )
    {
        case CHR_IDLE:
            switch( event )
            {
                case CHR_DISCONNECT:
                    break;
                case CHR_UPDATE_MODULE:
                    // need notify client these message only when charge is not connect
                    current_module_vol = adc_to_voltage(ADC_MODULE, current_module_adc );
                    //if( current_module_adc <= SHUTDOWN_MODULE_ADC )
                    if (current_module_vol <= SHUTDOWN_MODULE_VOLTAGE)
                    {
                        SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_SHUTDOWN_IND, (void *)NULL );
                    }
                    break;
                case CHR_UPDATE_BATTERY:
                	//check if there is a call currently, and we will reduce shutdown vol threshold in call 
					//this func is used in Ni-Hi battery ,and there is no need to check call vol in li-battery
  
                	#ifdef  LI_BATTERY
                	//no need to check call state in li-battery
                		call_state = SCI_FALSE;
                	#else
                	//this func is used in Ni-Hi battery ,and there is no need to check call vol in li-battery
                		call_state = CC_IsInCall();
                	#endif
                	if ( call_state_last != call_state )
                	{
                		reset_battery_adc_var();
	                	call_state_last = call_state;
	                	return;
                	}

                	if (call_state)
                	{
                		shutdown_battery_vol = SHUTDOWN_BATTERY_VOLTAGE - VOLATGE_OFFSET;
                		warning_battery_vol = WARNING_BATTERY_VOLTAGE - VOLATGE_OFFSET;
                		capacity_full_batt_vol = current_battery_voltage + VOLATGE_OFFSET;//when we are in call ,the battery vol may be ramp down 0.2v
                	}
                	else
                	{
                		shutdown_battery_vol = SHUTDOWN_BATTERY_VOLTAGE;
                		warning_battery_vol = WARNING_BATTERY_VOLTAGE;
                		capacity_full_batt_vol = current_battery_voltage;
                	}
                	
                    if( (current_battery_voltage <= shutdown_battery_vol) && (!is_charge_connect()) )//if we can discover the charge connect ,it means that the charge can supply phone to work normally.
                    {
                        SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_SHUTDOWN_IND, (void *)NULL );
                    }
                    else if( (current_battery_voltage < warning_battery_vol) && (!is_charge_connect()) )
                    {
                        if( ++warning_count >= 2 )
                        {
                            warning_count = 0;
                            SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_WARNING_IND, (void *)NULL );
                        }
                        else
                        {
                            SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_CAP_IND, (void *)NULL );
                        }
                    }
                    else
                    {
                        warning_count  = 0;
                        SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_CAP_IND, (void *)NULL );
                    }

                    if( capacity_full_batt_vol < CHR_VOLTAGE_100 )
                    {
                        bCapacityFull = FALSE;
                    }
                    break;
                case CHR_CONNECT:
                    chr_state = CHR_CHARGING;
                    start_charge();
                    break;
                default:
                    break;
            }
            break;

        case CHR_CHARGING:
            switch( event )
            {
                case CHR_CONNECT:
                case CHR_UPDATE_MODULE:
                    break;
                case CHR_UPDATE_BATTERY:
	              	//when the difference of the voltage measured is less than 0.1v ,
	               	//and this situation last CHR_MAX_VP_TIME_SLOT secs
	               	//then the battery is full ,complete charge stop it
                    tick = SCI_GetTickCount();
                    if( current_battery_voltage > battery_voltage_saved + 100 )
                    {
                        battery_voltage_saved = current_battery_voltage;
                        battery_time_saved    = tick;
                    }
                    else
                    {
                        time_slot = get_time_slot( battery_time_saved, tick );
                        if(time_slot > CHR_MAX_VP_TIME_SLOT )
                        {
                            DEBUG_PRINT("CHR : time slot is too long and need stop charge");
                            bCapacityFull = TRUE;
                            chr_state     = CHR_IDLE;
                            stop_charge();
                        }
                    }

                    break;
                case CHR_DISCONNECT:
                    bCapacityFull = FALSE;
                    chr_state     = CHR_IDLE;
                    stop_charge();
                    break;
                default:
                    break;
                }
                break;

            default:
                break;
    } // end switch( chr_state )
}

static void adc_module_voltage_handler( void )
{
	uint32 adc_val;
    uint32 vol;
	static uint8 warning_count = 0;
	
	adc_val = CHG_GetVBATADCResult();
	
    DEBUG_PRINT("CHR:module check module adc is %d",adc_val);
    if (adc_val <= SHUTDOWN_MODULE_ADC)
    {
		adc_val = CHG_GetVBATADCResult( );
		vol = adc_to_voltage(ADC_MODULE, adc_val );
		
		DEBUG_PRINT("CHR:module caculate module ADC is %d",adc_val);
		DEBUG_PRINT("CHR:module caculate module Vol is %d",vol);
		
		if(vol <= SHUTDOWN_MODULE_VOLTAGE)
			//if (adc_val <= SHUTDOWN_MODULE_ADC)
        {
			DEBUG_PRINT("CHR:Shut down please module adc =%d, vol=%d",adc_val, vol);
	    	current_module_adc = adc_val;
	    	charge_state_machine( CHR_UPDATE_MODULE );
    	}
    }
    else
    {
		vol = adc_to_voltage(ADC_MODULE, adc_val );
		DEBUG_PRINT("CHR:module check module Vol is %d",vol);
		
		/*
		#ifdef LI_BATTERY
		if (vol <= WARNING_BATTERY_VOLTAGE)
		{
			if (warning_count++ >= 20) //40s warning ind to mmi
			{
				warning_count = 0;
                SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_WARNING_IND, (void *)NULL );
			}
		}
		#endif
		*/
		/*
		if (warning_count++ >= 10) //40s warning ind to mmi
		{
			warning_count = 0;
            SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_WARNING_IND, (void *)NULL );
		}
		*/		
    }

	
  g_module_adc=adc_val;
  g_module_voltage=vol;
}

static void adc_battery_voltage_handler(
            uint32 adc_source,
            uint32 adc_result
            )
{
	uint32	temp_vol;
    if( bFirstTime == TRUE)
    {
    	uint32 adc_val;
		adc_val = calculate_adc_average(ADC_SOURCE_BATTERY, adc_result);
        DEBUG_PRINT("CHR : First Time BatteryADC = %d", adc_val);
	    bFirstTime = FALSE;
	    
	    #ifndef LI_BATTERY
        current_battery_voltage = adc_to_voltage(ADC_BATTERY, adc_val );
        #else
        current_battery_voltage = adc_to_voltage(ADC_MODULE, adc_val );
        #endif
		DEBUG_PRINT("CHR : First Time BatteryVol = %d", current_battery_voltage);
		if (!is_charge_connect())
		{
        	reset_battery_adc_var();		
        	charge_state_machine( CHR_UPDATE_BATTERY );
        }
    }
	else
	{
		#ifdef LI_BATTERY
		adc_result = read_adc_result(  1 << ADC_SOURCE_BATTERY );
		#endif
		
	    if( battery_adc_count < ( (0x1 << CHR_BATT_ADC_SHIFT_NUM) + 2) )
	    {
	    	#ifndef LI_BATTERY
			temp_vol = adc_to_voltage(ADC_BATTERY, adc_result );
			#else
			temp_vol = adc_to_voltage(ADC_MODULE, adc_result );
			#endif
			DEBUG_PRINT("CHR : BATTERY adc =%d, vol =%d",adc_result, temp_vol);
			
	    	//这里完成对各种电池的电位检测的处理，电位检测对锂电池无效
	    	if (is_charge_connect())//if charge not connect ,no need to check battery out ,as it will power off when battery out and not charge
	    	{
	    		#ifndef LI_BATTERY
	    		if (adc_result >= CHARGE_ENABLE_BATTERY_OUT )//when charge enable and battery out, the adc is more than CHARGE_ENABLE_BATTERY_OUT
	    		{
	   				is_battery_in = FALSE;
    				bCapacityFull = FALSE;
	   				DEBUG_PRINT("CHR : BATTERY OUT 1 %d",adc_result);
	   				return;
	    		}
	    		else
	    		{
	    			is_battery_in = TRUE;
	    			DEBUG_PRINT("CHR : BATTERY in %d",adc_result);
	    		}
	    		#else
	    		
	    		#endif
	    	}
	    	
	        battery_adc_count++;
	        battery_adc_total += adc_result;

	        if( adc_result > battery_adc_max )
	        {
	            battery_adc_max = adc_result;
	        }

	        if( adc_result < battery_adc_min )
	        {
	            battery_adc_min = adc_result;
	        }
	        DEBUG_PRINT("CHR : BatteryADC = %d, count = %d", adc_result, battery_adc_count);
	    }
	    else
	    {
	        battery_adc_total  -= (battery_adc_min + battery_adc_max);
	        battery_adc_total >>= CHR_BATT_ADC_SHIFT_NUM;

			DEBUG_PRINT("CHR : Final Battery ADC = %d", battery_adc_total);
			
		    #ifndef LI_BATTERY
			current_battery_voltage = adc_to_voltage( ADC_BATTERY, battery_adc_total );
	        #else
			current_battery_voltage = adc_to_voltage( ADC_MODULE, battery_adc_total );
	        #endif			
			
	        DEBUG_PRINT("CHR : Final Battery Voltage = %d", current_battery_voltage);

	        reset_battery_adc_var();
	        charge_state_machine( CHR_UPDATE_BATTERY );
	    }
    }
    
	adc_module_voltage_handler();
}


// This is a dummy function.
static void CHRSVR_default(
            BLOCK_ID id,
            uint32   argc,
            void     *argv
            )
{
}

void Charge_CheckPowerStatus(void)
{
    // Check if charge in?
    if( !is_charge_connect() )
    {    
        // Get current value for ADC
        uint32  voltage, adc_val;        
        uint32  adc = read_adc_result(  1 << ADC_SOURCE_BATTERY );
        
		adc_val = calculate_adc_average(ADC_SOURCE_BATTERY, adc);
        
        #ifndef LI_BATTERY
        voltage = adc_to_voltage(ADC_BATTERY, adc_val);	
        #else
        voltage = adc_to_voltage(ADC_MODULE, adc_val);
        #endif	
        
        if (voltage <= CHR_VOLTAGE_POWERON) 
        {
        	DEBUG_PRINT("CHR:Power off %d",voltage);
            POWER_PowerOff();
        }
     }
}

// Init the charge
PUBLIC void CHR_Init(
            void
            )
{
	read_adc_calibration();	
	Charge_CheckPowerStatus();//check power on enable?
    SCI_CreateClientList(CHR_SERVICE, (CHR_MSG_MAX_NUM & 0x0ff), (REG_CALLBACK) CHRSVR_default);
    reset_battery_adc_var();
}

//  Report the capacity of battery periodically.
//  called by timer callback or when layer1 process one page( save battery capacity ).
PUBLIC void Charge_ReportCapacity(
            void
            )
{
    static uint8   counter = 0;

    if( CHR_INIT_MAGIC == chr_init_flag )
    {
    	//if something happen ,handle here and send correspond indication to the chargin state machine
        if( CHR_CHARGING == chr_state )
        {
        	//when chargin ,need to handle battery out ,and charge disconnect
            if( !is_charge_connect())
            {
                DEBUG_PRINT( ("CHR : charge not connect") );
	            bFirstTime = TRUE;
                charge_state_machine( CHR_DISCONNECT );
                return;
            }

            if( !GPIO_IsBattery() )
            {
                chr_state     = CHR_IDLE;
        		battery_time_saved = 0;
    			battery_voltage_saved = 0;       
                SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_CHARGE_END_IND, (void *)NULL ); //indicate charge stop
                SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_CAP_IND, (void *)NULL );//indicate current battery capacity to MMI             
                return;
            }
        }
        else if (CHR_IDLE == chr_state)
        {
        	//when idle ,need charge?
            if (need_charge())
            {
                DEBUG_PRINT( ("CHR :charge connect") );
                charge_state_machine( CHR_CONNECT );
                return;
            }
            if (!GPIO_IsBattery())//if the charge connect ,and battery is full ,so the state is idle,and then battery out ,enter this state
            {
                SCI_SendEventToClient( CHR_SERVICE, (uint32)CHR_CAP_IND, (void *)NULL );//indicate current battery capacity to MMI             
            }
        }

			//如果充电结束，这时候电池的电量为满，为了保护电池，要在适当的时候enable charge 然后在disable charge
			//也就是在这种情况下有四分之一的时间在充电，其他的时间没有充电
			//如果电池没有满的话就不用这样充电了
	    if (bCapacityFull)
	    {
	        if (counter == 0)
	        {
	            if ((is_charge_connect()) && is_battery_in )//首先如果有充电器插入的话 disable charge 两秒
			    {
				    enable_charge(0);
				    counter++;
				    return;
			    }
	        }
	        else if (counter < 6)
	        {
	        	counter++;
	        	return;//do not get the battery adc to calculate average when disable charge ,as the value get is not correct!
	        }
	        else if (counter == 6)
	        {
	            if ((is_charge_connect()) && is_battery_in )//首先如果有充电器插入的话 enable charge 两秒
			    {
				    enable_charge(1);
				    counter++;
				    return;
			    }
	        }

            if (++counter >= 8)
            {
                counter = 0;
            }
	    }
		
		//#ifndef LI_BATTERY 
        ADC_Start( 1/*ADC_SOURCE_BATTERY*/, adc_battery_voltage_handler, FALSE );
        //#else
        //adc_module_voltage_handler();
        //#endif
        
    }
}

// get the battery capacity
PUBLIC uint32 CHR_GetBatCapacity(
              void
              )
{
    if( CHR_INIT_MAGIC != chr_init_flag )
    {
        // this function should be called after init the chr.
        // Pls call CHR_Init first.
        SCI_ASSERT( 0 );
        return 0;
    }
    else
    {
    	if (!GPIO_IsBattery())
    	{
			SCI_TraceLow("GPIO_IsBattery=0");

    	 	return DISPLAY_NO_BATTERY;//如果电池不在位我们在MMI上显示电池格数
    	}
    	else
    	{
			SCI_TraceLow("g_module_voltage=%d",g_module_voltage);

    		return voltage_to_percentum( g_module_voltage );//电池在位正常显示
    	}
    }
}

PUBLIC BOOLEAN CHR_IsChargeConnect(
			   void
			   )
{
	return GPIO_IsChargeConnected();
}

/*****************************************************************************
 ** Charge Manager Module ( AT Command )

Charge Manager Module Requirement:
    1：stop the battery charge.
    2：Start the battery charge.
    3：Get the current battery voltage.
    4：Set the battery charge parameters.
        The below paramters:
        (1)：level max for the battery voltage.When it is reached,
             the battery is considered as charged.
             (give the allowed range and the default value)
        (2)：level min for the battery voltage.When it is reached ,
             the battery is considered as discharged, and the module is turn off.
             (give the allowed range and the default value)
        (3)：Time between pulses for the pulsed charge.
             (give the allowed range and the default value)
        (4)：time pulse out charge.
             (give the allowed range and the default value)
        (5)：battery internal resistor.
             (give the allowed range and the default value)
    5:  Get the current battery state. The state will include:
        (0):  Minimum battery level reached - the battery is considered as discharged.
              The product is turned off (as with the +CPOF command).
        (1):  Maximum battery level reached - the battery is considered as charged.
              The battery charging is stopped.
        (2):  Battery currently in charge.
        (3):  Battery currently out of charge.


 *****************************************************************************/

#define CHR_TPULSE_IN_CHARGE_HIGH               10000
#define CHR_TPULSE_IN_CHARGE_LOW                100
#define CHR_TPULSE_IN_CHARGE_DEFAULT            100

#define CHR_TPULSE_OUT_CHARGE_HIGH              10000
#define CHR_TPULSE_OUT_CHARGE_LOW               100
#define CHR_TPULSE_OUT_CHARGE_DEFAULT           5000

#define CHR_BATT_INTERNAL_RESISTOR_HIGH         255
#define CHR_BATT_INTERNAL_RESISTOR_LOW          1
#define CHR_BATT_INTERNAL_RESISTOR_DEFAULT      1

static uint32  chr_batt_level_max  = CHR_MAX_LEVEL_DEFAULT;
static uint32  chr_batt_level_min  = CHR_MIN_LEVEL_DEFAULT;


PUBLIC void CHR_GetBattIntResParam(
            uint32 * low,
            uint32 * high,
            uint32 * deft
            )
{
    * low  = CHR_BATT_INTERNAL_RESISTOR_LOW;
    * high = CHR_BATT_INTERNAL_RESISTOR_HIGH;
    * deft = CHR_BATT_INTERNAL_RESISTOR_DEFAULT;
}

PUBLIC uint32 CHR_SetBattIntRes(
              uint32 level
              )
{
    if(     (level <= CHR_BATT_INTERNAL_RESISTOR_HIGH)
        &&  (level >= CHR_BATT_INTERNAL_RESISTOR_LOW) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

PUBLIC void CHR_GetTPulseOutChargeParam(
            uint32 * low,
            uint32 * high,
            uint32 * deft
            )
{
    * low  = CHR_TPULSE_OUT_CHARGE_LOW;
    * high = CHR_TPULSE_OUT_CHARGE_HIGH;
    * deft = CHR_TPULSE_OUT_CHARGE_DEFAULT;
}

PUBLIC uint32 CHR_SetTPulseOutCharge(
              uint32 level
              )
{
    if(     (level <= CHR_TPULSE_OUT_CHARGE_HIGH)
        &&  (level >= CHR_TPULSE_OUT_CHARGE_LOW) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

PUBLIC void CHR_GetTPulseInChargeParam(
            uint32 * low,
            uint32 * high,
            uint32 * deft
            )
{
    * low  = CHR_TPULSE_IN_CHARGE_LOW;
    * high = CHR_TPULSE_IN_CHARGE_HIGH;
    * deft = CHR_TPULSE_IN_CHARGE_DEFAULT;
}

PUBLIC uint32 CHR_SetTPulseInCharge(
              uint32 level
              )
{
    if( (level <= CHR_TPULSE_IN_CHARGE_HIGH) && (level >= CHR_TPULSE_IN_CHARGE_LOW) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

PUBLIC void CHR_GetBattLevelMaxParam(
            uint32 * low,
            uint32 * high,
            uint32 * deft
            )
{
    * low  = CHR_MAX_LEVEL_LOW;
    * high = CHR_MAX_LEVEL_HIGH;
    * deft = CHR_MAX_LEVEL_DEFAULT;
}

PUBLIC uint32 CHR_SetBattLevelMax(
              uint32 level
              )
{
    if( (level <= CHR_MAX_LEVEL_HIGH) && (level >= CHR_MAX_LEVEL_LOW) )
    {
        chr_batt_level_max = level;
        return 1;
    }
    else
    {
        // Invalid param.
        return 0;
    }
}

PUBLIC void CHR_GetBattLevelMinParam(
            uint32 * low,
            uint32 * high,
            uint32 * deft
            )
{
    * low  = CHR_MIN_LEVEL_LOW;
    * high = CHR_MIN_LEVEL_HIGH;
    * deft = CHR_MIN_LEVEL_DEFAULT;
}

PUBLIC uint32 CHR_SetBattLevelMin(
              uint32 level
              )
{
    if( (level <= CHR_MIN_LEVEL_HIGH) && (level >= CHR_MIN_LEVEL_LOW) )
    {
        chr_batt_level_min = level;
        return 1;
    }
    else
    {
        return 0;
    }
}

PUBLIC void CHR_StopCharge(
            void
            )
{
}

PUBLIC void CHR_StartCharge(
            void
            )
{
}

// return the m-Voltage of current battery voltage
PUBLIC uint32 CHR_GetBattVoltage(
              void
              )
{
    return  current_battery_voltage;
}

PUBLIC CHR_BATT_STATE_E CHR_GetBattState(
                        void
                        )
{
    CHR_BATT_STATE_E state;

    if( CHR_IDLE == chr_state )
    {
        if( current_battery_voltage >= chr_batt_level_max )
        {
            state = CHR_BATT_MAX_VOLTAGE;
        }
        else if( current_battery_voltage <= chr_batt_level_min )
        {
            state = CHR_BATT_MIN_VOLTAGE;
        }
        else
        {
            state = CHR_BATT_OUT_OF_CHARGE;
        }
    }
    else
    {
        state = CHR_BATT_IN_CHARGE;
    }

    return state;
}

uint32 CHR_CheckBatteryStatus(void)
{  
    return CHR_CheckBatteryStaus();
}

PUBLIC uint32 CHR_CheckBatteryStaus(
              void
              )
{
    if( CHR_INIT_MAGIC != chr_init_flag )
    {
        // this function should be called after init the chr.
        // Pls call CHR_Init first.
        SCI_ASSERT( 0 );
        return CHR_BATTERY_NONE_S;
    }

    switch( chr_state )
    {
        case CHR_IDLE:
            return CHR_BATTERY_NORMAL_S;
        case CHR_CHARGING:
            return CHR_BATTERY_CHARGING_S;
        default:  // the state is not defined.
            SCI_ASSERT( 0 );
            return CHR_BATTERY_NORMAL_S;
    }
}

/**---------------------------------------------------------------------------*
 ** DESCRIPTION                                                               *
 **     Get the voltage of battery from ADC result.                           *
 ** Input Parameter                                                           *
 **     None                                                                  *
 ** Retrun                                                                    *
 **     the voltage of battery.  The units is 0.01 V.                         *
 **---------------------------------------------------------------------------*/
uint32 CHR_GetVoltage( void )
{
	return CHR_GetBattVoltage();	
}

/**---------------------------------------------------------------------------*
 ** DESCRIPTION                                                               *
 **     Get the current voltage of battery from ADC result.                   *
 ** Input Parameter                                                           *
 **     None                                                                  *
 ** Retrun                                                                    *
 **     the current voltage of battery.  The units is 0.01 V.                 *
 **---------------------------------------------------------------------------*/
uint32 CHR_GetCurVoltage( void )
{
	// Driver,
	// return adc_to_battery_voltage(current_adc);	
	return CHR_GetBattVoltage();
}


/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif
