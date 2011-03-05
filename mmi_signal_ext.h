#ifndef MMI_SIGNAL_EXT_H
#define MMI_SIGNAL_EXT_H
    
#define MMI_SIGNAL_BASE  0xF000
#include "mn.h"


typedef struct
{
    MN_GPRS_ERR_E result;
}MMI_GPRS_T;

// add by allen 2004.09.28
typedef struct
{
	uint32	param1;
	uint32	param2;
} MMI_L1_MESSAGE;


typedef enum
{
	/* MMI Signal */
	SIG_MMI_KEY_PRESS_IND,	

	//MMI SIGNAL list */
	SIG_APP_MN_NETWORK_STATUS_IND ,	
	SIG_APP_MN_PLMN_LIST_CNF,	
	SIG_APP_MN_PLMN_SELECT_CNF,	
	SIG_APP_MN_SIM_GET_PIN_IND,	
	SIG_APP_MN_SIM_READY_IND,		
	SIG_APP_MN_SIM_NOT_READY_IND,	
	SIG_APP_MN_SIM_PIN_FUNC_CNF,	
	SIG_APP_MN_SCELL_RSSI_IND,	
    SIG_APP_MN_MS_POWERING_OFF_IND, 
    SIG_APP_MN_MS_POWER_OFF_CNF,	
	SIG_APP_MN_NCELL_RSSI_CNF,	
    SIG_APP_MN_AOC_MODIFIED_IND,   
	SIG_APP_MN_FDN_SERVICE_IND,		
	SIG_APP_MN_GPRS_ATTACH_RESULT,	
	SIG_APP_MN_GPRS_DETACH_RESULT,	
	SIG_APP_MN_PUCT_INFO_CNF,		
	SIG_APP_MN_PS_READY_IND,
			
	/* be used for (mn_TASK,sim_module) */	
    SIG_APP_MN_MSISDN_UPDATE_CNF_F,
    SIG_APP_MN_ADN_UPDATE_CNF_F,
    SIG_APP_MN_FDN_UPDATE_CNF_F,

	/* be used for (mn_TASK,call_module) */	

   SIG_APP_MN_CALL_READY_IND,   
   SIG_APP_MN_CALL_START_IND,	
   SIG_APP_MN_ALERTING_IND,	
   SIG_APP_MN_SETUP_COMPLETE_IND,	
   SIG_APP_MN_SETUP_COMPLETE_CNF,   
   SIG_APP_MN_CALL_DISCONNECTED_IND,	
   SIG_APP_MN_CALL_ERR_IND,	
   SIG_APP_MN_SETUP_IND,	
   SIG_APP_MN_START_DTMF_CNF,	
   SIG_APP_MN_STOP_DTMF_CNF,    

   SIG_APP_MN_CALL_HELD_CNF,	
   SIG_APP_MN_CALL_HELD_IND,    
   SIG_APP_MN_CALL_HOLDING_IND,
   SIG_APP_MN_CALL_RETRIEVED_CNF,	
   SIG_APP_MN_CALL_RETRIEVED_IND, 
   SIG_APP_MN_CALL_RETRIEVING_IND,
   SIG_APP_MN_BUILDING_MPTY_IND,
   SIG_APP_MN_SPLITTING_MPTY_IND,
   SIG_APP_MN_CALL_SWAP_IND,	
   SIG_APP_MN_CALL_WAIT_IND,	

   SIG_APP_MN_CALL_NOTIFY_IND,	
   SIG_APP_MN_CALL_MODIFY_CNF,	
   SIG_APP_MN_CALL_MODIFY_IND,	
   SIG_APP_MN_CALL_BARRED_IND,	
   SIG_APP_MN_BUILD_MPTY_CNF,
   SIG_APP_MN_BUILD_MPTY_IND,
   SIG_APP_MN_SPLIT_MPTY_CNF,
   SIG_APP_MN_ACMMAX_IND,
   SIG_APP_MN_TMPTY_EXP_IND,
   SIG_APP_MN_SYNC_IND,
   SIG_APP_MN_PROGRESS_IND,
   SIG_APP_MN_USER_TO_USER_IND,
   SIG_APP_MN_DISCONNECTING_IND,


    SIG_APP_MN_CALL_FORWARD_IND,	
    SIG_APP_MN_FORWARD_STATUS_IND,
	SIG_APP_MN_CONNECTING_IND,   

    SIG_APP_MN_MTACT_RESPOND_MODE_CNF, 
    SIG_APP_MN_ACTIVATE_PDP_CONTEXT_CNF, 
    SIG_APP_MN_SET_QOS_CNF, 
    SIG_APP_MN_SET_MIN_QOS_CNF, 
    SIG_APP_MN_SET_PDP_CONTEXT_CNF, 
    SIG_APP_MN_DEACTIVATE_PDP_CONTEXT_CNF,
    SIG_APP_MN_DELETE_PDP_CONTEXT_CNF,  
    SIG_APP_MN_DELETE_MIN_QOS_CNF,  
    SIG_APP_MN_DELETE_QOS_CNF, 

	/* be used for mnsms task */

	SIG_APP_MN_SMS_READY_IND              , 
	SIG_APP_MN_SMS_MEM_FULL_IND_F		  ,
    SIG_APP_MN_READ_SMS_CNF               , 
    SIG_APP_MN_UPDATE_SMS_STATE_CNF       , 
    SIG_APP_MN_READ_SMS_STATUS_REPORT_CNF , 
    SIG_APP_MN_SEND_SMS_CNF               , 
    SIG_APP_MN_WRITE_SMS_CNF              , 
    SIG_APP_MN_SMS_IND                    , 
    SIG_APP_MN_SMS_STATUS_REPORT_IND           , 
    SIG_APP_MN_READ_SMS_PARAM_CNF         , 
    SIG_APP_MN_WRITE_SMS_PARAM_CNF        , 
	SIG_APP_MN_DELETE_ALL_SMS_CNF		  , 

    /* be used for mnsmscb task */ 

    SIG_APP_MN_SMSCB_MSG_IND             , 
	SIG_APP_MN_SMSCB_MACRO_MSG_IND       , 
	SIG_APP_MN_SMSCB_REJ_IND             , 

	/* be used for mnss task */		

	SIG_APP_MN_GET_PASSWORD_IND					, 
	SIG_APP_MN_USSD_IND							, 
	SIG_APP_MN_USSD_NOTIFY_IND					, 
	SIG_APP_MN_RELEASE_SS_IND					, 
	SIG_APP_MN_BEGIN_REGISTER_SS				, 
	SIG_APP_MN_BEGIN_ERASE_SS					, 
	SIG_APP_MN_BEGIN_ACTIVATE_SS				, 
	SIG_APP_MN_BEGIN_DEACTIVATE_SS				, 
	SIG_APP_MN_BEGIN_INTERROGATE_SS				, 
	SIG_APP_MN_BEGIN_REGISTER_PASSWORD			, 
	SIG_APP_MN_BEGIN_USSD_SERVICE				, 
	SIG_APP_MN_REGISTER_SS_CNF					, 
	SIG_APP_MN_ERASE_SS_CNF						, 
	SIG_APP_MN_ACTIVATE_SS_CNF					, 
	SIG_APP_MN_DEACTIVATE_SS_CNF				, 
	SIG_APP_MN_INTERROGATE_CNF				, 
	SIG_APP_MN_REGISTER_PASSWORD_CNF			, 
	SIG_APP_MN_SS_BUSYING_IND					, 
	SIG_APP_MN_USSD_SERVICE_CNF					, 

	// LCD timer used 
	SIG_LCD_CURSOR_BLINK_TIMER,
	SIG_LCD_ICON_BLINK_TIMER,
	SIG_LCD_ANIMATION_TIMER,
	SIG_LCD_BATTERY_SCROLL_TIMER,

	
   //MMK timer,
	SIG_MMI_KBD_REP_TIMER,
    SIG_MMI_KBD_KBD_TIMER,
    SIG_MMI_SCREEN_TIMER,
    SIG_MMI_INPUT_ALPHA_TIMER,

    SIG_MMK_TEMP_CTXT_TIMER,
    SIG_MMK_TMP_SHOW_MENU_TIMER,
	// MMI timer
//	SIG_MMI_SYS_SHOW_TIMER,
    SIG_MMI_BACKLIGHT_TIMER,
	// the customer timer
	SIG_MMI_CUSTOMER_TIMER,

	//cc timer
	SIG_MMI_CC_DISPLAY_TIMER,
	// IM TIMER
	SIG_MMI_IM_EXPL_TIMER,
    SIG_TIMER_EXPIRY,
	// MISC event
	SIG_MMI_MISC_TIME_IND,
	SIG_MMI_MISC_DATE_IND,
	SIG_MMI_MISC_ALARM_IND,

	/* be used for charge task */
	SIG_CHR_CAP_IND,
	SIG_CHR_CHARGE_START_IND,
	SIG_CHR_CHARGE_END_IND,
	SIG_CHR_WARNING_IND,
	SIG_CHR_SHUTDOWN_IND

}SignalId;


