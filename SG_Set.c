/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Set.c
  �汾   ��1.00
  ������ �����ɷ�
  ����ʱ�䣺2005-9-23
  ����������SG2000 ϵͳ������Ϣ���û�����  
  �޸ļ�¼��
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dal_power.h"
#include "SG_GPS.h"
#include "SG_Set.h"
#include "SG_Send.h"
#include "SG_Receive.h"
#include "Msg.h"
#include "os_api.h"
#include "tb_hal.h"
#include "Mmi_descontrol.h"
#include "SG_Crc16.h"

#if 1

#define  PIC_OFF  0xA6
#define  PIC_ON   0xC5

#else

#define  PIC_OFF  0xA9
#define  PIC_ON   0xCA

#endif

//ȫ�����ýṹ
uint8 tryCount = 0;
sg_xih_ctrl g_xih_ctrl;
sg_state_info g_state_info;
sg_set_info g_set_info;
sg_watch_info g_watch_info;
sg_sms_save g_sms_save;

unsigned char Uart_Read_Buf[2048];
unsigned char Uart_Parse[1024];
unsigned char Uart_Snd_Buf[256];
unsigned char TTS_Snd_Buf[1024];

int UartReadCnt ;
int UartParseCnt;

#if _DUART
sg_pro_lock g_pro_lock;
unsigned char Uart_Read_COM2[1024];
unsigned char Uart_Parse_COM2[1024]; //��Ŵ���2���յ�������
unsigned char Uart_Snd_COM2[256];
int UartReadCnt_2 ;
int UartParseCnt_2;//����2���յ������ݳ���
#endif

uint32 SecTimer;		//
uint32 SmsSndTimer; 	//sms���Ͷ�ʱ�� 30s��ʱ
uint32 ReConCnt;		//�ۼ�����GPRS�Ĵ���(�����) ����5����������
uint32 HhTimeCorrectTimer; // �ֱ�Уʱ��ʱʱ��
uint32 ConnectPhoneTimer;  // �Զ������ļ�����
char PhoneFlag;		//ͨ��״̬1: ͨ����0:�Ҷ�
BOOLEAN PhoneAllow;     //������־1:��ֹ0:����
char PictureUpFlag;// 0 not up 1 up
uint32 Pwm_Start_Time;
uint32 Pwm_End_Time;
uint8 GPSPutBuf[GPS_PUT_BUF_SIZE];	
uint32 WRGPSPutBuf,RDGPSPutBuf;
uint32 GPSTimer;
uint32 LedTimer;
uint8  LedCnt;
uint8  DataLedCnt;
int Restart_Reason;
uint32 CameraWatchTimer;
uint32 CameraConditionTimer;
uint32 WatchDevTimer;
uint32 NetSearchTimer;
int NetSearchCount;

int GPRS_RDIndex,GPRS_WRIndex;
int GPRS_RxLen;//gprs����������ܳ���
uint8 GPRS_RxHead;	//0:���մ���Ӧ��1:���հ�ͷ2:��������
int WDT_Timer;
int GPS_Timer;


SCI_TIME_T  g_reboot_time;
SCI_DATE_T	g_reboot_date;

uint32 ResetTimer = 0;	

char PicAckFlag = 0;//1:Ϩ�� 2:�ָ�
char PicAckTime = 0;
char PicSndCnt = 0;
char PicAckLowTime = 0;
char PicAckHighTime = 0;

uint32 Gps_Rev_Timeout;
int Gps_PwrOff_Timeout;

char Sms_Long_Send_Flag;
	
char Log_Enable1;
char Log_Enable2;
IO_STATUS_T g_io_status = {0};

GPRS_T g_gprs_t={0};



/****************************************************************
  ��������SG_Save_New_List_File
  ��  ��  ��������ͨ���Ķ����ļ�
  �����������
  �����������
   ���أ�1 ��ȷ�� 0 ����
  ��д��  ���º���
  �޸ļ�¼��������2007��11��7��
****************************************************************/
int SG_Save_New_List_File(void)
{
	SCI_FILE *fp = NULL;
	sg_send_list *item = NULL;

	
	SCI_TRACE_LOW( "--SG_Save_New_List_File gp:%p,gn:%d\r\n",gp_new_list,gn_new_list);

	if (gn_new_list == 0)
	{
		SCI_TRACE_LOW("--SG_Save_New_List_File: NO Item To Save!!!");
		return 0;
	}
	fp = EFS_fopen(NEW_LIST_FILE_NAME, "wb");
	if (fp == NULL)
	{		
		SCI_TRACE_LOW( "--SG_Save_New_List_File: OPEN FILE FAIL\r\n");	
		return 0;
	}	

	if(g_set_info.EfsFlag == 1)
	{
		SCI_TRACE_LOW("===========efs err!!!");
		EFS_fclose(fp);
		EFS_Delete(NEW_LIST_FILE_NAME);
		SG_Set_Cal_CheckSum();
		g_set_info.EfsFlag = 0;
		SG_Set_Save();
		return 0;
	}
	else
	{
		SG_Set_Cal_CheckSum();
		g_set_info.EfsFlag = 1;
		SG_Set_Save();
	}
	
	
	while(item=SG_Send_Get_New_Item())
	{

		if(item->type != 3)
		{
			SG_Send_Free_Item(&item);
			SCI_Sleep(1);
			continue;
		}

		//��д��ڵ�
		if(EFS_fwrite(item, sizeof(sg_send_list), 1, fp) != 1)
		{
			EFS_fclose(fp);
			SG_Send_Free_Item(&item);
			SCI_TRACE_LOW( "--SG_Save_New_List_File: SAVE FILE FAIL\r\n");	
			return 0;
		}


		
		//д������
		if(EFS_fwrite(item->msg, item->len, 1, fp) != 1)
		{
			EFS_fclose(fp);
			SG_Send_Free_Item(&item);
			SCI_TRACE_LOW( "--SG_Save_New_List_File: SAVE2 FILE FAIL\r\n");	
			return 0;
		}

		//�ͷŽڵ�
		SG_Send_Free_Item(&item);
		SCI_Sleep(1);
	}

	EFS_fclose(fp);

	SG_Set_Cal_CheckSum();
	g_set_info.EfsFlag = 0;
	SG_Set_Save();

	SCI_TRACE_LOW( "--SG_Save_New_List_File gp:%p,gn:%d\r\n",gp_new_list,gn_new_list);

	return 1;

}

