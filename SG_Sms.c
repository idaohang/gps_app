/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Sms.c
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
#include "mn_events.h"
#include "Msg.h"
#include "SG_Hh.h"
#include "SG_Ext_Dev.h"

int SmsReadTime = 0xffffffff;
int SmsSaveTime = 0xffffffff;
int AlarmSmsSendTime = 0xffffffff;



//the table is for changing from 03.38 alphabet table to ASCII alphabet table  
//add by sunsome.ju
const unsigned char default_to_ascii_table[128] =
{
    0x40,0x20,0x24,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x0A,0x20,0x20,0x0D,0x20,0x20,
    0x20,0x5F,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x21,0x22,0x23,0x20,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x20,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x20,0x20,0x20,0x20,0x15,
    0x20,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x20,0x20,0x20,0x20,0x20
};

//the table is for changing from ASCII alphabet table to 03.38 alphabet table  
//add by sunsome.ju
const unsigned char ascii_to_default_table[128] =
{
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x0A,0x20,0x20,0x0D,0x20,0x20,
    0x20,0x20,0x20,0x20,0x20,0x5F,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x21,0x22,0x23,0x02,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x00,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x20,0x20,0x20,0x20,0x11,
    0x20,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x20,0x20,0x20,0x20,0x20
};




SG_SMS_T g_sms_t={0};
SG_SMS_T g_sms_mo ={0};

extern uint smsbuflen; // ����buf����
extern uint8 smsbuf[1024]; // ����buffer


/********************************************************************
*******************************�����嶨��****************************
********************************************************************/

/****************************************************************
  ��������	SG_AlarmSms_Send
  ��  ��  �����ͱ�������
  �����������
  �����������
  ��д��  �����׽�
  �޸ļ�¼������ 2009/4/16
  ��ע    : 
****************************************************************/
void SG_AlarmSms_Send(void)
{
	char title[140]= "";				
	int pTP_len;				
	unsigned char pTP_UD[140]="";						


	if(g_state_info.SendSMSType ==1)
	{
		SCI_MEMSET(title, 0, 140);
		strcpy(title, "���İ����ִ��ڷ�������״̬���뼰ʱȷ���ֳ���������ϳ�̨��ȫ��лл!\r\n");	
	}
	else if(g_state_info.SendSMSType ==2)
	{
		SCI_MEMSET(title, 0, 140);
		strcpy(title, "���İ����ִ��ڽ�������״̬���뼰ʱȷ���ֳ���������ϳ�̨��ȫ��лл!\r\n");	
	}

	pTP_len= GBK2Unicode(pTP_UD, (unsigned char*)title);				
	UnicodeBE2LE(pTP_UD, pTP_len);	

	if(g_state_info.SendSMSCount == 2)
	{
		if(strlen(g_set_info.sSmsAlarmNo1) == 11)
			SendMsgReqToMN((uint8*)g_set_info.sSmsAlarmNo1,(uint8*)pTP_UD,pTP_len,MN_SMS_UCS2_ALPHABET);	
		else
			g_state_info.SendSMSCount--;
	}

	if(g_state_info.SendSMSCount == 1)
	{
		if(strlen(g_set_info.sSmsAlarmNo2) == 11)
			SendMsgReqToMN((uint8*)g_set_info.sSmsAlarmNo2,(uint8*)pTP_UD,pTP_len,MN_SMS_UCS2_ALPHABET);	
		else
			g_state_info.SendSMSCount--;
	}

	if(g_state_info.SendSMSCount > 0)
		g_state_info.SendSMSCount--;

}

/****************************************************************
  ��������	SG_Read_Flag
  ��  ��  �����ż���־�ж�
  �����������
  �����������
  ��д��  ��
  �޸ļ�¼��
  ��ע    : 
****************************************************************/
void SG_SendSms_Flag(void)
{
	if(CheckTimeOut(SmsReadTime) == 1)
	{
		g_state_info.SendSMSFlag = 0;
		SmsReadTime = 0xffffffff;
	}
}

/****************************************************************
  ��������	SG_SaveSms_Send
  ��  ��  �������ط�
  �����������
  �����������
  ��д��  ��
  �޸ļ�¼��
  ��ע    : 
****************************************************************/
void SG_SaveSms_Send(void)
{
	if(CheckTimeOut(SmsSaveTime) == 1)
	{
		SendMsgReqToMN(g_sms_save.telenum, g_sms_save.msg, g_sms_save.len,g_sms_save.alphabet_type);
		memset(&g_sms_save, 0, sizeof(g_sms_save));
		SCI_TRACE_LOW("======g_sms_save.flag: %d",g_sms_save.flag);
		SmsSaveTime = 0xffffffff;
	}
}

/****************************************************************
  ��������	SG_AlarmSms_Send
  ��  ��  �����������ط�
  �����������
  �����������
  ��д��  ��
  �޸ļ�¼��
  ��ע    : 
****************************************************************/
void SG_AlarmSaveSms_Send(void)
{
	
	if(CheckTimeOut(AlarmSmsSendTime) == 1)
	{
		g_sms_save.flag--;

		if(g_sms_save.flag > 0)
		{
			if((g_gprs_t.state != GPRS_RXHEAD)&&( g_gprs_t.state != GPRS_RXCONTENT))
			{
				SendMsgReqToMN(g_sms_save.telenum, g_sms_save.msg, g_sms_save.len,g_sms_save.alphabet_type);
			}
			AlarmSmsSendTime = GetTimeOut(180);
		}
		else
		{
			AlarmSmsSendTime = 0xffffffff;
			memset(&g_sms_save, 0, sizeof(g_sms_save));
		}
		SCI_TRACE_LOW("======g_sms_save.flag: %d",g_sms_save.flag);
	}

}


