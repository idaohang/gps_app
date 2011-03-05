/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_Phone.c
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-7-23
  内容描述：  
  修改记录：
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SG_GPS.h"
#include "SG_Send.h"
#include "SG_Receive.h"
#include "mn_events.h"
#include "sci_types.h"
#include "os_api.h"

#include "mn_type.h"
#include "simat_data_object.h"
#include "simat_proactive.h"
#include "simat_api.h"
#include "simat_callback.h"
#include "sim_dphone_stk_verify.h"
#include "mmi_signal_ext.h"
#include "env_app_signal.h"
#include "string.h"
#include "SG_GPS.h"
#include "app_tcp_if.h"
#include "SG_Net.h"
#include "SG_Call.h"
#include "SG_Phone.h"
#include "SG_Timer.h"
#include "SG_Hh.h"


/*****************************************************************************/
// 	Description : get the network name
//	Global resource dependence : 
//  Author: Great.Tian
//	Note:
/*****************************************************************************/
void GetNetWorkNameId(
                                     uint16                  mcc,
                                     uint16                  mnc,
                                     MN_PHONE_PLMN_STATUS_E  plmn_status,
                                     BOOLEAN                 is_slecting
                                     )
{
	xSignalHeaderRec      *signal_ptr = NULL;
	char sig[2] = "";

	SCI_TRACE_LOW("-----GetNetWorkNameId %d %d %d %d",is_slecting,plmn_status,mcc,mnc);

	if(mcc == 460){
		if(mnc == 0){
			g_state_info.opter = 46000;// 移动
		}
		else if(mnc == 1){
			g_state_info.opter = 46001; // 联通
		}
#if (0)
		else if(mnc == 2){
			g_state_info.opter = 46000;
		}
#endif /* (0) */
		else
			g_state_info.opter = 0;
	}
	else				
		g_state_info.opter = 0;

	g_state_info.plmn_status = plmn_status;
	//非GSM和GPRS都可用的状态时都自动搜索网络
	if(g_state_info.plmn_status != PLMN_NORMAL_GSM_GPRS_BOTH && NetSearchTimer == 0xffffffff)
	{
#if (0)
		sig[0] = ERR_NO_NETWORK;
		SG_Send_Net_Err(sig[0]);	
#endif /* (0) */
		NetSearchTimer = GetTimeOut(180);
		
	}
	else
	{
		NetSearchTimer = 0xffffffff;
		NetSearchCount = 0;
	}
    return ;
}


/*****************************************************************************/
// 	Description : deal with signal of APP_MN_SCELL_RSSI_IND
//	Global resource dependence : 
//  Author: Jassmine
//	Note:
/*****************************************************************************/
void HandleScellRssiInd(
                                      DPARAM              param
                                      )
{
    MN_PHONE_SCELL_MEAS_T     srri_ind;// = *(APP_MN_SCELL_RSSI_IND_T *)param;
	xSignalHeaderRec      *signal_ptr = NULL;

	static int rxlevnum = 0;

	srri_ind = MNPHONE_GetRSSI();
	g_state_info.rxLevel = srri_ind.rxlev;
	SCI_TRACE_LOW("<<<<<<EV_MN_APP_SCELL_RSSI_IND_F rx %d opter %d fn %d bs %d lac %d id %d<<<<<<",srri_ind.rxlev,g_state_info.opter,srri_ind.arfcn,srri_ind.bsic,srri_ind.lac,srri_ind.cell_id);

  //  //1：1-6； 2：7-10； 3：11-35； 4：36-63
    if (36 <= g_state_info.rxLevel)
    {
        g_state_info.rxLevelnum = 4;
    }
    else if (11 <= g_state_info.rxLevel)
    {
        g_state_info.rxLevelnum = 3;
    }
    else if (7 <= g_state_info.rxLevel)
    {
        g_state_info.rxLevelnum = 2;
    }
    else if (1 <= g_state_info.rxLevel)
    {
        g_state_info.rxLevelnum = 1;
    }
    else
    {
       g_state_info.rxLevelnum = 0;
    }

	if(rxlevnum != g_state_info.rxLevelnum)
	{
		SG_Send_OptrRexlev(1);
	}

	rxlevnum = g_state_info.rxLevelnum;

	return ;
    
}


