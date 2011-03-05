/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2007-2008 ��Ȩ����
  �ļ��� ��SG_Hh.c
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2005-7-23
  ����������SG2000  ���ķ��ͺʹ���  
  �޸ļ�¼��

***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SG_GPS.h"
#include "SG_Send.h"
#include "SG_Set.h"
#include "os_api.h"
#include "tb_hal.h"
#include "SG_Hh.h"
#include "SG_Call.h"
#include "Msg.h"
#include "sio.h"
#include "ref_engineering.h"
#include "SG_DynamicMenu.h"
#include "SG_MsgHandle.h"
#include "SG_Camera.h"
#include "SG_Receive.h"
#include "SG_Timer.h"
#include "SG_Menu.h"
#include "SG_Sms.h"
#include "SG_Ext_Dev.h"


SENDFRAME g_gpsframe;
extern int bNeed;
extern SG_INFO_OIL *gp_oil_list;
uint32 TTSSendTimer;
uint8 TTSSendFlag = 0;    //TTS�ְ���־
extern long AlarmTime;
uint8 WatchReadFlag;  //͸�����ݶ���־   0ͣ  1��ʼ


/*************************************************************************/
/*********************************���蹫������****************************/
/*************************************************************************/

/*************************************************************
��ѯ�ַ�
**************************************************************/
unsigned char * FindChar(unsigned char *buf,int len,unsigned char c)
{
	int i = 0;
	while (i < len) {
		if(*(buf+i) == c)
			return (unsigned char *)(buf+i);
		i++;
	}
	return NULL;
}

/****************************************************************
  ������UnicodeBE2LE
  ���ܣ�unicode big-edien ת��little-edien
  ���룺str:����  len:����	   
  �������
  ���أ���	 
  ��д���º���
  ��¼��������2007-5-23
****************************************************************/

void UnicodeBE2LE(unsigned char *str,int len)
{
	int i=0;
	char temp=0;
	
	for(i=0;i<len;i+=2)
	{
		temp=str[i];
		str[i]=str[i+1];
		str[i+1]=temp;
	}
} 
#ifdef _DUART
/****************************************************************
  ��������SG_Uart_Read_Clear
  ��  ��  �����ڻ������������
  ���������posi�����β��ָ���ַ
  �����������
  ��д��  ���º���
  �޸ļ�¼��������2006-7-7
****************************************************************/
void SG_Uart2_Read_Clear(unsigned char* posi)
{
	int  offset = posi - Uart_Read_COM2;

	if(UartReadCnt_2 == 0)
		return;

	if(offset < 0)
	{
		memset(Uart_Read_COM2,0,UartReadCnt_2);
		UartReadCnt_2 = 0;
		return;
	}
	if((offset > 0) && (offset <= UartReadCnt_2))
	{
		UartReadCnt_2 = UartReadCnt_2-offset;
		memcpy(Uart_Read_COM2,posi,UartReadCnt_2);

		memset(Uart_Read_COM2+UartReadCnt_2,0,offset);
	}
	return;
}
#endif


/****************************************************************
  ��������SG_Uart_Read_Clear
  ��  ��  �����ڻ������������
  ���������posi�����β��ָ���ַ
  �����������
  ��д��  ���º���
  �޸ļ�¼��������2006-7-7
****************************************************************/
void SG_Uart_Read_Clear(unsigned char* posi)
{
	int  offset = posi - Uart_Read_Buf;

	if(UartReadCnt == 0)
		return;

	if(offset < 0)
	{
		memset(Uart_Read_Buf,0,UartReadCnt);
		UartReadCnt = 0;
		return;
	}
	if((offset > 0) && (offset <= UartReadCnt))
	{
		UartReadCnt = UartReadCnt-offset;
		memcpy(Uart_Read_Buf,posi,UartReadCnt);
		
		memset(Uart_Read_Buf+UartReadCnt,0,offset);
	}
	return;
}


/*********************************���蹫����������************************/

/*************************************************************************/
/*********************************�Ե�����غ���**************************/
/*************************************************************************/

// �����Ե�����֡��ZB������
void GpsDevZB_Sendframe(SENDFRAME * sendframe)
{
	unsigned int i = 0,j = 0;
	char datalen;
	unsigned char temp;
	unsigned char buf[600] = "";
	unsigned char*ptr = NULL;
	//unsigned char len[4] = "";
	short len  =0;
	unsigned char checksum;
	static unsigned char ID = 1;

	SCI_TRACE_LOW("========GpsDevZB_Sendframe");

	if(g_set_info.GpsDevType == 1) // ZB������
	{
		//֡ͷ
		buf[i++] = ZB_GPS_START;
		
		//�����ֽ�
		buf[i++] = 0x00; 
		
		//֡��ID��
		temp = ID++;
		buf[i++] = temp;  
		checksum = temp;
		
		// ������
		temp = sendframe->type;
		buf[i++] = temp;
		checksum^=temp;
			
		//����
		temp = (unsigned char)((sendframe->len >> 8)&0xff);
		buf[i++] = temp;
		checksum^=temp;

		temp = (unsigned char)(sendframe->len & 0xff);
		buf[i++] = temp;
		checksum^=temp;
		//i += 2; 

		//��Ϣ��
		ptr = sendframe->dat;
		
		for(j=0;j<sendframe->len;j++)
		{
			temp = *ptr;

			switch(temp)
			{
#if (1)
				case ZB_GPS_START:
					buf[i++] = 0X7D;
					buf[i++] = 0x5E;
				//	len += 1;
					break;
				case ZB_GPS_END:
					buf[i++] = 0X7D;
					buf[i++] = 0X5F;
				//	len += 1;
					break;
				case ZB_GPS_SPE:
					buf[i++] = 0X7D;
					buf[i++] = 0X5D;
				//	len += 1;
					break;
#endif /* (0) */
				default:
					buf[i++] = temp;
					break;
			}
			checksum^=temp;
			ptr ++;
		}

		
		//У���
		switch(checksum)
		{
			case ZB_GPS_START:
				buf[i++] = 0X7D;
				buf[i++] = 0x5E;
				break;
				
			case ZB_GPS_END:
				buf[i++] = 0X7D;
				buf[i++] = 0X5F;
				break;
				
			case ZB_GPS_SPE:
				buf[i++] = 0X7D;
				buf[i++] = 0X5D;
				break;

			default:
				buf[i++] = checksum;
				break;
				
		}

		//֡β
		buf[i++] = ZB_GPS_END;

	}
	else
	{
		SCI_TRACE_LOW("<<<GpsDev type is err!!!");
		return;
	}

    if(((sendframe->type == CMD_ZB_ACK)&&(sendframe->dat[0] != CMD_SAFETY_SET))
		||(sendframe->type == CMD_CALL_RING)||(sendframe->type == CMD_CALL_SPEAK)||(sendframe->type == CMD_CALL_END))
    {
		g_state_info.GpsEnable = 1;
		SIO_WriteFrame(COM_DEBUG, buf, i);
		SCI_Sleep(5);
		SIO_WriteFrame(COM_DEBUG, buf, i);
		g_state_info.GpsEnable = 0;
	}
	else
	{
		SIO_WriteFrame(COM_DEBUG, buf, i);
	}



}





// �����Ե�����֡��������
void YHGpsDev_Sendframe(SENDFRAME * sendframe)
{
	unsigned int i = 0,j = 0;
	char datalen;
	char temp;
	unsigned char buf[350] = "";
	unsigned char*ptr = NULL;
	unsigned char len[4] = "";
	char checksum;

	if(g_set_info.GpsDevType == 2)// YH������
	{
		buf[i++] = YH_GPS_START;
		buf[i++] = sendframe->type; // ����
		sprintf((char *)len,"%02X",sendframe->len);
		memcpy(&buf[i],len,2);
		i += 2;
		ptr = sendframe->dat;
		
		SCI_TRACE_LOW("YHGpsDev_Sendframe:@_@_@type %02x  len %d ",sendframe->type,sendframe->len);

		for(j=0;j<sendframe->len;j++)
		{
			temp = *ptr;
			buf[i++] = temp;
			ptr ++;
		}

		checksum = CheckSum(&buf[1],1+2+sendframe->len);
		buf[i++] = checksum;

	//	SCI_TRACE_LOW("YHGpsDev_Sendframe: checksum %02x",checksum);

		buf[i++] = YH_GPS_END;
		buf[i++] = YH_GPS_END2;
	}
	else if(g_set_info.GpsDevType == 1) // ZB������
	{
		buf[i++] = ZB_GPS_START;
		
		buf[i++] = sendframe->type; // ����

	//	sprintf((char *)&buf[i],"%02x",sendframe->len);
		i += 2;


		SCI_TRACE_LOW("YHGpsDev_Sendframe:@_@_@type %02x  len %d ",sendframe->type,sendframe->len);
		ptr = sendframe->dat;
		
		for(j=0;j<sendframe->len;j++)
		{
			temp = *ptr;
			buf[i++] = temp;
#if (0)
			switch(temp)
			{
				case ZB_GPS_START:
					buf[i++] = 0X2F;
					buf[i++] = 'H';
					break;
				case ZB_GPS_END:
					buf[i++] = 0X2F;
					buf[i++] = 'T';
					break;
				case ZB_GPS_SPE:
					buf[i++] = 0X2F;
					buf[i++] = 0X2F;
					break;
				default:
					buf[i++] = temp;
					break;
			}
#endif /* (0) */
			ptr ++;
		}

		buf[i++] = ZB_GPS_END;
		sprintf((char *)len,"%02x",i-5);
		SCI_MEMCPY(&buf[2],len,2);
	}
	else
	{
		SCI_TRACE_LOW("<<<GpsDev type is err!!!");
		return;
	}

#if (0)
	{
		int m;
		for(m=0;m<i;m++)
		{
			SCI_TRACE_LOW("<<%02x",buf[m]);
		}
	}
#endif /* (0) */
	#if __gps_debug_
	for(j=0;j<i;j++)
	{
		HAL_DumpPutChar(COM_DATA,buf[j]);
	}
	#else
	SIO_WriteFrame(COM_DEBUG,buf,i);
	#endif

}



// ��̨���Ͷ�ʱ���ֵ�������
void YHGpsDev_Send_Handup(void)
{
	SCI_TRACE_LOW("YHGpsDev_Send_Handup!!");
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));

	g_gpsframe.type = CMD_YH_HEART;
	g_gpsframe.len = 1;
	g_gpsframe.dat[0] = 0;

	YHGpsDev_Sendframe(&g_gpsframe);
}

// ��ʾ�����͹̶��������̨�Ժ󣬳�̨���ͻظ�����ʾ��
void GpsDevReplyBack(void)
{
	SCI_TRACE_LOW("GpsDevReplyBack!!");
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	g_gpsframe.type = CMD_RCV_ACK;
	g_gpsframe.len = 1;
	strcpy((char *)g_gpsframe.dat,"1"); // ���״̬	

	YHGpsDev_Sendframe(&g_gpsframe);
}


// ��̨����������Ϣ��������
void YHGpsDev_Send_Msg(char *msg, int len, char *telenum, char type)
{

	int msglen = 0;
	SCI_DATE_T  currDate;
	SCI_TIME_T  currTime;
	char temp[10] = "";


	SCI_TRACE_LOW("YHGpsDev_Send_Msg len = %d!!",len);
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));



	TM_GetSysDate(&currDate);
	TM_GetSysTime(&currTime);
	time_trans(&currDate,&currTime);
	
	g_gpsframe.type = CMD_YH_RCVSMS;

	g_gpsframe.dat[0] = 'N'; // ���ͷ�����
	msglen += 1;

	memset(&g_gpsframe.dat[msglen],0xFF,20);
	memcpy(&g_gpsframe.dat[msglen],telenum,strlen(telenum));
	msglen += 20;

	g_gpsframe.dat[msglen] = 'T'; // ʱ��
	msglen += 1;

	
	sprintf((char *)&g_gpsframe.dat[msglen],"%02d",currDate.year-2000);
	sprintf((char *)&g_gpsframe.dat[msglen+2],"%02d",currDate.mon);
	sprintf((char *)&g_gpsframe.dat[msglen+4],"%02d",currDate.mday);
	sprintf((char *)&g_gpsframe.dat[msglen+6],"%02d",currTime.hour);
	sprintf((char *)&g_gpsframe.dat[msglen+8],"%02d",currTime.min);	
	sprintf((char *)&g_gpsframe.dat[msglen+10],"%02d",currTime.sec);

#if (0)
	{
		int i;
		for(i=0;i<12;i++)
			SCI_TRACE_LOW("==%02x",g_gpsframe.dat[msglen+i]);
	}
#endif /* (0) */
	SCI_MEMCPY(temp,(char *)&g_gpsframe.dat[msglen],12);
//	SCI_TRACE_LOW("YHGpsDev_Send_Msg  %s",temp);
	gsmInvertNumbers(&g_gpsframe.dat[msglen],temp,12);
//	SCI_TRACE_LOW("YHGpsDev_Send_Msg  %s afterchange",temp);

	SCI_MEMCPY(&g_gpsframe.dat[msglen],temp,12);

	msglen += 12;

	if(type == MN_SMS_DEFAULT_ALPHABET)
	{
		g_gpsframe.dat[msglen] = 'E'; // ��������
		msglen += 1;
	}
	else if(type == MN_SMS_UCS2_ALPHABET)
	{
		g_gpsframe.dat[msglen] = 'C'; // ��������
		msglen += 1;		
	}
		

	SCI_MEMCPY(&g_gpsframe.dat[msglen],msg,len);
	msglen += len;

	g_gpsframe.len = msglen;	
	
	YHGpsDev_Sendframe(&g_gpsframe);
}

//չ�������������Ϸ�����Ϣ
unsigned char ZB_GPS_Uart_Read(void)
{
	unsigned char *frameBegin = NULL,*frameEnd=NULL;
	unsigned char *readBuf = Uart_Read_Buf;
	int locklen=0;

	if(UartReadCnt > 0|| (SIO_ReadFrame(COM_DEBUG,(uint8*)Uart_Read_Buf,UART_READ_BUF,(uint32*)&UartReadCnt) > 0))
	{
		frameBegin = FindChar(readBuf,UartReadCnt+1,0x7E);
		if(frameBegin != NULL)
		{
			frameEnd=FindChar(frameBegin+1,UartReadCnt-(frameBegin-Uart_Read_Buf)+1,0x7F);
			if(frameEnd != NULL)
			{
				if(frameEnd==frameBegin+1)
				{
					SG_Uart_Read_Clear(frameEnd);
					return 0;
				}
				else
				{
					locklen=frameEnd-frameBegin+1;
					SCI_TRACE_LOW("SG_Lock_Uart_Read:UartReadCnt:%d,locklen:%d\r\n",UartReadCnt,locklen);
					return locklen;
				}
			}
		}
		if (UartReadCnt >= UART_READ_BUF -1)
		{	
			SCI_TRACE_LOW("SG_Lock_Uart_Read: read buf overflow:len:%d\n",UartReadCnt);
			memset(Uart_Read_Buf,0,sizeof(Uart_Read_Buf));
			UartReadCnt = 0;
		}
	}
	return 0;
}

//֪ͨ�������һ�(�����ն�)
void ZBGpsDev_SendCalloff(void)
{
	SCI_TRACE_LOW("@_@_@ZBGpsDev_SendCalloff!!!!");	
	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	g_gpsframe.type = CMD_CALL_END;
	g_gpsframe.len = 0x00;
	g_gpsframe.dat[0] = 0x00;
	GpsDevZB_Sendframe(&g_gpsframe);

}

//���ŷ���Ӧ��
void SG_ZBGPS_ACK(uint8 cmd, uint8 ack)
{

	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));

	g_gpsframe.type = CMD_ZB_ACK;
	g_gpsframe.len = 0x03;
	g_gpsframe.dat[0] = cmd;
	g_gpsframe.dat[1] = g_state_info.ZB_Num;
	g_gpsframe.dat[2] = ack;

	GpsDevZB_Sendframe(&g_gpsframe);	
	
}

//�������ֻ����Ͷ���Ϣ(������)
void SG_Sms_Down_Hh(uint8 type, unsigned char *msg, short msglen)
{
		char telnum[20]="";
		uint8 telen = 0;
		uint8 pTP_UD[200] ="";
		uint8 pTP_len =0;


		for(telen=0; telen<20; telen++)
		{
			if(msg[telen] == 0x00)
				break;
			else
				telnum[3+telen] = msg[telen];
		}
		telnum[0]='+';
		telnum[1]='8';
		telnum[2]='6';
		pTP_len = msglen-telen-3;

		if((pTP_len > 140) || (pTP_len < 2))
		{
			SG_ZBGPS_ACK(CMD_SMS_OUT, 0x01);
			return;
		}
		
		SCI_MEMCPY(pTP_UD, msg+telen+1, pTP_len);
		
		SendMsgReqToMN((uint8*)telnum, pTP_UD, pTP_len, MN_SMS_UCS2_ALPHABET);

		
}