/****************************************************************
  ��������	SG_AlarmSms_Save
  ��  ��  ���������ű���
  �����������
  �����������
  ��д��  ��
  �޸ļ�¼��
  ��ע    : 
****************************************************************/
void SG_AlarmSms_Save(void *msg, int len)
{
	
	//���ű���
	g_sms_save.flag = 4;
	g_sms_save.alphabet_type = MN_SMS_8_BIT_ALPHBET;
	g_sms_save.len = len;
	memset(g_sms_save.msg, 0, 200);
	memcpy((char *)g_sms_save.msg, (char *)msg, len);
	memset(g_sms_save.telenum, 0, 20);
	strcpy((char *)g_sms_save.telenum, (char *)g_set_info.sCenterNo);
	SCI_TRACE_LOW("======g_sms_save.flag: %d",g_sms_save.flag);
    g_state_info.SendSMSFlag = 1;
	SmsReadTime = GetTimeOut(600);
	AlarmSmsSendTime = GetTimeOut(60);
}

/****************************************************************
  ��������	SG_Sms_Read
  ��  ��  ����ȡ����
  �����������
  �����������
  ��д��  ��
  �޸ļ�¼��
  ��ע    : 
****************************************************************/
void SG_Sms_Read(void)
{

	ERR_MNSMS_CODE_E ret;

	SCI_TRACE_LOW("<<<<<<SG_Sms_Read<<<<<<");

	if(g_state_info.SimFlag == 0)
		return;

	//�õ�����Ϣ״̬
    ret = MNSMS_SetSmsReceiveFunc(TRUE);

	if(ERR_MNSMS_NONE == ret)
	{
		SCI_TRACE_LOW("==========MNSMS_SetSmsReceiveFunc");
	}
	else
	{
		ret = MNSMS_GetErrCode();
		SCI_TRACE_LOW("--SG_Sms_Read: MNSMS_SetStoragePriority failure: err:%x",ret);
	}
	
}

/****************************************************************
  ��������	SG_Sms_Init
  ��  ��  �����ö��Ŵ洢����ΪME
  �����������
  �����������
  ��д��  ��������
  �޸ļ�¼������ 2007/9/11
  ��ע    : ɾ������ME�д洢�Ķ��Ź���ע��
****************************************************************/
void SG_Sms_Init(void)
{
//	MN_SMS_RECORD_ID_T	record_id=0;
	MN_SMS_STATUS_ARR_E status_arr = {0};
	ERR_MNSMS_CODE_E ret;
	MN_SMS_RECORD_ID_T max_sms_num = 0;
	int i;
//	uint8 max_sms_num = 0;
//	uint16 max_num = 0;
//	int i;
//	BOOLEAN IsMe;

	SCI_TRACE_LOW("<<<<<<SG_Sms_Init<<<<<<");

/*
	SCI_MEMSET(&status_arr,0x00,sizeof(MN_SMS_STATUS_ARR_E)); // clear 
	
	// ��ȡSIM����ME������֧�ִ洢����������
	MNSMS_GetSmsNum(&status_arr.mn_sms_in_sim_nv_max_num);	
	SCI_TRACE_LOW("<<<<<<MAX SMS NUM of all storage = %d",status_arr.mn_sms_in_sim_nv_max_num);
	status_arr.mn_sms_status_arr_e_ptr = SCI_ALLOC(status_arr.mn_sms_in_sim_nv_max_num*sizeof(MN_SMS_STATUS_E));
	
	// ȡ��ɾ��ME ��֧�ֵ���������
	ret = MNSMS_GetSmsStatus(MN_SMS_STORAGE_ME,&max_sms_num,status_arr.mn_sms_status_arr_e_ptr);
	if(ERR_MNSMS_NONE == ret)
		SCI_TRACE_LOW("<<<<<<MNSMS_GetSmsStatus: successful<<<<<<ME MAX %d",max_sms_num);
	else
		SCI_TRACE_LOW("<<<<<<MNSMS_GetSmsStatus: failure<<<<<<err code :%x",ret);
	
	// ME�еĶ���ȫ��ɾ��
	for(i = 0; i < 20; i++)
	{
		ret = MNSMS_UpdateSmsStatus(MN_SMS_STORAGE_ME,i,MN_SMS_FREE_SPACE);
		if(ERR_MNSMS_NONE == ret)
		{
			SCI_TRACE_LOW("--SG_Sms_Init: del sms ME record_id:%d",i);
		}
		else
		{
			SCI_TRACE_LOW("--SG_Sms_Init: MNSMS_UpdateSmsStatus() err:%x",ret);
		}
	}
	
	SCI_FREE(status_arr.mn_sms_status_arr_e_ptr);


*/	
	//���ö���Ϣ�洢ME����
	ret = MNSMS_SetStoragePriority(FALSE);

	if(ERR_MNSMS_NONE == ret)
	{
		SCI_TRACE_LOW("--SG_Sms_Init: MNSMS_SetStoragePriority successful: ME FIRST");
	}
	else
	{
		ret = MNSMS_GetErrCode();
		SCI_TRACE_LOW("--SG_Sms_Init: MNSMS_SetStoragePriority failure: err:%x",ret);
	}

	
}

