/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2008��Ȩ����
  �ļ��� ��SG_Timer.c
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
#include "SG_Receive.h"
#include "SG_Sms.h"
#include "SG_Set.h"
#include "SG_Call.h"
#include "mn_events.h"
#include "sci_types.h"
#include "os_api.h"
#include "tb_hal.h"
#include "mn_type.h"
#include "simat_data_object.h"
#include "simat_proactive.h"
#include "simat_api.h"
#include "simat_callback.h"
#include "sim_dphone_stk_verify.h"

#include "env_app_signal.h"
#include "string.h"
#include "SG_GPS.h"
#include "app_tcp_if.h"
#include "SG_Net.h"
#include "SG_Call.h"
#include "SG_Phone.h"
#include "SG_Timer.h"
#include "SG_Hh.h"
#include "SG_Camera.h"

extern SG_CAMERA CurrCamera;
extern SCI_TIMER_PTR g_RevTmr_ptr;
uint8 StatusLedTimer;
uint8 CameraSyncTimer;
uint8 CameraSendPicTimer;

void SG_CreateSignal (uint16       signalType,
                       int16      signalSize,
                       xSignalHeaderRec **signal)
{
	(*signal) = SCI_ALLOC(signalSize + 16);
    if((*signal) != NULL){
	    (*signal) ->SignalCode = signalType;
		SCI_SendSignal((xSignalHeader)(*signal),P_APP);
   }
    else{
        SCI_TRACE_LOW("SG_CreateSignal : alloc signal error, signal szie = %d",signalSize);
    }
	//SCI_TRACE_LOW("==SG_CreateSignal:%d",signalType);
}


void SG_CreateSignal_Para (uint16       signalType,
                       int16      signalSize,
                       xSignalHeaderRec **signal,
                       char *para)
{
	uint32 sig_size;

	sig_size = signalSize+sizeof(xSignalHeaderRec);
    (*signal) = SCI_ALLOC(signalSize + 16);
	if(signal != NULL)
	{
		(*signal)->Sender	= P_APP;
		(*signal)->SignalSize = sig_size;
		(*signal)->SignalCode = signalType;
		SCI_MEMCPY((void *)((*signal) + 1), para, signalSize);
		SCI_SendSignal((xSignalHeader)(*signal),P_APP);
	}
	else
	{
        SCI_TRACE_LOW("SG_CreateSignal : alloc signal error, signal szie = %d",signalSize);
	}
}


void MnTime_EventCallback(
                                uint32 task_id, //task ID
                                uint32 event_id, //event ID
                                void *param_ptr
                                )
{


	SCI_ASSERT(PNULL != param_ptr);
	if( PNULL == param_ptr )
	{
		SCI_TRACE_HIGH("-----mmi_callback: the ponit of param is empty");
		return;
	}

	// distinguish the different message type to set the signal_size and signal_code
	switch(event_id)
	{
		SCI_TRACE_LOW("***:%d",event_id);
		break;
	}
}


