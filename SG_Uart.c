/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2007-2008 ��Ȩ����
  �ļ��� ��SG_Uart.c
  �汾   ��1.50
  ������ ��������
  ����ʱ�䣺  ���� 2009/1/05 
  ����������SG2000  ���ķ��ͺʹ���  
  �޸ļ�¼��

***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SG_GPS.h"
#include "SG_Set.h"
#include "tb_hal.h"
#include "sio.h"
#include "SG_Hh.h"


uint8 BAUD_FLAG = 0;
int checkhandset = 0;
uint32 bandtime = 0xffffffff;
extern SENDFRAME g_gpsframe;


#ifdef _DUART
void SG_Do_Uart2Handle()
{

	if(Log_Enable2 == HH_ENABLE)
	{
		//���ա������ֱ���������
		
		while(SIO_ReadFrame(COM2,Uart_Parse_COM2,UART_READ_BUF,(uint32*)&UartParseCnt_2) == SIO_PPP_SUCCESS)
		{
			//���У��
			if(CheckSum(Uart_Parse_COM2,UartParseCnt_2-1)==Uart_Parse_COM2[UartParseCnt_2-1])
			{
				checkhandset = 1;		
				//������������
				DoUart();
			}
		}			
	}
	else if(Log_Enable2 == LED2_ENABLE)
	{
		if(g_set_info.bNewUart4Alarm & UT_LED2)
		{
			int count = 0;
			while(SIO_ReadFrame(COM2,Uart_Read_COM2+UartReadCnt_2,UART_READ_BUF,(uint32*)&count) > 0)
			{
			
				UartReadCnt_2 += count;
				if(Uart_Read_COM2[UartReadCnt_2-1] == 0x0d)
				{
					Led_COM2_Handle();
					UartReadCnt_2 = 0;
				}
			}	
		}
	}
}

#endif

#ifdef _DUART
void SG_Do_hhHandle(){
	//���ա������ֱ���������
	while(SIO_ReadFrame(COM2,Uart_Parse_COM2,UART_READ_BUF,(uint32*)&UartParseCnt_2) == SIO_PPP_SUCCESS)
	{
		//���У��
		if(CheckSum(Uart_Parse_COM2,UartParseCnt_2-1)==Uart_Parse_COM2[UartParseCnt_2-1])
		{
			checkhandset = 1;
			//������������
			DoUart();
		}
	}

}

#else
void SG_Do_hhHandle(){
	//���ա������ֱ���������
	while(SIO_ReadFrame(COM_DEBUG,Uart_Parse,UART_READ_BUF,(uint32*)&UartParseCnt) == SIO_PPP_SUCCESS)
//	if(Sio_Camera_Read(COM_DATA,Uart_Parse,UART_PARSE_BUF,(uint32*)&UartParseCnt))
	{
		//���У��
		if(CheckSum(Uart_Parse,UartParseCnt-1)==Uart_Parse[UartParseCnt-1])
		{
			checkhandset = 1;
			//������������
			DoUart();
		}
	}

}
#endif