/****************************************************************
  ��������	HandleSMSMTInd
  ��  ��  ��������ն��ŵĺ���
  �����������
  �����������
  ��д��  ��������
  �޸ļ�¼������ 2007/9/11
  ��ע    : ��
****************************************************************/
void HandleSMSMTInd(DPARAM param)
{    
    APP_MN_SMS_IND_T 	*sig_ptr = (APP_MN_SMS_IND_T *)param;
    APP_SMS_USER_DATA_T	sms_user_data_t; 
	ERR_MNSMS_CODE_E 	ret;
	char tele_num[20] = "";


    SCI_ASSERT( PNULL != sig_ptr );
    SCI_TRACE_LOW("***************HandleSMSMTInd********************");

	// ��ӡ�յ��Ķ��ŵķ��ͺ���
	MMI_GenDispNumber( sig_ptr->sms_t.origin_addr_t.number_type,
				(uint8*)sig_ptr->sms_t.origin_addr_t.num_len,
				(uint8*)sig_ptr->sms_t.origin_addr_t.party_num,
				tele_num ,
				40 + 2);
	SCI_TRACE_LOW("<<<<<<record id is %d",sig_ptr->record_id); 
	SCI_TRACE_LOW("The SMS is from Number:%s",tele_num);
	
    DataLedCnt = 15;//�յ���������.
    // 30���ڲ��ٽ��ж��ż��
    g_state_info.SendSMSFlag = 1;
	SmsReadTime = GetTimeOut(30);
	
	SCI_MEMSET( &sms_user_data_t, 0x00, sizeof( APP_SMS_USER_DATA_T ) );
	SCI_TRACE_LOW("sig_ptr->sms_t.user_head_is_exist %d",sig_ptr->sms_t.user_head_is_exist);

	SCI_TRACE_LOW("sig_ptr->sms_t.dcs.alphabet_type %d",sig_ptr->sms_t.dcs.alphabet_type);
	
	
	// ���յ��Ķ���ϢTPDU��ʽ�е����ݽ�����user data header����Ч��user data
	MNSMS_DecodeUserData(sig_ptr->sms_t.user_head_is_exist,
					&sig_ptr->sms_t.dcs,
					&sig_ptr->sms_t.user_data_t,
					&sms_user_data_t.user_data_head_t,
					&sms_user_data_t.user_valid_data_t);

	// ��ӡδ�����ͽ���������
#if (0)
	{
		int i;

		// δ��������
		SCI_TRACE_LOW("*******sms_t.user_data_t.user_data_arr***");
		for(i=0;i<sig_ptr->sms_t.user_data_t.length;i++)
	   	 	SCI_TRACE_LOW("1--%02x",sig_ptr->sms_t.user_data_t.user_data_arr[i]);

		// ��������
		SCI_TRACE_LOW("****sms_user_data_t.user_valid_data_t.user_valid_data_arr***");
		for(i=0;i<sms_user_data_t.user_valid_data_t.length;i++)
	    	SCI_TRACE_LOW("3--%02x",sms_user_data_t.user_valid_data_t.user_valid_data_arr[i]);
	}
#endif /* (0) */

	if(sig_ptr->sms_t.dcs.alphabet_type != MN_SMS_8_BIT_ALPHBET) // �����·��Ķ��Ų��账��,�����͵��ֱ�
	{
		xSignalHeaderRec     *signal_ptr = NULL;
		uint8 type = sig_ptr->sms_t.dcs.alphabet_type;
		uint8 siglen;
		uint8 sigdata[500] = "";
		uint8 len,tele_len;
		char head[]=ALLSMS_HEAD;

		//���������ж�
		SCI_MEMCPY(head, sms_user_data_t.user_valid_data_t.user_valid_data_arr, 3);
		SCI_TRACE_LOW("The SMS head is: %s",head);
		if (strcmp(head, ALLSMS_HEAD) == 0)
		{
			siglen = sms_user_data_t.user_valid_data_t.length;	
			memset(g_state_info.sUserNo, 0, SG_PHONE_LEN + 5);
			memcpy(g_state_info.sUserNo, tele_num, strlen(tele_num));
			sigdata[0] = siglen;			
			memcpy(sigdata+1,sms_user_data_t.user_valid_data_t.user_valid_data_arr,siglen);		
			SG_CreateSignal_Para(SG_SMS_HH,siglen+1, &signal_ptr,sigdata); 
		}
		else
		{
			if(g_set_info.bNewUart4Alarm & UT_GPS)
			{
				siglen = sms_user_data_t.user_valid_data_t.length;
				memcpy(sigdata+3,sms_user_data_t.user_valid_data_t.user_valid_data_arr,siglen);
			}
			else 
			{
				siglen = sig_ptr->sms_t.user_data_t.length;
				memcpy(sigdata+3,sig_ptr->sms_t.user_data_t.user_data_arr,siglen);
			}
			sigdata[1] = type;
			sigdata[2] = siglen;
			tele_len = strlen(tele_num);
			sigdata[3+siglen] = tele_len;
			memcpy(sigdata+4+siglen,tele_num,tele_len);
			len = 2+siglen+1+tele_len;
			sigdata[0] = len;
			SCI_TRACE_LOW("<<<<<<<signal length = %d  %d  %d",siglen,tele_len,len);
			
			if((g_set_info.bNewUart4Alarm & UT_GPS)&&(g_set_info.GpsDevType == 1))
			{
				SCI_TRACE_LOW("<<<<<<<SG_SMS_ZB======");
				SG_CreateSignal_Para(SG_SMS_ZB,len+1, &signal_ptr,sigdata); //չ��������
			}
			else if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 2)//YH������
			{
				SCI_TRACE_LOW("<<<<<<<SG_SMS_YH======");
				SG_CreateSignal_Para(SG_SMS_YH,len+1, &signal_ptr,sigdata); 
			}
			else if(!(g_set_info.bNewUart4Alarm & UT_GPS) || checkhandset == 1)
			{
				SCI_TRACE_LOW("<<<<<<<SG_MSG_HH======");
				SG_CreateSignal_Para(SG_MSG_HH,len+1, &signal_ptr,sigdata); 
			}
		}
	}
	else
	{
		if(sms_user_data_t.user_data_head_t.length == 0)
		{
			 // ����Ϣ����ͷ�뱨��ͷ�Ƚ�
	        if(memcmp(sms_user_data_t.user_valid_data_t.user_valid_data_arr,(char*)MSG_HEAD,MSG_HEAD_LEN)==0)
	        {
				xSignalHeaderRec      *signal_ptr = NULL;
				char siglen = sms_user_data_t.user_valid_data_t.length;
				char sigdata[500] = "";
				
				sigdata[0] = siglen;
				SCI_TRACE_LOW("<<<<<<<signal length = %02x  %02x",siglen,sigdata[0]);
				
				memcpy(sigdata+1,sms_user_data_t.user_valid_data_t.user_valid_data_arr,siglen);

				SG_CreateSignal_Para(SG_MSG_HANDLE,siglen+1, &signal_ptr,sigdata); 

				if(g_set_info.nNetType == 0) // ����ģʽ
				{
					SmsSndTimer = g_set_info.nSmsInterval-2;
				}
				else
				{
					SmsSndTimer = g_set_info.nSmsInterval_data-2;
				}
				
//	        	SG_Rcv_Handle_Msg(sms_user_data_t.user_valid_data_t.user_valid_data_arr,sms_user_data_t.user_valid_data_t.length);
	        }
		}
		else	//��������
		{
			//�����ŵ�һ������
			if(g_sms_t.sms_ide[0] != sms_user_data_t.user_data_head_t.user_header_arr[2])
			{
				g_sms_t.sms_ide[0] = sms_user_data_t.user_data_head_t.user_header_arr[2];
				g_sms_t.sms_ide[1] = sms_user_data_t.user_data_head_t.user_header_arr[3];
				g_sms_t.sms_ide[2] = sms_user_data_t.user_data_head_t.user_header_arr[4];
				
				SCI_MEMCPY(g_sms_t.sms_buf,sms_user_data_t.user_valid_data_t.user_valid_data_arr,sms_user_data_t.user_valid_data_t.length);
				g_sms_t.sms_len = sms_user_data_t.user_valid_data_t.length;
			}
			else //��������������
			{
				g_sms_t.sms_ide[2] = sms_user_data_t.user_data_head_t.user_header_arr[4];

				if(g_sms_t.sms_ide[2] <= g_sms_t.sms_ide[1])
				{
					SCI_MEMCPY(g_sms_t.sms_buf+g_sms_t.sms_len,sms_user_data_t.user_valid_data_t.user_valid_data_arr,sms_user_data_t.user_valid_data_t.length);
					g_sms_t.sms_len += sms_user_data_t.user_valid_data_t.length;

					//�������һ��
					if(g_sms_t.sms_ide[2] == g_sms_t.sms_ide[1])
					{
						
						xSignalHeaderRec      *signal_ptr = NULL;
						char sigdata[500] = "";
						short len = 0;
						char len1 = 0;
						char len2 = 0;
						
						len = (short)(g_sms_t.sms_len);
						len1 = (char)(g_sms_t.sms_len>>8);
						len2 = (char)(g_sms_t.sms_len&0x00ff);

						sigdata[0] = len1;
						sigdata[1] = len2;
						
						memcpy(sigdata+2,g_sms_t.sms_buf,len);
						SG_CreateSignal_Para(SG_LONGMSG_HANDLE,len+2, &signal_ptr,sigdata); 
						//SG_Rcv_Handle_Msg(g_sms_t.sms_buf,g_sms_t.sms_len);
						SCI_MEMSET(&g_sms_t,0,sizeof(g_sms_t));
					}
				}
				else
					SCI_MEMSET(&g_sms_t,0,sizeof(g_sms_t));
				
			}
		}
		
	}
	
	SCI_TRACE_LOW("<<<<<<record id is %d",sig_ptr->record_id); 
	
	// ���������ɾ��
	ret = MNSMS_UpdateSmsStatus(MN_SMS_STORAGE_ME,sig_ptr->record_id,MN_SMS_FREE_SPACE);
	if(ERR_MNSMS_NONE == ret)
	{
		SCI_TRACE_LOW("--HandleSMSMTInd:MNSMS_UpdateSmsStatus successful");
		SCI_TRACE_LOW("--the SMS,record id of %d is delete", sig_ptr->record_id);
	}
	else
	{
		SCI_TRACE_LOW("--HandleSMSMTInd:MNSMS_UpdateSmsStatus failure: err:%x",ret);
	}
	
}