//��绰
void ZB_ConnectCall(char* msg)
{
	
	if(g_set_info.bCallOutDisable==1)
	{
		if( 0 != strcmp(g_state_info.sAllowCall,""))
		{
				
			//	�жϵ�ǰincoming�Ƿ��������
			if (NULL == strstr(g_state_info.sAllowCall, msg))
			{
				SCI_TRACE_LOW("<<<<<<The Number is been reject:%s", msg);// ���벻������
				ZBGpsDev_SendCalloff();
				return;
			}
		}
		else
		{
			SCI_TRACE_LOW("THE sAllowCall LIST is NULL");
			ZBGpsDev_SendCalloff();
			return;
		}
	}	

	CC_ConnectCall(msg);

}

//��ȫ��������
void ZB_Safety_Set(unsigned char* msg)
{

	switch(msg[0])
	{
		//�������
		case 1:
			{
				//��
				if(msg[1] == 1)
				{
					SG_Set_Cal_CheckSum();
					g_set_info.GPSEnable = 0;
					SG_Set_Save();
					SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x00);
				}
				//��
				else if(msg[1] == 0)
				{
					SG_Set_Cal_CheckSum();
					g_set_info.GPSEnable = 1;
					SG_Set_Save();
					SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x00);
				}
				else
				{
					SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x01);
				}

			}
			break;
		//��������
		case 2:
			{
				//��
				if(msg[1] == 1)
				{
					if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
					{
						SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x01);
					}
					else
					{
						SG_Set_Cal_CheckSum();
						g_set_info.nTheftState = 1;
						SG_Set_Save();
						SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x00);
					}
				}
				//��
				else if(msg[1] == 0)
				{
					if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
					{
						SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x01);
					}
					else
					{
						SG_Set_Cal_CheckSum();
						g_set_info.nTheftState = 0;
						if(g_state_info.alarmState & GPS_CAR_STATU_ROB_ALARM)
						{
							g_state_info.alarmState &= ~GPS_CAR_STATU_ROB_ALARM;
							g_set_info.alarmState &= ~GPS_CAR_STATU_ROB_ALARM;
						}
						SG_Set_Save();		
						SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x00);
						Pic_Send(0xc5); 
				
					}
				}
				else
				{	
					SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x01);
				}
			}
			break;
		//��ȫ������������
		case 3:
			{
				if(strlen((char*)msg+1) == 6)
				{
					int i;
					char psw[7];
					for(i=0; i<6; i++)
					{
						psw[i] = msg[1+i]+'0';
					}
					
					SG_Set_Cal_CheckSum();
					SCI_MEMSET((char*)g_set_info.sSafetyPsw, 0, 8);
					strncpy((char*)g_set_info.sSafetyPsw, psw, 6);
					SG_Set_Save();	 
					SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x00);
				}
				else
				{
					SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x01);
				}
				
			}		
			break;
		default:
			{
				SG_ZBGPS_ACK(CMD_SAFETY_SET, 0x01);
			}
			break;
	}
}



//����չ�������������Ϸ�����Ϣ
void ZB_GPS_Uart_Handle(void)
{

	unsigned char *framePtr = NULL;
	unsigned char *frameEnd = NULL;
	unsigned char buf[1024]="";
	unsigned char msg[1024]="";
	short msglen;
	int i = 0,j = 0,temp = 0;
	unsigned char checksum = 0;
	uint8 len;
	uint8 type;
	uint8 num;

	framePtr = FindChar(Uart_Read_Buf,UartReadCnt+1,0x7e);
	if(framePtr == NULL)
	{
		return ;
	}
	
	frameEnd = FindChar(framePtr+1,UartReadCnt-(framePtr-Uart_Read_Buf)+1,0x7F);
	if(frameEnd == NULL)
	{
		return ;
	}
	
	//֡����
	len = frameEnd-framePtr+1;
	
	//ת��
	for(i=1; i<len-1; i++)
	{
		if(*(framePtr+i) == 0x7d)
		{
			i++;
			buf[j++] = (*(framePtr+i)^0x20);
		}
		else 
		{
			buf[j++] = *(framePtr+i);
		}
	}
	
	//֡��ˮ��
	g_state_info.ZB_Num = buf[1];
	
	//У��
	for(i=0; i<j; i++)
	{
		checksum^=buf[i];
	}

	if(checksum)
	{
		ZBGpsDev_SendCalloff();
		return;
	}
	else
	{
		SCI_MEMSET(Uart_Read_Buf,0,UART_READ_BUF);
	}

		
#if (0)
	SG_ZBGPS_ACK(CMD_CALL_OUT, 0x00);
		return;
#endif /* (0) */

	//���ݳ���
	msglen = buf[3];
	msglen = (msglen<<8)+buf[4];
	
	//����
	SCI_MEMCPY(msg, buf+5, msglen);

	//֡����
	type = buf[2];

	
	switch(type)
	{
		//���ֻ�������Ϣ
		case CMD_SMS_OUT:
			{
				SG_Sms_Down_Hh(type, msg, msglen);				
			}
			break;
		//����һ����������
		case CMD_CALL_CENTER:
			{
				if(strlen(g_set_info.sOneGPSNo) == 0)
				{
					ZBGpsDev_SendCalloff();
				}
				else
				{
					ZB_ConnectCall((char*)g_set_info.sOneGPSNo);
				}
				
			}
		break;
		//��������ȫ��������
		case CMD_SAFETY_SET:
			{
				ZB_Safety_Set(msg);
			}
			break;
		//����绰
		case CMD_CALL_OUT:
			{
				ZB_ConnectCall((char*)msg);
			}
			break;
		//�绰����
		case CMD_CALL_REV:
			{
				ConnectPhoneTimer = 0;
				AUD_StopRing();
				AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
				AUD_EnableVoiceCodec( 1 );
#ifndef _TRACK
				GPIO_SetValue(HF_MUTE,SCI_FALSE);
#endif
				MNCALL_ConnectCall(CurrCallId,PNULL);
			}
		  	break;
		//�绰�һ�
		case CMD_CALL_OFF:
			{
				if(MN_RETURN_SUCCESS != MNCALL_DisconnectCall(CurrCallId, PNULL))
				{
					SCI_TRACE_LOW("<<<<<<ccapp.c:CC_DisconnectCall() MNCALL_DisconnectCall return failure");
					
				}
			}
			break;
		//����ֻ�
		case CMD_CALL_DTMF:
			SG_Send_DTMF(msg);
			break;
			
		//��������
		case CMD_CALL_VOL:
			{
				
				SG_Set_Cal_CheckSum();
				g_set_info.speaker_vol = msg[0]-'0';
				SG_Set_Save();
				if(AUD_GetDevMode() == AUD_DEV_MODE_HANDFREE)
				{
					AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����			
				}
				else if(AUD_GetDevMode() == AUD_DEV_MODE_HANDHOLD)
				{
					AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����			
				}
			}
			break;
		default:
			break;
	}

}

//����GPS��Ϣ
void  SG_ZBGPS_Send(uint8* msg, unsigned short msglen)
{
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	g_gpsframe.type = CMD_GPS_SEND;
	g_gpsframe.len = msglen;
	SCI_MEMCPY(g_gpsframe.dat, msg, msglen);
	GpsDevZB_Sendframe(&g_gpsframe);
}

//֪ͨ��������ͨ(�����ն�)
void ZBGpsDev_SendAnsCall(void)
{
	SCI_TRACE_LOW("@_@_@ZBGpsDev_SendAnsCall!!!!");	
	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	g_gpsframe.type = CMD_CALL_SPEAK;
	g_gpsframe.len = 2;
	g_gpsframe.dat[0] = g_set_info.speaker_vol+'0';
	g_gpsframe.dat[1] = '9';
	GpsDevZB_Sendframe(&g_gpsframe);

}
//��������(�����ն�)
void ZBGpsDev_SendIncoming(char *tel_num)
{
	SCI_TRACE_LOW("@_@_@ZBGpsDev_SendIncoming!!!!");	
	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	
	g_gpsframe.type = CMD_CALL_RING;
	g_gpsframe.len = strlen(tel_num);
	strcpy((char *)g_gpsframe.dat,tel_num);

	GpsDevZB_Sendframe(&g_gpsframe);


}

//�����ֻ������ѺƵ������Ķ���
void SG_Sms_YH(xSignalHeaderRec   *receiveSignal)
{
	short msglen;
	unsigned char msg[300] = "";
	char tele_num[23] =""; 
	MN_SMS_ALPHABET_TYPE_E type = 0;
	uint8 tele_len;
	
	
	type = *((char*)receiveSignal + 17);    //��������
	msglen = *((char*)receiveSignal + 18);	//�������ݳ���
	memcpy(msg,(char*)receiveSignal + 19,msglen); //��������
	tele_len = *((char*)receiveSignal + 19+msglen);//���ź��볤��
	memcpy(tele_num,(char*)receiveSignal + 19+msglen+1,tele_len); //���ź���

	YHGpsDev_Send_Msg((char *)msg, msglen, tele_num, type);
	
}

//�����ֻ�����չ���������Ķ���
void SG_Sms_ZB(xSignalHeaderRec   *receiveSignal)
{
	uint8 len = 0;
	short msglen;
	unsigned char msg[300] = "";
	char tele_num[23] =""; 
	MN_SMS_ALPHABET_TYPE_E type = 0;
	uint8 tele_len;
	char tt[5] = "";
	int i;
	gps_data *gps= (gps_data *)g_state_info.pGpsCurrent;	
	
	//len = *((char*)receiveSignal + 16);
	type = *((char*)receiveSignal + 17);    //��������
	msglen = *((char*)receiveSignal + 18);	//�������ݳ���
	memcpy(msg,(char*)receiveSignal + 19,msglen); //��������
	tele_len = *((char*)receiveSignal + 19+msglen);//���ź��볤��
	memcpy(tele_num,(char*)receiveSignal + 19+msglen+1,tele_len); //���ź���

	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	
	if((msg[0] == 0x00)&&(msg[1] == '*'))
	{
		uint8 temp;
		int n_long;
		int n_lat;

		temp = msg[3];
		switch(temp)
		{
			case 0x30:
			case 0x31:
			case 0x32:
				{
					unsigned int n_long = 0;
					unsigned int n_lat = 0;
					int kk = 0;
					int aa = 0;
					int i=0;
					char tel[20] ="";

					n_lat = (msg[5]-'0');
					n_lat = (n_lat*10)+(msg[7]-'0');
					n_lat = (n_lat*10)+(msg[9]-'0');
					n_lat = (n_lat*10)+(msg[11]-'0');
					n_lat = (n_lat*10)+(msg[13]-'0');
					n_lat = (n_lat*10)+(msg[15]-'0');
					n_lat = (n_lat*10)+(msg[17]-'0');
					n_lat = (n_lat*10)+(msg[19]-'0');
					n_lat = (n_lat*10)+(msg[21]-'0');
					n_lat = (unsigned int)(n_lat*3.6);
					
					n_long = (msg[23]-'0');
					n_long = (n_long*10)+(msg[25]-'0');
					n_long = (n_long*10)+(msg[27]-'0');
					n_long = (n_long*10)+(msg[29]-'0');
					n_long = (n_long*10)+(msg[31]-'0');
					n_long = (n_long*10)+(msg[33]-'0');
					n_long = (n_long*10)+(msg[35]-'0');
					n_long = (n_long*10)+(msg[37]-'0');
					n_long = (unsigned int)(n_long*3.6);

					g_gpsframe.dat[0] = temp-'0';
					kk += 1;
					SCI_MEMCPY(g_gpsframe.dat+kk, &n_lat, 4);
					kk += 4;
					SCI_MEMCPY(g_gpsframe.dat+kk, &n_long, 4);
					kk += 4;
					while((aa<30) && (msg[39+aa]!='*'))
					{
						tel[i++] = msg[39+aa];
						aa += 2;
					}
						
					SCI_MEMCPY(g_gpsframe.dat+kk, tel, 20);
					kk += 20;
					SCI_MEMCPY(g_gpsframe.dat+kk, aa+2+msg+38, 150);
					
					g_gpsframe.type = CMD_LETTER_UP;
					g_gpsframe.len =  179;
					GpsDevZB_Sendframe(&g_gpsframe);	
					
				}
				break;

			case 0x33:
				{
					int gbklen;
					unsigned char gbk[200] ="";
					g_gpsframe.type = CMD_LETTER_UP;
					g_gpsframe.dat[0] = 0x03;
					//SCI_MEMCPY(g_gpsframe.dat+1, msg+4, msglen-4);
					//g_gpsframe.len = msglen+1-4;

					UnicodeBE2LE(msg, msglen);
					gbklen = Unicode2GBK(gbk, msg+4, msglen-4);
					SCI_MEMCPY(g_gpsframe.dat+1, gbk, gbklen);
					g_gpsframe.len = gbklen+1;
					GpsDevZB_Sendframe(&g_gpsframe);	
				}
				break;
			case 0x34:
				{
					int kk = 0;
					int gbklen;
					unsigned char gbk[200] ="";
					
					g_gpsframe.type = CMD_LETTER_UP;
					g_gpsframe.dat[0] = 0x04;

					UnicodeBE2LE(msg, msglen);
					gbklen = Unicode2GBK(gbk, msg+4, msglen-4);

					
					SCI_MEMCPY(g_gpsframe.dat+1, gbk, gbklen);
					g_gpsframe.len = gbklen+1;
					GpsDevZB_Sendframe(&g_gpsframe);	
				}
				break;
			//�������
			case 0x35:
			case 0x36:
			case 0x37:
				{
					int kk = 0;
					int aa = 0;
					int i=0;
					char tel[20] ="";
					unsigned char gbk[200] ="";
					int gbklen = 0;


					g_gpsframe.type = CMD_LETTER_UP;
					g_gpsframe.len =  179;
					g_gpsframe.dat[0] = temp-'0'-5;
					kk += 1;

					UnicodeBE2LE(msg, msglen);
					gbklen = Unicode2GBK(gbk, msg+4, msglen-4);
					
					SCI_MEMSET(g_gpsframe.dat+kk, 0, 8);
					kk += 8;

					while((aa<15) && (gbk[aa]!='('))
					{
						tel[i++] = gbk[aa];
						aa += 1;
					}
		
					SCI_MEMCPY(g_gpsframe.dat+kk, tel, 20);
					kk += 20;
					SCI_MEMCPY(g_gpsframe.dat+kk, gbk+aa, 150);
					
					GpsDevZB_Sendframe(&g_gpsframe);	
					
					
				}
				break;
			case 0x39:
				{
					unsigned int n_long = 0;
					unsigned int n_lat = 0;
					int kk = 0;
					int gbklen;
					unsigned char gbk[200] ="";


					n_lat = (msg[5]-'0');
					n_lat = (n_lat*10)+(msg[7]-'0');
					n_lat = (n_lat*10)+(msg[9]-'0');
					n_lat = (n_lat*10)+(msg[11]-'0');
					n_lat = (n_lat*10)+(msg[13]-'0');
					n_lat = (n_lat*10)+(msg[15]-'0');
					n_lat = (n_lat*10)+(msg[17]-'0');
					n_lat = (n_lat*10)+(msg[19]-'0');
					n_lat = (n_lat*10)+(msg[21]-'0');
					n_lat = (unsigned int)(n_lat*3.6);
					
					n_long = (msg[23]-'0');
					n_long = (n_long*10)+(msg[25]-'0');
					n_long = (n_long*10)+(msg[27]-'0');
					n_long = (n_long*10)+(msg[29]-'0');
					n_long = (n_long*10)+(msg[31]-'0');
					n_long = (n_long*10)+(msg[33]-'0');
					n_long = (n_long*10)+(msg[35]-'0');
					n_long = (n_long*10)+(msg[37]-'0');
					n_long = (unsigned int)(n_long*3.6);

					SCI_MEMCPY(g_gpsframe.dat+kk, &n_lat, 4);
					kk += 4;
					SCI_MEMCPY(g_gpsframe.dat+kk, &n_long, 4);
					kk += 4;

					UnicodeBE2LE(msg, msglen);
					gbklen = Unicode2GBK(gbk, msg+38, msglen-38);
					
					SCI_MEMCPY(g_gpsframe.dat+kk, gbk, gbklen);
					g_gpsframe.type = CMD_CENTER_UP;
					g_gpsframe.len =  kk+gbklen;
					GpsDevZB_Sendframe(&g_gpsframe);	
				}
				break;
				
			default:
				break;
				
				
		}
			
	}
	else 
	{
		g_gpsframe.type = CMD_SMS_UP;
		SCI_MEMCPY(g_gpsframe.dat, tele_num+3, tele_len-3);
		len = tele_len-3;
		g_gpsframe.dat[len] = 0x00;
		len +=1;

		//���ڡ�ʱ��
		tt[0] = gps->sDate[4];
		tt[1] = gps->sDate[5];	
		g_gpsframe.dat[len] = (char)atoi(tt);
		len += 1;

		tt[0] = gps->sDate[2];
		tt[1] = gps->sDate[3];	
		g_gpsframe.dat[len] = (char)atoi(tt);
		len += 1;

		tt[0] = gps->sDate[0];
		tt[1] = gps->sDate[1];	
		g_gpsframe.dat[len] = (char)atoi(tt);
		len += 1;

		tt[0] = gps->sTime[0];
		tt[1] = gps->sTime[1];	
		g_gpsframe.dat[len] = (char)atoi(tt);
		len += 1;

		tt[0] = gps->sTime[2];
		tt[1] = gps->sTime[3];	
		g_gpsframe.dat[len] = (char)atoi(tt);
		len += 1;

		tt[0] = gps->sTime[4];
		tt[1] = gps->sTime[5];	
		g_gpsframe.dat[len] = (char)atoi(tt);
		len += 1;

		if(type == MN_SMS_DEFAULT_ALPHABET)
			g_gpsframe.dat[len] = 0x00;
		else if(type == MN_SMS_UCS2_ALPHABET)
			g_gpsframe.dat[len] = 0x01;
		len += 1;

		SCI_MEMCPY(g_gpsframe.dat+len, msg, msglen);

		len += msglen;
		g_gpsframe.len = len;
		
		GpsDevZB_Sendframe(&g_gpsframe);	
		
	}

	
}