void MnPhone_EventCallback(
                                uint32 task_id, //task ID
                                uint32 event_id, //event ID
                                void *param_ptr
                                )
{
	APP_MN_SIM_READY_IND_T             sim_ready_ind;  //used to get sim imsi for lock process
	xSignalHeaderRec      *signal_ptr = NULL;

	SCI_ASSERT(PNULL != param_ptr);
	if( PNULL == param_ptr )
	{
		SCI_TRACE_HIGH("<<<<<<MnPhone_EventCallback: the ponit of param is empty");
		return;
	}
	switch(event_id)
	{
	/*
	EV_MN_APP_NETWORK_STATUS_IND_F 
	EV_MN_APP_SIMCARD_READY_IND_F, 
	EV_MN_APP_SIMCARD_NOT_READY_IND_F, 
	EV_MN_APP_SIMCARD_GET_PIN_IND_F, 
	EV_MN_APP_SCELL_RSSI_IND_F, 
 	EV_MN_APP_MS_POWERING_OFF_IND_F, 
	EV_MN_APP_DEACTIVE_PS_CNF_F, 
	EV_MN_APP_NCELL_RSSI_CNF_F,	
   	EV_MN_APP_AOC_MODIFIED_IND_F, 
	EV_MN_APP_FDN_SERVICE_IND_F, 
 	EV_MN_APP_PIN_FUNC_CNF_F,	
 	EV_MN_APP_PLMN_LIST_CNF_F,	
   	EV_MN_APP_PLMN_SELECT_CNF_F, 
 	EV_MN_APP_GPRS_ATTACH_RESULT_F,
	EV_MN_APP_GPRS_DETACH_RESULT_F,
  	EV_MN_APP_DETACH_GSM_CNF_F,
  	EV_MN_APP_PUCT_INFO_CNF_F,
	EV_MN_APP_PS_READY_IND_F,
	EV_MN_APP_SET_PREF_MODE_CNF_F,
	EV_MN_APP_SET_BAND_CNF_F,
	EV_MN_APP_SET_SERVICE_TYPE_CNF_F,
	EV_MN_APP_SET_ROAM_MODE_CNF_F,
	MAX_MN_APP_PHONE_EVENTS_NUM, 
	*/
	case EV_MN_APP_PS_READY_IND_F:
		SCI_TRACE_LOW("<<<<<<EV_MN_APP_PS_READY_IND_F<<<<<<");
		break;
		
	case EV_MN_APP_NETWORK_STATUS_IND_F:
        {
			APP_MN_NETWORK_STATUS_IND_T    network_status_ind;               //used to lock operator lac cell
	        network_status_ind = *((APP_MN_NETWORK_STATUS_IND_T *)param_ptr);
//	        SCI_TRACE_LOW("<<<<<<EV_MN_APP_NETWORK_STATUS_IND_F %p %d %d %d %d<<<<<<",param_ptr,((APP_MN_NETWORK_STATUS_IND_T *)param_ptr)->plmn_is_selecting,((APP_MN_NETWORK_STATUS_IND_T *)param_ptr)->mcc,((APP_MN_NETWORK_STATUS_IND_T *)param_ptr)->mnc,((APP_MN_NETWORK_STATUS_IND_T *)param_ptr)->plmn_status);
			GetNetWorkNameId(network_status_ind.mcc,network_status_ind.mnc,network_status_ind.plmn_status,network_status_ind.plmn_is_selecting);
			SG_Send_OptrRexlev(1);

	}
        break;

    case EV_MN_APP_PLMN_LIST_CNF_F:
        SCI_TRACE_LOW("<<<<<<EV_MN_APP_PLMN_LIST_CNF_F<<<<<<");
        //该消息用于通知用户MNPHONE_ListPLMN()的操作结果。
        // 结构中包含存在网络的列表和每个网络的具体信息。
        break;

    case EV_MN_APP_PLMN_SELECT_CNF_F:
        SCI_TRACE_LOW("<<<<<< APP_MN_PLMN_SELECT_CNF<<<<<<");
        // 该消息用于通知用户MNPHONE_SelectPLMN()的操作结果。
        // 结构中包含选择操作完成后当前的网络状况。
        {
			APP_MN_PLMN_SELECT_CNF_T        plmn_select_cnf = *((APP_MN_PLMN_SELECT_CNF_T *)param_ptr);
			GetNetWorkNameId(plmn_select_cnf.mcc,plmn_select_cnf.mnc,plmn_select_cnf.plmn_status,plmn_select_cnf.plmn_is_selected);
			SCI_TRACE_LOW("<<<<<< select PLMN result %d<<<",plmn_select_cnf.plmn_status);
		}
		break;

    case EV_MN_APP_SIMCARD_GET_PIN_IND_F:
        SCI_TRACE_LOW("<<<<<< APP_MN_SIM_GET_PIN_IND<<<<<<");
        //wenming feng prd1542 Jan/19/2005 lock sim and phone process modifyed
        // 该消息用于通知用户要求输入PIN1或者PUK1，并可获得PIN1是否已经被锁。
    
        break;

    case APP_MN_SIM_PIN_FUNC_CNF:
   	 	SCI_TRACE_LOW("<<<<<< APP_MN_SIM_PIN_FUNC_CNF<<<<<<");
         //wenming feng prd1542 Lock sim and phone process Jan/19/2005 
         //In this EV the lock process is divided into two cases  unlock simcard or lock simcard 
         //which is packed in func MMI_EvSimPinFuncCnfProcessLockAndUnlockSim
	    //MMI_InitLockParam(&g_mmi_lock_info);//获得锁机卡相关初始参数
        //begin prd2359
         {
		 	APP_MN_SIM_PIN_FUNC_CNF_T  pin_result = *((APP_MN_SIM_PIN_FUNC_CNF_T *)param_ptr);
			SCI_TRACE_LOW("<<<<<< PIN RESULT %d",pin_result.pin_operate_result);
         }
	   
      
        break;

    case EV_MN_APP_SIMCARD_READY_IND_F:
        // 该消息用于汇报SIM卡准备就绪，同时会附带PIN的状况，HPLMN 等SIM卡的信息。
        //wenming feng prd1542 lock sim and phone process Jan/19/2005
#if (0)
	    sim_ready_ind = *((APP_MN_SIM_READY_IND_T *)param_ptr);
		SCI_MEMCPY(g_state_info.imsi,sim_ready_ind.imsi.imsi_val,sim_ready_ind.imsi.imsi_len);
#endif /* (0) */
		g_state_info.SimFlag = 1;

        SCI_TRACE_LOW("=======EV_MN_APP_SIMCARD_READY_IND_F IMSI =======");
        break;
        
    case EV_MN_APP_SIMCARD_NOT_READY_IND_F:
        SCI_TRACE_LOW("<<<<<< EV_MN_APP_SIMCARD_NOT_READY_IND_F<<<<<<");
        // 该消息用于汇报SIM卡未能成功初始化或是SIMCARD被拔出。从该消息中可获得具体的原因，
        // 并显示在界面上。
		g_state_info.SimFlag = 0;
		{
			char sig[2];
			char net_status = 0;

			net_status = SG_Net_Judge();
			if(net_status == NO_SIM_CARD)
			{
				sig[0] = ERR_NO_SIM_CARD;
				SG_Send_Net_Err(sig[0]);	
			}			
        }
        break;
        
    case EV_MN_APP_SCELL_RSSI_IND_F:
        //SCI_TRACE_LOW("HandlePsAndRefMsg:receive APP_MN_SCELL_RSSI_IND");
        // 该消息用于在服务小区信号强度改变时，汇报当前服务小区信号强度。
        // 例：在idle界面显示。
		//prd2325 begin 
        HandleScellRssiInd(param_ptr); 		

		//@_@ dongjz post sig req handset updateing rssi 
        break;
        
    case EV_MN_APP_DEACTIVE_PS_CNF_F:
        //SCI_TRACE_LOW("HandlePsAndRefMsg:receive APP_MN_DEACTIVE_PS_CNF");
        // 该消息用于通知用户关机请求已执行完毕需要保存的数据也已经写到SIM卡中。
        // 可参考MNPHONE_PowerOff()。
		SCI_TRACE_LOW("<<<<<<EV_MN_APP_DEACTIVE_PS_CNF_F<<<<<<");
		//POWER_PowerOff();
		//wenming feng prd2230 begin
		
        break;


    case MISC_MSG_RTC_ALARM:
        SCI_TRACE_LOW("mmi: MISC_MSG_RTC_ALARM");
        // 该消息用于汇报闹钟所设定的时间到，开始闹铃。
        // 注：设备关机时，启动闹铃的方式与该消息无关

        break;
	default:
		SCI_TRACE_LOW("MnPhone_EventCallback:event Id is %d", event_id);
		break;
	}
}


//获取IMEI号
void SG_Get_IMEI()
{
	 MN_IMEI_T imei;	
	 
	SCI_MEMSET(g_state_info.IMEI, 0, 20);	
	SCI_MEMSET(imei, 0, 8);	
	
	if(MNNV_GetIMEI(imei))	
	{		
		MMI_BcdToStr(PACKED_LSB_FIRST,imei,MN_MAX_MOBILE_ID_LENGTH<<1,(char*)g_state_info.IMEI);		
		SCI_MEMCPY(g_state_info.IMEI, &g_state_info.IMEI[1], 15);		
		g_state_info.IMEI[15]=0;	
	}	
	SCI_TRACE_LOW("====imei:%s",g_state_info.IMEI);






}