typedef struct 
{
	int16   dummy;
}EmptySignalS;

// keypad signal 

typedef struct MmiKeyPressTag
{
    uint32    keyCode; // modify by louis.wei to keep accord with the Ref
}MmiKeyPress;


/* timer expiry signal structure */
typedef struct MmiTimerExpirySTag
{
    /*
    if timerId is not the same as originally returned to the user
    then ignore this signal
    */
    uint8                         timerId;
    uint8                         ref_num;
   //int16                          userValue;
}MmiTimerExpiryS;
/*************************************************************jakle add mmi signa.h"********/


/*****************************The signal structure****************************/

/*
the signal indicate the mnsms module is ok!
*/
typedef struct
{
	uint16   dummy;
} MnSmsReadyIndS;

/*
the signal indicate the memory used to save short message is full
*/
typedef struct
{
	uint16   dummy;
} MnSmsMemFullIndS;

/*
 the load sms confirm structure , respond to the MNSMS_load_sms()
*/
typedef struct
{
	MN_SMS_CAUSE_E     cause    ;         // the opreate cause
	// MN_SMS_RECORD_ID_T record_id;         // the sms position 
	MN_SMS_STORAGE_E   storage; // where the short message is saved
    MN_SMS_RECORD_ID_T record_id;         // the sms position
	MN_SMS_STATUS_E    status    ;         // the sms state
	MN_SMS_U_T         sms_t_u  ;          // the sms umion , mt or mo
} MnReadSmsCnfS;

/*
 the update sms confirm structure , respond to the MNSMS_update_sms()
*/
typedef struct
{

	MN_SMS_CAUSE_E     cause    ;         // the operate cause
	MN_SMS_RECORD_ID_T record_id;         // the sms position 
	MN_SMS_STORAGE_E   storage  ;		  // where the short message is saved
} MnUpdateSmsStateCnfS;

/*
  the read sms's status report structure, 
*/

typedef struct 
{
	MN_SMS_CAUSE_E   cause;         // the result of read status report 
	MN_SMS_STORAGE_E    storage;       // where the corresponded message is saved
	MN_SMS_RECORD_ID_T record_id;   // the index of corresponded message
	MN_SMS_STATUS_REPORT_T status_report;// the status report according to a short message
} MnReadSmsStatusReportCnfS;

/*
 the send sms confirm structure , it respond the MNSMS_send_sms () or MNSMS_send_command_sms()
*/
typedef struct 
{

	MN_SMS_CAUSE_E     cause    ;  // the operate cause
	//????? Maybe we will not use the record_id in future
//	MN_SMS_RECORD_ID_T record_id;  // the sms position
    MN_SMS_TP_MR_T     tp_mr;
} MnSendSmsCnfS;