// ��̨����״̬��Ϣ��չ����ʾ�����������ź�ǿ�ȣ�������Ӫ�̣�GPS��λ״̬��GPRS״̬��ACC״̬��
void GpsDevZB_Send_State(void)
{
	char buf[5] = "";
	int i = 0;

	SCI_TRACE_LOW("@_@_@YHGpsDev_Send_State!!!!");	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));

	if(g_set_info.GpsDevType == 1)
	{
		g_gpsframe.type = CMD_STATE_UP;

		
		g_gpsframe.len = 8;
		sprintf((char *)&g_gpsframe.dat[i++],"%d",g_state_info.rxLevelnum);// �ź�ǿ�ȵȼ�

		//  N:û��������Ӫ��       Y:�й��ƶ�       L:�й���ͨ       Q:����

		if(g_state_info.opter == 0) // û��������Ӫ��
		{
			g_gpsframe.dat[i++] = 'N'; 		
		}
		else if(g_state_info.opter == 46000) // �й��ƶ�
		{
			g_gpsframe.dat[i++] = 'Y'; 
		}
		else if(g_state_info.opter == 46001)// �й���ͨ
		{
			g_gpsframe.dat[i++] = 'L'; 
		}
		else                          // ������Ӫ��
		{
			g_gpsframe.dat[i++] = 'Q'; 
		}
		
		// V:û�ж�λ A:�Ѿ���λ

		if(((gps_data *)(g_state_info.pGpsCurrent))->status)// �Ѿ���λ
		{
			g_gpsframe.dat[i++] = 'A'; 
		}
		else
		{
			g_gpsframe.dat[i++] = 'V'; 
		}
		
		//  S:����״̬  G:GPRS״̬

		if(g_set_info.nNetType == 0)
		{
			g_gpsframe.dat[i++] = 'S';
		}
		else if(g_set_info.nNetType == 1)
		{
			g_gpsframe.dat[i++] = 'G';
		}	
		else if(g_set_info.nNetType == 2 && g_state_info.socket > 0)
		{
			g_gpsframe.dat[i++] = 'G';
		}
		else if(g_set_info.nNetType == 2 && g_state_info.socket < 0)
		{
			g_gpsframe.dat[i++] = 'S';
		}

	//	ACC״̬  0:ACCϨ��  1:ACC���
		if(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)
		{
			g_gpsframe.dat[i++] = '1';	
		}
		else
		{
			g_gpsframe.dat[i++] = '0';	
		}

		// ���߱�־λ 0 δ���� 1 ����   
		if((g_gprs_t.state == GPRS_RXHEAD)||(g_gprs_t.state == GPRS_RXCONTENT))
		{
			g_gpsframe.dat[i++] = '1';	
		}
		else
		{
			g_gpsframe.dat[i++] = '0';	
		}

		//��ر�־
		if(g_set_info.GPSEnable == 0)
		{
			g_gpsframe.dat[i++] = '1';	
		}
		else if(g_set_info.GPSEnable == 1)
		{
			g_gpsframe.dat[i++] = '0';	
		}
		//������־
		if(g_set_info.nTheftState == 1)
		{
			g_gpsframe.dat[i++] = '1';	
		}
		else if(g_set_info.nTheftState == 0)
		{
			g_gpsframe.dat[i++] = '0';	
		}
			
		
	}
	else
	{
		SCI_TRACE_LOW("<<<GpsDev type is err!!!");
		return;
	}

	GpsDevZB_Sendframe(&g_gpsframe);
	
}


// ��̨����״̬��Ϣ����ʾ�����������ź�ǿ�ȣ�������Ӫ�̣�GPS��λ״̬��GPRS״̬��ACC״̬��
void YHGpsDev_Send_State(void)
{
	char buf[5] = "";
	int i = 0;

	SCI_TRACE_LOW("@_@_@YHGpsDev_Send_State!!!!");	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	if(g_set_info.GpsDevType == 2)
	{
		g_gpsframe.type = CMD_YH_STATE;

		g_gpsframe.len = 22;

	//	SCI_MEMSET(g_gpsframe.dat,0xFF,20);
		if(g_state_info.opter == 46000) // �й��ƶ�
		{
			SCI_MEMCPY((char *)g_gpsframe.dat,"CT-GSM",strlen("CT-GSM"));
		}
		else if(g_state_info.opter == 46001)// �й���ͨ
		{
			SCI_MEMCPY((char *)g_gpsframe.dat,"CU-GSM",strlen("CU-GSM"));		
		}

		g_gpsframe.dat[20] = 0x2C;

		sprintf((char *)&g_gpsframe.dat[21],"%d",g_state_info.rxLevelnum+2);
	}
	else if(g_set_info.GpsDevType == 1)
	{
		g_gpsframe.type = CMD_STATE_UP;

		
		g_gpsframe.len = 5;
		sprintf((char *)&g_gpsframe.dat[i++],"%d",g_state_info.rxLevelnum);// �ź�ǿ�ȵȼ�

		//  N:û��������Ӫ��       Y:�й��ƶ�       L:�й���ͨ       Q:����

		if(g_state_info.opter == 0) // û��������Ӫ��
		{
			g_gpsframe.dat[i++] = 'N'; 		
		}
		else if(g_state_info.opter == 46000) // �й��ƶ�
		{
			g_gpsframe.dat[i++] = 'Y'; 
		}
		else if(g_state_info.opter == 46001)// �й���ͨ
		{
			g_gpsframe.dat[i++] = 'L'; 
		}
		else                          // ������Ӫ��
		{
			g_gpsframe.dat[i++] = 'Q'; 
		}
		
		// V:û�ж�λ A:�Ѿ���λ

		if(((gps_data *)(g_state_info.pGpsCurrent))->status)// �Ѿ���λ
		{
			g_gpsframe.dat[i++] = 'A'; 
		}
		else
		{
			g_gpsframe.dat[i++] = 'V'; 
		}
		
		//  S:����״̬  G:GPRS״̬

		if(g_set_info.nNetType == 0)
		{
			g_gpsframe.dat[i++] = 'S';
		}
		else if(g_set_info.nNetType == 1)
		{
			g_gpsframe.dat[i++] = 'G';
		}	
		else if(g_set_info.nNetType == 2 && g_state_info.socket > 0)
		{
			g_gpsframe.dat[i++] = 'N';
		}
		else if(g_set_info.nNetType == 2 && g_state_info.socket < 0)
		{
			g_gpsframe.dat[i++] = 'S';
		}

	//	ACC״̬  0:ACCϨ��  1:ACC���
		if(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)
		{
			g_gpsframe.dat[i++] = '1';	
		}
		else
		{
			g_gpsframe.dat[i++] = '0';	
		}
	}
	else
	{
		SCI_TRACE_LOW("<<<GpsDev type is err!!!");
		return;
	}

	YHGpsDev_Sendframe(&g_gpsframe);
	
}

void GpsDev_DialNum(char *buf)
{
	int len = 0;
	char telnum[20] = "";
	char tele[20] = "";
	char temp[5] = "";	 
	
	SCI_MEMCPY(temp,&buf[2],2);
	sscanf(temp,"%02x",&len);
//	len = atoi(temp);
	SCI_MEMCPY(telnum,&buf[4],len);

	if((telnum[0] == '+') && (telnum[1] == '8') && (telnum[2] == '6'))
	{
		memcpy(tele, telnum+3, len-3);
	}
	else
	{
		memcpy(tele, telnum, len);
	}
	

	SCI_TRACE_LOW("<<GpsDev_DialNum:telnum %s len %d",telnum,len);
	
	if(CC_RESULT_SUCCESS != CC_ConnectCall(tele))
	{
		SCI_TRACE_LOW("<<<<<<SG_Send_Call_Handle:connect No Failure!!");
	}
	else
	{
		SCI_TRACE_LOW("<<<<<<SG_Send_Call_Handle:connect No Successful!!");
	}

}


void GpsDev_SendSms(char *buf)
{
	int len = 0;
	int i;
	char temp[300] = "";
	char msglen[3] = "";
	char telnum[30] = "";
	
	SCI_MEMCPY(msglen,&buf[2],2);
	sscanf(msglen,"%02x",&len);
	SCI_TRACE_LOW("<<<<<<<<<msg len %d",len);

	len -= 22;

	SCI_MEMCPY(telnum,&buf[5],20);
	for(i=0;i<20;i++)
	{
		if(telnum[i] == 0xff)
		{
			telnum[i] = 0;
		}
	}
	
	SCI_TRACE_LOW("GpsDev_SendSms: telnum %s len %d",telnum,len);
	
	SCI_MEMCPY(temp,&buf[26],len);
	SendMsgReqToMN((uint8 *)telnum,(uint8*)temp,len,MN_SMS_UCS2_ALPHABET);
}



void GpsDev_SendAnsCall(void)
{
	SCI_TRACE_LOW("@_@_@GpsDev_SendAnsCall!!!!");	
	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	g_gpsframe.type = CMD_YH_SINBYTE;
	g_gpsframe.len = 1;
	g_gpsframe.dat[0] = SINBYTEDN_MOANS;
	YHGpsDev_Sendframe(&g_gpsframe);

}

void GpsDev_SendDTMF(char *buf)
{
	int ret;
	uint8 dtmf = buf[4];

	SCI_TRACE_LOW("<<GpsDev_SendDTMF:dtmf %c",dtmf);
	
	if(PhoneFlag == 2) // ��ͨ�绰�Ժ�
	{
	  	if((ret = MNCALL_StartDTMF(dtmf, CurrCallId)) != MN_RETURN_SUCCESS)
        {
           SCI_TRACE_LOW("<<<<<<Send DTMF err!!! %d",ret);
        }
        //stop send dtmf
        if((ret == MNCALL_StopDTMF(CurrCallId)) != MN_RETURN_SUCCESS)
        {
           SCI_TRACE_LOW("<<<<<<Stop Send DTMF err!!! %d",ret);
        }

	}

}
void GpsDev_SendCalloff(void)
{
	SCI_TRACE_LOW("@_@_@GpsDev_SendCalloff!!!!");	
	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	g_gpsframe.type = CMD_YH_SINBYTE;
	g_gpsframe.len = 1;
	g_gpsframe.dat[0] = SINBYTEDN_MOOFF;
	YHGpsDev_Sendframe(&g_gpsframe);

}

void GpsDev_SendSmsResult(int result)
{
	SCI_TRACE_LOW("@_@_@GpsDev_SendSmsResult!!!!");	
	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	g_gpsframe.type = CMD_YH_SINBYTE;
	g_gpsframe.len = 1;
	if(result == 0) // �ɹ�
	{
		g_gpsframe.dat[0] = SINBYTEDN_SENDOK;
	}
	else if(result == 1)// ʧ��
	{
		g_gpsframe.dat[0] = SINBYTEDN_SENDERR;		
	}
	else
	{
		return;
	}

	YHGpsDev_Sendframe(&g_gpsframe);

}

void GpsDev_SendIncoming(char *tel_num)
{
	SCI_TRACE_LOW("@_@_@GpsDev_SendIncoming!!!!");	
	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	
	g_gpsframe.type = CMD_YH_INCOMING;
	g_gpsframe.len = strlen(tel_num);
	strcpy((char *)g_gpsframe.dat,tel_num);

	YHGpsDev_Sendframe(&g_gpsframe);


}

// ���͵���Ŀ�ĵ�ַ��������
void GpsDev_SendDestAddr(char *lat,char *lan)
{
	int len = 0;

	
	SCI_TRACE_LOW("@_@_@GpsDev_SendDestAddr!!!!");	
	
	SCI_MEMSET(&g_gpsframe,0,sizeof(g_gpsframe));
	
	g_gpsframe.type = CMD_DST_UP;
	g_gpsframe.len = 0x13;
	
	SCI_MEMCPY(g_gpsframe.dat,lat,strlen(lat));
	len += strlen(lat);
	
	g_gpsframe.dat[len] = 'N';
	len += 1;
	
	SCI_MEMCPY(&g_gpsframe.dat[len],lan,strlen(lan));
	len += strlen(lan);

	g_gpsframe.dat[len] = 'E';
	len += 1;
	
	YHGpsDev_Sendframe(&g_gpsframe);
	
}


// �Ե�����ʱ�������ֺ�״̬
void SG_GpsDev_Handle(void)
{
	if(!(g_set_info.bNewUart4Alarm & UT_GPS))
	{
		return;
	}
	// ��YH��������ʱ����(ÿ��1����)
	if(g_set_info.GpsDevType == 2) // YH������
	{

		if(SecTimer%15 == 0)
		{
			YHGpsDev_Send_State();
		}
		
		if(SecTimer%3 == 0)
		{
			YHGpsDev_Send_Handup();
		}
		
	}
	// ����״̬��Ϣ��������(5sһ��)
	else if(g_set_info.GpsDevType == 1)
	{
		if(SecTimer % 6 == 0)
		{
			GpsDevZB_Send_State();
		}
			
	}
}


// �Ե������ڴ���
void YHGPS_Uart_Handle(void)
{
	unsigned char *buf = Uart_Read_Buf;
	int len;
	
	len = UartReadCnt;

#if (0)			
	{						
		SIO_WriteFrame(COM2, buf, len);				
	}
#endif /* (0) */


	if(buf[0] != YH_GPS_START)
	{
		SCI_TRACE_LOW("YHGPS_Uart_Handle err !!!!");
		g_state_info.nGPSDevErrCnt ++;
		return;
	}

	switch(buf[1])
	{
		case CMD_YH_SINBYTE: // ���ֽ�ָ��
			g_state_info.nGPSDevOtherAckCnt ++;
			switch(buf[4])
			{
				case SINBYTEUP_HEART: 	// �ն˹�������
					SCI_TRACE_LOW("@_@@@@@       SINBYTEUP_HEART");
					g_state_info.nGPSDevHandupAckCnt ++;			
					break;
				case SINBYTEUP_ANSCALL: // ��������
					SCI_TRACE_LOW("@_@@@@@       SINBYTEUP_ANSCALL");
					ConnectPhoneTimer = 0;
					AUD_StopRing();
					AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
					AUD_EnableVoiceCodec( 1 );
#ifndef _TRACK
					GPIO_SetValue(HF_MUTE,SCI_FALSE);
#endif
					MNCALL_ConnectCall(CurrCallId,PNULL);
					break;
				case SINBYTEUP_CALLOFF:	// �Ҷ�����
					SCI_TRACE_LOW("@_@@@@@       SINBYTEUP_CALLOFF");
					// ֱ�ӹҶ�
					if(MN_RETURN_SUCCESS != MNCALL_DisconnectCall(CurrCallId, PNULL))
					{
						SCI_TRACE_LOW("<<<<<<ccapp.c:CC_DisconnectCall() MNCALL_DisconnectCall return failure");
					}

					break;
				case SINBYTEUP_MEDREQ:	// ҽ�Ʒ������� 
					break;
				case SINBYTEUP_FALREQ:  // ά�޷�������
					break;
				case SINBYTEUP_INFOREQ: // ��Ϣ�������� �������ĺ���
					CC_ConnectCall(g_set_info.sOneGPSNo);
					break;
				case SINBYTEUP_HANDFREE:// �����������л�ͨ��
					break;
				case SINBYTEUP_VOLUP:   // ��������
					{
						SG_Set_Cal_CheckSum();
						g_set_info.speaker_vol += 1 ;
						if(g_set_info.speaker_vol >= 7)
							g_set_info.speaker_vol = 7;
						SG_Set_Save();
						if(AUD_GetDevMode() == AUD_DEV_MODE_HANDFREE)
						{
							AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����			
						}
						else if(AUD_GetDevMode() == AUD_DEV_MODE_HANDHOLD)
						{
							AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����			
						}
					}
					break;
				case SINBYTEUP_VOLDN:   // ��������
					{
						SG_Set_Cal_CheckSum();
						g_set_info.speaker_vol -= 1 ;
						if(g_set_info.speaker_vol <= 1)
							g_set_info.speaker_vol = 1;
						SG_Set_Save();
						if(AUD_GetDevMode() == AUD_DEV_MODE_HANDFREE)
						{
							AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����			
						}
						else if(AUD_GetDevMode() == AUD_DEV_MODE_HANDHOLD)
						{
							AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����			
						}
					}
					break;
				case SINBYTEUP_CONNECT: // �ն�������
					break;
			}
			break;
		case CMD_YH_DIALNUM: // ��������
			SCI_TRACE_LOW("@_@@@@@       CMD_YH_DIALNUM");
			GpsDev_DialNum((char *)buf);
			break;
		case CMD_YH_SENDSMS: // ���Ͷ���Ϣ (�ֳ��ն���̨���Ͷ���Ϣ����ʱ����)
			SCI_TRACE_LOW("@_@@@@@       CMD_YH_SENDSMS");
			{
				int i;
				for(i=0;i<len;i++)
					SCI_TRACE_LOW("=%02x",buf[i]);
			}
			GpsDev_SendSms((char *)buf);
			break;
		case CMD_YH_SENDDTMF: // ����DTMF(�ֳ��ն���̨���Ͷ���Ϣ����ʱ����)
			SCI_TRACE_LOW("@_@@@@@       CMD_YH_SENDDTMF");
			GpsDev_SendDTMF((char *)buf);
			break;
		default:
			break;
			
	}


}