/****************************************************************
  ��������SG_Get_New_List_File
  ��  ��  ����ȡ��ͨ���Ķ����ļ�
  �����������
  �����������
  ���أ�1 ��ȷ�� 0 ����
  ��д��  ���º���
  �޸ļ�¼��������2007��11��7��
****************************************************************/
int SG_Get_New_List_File(void)
{
	SCI_FILE *fp = NULL;
	sg_send_list *item = NULL;
	
	fp = EFS_fopen(NEW_LIST_FILE_NAME, "rb");
	if (fp == NULL)
	{		
		SCI_TRACE_LOW( "--SG_Get_New_List_File: OPEN FILE FAIL\r\n");	
		return 0;
	}	

	if(g_set_info.EfsFlag == 1)
	{
		SCI_TRACE_LOW("=========read efs err!!!");
		EFS_fclose(fp);
		EFS_Delete(NEW_LIST_FILE_NAME);
		SG_Set_Cal_CheckSum();
		g_set_info.EfsFlag = 0;
		SG_Set_Save();
		return 0;
	}
	else
	{
		SG_Set_Cal_CheckSum();
		g_set_info.EfsFlag = 1;
		SG_Set_Save();
	}
	

	item = SCI_ALLOC(sizeof(sg_send_list));
	if(item == NULL)
	{
		SCI_TRACE_LOW( "--SG_Get_New_List_File: OPEN FILE FAIL\r\n");	
		return 0;
	}

	while(EFS_feof(fp)==0)
	{
		if (EFS_fread(item, sizeof(sg_send_list), 1, fp) != 1)
		{
			SCI_TRACE_LOW( "--SG_Get_New_List_File: read FILE FAIL\r\n");
			goto err;
		}

		if(item->len > 100)
		{
			SCI_TRACE_LOW( "--SG_Get_New_List_File: %d\r\n",item->len);
			goto err;
		}
		item->msg = NULL;
		item->msg = SCI_ALLOC(item->len);
		if(item->msg == NULL)
		{
			SCI_TRACE_LOW( "--SG_Get_New_List_File: calloc err\r\n");	
			goto err;
		}

		
		if (EFS_fread(item->msg, item->len, 1, fp) != 1)
		{
			SCI_TRACE_LOW( "--SG_Get_New_List_File: read2 FILE FAIL\r\n");	
			SCI_FREE(item->msg);
			goto err;
		}

		if(SG_Send_Put_New_Item(item->type,item->msg,item->len)!= 1)
		{
			SCI_TRACE_LOW( "--SG_Get_New_List_File: put item FAIL\r\n");
			//SCI_FREE(item->msg);
			goto err;
		}

		SCI_Sleep(1);
		
	}

	SCI_TRACE_LOW( "==SG_Get_New_List_File: %p ,%d\r\n",gp_new_list,gn_new_list);	

	SCI_FREE(item);
	EFS_fclose(fp);

	EFS_Delete(NEW_LIST_FILE_NAME);
	SG_Set_Cal_CheckSum();
	g_set_info.EfsFlag = 0;
	SG_Set_Save();
	return 1;
err:
	SCI_FREE(item);
	EFS_fclose(fp);
	EFS_Delete(NEW_LIST_FILE_NAME);
	return 0;
		
}