/*
 the save sms confirm structure , it respond the MNSMS_save_mo_sms() or MNSMS_save_mt_sms() 
*/
typedef struct
{
	MN_SMS_CAUSE_E     cause    ; // the operate cause
	MN_SMS_RECORD_ID_T record_id; // the sms position
	MN_SMS_STORAGE_E   storage;       // where the corresponded message is saved
} MnWriteSmsCnfS;

/*
 the sms indication structure , it indicate a mt sms to the mmi
*/
typedef struct
{

	MN_SMS_CAUSE_E		cause;		// the operate cause
	MN_SMS_RECORD_ID_T	record_id;	// the sms position		
	MN_SMS_STORAGE_E	storage;	// message class
	MN_SMS_MT_SMS_T		sms_t;		// the mt sms structure   
}MnSmsMtIndS;

/*
 the state report indication structure, it indicate a status report to mmi
*/
typedef struct
{
	
	MN_SMS_STORAGE_E       storage;   // where the sms is saved
	MN_SMS_RECORD_ID_T  record_id;  // the relating sms position
	MN_SMS_STATUS_REPORT_T  status_report;  // whether the short message is received by SME
} MnSmsReportIndS;

/*
 the load parameter confirm structure , it respond the MNSMS_load_param()
*/
typedef struct
{
	MN_SMS_CAUSE_E     cause    ;  // the operate cause
	MN_SMS_RECORD_ID_T record_id;  // the parameter position
	MN_SMS_PARAM_T     param_t;    // the parameter structure
} MnReadSmsParamCnfS;

/*
the update the mo sms confirm struct
*/
typedef struct
{
	MN_SMS_CAUSE_E     cause    ; // the operate cause
	MN_SMS_RECORD_ID_T record_id; // the sms position
	MN_SMS_STORAGE_E       storage; // where the short message is saved
	MN_SMS_STATUS_E		status; // the sms state
} MnEditSmsCnfS;

/*
the signal indicate the memory of save short message is full
*/
typedef struct
{
	MN_SMS_MEM_FULL_STATUS_E	mem_status;
}MnMemFullSmsInd;


/*
give the result of setting sms receive function , it respond the MNSMS_SetSmsReceiveFunc()
*/
typedef struct 
{
	MN_SMS_CAUSE_E	cause;
	BOOLEAN			is_receive_sms;
} MnSetSmsReceiveFuncCnf;

/*
 the update the paramter confirm sturcure , it respond the MNSMS_update_param()
*/
typedef struct
{
	
	MN_SMS_CAUSE_E     cause    ;  // the operate cause
	MN_SMS_RECORD_ID_T record_id;  // the parmeter position
} MnWriteSmsParamCnfS;

/*
	the result of delete all short message
*/
typedef struct
{

	MN_SMS_CAUSE_E     cause    ;  // the operate cause
}MnDeleteAllSmsCnfS;
/*
	Indicate reading sms states
*/
typedef struct
{
   int16   dummy;
} MnReadingSmsStateIndS;

/*
	Indicate reding sms
*/
typedef struct
{
   int16 dummy;
}MnReadingSmsIndS;

/*
	Indicate updating sms state
*/
typedef struct
{
   int16 dummy;
} MnUpdatingSmsStateIndS;

/*
	Indicate sending sms
*/
typedef struct
{
   int16 dummy;
} MnSendingSmsIndS;

/*
	Indicate saving sms 
*/
typedef struct
{
   int16 dummy;
} mnSavingSmsIndS;

/*
	Indicate reading sms param;
*/
typedef struct
{
   int16 dummy;
} MnReadingSmsParamIndS;

/*
	Indicate writing sms param
*/
typedef struct
{
   int16 dummy;
} MnWritingSmsIndS;


 /*****************************************************************************
 **                        SS MODULE DEFINE                                   *
 ******************************************************************************/

/* define signals between App-layer with MN-layer */
typedef struct{
	int16 dummy;
}	MnSsBeginReisterS,
    MnSsBeginEraseS,
	MnSsBeginActiveS,
	MnSsBeginDeactiveS,
	MnSsBeginInterrogateS,
	MnSsBeginRegisterPasswordS,
	MnSsBussyingIndS;

// 	
typedef struct{

	MN_SS_OPERATE_RESULT_E				result_type;	// success, error or reject
	uint8                               list_num;       // number of interrogate information structure
	MN_SS_INTERROGATE_INFO_T        	info_list[MN_SS_MAX_NUM_OF_FEATURE_LIST];
	MN_SS_ERROR_CODE_E					err_code;		// if cnf-type = cnf_error, the parameter is included
}	MnSsInterrogateCnfS,
    MnSsRegisterCnfS,
	MnSsEraseCnfS,
	MnSsActivateCnfS,
	MnSsDeactivateCnfS;
	

typedef struct{

	MN_SS_OPERATE_RESULT_E	result_type;	// success, error or reject
	MN_SS_ERROR_CODE_E		err_code;		// if cnf-type = cnf_error, the parameter is included
}	MnSsRegisterPasswordS;


typedef struct{

	MN_SS_PASSWORD_GUIDANCE_INFO_E	    guidance_info;	// guide user to input password which is needed
}MnSsGetPasswordIndS;
	

typedef struct{
	
	MN_SS_USSD_DATA_T			        ussd_data;
}	MnSsUssdIndS,
	MnSsUssdNotifyIndS,
	MnSsBegingUssdServiceS;
	

typedef struct{
	
	MN_SS_OPERATE_RESULT_E	            result_type;	// success, error or reject
	MN_SS_USSD_DATA_T		            ussd_data;      // result data
	MN_SS_ERROR_CODE_E		            err_code;		// if cnf-type = cnf_error, the parameter is included
}	MnSsUssdServiceCnfS;