void SG_GPS_UartRead(void)
{
	if(g_set_info.GpsDevType == 1)
	{
		int count = 0;
		while(SIO_ReadFrame(COM_DEBUG,Uart_Read_Buf+UartReadCnt,UART_READ_BUF,(uint32*)&count) > 0)
		{
		
			UartReadCnt += count;
			if(Uart_Read_Buf[UartReadCnt-1] == 0x7F)
			{
				ZB_GPS_Uart_Handle();
				UartReadCnt = 0;
			}
		}	
	}
	else if(g_set_info.GpsDevType == 2)
	{
		int count = 0;
		while(SIO_ReadFrame(COM_DEBUG,Uart_Read_Buf+UartReadCnt,UART_READ_BUF,(uint32*)&count) > 0)
		{
		
			UartReadCnt += count;
			if((Uart_Read_Buf[UartReadCnt-1] == 0xF0))
			{
				YHGPS_Uart_Handle();
				UartReadCnt = 0;
			}
		}			
	}
	
}

/******************************�Ե�������*********************************/


#ifdef _DUART // �ع���������˫�����Ӱ汾
/*****************************�ع�������غ���*******************************/

/****************************************************************
  ��������SG_Rcv_Safety_VOLT_UnLock
  ��  ��  ����ѹ����	
  �����������
  �����������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/21
****************************************************************/
void SG_Rcv_Safety_VOLT_UnLock(void)
{
	char msg[100]="";	
	int p = 0;	

	//��ʼ�ֽ�
	msg[p++] = 0x32;
	//������
	msg[p++] = 0x01;

	SIO_WritePPPFrame(COM_DEBUG, (uint8*)msg, p);	

	//g_pro_lock.VoltTime = 1; //��ʼ��ʱ

}


/****************************************************************
  ��������SG_Rcv_Safety_VOLT_Lock
  ��  ��  ����ѹ����	
  �����������
  �����������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/21
****************************************************************/
void SG_Rcv_Safety_VOLT_Lock(void)
{
	char msg[100]="";	
	int p = 0;	

	//��ʼ�ֽ�
	msg[p++] = 0x32;
	//������
	msg[p++] = 0x00;

	SIO_WritePPPFrame(COM_DEBUG, (uint8*)msg, p);	

	//g_pro_lock.VoltTime = 1; //��ʼ��ʱ

}


/****************************************************************
  ��������SG_Rcv_Safety_One_Lock
  ��  ��  ��һ������	
  �����������
  �����������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/11
****************************************************************/
void SG_Rcv_Safety_One_Lock(void)
{
	char msg[100]="";	
	int p = 0;	

	//��ʼ�ֽ�
	msg[p++] = 0x31;
	//������
	msg[p++] = 0x06;
	//������
	msg[p++] = 0x01;
	//֡����
	msg[p++] = 0x07;
	//��������
	msg[p++] = g_pro_lock.nCarNum;
	//У��͵�λ
	msg[p++] = 0x39;
	//У��͸�λ
	msg[p++] = 0x00;
	
	SIO_WritePPPFrame(COM_DEBUG, (uint8*)msg, p);	

	g_pro_lock.time = 1; //��ʼ��ʱ

}

/************************************************************
  ��������SG_Rcv_Safety_Two_Lock
  ��  ��  ����������	
  �����������
  �����������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/11
*************************************************************/
void SG_Rcv_Safety_Two_Lock(void)
{
	char msg[100]="";	
	int p = 0;	

	//��ʼ�ֽ�
	msg[p++] = 0x31;
	//������
	msg[p++] = 0x06;
	//������
	msg[p++] = 0x02;
	//֡����
	msg[p++] = 0x07;
	//��������
	msg[p++] = g_pro_lock.nCarNum;
	//У��͵�λ
	msg[p++] = 0x39;
	//У��͸�λ
	msg[p++] = 0x00;

	SIO_WritePPPFrame(COM_DEBUG, (uint8*)msg, p);	

	g_pro_lock.time = 1; //��ʼ��ʱ

}

/************************************************************
  ��������SG_Rcv_Safety_Uncoil_Lock
  ��  ��  ������
  �����������
  �����������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/11
*************************************************************/
void SG_Rcv_Safety_Uncoil_Lock(void)
{
	
	char msg[100]="";	
	int p = 0;	

	//��ʼ�ֽ�
	msg[p++] = 0x31;
	//������
	msg[p++] = 0x07;
	//������
	msg[p++] = 0x00;
	//֡����
	msg[p++] = 0x07;
	//��������
	msg[p++] = g_pro_lock.nCarNum;
	//У��͵�λ
	msg[p++] = 0x37;
	//У��͸�λ
	msg[p++] = 0x00;

	SIO_WritePPPFrame(COM_DEBUG, (uint8*)msg, p);	

	g_pro_lock.time = 1; //��ʼ��ʱ
	
	
}

/************************************************************
  ��������SG_Pro_Ack_Handle
  ��  ��  �������ڽ��յ�������Ӧ��
  �����������
  �����������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/11
*************************************************************/
void SG_Pro_Ack_Handle(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;
	unsigned char *buf = Uart_Parse;
	uint8 type;

	type = buf[1];

	if((type == PRO_LOCK_ACK)&&((g_pro_lock.nType == PRO_ONE_LOCK)||(g_pro_lock.nType == PRO_TWO_LOCK)))
	{
		g_pro_lock.ack++;
		g_pro_lock.time = 0;
		SG_CreateSignal(SG_PRO_ACK, 0, &signal_ptr); 		
	}
	else if((type == PRO_UNCOIL_ACK)&&(g_pro_lock.nType == PRO_UNCOIL_LOCK))
	{
		g_pro_lock.ack++;
		g_pro_lock.time = 0;
		SG_CreateSignal(SG_PRO_ACK, 0, &signal_ptr); 	
	}

	


	
}

/************************************************************
  ��������Pro_Uart_Handle
  ��  ��  �������ڽ��յ�������
  �����������
  �����������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/11
*************************************************************/
void Pro_Uart_Handle(void)
{
	char msg[4] = {0x01, 0x02, 0x03};
	int rcvLen;

	while(SIO_ReadFrame(COM_DEBUG,Uart_Parse,UART_READ_BUF,(uint32*)&UartParseCnt) == SIO_PPP_SUCCESS)
	{
		if(Uart_Parse[3] == UartParseCnt)
		{
			SG_Pro_Ack_Handle();
		}
		
	}	

	
}

void Pro_Time_Handle(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	//�ع�����
	if(g_set_info.bNewUart4Alarm&UT_PROLOCK)	
	{		
		if(SecTimer%180 == 0)		
		{			
			SG_CreateSignal(SG_PRO_LOCK, 0, &signal_ptr); 			
		}	

		if(g_pro_lock.time > 0)
		{
			g_pro_lock.time++;

			if(g_pro_lock.time >= 50)
			{
				g_pro_lock.time = 0;
				g_pro_lock.timeout++;
				SG_CreateSignal(SG_PRO_ACK, 0, &signal_ptr); 		
			}
		}

		if(g_pro_lock.VoltTime > 0)
		{
			g_pro_lock.VoltTime++;
			if(g_pro_lock.VoltTime == 3)
			{
				g_pro_lock.VoltTime = 0;
				SG_CreateSignal(SG_PRO_VO, 0, &signal_ptr); 		
			}	
		}
	}

}

/****************************************************************
  ��������SG_Pro_Lock_Ack
  ��  �� :�����ع�����Ӧ��
  �����������
  �����������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/10
****************************************************************/
void SG_Pro_Lock_Ack(void)
{

	MsgUChar *msg = NULL;
	MsgInt msgLen;
	int flag = 1;

	switch(g_pro_lock.nType)
	{
		case PRO_ONE_LOCK:
			if(g_pro_lock.ack >= 3)
			{
				if(g_pro_lock.state == SG_MENU_CTRL)
					SG_DynMenu_Put_MsgBox (g_pro_lock.menuid, "һ�������ɹ�");
				else 
					g_pro_lock.para[1] = 0;
				
				g_pro_lock.ack = 0;
			}
			else if(g_pro_lock.timeout >= 3)
			{
				if(g_pro_lock.state == SG_MENU_CTRL)
					SG_DynMenu_Put_MsgBox (g_pro_lock.menuid, "һ������ʧ��");
				else
					g_pro_lock.para[1] = 1;
				
				g_pro_lock.timeout = 0;
			}
			else 
			{
				SG_Rcv_Safety_One_Lock();
				flag = 0;
			}
			break;

		case PRO_TWO_LOCK:
			if(g_pro_lock.ack >= 3)
			{
				if(g_pro_lock.state == SG_MENU_CTRL)
					SG_DynMenu_Put_MsgBox (g_pro_lock.menuid, "���������ɹ�");
				else
					g_pro_lock.para[1] = 0;
				
				g_pro_lock.ack = 0;
			}
			else if(g_pro_lock.timeout >= 3)
			{
				if(g_pro_lock.state == SG_MENU_CTRL)
					SG_DynMenu_Put_MsgBox (g_pro_lock.menuid, "��������ʧ��");
				else
					g_pro_lock.para[1] = 1;
				
				g_pro_lock.timeout = 0;
			}
			else 
			{
				SG_Rcv_Safety_Two_Lock();
				flag = 0;
			}
			break;

		case PRO_UNCOIL_LOCK:
			if(g_pro_lock.ack >= 3)
			{
				if(g_pro_lock.state == SG_MENU_CTRL)
					SG_DynMenu_Put_MsgBox (g_pro_lock.menuid, "�����ɹ�");
				else
					g_pro_lock.para[1] = 0;
				
				g_pro_lock.ack = 0;
			}
			else if(g_pro_lock.timeout >= 3)
			{
				if(g_pro_lock.state == SG_MENU_CTRL)
					SG_DynMenu_Put_MsgBox (g_pro_lock.menuid, "����ʧ��");
				else
					g_pro_lock.para[1] = 1;
				
				g_pro_lock.timeout = 0;
			}
			else 
			{
				SG_Rcv_Safety_Uncoil_Lock();
				flag = 0;
			}
			break;
			
		default:
			break;

	}

	if((g_pro_lock.state == SG_REMOTE_CTRL)&&(flag == 1))
	{
		MsgUChar *msg = NULL;
		MsgInt msgLen;
		
		SCI_MEMCPY(g_state_info.sMsgNo, g_pro_lock.sMsgNum,SG_MSG_NO_LEN);
		g_pro_lock.para[0] = g_pro_lock.nType;
		

		//������Ӧ����
		if (MsgSafetyProLock(g_pro_lock.para, &msg, &msgLen) == MSG_TRUE)
		{
			//���ͱ��ĵ����Ͷ���
			SG_Send_Put_New_Item(1, msg, msgLen);
		}
	}

		

}

/****************************************************************
  ��������SG_Send_Project_GPS
  ��  ��  ������GPS��Ϣ��CAN
  �����������
  �����������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/10
****************************************************************/
void SG_Send_Project_GPS(void)
{	
	gps_data *gps= (gps_data *)g_state_info.pGpsFull;	
	char msg[100]="";	
	char tt[5] = "";	
	char time[2] = "";
	int p = 0;	
	short year;	
	int speed;	
	int angle;	
	uint8 len;	
	//��ʼ�ֽ�	
	msg[p] = 0x31;	
	p+=1;	
	//������	
	msg[p] = 0x3f;	
	p+=1;	
	//������	
	msg[p] = 0x00;	
	p+=1;	
	//֡����	
	msg[p] = 0x28;	
	p+=1;	
	//����λ	
	msg[p] = 0x00;	
	msg[p+1] = 0x00;	
	p+=2;	
	//����	
	msg[p] = 0x51;	
	msg[p+1] = 0x00;	
	p+=2;	
	//��	
	time[0] = gps->sDate[4];
	time[1] = gps->sDate[5];
	MsgAscii2Bcd(time,&msg[p]);
	p+=1;	
	
	msg[p] = 0x20;
	p+=1;	
	//��	
	time[0] = gps->sDate[2];
	time[1] = gps->sDate[3];
	MsgAscii2Bcd(time,&msg[p]);	
	p+=1;	
	//��
	time[0] = gps->sDate[0];
	time[1] = gps->sDate[1];
	MsgAscii2Bcd(time,&msg[p]);
	p+=1;	
	//ʱ
	time[0] = gps->sTime[0];
	time[1] = gps->sTime[1];
	MsgAscii2Bcd(time,&msg[p]);	
	p+=1;	
	//��
	time[0] = gps->sTime[2];
	time[1] = gps->sTime[3];
	MsgAscii2Bcd(time,&msg[p]);		
	p+=1;	
	//��
	time[0] = gps->sTime[4];
	time[1] = gps->sTime[5];
	MsgAscii2Bcd(time,&msg[p]);		
	p+=1;	
	//�ź�ǿ��	
	msg[p] = (int)(gps->nNum*100)/10;	
	if(msg[p] >100)
		msg[p] = 100;
	p+=1;	
	//��������γ	
	msg[p] = 01;	
	p+=1;	
	//����	
	memcpy(tt, gps->sLongitude, 3);	
	msg[p+3] = (char)atoi(tt);	
	memset(tt, 0, 5);	
	memcpy(tt, &gps->sLongitude[3], 2);	
	msg[p+2] = (char)atoi(tt);	
	memset(tt, 0, 5);	
	memcpy(tt, &gps->sLongitude[6], 2);	
	msg[p+1] = (char)atoi(tt);	memset(tt, 0, 5);	
	memcpy(tt, &gps->sLongitude[8], 2);	
	msg[p] = (char)atoi(tt);	
	p+=4;	    
	//γ��    
	memset(tt, 0, 5);	
	memcpy(tt, gps->sLatitude, 2);	
	msg[p+3] = (char)atoi(tt);	
	memset(tt, 0, 5);	
	memcpy(tt, &gps->sLatitude[2], 2);	
	msg[p+2] = (char)atoi(tt);	
	memset(tt, 0, 5);	
	memcpy(tt, &gps->sLatitude[5], 2);	
	msg[p+1] = (char)atoi(tt);	
	memset(tt, 0, 5);	
	memcpy(tt, &gps->sLatitude[7], 2);	
	msg[p] = (char)atoi(tt);	
	p+=4;	
	//�ٶ�	
	speed = atoi(gps->sSpeed);	
	speed = (int)(speed*1.85);	
	msg[p] = (char)(speed & 0xff);	
	msg[p+1] = (char)((speed >> 8)&0xff);	
	msg[p+2] = (char)((speed >> 16)&0xff);	
	msg[p+3] = (char)((speed >> 24)&0xff);	
	p+=4;	
	//�Ƕ�	
	angle= atoi(gps->sAngle);	
	msg[p] = (char)(angle & 0xff);	
	msg[p+1] = (char)((angle >> 8)&0xff);	
	msg[p+2] = (char)((angle >> 16)&0xff);	
	msg[p+3] = (char)((angle >> 24)&0xff);	
	p+=4;	
	//�ڲ�DO	
	msg[p] = 0x00;	
	msg[p+1] = 0x00;	
	p+=2;	
	//�ڲ�DI	
	msg[p] = 0x00;	
	msg[p+1] = 0x00;	
	p+=2;
	//У��λ	
	msg[p] = 0x01;	
	msg[p+1] = 0x02;	
	p+=2;	
	//����λ
	msg[p] = 0x03;
	p+=1;
	
	SIO_WritePPPFrame(COM_DEBUG, (uint8*)msg, p);	
	
}
#endif

/*************************************************************************/
/*****************************LED����غ���*******************************/
/*************************************************************************/



/************************************************************
LED��ʾ����������
*************************************************************/
int SG_Led_Parse(int len)
{
	unsigned char *posi=NULL;

	posi=FindChar(Uart_Read_Buf,UartReadCnt+1,0x0a);
	if(posi)
	{
		memset(Uart_Snd_Buf,0,sizeof(Uart_Snd_Buf));
		memcpy(Uart_Snd_Buf,posi,len);
		SG_Uart_Read_Clear(posi+len);
		return 1;
		
	}
	return 0;
}

