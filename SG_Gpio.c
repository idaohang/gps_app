/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Gpio.c
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-7-23
  ����������  
  �޸ļ�¼��
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SG_GPS.h"
#include "SG_Send.h"
#include "Msg.h"
#include "SG_Set.h"
#include "tb_hal.h"
#include "sio.h"
#include "SG_DynamicMenu.h"
#include "SG_CRC16.h"

extern uint8 tryCount;
extern long AlarmTime;

void Gpio_Disable_Int (void)
{

	HAL_DisableGPIOIntCtl(CPU_JJQ_IN);
	
#if (0)
	HAL_DisableGPIOIntCtl(CPU_ACC_IN);
	HAL_DisableGPIOIntCtl(CPU_CMKG_IN);
	HAL_DisableGPIOIntCtl(CPU_SDBJ_IN);
	HAL_DisableGPIOIntCtl(VCAR_PFO);
	#ifdef _TRACK
	HAL_DisableGPIOIntCtl(CPU_DD_IN);
	#endif
#endif /* (0) */
}

// ��ʼ��gpio�ڵķ���ֵ
void Gpio_Custom_Init(void)
{
	// �� CLK/ACK��TXD ��������Ϊ�������������ݸ�	
	HAL_SetGPIODirection(PIC_ACK ,SCI_TRUE ); 	// ����CLK/ACK���ŷ���Ϊ���	
	HAL_SetGPIOVal(PIC_ACK ,SCI_TRUE);			// ����CLK/ACK��������ߵ�ƽ		
	HAL_SetGPIODirection(PIC_TXD ,SCI_TRUE ); 	// ����TXD���ŷ���Ϊ���	PIC_GP1
	HAL_SetGPIOVal(PIC_TXD,SCI_TRUE);			// ����TXD��������ߵ�ƽ		
	HAL_SetGPIODirection(CPU_WDT,1);			// ����CPU_WDT �������
	HAL_SetGPIOVal(CPU_WDT,0);					// ����CPU_WDT ���ŵ�
	HAL_SetGPIODirection(PWR_SB,1);				// ����PWR_SB �������
	HAL_SetGPIOVal(PWR_SB,1);					// ����PWR_SB ���ŵ�

	HAL_SetGPIODirection(PWREN_GPS,1);			// ����PWR_GPS�������
	if(g_set_info.GpsPowrFlag == 1)
	{
#ifdef _TRACK
	HAL_SetGPIOVal(PWREN_GPS,0);				// ����PWR_GPS���ŵ�
#else
	HAL_SetGPIOVal(PWREN_GPS,1);				// ����PWR_GPS���Ÿ�
#endif
	}
	else
	{
#ifdef _TRACK
	HAL_SetGPIOVal(PWREN_GPS,1);				
#else
	HAL_SetGPIOVal(PWREN_GPS,0);				
#endif

	}

}


void Gpio_Callback_Func(uint32 gpio_id, uint32 gpio_state)
{
	xSignalHeaderRec      *signal_ptr = NULL;
	char sig[2];

	sig[0] = gpio_id;
	sig[1] = gpio_state;
	
	
	HAL_DisableGPIOIntCtl(gpio_id);
	SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���

}

void Gpio_Register_Callback(void)
{
	HAL_AddGPIOToCallbackTable(CPU_JJQ_IN,1,20,Gpio_Callback_Func);
#if (0)
	if(g_state_info.user_type == PRIVATE_CAR_VER)
		HAL_AddGPIOToCallbackTable(CPU_JJQ_IN,1,1300,Gpio_Callback_Func);
	else
		HAL_AddGPIOToCallbackTable(CPU_JJQ_IN,1,1300,Gpio_Callback_Func);

	HAL_AddGPIOToCallbackTable(CPU_ACC_IN,1,1200,Gpio_Callback_Func);
	if (g_state_info.user_type == PRIVATE_CAR_VER)
		HAL_AddGPIOToCallbackTable(CPU_CMKG_IN,1,1000,Gpio_Callback_Func);
	else
		HAL_AddGPIOToCallbackTable(CPU_CMKG_IN,1,1000,Gpio_Callback_Func);

	HAL_AddGPIOToCallbackTable(CPU_SDBJ_IN,1,1100,Gpio_Callback_Func);
	HAL_AddGPIOToCallbackTable(VCAR_PFO,1,2900,Gpio_Callback_Func);

#ifdef _TRACK
	HAL_AddGPIOToCallbackTable(CPU_DD_IN,1,1500,Gpio_Callback_Func);
#endif
#endif /* (0) */
}