/****************************************************************
  ��������SG_Save_Alarm_List_File
  ��  ��  �����汨�����Ķ����ļ�
  �����������
  �����������
   ���أ�1 ��ȷ�� 0 ����
  ��д��  ���º���
  �޸ļ�¼��������2007��11��7��
****************************************************************/
int SG_Save_Alarm_List_File(void)
{

	SCI_FILE *fp = NULL;
	sg_send_list *item = NULL;

	
	SCI_TRACE_LOW( "--SG_Save_Alarm_List_File gp:%p,gn:%d\r\n",gp_alarm_list,gn_alarm_list);

	if (gn_alarm_list == 0)
	{
		SCI_TRACE_LOW("--SG_Save_Alarm_List_File: NO Item To Save!!!");
		return 0;
	}
	
	fp = EFS_fopen(ALARM_LIST_FILE_NAME, "wb");
	if (fp == NULL)
	{		
		SCI_TRACE_LOW( "--SG_Save_Alarm_List_File: OPEN FILE FAIL\r\n");	
		return 0;
	}	

	if(g_set_info.EfsFlag == 1)
	{
		SCI_TRACE_LOW("===========efs err!!!");
		EFS_fclose(fp);
		EFS_Delete(ALARM_LIST_FILE_NAME);
		SG_Set_Cal_CheckSum();
		g_set_info.EfsFlag = 0;
		SG_Set_Save();
		return 0;
	}
	else
	{
		SG_Set_Cal_CheckSum();
		g_set_info.EfsFlag = 1;
		SG_Set_Save();
	}
	
	
	while(item=SG_Send_Get_Item(&gp_alarm_list, &gn_alarm_list))
	{

		SCI_TRACE_LOW( "SG_Send_Get_Alarm_Item: MSG LEN: %d, NEW LIST NO: %d\r\n", item->len, gn_alarm_list);
		if(item->type != 4)
		{
			SG_Send_Free_Item(&item);
			SCI_Sleep(1);
			continue;
		}

		//��д��ڵ�
		if(EFS_fwrite(item, sizeof(sg_send_list), 1, fp) != 1)
		{
			EFS_fclose(fp);
			SCI_TRACE_LOW( "--SG_Save_Alarm_List_File: SAVE FILE FAIL\r\n");	
			return 0;
		}

		//д������
		if(EFS_fwrite(item->msg, item->len, 1, fp) != 1)
		{
			EFS_fclose(fp);
			SCI_TRACE_LOW( "--SG_Save_Alarm_List_File: SAVE2 FILE FAIL\r\n");	
			return 0;
		}
		
		//�ͷŽڵ�
		SG_Send_Free_Item(&item);
		SCI_Sleep(1);
	}

	EFS_fclose(fp);

	SG_Set_Cal_CheckSum();
	g_set_info.EfsFlag = 0;
	SG_Set_Save();

	SCI_TRACE_LOW( "--SG_Save_Alarm_List_File gp:%p,gn:%d\r\n",gp_alarm_list,gn_alarm_list);

	return 1;


}

/****************************************************************
  ��������SG_Get_Alarm_List_File
  ��  ��  ����ȡ�������Ķ����ļ�
  �����������
  �����������
  ���أ�1 ��ȷ�� 0 ����
  ��д��  ���º���
  �޸ļ�¼��������2007��11��7��
****************************************************************/
int SG_Get_Alarm_List_File(void)
{
	SCI_FILE *fp = NULL;
	sg_send_list *item = NULL;
	
	fp = EFS_fopen(ALARM_LIST_FILE_NAME, "rb");
	if (fp == NULL)
	{		
		SCI_TRACE_LOW( "--SG_Get_Alarm_List_File: OPEN FILE FAIL\r\n");	
		return 0;
	}	

	if(g_set_info.EfsFlag == 1)
	{
		SCI_TRACE_LOW("=========read efs err!!!");
		EFS_fclose(fp);
		EFS_Delete(ALARM_LIST_FILE_NAME);
		SG_Set_Cal_CheckSum();
		g_set_info.EfsFlag = 0;
		SG_Set_Save();
		return 0;
	}
	else
	{
		SG_Set_Cal_CheckSum();
		g_set_info.EfsFlag = 1;
		SG_Set_Save();
	}
	

	item = SCI_ALLOC(sizeof(sg_send_list));
	if(item == NULL)
	{
		return 0;
	}
	

	while(EFS_feof(fp)==0)
	{
		if (EFS_fread(item, sizeof(sg_send_list), 1, fp) != 1)
		{
			SCI_TRACE_LOW( "--SG_Get_Alarm_List_File: read FILE FAIL\r\n");
			goto err;
		}

		if(item->len > 100)
		{
			SCI_TRACE_LOW( "--SG_Get_Alarm_List_File: %d\r\n",item->len);
			goto err;
		}
		
		item->msg = NULL;
		item->msg = SCI_ALLOC(item->len);
		if(item->msg == NULL)
		{
			SCI_TRACE_LOW( "--SG_Get_Alarm_List_File: calloc err\r\n");	
			goto err;
		}

		
		if (EFS_fread(item->msg, item->len, 1, fp) != 1)
		{
			SCI_TRACE_LOW( "--SG_Get_Alarm_List_File: read2 FILE FAIL\r\n");	
			SCI_FREE(item->msg);
			goto err;
		}

		if(SG_Send_Put_Item(item->type,item->msg,item->len, &gp_alarm_list, &gn_alarm_list) != 1)
		{
			SCI_TRACE_LOW( "--SG_Get_Alarm_List_File: put item FAIL\r\n");
			SCI_FREE(item->msg);
			goto err;
		}
		else
		{
			SCI_TRACE_LOW( "SG_Send_Put_Alarm_Item: MSG LEN: %d, NEW LIST NO: %d", item->len, gn_alarm_list);
		}
		
		SCI_Sleep(1);
		
	}

	SCI_TRACE_LOW( "==SG_Get_New_List_File: %p ,%d\r\n",gp_alarm_list,gn_alarm_list);	

	SCI_FREE(item);
	EFS_fclose(fp);

	EFS_Delete(ALARM_LIST_FILE_NAME);
	SG_Set_Cal_CheckSum();
	g_set_info.EfsFlag = 0;
	SG_Set_Save();
	return 1;
err:
	SCI_FREE(item);
	EFS_fclose(fp);
	EFS_Delete(ALARM_LIST_FILE_NAME);
	return 0;

}