#ifdef _DUART
/************************************************************
LED��ʾ����������
*************************************************************/
int SG_Led2_Parse(int len)
{
	unsigned char *posi=NULL;

	posi=FindChar(Uart_Read_COM2,UartReadCnt_2+1,0x0a);
	if(posi)
	{
		memset(Uart_Snd_COM2,0,sizeof(Uart_Snd_Buf));
		memcpy(Uart_Snd_COM2,posi,len);
		SG_Uart2_Read_Clear(posi+len);
		return 1;
		
	}
	return 0;
}


/************************************************************
LED�����ں���
*************************************************************/
int  SG_Led2_Read(void)
{
	int len=0;
	unsigned char *posi_start=NULL,*posi_end=NULL;
	unsigned char *readBuf = Uart_Read_COM2;

	if(UartReadCnt_2 > 0 || (SIO_ReadFrame(COM2,(uint8*)Uart_Read_COM2,UART_PARSE_BUF,(uint32*)&UartReadCnt_2) > 0))
	{	
		posi_start = FindChar(readBuf,UartReadCnt_2+1,0x0a);
		if(posi_start != NULL)
		{
			posi_end=FindChar(posi_start+1,UartReadCnt_2-(posi_start-readBuf), 0x0d);
			if(posi_end!=NULL)
			{	
				len=posi_end-posi_start+1;
				if(len<6)
				{
					return 0;
				}
				SCI_TRACE_LOW("SG_Led_Read: find 0x0d data. \n");
				return len;
			}
			
		}
		if (UartReadCnt_2 >= UART_RED_BUF -1)
		{	
			SCI_TRACE_LOW("SG_Led_Read: read buf overflow,ReadCnt:%d",UartReadCnt_2);
			memset(Uart_Read_COM2,0,sizeof(Uart_Read_COM2));
			UartReadCnt_2 = 0;
		}
	}
	return 0;
}
#endif


/************************************************************
LED�����ں���
*************************************************************/
int  SG_Led_Read(void)
{
	int len=0;
	unsigned char *posi_start=NULL,*posi_end=NULL;
	unsigned char *readBuf = Uart_Read_Buf;

	if(UartReadCnt > 0 || (SIO_ReadFrame(COM_DEBUG,(uint8*)Uart_Read_Buf,UART_PARSE_BUF,(uint32*)&UartReadCnt) > 0))
	{	
		//SIO_WriteFrame(COM2,Uart_Read_Buf,UartReadCnt);	
		posi_start = FindChar(readBuf,UartReadCnt+1,0x0a);
		if(posi_start != NULL)
		{
			posi_end=FindChar(posi_start+1,UartReadCnt-(posi_start-readBuf), 0x0d);
			if(posi_end!=NULL)
			{	
				len=posi_end-posi_start+1;
				if(len<6)
				{
					return 0;
				}
				SCI_TRACE_LOW("SG_Led_Read: find 0x0d data. \n");
				return len;
			}
			
		}
		if (UartReadCnt >= UART_RED_BUF -1)
		{	
			SCI_TRACE_LOW("SG_Led_Read: read buf overflow,ReadCnt:%d",UartReadCnt);
			memset(Uart_Read_Buf,0,sizeof(Uart_Read_Buf));
			UartReadCnt = 0;
		}
	}
	return 0;
}

void Led_Uart_Handle(void)
{
	unsigned char *framePtr = NULL;
	unsigned char *frameEnd = NULL;
	int rcvLen=0;
	unsigned char msg[256];
	int pos = 0;
	int devID= 0;
	int msglen = 0;
	unsigned char *pOut = NULL;
	int outlen = 0;
	uint8 ptr;
	uint8 end;

	if(g_set_info.bNewUart4Alarm&UT_HBLED)
	{
		ptr = 0x7e;
		end = 0x7e;
	}
	else
	{
		ptr = 0x0a;
		end = 0x0d;
	}

	framePtr = FindChar(Uart_Read_Buf,UartReadCnt+1,ptr);
	if(framePtr == NULL)
	{
		return ;
	}
	
	frameEnd = FindChar(framePtr+1,UartReadCnt-(framePtr-Uart_Read_Buf)+1,end);
	if(frameEnd == NULL)
	{
		return ;
	}

	rcvLen = frameEnd-framePtr+1;
	memset(Uart_Snd_Buf,0,sizeof(Uart_Snd_Buf));
	memcpy(Uart_Snd_Buf,framePtr,rcvLen);
	
	g_state_info.nLedCnt ++;

	//����������Ӧ�𣬲���ƽ̨����
	if(Uart_Snd_Buf[9] == 0x48)
	{
		if(Uart_Snd_Buf[15] == 0x31)
		{
			g_state_info.LedState = 1;
			g_state_info.LedTimeCount = 0;
			g_state_info.LedOverTime = 0;
		}
		else if(Uart_Snd_Buf[15] == 0x30)
		{
			g_state_info.LedState = 0;
			g_state_info.LedTimeCount = 0;
			g_state_info.LedOverTime = 0;
		}

		return;
	}



	
	memset(msg,0,sizeof(msg));
	
	devID = 0x03;
	msg[0] = ((devID>>8) & 0xff);
	msg[1] = (devID & 0xff);
	
	//gps �� �ĳ���
	msg[2] = ((MSG_GPS_LEN>>8) & 0xff);
	msg[3] = (MSG_GPS_LEN & 0xff);

	//����gps����
	pos = 4;
	ParseGPS((gps_data*)g_state_info.pGpsFull, (char*)msg, &pos);


	//�����������ݣ�n��
	memcpy(msg+pos,Uart_Snd_Buf,rcvLen);
	msglen=pos+rcvLen;
	
	if (WatchDevReport(msg,msglen, &pOut, &outlen) == MSG_TRUE)
	{
		//���ͱ��ĵ����Ͷ���
		SG_Send_Put_New_Item(2, pOut, outlen);
	}

}


void SG_LED_UartRead(void)
{
	int count = 0;
	uint8 end = 0;

	if(g_set_info.bNewUart4Alarm&UT_HBLED)
	{
		end = 0x7e;
	}
	else
	{
		end = 0x0d;
	}
	
	while(SIO_ReadFrame(COM_DEBUG,Uart_Read_Buf+UartReadCnt,UART_READ_BUF,(uint32*)&count) > 0)
	{
	
		UartReadCnt += count;
		if((Uart_Read_Buf[UartReadCnt-1]) == end && (UartReadCnt >3))
		{
			Led_Uart_Handle();
			UartReadCnt = 0;
			memset(Uart_Read_Buf, 0, UART_READ_BUF);
		}

		if(UartReadCnt >= UART_READ_BUF)
		{
			UartReadCnt = 0;
			memset(Uart_Read_Buf, 0, UART_READ_BUF);
		}
	}	
	
}



#ifdef _DUART
void Led_COM2_Handle(void)
{
	unsigned char *framePtr = NULL;
	unsigned char *frameEnd = NULL;
	int rcvLen=0;
	unsigned char msg[256];
	int pos = 0;
	int devID= 0;
	int msglen = 0;
	unsigned char *pOut = NULL;
	int outlen = 0;
	int i;
	
	framePtr = FindChar(Uart_Read_COM2,UartReadCnt_2+1,0x0a);
	if(framePtr == NULL)
	{
		return ;
	}
	
	frameEnd = FindChar(framePtr+1,UartReadCnt_2-(framePtr-Uart_Read_COM2)+1,0x0d);
	if(frameEnd == NULL)
	{
		return ;
	}

	rcvLen = frameEnd-framePtr+1;
	memset(Uart_Snd_COM2,0,sizeof(Uart_Snd_COM2));
	memcpy(Uart_Snd_COM2,framePtr,rcvLen);
	
	g_state_info.nLedCnt ++;

	//����������Ӧ�𣬲���ƽ̨����
	if(Uart_Snd_COM2[9] == 0x48)
	{
		if(Uart_Snd_COM2[15] == 0x31)
		{
			g_state_info.LedState = 1;
			g_state_info.LedTimeCount = 0;
			g_state_info.LedOverTime = 0;
		}
		else if(Uart_Snd_COM2[15] == 0x30)
		{
			g_state_info.LedState = 0;
			g_state_info.LedTimeCount = 0;
			g_state_info.LedOverTime = 0;
		}

		return;
	}

	
	memset(msg,0,sizeof(msg));
			
	devID = 0x03;
	msg[0] = ((devID>>8) & 0xff);
	msg[1] = (devID & 0xff);
	
	//gps �� �ĳ���
	msg[2] = ((MSG_GPS_LEN>>8) & 0xff);
	msg[3] = (MSG_GPS_LEN & 0xff);

	//����gps����
	pos = 4;
	ParseGPS((gps_data*)g_state_info.pGpsFull, (char*)msg, &pos);


	//�����������ݣ�n��
	memcpy(msg+pos,Uart_Snd_COM2,rcvLen);
	msglen=pos+rcvLen;

	if (WatchDevReport(msg,msglen, &pOut, &outlen) == MSG_TRUE)
	{
		//���ͱ��ĵ����Ͷ���
		SG_Send_Put_New_Item(2, pOut, outlen);
	}

}
#endif



/*****************************LED����*************************************/


/*************************************************************************/
/*****************************͸���豸��غ���****************************/
/*************************************************************************/

void SG_Watch_Init()
{
	uint32 bd =0;	

	switch(g_set_info.watchbd)
	{
		case 115200:
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_115200)				
			SIO_SetBaudRate(COM_DEBUG,BAUD_115200);	
			break;

		case 9600:
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_9600)				
			SIO_SetBaudRate(COM_DEBUG,BAUD_9600);	
			break;
			
		case 4800:
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_4800)				
			SIO_SetBaudRate(COM_DEBUG,BAUD_4800);	
			break;

		case 2400:
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_2400)				
			SIO_SetBaudRate(COM_DEBUG,BAUD_2400);	
			break;

		case 1200:
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_1200)				
			SIO_SetBaudRate(COM_DEBUG,BAUD_1200);	
			break;

		case 19200:
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_19200)				
			SIO_SetBaudRate(COM_DEBUG,BAUD_19200);	
			break;

		case 38400:
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_38400)				
			SIO_SetBaudRate(COM_DEBUG,BAUD_38400);	
			break;

		case 57600:
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_57600)				
			SIO_SetBaudRate(COM_DEBUG,BAUD_57600);	
			break;

		case 230400:
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_230400)				
			SIO_SetBaudRate(COM_DEBUG,BAUD_230400);	
			break;

		case 460800:
			bd = SIO_GetBaudRate(COM_DEBUG);			
			if(bd != BAUD_460800)				
			SIO_SetBaudRate(COM_DEBUG,BAUD_460800);	
			break;
			
	}
	
}




void SG_Watch_Send(void)
{
	unsigned char* posi;
	unsigned char msg[1024];
	int pos = 0;
	int msglen = 0;
	unsigned char *pOut = NULL;
	int outlen = 0;
	int devID= 0;
	
	if(!(g_set_info.bNewUart4Alarm & UT_WATCH))
	{
		return;
	}
	if((WatchReadFlag == 0)||(!CheckTimeOut(WatchDevTimer)))
	{
		return;
	}

	if(WatchReadFlag == 1)
	{
		WatchReadFlag = 0;
	}
	
//	WatchDevTimer = GetTimeOut(g_set_info.nSetTime);


#if (0)
 	if(UartReadCnt>5)
 	{
 		posi=FindChar(Uart_Read_Buf,UartReadCnt+1,0xAA);
 		if(posi==NULL)
 		{
 			g_state_info.WatchError++;
 			SCI_TRACE_LOW("SG_Watch_Parse:No Find 0xAA\r\n");
 			return;

 		}
  		SCI_TRACE_LOW("SG_Watch_Parse:Get Correct Data\r\n");
  	}
	else
	{
		return;
	}
#endif /* (0) */

	memset(msg,0,sizeof(msg));
	//Uart_Parse�ṹ:0x0a+ ����(2)��+���豸��(1) +������(1)+��������(n)+У��(2)+0x0d
	//msg�ṹ:�����ʶ��2��+ GPS���ݰ���С��2�� + GPS���ݰ�+�����������ݣ�n��
	//�����ʶ
	devID = g_set_info.devID;
	msg[0] = ((devID>>8) & 0xff);
	msg[1] = (devID & 0xff);
	//gps���ĳ���
	msg[2] = ((MSG_GPS_LEN>>8) & 0xff);
	msg[3] = (MSG_GPS_LEN & 0xff);

	//����gps����
	pos = 4;
	ParseGPS(g_state_info.pGpsFull, (char *)msg, &pos);
	//�����������ݣ�n��
	memcpy(msg+pos,Uart_Read_Buf,UartReadCnt);

	msglen =  pos + UartReadCnt;


	if(ElectronDevReport(msg,msglen, &pOut, &outlen) == MSG_TRUE)
	{
		g_state_info.WatchFlag++;
		//���ͱ��ĵ����Ͷ���
		SG_Send_Put_New_Item(2, pOut, outlen);
	}


	SCI_MEMSET(Uart_Read_Buf,0,UART_READ_BUF);
	UartReadCnt = 0;

}


/*****************************************************************
��ȫ͸��
*****************************************************************/
 void SG_Watch_Parse(void)
 {
 	int nread=0;
 	unsigned char readbuf[1024]={0};


	memset(readbuf,0,sizeof(readbuf));
	nread = SIO_ReadFrame(COM_DEBUG, readbuf, sizeof(readbuf)-1);
	if(nread == -1)
	{	
		return;
	}
	if(nread > 0)
	{

		if(WatchReadFlag == 0)
		{
			WatchDevTimer = GetTimeOut(g_set_info.nSetTime);
			WatchReadFlag = 1;
		}
		
   		SCI_TRACE_LOW( "SG_Watch_Parse:read data %d\r\n",nread);
		if(UartReadCnt+nread >= UART_READ_BUF) 
		{
			nread = UART_READ_BUF - UartReadCnt-1;		
			SCI_TRACE_LOW("SG_Watch_Parse: read buf overfolw!len:%d\n",UartReadCnt+nread);
			UartReadCnt=0;
		}
		
		memcpy(Uart_Read_Buf+UartReadCnt, readbuf, nread);
		UartReadCnt += nread;
	}
 		
 #if 0
 	SCI_TRACE_LOW("SG_Watch_Parse:");
	{
	 	int i=0;
	 	for(i=0;i<UartReadCnt;i++)
	 	{
	 		SCI_TRACE_LOW("%02x ",Uart_Read_Buf[i]);
	 		if((i+1)%20==0)
	 			SCI_TRACE_LOW("\r\n");
	 	}
	 	SCI_TRACE_LOW("\r\n");
 	}
 #endif
 	return;
 }

/*****************************͸���豸����********************************/

/*****************************************************************
���������
*****************************************************************/
 void SG_Drive_Parse(void)
{

	unsigned char check;

	check = CheckAdd(Uart_Read_Buf, UartReadCnt-1);
	
	if(check != Uart_Read_Buf[UartReadCnt -1])
	{
		SCI_MEMSET(Uart_Read_Buf, 0, 2048);
		return;
	}

	SCI_MEMSET(Uart_Snd_Buf, 0, 256);
	SCI_MEMCPY(Uart_Snd_Buf, Uart_Read_Buf+6, 30);
	SCI_MEMSET(Uart_Read_Buf, 0, 2048);
	//SIO_WriteFrame(COM2, Uart_Snd_Buf, 30);
	
}


 void SG_Drive_UartRead(void)
 {
	int count = 0;
	while(SIO_ReadFrame(COM_DEBUG,Uart_Read_Buf+UartReadCnt,UART_READ_BUF,(uint32*)&count) > 0)
	{
		
		UartReadCnt += count;
		if((Uart_Read_Buf[0] == 0x1A) && (UartReadCnt >= 45))
		{
			SG_Drive_Parse();
			UartReadCnt = 0;
		}
	}	
 }


/*************************************************************************/
/*****************************�п�����غ���******************************/
/*************************************************************************/

/****************************************************************
  ��������SG_Lock_Uart_Write
  ��  ��  ���п����������ݴ��
  ���������writeBuf����ָ���ַ(������֡ͷ֡β��У����)��len���ͳ���
  �����������
  ��д��  ���º���
  �޸ļ�¼��������2006-7-7
****************************************************************/
int SG_Lock_Uart_Write(unsigned char*writeBuf,int len)
{
	int i = 0;
	unsigned char checksum = 0; 
	unsigned char Uart_Send_Buf[UART_SEND_BUF];
	int UartSendCnt=0 ;

	//��У����
	for(i=0;i<len;i++)
		checksum ^=writeBuf[i];

	Uart_Send_Buf[UartSendCnt++] = 0xe7;
	//ת��
	for(i=0;i<len;i++)
	{
		if(writeBuf[i] == 0xe7)
		{
			Uart_Send_Buf[UartSendCnt++] = 0xd7;
			Uart_Send_Buf[UartSendCnt++] = 0xe5;
		}
		else if(writeBuf[i] == 0xd7)
		{
			Uart_Send_Buf[UartSendCnt++] = 0xd7;
			Uart_Send_Buf[UartSendCnt++] = 0xd5;
		}
		else
		Uart_Send_Buf[UartSendCnt++] = writeBuf[i];
	}
	
	Uart_Send_Buf[UartSendCnt++] = checksum;
	Uart_Send_Buf[UartSendCnt++] = 0xe7;
	
	if(SIO_WriteFrame(COM_DEBUG,Uart_Send_Buf,UartSendCnt))
	{
		SCI_TRACE_LOW("SG_Lock_Uart_Write:UartSendCnt:%d\n",UartSendCnt);
		return 1;
	}
	
	return 0;

}