/****************************************************************
  ��������	MnSms_EventCallback
  ��  ��  ����ME����SIM���ж���Ϣ��ʱ��ɾ�����ж���Ϣ(1.5��ֻ����ME��)
  ���������DPARAM param
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
LOCAL	void HandleMemFullInd(
							  DPARAM param
							  )
{
	APP_MN_SMS_MEM_FULL_IND_T	*sig_ptr = (APP_MN_SMS_MEM_FULL_IND_T*)param;
	MN_SMS_STATUS_ARR_E status_arr = {0};
	ERR_MNSMS_CODE_E ret;
	MN_SMS_RECORD_ID_T max_sms_num = 0;
	int i = 0;

	SCI_TRACE_LOW("--HandleMemFullInd:");

	SCI_ASSERT( PNULL != sig_ptr);
	
	
	SCI_TRACE_LOW("--%d",sig_ptr->mem_status);
	switch( sig_ptr->mem_status ) 
	{
		// ME��ʱɾ�����ж���
		case MN_SMS_ME_FULL:
		case MN_SMS_ALL_FULL:
			SCI_MEMSET(&status_arr,0x00,sizeof(MN_SMS_STATUS_ARR_E)); // clear 

			// ��ȡSIM����ME������֧�ִ洢����������
			MNSMS_GetSmsNum(&status_arr.mn_sms_in_sim_nv_max_num);	
			SCI_TRACE_LOW("<<<<<<MAX SMS NUM of all storage = %d",status_arr.mn_sms_in_sim_nv_max_num);
    		status_arr.mn_sms_status_arr_e_ptr = SCI_ALLOC(status_arr.mn_sms_in_sim_nv_max_num*sizeof(MN_SMS_STATUS_E));

			// ȡ��ɾ��ME ��֧�ֵ���������
    		ret = MNSMS_GetSmsStatus(MN_SMS_STORAGE_ME,&max_sms_num,status_arr.mn_sms_status_arr_e_ptr);
			if(ERR_MNSMS_NONE == ret)
				SCI_TRACE_LOW("<<<<<<MNSMS_GetSmsStatus: successful<<<<<<ME MAX %d",max_sms_num);
			else
				SCI_TRACE_LOW("<<<<<<MNSMS_GetSmsStatus: failure<<<<<<err code :%x",ret);

			// ME�еĶ���ȫ��ɾ��
    		for(i = 0; i < max_sms_num; i++)
    		{
        		ret = MNSMS_UpdateSmsStatus(MN_SMS_STORAGE_ME,i,MN_SMS_FREE_SPACE);
				if(ERR_MNSMS_NONE == ret)
				{
					SCI_TRACE_LOW("--SG_Sms_Init: del sms ME record_id:%d",i);
				}
				else
				{
					SCI_TRACE_LOW("--SG_Sms_Init: MNSMS_UpdateSmsStatus() err:%x",ret);
				}
        	}

			SCI_FREE(status_arr.mn_sms_status_arr_e_ptr);
   
			break;
		default:
			break;
	}
}

/****************************************************************
  ��������	MnSms_EventCallback
  ��  ��  ���������ҵ���callback����
  ���������uint32 	event_id  �¼���id
  			uint32 	task_id
  			void*	param_ptr
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void MnSms_EventCallback( 
								uint32 task_id, //task ID
								uint32 event_id, //event ID
								void *param_ptr
								)

{	
	xSignalHeaderRec      *signal_ptr = NULL;

   	SCI_ASSERT(PNULL != param_ptr);
	if( PNULL == param_ptr )
	{
		SCI_TRACE_LOW("MnSms_EventCallback: the ponit of param is empty");
		return;
	}
	
    switch ( event_id)
    {
    /*
	EV_MN_APP_SMS_READY_IND_F = ( MN_APP_SMS_SERVICE << 8 ) , 
	EV_MN_APP_SMS_MEM_FULL_IND_F,
 	EV_MN_APP_READ_SMS_CNF_F          , 
	EV_MN_APP_UPDATE_SMS_STATUS_CNF_F  , 
 	EV_MN_APP_READ_STATUS_REPORT_CNF_F, 
	EV_MN_APP_SMS_STATUS_REPORT_IND_F      , 
 	EV_MN_APP_SEND_SMS_CNF_F          , 
 	EV_MN_APP_WRITE_SMS_CNF_F         , 
  	EV_MN_APP_EDIT_SMS_CNF_F         , 
  	EV_MN_APP_SMS_IND_F               , 
	EV_MN_APP_READ_SMS_PARAM_CNF_F    , 
 	EV_MN_APP_WRITE_SMS_PARAM_CNF_F   , 
	EV_MN_APP_DELETE_ALL_SMS_CNF_F	  ,
	EV_MN_SET_ASSERT_SMS_DESTADDR_CNF_F,
	EV_MN_APP_SET_SMS_RECEIVE_FUNC_CNF_F,
 	MAX_MN_APP_SMS_EVENTS_NUM         , 
	*/
    case EV_MN_APP_SMS_READY_IND_F:
    	SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_SMS_READY_IND_F");