void Gpio_Get_Init_Val(void)
{
	static int bkalarmState=0;
	int bNeed = 0;



	//�ֶ�����
	if( 0 != strcmp (g_set_info.sOwnNo,"")  )// �Ѿ����ñ�������,�ֶ�����״̬������ȡ��
	{
		if(g_state_info.sdbj == 1)
		{
			if(!(g_state_info.alarmState & GPS_CAR_STATU_HIJACK))
			{
				g_state_info.alarmState |= GPS_CAR_STATU_HIJACK;
				bNeed =1;
				SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_SDBJ_IN ON");
			}
		}


	}
	else// û�����ñ�������ʱ���ֶ�����״̬��io��״̬ȡ��
	{
		if(g_state_info.sdbj == 1)
		{
			if(!(g_state_info.alarmState & GPS_CAR_STATU_HIJACK))
			{
				g_state_info.alarmState |= GPS_CAR_STATU_HIJACK;
				bNeed =1;
				SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_SDBJ_IN ON");
			}
		}
		else
		{	
			if(g_state_info.alarmState & GPS_CAR_STATU_HIJACK)
			{
				SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_SDBJ_IN %x ",g_set_info.alarmState);
				g_state_info.alarmState &= ~GPS_CAR_STATU_HIJACK;
				if(g_set_info.alarmEnable & GPS_CAR_STATU_HIJACK)
				{
					if(bNeed != 1)
						bNeed =2;
				}
				SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_SDBJ_IN OFF");
			}
		}
	}
	

	//���ż��
	if(g_state_info.cmjc == 1)  //�Ѵ���
	{
		if(!(g_state_info.alarmState & GPS_CAR_STATU_IO1))
		{
			g_state_info.alarmState |= GPS_CAR_STATU_IO1;
			if(g_set_info.alarmEnable & GPS_CAR_STATU_IO1)
				bNeed =1;
			SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:SELF DEFINE ON");
		}

	}
	else
	{
		if(g_state_info.alarmState &GPS_CAR_STATU_IO1)
		{
			SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_CMKG_IN %x ",g_state_info.alarmState);
			g_state_info.alarmState &= ~GPS_CAR_STATU_IO1;
			if(g_set_info.alarmEnable & GPS_CAR_STATU_IO1)
			{
				if(bNeed != 1)
					bNeed =2;
			}
			SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_CMKG_IN OFF");
		}
	}
		
#ifdef _TRACK
	//���߼��
	if(g_state_info.gpstx == 1)  //�Ѵ���
	{
		g_state_info.line = 1;
		if(g_set_info.bNewUart4Alarm & UT_LOCK)
		{
			if(!(g_state_info.alarmState & GPS_CAR_STATU_ROB_ALARM))
			{
			
				g_state_info.alarmState |= GPS_CAR_STATU_ROB_ALARM;
				if(g_set_info.alarmEnable & GPS_CAR_STATU_ROB_ALARM)	
				bNeed =1;
			
			}
		}
	}
	else
	{
		g_state_info.line = 0;
		if(g_set_info.bNewUart4Alarm & UT_LOCK)
		{
			if(g_state_info.alarmState & GPS_CAR_STATU_ROB_ALARM)
			{
				g_state_info.alarmState &= ~GPS_CAR_STATU_ROB_ALARM;
				if(g_set_info.alarmEnable & GPS_CAR_STATU_ROB_ALARM)
				{
					if(bNeed != 1)
						bNeed =2;
				}
			}	
		}
	}


	//����
	if(g_state_info.dd == 1)  //�Ѵ���
	{
		if(!(g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF))
		{
			g_state_info.alarmState |= GPS_CAR_STATU_POWER_OFF;
			if(g_set_info.alarmEnable & GPS_CAR_STATU_POWER_OFF)	
				bNeed =1;
			SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:power off alarm ON");
		}
	}
	else
	{
		if(g_state_info.alarmState &GPS_CAR_STATU_POWER_OFF)
		{
			g_state_info.alarmState &= ~GPS_CAR_STATU_POWER_OFF;
			if(g_set_info.alarmEnable & GPS_CAR_STATU_POWER_OFF)
			{
				if(bNeed != 1)
					bNeed =2;
			}
			SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:power off alarm OFF");
		}
	}

	if (!(g_state_info.user_type == PRIVATE_CAR_VER))
	{
		//�Ƽ������	
		if(g_state_info.jjq == 1)  //�Ѵ���
		{
			if(!(g_state_info.alarmState & GPS_CAR_STATU_JJQ))
			{
				g_state_info.alarmState |= GPS_CAR_STATU_JJQ;

				SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_JJQ_IN ON");
			}

		}
		else
		{
			if(g_state_info.alarmState &GPS_CAR_STATU_JJQ)
			{
				g_state_info.alarmState &= ~GPS_CAR_STATU_JJQ;

				SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_JJQ_IN OFF");
			}		
		}
	}
#else
	if (!(g_state_info.user_type == PRIVATE_CAR_VER))
	{
		//�Ƽ������	
		if(g_state_info.jjq == 1)  //�Ѵ���
		{
			if(g_set_info.bBatteryCheckEnable) // ���õ��籨��
			{
				if(!(g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF))
				{
					g_state_info.alarmState |= GPS_CAR_STATU_POWER_OFF;
					if(g_set_info.alarmEnable & GPS_CAR_STATU_POWER_OFF)	
						bNeed =1;
					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:power off alarm ON");
				}
				
			}
			else
			{
				if(!(g_state_info.alarmState & GPS_CAR_STATU_JJQ))
				{
					g_state_info.alarmState |= GPS_CAR_STATU_JJQ;

					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_JJQ_IN ON");
				}
			}

		}
		else
		{
			if(g_set_info.bBatteryCheckEnable) // ���õ��籨��
			{
				if(g_state_info.alarmState &GPS_CAR_STATU_POWER_OFF)
				{
					g_state_info.alarmState &= ~GPS_CAR_STATU_POWER_OFF;
					if(g_set_info.alarmEnable & GPS_CAR_STATU_POWER_OFF)
					{
						if(bNeed != 1)
							bNeed =2;
					}
					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:power off alarm OFF");
				}
			}
			else
			{
				if(g_state_info.alarmState &GPS_CAR_STATU_JJQ)
				{
					g_state_info.alarmState &= ~GPS_CAR_STATU_JJQ;

					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_JJQ_IN OFF");
				}
			}
		}
	}
#endif

	//acc
	if(g_state_info.acc == 1)  //�Ѵ���
	{
		if(!(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
		{
			g_state_info.alarmState |= GPS_CAR_STATU_GAS_ALARM;
			SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_ACC_IN ON");
			if(g_set_info.alarmState & GPS_CAR_STATU_ROB_ALARM)	
			{
				tryCount = 3;
				Pic_Send(0xa6); 	
			}

			//��������������Ϩ��
			if ((g_set_info.bTheftDisable == 0 && g_set_info.nTheftState == 1))	
			{						
				int menuid = MENUID_SINGLE;		
				if((g_set_info.alarmState & GPS_CAR_STATU_ROB_ALARM) == 0)		
				{							
					g_state_info.alarmState |= GPS_CAR_STATU_ROB_ALARM;		
					SG_Set_Cal_CheckSum();							
					g_set_info.alarmState |= GPS_CAR_STATU_ROB_ALARM;		
					SG_Set_Save();							
					
					SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "��ӭʹ��������ݳ���ϵͳ");			
					tryCount = 3;
					Pic_Send(0xa6); 	
				}						
			}
		
		}
	}
	else
	{
		if(g_state_info.alarmState &GPS_CAR_STATU_GAS_ALARM)
		{
			g_state_info.alarmState &= ~GPS_CAR_STATU_GAS_ALARM;
			SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_ACC_IN OFF");
		}
	}
	
	//VCAR_PFO  Ƿѹ
	if(g_state_info.qy== 1)  //�Ѵ���
	{
		if(!(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE))
		{
			g_state_info.alarmState |= GPS_CAR_STATU_LESS_PRESSURE;
			if(g_set_info.alarmEnable & GPS_CAR_STATU_LESS_PRESSURE)
				bNeed =1;
			SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:VCAR_PFO ON");
		}
	}
	else
	{
		if(g_state_info.alarmState &GPS_CAR_STATU_LESS_PRESSURE)
		{
			g_state_info.alarmState &= ~GPS_CAR_STATU_LESS_PRESSURE;
			if(g_set_info.alarmEnable & GPS_CAR_STATU_LESS_PRESSURE)
			{
				if(bNeed != 1)
					bNeed =2;
			}
			SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:VCAR_PFO OFF");
		}
	}
	
	if( 0 != strcmp (g_set_info.sOwnNo,""))
	{
		if(g_state_info.alarmState &GPS_CAR_STATU_HIJACK && !(bkalarmState &GPS_CAR_STATU_HIJACK)) 
		{
			SG_Set_Cal_CheckSum();
			g_set_info.alarmState |= GPS_CAR_STATU_HIJACK;
			SG_Set_Save();
		}
	}
	bkalarmState = g_state_info.alarmState;

	
}




void SG_Do_IO_Handle(int SignalCode)
{
	static int bkalarmState=0;
	int bNeed = 0;

	switch(SignalCode)
	{
		case SG_CPU_SDBJ:
			g_state_info.alarmState |= GPS_CAR_STATU_HIJACK;
				
			if(g_set_info.bNewUart4Alarm & UT_LED)
			{
				char ledmsg[128]={0};
				int ledlen=0;
				ledmsg[0]=0x0a;
				strcpy(&ledmsg[1],"$$00S0FF0102�ұ����,�뱨��&&");
				ledlen=strlen(ledmsg);
				ledmsg[ledlen]=0x0d;
				SIO_WriteFrame(COM_DEBUG,ledmsg,ledlen+1);				
			}
			#ifdef _DUART
			else if(g_set_info.bNewUart4Alarm & UT_LED2)
			{
				char ledmsg[128]={0};
				int ledlen=0;
				ledmsg[0]=0x0a;
				strcpy(&ledmsg[1],"$$00S0FF0102�ұ����,�뱨��&&");
				ledlen=strlen(ledmsg);
				ledmsg[ledlen]=0x0d;
				SIO_WriteFrame(COM2,ledmsg,ledlen+1);	
			}
			#endif
			
			//���ͱ������Ÿ�����			
			if(g_set_info.SmsEnable == 1)			
			{				
				char title[140]= "";				
				int pTP_len;				
				unsigned char pTP_UD[140]="";						
				strcpy(title, "������������!!!\r\n");				
				pTP_len= GBK2Unicode(pTP_UD, (unsigned char*)title);				
				UnicodeBE2LE((char*)pTP_UD, pTP_len);					
				SendMsgReqToMN((uint8*)g_state_info.sUserNo,(uint8*)pTP_UD,pTP_len,MN_SMS_UCS2_ALPHABET);			
			}
			
			if(g_set_info.alarmEnable & GPS_CAR_STATU_HIJACK)
			{
				bNeed = 1;
			}
			break;
			
		case SG_CPU_CMKJ:
			if(!(g_state_info.alarmState & GPS_CAR_STATU_IO1))
			{
				g_state_info.alarmState |= GPS_CAR_STATU_IO1;
				if(g_set_info.alarmEnable & GPS_CAR_STATU_IO1)
				{
					bNeed = 1;
				}					
				SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:SELF DEFINE ON");
			}
			break;
			
		case SG_CPU_QY:
			if(!(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE))
			{
				g_state_info.alarmState |= GPS_CAR_STATU_LESS_PRESSURE;
				if(g_set_info.alarmEnable & GPS_CAR_STATU_LESS_PRESSURE)
				{
					bNeed = 1;
				}					
				SCI_TRACE_LOW("@_@@@@@@ --SG_Do_QY:SELF DEFINE ON");
			}

			g_watch_info.nGetTotalDistance = g_state_info.nGetTotalDistance;
			g_watch_info.nGetDistance[0] = g_state_info.nGetDistance[0];
			g_watch_info.nGetDistance[1] = g_state_info.nGetDistance[1];
			SG_Watch_Save();
			break;

#ifdef _TRACK

		case SG_CPU_JJQ:
			if(!(g_state_info.alarmState & GPS_CAR_STATU_JJQ))
			{
				g_state_info.alarmState |= GPS_CAR_STATU_JJQ;

				SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_JJQ_IN ON");
			}			
			break;
			
		case SG_CPU_DD:
			if(!(g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF))
			{
				g_state_info.alarmState &= ~GPS_CAR_STATU_LESS_PRESSURE;
				g_state_info.alarmState |= GPS_CAR_STATU_POWER_OFF;
				if(g_set_info.alarmEnable & GPS_CAR_STATU_POWER_OFF)	
				{
					bNeed = 1;
				}
				SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:power off alarm ON");
			}
			g_watch_info.nGetTotalDistance = g_state_info.nGetTotalDistance;
			g_watch_info.nGetDistance[0] = g_state_info.nGetDistance[0];
			g_watch_info.nGetDistance[1] = g_state_info.nGetDistance[1];
			SG_Watch_Save();
			break;
			
#else	
		case SG_CPU_JJQ:
			if(g_set_info.bBatteryCheckEnable) // ���õ��籨��
			{
				if(!(g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF))
				{
					g_state_info.alarmState &= ~GPS_CAR_STATU_LESS_PRESSURE;
					g_state_info.alarmState |= GPS_CAR_STATU_POWER_OFF;
					if(g_set_info.alarmEnable & GPS_CAR_STATU_POWER_OFF)	
					{
						bNeed = 1;
					}
					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:power off alarm ON");
				}
				g_watch_info.nGetTotalDistance = g_state_info.nGetTotalDistance;
				g_watch_info.nGetDistance[0] = g_state_info.nGetDistance[0];
				g_watch_info.nGetDistance[1] = g_state_info.nGetDistance[1];
				SG_Watch_Save();
						
			}
			else
			{
				if(!(g_state_info.alarmState & GPS_CAR_STATU_JJQ))
				{
					g_state_info.alarmState |= GPS_CAR_STATU_JJQ;


					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_JJQ_IN ON");
				}
			}
			break;
#endif
		default:
			break;
	}

	if( 0 != strcmp (g_set_info.sOwnNo,""))
	{
		if(g_state_info.alarmState &GPS_CAR_STATU_HIJACK && !(bkalarmState &GPS_CAR_STATU_HIJACK)) 
		{
			SG_Set_Cal_CheckSum();
			g_set_info.alarmState |= GPS_CAR_STATU_HIJACK;
			SG_Set_Save();
		}
	}
	bkalarmState = g_state_info.alarmState;

	Camera_Condition_Watch();

	//���������㱨����
	if (bNeed == 1)
	{
		unsigned char *msg = NULL;
		int len;

		bNeed = 0;

		SCI_TRACE_LOW("<<<%&^&$$&*(");
		if (MsgSafetyAlarm(g_state_info.pGpsFull,  &msg, &len) != MSG_TRUE)
			return ;

		SG_Send_Put_Alarm_Item(msg,len);
		
		if(g_set_info.nNetType == 0) // ����ģʽ		
		{			
			xSignalHeaderRec     *signal_ptr = NULL;
			SG_CreateSignal(SG_SEND_SMS,0, &signal_ptr); 
			SCI_TRACE_LOW("============== send IO alarm sms==============");
		}

		return ;
	}
	else if(bNeed == 2)   //����ȡ��Ҳ����һ�����汨��
	{
		unsigned char *msg = NULL;
		int len;
		bNeed = 0;
		SCI_MEMCPY(g_state_info.sMsgNo, g_set_info.sWatchMsgNo,sizeof(g_state_info.sMsgNo));
		if (MsgWatchByTime((gps_data*)g_state_info.pGpsFull,  &msg, &len) != MSG_TRUE)
			return ;

		SG_Send_Put_New_Item(1, msg, len);
	}

	
}

 
void SG_Check_IO(xSignalHeaderRec   *receiveSignal)
{
	char gpio_id;
	char gpio_state;
	char ledmsg[128]={0};
	int ledlen=0;
	static int pwm_low = 0;
	static int pwm_high = 0; 
	int bNeed = 0;
	
	gpio_id = *((char*)receiveSignal + 16);
	gpio_state = *((char*)receiveSignal + 17);
	SCI_TRACE_LOW("<<<<<<ID = %d,state = %d",gpio_id,gpio_state);

	switch(gpio_id)
	{

#ifdef _TRACK
		case CPU_LINE_IN:
			if(gpio_state == 0)
			{
				g_state_info.line = 0;
			}
			else
			{
				g_state_info.line = 1;

				if(g_set_info.bNewUart4Alarm & UT_LOCK)
				{
					if((g_set_info.alarmState & GPS_CAR_STATU_ROB_ALARM) == 0)		
					{							
						g_state_info.alarmState |= GPS_CAR_STATU_ROB_ALARM;		
						SG_Set_Cal_CheckSum();							
						g_set_info.alarmState |= GPS_CAR_STATU_ROB_ALARM;		
						SG_Set_Save();							
						if(g_set_info.alarmEnable & GPS_CAR_STATU_ROB_ALARM)	
						{							
							bNeed = 1;							
						}								
		
						{
							unsigned char closeLockMsg[]={0x02,0x01,0x02};
							SG_Lock_Uart_Write(closeLockMsg, sizeof(closeLockMsg));
						}
						Pic_Send(0xa6); 
						
					}		
				}

			}
			break;

		case CPU_JJQ_IN: //  �Ƽ������
			if (g_state_info.user_type == PRIVATE_CAR_VER) // ˽�ҳ���⳵������
			{
				if(gpio_state == 0)
				{
					pwm_low = GetTimeOut(0);
					if(Pwm_Start_Time == 0) // �״α�����
					{
						SCI_TRACE_LOW("----First start!!");
						Pwm_Start_Time = GetTimeOut(0);
						pwm_low = GetTimeOut(0);
					}
					else
					{
						if(pwm_low - pwm_high <= 1)
						{
							Pwm_End_Time = GetTimeOut(0);
						}
						else
						{
							Pwm_Start_Time = 0;
							Pwm_End_Time = 0;
							pwm_high = 0;
							pwm_low = 0;
						}
						
					}
				}
				else
				{
					pwm_high = GetTimeOut(0);
					if(pwm_high - pwm_low <= 1) // ��һ������
					{
						Pwm_End_Time = pwm_high;
					}
					else
					{
						Pwm_Start_Time = 0;
						Pwm_End_Time = 0;
						pwm_high = 0;
						pwm_low = 0;
					}

				}
			}
			else
			{
				if(gpio_state == 0)
				{

					if(!(g_state_info.alarmState & GPS_CAR_STATU_JJQ))
					{
						g_state_info.alarmState |= GPS_CAR_STATU_JJQ;

						SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_JJQ_IN ON");
					}	

				}
				else
				{
						
					if(g_state_info.alarmState &GPS_CAR_STATU_JJQ)
					{
						g_state_info.alarmState &= ~GPS_CAR_STATU_JJQ;

						SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_JJQ_IN OFF");
					}
				}
			}	
			break;
		case CPU_DD_IN:
			if(gpio_state == 0)
			{
				if(!(g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF))
				{
					g_state_info.alarmState &= ~GPS_CAR_STATU_LESS_PRESSURE;
					g_state_info.alarmState |= GPS_CAR_STATU_POWER_OFF;
					if(g_set_info.alarmEnable & GPS_CAR_STATU_POWER_OFF)	
					{
						bNeed = 1;
					}
					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:power off alarm ON");
				}
				g_watch_info.nGetTotalDistance = g_state_info.nGetTotalDistance;
				g_watch_info.nGetDistance[0] = g_state_info.nGetDistance[0];
				g_watch_info.nGetDistance[1] = g_state_info.nGetDistance[1];
				g_watch_info.AccOnTime = g_state_info.AccOnTime;
				SG_Watch_Save();
			}
			else
			{

				if(g_state_info.alarmState &GPS_CAR_STATU_POWER_OFF)
				{
					g_state_info.alarmState &= ~GPS_CAR_STATU_POWER_OFF;
					if(g_set_info.alarmEnable & GPS_CAR_STATU_POWER_OFF)
					{
						if(bNeed != 1)
							bNeed =2;
					}
					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:power off alarm OFF");
				}
			}
			break;
			
#else

		case CPU_JJQ_IN: //  �Ƽ������
			if (g_state_info.user_type == PRIVATE_CAR_VER) // ˽�ҳ���⳵������
			{
				//ACC�زŽ��з����ж�
				if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM) == 0)
				{
					if(gpio_state == 0)
					{
						pwm_low = GetTimeOut(0);
						if(Pwm_Start_Time == 0) // �״α�����
						{
							SCI_TRACE_LOW("----First start!!");
							Pwm_Start_Time = GetTimeOut(0);
							pwm_low = GetTimeOut(0);
						}
						else
						{
							if(pwm_low - pwm_high <= 1)
							{
								Pwm_End_Time = GetTimeOut(0);
							}
							else
							{
								Pwm_Start_Time = 0;
								Pwm_End_Time = 0;
								pwm_high = 0;
								pwm_low = 0;
							}
							
						}
					}
					else
					{
						pwm_high = GetTimeOut(0);
						if(pwm_high - pwm_low <= 1) // ��һ������
						{
							Pwm_End_Time = pwm_high;
						}
						else
						{
							Pwm_Start_Time = 0;
							Pwm_End_Time = 0;
							pwm_high = 0;
							pwm_low = 0;
						}

					}
				}
			}
			else
			{
				if(gpio_state == g_set_info.JJQLev)
				{

					if(g_set_info.bBatteryCheckEnable) // ���õ��籨��
					{
						if(!(g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF))
						{
							g_state_info.alarmState &= ~GPS_CAR_STATU_LESS_PRESSURE;
							g_state_info.alarmState |= GPS_CAR_STATU_POWER_OFF;
							if(g_set_info.alarmEnable & GPS_CAR_STATU_POWER_OFF)	
							{
								bNeed = 1;
							}
							SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:power off alarm ON");
						}
						g_watch_info.nGetTotalDistance = g_state_info.nGetTotalDistance;
						g_watch_info.nGetDistance[0] = g_state_info.nGetDistance[0];
						g_watch_info.nGetDistance[1] = g_state_info.nGetDistance[1];
						SG_Watch_Save();
								
					}
					else
					{
						if(!(g_state_info.alarmState & GPS_CAR_STATU_JJQ))
						{
							g_state_info.alarmState |= GPS_CAR_STATU_JJQ;


							SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_JJQ_IN ON");
						}
					}
				}
				else
				{					
					if(g_set_info.bBatteryCheckEnable) // ���õ��籨��
					{
						if(g_state_info.alarmState &GPS_CAR_STATU_POWER_OFF)
						{
							g_state_info.alarmState &= ~GPS_CAR_STATU_POWER_OFF;
							if(g_set_info.alarmEnable & GPS_CAR_STATU_POWER_OFF)
							{
								if(bNeed != 1)
									bNeed =2;
							}
							SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:power off alarm OFF");
						}
					}
					else
					{
						if(g_state_info.alarmState &GPS_CAR_STATU_JJQ)
						{
							g_state_info.alarmState &= ~GPS_CAR_STATU_JJQ;

							SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_JJQ_IN OFF");
						}
					}

				}
			}	
			break;

#endif

		case CPU_CMKG_IN: // ���ż��
			if(!(g_set_info.alarmSet & GPS_CAR_STATU_IO1))// �Զ��屨���ߵ�ƽ����
			{
				if(gpio_state == 1)
				{
					if(!(g_state_info.alarmState & GPS_CAR_STATU_IO1))
					{
						g_state_info.alarmState |= GPS_CAR_STATU_IO1;
                        //���ͱ������Ÿ�����
    					if(g_set_info.SmsEnable == 1)
    					{				
    						char title[140]= "";				
    						int pTP_len;				
    						unsigned char pTP_UD[140]="";						
    						strcpy(title, "���ĳ��������쳣��������״̬���뼰ʱȷ���ֳ���������ϳ�����ȫ��лл!\r\n");											
    						pTP_len= GBK2Unicode(pTP_UD, (unsigned char*)title);				
    						UnicodeBE2LE((char*)pTP_UD, pTP_len);					
    						SendMsgReqToMN((uint8*)g_set_info.sUserNo,(uint8*)pTP_UD,pTP_len,MN_SMS_UCS2_ALPHABET);			
    					}
						if(g_set_info.alarmEnable & GPS_CAR_STATU_IO1)
						{
							bNeed = 1;
						}					
						SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:SELF DEFINE ON");
					}

				}
				else
				{
					if(g_state_info.alarmState & GPS_CAR_STATU_IO1)
					{
						SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_CMKG_IN %x ",g_state_info.alarmState);
						g_state_info.alarmState &= ~GPS_CAR_STATU_IO1;
						if(g_set_info.alarmEnable & GPS_CAR_STATU_IO1)
						{
							if(bNeed != 1)
								bNeed =2;
						}
						SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_CMKG_IN OFF");
					}
				}
				
			}
			else // �Զ��屨���͵�ƽ����
			{
				if(gpio_state == 0)
				{
					if(!(g_state_info.alarmState & GPS_CAR_STATU_IO1))
					{
						g_state_info.alarmState |= GPS_CAR_STATU_IO1;
                        //���ͱ������Ÿ�����
    					if(g_set_info.SmsEnable == 1)
    					{				
    						char title[140]= "";				
    						int pTP_len;				
    						unsigned char pTP_UD[140]="";						
    						strcpy(title, "���ĳ��������쳣��������״̬���뼰ʱȷ���ֳ���������ϳ�����ȫ��лл!\r\n");											
    						pTP_len= GBK2Unicode(pTP_UD, (unsigned char*)title);				
    						UnicodeBE2LE((char*)pTP_UD, pTP_len);					
    						SendMsgReqToMN((uint8*)g_set_info.sUserNo,(uint8*)pTP_UD,pTP_len,MN_SMS_UCS2_ALPHABET);			
    					}
						if(g_set_info.alarmEnable & GPS_CAR_STATU_IO1)
						{
							bNeed = 1;
						}					
						SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:SELF DEFINE ON");
					}

				}
				else
				{

					if(g_state_info.alarmState & GPS_CAR_STATU_IO1)
					{
						SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_CMKG_IN %x ",g_state_info.alarmState);
						g_state_info.alarmState &= ~GPS_CAR_STATU_IO1;
						if(g_set_info.alarmEnable & GPS_CAR_STATU_IO1)
						{
							if(bNeed != 1)
								bNeed =2;
						}
						SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_CMKG_IN OFF");
					}
				}
			}
			break;
		case CPU_ACC_IN: // ACC���
			if(gpio_state == 0)
			{
				if(!(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
				{
					g_state_info.alarmState |= GPS_CAR_STATU_GAS_ALARM;
					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_ACC_IN ON");
				}
				//��������������Ϩ��
				if ((g_set_info.bTheftDisable == 0 && g_set_info.nTheftState == 1))	
				{						
					int menuid = MENUID_SINGLE;		
					if((g_set_info.alarmState & GPS_CAR_STATU_ROB_ALARM) == 0)		
					{							
						g_state_info.alarmState |= GPS_CAR_STATU_ROB_ALARM;		
						SG_Set_Cal_CheckSum();							
						g_set_info.alarmState |= GPS_CAR_STATU_ROB_ALARM;		
						SG_Set_Save();							
						if(g_set_info.alarmEnable & GPS_CAR_STATU_ROB_ALARM)	
						{							
							bNeed = 1;							
						}								
						SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "��ӭʹ��������ݳ���ϵͳ");			
						tryCount = 3;
						Pic_Send(0xa6); 	

						
						
						//չ������������������
						if(g_set_info.GpsDevType == 1)	
						{				
							g_state_info.SendSMSType = 1;
							g_state_info.SendSMSCount = 2;
							SG_AlarmSms_Send();		
						}
					}						
				}
			}
			else
			{
			
				if(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)
				{

					unsigned char *msg = NULL;
					int len;
					g_state_info.alarmState &= ~GPS_CAR_STATU_GAS_ALARM;
					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_ACC_IN OFF");
					g_watch_info.nGetTotalDistance = g_state_info.nGetTotalDistance;
					g_watch_info.nGetDistance[0] = g_state_info.nGetDistance[0];
					g_watch_info.nGetDistance[1] = g_state_info.nGetDistance[1];
					g_watch_info.AccOnTime = g_state_info.AccOnTime;
					SG_Watch_Save();
					if (MsgWatchByTime(g_state_info.pGpsFull, &msg, &len) == MSG_TRUE)
					{
						SG_Send_Put_New_Item(3, msg, len);
					}
					
				}
			}
			break;
		case VCAR_PFO://VCAR_PFO  Ƿѹ
			if(gpio_state == 0)
			{
				if(!(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE))
				{
					g_state_info.alarmState |= GPS_CAR_STATU_LESS_PRESSURE;
					if(g_set_info.alarmEnable & GPS_CAR_STATU_LESS_PRESSURE)
					{
						bNeed = 1;
					}					
					SCI_TRACE_LOW("@_@@@@@@ --SG_Do_QY:SELF DEFINE ON");
				}

				g_watch_info.nGetTotalDistance = g_state_info.nGetTotalDistance;
				g_watch_info.nGetDistance[0] = g_state_info.nGetDistance[0];
				g_watch_info.nGetDistance[1] = g_state_info.nGetDistance[1];
				SG_Watch_Save();

			}
			else
			{
				if(g_state_info.alarmState &GPS_CAR_STATU_LESS_PRESSURE)
				{
					g_state_info.alarmState &= ~GPS_CAR_STATU_LESS_PRESSURE;
					if(g_set_info.alarmEnable & GPS_CAR_STATU_LESS_PRESSURE)
					{
						if(bNeed != 1)
							bNeed =2;
					}
					SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:VCAR_PFO OFF");
				}
			}
			break;
		case CPU_SDBJ_IN:	//�ֶ�����
			if(gpio_state == 0)
			{
				if(!(g_state_info.alarmState & GPS_CAR_STATU_HIJACK))
				{
					g_state_info.alarmState |= GPS_CAR_STATU_HIJACK;
						
					if(g_set_info.bNewUart4Alarm & UT_HBLED)
					{
						unsigned short CRC;
						unsigned char crctmp[64]={0};
						
						ledmsg[0]=0x7e;
						ledmsg[1]=0x43;
						ledmsg[2]=0x00;
						ledmsg[3]=0x0B;
						strcpy(&ledmsg[6],"ALAR");

						memcpy(crctmp,&ledmsg[1],11);
						
						CRC = CRC_16(crctmp, 11);
						ledmsg[12] = *((MsgUChar*)(&CRC));
						ledmsg[13] = *((MsgUChar*)(&CRC)+1);

						ledmsg[14]=0x7e;
						
						SIO_WriteFrame(COM_DEBUG,ledmsg,15);	

					}
					else if(g_set_info.bNewUart4Alarm & UT_LED)
					{
						char ledmsg[128]={0};
						int ledlen=0;
						ledmsg[0]=0x0a;
						strcpy(&ledmsg[1],"$$00S0FF0102�ұ����,�뱨��&&");
						ledlen=strlen(ledmsg);
						ledmsg[ledlen]=0x0d;
						SIO_WriteFrame(COM_DEBUG,ledmsg,ledlen+1);				
					}
					#ifdef _DUART
					else if(g_set_info.bNewUart4Alarm & UT_LED2)
					{
						char ledmsg[128]={0};
						int ledlen=0;
						ledmsg[0]=0x0a;
						strcpy(&ledmsg[1],"$$00S0FF0102�ұ����,�뱨��&&");
						ledlen=strlen(ledmsg);
						ledmsg[ledlen]=0x0d;
						SIO_WriteFrame(COM2,ledmsg,ledlen+1);	
					}
					#endif
					
					//���ͱ������Ÿ�����			
					
					if(g_set_info.GpsDevType == 1)	
					{				
						g_state_info.SendSMSType = 2;
						g_state_info.SendSMSCount = 2;
						SG_AlarmSms_Send();		
					}
					else if(g_set_info.SmsEnable == 1)
					{											
						char title[140]= "";											
						int pTP_len;											
						unsigned char pTP_UD[140]="";													
						strcpy(title, "���İ����ִ��ڽ�������״̬���뼰ʱȷ���ֳ���������ϳ�̨��ȫ��лл!\r\n");											
						pTP_len= GBK2Unicode(pTP_UD, (unsigned char*)title);											
						UnicodeBE2LE((char*)pTP_UD, pTP_len);												
						SendMsgReqToMN((uint8*)g_set_info.sUserNo,(uint8*)pTP_UD,pTP_len,MN_SMS_UCS2_ALPHABET);		
					}
			
					
					if(g_set_info.alarmEnable & GPS_CAR_STATU_HIJACK)
					{
						bNeed = 1;
					}
					if( 0 != strcmp (g_set_info.sOwnNo,""))
					{
						SG_Set_Cal_CheckSum();
						g_set_info.alarmState |= GPS_CAR_STATU_HIJACK;
						SG_Set_Save();
					}
				}

			}
			else
			{
				if( 0 == strcmp (g_set_info.sOwnNo,"")  )
				{
					if(g_state_info.alarmState &GPS_CAR_STATU_HIJACK)
					{
						SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_SDBJ_IN %x ",g_state_info.alarmState);
						g_state_info.alarmState &= ~GPS_CAR_STATU_HIJACK;
						if(g_set_info.alarmEnable & GPS_CAR_STATU_HIJACK)
						{
							if(bNeed != 1)
								bNeed =2;
						}
						SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:CPU_SDBJ_IN OFF");
					}
				}
			}
			break;
		default:
			break;

	}

#if (0)
	if( 0 != strcmp (g_set_info.sOwnNo,""))
	{
		if(g_state_info.alarmState &GPS_CAR_STATU_HIJACK && !(bkalarmState &GPS_CAR_STATU_HIJACK)) 
		{
			SG_Set_Cal_CheckSum();
			g_set_info.alarmState |= GPS_CAR_STATU_HIJACK;
			SG_Set_Save();
		}
	}
	bkalarmState = g_state_info.alarmState;
#endif /* (0) */

	Camera_Condition_Watch();

	//���������㱨����
	if (bNeed == 1)
	{
		unsigned char *msg = NULL;
		int len;

		bNeed = 0;
		if(g_set_info.nNetType == 0) // ����ģʽ
			AlarmTime = GetTimeOut(g_set_info.nSmsInterval);
		else
			AlarmTime = GetTimeOut(SG_ALARM_TIME);
		SCI_TRACE_LOW("====AlarmTime:%d",AlarmTime);
		if (MsgSafetyAlarm(g_state_info.pGpsFull,  &msg, &len) != MSG_TRUE)
			return ;

		//����ģʽ�£�GPRS��ͨ�����Ƿ��Ͷ���
		if((g_set_info.SmsAlarmFlag == 1)&&(g_set_info.nNetType == 1))
		{
			SG_AlarmSms_Save(msg, len);
		}

		SG_Send_Put_Alarm_Item(msg,len);
		
		if(g_set_info.nNetType == 0) // ����ģʽ		
		{			
			xSignalHeaderRec     *signal_ptr = NULL;
			SG_CreateSignal(SG_SEND_SMS,0, &signal_ptr); 
			SCI_TRACE_LOW("============== send gps alarm sms==============");
		}



		return ;
	}
	else if(bNeed == 2)   //����ȡ��Ҳ����һ�����汨��
	{
		unsigned char *msg = NULL;
		int len;
		bNeed = 0;
		SCI_MEMCPY(g_state_info.sMsgNo, g_set_info.sWatchMsgNo,sizeof(g_state_info.sMsgNo));
		if (MsgWatchByTime((gps_data*)g_state_info.pGpsFull,  &msg, &len) != MSG_TRUE)
			return ;

		SG_Send_Put_New_Item(1, msg, len);
	}

}

#if (0)
void SG_Hand_ADC()
{
	uint32 temp;
	
	temp = 0;

	//*(volatile uint32*)(0x8B000000) |= 0x502;
	//ADC_START(0x51);

	//temp =   *(volatile uint32 *)(0x8B000004);
	//temp = read_adc_result(0x51);
	temp = ADC_GetResultDirectly(1, 0);
	g_state_info.CurrAD = temp;
	
	SCI_TRACE_LOW("==AD:%d",temp);




}
#endif /* (0) */