/****************************************************************
  ��������SG_Lock_Uart_Read
  ��  ��  �����ڶ�����
  ���������
  �����������
  ��д��  ���º���
  �޸ļ�¼��������2006-7-7
****************************************************************/
unsigned char SG_Lock_Uart_Read(void)
{
	unsigned char *frameBegin = NULL,*frameEnd=NULL;
	unsigned char *readBuf = Uart_Read_Buf;
	int locklen=0;

	if(UartReadCnt > 0|| (SIO_ReadFrame(COM_DEBUG,(uint8*)Uart_Read_Buf,UART_PARSE_BUF,(uint32*)&UartReadCnt) > 0))
	{
		frameBegin = FindChar(readBuf,UartReadCnt+1,0xe7);
		if(frameBegin != NULL)
		{
			frameEnd=FindChar(frameBegin+1,UartReadCnt-(frameBegin-Uart_Read_Buf)+1,0xe7);
			if(frameEnd != NULL)
			{
				if(frameEnd==frameBegin+1)
				{
					SG_Uart_Read_Clear(frameEnd);
					return 0;
				}
				else
				{
					locklen=frameEnd-frameBegin+1;
					SCI_TRACE_LOW("SG_Lock_Uart_Read:UartReadCnt:%d,locklen:%d\r\n",UartReadCnt,locklen);
					return locklen;
				}
			}
		}
		if (UartReadCnt >= UART_READ_BUF -1)
		{	
			SCI_TRACE_LOW("SG_Lock_Uart_Read: read buf overflow:len:%d\n",UartReadCnt);
			memset(Uart_Read_Buf,0,sizeof(Uart_Read_Buf));
			UartReadCnt = 0;
		}
	}
		return 0;
}

/****************************************************************
  ��������SG_Lock_Uart_Msg_Parse
  ��  ��  �����ڻ��������ݽ�������
  �����������
  �����������
  ��д��  ���º���
  �޸ļ�¼��������2006-7-77
	������ת�壬�º�����2006-7-31
****************************************************************/
int SG_Lock_Uart_Msg_Parse()
{
	unsigned char *framePtr = NULL;
	unsigned char temp;
	int i = 0,j = 0;
	int err = 0;
	unsigned char checksum = 0;
		
	framePtr = FindChar(Uart_Read_Buf,UartReadCnt+1,0xe7);

	if(framePtr==NULL)
	{
		SCI_TRACE_LOW("SG_Lock_Uart_Msg_Parse:frameBegin is NULL\r\n");
		return 0;
	}

	UartParseCnt = 0;
	
	if(framePtr != NULL) 
	{
		Uart_Parse[UartParseCnt++] =0xe7;
		
		while(framePtr[i+1] != 0xe7)
		{
			if((framePtr+i+1) >= (Uart_Read_Buf+UART_READ_BUF))
			{
				SG_Uart_Read_Clear(framePtr);
				SCI_TRACE_LOW("SG_Lock_Uart_Msg_Parse: on 0xe7 data \r\n");
				return 0;
			}
			temp = framePtr[++i];
		
			//��ת��
			if (temp == 0xd7)
			{
				i++;
				if(framePtr[i]==0xe5)
					Uart_Parse[UartParseCnt++] = 0xe7;
				else if(framePtr[i]==0xd5)
					Uart_Parse[UartParseCnt++] = 0xd7;
				else
				{
					err = 1;
					SCI_TRACE_LOW("SG_Lock_Uart_Msg_Parse: error 0x1a \r\n");
					break;
				}
			}
			else
			{
				Uart_Parse[UartParseCnt++] = temp;
			}

			if (i > UART_PARSE_BUF)
			{
				err = 1;
				SG_Uart_Read_Clear(framePtr+UART_PARSE_BUF);
				SCI_TRACE_LOW("SG_Lock_Uart_Msg_Parse:  data length overflow ,%d\r\n",framePtr -Uart_Read_Buf );
				return 0;
			}
	
		}

		Uart_Parse[UartParseCnt++] = 0xe7;
	
		if(err == 0)
		{
			for(j=1;j<UartParseCnt-1;j++)
				checksum ^= Uart_Parse[j];
			if(checksum) 
			{
				err = 1;
				SCI_TRACE_LOW("SG_Lock_Uart_Msg_Parse: checksum error \r\n");
			}
		}

		//�յ��ı�����û�б�Ҫ�����ˣ�����uart_parse����
		SG_Uart_Read_Clear(framePtr+i+2);//�Ӱ�ͷ��β
		if (err == 1) 
		{
			SCI_TRACE_LOW("SG_Lock_Uart_Msg_Parse: error data\r\n");
			err = 0;
			return 0;
		}
	}

	SCI_TRACE_LOW("SG_Lock_Uart_Msg_Parse: parse data ok!framePtr:%d i:%d\r\n",framePtr-Uart_Read_Buf,i);
	
	return 1;
}

void Lock_Uart_Handle(void)
{
	unsigned char closeLockMsg[]={0x02,0x01,0x02};
	unsigned char openLockMsg[]={0x02,0x01,0x01};
	int rcvLen=0;
	static unsigned int state = 0;
	static char testmode = 0;
	static int oldiostate = 0;
	static int alarmState;

	if ((g_state_info.screen & 0x80) == 0)
	{
		MsgUChar *msg = NULL;
		MsgInt msgLen;

		if (g_state_info.set & 0x80)
		{
				//�������IO��
			SG_Lock_Uart_Write(closeLockMsg, sizeof(closeLockMsg));
		}
		else
		{
				//���￪��IO��
			SG_Lock_Uart_Write(openLockMsg, sizeof(openLockMsg));
		}
		g_state_info.screen=0xff;
#ifdef _TRACK
	//��/����
		
		//������Ӧ����
		if (MsgSafetyOilLock(g_xih_ctrl.para, &msg, &msgLen) == MSG_TRUE)
		{
			//���ͱ��ĵ����Ͷ���
			SG_Send_Put_New_Item(1, msg, msgLen);
		}
#endif
		
	}
	if((rcvLen=SG_Lock_Uart_Read())){ // ���������п�������Ϣ

		if(rcvLen==11)
		{
			if(SG_Lock_Oil_Msg_Parse())
			{	
							
				switch(Uart_Snd_Buf[1])
				{
					case 2:
						//��ȷ��Ӧ֡
						g_state_info.lockState= (Uart_Snd_Buf[3] & 0x9f);
								
#if (0)
						if(g_state_info.lockState & 0x80)
							g_state_info.alarmState |= GPS_CAR_STATU_DETECT;
						else
							g_state_info.alarmState &= ~GPS_CAR_STATU_DETECT;
#endif /* (0) */
									
						if(state != (Uart_Snd_Buf[3] & 0x1f))
						{
							state = (Uart_Snd_Buf[3] & 0x1f);
							g_state_info.ioState &= ~0x3e000000; 
							g_state_info.ioState |= (state <<25);
							SCI_TRACE_LOW("SG_Ext_Dev_Main:g_set_info.ioState:0x%08x\r\n",g_state_info.ioState);
						}
						break;
					default:
						break;
				}

			}			
		}

		if(SG_Lock_Uart_Msg_Parse())
		{	
			
			switch(Uart_Parse[1])
			{
			case 2:
				//��ȷ��Ӧ֡
				g_state_info.lockState= (Uart_Parse[3] & 0x9f);
				
#if (0)
				if(g_state_info.lockState & 0x80)
					g_state_info.alarmState |= GPS_CAR_STATU_DETECT;
				else
					g_state_info.alarmState &= ~GPS_CAR_STATU_DETECT;
#endif /* (0) */
					
				if(state != (Uart_Parse[3] & 0x1f))
				{
					state = (Uart_Parse[3] & 0x1f);
					oldiostate = g_state_info.ioState;
					g_state_info.ioState &= ~0x3e000000; 
					g_state_info.ioState |= (state <<25);
					
					if(testmode == 1) // �����п������Ե���ѯ������(��һ���ߵ�ƽ�仯ʱ�Ქ����Ӧ�ı仯ֵ)
					{
						if(oldiostate != g_state_info.ioState)
						{
							if((oldiostate ^ g_state_info.ioState)%0x02000000 != 0) 
								break;// ͬʱ��ֹһ�����б仯,���账��
							else
							{
								char io_no;
								char io_status;
								char io_val;
								char title[100] = "";
								int i;
								io_status= (oldiostate ^ g_state_info.ioState)/0x02000000;
								
								for(i=0;i<5;i ++)
								{
									if((io_status >>i) & 0x01)
									{
										io_no = i+1;
										
										if(g_state_info.ioState & (oldiostate ^ g_state_info.ioState))
										{
											io_val = 1;
										}
										else
										{
											io_val = 0;
										}
										break;	
									}
								}

								switch(io_no)
								{
									case 1:
										strcat(title,"�ߴ���1��");
										break;		
									case 2:
										strcat(title,"�ʹ���1��");
										break;		
									case 3:
										strcat(title,"�ʹ���2��");
										break;		
									case 4:
										strcat(title,"�ߴ���2��");
										break;
									case 5:
										strcat(title,"�ߴ���3��");
										break;
								}

								if(io_val)
									strcat(title,"�ߵ�ƽ");
								else
									strcat(title,"�͵�ƽ");
								cmd_combin((unsigned char *)title,strlen(title));
							}
						}
					}
				  //  һ�����п�������ģʽ����������ǰ��io�ߵ�״̬ 
					if(g_set_info.bLockTest == 1 && (g_set_info.bNewUart4Alarm & UT_LOCK))
					{
						char TTSmsg[500] = "";
						testmode = 1;
						SG_Set_Cal_CheckSum();
						g_set_info.bLockTest = 0;
						SG_Set_Save();

						strcpy(TTSmsg,"����TTS���Բ˵���ǰ״̬");
						cmd_combin((unsigned char*)TTSmsg,strlen(TTSmsg));
						
						if(g_state_info.ioState & 0x20000000)
						{
							strcat(TTSmsg+strlen(TTSmsg),"�ߴ���1�߸ߵ�ƽ");
						}
						else
						{
							sprintf(TTSmsg+strlen(TTSmsg),"�ߴ���1�ߵ͵�ƽ");
						}

						if(g_state_info.ioState & 0x10000000)
						{
							strcat(TTSmsg+strlen(TTSmsg),"�ʹ���1�߸ߵ�ƽ");
						}
						else
						{
							sprintf(TTSmsg+strlen(TTSmsg),"�ʹ���1�ߵ͵�ƽ");
						}

						if(g_state_info.ioState & 0x08000000)
						{
							strcat(TTSmsg+strlen(TTSmsg),"�ʹ���2�߸ߵ�ƽ");
						}
						else
						{
							sprintf(TTSmsg+strlen(TTSmsg),"�ʹ���2�ߵ͵�ƽ");
						}

						if(g_state_info.ioState & 0x04000000)
						{
							strcat(TTSmsg+strlen(TTSmsg),"�ߴ���2�߸ߵ�ƽ");
						}
						else
						{
							sprintf(TTSmsg+strlen(TTSmsg),"�ߴ���2�ߵ͵�ƽ");
						}

						if(g_state_info.ioState & 0x02000000)
						{
							strcat(TTSmsg+strlen(TTSmsg),"�ߴ���3�߸ߵ�ƽ");
						}
						else
						{
							sprintf(TTSmsg+strlen(TTSmsg),"�ߴ���3�ߵ͵�ƽ");
						}

						cmd_combin((unsigned char*)TTSmsg,strlen(TTSmsg));
						
					}
					SCI_TRACE_LOW("SG_Ext_Dev_Main:g_state_info.ioState:0x%08x\r\n",g_state_info.ioState);
				}
					//������״̬����
				if(g_set_info.bNewUart4Alarm&UT_LOCK)
				{
					
					if((g_state_info.ioState ^  (g_set_info.alarmSet & 0x3e000000))!= (g_state_info.alarmState & 0x3e000000))
					{
						unsigned long lastst = 0;
						lastst = (g_state_info.alarmState & 0x3e000000 &  g_set_info.alarmEnable);
						
						g_state_info.alarmState &= ~0x3e000000;
						g_state_info.alarmState |= (g_state_info.ioState ^ g_set_info.alarmSet)&0x3e000000;
						if(g_set_info.alarmEnable & 0x3e000000)
						{
							if((g_state_info.alarmState & 0x3e000000 &  g_set_info.alarmEnable) != lastst)
							{
								//if((g_state_info.ioState ^ g_set_info.alarmSet)&0x3e000000)
								int tmp;

								tmp = (g_state_info.alarmState^lastst)&0x3e000000;
								
								if(tmp & g_state_info.alarmState & 0x3e000000)
									bNeed = 1;
								else 
									bNeed = 2;
							}
							SCI_TRACE_LOW("@_@@@@@@ --SG_Check_IO:ADAPTER ON");
						}
					}


					
#if (0)
					//����п�������״̬
					if((g_set_info.alarmEnable & GPS_CAR_STATU_DETECT) && ((g_state_info.alarmState&GPS_CAR_STATU_DETECT) != alarmState))
					{
						
						if (g_state_info.alarmState & GPS_CAR_STATU_DETECT)
						{
							bNeed =1;

			  				SCI_TRACE_LOW("SG_Alarm_Check_IO:alarmEnable%08x,alarmState%08x\r\n",g_set_info.alarmEnable,g_state_info.alarmState);	

						}
					}
					
					alarmState = g_state_info.alarmState & GPS_CAR_STATU_DETECT;
#endif /* (0) */
					//���������㱨����
					if (bNeed == 1)
					{
						unsigned char *msg = NULL;
						int len;

						bNeed = 0;

						if (MsgSafetyAlarm(g_state_info.pGpsFull,  &msg, &len) != MSG_TRUE)
							return ;

						SG_Send_Put_Alarm_Item(msg,len);
						if(g_set_info.nNetType == 0) // ����ģʽ		
						{			
							xSignalHeaderRec     *signal_ptr = NULL;
							SG_CreateSignal(SG_SEND_SMS,0, &signal_ptr); 
							SCI_TRACE_LOW("============== send ext alarm sms==============");
						}
						return ;
					}
					else if(bNeed == 2)   //����ȡ��Ҳ����һ�����汨��
					{
						unsigned char *msg = NULL;
						int len;
						bNeed = 0;
						if (MsgWatchByTime((gps_data*)g_state_info.pGpsFull,  &msg, &len) != MSG_TRUE)
							return ;

						SG_Send_Put_New_Item(1, msg, len);
					}

				}	
				break;
			default:
				break;
			}

		}
		
	} 		

}


/*****************************�п�������**********************************/



/*************************************************************************/
/*****************************TTS��غ���*********************************/
/*************************************************************************/