//		HandleSMSReadyInd(param);
        break;
		
    case EV_MN_APP_READ_SMS_CNF_F:
   		SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_READ_SMS_CNF_F");
//		MMISMS_HandleUserReadSMSCnf( param ); 
        break;
		
		
    case EV_MN_APP_UPDATE_SMS_STATUS_CNF_F:
    	SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_UPDATE_SMS_STATUS_CNF_F");
//        HandleUpdateSMSCnf( param );
        break;
		
    case EV_MN_APP_SEND_SMS_CNF_F:
		{
			APP_MN_SEND_SMS_CNF_T *sig_ptr = (APP_MN_SEND_SMS_CNF_T *)param_ptr;
			
    		SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_SEND_SMS_CNF_F");
			SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_SEND_SMS_CNF_F case %d",sig_ptr->cause);

			if(g_sms_save.flag == 1 && g_set_info.SmsAlarmFlag == 0)
			{
				if((sig_ptr->cause == MN_SMS_MN_BUSY)||(sig_ptr->cause == MN_SMS_SIM_MEM_FULL))
				{
					SmsSaveTime = GetTimeOut(8);
				}
				else
				{
					memset(&g_sms_save, 0, sizeof(g_sms_save));
					SCI_TRACE_LOW("======g_sms_save.flag: %d",g_sms_save.flag);
				}
			}

			if(Sms_Long_Send_Flag == 1)// ���ͳ�������ʱ
			{
				SG_CreateSignal(SG_SEND_LONGSMS,0, &signal_ptr); 	
			}

			if((g_set_info.bNewUart4Alarm & UT_GPS)&&(g_set_info.GpsDevType == 1))
			{
				if(sig_ptr->cause == 0)
				{
					SG_ZBGPS_ACK(CMD_SMS_OUT, 0x00);
				}
				else
				{
					SG_ZBGPS_ACK(CMD_SMS_OUT, 0x01);
				}
				if(g_state_info.SendSMSCount > 0)
					SG_AlarmSms_Send();
			}
			


   		 }
//        HandleSendCnf( param);
        break;
		
    case EV_MN_APP_SMS_IND_F:
		// ������յ��Ķ���
    	SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_SMS_IND_F");
        HandleSMSMTInd( param_ptr );
        break;

    case EV_MN_APP_SMS_STATUS_REPORT_IND_F:
     	SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_SMS_STATUS_REPORT_IND_F");
//     	HandleSMSReportInd( param);
     	break;
		
	
	case EV_MN_APP_EDIT_SMS_CNF_F :
		SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_EDIT_SMS_CNF_F");
//		HandleEditSMSCnf( param );
		break;
		
    case EV_MN_APP_SMS_MEM_FULL_IND_F:
		// ��ME����SIM���ж���Ϣ��ʱ��ɾ�����ж���Ϣ(1.5��ֻ����ME��)
  		SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_SMS_MEM_FULL_IND_F");
		HandleMemFullInd( param_ptr );
        break;
		
	case EV_MN_APP_WRITE_SMS_PARAM_CNF_F:
		SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_WRITE_SMS_PARAM_CNF_F");
//		HandleSaveSMSCnf( param);
		break;
	case EV_MN_APP_SET_SMS_RECEIVE_FUNC_CNF_F:
		SCI_TRACE_LOW("--MnSms_EventCallback:EV_MN_APP_SET_SMS_RECEIVE_FUNC_CNF_F");
//		HandleSetSMSReceiveCnf( param );
        break;	
    default:
        break;
    }
    
}

