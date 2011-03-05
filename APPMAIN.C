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
** 12/2003       Tracy Zhang       对任务初始化和对服务的注册进行修改      *
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
//  Note: 注册app回调函数
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

	GPS_Timer ++;   // GPS 系统时间描述记录

 	camera_timout_check();

	//手柄数据发送超时判断
	SG_HH_Send_To_Handle();

//短信检测使能
	if(g_state_info.SendSMSFlag == 1)
	{
		SG_SendSms_Flag();
	}
//短信重发
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

//熄火状态
	if(g_set_info.JDQ == 1 && SecTimer == 2)
	{
		JDQ_Init();
	}


//解决短信接收延时问题
#if (1)
	if(g_set_info.nNetType == 0)
	{

		if((SecTimer%70 == 0) && (g_state_info.SendSMSFlag == 0) && (g_gprs_t.state != GPRS_DEACTIVE_PDP))
		{
			SG_CreateSignal(SG_SMS_CHECK, 0, &signal_ptr);
		}
	}
#endif /* (0) */

//掉电熄火
#ifdef _TRACK
	if(g_set_info.ddjdqFlag == 1)
	{
		DDjdq_Handle();
	}
#endif


//解决网络异常
#if(1)
	{
		//约5小时主动断开
		if(SecTimer%18633 == 0)
		{
			SG_Net_Disconnect();
		}

		//网络状态为4,达2分钟系统重启
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

	//发送心跳
	SG_Send_Heart_Beat();

	//监听电话应答判断
	SG_Wait_ListenCall();

	//GPS波特率切换
	Gps_Auto_Adapt();


	// 重启系统
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

	// 拨打坐席电话
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
	Pro_Time_Handle(); // 重工锁车
	#endif
#endif /* (0) */

	WDT_Timer++;   // 喂狗
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

	//华翰屏握手
	SG_LED_Hand();

	SG_Watch_Send();

	SG_Net_Err_Handle();

	SG_GpsDev_Handle();

	Camera_Up_Pic_Judge();

#if (0)
	 // 创建 by 郭碧莲 for test2008/3/03
	if(SecTimer % 60 == 0){
		char sig[3];

		xSignalHeaderRec      *signal_ptr = NULL;
		sig[0] = 3;
		sig[1] = CAMERA_ONE_REQ;
		sig[2] = PIC_SAVE;
		SG_CreateSignal_Para(SG_CAMERA_ONE_ASK,3, &signal_ptr,sig); // 发送信号到主循环要求处理发送队列中的报文
	}
#endif /* (0) */



	if(testtimer%600 == 0) // 判断是否需要重启
	{
		SG_CreateSignal(SG_CHECK_REBOOT,0, &signal_ptr);
	}

	SG_Send_Item_Timeout_Handle();

	// GPRS超时判断
	SG_Do_GprsLink();

	SG_Wait_Ans_Call();

	//TTS分包发送
	Sg_Handle_TTS();


}

