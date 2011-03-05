#ifndef _SIMAT_CALLBACK_H_
#define _SIMAT_CALLBACK_H_
/**************************************************************************

  Copyright (C) 2001 SPREADTRUM COMMUNICATION CO.
  All rights reserved.

  This document contains proprietary information blonging to 
  SPREADTRUM. Passing on and copying this document, use and 
  communication of its contents is not permitted without prior
  writed authorization.

***************************************************************************/

/***************************************************************************

  Reversion information:
  File Name:    simat_callback.h
  Version:      0.1
  Author:       Raistlin.kong
  Date:         2003.1
  Description:  you can define your own message structure that to be used in
				your owner implemetation of the SIM application toolkit,and also
				used in the call back function of SIMAT.
***************************************************************************/
/***************************************************************************

                Edit Histroy
  Date          Name            Description
  --------------------------------------------------------------------------
  2003.1        Raistlin.kong    Create

***************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif
//the DISPLAY TEXT proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_DISPLAY_TEXT_T com;   //the  proactive command
}APP_MN_SIMAT_DISPLAY_TEXT_IND_T; 

//the GET INKEY proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_GET_INKEY_T com;  //the proactive command
}APP_MN_SIMAT_GET_INKEY_IND_T; 

//the GET INPUT proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_GET_INPUT_T com;  //the proactive command
}APP_MN_SIMAT_GET_INPUT_IND_T; 

//the MORE TIME proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    //SIMAT_MORE_TIME_T com;  //the proactive command
}APP_MN_SIMAT_MORE_TIME_IND_T; 


//the PLAY TONE proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_PLAY_TONE_T com;  //the proactive command
}APP_MN_SIMAT_PLAY_TONE_IND_T;  

typedef struct
{
    SIGNAL_VARS
    SIMAT_REFRESH_T com;
}APP_MN_SIMAT_REFRESH_IND_T;   

//the SETUP MENU proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_SETUP_MENU_T com; //the proactive command
}APP_MN_SIMAT_SETUP_MENU_IND_T; 

//the SELECT ITEM proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_SELECT_ITEM_T com;  //the proactive command
}APP_MN_SIMAT_SELECT_ITEM_IND_T; 

//the SEND SMS proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_SEND_SM_T    com;    //the proactive command
}APP_MN_SIMAT_SEND_SMS_IND_T; 

//the SEND SS proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_SEND_SS_T com;    //the proactive command
}APP_MN_SIMAT_SEND_SS_IND_T;

//the SEND USSD proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_SEND_USSD_T   com;    //the proactive command
}APP_MN_SIMAT_SEND_USSD_IND_T;

//the SET UP CALL proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_SETUP_CALL_T  com;    //the proactive command
}APP_MN_SIMAT_SETUP_CALL_IND_T;

typedef struct
{
    SIGNAL_VARS
    SIMAT_LOCAL_INFO_TYPE_E com;
}APP_MN_SIMAT_PRO_LOCAL_INFO_IND_T;

//the SET UP EVENT LIST proactive service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_SETUP_EVENT_LIST_T com;
}SIMAT_SIG_SETUP_EVENT_LIST_REQ_T;


//the SET IDLE MODE TEXT proactive command service request signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_SETUP_IDLE_TEXT_T com;  //the proactive command
}APP_MN_SIMAT_SETUP_IDLE_TEXT_IND_T;

//the SEND DTMF proactive command service request signal
typedef struct
{
    SIGNAL_VARS

    SIMAT_SEND_DTMF_T   com;    //the proactive command
}APP_MN_SIMAT_SEND_DTMF_IND_T;

//the LANGUAGE NOTIFY proactive command service indicator signal(the proactive do not need return value)
typedef struct
{
    SIGNAL_VARS
    SIMAT_LANG_NOTIFICATION_T com;    //the proactive command
}APP_MN_SIMAT_LANG_NOTIFY_IND_T;

//thye call control by SIM confirm signal


typedef struct
{
    SIGNAL_VARS
    SIMAT_CC_TO_MMI_IND_T  com;  
}APP_MN_SIMAT_CC_TO_MMI_IND_T;

//the MO SS control by SIM confirm signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_SS_CONTROL_RSP_T ss_rsp;
}SIMAT_SIG_MO_SS_CONTROL_RSP_T;

//the MO USSD control by SIM confirm signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_USSD_CONTROL_RSP_T ussd_rsp;
}SIMAT_SIG_MO_USSD_CONTROL_RSP_T;


//the MO Short message controled by SIM confirm signal
typedef struct
{
    SIGNAL_VARS
    SIMAT_SMS_CTL_TO_MMI_IND_T com;
}APP_MN_SIMAT_SMS_CTL_TO_MMI_IND_T;


//the SMS-PP data download confirm signal from the SMS layer
typedef struct
{
    SIGNAL_VARS
    SIMAT_SMS_PP_RSP_T pp_rsp;
}SIMAT_SIG_SMS_PP_RSP_T;

typedef struct
{
    SIGNAL_VARS
    SIMAT_SMS_CB_RES is_dl_success;  //is the CB sms download success or not
}SIMAT_SIG_SMS_CB_RSP_T;




#ifdef __cplusplus
	}
#endif
#endif