void SG_Send_LongSms(void)
{
	MN_SMS_MO_SMS_T      mo_sms;   // Ҫ���͵�MO���Žṹ��
	ERR_MNSMS_CODE_E	 mn_err_code = ERR_MNSMS_NONE;
	MN_SMS_USER_DATA_HEAD_T  sms_head;
	MN_SMS_STORAGE_E 		storage = MN_SMS_NO_STORED;	   // the storage (SIM or NV)
	BOOLEAN					status_report=0;
	BOOLEAN					is_more_msg=0;    //  the more message to send 
	MN_SMS_USER_VALID_DATA_T sms_context;
	MN_CALLED_NUMBER_T	dest_number,*p_dest_number;
	MMI_PARTY_NUMBER_T	party_number;
	char telnum[15]="86";
	static int packno = 1;

    SCI_MEMSET( &mo_sms, 0 , sizeof(MN_SMS_MO_SMS_T) );
    SCI_MEMSET(&party_number, 0, sizeof(MMI_PARTY_NUMBER_T));

	strcat(telnum,g_set_info.sCenterNo);
//	strcat(telnum,"15859005120");
	SCI_TRACE_LOW("*******telnum:%s***",telnum);
	// ��Ŀ�ĵ�ַ�ĵ绰������str��ʽת��Ϊbcd��ʽ
    MMI_GenPartyNumber(telnum, strlen(telnum) , &party_number );

    p_dest_number = &dest_number;

	// translate the struct from MMI_PARTY_NUM_T to MN_CALL_NUM_T
	MMIPARTYNUM_2_MNCALLEDNUM(party_number, p_dest_number);

	// copy��Ҫ���͵�MO���Žṹ����
	SCI_MEMCPY( &mo_sms.dest_addr_t,
				&dest_number,
				sizeof(MN_CALLED_NUMBER_T));

  	
    mo_sms.dest_addr_present        = TRUE;
	mo_sms.dest_addr_t.number_plan  = MN_NUM_PLAN_ISDN_TELE;
   
    mo_sms.dcs.alphabet_type    = MN_SMS_8_BIT_ALPHBET;
    mo_sms.dcs.class_is_present = FALSE;
    mo_sms.time_format_e = MN_SMS_TP_VPF_RELATIVE_FORMAT;
    mo_sms.time_valid_period.time_second = 0;
    mo_sms.status_report_is_request =  status_report ; 
    mo_sms.pid_present  = TRUE;
    mo_sms.pid_e        = MN_SMS_PID_DEFAULT_TYPE;

#if (0)
    mo_sms.user_head_is_exist = FALSE;


	SCI_MEMCPY(sms_head.user_header_arr,"No head",7);
	sms_head.length=0;
#endif /* (0) */

	// split the long message and get the total number of sms
	SCI_MEMSET(&sms_context, 0, sizeof( MN_SMS_USER_VALID_DATA_T ) );
	
   	
	SCI_TRACE_LOW("--The alphabet_type = %d",mo_sms.dcs.alphabet_type);

	mo_sms.user_head_is_exist = TRUE;
	sms_head.length=0x05;
	sms_head.user_header_arr[0] = 0x00;
	sms_head.user_header_arr[1] = 0x03;
	sms_head.user_header_arr[2] = 0xc7;
	sms_head.user_header_arr[3] = smsbuflen/(MN_SMS_USER_DATA_LENGTH-10) + 1;
	sms_head.user_header_arr[4] = packno;
	SCI_TRACE_LOW("<<<<<< all PACK %d packno %d",sms_head.user_header_arr[3],sms_head.user_header_arr[4]);
	
	if(packno == sms_head.user_header_arr[3])
	{
		SCI_TRACE_LOW("<<<<<<1111PACK  no %d",sms_head.user_header_arr[4]);
		SCI_MEMCPY(sms_context.user_valid_data_arr,smsbuf+ (packno-1)*(MN_SMS_USER_DATA_LENGTH-10),smsbuflen-(packno-1)*(MN_SMS_USER_DATA_LENGTH-10));
		sms_context.length = smsbuflen -(packno-1)*(MN_SMS_USER_DATA_LENGTH-10);
	    MNSMS_EncodeUserData(mo_sms.user_head_is_exist,	
		mo_sms.dcs.alphabet_type,
		&sms_head,
		&sms_context,
		&mo_sms.user_data_t);

		SCI_TRACE_LOW("<<<<<<1111original len %d send len %d",sms_context.length,mo_sms.user_data_t.length);
		SCI_Sleep(5);
		mn_err_code = MNSMS_AppSendSms(&mo_sms,storage,MN_SMS_GSM_PATH,is_more_msg);
		SCI_TRACE_LOW("<<<<<<1111Send Msg Result %d",mn_err_code);
		packno = 1;
		Sms_Long_Send_Flag = 0;
	}
	else
	{
		SCI_MEMCPY(sms_context.user_valid_data_arr,smsbuf + (packno-1)*(MN_SMS_USER_DATA_LENGTH-10),(MN_SMS_USER_DATA_LENGTH-10));
		sms_context.length = MN_SMS_USER_DATA_LENGTH-10;
		SCI_TRACE_LOW("<<<<<<2222PACK  no %d",sms_head.user_header_arr[4]);
		
	    MNSMS_EncodeUserData(mo_sms.user_head_is_exist,	
		mo_sms.dcs.alphabet_type,
		&sms_head,
		&sms_context,
		&mo_sms.user_data_t);
		SCI_TRACE_LOW("<<<<<<2222original len %d send len %d",sms_context.length,mo_sms.user_data_t.length);

		SCI_Sleep(5);
	    mn_err_code = MNSMS_AppSendSms(&mo_sms,storage,MN_SMS_GSM_PATH,is_more_msg);
		SCI_TRACE_LOW("<<<<<<2222Send Msg Result %d",mn_err_code);

		packno ++;

	}


	


}