void SG_Do_getDev()
{	//30s�������ж�			
	if(checkhandset == 1)
	{
		#ifdef _DUART
		Log_Enable2 = HH_ENABLE;
		#else
		return;
		#endif
	}
	else if(g_set_info.bRelease2 == LED2_ENABLE)
	{
		#ifdef _DUART
		Log_Enable2 = LED2_ENABLE;
		#endif
	}
		
	if(g_set_info.bRelease1 == DEV_ENABLE)
	{
		Log_Enable1 = DEV_ENABLE;
		
		#ifdef _SUART
		// ������Դ����
		HAL_SetGPIODirection(9,1);				// ����GPIO9 �������
		HAL_SetGPIOVal(9,0);					// ����GPIO9  ��
		#endif
		//��������ʱ
		if(	g_set_info.bNewUart4Alarm&(UT_HBLED|UT_LED|UT_LOCK|UT_TTS|UT_GPS|UT_BUS|UT_DRIVE))// �������LED �����п�����������TTS
		{
			uint32 bd =0;

			bd = SIO_GetBaudRate(COM_DEBUG);
			if(bd != BAUD_9600)
				SIO_SetBaudRate(COM_DEBUG,BAUD_9600);
		}
		else if(g_set_info.bNewUart4Alarm&(UT_PROLOCK))// �ع�����		
		{			
			uint32 bd =0;			
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_115200)				
				SIO_SetBaudRate(COM_DEBUG,BAUD_115200);		
		}
		else if(g_set_info.bNewUart4Alarm&(UT_XGCAMERA|UT_QQCAMERA))// �����������ͷ
		{
			Camera_Init();
		}
		else if(g_set_info.bNewUart4Alarm&UT_WATCH)
		{
			SG_Watch_Init();
		}
		
		SCI_SetAssertMode(0);
		SCI_SetArmLogFlag(0);
	}
	else if(g_set_info.bRelease1 == DEBUG_ENABLE)	//debugģʽ
	{
		uint32 bd =0;
		
		bd = SIO_GetBaudRate(COM_DEBUG);
		if(bd != BAUD_115200)
			SIO_SetBaudRate(COM_DEBUG,BAUD_115200);

		Log_Enable1 = DEBUG_ENABLE;
		SCI_SetAssertMode(1);
		SCI_SetArmLogFlag(1);

#ifdef _SUART
#ifndef _TRACK
		HAL_SetGPIODirection(9,1);				// ����GPIO9 �������
		HAL_SetGPIOVal(9,0);					// ����GPIO9  ��
#endif
#endif

	}
}


void SG_Start_Download()
{
	xSignalHeaderRec      *signal_ptr = NULL;
	char baud1_115200[] = "$PMST09,2,8*19\r\n";
	//char baud1_115200[] = "$PMST09,2,1*10\r\n";
	char baud_save[] = "$PMST09,,*13\r\n";

	int len = 0;
	int i;
	int bd;

#ifdef _DUART
	SIO_SetBaudRate(COM_DATA, BAUD_9600);
#else
	SIO_SetBaudRate(COM_DATA, BAUD_4800);
#endif
#if (1)
	len = strlen(baud1_115200);
	
	for(i=0; i<len; i++)
	{
		HAL_DumpPutChar(COM_DATA, baud1_115200[i]);	
	}

	SCI_Sleep(2);
	len = strlen(baud_save);
	
	for(i=0; i<len; i++)
	{
		HAL_DumpPutChar(COM_DATA, baud_save[i]);	
	}


	SCI_Sleep(2);
#endif /* (0) */
	SIO_WriteFrame(COM_DEBUG,"ok!!",4);
	SCI_Sleep(5);
	bandtime = GetTimeOut(2);

	
}

void SG_Star_UartRead(void)
{

	while(SIO_ReadFrame(COM_DEBUG,Uart_Read_Buf,UART_READ_BUF,(uint32*)&UartReadCnt) > 0)
	{
		if(Uart_Read_Buf[0] == 0x7e && Uart_Read_Buf[1] == '$' && Uart_Read_Buf[2] == 'P')
		{
			if(strncmp((char *)Uart_Read_Buf+1, "$PMST02", 7) == 0)
			{
				g_state_info.GpsDownloadFlag = 1;
				SG_Start_Download();
			}
			return;
		}
	}

}

void SG_Do_Uart0Handle()
{
	switch(Log_Enable1)
	{
		case DEV_ENABLE:
			SG_Ext_Dev_Main();
			break;
		case HH_ENABLE:			
			while(SIO_ReadFrame(COM_DEBUG,Uart_Parse,UART_READ_BUF,(uint32*)&UartParseCnt) == SIO_PPP_SUCCESS)
			{
#ifdef _TRACK
				if(Uart_Parse[0] == '$' && Uart_Parse[1] == 'P')
				{
					if(strncmp((char *)Uart_Parse, "$PMST02", 7) == 0)
					{
						Log_Enable1 = DEV_ENABLE;
						g_state_info.GpsDownloadFlag = 1;
						SG_Start_Download();
					}
					return;
				}
#endif
				//���У��
				if(CheckSum(Uart_Parse,UartParseCnt-1)==Uart_Parse[UartParseCnt-1])
				{
					checkhandset = 1;		
					//������������
					DoUart();
				}
			}
			break;
		case DEBUG_ENABLE:
			#if (__camera_debug_ | __gps_debug_)
			SG_Ext_Dev_Main();
			#endif
			break;
		default:
			break;			
	}
}