void SG_Set_Reset(void)
{
	SG_Set_Default();
	EFS_Delete(NEW_LIST_FILE_NAME);
	EFS_Delete(ALARM_LIST_FILE_NAME);
	EFS_Delete(REGION_FILE_PATH);
	EFS_Delete(SG_SET_LINE_FILE);
	EFS_Delete(CONSULT_FILE_PATH);
	EFS_Delete(CALLIN_FILE_FILE);
	EFS_Delete(CALLOUT_FILE_FILE);
	
	SCI_SetAssertMode(0);//release_mode
	ResetTimer = GetTimeOut(2);
}



/****************************************************************
  ��������SG_Set_Get_CheckSum
  ��  ��  ������g_set_info��У����Ƿ���ȷ������ȷ���nv�ж�ȡ�ɾ��������ļ�
  �����������
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
void SG_Set_Cal_CheckSum(void)
{
	int ret;
	
	if(TRUE == SG_Set_Check_CheckSum())
	{
		return;
	}
	else
	{
		SCI_TRACE_LOW("----------SG_Set_Cal_CheckSum: Checksum error");
		memset(&g_set_info,0,sizeof(g_set_info));
		ret = MNNV_ReadItem(MMINV_GSET_INFO,sizeof(g_set_info),&g_set_info);
	  	if(ret != ERR_MNDATAMAG_NO_ERR)
	  	{
			SCI_TRACE_LOW( "----------SG_Set_Cal_CheckSum: MNNV_ReadItem error  ret %d",ret);
			SG_Set_Default();
			return;
	  	}
	}
	
}




/****************************************************************
  ��������SG_Set_Get_CheckSum
  ��  ��  ������Ϊg_set_infoд��У���
  �����������
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
void SG_Set_Get_CheckSum(void)
{
	int len;
	unsigned char buf[1100] = "";
	
	len = sizeof(g_set_info)-2;

	memcpy(buf,(char*)&g_set_info,sizeof(g_set_info));

	g_set_info.checksum = Msg_Get_CRC16(buf,len);
}



/****************************************************************
  ��������SG_Set_Check_CheckSum
  ��  ��  ��������õĽṹ��g_set_info��У���
  �����������
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
BOOLEAN SG_Set_Check_CheckSum(void)
{
	unsigned char buf[1100] = "";
	unsigned checksum = 0;
	int len;

	
	len = sizeof(g_set_info)-2;
	memcpy(buf,(char*)&g_set_info,sizeof(g_set_info));

	checksum = Msg_Get_CRC16(buf,len);
	if(checksum == g_set_info.checksum)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/****************************************************************
  ��������SG_Set_Default
  ��  ��  ������Ĭ������
  �����������
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ���Ӹ�ָ�������ͷţ����ɷ壬2005-12-06
****************************************************************/
void SG_Set_Default(void)
{
	SCI_MEMSET(&g_set_info,0,sizeof(g_set_info));
	g_set_info.nOverSpeed = SG_DEFAULT_OVERSPEED;
	g_set_info.nWatchType = MSG_WATCH_CANCLE;
	g_set_info.alarmEnable = GPS_CAR_STATU_HIJACK| GPS_CAR_STATU_POWER_OFF  ;
	g_set_info.ReserveAlarmEnable = GPS_CAR_STATU_HIJACK| GPS_CAR_STATU_POWER_OFF  ;
	g_set_info.nWatchType = MSG_WATCH_CANCLE;
	g_set_info.nGpstTime = 10;
	g_set_info.nGpstDist = 50;
	g_set_info.EfsMax = 200;
	g_set_info.bStopReport = 1;
	g_set_info.watchbd = 9600;
	g_set_info.nSetTime=2;
	g_set_info.alarmSet |= GPS_CAR_STATU_IO1;
	g_set_info.bBatteryCheckEnable = 1; // Ĭ��ʹ�õ��籨��
	g_set_info.sg_camera[0].setCount = 1;
	g_set_info.sg_camera[0].nInterval = 0xffff;
	if(g_set_info.nSmsInterval < DEFAULT_SMS_INTERVAL)
		g_set_info.nSmsInterval = DEFAULT_SMS_INTERVAL;
	if(g_set_info.nSmsInterval_data < DEFAULT_SMS_INTERVAL)
		g_set_info.nSmsInterval_data = DEFAULT_SMS_INTERVAL;
	strcpy((char*)g_set_info.sSuperPsw,"46387827"); // ���ó�������
	strcpy((char*)g_set_info.sUserPsw,"888888"); // �����ֻ�������������
	strcpy((char*)g_set_info.sChangePsw,"387827"); // �ֻ��������ð���
	strcpy((char*)g_set_info.sSafetyPsw,"888888"); 
	strcpy(g_set_info.sVersion,SG2000_VER);
	MNNV_ReadItem(728,sizeof(g_state_info.hardware_ver),&g_state_info.hardware_ver);

	if(g_state_info.hardware_ver == 0x153)
		strcat(g_set_info.sVersion,"IN 6416 module");
	else
		strcat(g_set_info.sVersion,"IN 3208 module");

	SCI_TRACE_LOW("@_@--<<<<<<CURRENT VER:%s ,len %d",g_set_info.sVersion,strlen(g_set_info.sVersion));

//#ifdef _DUART
	g_set_info.bAutoAnswerDisable = 1; // Ĭ��ʹ���ֱ�ͨ�� G01B G01C ����ʹ���Զ����� 2010.12.01 cojone
//#else
//	g_set_info.bAutoAnswerDisable = 0;     // Ĭ��ʹ������ͨ��
//#endif
	g_set_info.speaker_vol = 7;	
	g_set_info.Heart_Interval = DEFAULT_HEART_INTERVAL;
	g_set_info.Heart_Switch = 1;
	g_set_info.GpsPowrFlag = 1;
	SG_Set_Save();
}