typedef struct{

    BOOLEAN		                        error_present;		// whether gsm cause present in signal
	MN_SS_ERROR_CODE_E	                error_code;			// release cause
}	MnSsReleaseIndS;
		// this signal used network release ss without ss operation


 /*****************************************************************************
 **                        PHONE MODULE DEFINE                                *
 ******************************************************************************/
 




typedef EmptySignalS MnPowerOffIndS;
//  struct of signal: APP_MN_MS_POWER_OFF_CNF_T
//  this signal indicate MMI that power off complete
typedef EmptySignalS  MnPowerOffCnfS;

//  struct of signal: APP_MN_PS_READY_IND_T
//  this signal indicate MMI that MS waitting for MMI's command now
typedef EmptySignalS  MnPSReadyIndS;



// this signal is used to report mmi call server ready
typedef struct
{
    BOOLEAN     fdn_is_ready;   // fdn is ready.
}MnCallReadyIndS;


//  struct of signal: APP_MN_AOC_INCREASE_ACM_IND
//  this signal indicate phone status to MMI 
typedef struct 
{
    BOOLEAN  is_ok;
    BOOLEAN  is_reach_max;
    uint32   acm_unit;
    uint32   acm_max;
} MnAocModifyIndS;

//  struct of signal: APP_MN_FDN_SERVICE_IND
//  this signal indicate fdn enable/disable to MMI 
typedef struct
{

    BOOLEAN  is_fdn_enable;
}MnFdnServiceIndS;


//  struct of signal: APP_MN_NETWORK_STATUS_IND
//  this signal indicate phone status to MMI 
typedef struct 
{
    MN_PHONE_PLMN_STATUS_E  plmn_status;        // plmn statue			
    BOOLEAN					plmn_is_selecting;  // if plmn is in selecting
    BOOLEAN					plmn_is_roaming;    // if plmn is roaming plmn	
//	MN_PLMN_T				current_plmn;       // selected plmn
	uint16					mcc;
	uint16					mnc;
	uint16					mnc_digit_num;
	uint16					lac;
    uint16					cell_id;			
} MnNetworkStatusIndS;
 

//  struct of signal: APP_MN_PLMN_LIST_CNF_T
//  this signal indicate plmns's detail to MMI
typedef struct 
{
 
    MN_PHONE_OPERATE_RESULT_E   is_valid_result;
	MN_PHONE_PLMN_INFO_LIST_T   plmn_detail_list; // PLMN detail list
}MnPlmnListCnfS;
 

//  struct of signal: APP_MN_PLMN_SELECT_CNF_T
//  this signal indicate MMI that PLMN select complete
typedef struct 
{
 
	BOOLEAN					plmn_is_selected;       // is selected?
    MN_PHONE_PLMN_STATUS_E  plmn_status;			  // plmn status
	BOOLEAN		    		mode_is_manual;         // is manual?
//	MN_PLMN_T				selected_plmn;          // selected plmn
	uint16					mcc;
	uint16					mnc;
	uint16					mnc_digit_num;
	uint16					lac;
    uint16					cell_id;			
}MnPlmnSelectCnfS;


//  struct of signal: APP_MN_SIM_GET_PIN_IND_T
//  this signal indicate MMI to input PIN1 value
typedef struct 
{

	MNSIM_PIN_INDICATION_E  pin_num;  // SIM_PIN_1 always
	BOOLEAN				  pin_is_blocked;	// PIN1 or blocked PIN1
}MnSimGetPinIndS;


//  struct of signal: APP_MN_SIM_READY_IND_T
//  this signal indicate MMI that SIM card init OK
typedef struct 
{

	BOOLEAN				   pin1_is_enabled; 	// PIN1 enabled/disabled when power on
	BOOLEAN				   pin1_is_blocked;		// is pin1 blocked?  
	MNSIM_PIN_STATUS_T 	   pin1_status;         // pin1 status
	MNSIM_PIN_STATUS_T	   pin2_status;         // pin2 status
	MNSIM_PIN_STATUS_T	   unblock_pin1_status;	// blocked PIN1 status
	MNSIM_PIN_STATUS_T	   unblock_pin2_Status;	// blocked PIN2 status
	BOOLEAN				   is_pin2_required_acm_max; // PIN2 is req'd for ACMmax update 
	BOOLEAN				   is_pin2_Required_acm;	 // PIN2 is req'd for Acm update 
	BOOLEAN				   is_pin2_Required_puct;	 // PIN2 is req'd for PUCT update 
	MN_IMSI_T			   imsi;			    // international mobile subscriber id 
	MN_PLMN_T			   hplmn;
	MN_LOCI_T			   location_info;		// Location area information 
	MNSIM_PHASE_E		   phase;			    // phase 1 or 2 
	MNSIM_SST_T   		   sim_service;		    // sim service table 
	MNSIM_ADMIN_T		   admin;               
}MnSimReadyIndS;
 

//  struct of signal: APP_MN_SIM_NOT_READY_IND_T
//  this signal indicate MMI that SIM not ready
typedef struct 
{
 
	MNSIM_ERROR_E reason;  // reason why SIM not ready
}MnSimNotReadyIndS;
 

//  struct of signal: APP_MN_SIM_PIN_FUNC_CNF_T
//  this signal indicate MMI that operation on PIN compete
typedef struct 
{
 
	MNSIM_PIN_REQ_RESULT_E				pin_operate_result;
	MNSIM_PIN_REQ_TYPE_E	    		pin_function_type;	// Verify, change, disable, enable, or unblock.
	MNSIM_PIN_INDICATION_E				pin_num;			// pin1 or pin2
	MNSIM_PIN_STATUS_T					pin_status;		    // Number of remaining retries and initialisation status
	MNSIM_PIN_STATUS_T					unblock_pin_status;	// Number of remaining retries and initialisation status
} MnSimPinFuncCnfS;