void RevTmr_isr_handler(uint32 state)
{	
	static int persec=0;	
	static uint8 cpu_led = 0;
	xSignalHeaderRec      *signal_ptr = NULL;//	static uint8 cpu_led = 0;
	static int init_count = 0;
	

#if (0)
	g_io_status.sdbj_cnt++;
	g_io_status.acc_cnt++;
	g_io_status.cmjc_cnt++;
	g_io_status.jjq_cnt++;
	g_io_status.qy_cnt++;
	g_io_status.dd_cnt++;
#endif /* (0) */

	
	persec++;
	StatusLedTimer++;
	
	if(LedCnt)
		LedTimer ++;
	if(g_state_info.Rcv_Data_To)
		g_state_info.Rcv_Data_To ++;
	//ÿ0.3��IO��ѯ���һ��
	if(persec % 3 == 0)
	{

		//�ֶ�����
		if(HAL_GetGPIOVal(CPU_SDBJ_IN) == 0)
		{	
			g_io_status.sdbj_cancel_cnt = 0;
			if(g_state_info.sdbj == 0)   //δ����
			{
				g_io_status.sdbj_cnt++;
				//SCI_TRACE_LOW("==%d",g_io_status.sdbj_cnt);
				if(g_io_status.sdbj_cnt >= 5)
				{

					xSignalHeaderRec      *signal_ptr = NULL;
					char sig[2];

					sig[0] = CPU_SDBJ_IN;
					sig[1] = g_state_info.sdbj;     //����
		
					SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					g_state_info.sdbj = 1;
					g_io_status.sdbj_cnt = 0;

				}
			}
		}
		else
		{
			g_io_status.sdbj_cnt = 0;
			//SCI_TRACE_LOW("==%d",g_io_status.sdbj_cnt);
			if(g_state_info.sdbj == 1)  //�Ѵ���
			{
				g_io_status.sdbj_cancel_cnt++;
				if(g_io_status.sdbj_cancel_cnt >= 2)
				{
					
					xSignalHeaderRec      *signal_ptr = NULL;
					char sig[2];

					sig[0] = CPU_SDBJ_IN;
					sig[1] = 1;			//ȡ��
		
					SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					g_state_info.sdbj = 0;
					g_io_status.sdbj_cancel_cnt = 0;
				}
					
			}
		}			
		

		//���ż��
		if(!(g_set_info.alarmSet & GPS_CAR_STATU_IO1))// �Զ��屨���ߵ�ƽ����
		{
			if(HAL_GetGPIOVal(CPU_CMKG_IN) == 1)
			{
				g_io_status.cmjc_cancel_cnt = 0;
				if(g_state_info.cmjc == 0)   //δ����
				{
					
					g_io_status.cmjc_cnt++;
					if(g_io_status.cmjc_cnt >= 6)
					{

						xSignalHeaderRec      *signal_ptr = NULL;
						char sig[2];

						sig[0] = CPU_CMKG_IN;
						sig[1] = 1;              //����
			
						SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
						g_state_info.cmjc = 1;
						g_io_status.cmjc_cnt = 0;
					}
				}
			}
			else
			{
				g_io_status.cmjc_cnt = 0;
				if(g_state_info.cmjc == 1)  //�Ѵ���
				{
					g_io_status.cmjc_cnt = 0;
					g_io_status.cmjc_cancel_cnt++;
					if(g_io_status.cmjc_cancel_cnt >= 2)
					{
						
						xSignalHeaderRec      *signal_ptr = NULL;
						char sig[2];

						sig[0] = CPU_CMKG_IN;
						sig[1] = 0;			//ȡ��
			
						SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
						g_state_info.cmjc = 0;
						g_io_status.cmjc_cancel_cnt = 0;
					}
						
				}

			}
			
		}
		else // �Զ��屨���͵�ƽ����
		{
			if(HAL_GetGPIOVal(CPU_CMKG_IN) == 0)
			{
				g_io_status.cmjc_cancel_cnt = 0;
				if(g_state_info.cmjc == 0)   //δ����
				{
					g_io_status.cmjc_cancel_cnt = 0;
					g_io_status.cmjc_cnt++;
					if(g_io_status.cmjc_cnt >= 6)
					{

						xSignalHeaderRec      *signal_ptr = NULL;
						char sig[2];

						sig[0] = CPU_CMKG_IN;
						sig[1] = 0;              //����
			
						SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
						g_state_info.cmjc = 1;
						g_io_status.cmjc_cnt = 0;
					}
				}
			}
			else
			{
				g_io_status.cmjc_cnt = 0;
				if(g_state_info.cmjc == 1)  //�Ѵ���
				{
				
					g_io_status.cmjc_cancel_cnt++;
					if(g_io_status.cmjc_cancel_cnt >= 2)
					{
						
						xSignalHeaderRec      *signal_ptr = NULL;
						char sig[2];

						sig[0] = CPU_CMKG_IN;
						sig[1] = 1;			//ȡ��
			
						SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
						g_state_info.cmjc = 0;
						g_io_status.cmjc_cancel_cnt = 0;
					}
						
				}

			}
		}
		
		//Ƿѹ����
		if(HAL_GetGPIOVal(VCAR_PFO) == 0)
		{
			g_io_status.qy_cancel_cnt = 0;
			if(g_state_info.qy == 0)   //δ����
			{
				g_io_status.qy_cancel_cnt = 0;
				g_io_status.qy_cnt++;
				if(g_io_status.qy_cnt >= 40)
				{

					xSignalHeaderRec      *signal_ptr = NULL;
					char sig[2];

					sig[0] = VCAR_PFO;
					sig[1] = 0;              //����
		
					SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					g_state_info.qy = 1;
					g_io_status.qy_cnt = 0;
				}
			}
		}
		else
		{	
			g_io_status.qy_cnt = 0;
			if(g_state_info.qy == 1)  //�Ѵ���
			{
				g_io_status.qy_cnt = 0;
				g_io_status.qy_cancel_cnt++;
				if(g_io_status.qy_cancel_cnt >= 3)
				{
					
					xSignalHeaderRec      *signal_ptr = NULL;
					char sig[2];

					sig[0] = VCAR_PFO;
					sig[1] = 1;			//ȡ��
		
					SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					g_state_info.qy = 0;
					g_io_status.qy_cancel_cnt = 0;
				}
					
			}
		}


		//�Ƽۼ��
		if (!(g_state_info.user_type == PRIVATE_CAR_VER))
		{
			if(HAL_GetGPIOVal(CPU_JJQ_IN) == g_set_info.JJQLev)
			{
				g_io_status.jjq_cancel_cnt = 0;
				if(g_state_info.jjq== 0)   //δ����
				{
					g_io_status.jjq_cancel_cnt = 0;
					g_io_status.jjq_cnt++;
					if(g_io_status.jjq_cnt >= 15)
					{

						xSignalHeaderRec      *signal_ptr = NULL;
						char sig[2];

						sig[0] = CPU_JJQ_IN;
						sig[1] = g_set_info.JJQLev;              //����
			
						SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
						g_state_info.jjq = 1;
						g_io_status.jjq_cnt = 0;
					}
				}
			}
			else
			{	
				g_io_status.jjq_cnt = 0;
				if(g_state_info.jjq == 1)  //�Ѵ���
				{
					g_io_status.jjq_cancel_cnt++;
					if(g_io_status.jjq_cancel_cnt >= 3)
					{
						
						xSignalHeaderRec      *signal_ptr = NULL;
						char sig[2];

						sig[0] = CPU_JJQ_IN;
						sig[1] = (!g_set_info.JJQLev);			//ȡ��
			
						SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
						g_state_info.jjq = 0;
						g_io_status.jjq_cancel_cnt = 0;
					}
						
				}
			}
		}
		//ACC���
		if(HAL_GetGPIOVal(CPU_ACC_IN) == 0)
		{
			g_io_status.acc_cancel_cnt = 0;
			if(g_state_info.acc== 0)   //δ����
			{
				g_io_status.acc_cnt++;
				if(g_io_status.acc_cnt >= 6)
				{

					xSignalHeaderRec      *signal_ptr = NULL;
					char sig[2];

					sig[0] = CPU_ACC_IN;
					sig[1] = 0;              //����
		
					SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					g_state_info.acc = 1;
					g_io_status.acc_cnt = 0;
				}
			}
		}
		else
		{	
			g_io_status.acc_cnt = 0;
			if(g_state_info.acc == 1)  //�Ѵ���
			{
				g_io_status.acc_cancel_cnt++;
				if(g_io_status.acc_cancel_cnt >= 2)
				{
					
					xSignalHeaderRec      *signal_ptr = NULL;
					char sig[2];

					sig[0] = CPU_ACC_IN;
					sig[1] = 1;			//ȡ��
		
					SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					g_state_info.acc = 0;
					g_io_status.acc_cancel_cnt = 0;
				}
					
			}
		}
		


#ifdef _TRACK
		//GPS���߼��
		if(HAL_GetGPIOVal(CPU_LINE_IN) == 0)
		{
			g_io_status.gpstx_cancel_cnt = 0;
			if(g_state_info.gpstx == 1)   //�Ѵ���
			{
			
				g_io_status.gpstx_cnt++;
				if(g_io_status.gpstx_cnt >= 18)
				{

					xSignalHeaderRec      *signal_ptr = NULL;
					char sig[2];

					sig[0] = CPU_LINE_IN;
					sig[1] = 0;             
		
					SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					g_state_info.gpstx = 0;
					g_io_status.gpstx_cnt = 0;
				}
			}
		}
		else
		{
			g_io_status.gpstx_cnt = 0;
			if(g_state_info.gpstx == 0)  //δ����
			{
			
				g_io_status.gpstx_cancel_cnt++;
				if(g_io_status.gpstx_cancel_cnt >= 18)
				{
					
					xSignalHeaderRec      *signal_ptr = NULL;
					char sig[2];

					sig[0] = CPU_LINE_IN;
					sig[1] = 1;			//ȡ��
		
					SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					g_state_info.gpstx = 1;
					g_io_status.gpstx_cancel_cnt = 0;
				}
					
			}
		}

		
		//������
		if(HAL_GetGPIOVal(CPU_DD_IN) == 0)
		{
			g_io_status.dd_cancel_cnt = 0;
			if(g_state_info.dd== 0)   //δ����
			{
			
				g_io_status.dd_cnt++;
				if(g_io_status.dd_cnt >= 18)
				{

					xSignalHeaderRec      *signal_ptr = NULL;
					char sig[2];

					sig[0] = CPU_DD_IN;
					sig[1] = 0;              //����
		
					SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					g_state_info.dd = 1;
					g_io_status.dd_cnt = 0;
				}
			}
		}
		else
		{	
			g_io_status.dd_cnt = 0;
			if(g_state_info.dd == 1)  //�Ѵ���
			{
				g_io_status.dd_cancel_cnt++;
				if(g_io_status.dd_cancel_cnt >= 18)
				{
					
					xSignalHeaderRec      *signal_ptr = NULL;
					char sig[2];

					sig[0] = CPU_DD_IN;
					sig[1] = 1;			//ȡ��
		
					SG_CreateSignal_Para(SG_GPIO_CALL,2, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					g_state_info.dd = 0;
					g_io_status.dd_cancel_cnt = 0;
				}
					
			}
		}
#endif
	}


	
	// ͬ��״̬ÿ��0.2s����һ��ͬ�����������ͷ


#if (0)
	if(CurrCamera.currst == CAMERA_SYNC)
	{
		CameraSyncTimer ++;
		if(CameraSyncTimer%2 == 0)
		{
			camera_construct_cmd(0x0D,0X00,0X00,0X00,0X00); // ����������ͷ����ͬ��
			
		}
	}

	if(CurrCamera.currst == CAMERA_INIT) //  ͬ��֮��2���ٷ���init��Ϣ
	{
		init_count++;
		if(init_count == 20)
		{
			int ImageQuality;
			ImageQuality = Camera_Get_Pic_IamgeQuality();
			camera_construct_cmd(0x01,0x00,0x07,0x03,ImageQuality);// ����������ͷ���ͳ�ʼ������(320*240)// set pic attr
			init_count = 0;
		}
	}
	else
	{
		init_count = 0;
	}

	if(CurrCamera.currst == CAMERA_WAIT_SET) //  INIT 2s֮���ٷ�������msg ��������
	{
		init_count++;
		if(init_count == 20)
		{
			CurrCamera.currst =CAMERA_SET_MSG_LEN;
			camera_construct_cmd(0x06,0x08,0x00,0x02,0x00);// �������ݰ���С(512byte)
			init_count = 0;
		}
	}
	else
	{
		init_count = 0;
	}

	if(g_set_info.bNewUart4Alarm&(UT_QQCAMERA|UT_XGCAMERA))
	{
		if(CurrCamera.currst == CAMERA_WAIT_GET_PIC)
		{
			CameraSendPicTimer ++;
			if(CameraSendPicTimer%5 == 0)
			{
				
				//SG_CreateSignal(SG_HH_BUF,0, &signal_ptr);		
			}
		}
		else
		{
			CameraSendPicTimer = 0;
		}
	}


#if   __cwt_		
#if (1)
	if((g_state_info.Camera_Up_Flag == 1) && (g_set_info.bNewUart4Alarm& UT_XGCAMERA))
	{
		CameraSendPicTimer ++;
		if(CameraSendPicTimer%2 == 0)
		{
			SG_CreateSignal(SG_CAMERA_UPLOAD,0, &signal_ptr);			
			
		}

	}
#endif /* (0) */
#endif
#endif /* (0) */
	//�������ݻ��߷�����������
	if(DataLedCnt)
	{
		DataLedCnt--;
		cpu_led ^= 0x01;
		GPIO_SetValue(CPU_LED,cpu_led);
		if(DataLedCnt == 0)
			GPIO_SetValue(CPU_LED,0);
	}
	
	//������
	if(persec>=10){
		persec = 0;
		SecTimer++;
		SG_CreateSignal(SG_TIME_OUT,0, &signal_ptr);
	}

	if(PicAckFlag)
	{
		PicAckTime++;
		if(GPIO_GetValue(PIC_ACK)) 
			PicAckHighTime++;
		else 
			PicAckLowTime++;
		SG_CreateSignal(SG_WAITACK,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
	}
	//����
	SG_LED_Status();


	SCI_ChangeTimer(g_RevTmr_ptr, RevTmr_isr_handler, 100);
	SCI_ActiveTimer(g_RevTmr_ptr);	
}


void SG_LED_Status(void)
{
	if(PhoneFlag)				//�绰״̬��һֱ����	
		GPIO_SetValue(CPU_LED,SCI_TRUE);
	else if(LedCnt)				//����	
	{
		if(LedTimer>=3)
		{
			LedTimer = 0;			
			GPIO_SetValue(CPU_LED,0);
			LedCnt--;			
			StatusLedTimer = 0;
		}
		else if(LedTimer>=2)
		{
			GPIO_SetValue(CPU_LED,1);			
		}
	}
	else if(StatusLedTimer>=50)	//״̬��5��һ��
	{
		GPIO_SetValue(CPU_LED,1);
		if(StatusLedTimer>=51)
		{
			StatusLedTimer = 0;
			GPIO_SetValue(CPU_LED,0);
		}
	}

}