/****************************************************************
  ��������SG_Set_Save
  ��  ��  ���������õ��ļ�
  �����������
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ����·�ߵȽṹ�ȴ泤���ٴ����������ļ������ɷ壬2005-12-07  
  ���ӱ����ļ����棬���ɷ壬2005-12-18
  �����ļ�ɾ������������ֵ�жϣ����ɷ壬2005-12-20  
****************************************************************/
int SG_Set_Save(void)
{
	int ret;

	// ���»�ȡchecksumд��g_set_info
	SG_Set_Get_CheckSum();

	// ����д��NV����
	ret = MNNV_WriteItem(MMINV_GSET_INFO,sizeof(g_set_info),&g_set_info);
	
 	if(ret != ERR_MNDATAMAG_NO_ERR)
  	{		
		SCI_TRACE_LOW( "--SG_Set_Save:  MNNV_WriteItem error ret %d",ret);		
		return 0;
	}

	SCI_TRACE_LOW( "SG_Set_Save \r\n");

	return 1;
}

/****************************************************************
  ��������SG_Watch_Save
  ��  ��  ��������̵��ļ�
  �����������
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ����·�ߵȽṹ�ȴ泤���ٴ����������ļ������ɷ壬2005-12-07  
  ���ӱ����ļ����棬���ɷ壬2005-12-18
  �����ļ�ɾ������������ֵ�жϣ����ɷ壬2005-12-20  
****************************************************************/
int SG_Watch_Save(void)
{
	int ret;


	// ����д��NV����
	ret = MNNV_WriteItem(MMINV_DIST_INFO,sizeof(g_watch_info),&g_watch_info);
	
 	if(ret != ERR_MNDATAMAG_NO_ERR)
  	{		
		SCI_TRACE_LOW( "--SG_Watch_Save:  MNNV_WriteItem error ret %d",ret);		
		return 0;
	}

	SCI_TRACE_LOW( "SG_Watch_Save \r\n");

	return 1;
}

void SG_Soft_Reset(int delay)
{	
	gps_data *gps= (gps_data *)g_state_info.pGpsFull;
	int tasknum = 0;
	static int flag = 0;

	if(flag == 1)
		return;
	else
		flag = 1;
	
	g_watch_info.nGetTotalDistance = g_state_info.nGetTotalDistance;
	g_watch_info.nGetDistance[0] = g_state_info.nGetDistance[0];
	g_watch_info.nGetDistance[1] = g_state_info.nGetDistance[1];
	g_watch_info.AccOnTime = g_state_info.AccOnTime;
	SCI_MEMCPY(g_watch_info.sLatitude, gps->sLatitude, SG_GPS_LAT_LEN);
	SCI_MEMCPY(g_watch_info.sLongitude, gps->sLongitude, SG_GPS_LAT_LEN);
	SG_Watch_Save();
	//�ر�GPS
	g_state_info.GpsEnable = 1; 
	SG_Save_New_List_File();
	SG_Save_Alarm_List_File();
	
	SG_Set_Cal_CheckSum();
	tasknum = camera_task_count();
	if(g_set_info.nCameraAlarm & 0x01) // ʵʱ���
	{
		g_set_info.sg_camera[0].nCount += tasknum;
	}
	else
	{
		g_set_info.sg_camera[1].nCount += tasknum;
	}
	g_set_info.nReConCnt++;
	//g_set_info.upAD = g_state_info.upAD;
	//g_set_info.upOilQuantity = g_state_info.upOilQuantity;
	
	g_set_info.drvTime = g_state_info.drvTime;
	g_set_info.drvstopTime = g_state_info.drvstopTime;
	g_set_info.stopTime = g_state_info.stopTime;
	g_set_info.lastCheckTime = g_state_info.lastCheckTime;
	g_set_info.SmsSndTimer = SmsSndTimer;
	SG_Set_Save();		
	if(delay > DEFAULT_CHECK_WATCHDOG)
		delay = DEFAULT_CHECK_WATCHDOG-2;
	
	SCI_SetAssertMode(0);//release_mode
	ResetTimer = GetTimeOut(delay);
}