/**************************************************************************
  ��������	SendMsgReqToMN
  ��  ��  �����ⷢ�Ͷ��ŵĴ�����
  ���������uint8 *dest_number_ptr   		str��ʽ�Ķ��ŷ���Ŀ�ĵ绰����
  			uint8 *sms_context_ptr			����Ϣ������
  			uint sms_context_len			����Ϣ�ĳ���
  			MN_SMS_ALPHABET_TYPE_E alphabet_type  ����Ϣ���ݵı�������
  			(��: ���뷽ʽ��������: 
  			MN_SMS_DEFAULT_ALPHABET��ȱʡ��ʽ����7��������ʾһ���ַ�(���λΪ0)
			MN_SMS_8_BIT_ALPHBET��   8���ر��뷽ʽ��
			MN_SMS_UCS2_ALPHABET��   16���ر��뷽ʽ��)
  �����������
  ����ֵ  : ERR_MNSMS_NONE �ɹ�������Ϊʧ��
  ��д��  ��������
  �޸ļ�¼������ 2007/8/20
***************************************************************************/
ERR_MNSMS_CODE_E SendMsgReqToMN(
									uint8 *dest_number_ptr,
									uint8 *sms_context_ptr,
									uint sms_context_len,
									MN_SMS_ALPHABET_TYPE_E alphabet_type)
{
	MN_SMS_MO_SMS_T      mo_sms;   // Ҫ���͵�MO���Žṹ��
	ERR_MNSMS_CODE_E	 mn_err_code = ERR_MNSMS_NONE;
	MN_SMS_USER_DATA_HEAD_T  sms_head;
	MN_SMS_STORAGE_E 		storage = MN_SMS_NO_STORED;	   // the storage (SIM or NV)
	BOOLEAN					status_report=1;
	BOOLEAN					is_more_msg=0;    //  the more message to send 
	MN_SMS_USER_VALID_DATA_T sms_context;
	MN_CALLED_NUMBER_T	dest_number,*p_dest_number;
	MMI_PARTY_NUMBER_T	party_number;
	char telnum[15]="";
	xSignalHeaderRec      *signal_ptr = NULL;


	SCI_TRACE_LOW("*******SendMsgReqToMN:%s***",dest_number_ptr);
	if((*dest_number_ptr == '8' && *(dest_number_ptr+1) == '6'))
	{
		strcpy(telnum,"+");
	}
	else if(!((*dest_number_ptr == '8' && *(dest_number_ptr+1) == '6') || (*dest_number_ptr == '+' && *(dest_number_ptr+1) == '8')))
	{
		strcpy(telnum,"+86");
	}

		
	// �������
	if( NULL == dest_number_ptr || NULL == sms_context_ptr || 0 == sms_context_len)
	{
		SCI_TRACE_LOW("****SendMsgReqToMN:PARA ERROR");
		return ERR_MNSMS_NOT_RIGHT_PARAM;
	}
	
	DataLedCnt = 15;//���Ͷ�������
	g_state_info.SendSMSFlag = 1;
	SmsReadTime = GetTimeOut(30);
	
    SCI_MEMSET( &mo_sms, 0 , sizeof(MN_SMS_MO_SMS_T) );
    SCI_MEMSET(&party_number, 0, sizeof(MMI_PARTY_NUMBER_T));

	strcat(telnum,(char*)dest_number_ptr);
	SCI_TRACE_LOW("*******telnum:%s***",telnum);
	// ��Ŀ�ĵ�ַ�ĵ绰������str��ʽת��Ϊbcd��ʽ
    MMI_GenPartyNumber(telnum, strlen(telnum) , &party_number );

    p_dest_number = &dest_number;

	// translate the struct from MMI_PARTY_NUM_T to MN_CALL_NUM_T
	MMIPARTYNUM_2_MNCALLEDNUM(party_number, p_dest_number);

	// copy��Ҫ���͵�MO���Žṹ����
	SCI_MEMCPY( &mo_sms.dest_addr_t,
				&dest_number,
				sizeof(MN_CALLED_NUMBER_T));

  	
    mo_sms.dest_addr_present        = TRUE;
	mo_sms.dest_addr_t.number_plan  = MN_NUM_PLAN_ISDN_TELE;
   
    mo_sms.dcs.alphabet_type    = alphabet_type;
    mo_sms.dcs.class_is_present = FALSE;
    mo_sms.time_format_e = MN_SMS_TP_VPF_RELATIVE_FORMAT;
    mo_sms.time_valid_period.time_second = 0;
    mo_sms.status_report_is_request =  status_report ; 
    mo_sms.pid_present  = TRUE;
    mo_sms.pid_e        = MN_SMS_PID_DEFAULT_TYPE;

    mo_sms.user_head_is_exist = FALSE;


	SCI_MEMCPY(sms_head.user_header_arr,"No head",7);
	sms_head.length=0;

	// split the long message and get the total number of sms
	SCI_MEMSET(&sms_context, 0, sizeof( MN_SMS_USER_VALID_DATA_T ) );
	
   	
	SCI_TRACE_LOW("--The alphabet_type = %d",alphabet_type);
    if( MN_SMS_DEFAULT_ALPHABET == alphabet_type ) // ȱʡ��7bit���뷽ʽ
    {
		
		sms_context.length = sms_context_len;
        // ��ASCII��ת��Ϊȱʡ��ʽ����
 	    MMI_Ascii2default(sms_context.user_valid_data_arr,
        	sms_context.user_valid_data_arr,
        	sms_context.length);
    }
    else if(MN_SMS_UCS2_ALPHABET == alphabet_type) // 16bit�ı��뷽ʽ
    {
		sms_context.length = sms_context_len;
		
		//copy the vaild data to the param
	    SCI_MEMCPY( sms_context.user_valid_data_arr, 
	                sms_context_ptr ,
	                sms_context.length );    
    	
    }
    else if(MN_SMS_8_BIT_ALPHBET == alphabet_type)	//8
    {
		
		//copy the vaild data to the param
    	
    	//��ͨ����
		if(sms_context_len <= (MN_SMS_USER_DATA_LENGTH-10))
		{
			sms_context.length = sms_context_len;
		    SCI_MEMCPY( sms_context.user_valid_data_arr, 
		                sms_context_ptr ,
		                sms_context.length );    
			// get the length of user valid data
		}
		//��������
    	else
    	{
    		Sms_Long_Send_Flag = 1;
			SG_CreateSignal(SG_SEND_LONGSMS,0, &signal_ptr); 	

#if (0)
    		mo_sms.user_head_is_exist = TRUE;
			sms_head.length=0x05;
			sms_head.user_header_arr[0] = 0x00;
			sms_head.user_header_arr[1] = 0x03;
			sms_head.user_header_arr[2] = 0xc7;
			sms_head.user_header_arr[3] = sms_context_len/(MN_SMS_USER_DATA_LENGTH-6);
			SCI_TRACE_LOW("<<<<<< all PACK %d",sms_head.user_header_arr[3]);

			for(i=1;i<sms_head.user_header_arr[3]+1;i++)
			{
				sms_head.user_header_arr[4] = i;
				if(i < sms_head.user_header_arr[3]) // �������һ��
				{
					SCI_MEMCPY(sms_context.user_valid_data_arr,sms_context_ptr + (i-1)*(MN_SMS_USER_DATA_LENGTH-6),(MN_SMS_USER_DATA_LENGTH-6))
					sms_context.length = MN_SMS_USER_DATA_LENGTH-6;
					SCI_TRACE_LOW("<<<<<< PACK  no %d",sms_head.user_header_arr[4]);
					
				    MNSMS_EncodeUserData(mo_sms.user_head_is_exist,	
					alphabet_type,
					&sms_head,
					&sms_context,
					&mo_sms.user_data_t);

				    mn_err_code = MNSMS_AppSendSms(&mo_sms,storage,MN_SMS_GSM_PATH,is_more_msg);
				}
				else
				{
					SCI_TRACE_LOW("<<<<<< PACK  no %d",sms_head.user_header_arr[4]);
					SCI_MEMCPY(sms_context.user_valid_data_arr,sms_context_ptr + (i-1)*(MN_SMS_USER_DATA_LENGTH-6),sms_context_len-(i-1)*(MN_SMS_USER_DATA_LENGTH-6))
					sms_context.length = sms_context_len-(i-1)*(MN_SMS_USER_DATA_LENGTH-6);
					
				    MNSMS_EncodeUserData(mo_sms.user_head_is_exist,	
					alphabet_type,
					&sms_head,
					&sms_context,
					&mo_sms.user_data_t);

				    mn_err_code = MNSMS_AppSendSms(&mo_sms,storage,MN_SMS_GSM_PATH,is_more_msg);
					
				}
			}
#endif /* (0) */

			return 1;
		}
    	
    }

	// ��user data header����Ч��user data����ɶ���ϢTPDU��ʽ�е�����	   
    MNSMS_EncodeUserData(mo_sms.user_head_is_exist,	
					alphabet_type,
					&sms_head,
					&sms_context,
					&mo_sms.user_data_t);

    
	 
   	if(Log_Enable1 == DEBUG_ENABLE)
    {
		int i;

		SCI_TRACE_LOW("--the encode user_data_arr"); // ��װ�õ�PDU��ʽ�Ķ�������
    	for(i=0;i<mo_sms.user_data_t.length;i++)
    		SCI_TRACE_LOW("%d--%02x",i,mo_sms.user_data_t.user_data_arr[i]);
    }
	
    // ���Ͷ���Ϣ
    SCI_Sleep(5);
    mn_err_code = MNSMS_AppSendSms(&mo_sms,storage,MN_SMS_GSM_PATH,is_more_msg);
    if( ERR_MNSMS_NONE == mn_err_code )
    {
		SG_Send_SendSMSok(0x00); //���ֱ����ͷ��Ͷ���Ϣ�ɹ�
		if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 2)
		{
			GpsDev_SendSmsResult(0x00);
		}
        SCI_TRACE_LOW("--SendMsgReqToMN: User MNSMS_AppSendSms api OK!");
    }
    else
    {
		SG_Send_SendSMSok(0x01); //���ֱ����ͷ��Ͷ���Ϣʧ��
		if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 2)
		{
			GpsDev_SendSmsResult(0x01);
		}
		memset(&g_sms_save, 0, sizeof(g_sms_save));
       	SCI_TRACE_LOW("--SendMsgReqToMN: User MNSMS_AppSendSms api	Failure! error code:%x",mn_err_code );
    }

    return ( mn_err_code );

}