//  struct of signal: APP_MN_GPRS_ATTACH_CNF_T
//  this signal indicate MMI that operation on PIN compete
typedef struct 
{

	MN_PHONE_GPRS_ATTACH_TYPE_E	attach_type;
	BOOLEAN						is_attach_ok;
}MnGprsAttachResultS;
 

//  struct of signal: APP_MN_GPRS_DETACH_CNF_T
//  this signal indicate MMI that operation on PIN compete
typedef struct 
{
 

	MN_PHONE_GPRS_DETACH_TYPE_E	detach_type;
	BOOLEAN						is_detach_ok;
}MnGprsDetachResultS;
 

//  struct of signal: APP_MN_SCELL_RSSI_IND_T
//  this signal indicate MMI service cell power level
typedef struct
{
	uint16      arfcn;
    uint8       bsic;
	BOOLEAN     is_traffic_mode;
	// idle info
    uint8	    rxlev;
	// traffic info
	BOOLEAN	    is_dtx_used;
	uint16	    rxqual_full;
	uint16      rxqual_sub;
	uint16	    rxlev_full;
	uint16	    rxlev_sub;
	uint16	    cellid;
	uint16	    lac;
}MnScellRssiIndS;
 

//  struct of signal: APP_MN_NCELL_RSSI_CNF_T
//  this signal indicate MMI neighbour cell power level
typedef struct
{

	uint16							ncell_num;   // Measured neighbour cell number 
	MN_PHONE_CELL_MEAS_T			ncell_power[MN_MAX_NCELL_NUM]; // Neighbour cells measurement report 
}MnNcellRssiCnfS;
 

//  struct of signal: APP_MN_PUCT_INFO_CNF_T
//  this signal indicate puct info
typedef struct
{

	BOOLEAN			   is_ok;
    BOOLEAN 		   is_service_support;
	MNSIM_PUCT_INFO_T  puct_info;
}MnPuctInfoCnfS;
 

 /*****************************************************************************
 **                        SIM MODULE DEFINE                                *
 ******************************************************************************/

//  struct of signal: APP_MN_MSISDN_UPDATE_CNF_T
//  this signal is cnf msisdn update or delete or add
typedef struct
{
    MN_MSISDN_OPERATE_TYPE_E    operate_type;
    BOOLEAN                     is_ok;
} MnMsisdnUpdateCnfS;

//  struct of signal: APP_MN_ADN_UPDATE_CNF_T
//  this signal is cnf adn update or delete or add
typedef struct
{
    MN_ADN_OPERATE_TYPE_E       operate_type;
    BOOLEAN                     is_ok;
} MnAdnUpdateCnfS;

//  struct of signal: APP_MN_FDN_UPDATE_CNF_T
//  this signal is cnf fdn update or delete or add
typedef struct
{
    MN_FDN_OPERATE_TYPE_E       operate_type;
    BOOLEAN                     is_ok;
} MnFdnUpdateCnfS;


 /*****************************************************************************
 **                        CALL MODULE DEFINE                                *
 ******************************************************************************/


// indicate the service status of call forwarding to MMI 
typedef struct
{

	MN_SS_CODE_E                   forward_operation;  // the type of call forwarding 
	MN_SS_STATUS_E                 forward_status;     // the status of the call forwarding service
}MnCcForwardStatusIndS;
 

// indicate to MMI that the MO call is ongoing 
typedef struct
{

    uint8                    call_id;
	MN_CALLED_NUMBER_T          called_num;     // the called address
}MnCcCallStartIndS;

// indicate to local user the remote user has been alerting
typedef struct
{

	uint8                    call_id;  
    MN_CALL_ALERT_TYPE_E     alerting_type;  // indicate the alerting type i.e(inband tone or remote alert)
}MnCcCallAlertingIndS;
 

// indicate the call has been setupped completely
typedef struct
{
 
	uint8                   call_id;
	BOOLEAN                 connected_num_present;
	MN_CALLING_NUMBER_T     connected_num;      // the remote user's address
	BOOLEAN                 connected_sub_addr_present;
    MN_SUBADDR_T               connected_sub_addr;  // the remote user's subadress
} MnCcSetupCompleteCnfS;


typedef struct
{

	uint8                   call_id;
}MnCcSetupCompleteIndS;
 

// indicate the current call entity has been disconnected by network
typedef struct
{

    uint8                 		   call_id;
	MN_CALL_DISCONNECT_CALL_CAUSE_E           disconnected_cause;  
}MnCcCallDisconnectdIndS;


// indicate that there is error in the current call entity
typedef struct
{

    uint8                 call_id;
    MN_CALL_DISCONNECT_CALL_CAUSE_E		  error_cause;
//	GSM_CAUSE_T           error_cause;  // the indicate the cause
}MnCcCallErrIndS;
 

// indicate there is a incoming call
typedef struct
{

    uint8                    call_id;
	BOOLEAN                  calling_num_present; // indicate the calling_num is present or not
	MN_CALLING_NUMBER_T      calling_num;      // the remote user's address
    BOOLEAN                  calling_sub_addr_present;
	MN_SUBADDR_T             calling_sub_addr; // the remote user's subaddress
} MnCcSetupIndS;


// the network accept or reject the DTMF tone
typedef struct
{

	uint8                 call_id;
	MN_CALL_DTMF_KEY_T    dtmf_key;  // the dtmf key's value
	BOOLEAN               req_is_accepted; // indicate the request of START DTMF is accepted or rejected by network
}MnCcStartDtmfCnfS;
 