void sg_set_cfg_init(uint8 type)
{
	SCI_FILE *pf = NULL;
	unsigned char *dst = NULL;
	int dstlen = 0;

	switch(type){
		case REGION:
			pf = EFS_fopen(REGION_FILE_PATH,"rb");
			break;		
		case LINE:
			pf = EFS_fopen(SG_SET_LINE_FILE,"r");
			break;		
		case MUSTER:
//			pf = EFS_fopen(MUSTER_FILE_PATH,"rb");
			break;	
		case CONSULT:
			pf = EFS_fopen(CONSULT_FILE_PATH,"r");
			break;
		case CALLIN:
			pf = EFS_fopen(CALLIN_FILE_FILE,"r");
			break;
		case CALLOUT:
			pf = EFS_fopen(CALLOUT_FILE_FILE,"r");
			break;
		default:
			break;
	}
	
	if(pf == NULL){
		SCI_TRACE_LOW("@_@ sg_set_cfg_init: file open fail %d\n",type);
		return ;
	}
		
	if (EFS_fread(&dstlen, 4, 1, pf) != 1)
	{
		SCI_TRACE_LOW( "--SG_Set_Init: READ len\r\n");		
		goto errcase; 	
	}

	SCI_TRACE_LOW("--SG_Set_Init: dstlen=%d",dstlen);

	if(dstlen <= 0){
		SCI_TRACE_LOW( "--SG_Set_Init: READ len\r\n");		
		goto errcase; 	
	}	

	dst = SCI_ALLOC(dstlen+4);
	if(dst ==NULL){
		SCI_TRACE_LOW( "--SG_Set_Init: READ dst\r\n");
		goto errcase;
	}
	SCI_MEMSET(dst,0,dstlen+4)

	if(EFS_fread(&dst[4], dstlen, 1, pf) != 1){
		SCI_TRACE_LOW("@_@ sg_set_cfg_init: read err\n");
		goto errcase;
	}

	SCI_MEMCPY(dst,(uint8*)&dstlen,4);

	switch(type){
		case REGION:
			Renew_Region_Tab(&dst[4], dstlen);
			g_state_info.pRegion = (char*)dst;
			break;		
		case LINE:
			renew_line_tab(&dst[4], dstlen);
			g_state_info.pLine = (char*)dst;
			break;		
		case MUSTER:
//			Renew_Interest_Piont_Tab(&dst[4], dstlen);
//			g_set_info.pInfoMuster = (char*)dst;
			break;	
		case CONSULT:
			renew_consult_tab(&dst[4],dstlen);
			SCI_TRACE_LOW( "----------renew_consult_tab end\r\n");		
			break;
		case CALLIN:
			SCI_MEMSET(g_state_info.sAllowAnswer, 0, SG_CALLNUM_SIZE);
			SCI_MEMCPY(g_state_info.sAllowAnswer, &dst[4], dstlen);
			break;
		case CALLOUT:
			SCI_MEMSET(g_state_info.sAllowCall, 0, SG_CALLNUM_SIZE);
			SCI_MEMCPY(g_state_info.sAllowCall, &dst[4], dstlen);
			break;
		
		default:
			break;
		}	

errcase:
	if(pf)
		EFS_fclose(pf);
	return ;	
}

void SG_Read_HW_Ver(void)
{
	int ret = 0;

	ret = MNNV_ReadItem(728,sizeof(g_state_info.hardware_ver),&g_state_info.hardware_ver);
  	if(ret != ERR_MNDATAMAG_NO_ERR)
  	{
		SCI_TRACE_LOW( "--------------SG_Set_Init: MNNV_ReadItem error  ret %d",ret);
  	}
	SCI_TRACE_LOW("The Hardware Ver is %x",g_state_info.hardware_ver);

	ret = MNNV_ReadItem(537,sizeof(g_state_info.user_type),&g_state_info.user_type);
  	if(ret != ERR_MNDATAMAG_NO_ERR)
  	{
		SCI_TRACE_LOW( "--------------SG_Set_Init: MNNV_ReadItem error  ret %d",ret);
  	}
	SCI_TRACE_LOW("The Usertype is %02x",g_state_info.user_type);
	

}


/****************************************************************
  ������SG_State_Init
  ���ܣ�������Ҫ��ʼ������
  ���룺�ޣ���ȫ�ֱ���
  �������
  ���أ���
  ��д���º���
  ��¼��������2007-8-28
 
****************************************************************/
void SG_State_Init(void)
{

	g_state_info.alarmState |= g_set_info.alarmState;
	g_state_info.screen = 0xff;
	g_state_info.loginID = -1;
	g_state_info.logoutID = -1;
	g_state_info.socket = -1;
	g_state_info.bRelease1 = g_set_info.bRelease1;
	g_state_info.bRelease2 = g_set_info.bRelease2;
	g_state_info.bNewUart4Alarm = g_set_info.bNewUart4Alarm;
	g_state_info.nGetTotalDistance = g_watch_info.nGetTotalDistance;
	g_state_info.nGetDistance[0] = g_watch_info.nGetDistance[0];
	g_state_info.nGetDistance[1] = g_watch_info.nGetDistance[1];
	g_state_info.AccOnTime = g_watch_info.AccOnTime ;

	//g_state_info.upAD =	g_set_info.upAD;
	//g_state_info.upOilQuantity = g_set_info.upOilQuantity;

	g_state_info.drvTime = g_set_info.drvTime;
	g_state_info.drvstopTime = g_set_info.drvstopTime;
	g_state_info.stopTime = g_set_info.stopTime;
	g_state_info.lastCheckTime = g_set_info.lastCheckTime;
	g_state_info.GpsDevType = g_set_info.GpsDevType;
	if(g_set_info.bnewRegion > 0){
		SG_Set_Cal_CheckSum();
		//��ֹ����๦�������save����λ���Ѿ��������� 
		g_set_info.alarmEnable = g_set_info.ReserveAlarmEnable;
		g_state_info.alarmState &= g_set_info.ReserveAlarmEnable|GPS_CAR_STATU_POS|GPS_CAR_STATU_GAS_ALARM;
		SG_Set_Save();
	}
	
	SCI_TRACE_LOW("<<<<<g_state_info.alarmState = %x g_set_info.as %x",g_state_info.alarmState,g_set_info.alarmState);

	g_state_info.randtime = (g_set_info.sOwnNo[9]-'0')*10+ (g_set_info.sOwnNo[10]-'0'); //�����
	if(g_state_info.randtime < 0)
		g_state_info.randtime = 0;

	g_state_info.plmn_status = 0;
	SG_Read_HW_Ver();
}
 
