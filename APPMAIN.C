/****************************************************************************
** File Name:      MMIMAIN.C                                               *
** Author:                                                                 *
** Date:           03/11/2003                                              *
** Copyright:      2003 Spreatrum, Incoporated. All Rights Reserved.       *
** Description:    This file is used to describe the MMI Kernel            *
****************************************************************************
**                         Important Edit History                          *
** ------------------------------------------------------------------------*
** DATE           NAME             DESCRIPTION                             *
** 11/2003       Louis.wei         modify
** 12/2003       Tracy Zhang       �������ʼ���ͶԷ����ע������޸�      *
****************************************************************************/
#define APPMAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sci_types.h"
#include "os_api.h"
#include "Nvitem.h"

#include "mn_type.h"
#include "env_app_signal.h"
#include "SG_Receive.h"
#include "SG_GPS.h"
#include "app_tcp_if.h"
#include "SG_Net.h"
#include "SG_Call.h"
#include "SG_Phone.h"
#include "SG_Timer.h"
#include "SG_Set.h"
#include "SG_Sms.h"
#include "SG_Send.h"
#include "SG_Hh.h"
#include "Msg.h"
#include "ref_engineering.h"
#include "dal_power.h"
#include "sio.h"
#include "SG_MsgHandle.h"
#include "SG_Camera.h"
#include "SG_DynamicMenu.h"
#include "tb_hal.h"
#include "SG_Ext_Dev.h"
#include "mmi_descontrol.h"

SCI_TIMER_PTR g_RevTmr_ptr;
extern SG_CAMERA CurrCamera;
extern unsigned char*Camera_Buf;
extern int Camera_BufCnt;
extern int bfirst;

int smshhlen;
char smshhbuf[200];
int SMSCheckTime = 0xffffffff;
extern uint32 CallBackTime;

/*-------------------------------------------------------------------------*/
/*                         TYPES/CONSTANTS                                 */
/*-------------------------------------------------------------------------*/




/*****************************************************************************/
//  Description : register the MN signal and callback function
//  Global resource dependence :
//
//  Author:
//  Note: ע��app�ص�����
/*****************************************************************************/
LOCAL void APP_RegisterPsService( void )
{
	//Register phone event
    SCI_RegisterMsg( MN_APP_PHONE_SERVICE,
        EV_MN_APP_NETWORK_STATUS_IND_F,
        MAX_MN_APP_PHONE_EVENTS_NUM - 1,
        MnPhone_EventCallback);

    //Register call event
    SCI_RegisterMsg( MN_APP_CALL_SERVICE,
        EV_MN_APP_CALL_START_IND_F,
        MAX_MN_APP_CALL_EVENTS_NUM - 1,
        MnCall_EventCallback);

	//Register sms event
    SCI_RegisterMsg( MN_APP_SMS_SERVICE,
        EV_MN_APP_SMS_READY_IND_F,
        MAX_MN_APP_SMS_EVENTS_NUM - 1,
        MnSms_EventCallback);

	//Register gprs event
    SCI_RegisterMsg(MN_APP_GPRS_SERVICE,
		   EV_MN_APP_SET_PDP_CONTEXT_CNF_F,
		   MAX_MN_APP_GPRS_EVENTS_NUM - 1,
		   MnGPRS_EventCallback );

	SCI_Sleep(100);
}