//TTS�ְ�����
void Sg_Handle_TTS(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	if((TTSSendFlag == 1) && (CheckTimeOut(TTSSendTimer) == 1))
	{
		TTSSendFlag = 2;
		SG_CreateSignal(SG_TTS_COMBIN,0, &signal_ptr);
	}

}
/****************************************************************
  ��������SG_Voice_Snd_Msg
  ��  ��  ��TTS ������Ϣ����
  ���������
  ���������
  ������      : ����
****************************************************************/
int  SG_Voice_Snd_Msg (unsigned char code,unsigned char *msg, int len)
{

	xSignalHeaderRec      *signal_ptr = NULL;


	SCI_MEMSET(TTS_Snd_Buf,0,TTS_SEND_BUF);
	TTS_Snd_Buf[0] = code;
	TTS_Snd_Buf[1] = (len&0xff00)>>8;
	TTS_Snd_Buf[2] = (len&0x00ff);
	SCI_TRACE_LOW( "===========SG_Voice_Snd_Msg: code: %d  len: %d\r\n", TTS_Snd_Buf[0],len);
	SCI_MEMCPY(TTS_Snd_Buf+3, msg, len);
	
	SG_CreateSignal(SG_TTS_COMBIN,0, &signal_ptr);
	return 1;
}
/**************************************************************
ֹͣ�ϳ�����
***************************************************************/
int   stop_combin(void)
{          
	unsigned char sendmsg[10]={0};
	int nwrite;
	
	sendmsg[0]=0xFE;
	sendmsg[1]=0x02;

	if(g_set_info.bNewUart4Alarm & UT_TTS)
		nwrite=SIO_WriteFrame(COM_DEBUG,sendmsg,2);
	else
		nwrite=SIO_WriteFrame(COM2,sendmsg,2);
	
	if(nwrite)
		SCI_TRACE_LOW("stop_combin:Write Success\n");
	return 0;
}
/*************************************************************
��ͣ�ϳ�����
*****************************************************************/
int   pause_combin(void)
{          
	unsigned char sendmsg[10]={0};
	int nwrite;
	
	sendmsg[0]=0xFE;
	sendmsg[1]=0x03;
	
	if(g_set_info.bNewUart4Alarm & UT_TTS)
		nwrite=SIO_WriteFrame(COM_DEBUG,sendmsg,2);
	else
		nwrite=SIO_WriteFrame(COM2,sendmsg,2);

	
	if(nwrite)
	{
		SCI_TRACE_LOW("pause_combin:Write Success\n");
		return 1;
	}		

	return 0;
}
/**************************************************************
�ָ��ϳ�����
***************************************************************/
int   return_combin(void)
{          
	unsigned char sendmsg[10]={0};
	int nwrite;
	
	sendmsg[0]=0xFE;
	sendmsg[1]=0x04;

	if(g_set_info.bNewUart4Alarm & UT_TTS)
		nwrite=SIO_WriteFrame(COM_DEBUG,sendmsg,2);
	else
		nwrite=SIO_WriteFrame(COM2,sendmsg,2);
	
	if(nwrite)
	{
		SCI_TRACE_LOW("return_combin:Write Success\n");
		return 1;
	}					
	return 0;
}
/**********************************************************
��������
*****************************************************************/
int   sleep_combin(void)
{          
	unsigned char sendmsg[10]={0};
	int nwrite;
	
	sendmsg[0]=0xFE;
	sendmsg[1]=0x88;
	
	if(g_set_info.bNewUart4Alarm & UT_TTS)
		nwrite=SIO_WriteFrame(COM_DEBUG,sendmsg,2);
	else
		nwrite=SIO_WriteFrame(COM2,sendmsg,2);
	
	if(nwrite)
		SCI_TRACE_LOW("sleep_combin:Write Success\n");
	
	return 1;
}
/****************************************************************
�ϳ�����
*****************************************************************/
int  cmd_combin(unsigned char *msg,int msgLen)
{	
	unsigned char sendmsg[1024]={0};
	int nwrite;
	sendmsg[0]=0xFE;
	sendmsg[1]=0x01;
	sendmsg[2]=(msgLen&0xff00)>>8;
	sendmsg[3]=msgLen&0x00ff;
	memcpy(sendmsg+4,msg,msgLen); 
	if(g_set_info.bNewUart4Alarm & UT_TTS)
		nwrite=SIO_WriteFrame(COM_DEBUG,sendmsg, msgLen+4);
	else
		nwrite=SIO_WriteFrame(COM2,sendmsg, msgLen+4);
	if(nwrite)
	{
		SCI_TRACE_LOW("cmd_combin:Write Success\n");
		return 1;
	}		
		
	return 0;
}






/*************************************************************
������Ϣ����
**************************************************************/
void  Long_Msg_Parse(unsigned char *Msg,int mLen)
{
	int total;
	int len;
	int i=0;
	unsigned char snd_msg[1024]={0};

	total=mLen/200;
	for(i=0;i<total;i++)
	{  	
		memset(snd_msg,0,sizeof(snd_msg));
		memcpy(snd_msg,Msg,200);
//		SG_Net_Voice_Snd_Msg(MSG_SND_VOICE,MSG_VOICE_SET_COMBIN,snd_msg,200);
		cmd_combin(snd_msg,200);
//       ������Ҫ��ʱ30s
//		sleep(30);
		memcpy(Msg,Msg+200,mLen-200*(i+1));
	}
	if(mLen%200)
	{	
		memset(snd_msg,0,sizeof(snd_msg));
		len=mLen%200;
		memcpy(snd_msg,Msg,len);
//		SG_Net_Voice_Snd_Msg(MSG_SND_VOICE,MSG_VOICE_SET_COMBIN,snd_msg,len);
//		cmd_combin(snd_msg,len);
	}
}

void TTS_Uart_Handle(){
	
	int index=0;
	int TTS_Snd_Len;
	int cmd;
	
	cmd = TTS_Snd_Buf[0];
	TTS_Snd_Len = TTS_Snd_Buf[1];
	TTS_Snd_Len = (TTS_Snd_Len<<8)+TTS_Snd_Buf[2];

	SCI_TRACE_LOW("=========cmd: %d  len:%d ======\n",cmd,TTS_Snd_Len);


	// ��TTS����Ҫ����
	switch(cmd)
	{
		case  0x01:   //�ϳ�����
			if(TTS_Snd_Len>200)
			{
				//Long_Msg_Parse(TTS_Snd_Buf+3,TTS_Snd_Len);
				if((TTSSendFlag == 0) || (TTSSendFlag == 1) )
				{
					index=cmd_combin(TTS_Snd_Buf+3,200);
					TTSSendTimer = GetTimeOut(SG_TTS_TIME);
					TTSSendFlag = 1;
		
				}
				if(TTSSendFlag == 2)
				{
					int i = 0;
					int j = 0;

								
					TTSSendFlag = 0;

					for(i=0; i<200; i++)
					{
						if(TTS_Snd_Buf[3+i] <= 0x7f)
							j++;
					}

					if(j%2 == 0)
						index=cmd_combin(TTS_Snd_Buf+3+200,TTS_Snd_Len-200);
					else 
						index=cmd_combin(TTS_Snd_Buf+3+199,TTS_Snd_Len-199);
					
				}
				break;
			}
			if(TTS_Snd_Len<=200)
			{
				TTSSendFlag = 0;
				SCI_TRACE_LOW("=========TTS_Snd_Len<200 ======\n");
				index=cmd_combin(TTS_Snd_Buf+3,TTS_Snd_Len);
				break;
			 }

		case  0x02:  //ֹͣ�ϳ�
			index=stop_combin();
			break;
		case 0x03:  //��ͣ�ϳ�
			index=pause_combin();
			break;
		case 0x04: //�ָ��ϳ�
			index=return_combin();
			break;
		case 0x05: //���ı�����ĺϳ�����
			break;
		case 0x06: //��������
			index=sleep_combin();
			break;
		default:
			break;
	}
	if(!index)
		SCI_TRACE_LOW("Write Error!\n");
}



/*****************************TTS����**********************************/



/*************************************************************************/
/*****************************�����ɼ�����غ���**************************/
/*************************************************************************/

/****************************************************************
ͨ��ADֵ�������ݻ�
*****************************************************************/
int  AD2Capacity(int  AD )
{
	SG_INFO_OIL *head=NULL,*lastitem=NULL;
	float L=0;
	int   volume=0;

	if(AD<=0)
	{
		SCI_TRACE_LOW("AD2Capacity:para err\r\n");
		return 0;
	}
	head=gp_oil_list;
	if(head==NULL)
	{
		SCI_TRACE_LOW("AD2Capacity:gp_oil_list is NULL\r\n");
		return 0;
	}
	if(AD<=head->nconsult)
	{
		volume=g_set_info.nTotalVolume*head->nPersernt/100;
		SCI_TRACE_LOW("AD2Capacity:AD:%d,volume: %d\r\n",AD,volume);
		return volume;
	}
	while(head)
	{
		if(AD>head->nconsult)
		{
			lastitem=head;
			head=head->next;
			continue;
		}
		else
		{
			L=(float)(AD-lastitem->nconsult)/(head->nconsult-lastitem->nconsult)*(head->nPersernt-lastitem->nPersernt)+lastitem->nPersernt;
			volume=g_set_info.nTotalVolume*L/100;
			SCI_TRACE_LOW("AD2Capacity:L:%f \r\n",L);
			break;
			
		}
	}
	return volume;
}

/*****************************************************************
�ο�����Ϣ
*****************************************************************/
int  msg_construct(unsigned char ch1,unsigned char ch2)
{
	unsigned short sum=0;

	sum=(ch1<<8)&0xff00;
	sum|=ch2&0x00ff;

	return sum;
	
}


/****************************************************************
//����������п�������
****************************************************************/
int SG_Lock_Oil_Msg_Parse()
{
	unsigned char *framePtr = NULL;
//	unsigned char temp;
	int i = 0,j = 0,temp = 0;
	unsigned char checksum = 0;
	int NoRunAD=0;
	static int index=0;
	static int nConsultNum=0;
	static int m=0;
	int volume=0;
	static int F=0,L1=0;
	int nstatis=0;
	unsigned char *msg=NULL;
	int len=0;
	int  K=g_set_info.K_AD;
	static  int  Z1=0; //����ʱ��AD���ֵ
	static int min_AD = 0;
	static int ACC = 0;
	static int RunFlag = 0;

	framePtr = FindChar(Uart_Read_Buf,UartReadCnt+1,0xe7);
	if(framePtr==NULL)
	{
		SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:frameBegin is NULL\r\n");
		return 0;
	}

	SCI_MEMSET(Uart_Snd_Buf,0,sizeof(Uart_Snd_Buf));
	SCI_MEMCPY(Uart_Snd_Buf,framePtr,11);
	
#if 1
	for(i=0;i<11;i++)
	{
		SCI_TRACE_LOW("%02x ",Uart_Snd_Buf[i]);
		g_state_info.AD[i]=Uart_Snd_Buf[i];
		
	}
	SCI_TRACE_LOW("\r\n");
#endif

	if(Uart_Snd_Buf[8]&0x10)
	{
		Uart_Snd_Buf[8]=Uart_Snd_Buf[8]&0x0f;
		Uart_Snd_Buf[9]=0xe7;
	}
	for(j=1;j<10;j++)
	{
		checksum^=Uart_Snd_Buf[i];
	}
	if(checksum)
	{
		SG_Uart_Read_Clear(framePtr+11);
//		g_set_info.nLockErr++;
		SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse: error data\r\n");
		return 0;
		
	}
	
	if(Uart_Snd_Buf[4]&0x10)
	{
		Uart_Snd_Buf[4]=Uart_Snd_Buf[4]&0x0f;
		Uart_Snd_Buf[5]=0xe7;
	}
	if(Uart_Snd_Buf[6]&0x10)
	{
		Uart_Snd_Buf[6]=Uart_Snd_Buf[6]&0x0f;
		Uart_Snd_Buf[7]=0xe7;
	}
	
#if 1	
	temp = msg_construct(Uart_Snd_Buf[4],Uart_Snd_Buf[5]);
	temp=100*temp;

	
	g_state_info.CurrAD=temp;  //��ǰʵʱADֵ,��Ϊ�˵���ѯʹ��
	SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:temp:%d,CurrAD:%d\r\n",temp,g_state_info.CurrAD);



	if(g_set_info.OilFlag == 0)
	{
		if(temp>g_set_info.MaxAD)    
		{
			temp=g_set_info.MaxAD;  //�������,�������
		}
	
		if(temp<g_set_info.MinAD-2000)       //С����С���¼���
		{
			index=0;
			nConsultNum=0;
		}
		else if((temp<g_set_info.MinAD)&&(temp>=g_set_info.MinAD-2000))
		{
			temp=g_set_info.MinAD;
		}
	
		if(g_set_info.MaxAD>2000)           //��ʼ��Z1ΪAD���ֵ
		{
			if(m==0)
			{
				Z1=g_set_info.MaxAD; 
			}
		}
		//��ACC ON	������²���������---------------ԭ��1
		if((HAL_GetGPIOVal(CPU_ACC_IN)==0)&&(Z1>2000))  
		{
			index++;
			nConsultNum+=temp;     //60��ƽ������������
			SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:%d %d\n",index,nConsultNum);
			if(index>=60)
			{
				temp=nConsultNum/60;
				index=0;
				nConsultNum=0;
			//////////////////////////////////////////--------------����2
				if(temp<=Z1)
				{
					nstatis=100*(Z1-temp)/Z1;   //����ƫ�Ʋ�
					if(nstatis>K+5)  //���˲������
					{
						Z1=temp;   //����Z1
						SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse :Z1:%d\n",Z1);
						m=1;
						F=1;          //���ͱ�־λ��1
					}
				}
			/////////////////////////////////////////----------------����3
				if(temp>Z1)
				{
					if(F)
					{
						nstatis=100*(temp-Z1)/Z1;  //����ƫ�Ʋ�
						SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse: nstatis:%d,k: %d\r\n",nstatis,K);
						if(nstatis>K)
						{
							F=0; //ȥ�����ͱ�־λ
						}
					}
					Z1=temp;	//����Z1
					SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse: F:%d\r\n",F);
				
					NoRunAD=100*temp/(100+K);   //�����δ����ʱ��ADֵ
					SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:NoRunAD:%d\r\n",NoRunAD);
				
					volume=AD2Capacity(NoRunAD); //ͨ���б���ɵ�ǰ����
					SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:volume:%d,margin:%d,nAlarmVol:%d\r\n",volume,L1-volume,g_set_info.nAlarmVol);

					if((F==0)&&((L1-volume)>100*g_set_info.nAlarmVol))
					{

						if(g_set_info.alarmEnable&GPS_CAR_STATU_FRONT_COVER)	
						{
							g_state_info.alarmState|=GPS_CAR_STATU_FRONT_COVER;   //͵�ͱ���(ǰ��)
						
							bNeed = 1;
						}
					}
					L1=volume;         //����L1
				}
			
				g_state_info.upAD=temp;
				SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:upAD:%d\n",temp);
			}
		}
		else
		{
			index=0;
			nConsultNum=0;
		}
	}
	else
	{
		if(temp<g_set_info.MinAD)    
		{			
			temp=g_set_info.MinAD; 
		}			

		if(temp>g_set_info.MaxAD+2000)       
		{			
			index=0;			
			nConsultNum=0;		
		}		
		else if((temp>g_set_info.MaxAD)&&(temp<=g_set_info.MaxAD+2000))		
		{			
			temp=g_set_info.MaxAD;		
		}			

		if(g_set_info.MinAD>2000)           //��ʼ��Z1ΪAD��Сֵ		
		{			
			if(m==0)			
			{				
				Z1=g_set_info.MinAD; 			
			}		
		}

		if((HAL_GetGPIOVal(CPU_ACC_IN)==0)&&(Z1>2000))  
		{
			index++;
			nConsultNum+=temp;     //60��ƽ������������
			SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:%d %d\n",index,nConsultNum);
			if(index>=60)
			{
				temp=nConsultNum/60;
				index=0;
				nConsultNum=0;
			//////////////////////////////////////////--------------����2
			
				if(temp>=Z1)
				{
					nstatis=100*(temp-Z1)/Z1;   //����ƫ�Ʋ�
					if(nstatis>K+5)  //���˲������
					{
						Z1= temp;   //����Z1
						SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse :Z1:%d\n",Z1);
						m = 1;
						F = 1;          //���ͱ�־λ��1
						ACC = 3;
					}
			
					//���ӱ�Ϊ����״̬
					if((nstatis>K-5)&&(nstatis<K+5))
					{
						F = 0;
						Z1 = temp;
						ACC = 3;
					}
					
					if((temp > Z1) && (ACC!=2))
					{
						F = 0;
						Z1 = temp;
						ACC = 3;
					}

				}

				SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse: F:%d\r\n",F);


				
				if((temp<Z1)&&(ACC==3))
				{
					ACC = 2;
				}

				
                //�жϳ����Ƿ���ȫ����

				if((temp<Z1)&&(ACC==1))
				{
					if(temp>min_AD)
					{
						min_AD = temp;
						SCI_TRACE_LOW("========= min_AD: %d\r\n",min_AD);
						if(RunFlag == 1)
							RunFlag =2;
					}
					else
					{
						nstatis=100*(Z1-temp)/temp;
						if((nstatis>K-5)&&(nstatis<K+5))
						{
							SCI_TRACE_LOW("========= RunFlag: %d\r\n",RunFlag);
							if(RunFlag == 2)
								ACC = 2;
						}
						else
							ACC = 2;	
					}

				}
					
				//ACC�տ�������δ��ȫ����
                if((temp<Z1)&&(ACC==0))
                {
						ACC = 1;
						min_AD = temp;	
										
						nstatis=100*(Z1-temp)/temp;
						if((nstatis>K+5))  //	�ͺĲ�����
							RunFlag =1;
				}

				SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse: ACC:%d\r\n",ACC);
			/////////////////////////////////////////----------------����3
				if((temp<Z1)&&(ACC==2))
				{
			
					Z1=temp;	//����Z1
					//DBG(APP_DBG|LEV_7,"SG_Lock_Oil_Msg_Parse: F:%d\r\n",F);
					
					NoRunAD=100*temp/(100+K);   //�����δ����ʱ��ADֵ
					SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:NoRunAD:%d\r\n",NoRunAD);
				
					volume=AD2Capacity(NoRunAD); //ͨ���б���ɵ�ǰ����
					SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:volume:%d,margin:%d,nAlarmVol:%d\r\n",volume,L1-volume,g_set_info.nAlarmVol);
				
					if((F==0)&&((L1-volume)>100*g_set_info.nAlarmVol))
					{

						if(g_set_info.alarmEnable&GPS_CAR_STATU_FRONT_COVER)	
						{
							g_state_info.alarmState|=GPS_CAR_STATU_FRONT_COVER;   //͵�ͱ���(ǰ��)
						
							bNeed = 1;
						}

					}
					L1=volume;         //����L1
				
				}
			
				g_state_info.upAD=temp;
				SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:upAD:%d\n",temp);
			}
		}
		else
		{
			index=0;
			nConsultNum=0;
			ACC = 0;
			RunFlag = 0;
		}
	}
	
	g_state_info.upOilQuantity=L1;//��0.01��Ϊ��λ
	SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse:upOilQuantity:%d,Z1:%d\r\n",L1,Z1);
#endif	

#if 0
	temp = msg_construct(Uart_Snd_Buf[6],Uart_Snd_Buf[7]);
	if(temp)
		g_set_info.AD2msg = temp;
#endif
		//�յ��ı�����û�б�Ҫ�����ˣ�����uart_parse����
	SG_Uart_Read_Clear(framePtr+11);//�Ӱ�ͷ��β

	SCI_TRACE_LOW("SG_Lock_Oil_Msg_Parse: parse data ok!framePtr:%d i:%d\r\n",framePtr-Uart_Read_Buf,i);
	
	return 1;
}