int SG_Set_Init(void)
{
	int ret;

	//���ļ���ȡ�ָ�������Ϣ

	SCI_MEMSET(&g_watch_info,0,sizeof(g_watch_info));
	SCI_MEMSET(&g_set_info,0,sizeof(g_set_info));

	SCI_TRACE_LOW("----SG_Set_Init:nv len  watch info: %d set info %d",sizeof(g_watch_info),sizeof(g_set_info));
	ret = MNNV_ReadItem(MMINV_GSET_INFO,sizeof(g_set_info),&g_set_info);
  	if(ret != ERR_MNDATAMAG_NO_ERR)
  	{
		SCI_TRACE_LOW( "--------------SG_Set_Init: MNNV_ReadItem error  ret %d",ret);
		SG_Set_Default();
		return 0;
  	}
	
#if (0)
	{
		int i;
		char *ptr = (char*)&g_set_info;
		for(i=0;i<500;i++)
		{
			SCI_TRACE_LOW("%02x",*(ptr+i));
		}
  	}
	SCI_TRACE_LOW(" ci %s ,cp %s ,apn %s ,pi %s,pp %d,cp %d ,own no %s nt %d pe %d,wt %d",g_set_info.sCenterIp,g_set_info.sCenterNo
		,g_set_info.sAPN,g_set_info.sProxyIp,g_set_info.nProxyPort,g_set_info.nCenterPort
		,g_set_info.sOwnNo,g_set_info.nNetType,g_set_info.bProxyEnable,g_set_info.nWatchType);
//	while(1);
#endif /* (0) */

	if(TRUE != SG_Set_Check_CheckSum())
	{
		SCI_TRACE_LOW( "--------------SG_Set_Init:  g_set_info check sum ERROR\r\n");		
		return 0;
	}
	
	ret = MNNV_ReadItem(MMINV_DIST_INFO,sizeof(g_watch_info),&g_watch_info);
	

	if(g_set_info.bRegionEnable){
		sg_set_cfg_init(REGION);
	}
	
	if(g_set_info.bLineEnable){
		sg_set_cfg_init(LINE);
	}

	if(g_set_info.bCallInDisable){
		sg_set_cfg_init(CALLIN);
	}

	if(g_set_info.bCallOutDisable){
		sg_set_cfg_init(CALLOUT);
	}
		
	sg_set_cfg_init(CONSULT);

	SG_Get_New_List_File();
	SG_Get_Alarm_List_File();

	return 1;
		  
}


/****************************************************************
  ������SG_Set_Cfg_Save
  ���ܣ����ݴ����ļ�
  ���룺type �������ͣ�para ����ָ�룬len ���ݳ���
  �������
  ���أ�0��ȷ��1����
  ��д���º���
  ��¼��������2007-8-28
 
****************************************************************/
int SG_Set_Cfg_Save(uint8 type,unsigned char *para,int len)
{
	SCI_FILE *fp = NULL;
//	unsigned char *dst = NULL;

	switch(type)
	{
		case REGION:	//����
			fp = EFS_fopen(REGION_FILE_PATH,"wb");
			break;		
		case LINE:		//·��
			fp = EFS_fopen(SG_SET_LINE_FILE,"wb");
			break;		
		case MUSTER:	//��Ȥ��
//			fp = EFS_fopen(MUSTER_FILE_PATH,"wb");
			break;	
		case CONSULT:
			fp = EFS_fopen(CONSULT_FILE_PATH,"wb");
			break;
		case CALLIN:
			fp = EFS_fopen(CALLIN_FILE_FILE,"wb");
			break;
		case CALLOUT:
			fp = EFS_fopen(CALLOUT_FILE_FILE,"wb");
			break;
			
		default:
			break;
	}

	if(fp == NULL)
		return -1;

	//д����ʱֹͣ����GPS�ź�
	g_state_info.GpsEnable = 1;
	EFS_fwrite(para,len,1,fp);	

	EFS_fclose(fp);
	g_state_info.GpsEnable = 0;
	SCI_TRACE_LOW("@_@ sg_set_cfg_save: len %d\r\n",len);
	SCI_TRACE_LOW("@_@ sg_set_cfg_save: save Ok\n");

	return 0;
}


/****************************************************************
  ������Pic_Send
  ���ܣ��������������б�
  ���룺data:0xA6 Ϩ��0xC5 �ָ�
  �������
  ���أ���
  ��д��chenhh
  ��¼��������2007-7-16
****************************************************************/
int Pic_Receive_Ack(void)
{

	if(PicAckFlag )
	{
		if(tryCount)
		{
			if(PicAckTime > 50)
			{
		//		test_wdg =1;	//ι��
				tryCount--;
				PicAckTime=0;
				if(tryCount)
				{
					if(PicAckFlag == 1)
					{
						Pic_Send(0xA6);
					}
					else if(PicAckFlag == 2)
					{
						Pic_Send(0xC5);
					}
				}
				
			}
			else if((PicAckLowTime>=6)&&(PicAckHighTime>=6))	//0.6s
			{
				tryCount=0;
				if (1 == PicAckFlag)
				{
					//g_state_info.alarmState |= GPS_CAR_STATU_GAS_ALARM;
					GPIO_SetValue(CPU_XIHUO_OUT,TRUE);
#ifdef _TRACK
					if(g_set_info.ddjdqFlag == 1 && g_xih_ctrl.XIH_State == SG_MENU_CTRL)
					{
						SG_Set_Cal_CheckSum();
						g_set_info.jdqFlag = 1;
						SG_Set_Save();
					}
#endif
				}
				else if(2 == PicAckFlag)
				{
					GPIO_SetValue(CPU_XIHUO_OUT,FALSE);
					//g_state_info.alarmState &= ~GPS_CAR_STATU_GAS_ALARM;
				}	
//				PicAckFlag = 0;
				PicAckTime = 0;
				PicAckLowTime=0;
				PicAckHighTime=0;
				HAL_SetGPIODirection(PIC_ACK ,SCI_TRUE ); 	// ����CLK/ACK���ŷ���Ϊ���	
				return 1;
			}
			else if((PicAckLowTime==0x00)&&(PicAckHighTime)) 
			{
				PicAckHighTime=0;
			}
		}
		else
		{
			PicAckFlag = 0;
			PicAckTime = 0;
			PicAckLowTime =0;
			PicAckHighTime=0;
			return 2;
			
		}
	}	
	return 0;

	
}