typedef struct
{

    uint8            call_id; // the call entity's ID
} MnCcStopDtmfReqS,    // local user stop transfer the DTMF tone
  MnCcStopDtmfCnfS,     // network has accept or reject the stopping DTMF
  MnCcCallHoldReqS,     // user hold the current active call or active MPTY
  MnCcCallHeldIndS,     // indicate to MMI task that the remote user has held the call entity
  MnCcCallRetrievedIndS, // indicate to MMI task that the remote user has retrieved the call entity
  MnCcBuildMPTYIndS,    // indicate that the remote user has added the call to MPTY
  MnCcCallRetrivedReqS, // user retrieved the held call entity
  MnCcSplitMptyREqS,    // user splitted the call from the MPTY 
  MnCcCallWaitIndS,     // indicate call has been waited by the remote user 
  MnCcCallHoldingIndS,  // inform clients the call is being holded
  MnCcCallRetrievingIndS, // inform clients that the call is being retrieved
  MnCcBuildingMptyIndS,
  MnCcSplitingMptyIndS;

// inform the MMI that the network has assigned TCH  
typedef struct
{

	MN_CALL_CHANNEL_MODE_E        channel_mode;  // the current channel mode
	MN_CALL_SYNC_CAUSE_E          sync_cause;    // indicate the cause of assigning resource
}MnCcSyncIndS;

// inform the network has rejected or accepted the HOLD request
typedef struct
{
	
    uint8                 call_id;
	BOOLEAN               req_is_accepted; // Indicate the request of holding call is accepted or rejected
}MnCcCallHeldCnfS;
 

// inform that the network has accepted or reject the RETRIEVE request 
typedef struct
{

    uint8                 call_id;
	BOOLEAN               req_is_accepted; // indicate the request of retrieving call is accepted or rejected
}MnCcCallRetrievedCnfS;
 

// inform that the remote user has alter the service type
typedef struct
{

    uint8                    call_id;
    MN_SERVICE_TYPE_E        service_type;          // The service type the user want to change to be
	BOOLEAN                  direction_is_reverse;  // This Parameter indicate that the user want to reverse the the call setup directive
}MnCcCallModifyIndS;
 

// inform the network accepted of reject the modification request
typedef struct
{

    uint8                  call_id;
	BOOLEAN                req_is_accepted;       // determint if the network has accepted the MODIFY request or not
    BOOLEAN                direction_is_reverse;  // TRUE: reverse the call's direction otherwise FALSE
}MnCcCallModifyCnfS;
 

// inform the MMI task that the incoming call or the outgoing call is forwarded
typedef struct
{

    uint8                       call_id;
	MN_SS_FORWARD_NOTIFY_E       forward_type;     // indicate the outgoing call or the incoming call is forwarded
}MnCcCallForwardIndS;
 

// indicate to MMI task that the outgoing call is barred
typedef struct
{
	
    uint8              call_id;
	MN_SS_CODE_E       barred_operation;  // indicat local user's outgoing call is barred or remote
	                                      // user's incoming call is barried
} MnCcCallbarredIndS;


// this signal inform MMI the result of BUILDING MPTY request
typedef struct
{

    uint8                 call_id;
	BOOLEAN               req_is_accepted; // indicate the network accept or
	                                       // reject the request of building MPTY
} MnCcBuildMptyCnfS;


// this signal inform the MMI task the result of SPLITTING MPTY
typedef struct
{

	uint8                 call_id;
	BOOLEAN               req_is_accepted; // indicate the request of splitting MPTY is accepted
	                                       // or rejected by network
}MnSplitMPTYCnfS;
 

// this signal is to indicate that the value of ACM is equal or great to the max value
typedef EmptySignalS MnCcAcmmaxIndS;

// inform the progress of current call in the interwork
typedef struct
{

	uint8                 	  call_id;
	MN_CALL_PROGRESS_T        progress_ind;  // indicate the progress of the call in the network
}MnCcProgressIndS;
 

// transfer the user-to-user information to local user
typedef struct
{

	uint8                 call_id;
    MN_CALL_UTU_T           user_to_user;  // user-to-user information
}MnCcUserToUserIndS;
 

// inform the clients that the call entity is been connecting
typedef struct
{

    uint8                 call_id;
}MnCcConnectingIndS;
 

// inform the clients that the call entity is been disconnecting
typedef struct
{

    uint8                 call_id;
}MnCcDisconnectingIndS;
 

/****************CB sigla list ****************/

typedef struct
{

	MN_CB_MSG_ID_T	msg_id;
	MN_SMSCB_DCS_T	dcs;
	uint16			msg_len;
	MN_CB_MSG_ARR_T	msg_arr;
} MnCbMsgIndS;


typedef struct
{

	MN_CB_MSG_ID_T			msg_id;
	MN_SMSCB_DCS_T			dcs;
	uint16					msg_len;
	MN_CB_MACRO_MSG_ARR_T	msg_arr;
}MnCbMarcoMsgIndS;
 

typedef EmptySignalS MnCbRejIndS;
//For charge in idle module
typedef EmptySignalS ChrCapIndS;
typedef EmptySignalS ChargeStartIndS;
typedef EmptySignalS ChargeEndIndS;
typedef EmptySignalS ChrWarningIndS;
typedef EmptySignalS ChrShutdownIndS;