void Pic_Handle(void)
{
	if(g_xih_ctrl.XIH_State == SG_MENU_CTRL) // 手柄安装调试断油恢复油路
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
					SG_DynMenu_Put_MsgBox (g_xih_ctrl.menuid, "熄火成功");

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
					SG_DynMenu_Put_MsgBox (g_xih_ctrl.menuid, "熄火失败");
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
					SG_DynMenu_Put_MsgBox (g_xih_ctrl.menuid, "恢复油路成功");
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
				SG_DynMenu_Put_MsgBox (g_xih_ctrl.menuid, "恢复油路失败");
				PicAckFlag = 0;

			}
		}
		else if(PicAckFlag != 0)
		{
			PicAckFlag = 0;
		}

	}
	else if(g_xih_ctrl.XIH_State == SG_REMOTE_CTRL)  // 中心远程断油，恢复油路
	{
		if(PicAckFlag == 1 || PicAckFlag == 2)
		{
			if( 1 == Pic_Receive_Ack())
			{
				MsgUChar *msg = NULL;
				MsgInt msgLen;
				SCI_MEMCPY(g_state_info.sMsgNo, g_xih_ctrl.sMsgNo,SG_MSG_NO_LEN);

				//发送响应报文
				if (MsgSafetyOilLock(g_xih_ctrl.para, &msg, &msgLen) == MSG_TRUE)
				{
					//发送报文到发送队列
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
		#ifdef _DUART // 双串口时串口2接手柄
		else if(SG_COM2_BUF == receiveSignal->SignalCode)
		{
			SG_Do_Uart2Handle();	//检测手柄是否存在
		}
		#else
		else if(SG_HH_BUF == receiveSignal->SignalCode)
		{
			SG_Do_hhHandle();//检测手柄是否存在
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

	// 10分钟判断一次是否应该重起
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
		case SG_TIME_OUT:  // 1秒钟定时器中断信号
			SG_HandleSignal(receiveSignal);
			break;
		case SG_GPS_BUF:  //  接收GPS报文
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
		//处理串口2上发的数据
		case SG_COM2_BUF:
			SG_Do_Uart2Handle();
			break;
		#endif
		case SG_RCV_BUF:
			SG_Do_Rcv_Gprs(receiveSignal); // 解析GPRS接收到的报文
			break;
		case SG_SEND_MSG: // 有ITEM被put到队列里面，请求发送报文
		case SG_ACK_MSG:	// 有应答回来，请求发送报文
		case SG_SEND_MSG_TO:// 发送报文超时，请求再次发送报文
			SG_GPRS_SEND_ITEM(receiveSignal->SignalCode);
			break;
		//发数据到手柄
		case SG_SEND_HH:
		case SG_ACK_HH:
		case SG_SEND_HH_TO:
			SG_HH_SEND_ITEM(receiveSignal->SignalCode);
			break;
		// 有发送短信的请求
		case SG_SEND_SMS:
			SG_SMS_SEND_ITEM();
			break;
		case SG_CREATE_SOCKET: // 有重新创建SOCKET的请求
			SG_Net_Init_Gprs();
			break;
		case SG_CHECK_SOCKET_CONNECT: // 检查socket连接是否连接上
			SG_Check_Socket_State();
			break;
		case SG_HANDUP:  // 握手
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
			//I/O检查
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
		//检查是否有短信
		case SG_SMS_CHECK:
			SG_Sms_Read();
			break;
		#ifdef _DUART
			//发GPS信息给CAN
		case SG_PRO_LOCK:
			SG_Send_Project_GPS();
			break;
			//重工锁车应答
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

//摄像头相关
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
			//青青子木摄像头
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
			// 删掉拍摄图像的队列中一个节点
			camera_cmd_delete();
			SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文
			break;

		case SG_MSG_HANDLE:   // 解析短信下发报文
			SG_Msg_Handle(receiveSignal);
			break;
		//处理手机下发短信
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


		case SG_SEND_TIME:   // 向手柄发送校时的信息
			SG_Send_Time_HH(receiveSignal);
			break;

		case SG_SEND_CALLOFF:   // 向手柄发送断开通话的
			SG_Send_CallStatus(CAll_OFF); // 向手柄发送通话结束的状态
			break;
		case SG_TTS_COMBIN:   // 发送TTS语言合成的消息
			TTS_Uart_Handle();
			break;
		case SG_SEND_LONGSMS:   // 发送长短信
			SG_Send_LongSms();
			break;

	#if __cwt_
		case SG_CAMERA_UPLOAD:   // 发送图片包给中心  灵图
			Camera_Data_Up(getpicnum);
			break;
	#endif
		case SG_PIC_UP:   // 发送图片包给中心  灵图
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
// 	Description : 初始化MMI中与PS有关的所有应用
//	Global resource dependence :
//  Author: Tracy Zhang
//	Note:
/*****************************************************************************/
void APP_Init(void)
{

#ifdef _DUART
#if (1)
	{	//串口相关
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
//		启用手柄
		SG_Do_PowerUpHd();
	}
#endif /* (0) */
#else
#if (1)
	{	//串口相关
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
//		启用手柄
		SG_Do_PowerUpHd();
		HAL_SetGPIODirection(9,1);				// 设置GPIO9 引脚输出
		HAL_SetGPIOVal(9,1);					// 设置GPIO9  低
	}
#endif /* (0) */
#endif


//启用gps串口收发
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

	//重启初始化变量
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
		//配置文件正确的情况下才允许短信发送定时器检测，避免配置文件异常导致的短信费用
		SmsSndTimer = g_set_info.SmsSndTimer + 1;
		SCI_TRACE_LOW("=====SmsSndTimer: %d",SmsSndTimer);
	}
	else
	{
		SCI_TRACE_LOW("-----------CHECK SUM ERROR!! do not send sms------------");
	}
	//不管配置文件是否异常，超级密码均可用。避免异常后本地无法恢复出厂设置

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

	Gpio_Custom_Init();// 初始化gpio口的方向及值
#ifndef _TRACK
	GPIO_SetValue(HF_MUTE, SCI_TRUE);
#endif

	//重启原因
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

	// 初始化菜单
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

#if (0)// 调试代码，开启log
	{//debug模式
		uint32 bd =0;
		bd = SIO_GetBaudRate(COM_DEBUG);
		if(bd != BAUD_115200)
			SIO_SetBaudRate(COM_DEBUG,BAUD_115200);
		if(Log_Enable1 != DEBUG_ENABLE)
			Log_Enable1 = DEBUG_ENABLE;

		SCI_SetArmLogFlag(1);
		SCI_SetAssertMode(1);
#ifdef _SUART
		// 启用有源串口
		HAL_SetGPIODirection(9,1);				// 设置GPIO9 引脚输出
		HAL_SetGPIOVal(9,0);					// 设置GPIO9  低
#endif
	}
#endif



#ifdef _DUART // 开启串口2
	{
		SIO_CONTROL_S m_dcb;

		//初始化串口2
		m_dcb.flow_control = 0;
		m_dcb.baud_rate = BAUD_9600;
		SIO_Create(2, COM2, &m_dcb);
	}
#endif

	// 根据系统启动的不同条件对服务进行注册
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
        // MMI所有的APP初始化
	   	APP_Init();


		//gps初始化
    	SG_GPS_Init();
 		// 注册PS服务
    	APP_RegisterPsService();
       // 激活协议栈
        MNPHONE_StartupPs();
		// SMS INIT
		SG_Sms_Init();
		//获取IMEI号
		SG_Get_IMEI();
    	break;
    default:
    	POWER_PowerOff();		// 关机
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
		strcpy(g_set_info.sOwnNo, "13840296139"); // 中移鼎讯
		strcpy(g_set_info.sCommAddr,"13840296139"); // 中移鼎讯
		g_set_info.nCenterPort= 6473; // 中移鼎讯
		strcpy(g_set_info.sOemCode,"6473"); // 中移鼎讯

	#endif


	#ifdef HHDZ
		#define APNSTR  "lncwt.ln"
		memset(g_set_info.sCenterIp,0,15);
		strcpy(g_set_info.sCenterIp,"10.64.62.138");
		strcpy(g_set_info.sOwnNo, "13840082645"); // 出租车平台
		strcpy(g_set_info.sCommAddr,"13840082645"); // 出租车平台
		g_set_info.nCenterPort= 6868; // 中移鼎讯
		strcpy(g_set_info.sOemCode,"6868"); // 中移鼎讯

	#endif


	#ifdef SZCS
		#define APNSTR  "lncwt.ln"
		memset(g_set_info.sCenterIp,0,15);
		strcpy(g_set_info.sCenterIp,"10.64.62.138");
		strcpy(g_set_info.sOwnNo, "13840043973"); // 数字城市
		strcpy(g_set_info.sCommAddr,"13840043973"); //  数字城市
		g_set_info.nCenterPort=7868; // 数字城市
		strcpy(g_set_info.sOemCode,"7868"); // 数字城市

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
	//设置终端已经登记
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
	//设置终端已经登记
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
	//设置终端已经登记
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

// 	HAL_SetGPIOVal(PWREN_GPS,0);					// 设置GPS端口断电

	//开机等待15秒
	while(SecTimer <= 14){
		SG_Do_timer(); //定时器
		Gpio_Get_Init_Val();
	}

//	HAL_SetGPIOVal(PWREN_GPS,1);

#if (!__camera_debug_ && !__gps_debug_)
	SG_Do_getDev();//启用外设
#endif

	{
		int len;
		len =sizeof(g_set_info);
		SCI_TRACE_LOW("-------------GSET LENTH == %d",len);
	}
	SCI_TRACE_LOW("@_@--<<<<<<CURRENT VER:%s ,len %d",g_set_info.sVersion,strlen(g_set_info.sVersion));
	SCI_TRACE_LOW("================randtime  %d",g_state_info.randtime);
	//重启后，非短信模式直接去连接
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