/*****************************�����ɼ�������******************************/

/*************************************************************************/
/*****************************������վ����غ���**************************/
/*************************************************************************/



/****************************************************************
  ��������SG_Bus_Uart_Read
  ��  ��  �����ڶ�����
  ���������
  �����������
  ��д��  ���º���
  �޸ļ�¼��������2006-7-7
****************************************************************/
unsigned char SG_Bus_Uart_Read(void)
{
	unsigned char *frameBegin = NULL;
	unsigned char *readBuf = Uart_Read_Buf;

	if(UartReadCnt > 0)
	{
		frameBegin = FindChar(readBuf,UartReadCnt+1,0x0a);
		if(frameBegin != NULL)
		{
			frameBegin = FindChar(frameBegin+1,UartReadCnt-(frameBegin-Uart_Read_Buf)+1,0x0d);
			if(frameBegin != NULL)
			{
					SCI_TRACE_LOW("SG_Bus_Uart_Read: find 0x0a 0x0d first  UartReadCnt:\n",UartReadCnt);
					return 1;
			}
		}
		if (UartReadCnt >= UART_READ_BUF -1)
		{	
			SCI_TRACE_LOW("SG_Bus_Uart_Read: read buf overflow !len:%d\n",UartReadCnt);
			memset(Uart_Read_Buf,0,sizeof(Uart_Read_Buf));
			UartReadCnt = 0;
		}
	}
		return 0;
}



/****************************************************************
  ��������SG_Bus_Uart_Msg_Parse
  ��  ��  �����ڻ��������ݽ�������
  �����������
  �����������
  ��д��  ���º���
  �޸ļ�¼��������2006-7-77
	������ת�壬�º�����2006-7-31
****************************************************************/
int SG_Bus_Uart_Msg_Parse()
{
	unsigned char *framePtr = NULL;
	unsigned char temp;
	unsigned char tmp_Uart_Parse[UART_READ_BUF]={0};
	int tmpLen = 0;
	int i = 0;
	int err = 0;
	union
	{
		int sum;
		unsigned char ch[2];
	}checksum;
		
	framePtr = FindChar(Uart_Read_Buf,UartReadCnt+1,0x0a);

	if(framePtr==NULL)
	{
		SCI_TRACE_LOW("SG_Bus_Uart_Msg_Parse:frameBegin is NULL\r\n");
		return 0;
	}

	UartParseCnt = 0;
	
	if(framePtr != NULL) 
	{
		Uart_Parse[UartParseCnt++] =0x0a;
		tmp_Uart_Parse[tmpLen++] =0x0a;
		
		while(framePtr[i+1] != 0x0d )
		{
			if( (framePtr+i+1) >= (Uart_Read_Buf+UART_READ_BUF))
			{
				SG_Uart_Read_Clear(framePtr);
				SCI_TRACE_LOW("SG_Bus_Uart_Msg_Parse: on 0x0a 0x0d data \r\n");
				return 0;
			}
			
			temp = framePtr[++i];
			Uart_Parse[UartParseCnt++] = temp;
			
			//���ת���ַ�
			if (temp == 0x1A)
			{
				i++;
				Uart_Parse[UartParseCnt++] = framePtr[i];
				if(framePtr[i]==0x01)
					tmp_Uart_Parse[tmpLen++]=0x1a;
				else if(framePtr[i]==0x02)
					tmp_Uart_Parse[tmpLen++]=0x0a;
				else if(framePtr[i]==0x03)
					tmp_Uart_Parse[tmpLen++]=0x0d;
				else
				{
					err = 1;
					SCI_TRACE_LOW("SG_Bus_Uart_Msg_Parse: error 0x1a \r\n");
					break;
				}
			}
			else
			{
				tmp_Uart_Parse[tmpLen++] = temp;
			}

			if (i > UART_PARSE_BUF)
			{
				err = 1;
				SG_Uart_Read_Clear(0);
				SCI_TRACE_LOW("SG_Bus_Uart_Msg_Parse:  data length overflow ,%d\r\n",framePtr -Uart_Read_Buf );
				return 0;
			}
	
		}

		Uart_Parse[UartParseCnt++] = 0x0d;
		tmp_Uart_Parse[tmpLen++] = 0x0d;
		
		if(err == 0)
		{
			checksum.sum=CheckSum(tmp_Uart_Parse+1,tmpLen - 4);	//ȥ��0x0a 0x0d  ��2bytes checksum
			if(checksum.ch[0]!=tmp_Uart_Parse[tmpLen - 2] ||checksum.ch[1]!=tmp_Uart_Parse[tmpLen - 3]) 
			{
				err = 1;
				SCI_TRACE_LOW("SG_Bus_Uart_Msg_Parse: checksum error \r\n");
			}
		}

		//�յ��ı�����û�б�Ҫ�����ˣ�����uart_parse����
		SG_Uart_Read_Clear(framePtr+i+2);
		if (err == 1) 
		{
			SCI_TRACE_LOW("SG_Bus_Uart_Msg_Parse: error data\r\n");
			err = 0;
			return 0;
		}
	}
	
	return 1;
}


/****************************************************************
  ��������SG_Bus_Dev_Snd_Msg
  ��  ��  �����ڷ�����Ϣ
  ���������cmd����;msg��Ϣָ���׵�ַ;len��Ϣ����
  ���������0:����ʧ��;1:���ͳɹ�
  ��д��  ���º���
  �޸ļ�¼��������2006-7-4
  	�޸�ȥ��ת�壬�º�����2006-7-31
****************************************************************/
int  SG_Bus_Dev_Snd_Msg (unsigned char devID,unsigned char cmd,unsigned char *msg, int len)
{
#if 1	
	int i;
	unsigned char tmpMsg[MSG_EXT_DEV_LEN];
	int tmpMsgLen = 0;
	union
	{
		short sum;
		unsigned char ch[2];
	}check;

	if(len < 0)
	{
		SCI_TRACE_LOW("SG_Bus_Dev_Snd_Msg:len %d\n",len);
		return 0;
	}

	//��ת�����֤,ת�岻������ͷ����β
	for(i=1;i<len-1;i++)
	{

		if(msg[i] == 0x1a)
		{
			if(msg[i+1] ==0x01)
				tmpMsg[tmpMsgLen++] = 0x1a;
			else if(msg[i+1] ==0x02)
				tmpMsg[tmpMsgLen++] = 0x0a;
			else if(msg[i+1] ==0x03)
				tmpMsg[tmpMsgLen++] = 0x0d;
		}
		else
		{
			tmpMsg[tmpMsgLen++] = msg[i];
		}
	}

	//У��,��������ͷ����β
	check.sum=CheckSum(tmpMsg, tmpMsgLen-2);
	if((tmpMsg[tmpMsgLen-2] != check.ch[1] )&&( tmpMsg[tmpMsgLen-1] != check.ch[0]))
	{
		SCI_TRACE_LOW("SG_Bus_Dev_Snd_Msg:CheckSum err!\n");
		return 0;
	}
#endif	

	if(SIO_WriteFrame(COM_DEBUG,msg,len))
	{
		SCI_TRACE_LOW("SG_Bus_Dev_Snd_Msg:OK\n");
		return 1;
	}

	SCI_TRACE_LOW("SG_Bus_Dev_Snd_Msg:err\n");
	return 0;
}



void Bus_Uart_Handle(void)
{
	unsigned char msg[256];
	int pos = 0;
	int devID= 0;
	int msglen = 0;
	unsigned char *pOut = NULL;
	int outlen = 0;

	if((SIO_ReadFrame(COM_DEBUG,(uint8*)Uart_Read_Buf,UART_PARSE_BUF,(uint32*)&UartReadCnt) > 0)||UartReadCnt>0)
	{
		int i =0;
		char buf[4] = {0};

		if(SG_Bus_Uart_Read())
		{
			if(SG_Bus_Uart_Msg_Parse())
			{    
				g_state_info.nBusCnt++;
				
				memset(msg,0,MSG_EXT_DEV_LEN);
				//Uart_Parse�ṹ:0x0a+ ����(2)��+���豸��(1) +������(1)+��������(n)+У��(2)+0x0d
				//msg�ṹ:�����ʶ��2��+ GPS���ݰ���С��2�� + GPS���ݰ�+�����������ݣ�n��
				//�����ʶ
				devID = 0x01;
				msg[0] = ((devID>>8) & 0xff);
				msg[1] = (devID & 0xff);
				//gps���ĳ���
				msg[2] = ((MSG_GPS_LEN>>8) & 0xff);
				msg[3] = (MSG_GPS_LEN & 0xff);

				//����gps����
				pos = 4;
				ParseGPS(g_state_info.pGpsFull, (char *)msg, &pos);
				//�����������ݣ�n��
				memcpy(msg+pos,Uart_Parse,UartParseCnt);//�Ӱ�ͷ��β�����ֽ�

				msglen =  pos + UartParseCnt;

				if (MsgBusDevReport(msg,msglen, &pOut, &outlen) == MSG_TRUE)
				{	
					//���ͱ��ĵ����Ͷ���
					SG_Send_Put_New_Item(2, pOut, outlen);
				}
			}
		}
	}

}







void SG_STAR_GPS_Handle(void)
{
	while(SIO_ReadFrame(COM_DEBUG,Uart_Read_Buf,UART_READ_BUF,(uint32*)&UartReadCnt) > 0)
	{
		int i;

		if(Uart_Read_Buf[0] == '&' && Uart_Read_Buf[1] == '&')
		{

			if(strncmp((char *)Uart_Read_Buf,"&&GPSON",7) == 0)
			{
#ifdef _DUART
				HAL_SetGPIOVal(PWREN_GPS,0);	
				SCI_Sleep(10);
				HAL_SetGPIOVal(PWREN_GPS,1);
#else
				HAL_SetGPIOVal(PWREN_GPS,1);	
				SCI_Sleep(10);
				HAL_SetGPIOVal(PWREN_GPS,0);
#endif
				return;
			}
		
		}
	
		for(i=0; i<UartReadCnt; i++)
		{
			HAL_DumpPutChar(COM_DATA, Uart_Read_Buf[i]);	
		}
		UartReadCnt = 0;
	}
}






/************************������վ������************************************/
/****************************************************************
  ��������SG_Ext_Dev_Main
  ��  ��  ��Ext_Devģ�����ݴ���ѭ�������
  ���������data:��ʼ���ɹ��źŵ�
  �����������
  ��д��  ���º���
  �޸ļ�¼��������2006-7-5
  �������п�����2006-7-7
****************************************************************/
void SG_Ext_Dev_Main(void)
{

	if(g_state_info.GpsDownloadFlag == 1)
	{
		SG_STAR_GPS_Handle();
	}


#ifdef _DUART
	if((g_set_info.bRegisted == 0)&&(g_set_info.bNewUart4Alarm == 0)&&(g_state_info.GpsDownloadFlag == 0))
	{
		//MSTARģ������
		SG_Star_UartRead();
	}
#endif

	// ������LED����
	if(g_set_info.bNewUart4Alarm&(UT_HBLED|UT_LED))
	{
		SG_LED_UartRead();
	}

	// �������п���
	if(g_set_info.bNewUart4Alarm & UT_LOCK)
	{
		Lock_Uart_Handle();
	}
	
	// ����������ͷ
	if(g_set_info.bNewUart4Alarm&UT_QQCAMERA) //������ľ
	{
		QQZM_Camera_Uart_Read();
	}
	else if(g_set_info.bNewUart4Alarm&UT_XGCAMERA)  //�ǹ�
	{
		Camera_Uart_Handle();
	}

	// �����˼��������
	if(g_set_info.bNewUart4Alarm & UT_DRIVE)
	{
		SG_Drive_UartRead();
	}

	// ������͸���豸
	if(g_set_info.bNewUart4Alarm&UT_WATCH)
	{
		SG_Watch_Parse();
	}

	// �������Ե�����
	if(g_set_info.bNewUart4Alarm & UT_GPS)
	{
		SG_GPS_UartRead();
	}

	if(g_set_info.bNewUart4Alarm & UT_BUS)
	{
		Bus_Uart_Handle();
	}

}




// ���������������ж�
void SG_Detect_Alarm(void)
{
	if(Pwm_Start_Time != 0 && Pwm_End_Time != 0)
	{

		int CurrTime = GetTimeOut(0);

		if(Pwm_End_Time - Pwm_Start_Time == 10) // �������˸10��
		{
			// ����񶯴����������������ǿ��ش�������
			if((!(g_state_info.alarmState & GPS_CAR_STATU_DETECT))&&((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM) == 0))
			{
				g_state_info.alarmState |= GPS_CAR_STATU_DETECT;
				
				if (g_set_info.alarmEnable & GPS_CAR_STATU_DETECT)
				{
					bNeed =1;

	  				SCI_TRACE_LOW("++++++Detect Alarm ON!!!!\r\n");	

				}

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
					

					if (MsgSafetyAlarm(g_state_info.pGpsFull,  &msg, &len) != MSG_TRUE)
						return ;

					SG_Send_Put_Alarm_Item(msg,len);
					if(g_set_info.nNetType == 0) // ����ģʽ		
					{			
						xSignalHeaderRec     *signal_ptr = NULL;
						SG_CreateSignal(SG_SEND_SMS,0, &signal_ptr); 
						SCI_TRACE_LOW("============== send Detect alarm sms==============");
					}

				}

#if (0)
				SG_Set_Cal_CheckSum();
				g_set_info.alarmState |= GPS_CAR_STATU_DETECT;
				SG_Set_Save();
#endif /* (0) */
			}
		}
		else if(CurrTime - Pwm_End_Time > 1)
		{
			SCI_TRACE_LOW("**********Not Alarm !!! Clear Time!!!");
			Pwm_End_Time = 0;
			Pwm_Start_Time = 0;
			if(g_state_info.alarmState &GPS_CAR_STATU_DETECT)
			{
				g_state_info.alarmState &= ~GPS_CAR_STATU_DETECT;
				SCI_TRACE_LOW("++++++Detect Alarm OFF!!!!\r\n");
			}
		}
		else
		{
			SCI_TRACE_LOW("+++++++start %d end %d keeptime %d",Pwm_Start_Time,Pwm_End_Time,Pwm_End_Time-Pwm_Start_Time);
		}
	}

	return;

}
// ����1 �����Դ����� 0 ���Դ�����
int SG_Ext_Dev_Enable_Judge(int open_ext)
{
	switch(open_ext)
	{
		case UT_XGCAMERA:
		case UT_GPS:
		case UT_WATCH:
		case UT_BUS:
			#ifdef _DUART
			if(g_state_info.bNewUart4Alarm & 0x7fff)
				return 1;
			#else
			if(g_state_info.bNewUart4Alarm != 0)
				return 1;
			#endif
			break;
		case UT_TTS:
			#ifdef _DUART
			if((g_state_info.bNewUart4Alarm & 0x17fff) && (!(g_state_info.bNewUart4Alarm & UT_LOCK)))
				return 1;
			#else
			if((g_state_info.bNewUart4Alarm != 0) && (!(g_state_info.bNewUart4Alarm & UT_LOCK)))
				return 1;
			#endif
			break;
		case UT_LOCK:
			#ifdef _DUART
			if((g_state_info.bNewUart4Alarm & 0x7fff) && (!(g_state_info.bNewUart4Alarm & UT_TTS)))
				return 1;
			#else
			if((g_state_info.bNewUart4Alarm != 0) && (!(g_state_info.bNewUart4Alarm & UT_TTS)))
				return 1;
			#endif
			break;
		case UT_LED:
		case UT_HBLED:
			if(g_state_info.bNewUart4Alarm & 0x0000ffff)
				return 1;
			break;
		case UT_PROLOCK:
			break;
		case UT_LED2:
			if(g_state_info.bNewUart4Alarm & (UT_LED | UT_TTS2))
				return 1;
			break;
		case UT_TTS2:
			if(g_state_info.bNewUart4Alarm & (UT_LED2 | UT_TTS))
				return 1;
			break;
		default:
			break;
	}
	return 0;
	
}