//  the signal define 
/* signal handle */
typedef union 
{
	MmiTimerExpiryS               timerExpiry;

	// mmi Signal 
	MmiKeyPress              keyPress;


	/***************mmi Signal list ********************/
/*****************************The signal structure****************************/

/*
the signal indicate the mnsms module is ok!
*/
        MnSmsReadyIndS    MnssmReadInd;

/*
the signal indicate the memory used to save short message is full
*/
		MnSmsMemFullIndS  MnSmsMemFullInd;

/*
 the load sms confirm structure , respond to the MNSMS_load_sms()
*/
        MnReadSmsCnfS    MnReadSmsCnf;

 /*the update sms confirm structure , respond to the MNSMS_update_sms() */

	 MnUpdateSmsStateCnfS MnUpdateSmsStateCnf;

/*the read sms status report confirm structure , respond to the MNSMS_Read_sms_Status_report() */
	 MnReadSmsStatusReportCnfS MnReadSmsStatusReportCnf;

/*
 the send sms confirm structure , it respond the MNSMS_send_sms () or MNSMS_send_command_sms()
*/
		 MnSendSmsCnfS  MnSendSmsCnf;

/*
 the save sms confirm structure , it respond the MNSMS_save_mo_sms() or MNSMS_save_mt_sms() 
*/
 MnWriteSmsCnfS     MnWriteSmsCnf;

/*
 the sms indication structure , it indicate a mt sms to the mmi
*/
         MnSmsMtIndS           MnSmsMtInd;
/*
 the state report indication structure, it indicate a status report to mmi
*/
		MnSmsReportIndS    MnSmsReportInd;

/*
 the load parameter confirm structure , it respond the MNSMS_load_param()
*/
 MnReadSmsParamCnfS   MnReadSmsParamCnf;

/*
 the update the paramter confirm sturcure , it respond the MNSMS_update_param()
*/
 MnWriteSmsParamCnfS    MnWriteSmsParamCnf;

/*
	the result of delete all short message
*/
 MnDeleteAllSmsCnfS MnDeleteAllSmsCnf;
/*
	Indicate reading sms states
*/
 MnReadingSmsStateIndS MnReadingSmsStateInd;

/*
	Indicate reding sms
*/
	MnReadingSmsIndS   MnReadingSmsInd;

/*
	Indicate updating sms state
*/
 MnUpdatingSmsStateIndS   MnUpdatingSmsStateInd;

/*
	Indicate sending sms
*/
 MnSendingSmsIndS   MnSendingSmsInd;

/*
	Indicate saving sms 
*/
 mnSavingSmsIndS   mnSavingSmsInd;

/*
	Indicate reading sms param;
*/
  MnReadingSmsParamIndS  MnReadingSmsParamInd;

/*
	Indicate writing sms param
*/
 MnWritingSmsIndS   MnWritingSmsInd;


 /*****************************************************************************
 **                        SS MODULE DEFINE                                   *
 ******************************************************************************/

/* define signals between App-layer with MN-layer */
	MnSsBeginReisterS MnSsBeginReister;
    MnSsBeginEraseS   MnSsBeginErase;
	MnSsBeginActiveS   MnSsBeginActive;
	MnSsBeginDeactiveS  MnSsBeginDeactive;
	MnSsBeginInterrogateS MnSsBeginInterrogate;
	MnSsBeginRegisterPasswordS  MnSsBeginRegisterPassword;
	MnSsBussyingIndS    MnSsBussyingInd;
	

	MnSsRegisterCnfS  MnSsRegisterCnf;
	MnSsEraseCnfS   MnSsEraseCnf;
	MnSsActivateCnfS MnSsActivateCnf;
	MnSsDeactivateCnfS  MnSsDeactivateCnf;
	

	MnSsInterrogateCnfS  MnSsInterrogateCnf;

	MnSsRegisterPasswordS  MnSsRegisterPassword;


	MnSsGetPasswordIndS   MnSsGetPasswordInd;
	

	MnSsUssdIndS   MnSsUssdInd;
	MnSsUssdNotifyIndS  MnSsUssdNotifyInd;
	MnSsBegingUssdServiceS MnSsBegingUssdService;
	

	MnSsUssdServiceCnfS   	MnSsUssdServiceCnf;



	MnSsReleaseIndS   MnSsReleaseInd;
		// this signal used network release ss without ss operation


 /*****************************************************************************
 **                        PHONE MODULE DEFINE                                *
 ******************************************************************************/
 




	MnPowerOffIndS  MnPowerOffInd;
//  struct of signal: APP_MN_MS_POWER_OFF_CNF_T
//  this signal indicate MMI that power off complete
	MnPowerOffCnfS  MnPowerOffCnf;

//  struct of signal: APP_MN_PS_READY_IND_T
//  this signal indicate MMI that MS waitting for MMI's command now
	MnPSReadyIndS  MnPSReadyInd;

//  struct of signal: APP_MN_AOC_INCREASE_ACM_IND
//  this signal indicate phone status to MMI 
	MnAocModifyIndS  MnAocModifyInd;

//  struct of signal: APP_MN_FDN_SERVICE_IND
//  this signal indicate fdn enable/disable to MMI 
	MnFdnServiceIndS  MnFdnServiceInd;


//  struct of signal: APP_MN_NETWORK_STATUS_IND
//  this signal indicate phone status to MMI 
	 MnNetworkStatusIndS  MnNetworkStatusInd;
 

//  struct of signal: APP_MN_PLMN_LIST_CNF_T
//  this signal indicate plmns's detail to MMI
	MnPlmnListCnfS  MnPlmnListCnf;
 

//  struct of signal: APP_MN_PLMN_SELECT_CNF_T
//  this signal indicate MMI that PLMN select complete
	MnPlmnSelectCnfS   MnPlmnSelectCnf;


//  struct of signal: APP_MN_SIM_GET_PIN_IND_T
//  this signal indicate MMI to input PIN1 value
	MnSimGetPinIndS  MnSimGetPinInd;


//  struct of signal: APP_MN_SIM_READY_IND_T
//  this signal indicate MMI that SIM card init OK
	MnSimReadyIndS  MnSimReadyInd;
 

//  struct of signal: APP_MN_SIM_NOT_READY_IND_T
//  this signal indicate MMI that SIM not ready
	MnSimNotReadyIndS  MnSimNotReadyInd;
 

//  struct of signal: APP_MN_SIM_PIN_FUNC_CNF_T
//  this signal indicate MMI that operation on PIN compete
	MnSimPinFuncCnfS  MnSimPinFuncCnf;


//  struct of signal: APP_MN_GPRS_ATTACH_CNF_T
//  this signal indicate MMI that operation on PIN compete
	MnGprsAttachResultS  MnGprsAttachResult;
 
//  struct of signal: APP_MN_GPRS_DETACH_CNF_T
//  this signal indicate MMI that operation on PIN compete
	MnGprsDetachResultS  MnGprsDetachResult;
 

//  struct of signal: APP_MN_SCELL_RSSI_IND_T
//  this signal indicate MMI service cell power level
	MnScellRssiIndS  MnScellRssiInd;
 

//  struct of signal: APP_MN_NCELL_RSSI_CNF_T
//  this signal indicate MMI neighbour cell power level
	MnNcellRssiCnfS  MnNcellRssiCnf;
 

//  struct of signal: APP_MN_PUCT_INFO_CNF_T
//  this signal indicate puct info
	MnPuctInfoCnfS  MnPuctInfoCnf;


 /*****************************************************************************
 **                        SIM MODULE DEFINE                                *
 ******************************************************************************/
//  struct of signal: APP_MN_MSISDN_UPDATE_CNF_T
//  this signal is cnf msisdn update or delete or add
    MnMsisdnUpdateCnfS MnMsisdnUpdateCnf;
 
//  struct of signal: APP_MN_ADN_UPDATE_CNF_T
//  this signal is cnf adn update or delete or add
    MnAdnUpdateCnfS MnAdnUpdateCnf;
 
//  struct of signal: APP_MN_FDN_UPDATE_CNF_T
//  this signal is cnf fdn update or delete or add
    MnFdnUpdateCnfS MnFdnUpdateCnf;
 

 /*****************************************************************************
 **                        CALL MODULE DEFINE                                *
 ******************************************************************************/
    MnCallReadyIndS      mnCallReadyInd;

	MnCcForwardStatusIndS MnCcForwardStatusInd;
 

	MnCcCallStartIndS  MnCcCallStartInd;

// indicate to local user the remote user has been alerting
	MnCcCallAlertingIndS MnCcCallAlertingInd;
 

	MnCcSetupCompleteCnfS MnCcSetupCompleteCnf;


	MnCcSetupCompleteIndS MnCcSetupCompleteInd;
 

// indicate the current call entity has been disconnected by network
	MnCcCallDisconnectdIndS  MnCcCallDisconnectdInd;

// indicate that there is error in the current call entity
	MnCcCallErrIndS  MnCcCallErrInd;
 

// indicate there is a incoming call
	MnCcSetupIndS  MnCcSetupInd;


// the network accept or reject the DTMF tone
	MnCcStartDtmfCnfS  MnCcStartDtmfCnf;
 
  MnCcStopDtmfReqS  MnCcStopDtmfReq;    // local user stop transfer the DTMF tone
  MnCcStopDtmfCnfS  MnCcStopDtmfCnf;     // network has accept or reject the stopping DTMF
  MnCcCallHoldReqS MnCcCallHoldReq;     // user hold the current active call or active MPTY
  MnCcCallHeldIndS MnCcCallHeldInd;     // indicate to MMI task that the remote user has held the call entity
  MnCcCallRetrievedIndS MnCcCallRetrievedInd; // indicate to MMI task that the remote user has retrieved the call entity
  MnCcBuildMPTYIndS  MnCcBuildMPTYInd;    // indicate that the remote user has added the call to MPTY
  MnCcCallRetrivedReqS MnCcCallRetrivedReq; // user retrieved the held call entity
  MnCcSplitMptyREqS  MnCcSplitMptyREq;    // user splitted the call from the MPTY 
  MnCcCallWaitIndS  MnCcCallWaitInd;     // indicate call has been waited by the remote user 
  MnCcCallHoldingIndS  MnCcCallHoldingInd;  // inform clients the call is being holded
  MnCcCallRetrievingIndS  MnCcCallRetrievingInd; // inform clients that the call is being retrieved
  MnCcBuildingMptyIndS  MnCcBuildingMptyInd;
  MnCcSplitingMptyIndS  MnCcSplitingMptyInd;

// inform the MMI that the network has assigned TCH  
	MnCcSyncIndS   MnCcSyncInd;

// inform the network has rejected or accepted the HOLD request
	MnCcCallHeldCnfS MnCcCallHeldCnf;
 
// inform that the network has accepted or reject the RETRIEVE request 
	MnCcCallRetrievedCnfS MnCcCallRetrievedCnf;
 

	MnCcCallModifyIndS  MnCcCallModifyInd;
 

	MnCcCallModifyCnfS  MnCcCallModifyCnf;
 

	MnCcCallForwardIndS  MnCcCallForwardInd;
 

// indicate to MMI task that the outgoing call is barred
	MnCcCallbarredIndS   MnCcCallbarredInd;


// this signal inform MMI the result of BUILDING MPTY request
	 MnCcBuildMptyCnfS   MnCcBuildMptyCnf;


// this signal inform the MMI task the result of SPLITTING MPTY
	MnSplitMPTYCnfS  MnSplitMPTYCnf;
 

// this signal is to indicate that the value of ACM is equal or great to the max value
    MnCcAcmmaxIndS  MnCcAcmmaxInd;

// inform the progress of current call in the interwork
	MnCcProgressIndS  MnCcProgressInd;
 
// transfer the user-to-user information to local user
	MnCcUserToUserIndS MnCcUserToUserInd;
 
// inform the clients that the call entity is been connecting
	MnCcConnectingIndS  MnCcConnectingInd;
 

// inform the clients that the call entity is been disconnecting
	MnCcDisconnectingIndS  MnCcDisconnectingInd;
 

/****************CB sigla list ****************/

	 MnCbMsgIndS    MnCbMsgInd;


	MnCbMarcoMsgIndS MnCbMarcoMsgInd;
 

   MnCbRejIndS   MnCbRejIndS;

	// add by allen 2004.09.28
	MMI_L1_MESSAGE L1MnGetDataCnf;
	
}Signal;


#endif