void SG_HandleSignal(xSignalHeaderRec *receiveSignal)
{
	static int testtimer=0;
	xSignalHeaderRec      *signal_ptr = NULL;
	static int EmergencyOnlyTime = 0;

	if(receiveSignal == NULL)
		return ;
	if(receiveSignal->SignalCode != SG_TIME_OUT){
		SCI_TRACE_LOW("<<<<REV OTHERS receiveSignal->SignalCode %x",receiveSignal->SignalCode);
		return ;
	}
	SCI_TRACE_LOW("***:%d",testtimer++);

	GPS_Timer ++;   // GPS ϵͳʱ��������¼

 	camera_timout_check();

	//�ֱ����ݷ��ͳ�ʱ�ж�
	SG_HH_Send_To_Handle();

//���ż��ʹ��
	if(g_state_info.SendSMSFlag == 1)
	{
		SG_SendSms_Flag();
	}
//�����ط�
	if(g_sms_save.flag > 0)
	{
		if(g_set_info.SmsAlarmFlag == 1)
		{
			SG_AlarmSaveSms_Send();
		}
		else
		{
			SG_SaveSms_Send();
		}
	}

//Ϩ��״̬
	if(g_set_info.JDQ == 1 && SecTimer == 2)
	{
		JDQ_Init();
	}


//������Ž�����ʱ����
#if (1)
	if(g_set_info.nNetType == 0)
	{

		if((SecTimer%70 == 0) && (g_state_info.SendSMSFlag == 0) && (g_gprs_t.state != GPRS_DEACTIVE_PDP))
		{
			SG_CreateSignal(SG_SMS_CHECK, 0, &signal_ptr);
		}
	}
#endif /* (0) */

//����Ϩ��
#ifdef _TRACK
	if(g_set_info.ddjdqFlag == 1)
	{
		DDjdq_Handle();
	}
#endif


//��������쳣
#if(1)
	{
		//Լ5Сʱ�����Ͽ�
		if(SecTimer%18633 == 0)
		{
			SG_Net_Disconnect();
		}

		//����״̬Ϊ4,��2����ϵͳ����
		if((g_state_info.plmn_status == PLMN_EMERGENCY_ONLY) && (g_gprs_t.state == GPRS_RXHEAD || g_gprs_t.state == GPRS_RXCONTENT))
		{
			EmergencyOnlyTime++;
			if(EmergencyOnlyTime == 120)
			{
				SG_Soft_Reset(3);
			}
		}
		else
		{
			EmergencyOnlyTime = 0;
		}
	}
#endif

	if(CheckTimeOut(SMSCheckTime))
	{
		SG_Sms_HH(smshhbuf);
		SMSCheckTime = 0xffffffff;
	}

	//��������
	SG_Send_Heart_Beat();

	//�����绰Ӧ���ж�
	SG_Wait_ListenCall();

	//GPS�������л�
	Gps_Auto_Adapt();


	// ����ϵͳ
	if(ResetTimer != 0)
	{
		if(CheckTimeOut(ResetTimer))
		{
			if(g_state_info.user_type == PRIVATE_CAR_VER)
			{
				Gpio_Disable_Int();
			}
			POWER_Reset();

		}
	}

	// ������ϯ�绰
	if(CallBackTime != 0)
	{
		if(CheckTimeOut(CallBackTime))
		{
		   CallBackTime = 0;
	       SG_Call_Back_To();
		}
	}
	
#if (0)
	#ifdef _DUART
	Pro_Time_Handle(); // �ع�����
	#endif
#endif /* (0) */

	WDT_Timer++;   // ι��
	if(WDT_Timer>=20)
	{
		static BOOLEAN cpu_wdt = 0;
		cpu_wdt = !cpu_wdt;
		GPIO_SetValue(CPU_WDT,cpu_wdt);
		WDT_Timer  = 0;
	}

	SG_Detect_Alarm();

	#if(!__camera_debug_ && !__gps_debug_)
	SG_Gps_Err_Handle();
	#endif

	Camera_Watch_By_Time();

	SG_CorrectTime_Handle();

	//����������
	SG_LED_Hand();

	SG_Watch_Send();

	SG_Net_Err_Handle();

	SG_GpsDev_Handle();

	Camera_Up_Pic_Judge();

#if (0)
	 // ���� by ������ for test2008/3/03
	if(SecTimer % 60 == 0){
		char sig[3];

		xSignalHeaderRec      *signal_ptr = NULL;
		sig[0] = 3;
		sig[1] = CAMERA_ONE_REQ;
		sig[2] = PIC_SAVE;
		SG_CreateSignal_Para(SG_CAMERA_ONE_ASK,3, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�������Ͷ����еı���
	}
#endif /* (0) */



	if(testtimer%600 == 0) // �ж��Ƿ���Ҫ����
	{
		SG_CreateSignal(SG_CHECK_REBOOT,0, &signal_ptr);
	}

	SG_Send_Item_Timeout_Handle();

	// GPRS��ʱ�ж�
	SG_Do_GprsLink();

	SG_Wait_Ans_Call();

	//TTS�ְ�����
	Sg_Handle_TTS();


}

void Pic_Handle(void)
{
	if(g_xih_ctrl.XIH_State == SG_MENU_CTRL) // �ֱ���װ���Զ��ͻָ���·
	{
		if(PicAckFlag == 1)
		{
			if( 1 == Pic_Receive_Ack())
			{

				if(g_state_info.TestFlag == 1)
				{
					char title[10] = "";
					short len;

					title[0] = 0x01;
					len = 1;
					SG_Menu_Send_Computer(SG_PC_XHON, len, title);

				}
				else
					SG_DynMenu_Put_MsgBox (g_xih_ctrl.menuid, "Ϩ��ɹ�");

				PicAckFlag = 0;


			}
			else if( 2 == Pic_Receive_Ack())
			{
				if(g_state_info.TestFlag == 1)
				{
					char title[10] = "";
					short len;

					title[0] = 0x00;
					len = 1;
					SG_Menu_Send_Computer(SG_PC_XHON, len, title);
				}
				else
					SG_DynMenu_Put_MsgBox (g_xih_ctrl.menuid, "Ϩ��ʧ��");
				PicAckFlag = 0;

			}


		}
		else if(PicAckFlag == 2)
		{
			if( 1 == Pic_Receive_Ack())
			{

				if(g_state_info.TestFlag == 1)
				{
					char title[10] = "";
					short len;

					title[0] = 0x01;
					len = 1;
					SG_Menu_Send_Computer(SG_PC_XHOFF, len, title);
				}
				else
					SG_DynMenu_Put_MsgBox (g_xih_ctrl.menuid, "�ָ���·�ɹ�");
				PicAckFlag = 0;

			}
			else if( 2 == Pic_Receive_Ack())
			{

				if(g_state_info.TestFlag == 1)
				{
					char title[10] = "";
					short len;

					title[0] = 0x00;
					len = 1;
					SG_Menu_Send_Computer(SG_PC_XHOFF, len, title);
			}
				else
				SG_DynMenu_Put_MsgBox (g_xih_ctrl.menuid, "�ָ���·ʧ��");
				PicAckFlag = 0;

			}
		}
		else if(PicAckFlag != 0)
		{
			PicAckFlag = 0;
		}

	}
	else if(g_xih_ctrl.XIH_State == SG_REMOTE_CTRL)  // ����Զ�̶��ͣ��ָ���·
	{
		if(PicAckFlag == 1 || PicAckFlag == 2)
		{
			if( 1 == Pic_Receive_Ack())
			{
				MsgUChar *msg = NULL;
				MsgInt msgLen;
				SCI_MEMCPY(g_state_info.sMsgNo, g_xih_ctrl.sMsgNo,SG_MSG_NO_LEN);

				//������Ӧ����
				if (MsgSafetyOilLock(g_xih_ctrl.para, &msg, &msgLen) == MSG_TRUE)
				{
					//���ͱ��ĵ����Ͷ���
					SG_Send_Put_New_Item(1, msg, msgLen);
				}
				PicAckFlag = 0;

			}
			else if(2 == Pic_Receive_Ack())
			{
				PicAckFlag = 0;
			}
		}
		else if(PicAckFlag != 0)
		{
			PicAckFlag = 0;
		}
	}

}


void SG_Do_timer()
{
		xSignalHeaderRec   *receiveSignal ;

		receiveSignal = SCI_GetSignal(P_APP);
		if(SG_TIME_OUT == receiveSignal->SignalCode)
		{
			SG_HandleSignal(receiveSignal);
		}
		#ifdef _DUART // ˫����ʱ����2���ֱ�
		else if(SG_COM2_BUF == receiveSignal->SignalCode)
		{
			SG_Do_Uart2Handle();	//����ֱ��Ƿ����
		}
		#else
		else if(SG_HH_BUF == receiveSignal->SignalCode)
		{
			SG_Do_hhHandle();//����ֱ��Ƿ����
		}
		#endif
		else if((SG_SEND_HH == receiveSignal->SignalCode) || (SG_ACK_HH == receiveSignal->SignalCode) || (SG_SEND_HH_TO == receiveSignal->SignalCode))
		{
			SG_HH_SEND_ITEM(receiveSignal->SignalCode);
		}
		else if(SG_WAITACK == receiveSignal->SignalCode)
		{
			Pic_Handle();
		}
		if(receiveSignal){
			SCI_FREE(receiveSignal);
			receiveSignal = NULL;
		}

}

void SG_Do_CheckReboot()
{

	// 10�����ж�һ���Ƿ�Ӧ������
	SCI_TIME_T curTime;
	static SCI_TIME_T lstTime ={0};

	SCI_TRACE_LOW("=====SG_CHECK_REBOOT bfirst:%d",bfirst);

	if(bfirst == 0)
	{
		TM_GetSysTime(&curTime);
		SCI_TRACE_LOW("=====SG_Do_CheckReboot: hour:%d, min:%d",curTime.hour,curTime.min);

		if(curTime.hour == 18)
		{
			if((curTime.min >= g_state_info.randtime) && (curTime.min < (g_state_info.randtime+10)))
				SG_Soft_Reset(5);
		}
		else if(curTime.hour == 19)
		{
			if(((curTime.min+60) >= g_state_info.randtime) && ((curTime.min+60) < (g_state_info.randtime+10)))
				SG_Soft_Reset(5);
		}


	}


}





void SG_Main_SigProc(){
	xSignalHeaderRec   *receiveSignal ;
	xSignalHeaderRec      *signal_ptr = NULL;
	int nImageQuality;
	int cam_cmd;
	int save_flag;

	receiveSignal = SCI_GetSignal(P_APP);
	switch (receiveSignal->SignalCode){
		case SG_TIME_OUT:  // 1���Ӷ�ʱ���ж��ź�
			SG_HandleSignal(receiveSignal);
			break;
		case SG_GPS_BUF:  //  ����GPS����
		#if (__camera_debug_ | __gps_debug_)
			SG_Do_Uart0Handle();
		#else
			Gps_Rev_Timeout = GetTimeOut(60);
			SG_Do_GpsHandle();
		#endif
			break;
		case SG_HH_BUF:
			SG_Do_Uart0Handle();
			break;
		#ifdef _DUART
		//��������2�Ϸ�������
		case SG_COM2_BUF:
			SG_Do_Uart2Handle();
			break;
		#endif
		case SG_RCV_BUF:
			SG_Do_Rcv_Gprs(receiveSignal); // ����GPRS���յ��ı���
			break;
		case SG_SEND_MSG: // ��ITEM��put���������棬�����ͱ���
		case SG_ACK_MSG:	// ��Ӧ������������ͱ���
		case SG_SEND_MSG_TO:// ���ͱ��ĳ�ʱ�������ٴη��ͱ���
			SG_GPRS_SEND_ITEM(receiveSignal->SignalCode);
			break;
		//�����ݵ��ֱ�
		case SG_SEND_HH:
		case SG_ACK_HH:
		case SG_SEND_HH_TO:
			SG_HH_SEND_ITEM(receiveSignal->SignalCode);
			break;
		// �з��Ͷ��ŵ�����
		case SG_SEND_SMS:
			SG_SMS_SEND_ITEM();
			break;
		case SG_CREATE_SOCKET: // �����´���SOCKET������
			SG_Net_Init_Gprs();
			break;
		case SG_CHECK_SOCKET_CONNECT: // ���socket�����Ƿ�������
			SG_Check_Socket_State();
			break;
		case SG_HANDUP:  // ����
			SG_Do_Handup();
			break;
		case SG_DEACTIVE_PDP:
			SG_GPRS_DeactivePdpContext();
			break;
		case SG_ACTIVE_PDP:
			SG_GPRS_SetAndActivePdpContex();
			break;
		case SG_CHECK_REBOOT:
			SG_Do_CheckReboot();
			break;
		case SG_ANS_CALL:
			SG_Do_Call();
			break;
		case SG_CALL_LISTEN:
			SG_Call_Listen();
			break;
		case SG_GPIO_CALL:
			//I/O���
			SG_Check_IO(receiveSignal);
			break;
		case SG_CPU_SDBJ:
		case SG_CPU_CMKJ:
		case SG_CPU_QY:
		case SG_CPU_JJQ:
		case SG_CPU_DD:
			SG_Do_IO_Handle(receiveSignal->SignalCode);
			break;
	    case SG_CHANGE_BAUD:
#if (!__camera_debug_ && !__gps_debug_)
			SG_Change_Baud();
#endif /* (0) */
			break;
		case SG_WAITACK:
			Pic_Handle();
			break;
		//����Ƿ��ж���
		case SG_SMS_CHECK:
			SG_Sms_Read();
			break;
		#ifdef _DUART
			//��GPS��Ϣ��CAN
		case SG_PRO_LOCK:
			SG_Send_Project_GPS();
			break;
			//�ع�����Ӧ��
		case SG_PRO_ACK:
			SG_Pro_Lock_Ack();
			break;
		case SG_PRO_VO:
			if(g_pro_lock.nType == PRO_VOLT_LOCK)
				SG_Rcv_Safety_VOLT_Lock();
			else if(g_pro_lock.nType == PRO_VOLT_UNLOCK)
				SG_Rcv_Safety_VOLT_UnLock();
			break;
		#endif

//����ͷ���
		case SG_CAMERA_ONE_ASK:
		case SG_CAMERA_MORE_ASK:
			nImageQuality = *((char*)receiveSignal + 16);
			cam_cmd = *((char*)receiveSignal + 17);
			save_flag = *((char*)receiveSignal + 18);
			//sg_camera_request(nImageQuality,cam_cmd,save_flag);
			SG_Camera_Request(nImageQuality,cam_cmd,save_flag);
			break;
		case SG_CAMERA_WAKEUP:
			Camera_Wakeup_Handle();
			break;
		case SG_CAMERA_TIMEOUT:
			//������ľ����ͷ
			if(g_set_info.bNewUart4Alarm&UT_QQCAMERA)
			{
				QQZM_Camera_TimeOut_Handle();
			}
			else if(g_set_info.bNewUart4Alarm&UT_XGCAMERA)
			{
				camera_timer_handle();
			}
			break;
		case SG_QQZM_PICTURE:
			QQZM_Camera_GetPicture_Send();
			break;
		case SG_CAMERA_RESET:
			camera_reset();
			break;
		case SG_CAMERA_MSGMAKE:
			Camera_MsgMake_Handle();
			// ɾ������ͼ��Ķ�����һ���ڵ�
			camera_cmd_delete();
			SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�������Ͷ����еı���
			break;

		case SG_MSG_HANDLE:   // ���������·�����
			SG_Msg_Handle(receiveSignal);
			break;
		//�����ֻ��·�����
		case SG_SMS_HH:
			smshhlen = *((char*)receiveSignal + 16);
			memcpy(smshhbuf,(char*)receiveSignal + 17,smshhlen);
			//SG_Sms_HH(receiveSignal);
			SMSCheckTime = GetTimeOut(3);
			break;

		case SG_MSG_HH:
			SG_Msg_HH(receiveSignal);
			break;
		case SG_SMS_ZB:
			SG_Sms_ZB(receiveSignal);
			break;
		case SG_SMS_YH:
			SG_Sms_YH(receiveSignal);
			break;


		case SG_SEND_TIME:   // ���ֱ�����Уʱ����Ϣ
			SG_Send_Time_HH(receiveSignal);
			break;

		case SG_SEND_CALLOFF:   // ���ֱ����ͶϿ�ͨ����
			SG_Send_CallStatus(CAll_OFF); // ���ֱ�����ͨ��������״̬
			break;
		case SG_TTS_COMBIN:   // ����TTS���Ժϳɵ���Ϣ
			TTS_Uart_Handle();
			break;
		case SG_SEND_LONGSMS:   // ���ͳ�����
			SG_Send_LongSms();
			break;

	#if __cwt_
		case SG_CAMERA_UPLOAD:   // ����ͼƬ��������  ��ͼ
			Camera_Data_Up(getpicnum);
			break;
	#endif
		case SG_PIC_UP:   // ����ͼƬ��������  ��ͼ
			Camera_Read_Up();
			break;
		default:
			SCI_TRACE_LOW("<<<<UNKOWN SignalCode %x",receiveSignal->SignalCode);
			break;
	}

	if(receiveSignal){
		SCI_FREE(receiveSignal);
		receiveSignal = NULL;
	}
}






/*****************************************************************************/
// 	Description : ��ʼ��MMI����PS�йص�����Ӧ��
//	Global resource dependence :
//  Author: Tracy Zhang
//	Note:
/*****************************************************************************/
void APP_Init(void)
{

#ifdef _DUART
#if (1)
	{	//�������
		if(Log_Enable2 != HH_ENABLE) // only check handset do not set log_enble!
		{
			Log_Enable2 = HH_ENABLE;
#if (1)
			SCI_SetArmLogFlag(0);
			SCI_SetAssertMode(0);
			Log_Enable1 = DEV_ENABLE;
			SIO_SetBaudRate(COM_DEBUG,BAUD_9600);
#endif /* (0) */
		}
		g_framesendok = 1;
//		�����ֱ�
		SG_Do_PowerUpHd();
	}
#endif /* (0) */
#else
#if (1)
	{	//�������
		uint32 bd =0;
		bd = SIO_GetBaudRate(COM_DEBUG);
		if(bd != BAUD_9600)
			SIO_SetBaudRate(COM_DEBUG,BAUD_9600);
		if(Log_Enable1 !=  HH_ENABLE) // only check handset do not set log_enble!
		{
			Log_Enable1 = HH_ENABLE;
			SCI_SetArmLogFlag(0);
			SCI_SetAssertMode(0);
		}
		g_framesendok = 1;
//		�����ֱ�
		SG_Do_PowerUpHd();
		HAL_SetGPIODirection(9,1);				// ����GPIO9 �������
		HAL_SetGPIOVal(9,1);					// ����GPIO9  ��
	}
#endif /* (0) */
#endif


//����gps�����շ�
#if (!__camera_debug_ && !__gps_debug_)
	SIO_SetBaudRate(COM_DATA,BAUD_4800);
#endif

#if __camera_debug_
	SIO_SetBaudRate(COM_DATA,BAUD_115200);
#endif


#if __gps_debug_
	SIO_SetBaudRate(COM_DATA,BAUD_9600);
#endif

	SIO_ATC_SetDataMode(ATC_MODE);

	//������ʼ������
	SecTimer =0;
	LedTimer =0;
	LedCnt   =0;
	GPRS_RDIndex = 0;
	GPRS_WRIndex = 0;
	CameraWatchTimer 		= 0xffffffff;
	CameraConditionTimer 	= 0xffffffff;
	HhTimeCorrectTimer 		= 0xffffffff;
	NetSearchTimer 			= 0xffffffff;
	Pwm_Start_Time	= 0;
	Pwm_End_Time 	= 0;
	if(g_set_info.bNewUart4Alarm&(UT_QQCAMERA|UT_XGCAMERA))
	{
		if(Camera_Buf == NULL)
		{
			Camera_Buf = SCI_ALLOC(35000);
		}

	}
	else
	{
		SCI_FREE(Camera_Buf);
		Camera_Buf = NULL;
	}


	if(TRUE == SG_Set_Check_CheckSum())
	{
		//�����ļ���ȷ������²��������ŷ��Ͷ�ʱ����⣬���������ļ��쳣���µĶ��ŷ���
		SmsSndTimer = g_set_info.SmsSndTimer + 1;
		SCI_TRACE_LOW("=====SmsSndTimer: %d",SmsSndTimer);
	}
	else
	{
		SCI_TRACE_LOW("-----------CHECK SUM ERROR!! do not send sms------------");
	}
	//���������ļ��Ƿ��쳣��������������á������쳣�󱾵��޷��ָ���������

	g_gprs_t.state = GPRS_DISCONNECT;

	GPIO_Init();

	MAX_LIST_NUM = g_set_info.EfsMax-g_set_info.nRegionPiont-g_set_info.nLinePiont;
	if(MAX_LIST_NUM > g_set_info.EfsMax)
		MAX_LIST_NUM = g_set_info.EfsMax;
	else if(MAX_LIST_NUM<30)
		MAX_LIST_NUM = 30;
	SG_Set_Cal_CheckSum();
	g_set_info.nResetCnt++;
	SG_Set_Save();

	Gpio_Custom_Init();// ��ʼ��gpio�ڵķ���ֵ
#ifndef _TRACK
	GPIO_SetValue(HF_MUTE, SCI_TRUE);
#endif

	//����ԭ��
	TM_GetSysDate(&g_reboot_date);
	TM_GetSysTime(&g_reboot_time);
	POWER_GetResetMode();
	GPS_Timer = TM_GetTotalSeconds();
	Gps_Rev_Timeout = GetTimeOut(120);

	g_RevTmr_ptr = SCI_CreateTimer("GPRSTimer",
			RevTmr_isr_handler,
			0,
			1000,
			SCI_AUTO_ACTIVATE);

	// ��ʼ���˵�
	SG_Init_MenuList();
	if(g_state_info.user_type == PRIVATE_CAR_VER)
	{
		Gpio_Register_Callback();
	}
}



/*****************************************************************************/
//  Description:    The function is used to read assert Info from NV and send to the tool.
//	Global resource dependence:
//  Author:         Johnson.sun
//	Note:
/*****************************************************************************/
PUBLIC void SG_Read_AssertInfo(viod)
{
    uint8       cmd_type;
	uint8       cmd_subtype;
	MSG_HEAD_T  *msg_head;
    uint16      len;
    uint16      id;
    uint8       status;
    uint16      sum_len;
    char        *buf;

    char        *str="Recently 10 Assert Informations:";

    buf = (char *)SCI_ALLOC(1024);
    SCI_ASSERT(SCI_NULL != buf);

	sum_len = strlen(str);
	strcpy((char *)buf, (const char *)str);
	strcat((char *)(buf+sum_len), "\r\n");
	sum_len = sum_len + 2;

    for (id = NV_ASSERT_BASE; id < NV_ASSERT_END; id++)
    {
	   char        temp_buf[1024];
       len = NVITEM_GetLength(id);

	   SCI_TRACE_LOW("<<<<LEN = %d",len);
       if (len > 0)
       {
          status = EFS_NvitemRead(id, len, (uint8 *)temp_buf);
          if (NVERR_NONE == status)
          {
              strcat((char *)(buf+sum_len), "\r\n");
              strcat((char *)(buf+sum_len), (const char *)temp_buf);
              sum_len = sum_len + len + 2;
          }
       }
    }

  	SCI_TRACE_LOW("======assert %s",buf);

	SCI_FREE(buf);
}



/*****************************************************************************/
//  Description : the function of mmi entry
//  Global resource dependence :
//
//
//  Author:Louis.wei
//  Modify: Tracy Zhang
//  Note:
/*****************************************************************************/

void APP_Task(
    uint32 argc,
    void * argv
)

{

	POWER_RESTART_CONDITION_E   restart_condition = RESTART_BY_NONE;

	restart_condition = POWER_GetRestartCondition();
	SCI_TRACE_LOW("APP_Task: restart_condition is %d",restart_condition);
	Restart_Reason = restart_condition;
	memset(&g_state_info,0,sizeof(g_state_info));

	GPIO_SetValue(CPU_LED,1);

#if (0)// ���Դ��룬����log
	{//debugģʽ
		uint32 bd =0;
		bd = SIO_GetBaudRate(COM_DEBUG);
		if(bd != BAUD_115200)
			SIO_SetBaudRate(COM_DEBUG,BAUD_115200);
		if(Log_Enable1 != DEBUG_ENABLE)
			Log_Enable1 = DEBUG_ENABLE;

		SCI_SetArmLogFlag(1);
		SCI_SetAssertMode(1);
#ifdef _SUART
		// ������Դ����
		HAL_SetGPIODirection(9,1);				// ����GPIO9 �������
		HAL_SetGPIOVal(9,0);					// ����GPIO9  ��
#endif
	}
#endif



#ifdef _DUART // ��������2
	{
		SIO_CONTROL_S m_dcb;

		//��ʼ������2
		m_dcb.flow_control = 0;
		m_dcb.baud_rate = BAUD_9600;
		SIO_Create(2, COM2, &m_dcb);
	}
#endif

	// ����ϵͳ�����Ĳ�ͬ�����Է������ע��
    switch (restart_condition)
    {
	    case RESTART_BY_POWER_BUTTON:
	    case RESTART_BY_SOFTWARE:
		case RESTART_BY_CHARGE:
	    case RESTART_BY_ALARM:
	    case RESTART_BY_ASSERT:
	    case RESTART_BY_NONE:
		SG_Set_Init();


#if (0)
	{
		SG_Set_Cal_CheckSum();
		g_set_info.bRelease1 = DEV_ENABLE;

		g_set_info.bNewUart4Alarm |= UT_BUS;
//		g_set_info.GpsDevType = 1;
		g_set_info.bNewUart4Alarm = 0;
//		g_set_info.bNewUart4Alarm |= UT_XGCAMERA;

		CurrCamera.currst = CAMERA_START;
		SG_Set_Save();

	}
#endif /* (0) */


		SG_State_Init();
        // MMI���е�APP��ʼ��
	   	APP_Init();


		//gps��ʼ��
    	SG_GPS_Init();
 		// ע��PS����
    	APP_RegisterPsService();
       // ����Э��ջ
        MNPHONE_StartupPs();
		// SMS INIT
		SG_Sms_Init();
		//��ȡIMEI��
		SG_Get_IMEI();
    	break;
    default:
    	POWER_PowerOff();		// �ػ�
    	break;
    }

#if(__cwt_)
#define APNSTR  "lncwt.ln"
//#define ZYDX
//#define HHDZ
#define SZCS
#if (0)
{
	SG_Set_Cal_CheckSum();

	#ifdef ZYDX
		#define APNSTR  "lncwt.ln"
		memset(g_set_info.sCenterIp,0,15);
		strcpy(g_set_info.sCenterIp,"10.64.62.138");
		strcpy(g_set_info.sOwnNo, "13840296139"); // ���ƶ�Ѷ
		strcpy(g_set_info.sCommAddr,"13840296139"); // ���ƶ�Ѷ
		g_set_info.nCenterPort= 6473; // ���ƶ�Ѷ
		strcpy(g_set_info.sOemCode,"6473"); // ���ƶ�Ѷ

	#endif


	#ifdef HHDZ
		#define APNSTR  "lncwt.ln"
		memset(g_set_info.sCenterIp,0,15);
		strcpy(g_set_info.sCenterIp,"10.64.62.138");
		strcpy(g_set_info.sOwnNo, "13840082645"); // ���⳵ƽ̨
		strcpy(g_set_info.sCommAddr,"13840082645"); // ���⳵ƽ̨
		g_set_info.nCenterPort= 6868; // ���ƶ�Ѷ
		strcpy(g_set_info.sOemCode,"6868"); // ���ƶ�Ѷ

	#endif


	#ifdef SZCS
		#define APNSTR  "lncwt.ln"
		memset(g_set_info.sCenterIp,0,15);
		strcpy(g_set_info.sCenterIp,"10.64.62.138");
		strcpy(g_set_info.sOwnNo, "13840043973"); // ���ֳ���
		strcpy(g_set_info.sCommAddr,"13840043973"); //  ���ֳ���
		g_set_info.nCenterPort=7868; // ���ֳ���
		strcpy(g_set_info.sOemCode,"7868"); // ���ֳ���

	#endif


	strcpy(g_set_info.sAPN,APNSTR);
	strcpy(g_set_info.sGprsUser,APNSTR);
	strcpy(g_set_info.sGprsPsw,APNSTR);

	g_set_info.nNetType = 1;
	g_set_info.nWatchType = 3;
	g_set_info.nWatchInterval = 3600;
	g_set_info.bStopReport = 1;
	g_set_info.nWatchTime = 100;

	strcpy(g_set_info.sCenterNo, "13950490809");
	strcpy(g_set_info.sProxyIp,"10.0.0.172");
	g_set_info.nProxyPort=80;
	//�����ն��Ѿ��Ǽ�
	g_set_info.bRegisted = 0;
	g_set_info.bProxyEnable = 0;
	SG_Set_Save();

}
#endif /* (0) */
#else
#if (0)
{
	SG_Set_Cal_CheckSum();

	strcpy(g_set_info.sAPN,"cmnet");
	strcpy(g_set_info.sGprsUser,"cmnet");
	strcpy(g_set_info.sGprsPsw,"cmnet");
	g_set_info.nNetType = 1;
	//g_set_info.nWatchType = 1;
	//g_set_info.nWatchInterval = 10;
	//g_set_info.bStopReport = 1;

	memset(g_set_info.sCenterIp,0,15);
	strcpy(g_set_info.sCenterIp,"211.138.135.58");
	strcpy(g_set_info.sOwnNo, "13075829962");
//	strcpy(g_set_info.sCenterNo, "13799325132");
	strcpy(g_set_info.sProxyIp,"10.0.0.172");
	g_set_info.nProxyPort=80;
	//�����ն��Ѿ��Ǽ�
	g_set_info.bRegisted = 1;
	g_set_info.nCenterPort=443;
	g_set_info.bProxyEnable = 0;

//    strcpy(g_set_info.sListenNo, "13959859567");
	SG_Set_Save();
}
#endif /* (0) */

#if (0)
{
	SG_Set_Cal_CheckSum();
	strcpy(g_set_info.sAPN,"cmwap");
	strcpy(g_set_info.sGprsUser,"cmwap");
	strcpy(g_set_info.sGprsPsw,"cmwap");

	g_set_info.nNetType = 0;
	g_set_info.nWatchType = 1;
	g_set_info.nWatchInterval = 3600;
	g_set_info.bStopReport = 1;

	memset(g_set_info.sCenterIp,0,15);
	strcpy(g_set_info.sCenterIp,"218.5.3.228");
//	strcpy(g_set_info.sCenterIp,"61.154.22.43");
//	strcpy(g_set_info.sOwnNo, "13559172704");
	strcpy(g_set_info.sOwnNo, "15859106834");

//	strcpy(g_set_info.sOwnNo, "");
	strcpy(g_set_info.sCenterNo, "15005076962");
	strcpy(g_set_info.sProxyIp,"10.0.0.172");
	g_set_info.nProxyPort=80;
	//�����ն��Ѿ��Ǽ�
	g_set_info.bRegisted = 1;
	g_set_info.nCenterPort=80;
	g_set_info.bProxyEnable = 1;

	strcpy(g_set_info.sHelpNo,"15859005120");
	strcpy(g_set_info.sServiceNo,"15859005120");
	strcpy(g_set_info.sMedicalNo,"15859005120");
	SG_Set_Save();

}
#endif /* (0) */

#endif

// 	HAL_SetGPIOVal(PWREN_GPS,0);					// ����GPS�˿ڶϵ�

	//�����ȴ�15��
	while(SecTimer <= 14){
		SG_Do_timer(); //��ʱ��
		Gpio_Get_Init_Val();
	}

//	HAL_SetGPIOVal(PWREN_GPS,1);

#if (!__camera_debug_ && !__gps_debug_)
	SG_Do_getDev();//��������
#endif

	{
		int len;
		len =sizeof(g_set_info);
		SCI_TRACE_LOW("-------------GSET LENTH == %d",len);
	}
	SCI_TRACE_LOW("@_@--<<<<<<CURRENT VER:%s ,len %d",g_set_info.sVersion,strlen(g_set_info.sVersion));
	SCI_TRACE_LOW("================randtime  %d",g_state_info.randtime);
	//�����󣬷Ƕ���ģʽֱ��ȥ����
	if(g_set_info.nNetType && !(strlen(g_set_info.sOwnNo) == 0))
	{
		SG_GPRS_SetAndActivePdpContex();
	}

	if((g_set_info.bNewUart4Alarm & (UT_XGCAMERA|UT_QQCAMERA)) && ((g_set_info.sg_camera[0].nCount > 0)||(g_set_info.sg_camera[0].setCount == 0)))
	{
		CameraWatchTimer = GetTimeOut(1);
	}

	while(1)
	{
		SG_Main_SigProc();
 	}
}  //End of mmi_Task