/****************************************************************
  ������Pic_Send
  ���ܣ��������������б�
  ���룺data:0xA6 Ϩ��0xC5 �ָ�
  �������
  ���أ���
  ��д��chenhh
  ��¼��������2007-7-16
****************************************************************/
void Pic_Send (char data)
{
	int i,j;	
//	int us = 0;

	PicAckFlag = 0;

	SCI_TRACE_LOW("<<<<<<Pic_Send<<<<<<%02x",data);

	// ������
	if(data != 0xA6 && data != 0xC5)
	{
		SCI_TRACE_LOW("The Command is a wrong CMD %02x",data);
		return;
	}

	if(data == 0xA6)
		data = PIC_OFF;
	else if(data == 0xC5)
		data = PIC_ON;	
	
	// �� CLK/ACK��TXD ��������Ϊ�������������ݸ�	
	HAL_SetGPIODirection(PIC_ACK ,SCI_TRUE ); 	// ����CLK/ACK���ŷ���Ϊ���	
	HAL_SetGPIOVal(PIC_ACK ,SCI_TRUE);// ʱ������ߵ�ƽ	
	HAL_SetGPIOVal(PIC_TXD ,SCI_TRUE);	// ��TXD���Ÿߵ�ƽ(1)		

	// ��ʱ10ms=10000*0.9/78
	for(j=0;i<12000;i++);

	// ���������data�Ӹߵ��͵ĸ�λ: 0xA6 :�ر���·���� 0xC5 ����·����	
	for( i=0; i<8; i++ ) 
	{		
		if( data &(0x80>>i) ) 
		{         			
			HAL_SetGPIOVal(PIC_TXD ,SCI_TRUE);	// ��TXD���Ÿߵ�ƽ(1)		
		}		
		else 
		{			
			HAL_SetGPIOVal(PIC_TXD ,SCI_FALSE);	// ��TXD���ŵ͵�ƽ(0)		
		}		
		
		// ��ʱ1ms
		for(j=0;j<120;j++);		
		HAL_SetGPIOVal(PIC_ACK ,SCI_FALSE); // ʱ������͵�ƽ				
		// ��ʱ10ms
		for(j=0;j<12000;j++);		
		HAL_SetGPIOVal(PIC_ACK ,SCI_TRUE);// ʱ������ߵ�ƽ	
		// ��ʱ10ms
		for(j=0;j<12000;j++);
		
	}	

	// ��ʱ10ms
	for(j=0;j<12000;j++);
	HAL_SetGPIODirection(PIC_ACK ,SCI_FALSE); 	// ����CLK/ACK���ŷ���Ϊ���룬�ȴ�Ӧ��	
	HAL_SetGPIOVal(PIC_TXD ,SCI_TRUE);	// ��TXD���Ÿߵ�ƽ(1)		

	if(data == PIC_OFF)
		PicAckFlag =1;
	else 
	{
		PicAckFlag =2;
#ifdef _TRACK
		if((g_set_info.ddjdqFlag == 1) && (g_xih_ctrl.XIH_State == SG_MENU_CTRL))
		{
			SG_Set_Cal_CheckSum();
			g_set_info.jdqFlag = 0;
			SG_Set_Save();
		}
#endif
	}

	PicAckHighTime =0;
	PicAckLowTime =0;
	
}

#ifdef _TRACK
void DDjdq_Handle(void)
{
	
	if((g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF) )
	{
		gps_data *gps= (gps_data *)g_state_info.pGpsFull;

		//������������ǰ״̬
		if((SecTimer == 15) && (g_set_info.jdqFlag == 1))
		{
			g_xih_ctrl.XIH_State = SG_MENU_CTRL;
			tryCount = 3;
			Pic_Send(0xa6);
		}
		else if((g_set_info.jdqFlag == 0)&&(SecTimer >= 20)&&(SecTimer%9 == 0)&&(atoi(gps->sSpeed) < 11))
		{
			g_xih_ctrl.XIH_State = SG_MENU_CTRL;
			tryCount = 3;
			Pic_Send(0xa6);
		}
	}
	else
	{	
		if(g_set_info.jdqFlag == 1)
		{
			g_xih_ctrl.XIH_State = SG_MENU_CTRL;
			tryCount = 3;
			Pic_Send(0xc5);
		}
	}
}
#endif

void JDQ_Init(void)
{
	g_xih_ctrl.XIH_State = SG_MENU_CTRL;
	tryCount = 3;
	Pic_Send(0xa6);
}