void SG_Do_GpsHandle(){
	uint32 len=0;
	int i;
	static gps_data gps={0};

	len=GetGPSData();
	if(len)
	{
		if(g_set_info.GpsPowrFlag == 0)
		{
#ifdef _TRACK
			HAL_SetGPIOVal(PWREN_GPS,1);				
#else
			HAL_SetGPIOVal(PWREN_GPS,0);			
#endif
			return;
		}
		
		if((g_set_info.bNewUart4Alarm & UT_GPS) && (Log_Enable1 == DEV_ENABLE) && (g_set_info.PNDEnable == 0))
		{
			if(strstr((char*)GPSPutBuf,"GGA") || strstr((char*)GPSPutBuf,"RMC") || strstr((char*)GPSPutBuf,"GSV"))
			{
				
				if(g_set_info.GpsDevType == 1)
				{
					g_gpsframe.type = 0xf0;
					g_gpsframe.len = strlen((char *)GPSPutBuf);
					SCI_MEMCPY(g_gpsframe.dat, GPSPutBuf, g_gpsframe.len);
					GpsDevZB_Sendframe(&g_gpsframe);
				}
				else if(g_set_info.GpsDevType == 2)
				{
					SIO_WriteFrame(COM_DEBUG,GPSPutBuf,strlen((char *)GPSPutBuf));
				}
			}		
		}
		if(strstr((char*)GPSPutBuf,"GGA") || strstr((char*)GPSPutBuf,"RMC") )
		{
			SCI_TRACE_LOW("%s",GPSPutBuf);

			//������GGA��RMC�ж�ʱ��һ���������GPS��Ϣ
			if ((SG_GPS_Check(NMEA_GPGGA,(char*) GPSPutBuf) == 1 && SG_GPS_Decode_GGA((char*)GPSPutBuf, &gps) == 1)
				|| (SG_GPS_Check(NMEA_GPRMC,(char*) GPSPutBuf) == 1 && SG_GPS_Decode_RMC((char*)GPSPutBuf, &gps) == 1))
			{				
				//GPS�Ѿ���λʱֻ�㱨������Ϣ
				if (gps.nFull >= 3)
				{
                    //GPS������
					BAUD_FLAG = 1;

					//������λ����
					if(gps.status == 1)
						LedCnt = 1;	

					if((gps.status == 1) && (g_set_info.bNewUart4Alarm & (UT_HBLED|UT_LED2|UT_LED)))
					{
						SG_LED_CorrectTime(&gps);
					}
					
					SG_GPS_Main(&gps);	
				}
			}
			else
			{
				memset(&gps, 0, sizeof(gps_data));
			}
		}

		
 	}

 }


/****************************************************************
  ��������SG_Change_Baud
  ��  ��  ���޸�GPSģ�鲨����,gpsģ������Ӧ
  �����������
  �����������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/10/14
****************************************************************/
void SG_Change_Baud(void)
{
    uint32 bd =0;

	bd = SIO_GetBaudRate(COM_DATA);
	SCI_TRACE_LOW("========= SG_Change_Baud:%x",bd);
	if(bd != BAUD_9600)
	{
		SIO_SetBaudRate(COM_DATA,BAUD_9600);
		SCI_TRACE_LOW("========= BAUD_9600");	
	}
	else
	{
		SIO_SetBaudRate(COM_DATA,BAUD_4800);
		SCI_TRACE_LOW("========= BAUD_4800");
	}
	
}

void Gps_Auto_Adapt(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	if(g_set_info.GpsPowrFlag == 0)
	{
		return;
	}

	if(g_state_info.GpsDownloadFlag == 1)
	{
		if(CheckTimeOut(bandtime) == 1)
		{
			SIO_SetBaudRate(COM_DATA,BAUD_115200);
			SIO_SetBaudRate(COM_DEBUG,BAUD_115200);
			bandtime = 0xffffffff;
		}
		return;
	}

	if((SecTimer%21==0)&&(BAUD_FLAG==0))
	{
		SG_CreateSignal(SG_CHANGE_BAUD,0, &signal_ptr); 
	}

}