/**************************************************************************
  ��������	SendMsgReqToMN
  ��  ��  ��This function is to changing from ASCII alphabet table to 
  			03.38 alphabet table(��ascII��ת��Ϊ���ŵ�7bitĬ�ϱ��뷽ʽ
  ���������uint8 *ascii_ptr   		str��ʽ��ת�����ַ���
  			uint16 str_len			��ת�����ַ�������
  ���������uint8  *default_ptr  	����ת���������
  ����ֵ  : ERR_MNSMS_NONE �ɹ�������Ϊʧ��
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/20
***************************************************************************/

unsigned char MMI_Ascii2default(
    uint8             *ascii_ptr,
    uint8             *default_ptr,
    uint16            str_len)
{
    unsigned char return_val = 0;
    uint16 i = 0;


    for(i=0;i<str_len;i++)
    {
        if(*ascii_ptr >=0x80)
        {
            return_val = 0;
            return return_val;
        }
        else
        {
            *default_ptr++ = ascii_to_default_table[*ascii_ptr++];
        }
    }

    return_val = 1;
    return return_val;

}

 // ���������·�����
void SG_Msg_Handle(xSignalHeaderRec   *receiveSignal)
{
	char msglen;
	char msg[500] = "";

	msglen = *((char*)receiveSignal + 16);
	memcpy(msg,(char*)receiveSignal + 17,msglen);
	SCI_TRACE_LOW("<<<<<<SG_Msg_Handle: %d",msglen);
	SG_Rcv_Handle_Msg((unsigned char *)msg,msglen);
}

 
