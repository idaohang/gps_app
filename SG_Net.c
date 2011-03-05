#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sci_types.h"
#include "os_api.h"
#include "app_tcp_if.h"
#include "mn_events.h"

#include "Msg.h"
#include "SG_Net.h"
#include "SG_Set.h"
#include "SG_GPS.h"
#include "mn_events.h"
#include "SG_Send.h"
#include "Mn_type.h"
#include "Mn_api.h"
#include "SG_Timer.h"
#include "SG_MsgHandle.h"
#include "SG_Sms.h"
#include "SG_Hh.h"

#define  SO_ISCONNECTED       0x0002   /* socket connected to a peer */
char GPRS_RxBuf[1024*10];

/********************************�궨��****************************/
#define TCP_MSS 1500 				// max segment size
#define BUFFER_LEN  2000

/*******************************��������****************************/

int bhangup =0; 		// 0 û�н������� -1 �ȴ�����Ӧ�� 1 ���ֳɹ�
sg_send_list *msgItem = NULL;// �����е�һ���ڵ�
int gprs_snd_tmout_cnt = 0; //gprs���ͳ�ʱ����(�ۼ����γ���90s�Ͽ�����)
uint smsbuflen = 0; // ����buf����
uint8 smsbuf[1024]={0}; // ����buffer
static int EwouldblockCnt=0;

//used for udp interface ��ͼƽ̨
//temp variable used for dummy application
unsigned short temp_s_lport = 0; //arbiatry
unsigned short temp_s_fport = 0;
unsigned long temp_s_fhost = 0;
void * temp_s_label = (void*)(-45);
void * temp_s_udpconn = NULL;
uint32 reConsocket;		//���´���socket�Ĵ�������
uint32 ReDeactiveCnt;		//����deactive�Ĵ�������
uint32 GprsSndTimer; 	//gprs���Ͷ�ʱ�� 30s��ʱ

/****************************************************************
  ��������	MnGPRS_EventCallback
  ��  ��  ��this function set and activate a pdp context
  �����������
  �����������
  ����:		��ȷ1������0
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
BOOLEAN	SG_GPRS_SetAndActivePdpContex(void)
{
	ERR_MNGPRS_CODE_E ret = ERR_MNGPRS_NO_ERR;
	BOOLEAN param_used[7];
	uint32 pdpid_ptr[2] = {1,0};
	MN_GPRS_PDP_ADDR_T pdp_addr;

	DataLedCnt = 10;	//ppp��������10��
	g_gprs_t.state = GPRS_ACTIVE_PDP;
	g_gprs_t.timeout = 1;
	g_state_info.nActiveCnt++;


	SCI_TRACE_LOW("================reconnect count %d",ReConCnt);
	if(ReConCnt++ > 10){
#if (0) // ��ʱ����
		if(g_state_info.ActiveErr == 19 && g_state_info.lasterr == SG_ERR_TIME_OUT
			&&g_state_info.nNetTimeOut == GPRS_ACTIVE_PDP)
		{
			// �����������һֱ�޷�active pdp,ʹ��Ӳ������λ
			while(1){;
			}
		}
#endif /* (0) */
		SG_Soft_Reset(3);
		return 0;
	}

	memset(param_used,1,7); // ȫ��������Ч
	memset(&pdp_addr,0,sizeof(pdp_addr));

	ret = MNGPRS_SetPdpContext(param_used,1,(uint8*)"IP",(uint8*)g_set_info.sAPN,pdp_addr,FALSE,FALSE,NULL);
	if( ERR_MNGPRS_NO_ERR ==  ret)
	{
		SCI_TRACE_LOW("-----SG_GPRS_SetAndActivePdpContex: MNGPRS_SetPdpContext successful");
	}
	else
	{
		SCI_TRACE_LOW("-----SG_GPRS_SetAndActivePdpContex:MNGPRS_SetPdpContext error, ret = %d",ret);
		g_state_info.lasterr = SG_ERR_ACTIVE_PDP;
		return 0;
	}


	ret = MNGPRS_SetPdpContextPco(1,(uint8*)g_set_info.sGprsUser,(uint8*)g_set_info.sGprsPsw);
	if( ERR_MNGPRS_NO_ERR ==  ret)
	{
		SCI_TRACE_LOW("-----SG_GPRS_SetAndActivePdpContex: MNGPRS_ReadPdpContextPco successful");
	}
	else
	{
		SCI_TRACE_LOW("-----SG_GPRS_SetAndActivePdpContex:MNGPRS_ReadPdpContextPco error, ret = %d",ret);
		g_state_info.lasterr = SG_ERR_ACTIVE_PDP;
		return 0;
	}


	ret = MNGPRS_ActivatePdpContext(FALSE,pdpid_ptr);
	if( ERR_MNGPRS_NO_ERR ==  ret)
	{
		SCI_TRACE_LOW("-----SG_GPRS_SetAndActivePdpContex: MNGPRS_ActivatePdpContext successful");
		return 1;
	}
	else
	{
		SCI_TRACE_LOW("-----SG_GPRS_SetAndActivePdpContex:MNGPRS_ActivatePdpContext error, ret = %d",ret);
		g_state_info.lasterr = SG_ERR_ACTIVE_PDP;
		return 0;
	}

}


/****************************************************************
  ��������	MnGPRS_EventCallback
  ��  ��  ��this function deactive a pdp context
  �����������
  �����������
  ����:	��ȷ1������0
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
BOOLEAN SG_GPRS_DeactivePdpContext(void)
{
	MMI_PDP_ACT_REQ_T          mn_act_info;
	ERR_MNGPRS_CODE_E ret = ERR_MNGPRS_NO_ERR;

	g_gprs_t.state = GPRS_DEACTIVE_PDP;
	g_gprs_t.timeout = 0;

	SCI_TRACE_LOW("--ReDeactiveCnt = %d",ReDeactiveCnt);

	if(ReDeactiveCnt ++ > 2){
		SG_Soft_Reset(3);
	}


#if (0)
	if(g_state_info.plmn_status != PLMN_NORMAL_GSM_GPRS_BOTH)
	{
		MN_PLMN_T     select_plmn = {0,0,2};
	    MNPHONE_SelectPLMN(FALSE,select_plmn);
		SCI_TRACE_LOW("=======================Auto Search Network!!!!");
	}
#endif /* (0) */

	SCI_MEMSET(&mn_act_info, 0, sizeof(MMI_PDP_ACT_REQ_T));

	mn_act_info.is_all = TRUE;

	g_state_info.nDeactiveCnt++;

	ret = MNGPRS_DeactivatePdpContext(mn_act_info.is_all, mn_act_info.pdp_id);
	if (ERR_MNGPRS_NO_ERR == ret)
	{
		SCI_TRACE_LOW("--SG_GPRS_DeactivePdpContext:deactivate PDP context success");
		return 1;
	}
	else
	{
		SCI_TRACE_LOW("--SG_GPRS_DeactivePdpContext:deactivate PDP context fail err code %x",ret);
		g_state_info.lasterr = SG_ERR_DEACTIVE_PDP;
		return 0;
    }
}

// Deactive pdp ʱ�䴦�����²��Ŵ���
void SG_Redial_Handle(int result)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	if(0 == result) // Deactive �ɹ������²���
	{
		if((g_set_info.nNetType >  MSG_NET_SMS) && (!(strlen(g_set_info.sOwnNo) == 0)))
		{

			SG_CreateSignal(SG_ACTIVE_PDP,0, &signal_ptr); // �����źŵ���ѭ��Ҫ��������ACTIVE PDP
		}
		#if (__cwt_)
		SG_Close_an_Udp();
		#else
		g_gprs_t.state = GPRS_DISCONNECT;
		socket_close_an_tcp();
		g_state_info.socket = -1;
		#endif
		ReDeactiveCnt = 0;
	}
	else
	{
		SCI_TRACE_LOW("--SG_Redial_Handle:Deactive err %d",result);

	}

}



// ����ͨ�ĳ�ʼ����������
void SG_Net_Init_Udp(void)
{
	int ret;
	xSignalHeaderRec      *signal_ptr = NULL;

	SG_Set_Cal_CheckSum();
	if(g_set_info.bRegisted)
		g_set_info.bRegisted = 0;
	SG_Set_Save();

	ret = SG_Open_an_Udp();
	if(ret < 0) // ���ɹ���deactive pdp������
	{
		SCI_TRACE_LOW("--Open Udp err!!!!");
		g_gprs_t.state = SG_ERR_OPEN_UDP;
		SG_CreateSignal(SG_DEACTIVE_PDP,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
	}
	else // �ɹ�����е�¼(����)
	{
		SCI_TRACE_LOW("--Open Udp sucess!!!!");
		SG_CreateSignal(SG_HANDUP,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���

	}
}




/****************************************************************
  ��������	SG_Net_Init_Tcp
  ��  ��  ����ȡ���������ӵ�SOCKET,����SOCKET���������ֱ��ĸ�����
  ���������type:����/���б�־λ��1��ע��0x00�����У�0x01������
  			ip: Ŀ��IP
  			prot:Ŀ��˿�
  			proxyIp:����IP
  			proxyPort:����˿�
  			porxyEnable:����ʹ��1:ʹ�ô���0:��ʹ�ô���
  �����������
  ����ֵ��	0: �ɹ� 	-1:ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2007-9-2
****************************************************************/
void  SG_Net_Init_Tcp(	unsigned char type,
									char *ip, int port,
									char *proxyIp,
									int proxyPort,
									int porxyEnable)
{
 	struct sci_sockaddr him;
 	int e;
	xSignalHeaderRec      *signal_ptr = NULL;


	socket_close_an_tcp();
	SCI_TRACE_LOW("--SG_Net_Init_Tcp:reConsocket:%d",reConsocket);
	if(reConsocket++ > 2){  // ��������SOCKET3��,DEACTIVE pdp context
		reConsocket =0;
		SG_CreateSignal(SG_DEACTIVE_PDP,0, &signal_ptr);
		return;
	}

	g_state_info.socket = sci_sock_socket(AF_INET, SOCK_STREAM, 0);// ����һ��tcp��socket
	// ����socket��������
	if(g_state_info.socket < 0)
	{
		e = sci_sock_errno(g_state_info.socket);
		SCI_TRACE_LOW("--SG_Net_Init_Tcp:Socket open error:%d",e);
		g_state_info.lasterr = SG_ERR_CREAT_SOCKET;
		goto err;
	}

	bhangup = -1; // �ȴ�����Ӧ��
	GPRS_RxHead = 0;

	e =sci_sock_setsockopt(g_state_info.socket, SO_NBIO, NULL); // ����socketΪ����ģʽ
	if(e < 0)
	{
		e = sci_sock_errno(g_state_info.socket);
		SCI_TRACE_LOW("--Socket TCP sci_sock_setsockopt error:%d",e);
		g_state_info.lasterr = SG_ERR_SET_OPT;
		goto err;
	}

	tcp_set_callback(SG_GPRS_GetData); // ע����յĻص�����
	SCI_MEMSET(&him,0,sizeof(him));
	if(porxyEnable){    // ʹ�ô���ʱ
		him.family = AF_INET;
		him.port = htons(proxyPort); //set current server's service port
		e = sci_parse_host(proxyIp,&him.ip_addr,0); // ����������ַ
	}
	else{				 // ��ʹ�ô���ʱ
		him.family = AF_INET;
		him.port = htons(port);	//set current server's service port
		e = sci_parse_host(ip,&him.ip_addr,0); // ����������ַ
	}
	if(0 != e)
	{
		SCI_TRACE_LOW("--Socket TCP Connection host parse err:%d,him.ip_addr %d",e,him.ip_addr);
		g_state_info.lasterr = SG_ERR_HOST_PARSE;
		goto err;
	}

//	SCI_TRACE_LOW("--Socket TCP Connection ip %s,port %d porxyEnable %d",ip,port,porxyEnable);
	e = sci_sock_connect(g_state_info.socket,(struct sci_sockaddr*)&him, 0); // ����socket

	g_gprs_t.state = GPRS_SOCKET_CONNECT;
	g_gprs_t.timeout =0;
	return;
err:
	SG_Reconnect_Socket();
	return ;
}

void SG_Net_Init_Gprs(void)
{
	#if(__cwt_)
	SG_Net_Init_Udp();
	#else
	SG_Net_Init_Tcp(1,g_set_info.sCenterIp,g_set_info.nCenterPort,g_set_info.sProxyIp,g_set_info.nProxyPort,g_set_info.bProxyEnable);
	#endif
}

// �Ͽ�SOCKET��������
void SG_Reconnect_Socket(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	SCI_TRACE_LOW("--SG_Reconnect_Socket:last err %d",g_state_info.lasterr);

#if(__cwt_)
	SG_Close_an_Udp();
#else
	socket_close_an_tcp();
	g_state_info.socket = -1;
#endif
	SG_CreateSignal(SG_CREATE_SOCKET,0, &signal_ptr);
}

// ��ͨ�汾�����ִ���
void SG_Do_Handup_Tcp(void)
{
	char *buffer = NULL;
	int len =0,e;
	unsigned char *out = NULL;
	int outlen;

	if(g_state_info.socket < 0)
		return;

	if(g_set_info.bProxyEnable){  //ʹ�ô���
		buffer = SCI_ALLOC(500);
		if(buffer == NULL)
			return ;
		SCI_MEMSET(buffer,0,sizeof(buffer));
		sprintf(buffer, "CONNECT %s:%d HTTP/1.1\r\nProxy-Connection: Keep-Alive\r\n\r\n", g_set_info.sCenterIp, g_set_info.nCenterPort);
		// ����
		len = sci_sock_send(g_state_info.socket, buffer, strlen(buffer), 0);
		if(len < strlen(buffer))
		{
			e = sci_sock_errno(g_state_info.socket);
			SCI_TRACE_LOW("--SG_Do_Handup: error:%d\n",e);
			SCI_FREE(buffer);
			g_state_info.lasterr = SG_ERR_SEND_DATA;
			goto err;
		}
		g_gprs_t.state	 = GPRS_PORXY;
		g_gprs_t.timeout = 1;
		g_state_info.nPorxyConCnt++;
		SCI_FREE(buffer);
	}
	else{                        // ��ʹ�ô���
		SCI_MEMSET(GPRS_RxBuf,0,sizeof(GPRS_RxBuf));
		GPRS_WRIndex =0;
		GPRS_RxHead =1;

		if (MsgWatchHanleUp(1, &out, &outlen) != MSG_TRUE)
		{
			SCI_TRACE_LOW("--SG_Do_Handup: MsgWatchHanleUp() \r\n");
			goto err;
		}

		if(socket_tcp_client_send((char*)out, outlen)<0)
		{
			SCI_FREE(out);
			SCI_TRACE_LOW("--SG_Do_Handup: MsgWatchHanleUp snd Err \r\n");
			g_state_info.lasterr = SG_ERR_SEND_DATA;
			goto err;
		}
		SCI_FREE(out);

		g_gprs_t.state   = GPRS_HANDUP;
		g_gprs_t.timeout = 1;
		g_state_info.nHandupConCnt++;
	}
	return;
err:
	SG_Reconnect_Socket();
	return;

}


#if (__cwt_)

// ����ͨ�汾�����ִ���
void SG_Do_Handup_Cwt(void){
	char *buffer = NULL;
	int len =0,e;
	unsigned char *out = NULL;
	int outlen;
	xSignalHeaderRec      *signal_ptr = NULL;


	if (MsgLogInMsg((gps_data*)g_state_info.pGpsCurrent,&out,&outlen) != MSG_TRUE)
	{
		SCI_TRACE_LOW("--SG_Do_Handup: MsgLogInMsg ERR \r\n");
		goto err;
	}

	if(SG_Send_an_Udp((char*)out, outlen)<0)
	{
		SCI_FREE(out);
		SCI_TRACE_LOW("--SG_Do_Handup: Send Login Msg Err!!! \r\n");
		g_state_info.lasterr = SG_ERR_SEND_DATA;
		goto err;
	}
	SCI_FREE(out);

	bhangup = -1; // �ȴ�����Ӧ��
	g_gprs_t.state   = GPRS_HANDUP;
	g_gprs_t.timeout = 1;
	g_state_info.nHandupConCnt++;

	return;
err:

	SG_Reconnect_Socket();
	return;

}
#endif /* (__cwt_) */

// ���ִ���
void SG_Do_Handup(void)
{
	#if (__cwt_)
	SG_Do_Handup_Cwt();
	#else
	SG_Do_Handup_Tcp();
	#endif
}


void SG_Send_Item_Timeout_Handle(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	if(GprsSndTimer) // �ж�GPRS���ķ����Ƿ�ʱ,����1k�İ��ȴ�Ӧ��ʱ���Ϊ60s,��ͨ��30s��ʱ
	{
		int timeout = 30;
		if(msgItem)
		{
			if(msgItem->len >1024)
			{
				timeout = GPRS_RCVANS_TIMEOUT * 2;
			}
			else
			{
				timeout = GPRS_RCVANS_TIMEOUT;
			}
		}
		GprsSndTimer++;
		if(GprsSndTimer >= timeout)
		{
			GprsSndTimer =0;
			SG_CreateSignal(SG_SEND_MSG_TO,0, &signal_ptr);
		}
	}

	 // ��鷢�Ͷ���
	if(SmsSndTimer)
	{
		SmsSndTimer++;
		if(g_set_info.nNetType > MSG_NET_GPRS && SmsSndTimer > g_set_info.nSmsInterval_data)
		{
			SmsSndTimer =1;
			SG_CreateSignal(SG_SEND_SMS,0, &signal_ptr);
		}
		else if(g_set_info.nNetType == MSG_NET_SMS && SmsSndTimer > g_set_info.nSmsInterval)
		{
			SmsSndTimer =1;
			SG_CreateSignal(SG_SEND_SMS,0, &signal_ptr);
		}
	}

}

#if (__cwt_)
void SG_sndmsgby_gprs()
{
	int sndret =0;
	xSignalHeaderRec      *signal_ptr = NULL;

	if(msgItem == NULL)
		msgItem = SG_Send_Get_Alarm_Item();
	if (msgItem == NULL)
		msgItem = SG_Send_Get_New_Item();// ����ͨ�����ж�ȡһ���ڵ�
	/**********�����alarm������ȡ�õĽڵ㲻Ϊ��**********/
	if (msgItem !=NULL)
	{
		sndret = SG_Send_an_Udp(msgItem->msg,msgItem->len);
		if(sndret == 0) // ���ͳɹ�
		{
			SG_Send_Free_Item(&msgItem); // ���ͳɹ������ȴ�Ӧ��ֱ���ͷŶ����еĽڵ�
			SG_CreateSignal(SG_SEND_MSG,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
			return;
		}
		else
		{

			SCI_TRACE_LOW("@_@ ----:SG_sndmsgby_gprs snd Item err%d",sndret);
			g_gprs_t.state = SG_ERR_SEND_UDP;
			SG_Reconnect_Socket();
			return;
		}
	}
}

#else

void SG_sndmsgby_gprs()
{
	int sndret =0;
	xSignalHeaderRec      *signal_ptr = NULL;

	if(GprsSndTimer > 0)
		return;
	if(msgItem == NULL)
		msgItem = SG_Send_Get_Alarm_Item();
	if (msgItem == NULL)
		msgItem = SG_Send_Get_New_Item();// ����ͨ�����ж�ȡһ���ڵ�
	/**********�����alarm������ȡ�õĽڵ㲻Ϊ��**********/
	if (msgItem !=NULL)
	{
		sndret = socket_tcp_client_send(msgItem->msg,msgItem->len);
		if(sndret < 0)
		{

			SCI_TRACE_LOW("@_@ ----:socket_tcp_client_send snd Item err%d",sndret);
			SG_Reconnect_Socket();
			// post req data connect
			GprsSndTimer = 0;

			return;
		}
		GprsSndTimer = 1;
	}
}

#endif



int SG_Receive_an_Udp(char* data_ptr, int data_len, void* data,uint16 srcport)
{

	xSignalHeaderRec      *signal_ptr = NULL;
	char sendbuf[1000] = "";
	int sendlen = 0;

	if( data != temp_s_label)
	{
		//the packet is not mine
		SCI_FREE(data_ptr);
		return -1;
	}
	g_state_info.nUdpRcvCnt ++;
	SCI_TRACE_LOW("--SG_Receive_an_Udp data: datalen %d \n",data_len);

#if (0)
	int i;

	for( i = 0; i < data_len; i++)
	{
		SCI_TRACE_LOW("%02x ", *(data_ptr + i));
	}
#endif /* (0) */
	SCI_MEMCPY(sendbuf,(char *)&data_len,4);
	SCI_MEMCPY(sendbuf + 4,data_ptr,data_len);
	sendlen = data_len + 4;

	SG_CreateSignal_Para(SG_RCV_BUF,sendlen, &signal_ptr,sendbuf); // �����źŵ���ѭ��Ҫ�����������


	//free the memory
	SCI_FREE(data_ptr);
	return 0;
}

int SG_Open_an_Udp(void)
{
	struct sci_sockaddr him;
	unsigned short fport;
	unsigned short lport;
	unsigned long fhost;
	void* lable;
	void* ptr = NULL;
	int e;

	e = sci_parse_host(g_set_info.sCenterIp,&temp_s_fhost,0); // ����������ַ
	if(0 != e)
	{
		SCI_TRACE_LOW("--SG_Open_an_Udp host parse err:%d,him.ip_addr %d",e,him.ip_addr);
		return -1;
	}

	fport = g_set_info.nCenterPort; //default 0
	lport = temp_s_lport; //arbitary
	lable = temp_s_label;
	ptr = (void *)sci_udp_open(fhost, fport, &lport, SG_Receive_an_Udp,lable);
	temp_s_lport = lport;
	if(ptr == NULL)
	{
		SCI_TRACE_LOW("--SG_Open_an_Udp:Fail to Open a Lightweight UDP Connection!\n");
		g_state_info.lasterr = SG_ERR_ACTIVE_PDP;
		return -1;
	}
	temp_s_udpconn = ptr;
	SCI_TRACE_LOW("--SG_Open_an_Udp:Success to Open a Lightweight UDP Connection!\n lport %d",lport);
	SCI_TRACE_LOW("--SG_Open_an_Udp:Open %s :%d\n",g_set_info.sCenterIp,g_set_info.nCenterPort);
	return 0;
}

int SG_Send_an_Udp(char *data_ptr,int data_len)
{
	int e;

	if(temp_s_udpconn == NULL)
	{
		SCI_TRACE_LOW( "--SG_Send_an_Udp: Have Not Open An Available Link!!!\r\n");
		return -1;
	}

	g_state_info.nUdpSendCnt ++;
	e = sci_udp_send(temp_s_fhost, g_set_info.nCenterPort, temp_s_lport, data_ptr, data_len);
	SCI_TRACE_LOW("--SG_Send_an_Udp: data_len %d e %d",data_len,e);
	if(e < 0)
	{
		/*send error*/
		return -1;
	}
	else
	{
		return 0;
	}
}


//app close udp function
void SG_Close_an_Udp(void)
{
	if(NULL != temp_s_udpconn)
	{
		sci_udp_close(temp_s_udpconn);
		temp_s_udpconn = NULL;
		SCI_TRACE_LOW("--SG_Close_an_Udp:Close an Lightweight UDP Connection!\n");
	}
}


BOOLEAN SG_Check_Socket_State(void){
	int16 state = 0;
	int e = 0;
	xSignalHeaderRec      *signal_ptr = NULL;

	if(g_state_info.socket < 0){
		return FALSE;
	}
	e = sci_sock_getsockstate(g_state_info.socket,&state);
	if((0 == e) && (state & SO_ISCONNECTED))
	{
		SG_CreateSignal(SG_HANDUP,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}




// GPRS��ʱ����
void SG_Do_GprsLink(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;
	int overtime;

    // ��绰�����У����ж�GPRS״̬
    if (PhoneFlag != 0)
    {
        return;
    }
	g_gprs_t.timeout++;

	switch (g_gprs_t.state){
		case GPRS_ACTIVE_PDP:
			SCI_TRACE_LOW("===========GPRS_ACTIVE_PDP:%d",g_gprs_t.timeout);
			if(ReConCnt<6){
				if(g_gprs_t.timeout > ACTIVE_OVERTIME_1)
				{
					MN_PLMN_T   select_plmn = {0,0,2};

					MNPHONE_SelectPLMN(FALSE,select_plmn);

					SG_Net_Disconnect();
					SCI_TRACE_LOW("----GPRS TIMEOUT ACTIVE");
				}
			}
			else
			{
				if(g_gprs_t.timeout > ACTIVE_OVERTIME_2)
				{
					SG_Net_Disconnect();
					SCI_TRACE_LOW("----GPRS TIMEOUT ACTIVE");
				}
			}
			break;
		case GPRS_PORXY:
			if(g_gprs_t.timeout > PORXY_OVERTIME){
				SG_Reconnect_Socket();
				SCI_TRACE_LOW("----GPRS TIMEOUT PORXY");
			}
			break;
		case GPRS_HANDUP:
			if(g_gprs_t.timeout > HANDUP_OVERTIME){
				SG_Reconnect_Socket();
				SCI_TRACE_LOW("----GPRS TIMEOUT HAND UP");
			}
			break;
		case GPRS_DEACTIVE_PDP:
			SCI_TRACE_LOW("===========GPRS  DEACTIVE PDP:%d",g_gprs_t.timeout);
			if(g_gprs_t.timeout > DEACTIVE_OVERTIME){
				SG_Net_Disconnect();
				SCI_TRACE_LOW("----GPRS TIMEOUT DEACTIVE PDP");
			}
			break;
		case GPRS_RXHEAD:
		case GPRS_RXCONTENT:
			#if (__cwt_)
			if(g_gprs_t.timeout > RCVDATA_OVERTIME_CWT){
				SG_Reconnect_Socket();
				SCI_TRACE_LOW("----GPRS TIMEOUT RCV DATA");
			}
			#else
			if(g_set_info.Heart_Switch)
			{
				if(g_gprs_t.timeout > g_set_info.Heart_Interval*2)
				{
					SG_Reconnect_Socket();
					SCI_TRACE_LOW("----GPRS TIMEOUT RCV DATA heart");
				}
			}
			else
			{
				if(g_gprs_t.timeout > RCVDATA_OVERTIME)
				{
					SG_Reconnect_Socket();
					SCI_TRACE_LOW("----GPRS TIMEOUT RCV DATA");
				}
			}
			#endif
			break;
		case GPRS_SOCKET_CONNECT:
			if(ReConCnt < 6)
			{
				overtime = SOCKCON_OVERTIME;
			}
			else
			{
				overtime = ACTIVE_OVERTIME_2;
			}
			if(g_gprs_t.timeout > overtime){
				SG_Reconnect_Socket();
				SCI_TRACE_LOW("----GPRS TIMEOUT GPRS_SOCKET_CONNECT");
			}
			else if(g_gprs_t.timeout %3 == 0){
				g_gprs_t.timeout ++; //
				SG_CreateSignal(SG_CHECK_SOCKET_CONNECT,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
				SCI_TRACE_LOW("----GPRS SG_CHECK_SOCKET_CONNECT: %d",g_gprs_t.timeout);
			}
			break;

		default:
			return;
	}
	g_state_info.nNetTimeOut = g_gprs_t.state;
	g_state_info.lasterr = SG_ERR_TIME_OUT;

}

/****************************************************************
  ��������	socket_tcp_client_send
  ��  ��  �������ķ�������
  ���������pchar: ��������
  			length: �������ĳ���
  �����������
  ����ֵ��	0: �ɹ� -1: ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2007-9-2
****************************************************************/
int socket_tcp_client_send(char * pchar, int length)
{

	int len, e;
	char *pdu;

	SCI_TRACE_LOW("********socket_tcp_client_send*******");

	// �������
	if((length <= 0) || (pchar == NULL ))
	{
		SCI_TRACE_LOW("--socket_tcp_client_send:  wrong para.");
		return -1;
	}

	//�ն�δע��
	if (g_set_info.bRegisted == 0)
	{
		SCI_TRACE_LOW("--socket_tcp_client_send: TERMINAL NOT REGISTER");
		return -1;
	}

	DataLedCnt = 10;//������������

	pdu = SCI_ALLOC(length+10);

	if(pdu == NULL)
		return -1;

	SCI_MEMSET(pdu,0,length+10);

	pdu[0] = 0x7e;
	pdu[1] = 0;
	SCI_MEMCPY(pdu+2, &length, 4);
	SCI_MEMCPY(pdu+6, pchar, length);


	len = sci_sock_send(g_state_info.socket, pdu, length+6, 0);// ���ͱ���
	if(len < (length+6) || len < 0)
	{
		e = sci_sock_errno(g_state_info.socket);
		SCI_FREE(pdu);
		SCI_TRACE_LOW("--socket_tcp_client_send: error:%d\n",e);
		return -1;
	}
	SCI_TRACE_LOW("-----------------------------");
	SCI_TRACE_LOW("--socket_tcp_client_send:0x%02x 0x%02x len:%d",pdu[19],pdu[20],len);
	SCI_TRACE_LOW("-----------------------------");
	SCI_FREE(pdu);
	return 0;
}


/****************************************************************
  ��������	socket_tcp_client_recv
  ��  ��  �������Ľ�������
  ���������pBuf: ���ձ��Ĵ�ŵ�buffer
  			size: ���ձ��Ĵ�ŵ�buffer����
  �����������
  ����ֵ��	0: �ɹ� -1: ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2007-9-2
****************************************************************/
int socket_tcp_client_recv(char * pBuf, int size)
{
	int len;
	int e;

	SCI_ASSERT( pBuf!=PNULL );

	//�ն�δע��ֱ�ӷ���
	if (g_set_info.bRegisted == 0)
	{
		SCI_TRACE_LOW("--socket_tcp_client_recv: TERMINAL NOT REGISTER");
		return -1;
	}

	//ͨ�ŷ�ʽΪ���ţ�ֱ�ӷ���
	if (g_set_info.nNetType == MSG_NET_SMS)
	{
		SCI_TRACE_LOW("--socket_tcp_client_recv: USE SMS, NOT NEED SOCKET RCV !!!");
		return -1;
	}

	//socket ��������
	if(g_state_info.socket == -1)
	{
		return -1;
	}

	len = sci_sock_recv(g_state_info.socket, pBuf, size/*TCP_MSS*/, 0);
	if(len < 0)// ����ʧ��
	{
		e = sci_sock_errno(g_state_info.socket);
		SCI_TRACE_LOW("--socket_tcp_client_recv:error:%d",e);
		return -1;
	}
	else if(len > 0) // ���ճɹ�
	{
		SCI_TRACE_LOW("--socket_tcp_client_recv: len is:%d",len);
		return len;
	}
	else //  len = 0
	{
		SCI_TRACE_LOW("--socket_tcp_client_recv:len=0");
		return 0;
	}

}

/****************************************************************
  ��������	socket_close_an_tcp
  ��  ��  ���ر�tcp����
  �����������
  �����������
  ����ֵ��	0: �ɹ� 	-1:ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2007-9-2
****************************************************************/
int socket_close_an_tcp(void)
{
	int ret;
	xSignalHeaderRec      *signal_ptr = NULL;

	//SG_CreateSignal(SG_SEND_NETSTAT,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
	SG_Send_Net_State(0);

	if(g_state_info.socket < 0)
		return 0;

	g_state_info.nReConnect ++;

	ret = sci_sock_socketclose(g_state_info.socket); // �ر�socket����

	if(ret < 0)
	{
		ret = sci_sock_errno(g_state_info.socket);
		SCI_TRACE_LOW("----socket_close_an_tcp:error:%d",ret);
		g_state_info.lasterr = SG_ERR_CLOSE_SOCKET;
		return -1;
	}
	else{
		g_state_info.socket =-1;
		return 0;
	}

}

// active pdp �������¼����صĴ���
void SG_Active_Pdp_Handle(int result)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	if(0 == result) // active �ɹ�������SOCKET
	{
		SG_CreateSignal(SG_CREATE_SOCKET,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
	}
	else
	{
		g_state_info.ActiveErr = result;
		SCI_TRACE_LOW("--SG_Active_Pdp_Handle:Active err %d",result);
	}

}


/****************************************************************
  ��������	MnGPRS_EventCallback
  ��  ��  ��GPRS���ҵ���callback����
  ���������uint32 	event_id  �¼���id
  			uint32 	task_id
  			void*	param_ptr
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void MnGPRS_EventCallback(
                                uint32 task_id, //task ID
                                uint32 event_id, //event ID
                                void *param_ptr
                                )
{
	APP_MN_GPRS_T *p_mn_gprs_t = (APP_MN_GPRS_T*)param_ptr;
	SCI_TRACE_LOW("--MnGPRS_EventCallback: %d",p_mn_gprs_t->result);

	SCI_ASSERT(PNULL != param_ptr);
	if( PNULL == param_ptr )
	{
		SCI_TRACE_HIGH("-----mmi_callback: the ponit of param is NULL");
		return;
	}

	// distinguish the different message type to set the signal_size and signal_code
	switch(event_id)
	{
	/*
	EV_MN_APP_SET_PDP_CONTEXT_CNF_F = (MN_APP_GPRS_SERVICE << 8),
	EV_MN_APP_SET_MIN_QOS_CNF_F,
	EV_MN_APP_SET_QOS_CNF_F,
	EV_MN_APP_ACTIVATE_PDP_CONTEXT_CNF_F,
	EV_MN_APP_DELETE_PDP_CONTEXT_CNF_F,
	EV_MN_APP_DELETE_MIN_QOS_CNF_F,
	EV_MN_APP_DELETE_QOS_CNF_F,
	EV_MN_APP_DEACTIVATE_PDP_CONTEXT_CNF_F,
  	EV_MN_APP_MTACT_RESPOND_MODE_CNF_T,
	EV_MN_APP_PUSH_GPRS_DATA_CNF_F,
	EV_MN_APP_DEACTIVATE_PDP_CONTEXT_IND_F,
	MAX_MN_APP_GPRS_EVENTS_NUM,
	*/

	case EV_MN_APP_SET_PDP_CONTEXT_CNF_F:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_SET_PDP_CONTEXT_CNF_F");

		break;

	case EV_MN_APP_SET_MIN_QOS_CNF_F:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_SET_MIN_QOS_CNF_F");

		break;

	case EV_MN_APP_SET_QOS_CNF_F:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_SET_QOS_CNF_F");

		break;

	case EV_MN_APP_ACTIVATE_PDP_CONTEXT_CNF_F:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_ACTIVATE_PDP_CONTEXT_CNF_F");
      	SCI_TRACE_LOW("--MnGPRS_EventCallback: %d",p_mn_gprs_t->result);
		SG_Active_Pdp_Handle(p_mn_gprs_t->result);
		break;

	case EV_MN_APP_DELETE_PDP_CONTEXT_CNF_F:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_DELETE_PDP_CONTEXT_CNF_F");

		break;

	case EV_MN_APP_DELETE_MIN_QOS_CNF_F:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_DELETE_MIN_QOS_CNF_F");

		break;

	case EV_MN_APP_DELETE_QOS_CNF_F:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_DELETE_QOS_CNF_F");

		break;

	case EV_MN_APP_DEACTIVATE_PDP_CONTEXT_CNF_F:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_DEACTIVATE_PDP_CONTEXT_CNF_F");
     	SCI_TRACE_LOW("--MnGPRS_EventCallback: %d",p_mn_gprs_t->result);
		SG_Redial_Handle(p_mn_gprs_t->result);
		break;

	case EV_MN_APP_MTACT_RESPOND_MODE_CNF_T:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_MTACT_RESPOND_MODE_CNF_T");
		break;

	case EV_MN_APP_PUSH_GPRS_DATA_CNF_F:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_PUSH_GPRS_DATA_CNF_F");
		break;

	case EV_MN_APP_DEACTIVATE_PDP_CONTEXT_IND_F:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: EV_MN_APP_DEACTIVATE_PDP_CONTEXT_IND_F");
  		SCI_TRACE_LOW("--MnGPRS_EventCallback: %d",p_mn_gprs_t->result);
		SG_Redial_Handle(p_mn_gprs_t->result);
		break;

	case MAX_MN_APP_GPRS_EVENTS_NUM:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: MAX_MN_APP_GPRS_EVENTS_NUM");
		break;

	default:
		SCI_TRACE_LOW("--MnGPRS_EventCallback: default event_id=%d",event_id);
		break;
	}
}
// ��ս��յ�buf�ͽ��ճ��ȵ�
void SG_Clear_Rcv(void)
{
//	GPRS_RxHead  = 0;
	SCI_TRACE_LOW("--SG_Clear_Rcv!!!!!!%d %d",GPRS_RxLen,GPRS_WRIndex);
	GPRS_RxLen 	 = 0;
	GPRS_WRIndex = 0;
	SCI_MEMSET(GPRS_RxBuf,0,GPRS_RXBUF_SIZE);

}

/****************************************************************
  ��������	SG_GPRS_GetData
  ��  ��  ��GPRS��������
  �����������
  �����������
  ����ֵ��	��
  ��д��  ���º���
  �޸ļ�¼��������2007-9-2
****************************************************************/
void SG_GPRS_GetData(             const uint32 so, // socket
             int32 fin        // socket fin flag, 1 - socket closed
)
{
	#define     EWOULDBLOCK    6

	xSignalHeaderRec      *signal_ptr = NULL;
	int len=0;
	int e=0;

	SCI_TRACE_LOW("--SG_GPRS_GetData enter %x,%x,%d,WR Index %d",so,g_state_info.socket,fin,GPRS_WRIndex);

	if(g_state_info.socket != so){
		return;
	}

	if(fin){
		g_gprs_t.state=GPRS_EWOULDBLOCK;
		g_gprs_t.timeout=1;
		goto err;
	}

	len = socket_tcp_client_recv(GPRS_RxBuf+GPRS_WRIndex, GPRS_RXBUF_SIZE-GPRS_WRIndex);

	if( len < 0)	// ����ʧ��
	{
		e = sci_sock_errno(g_state_info.socket);
		if(e == EWOULDBLOCK){
			SCI_TRACE_LOW("--SG_GPRS_GetData: tcp recv error:EWOULDBLOCK!! GPRS_RxHead:%d ,GPRS_WRIndex:%d,e = %d",GPRS_RxHead,GPRS_WRIndex,e);
			if(EwouldblockCnt++ < 5){
//				GPRS_RxHead  = 0;
//				SG_Clear_Rcv();
				g_gprs_t.state=GPRS_EWOULDBLOCK;
				g_gprs_t.timeout=1;

			}
			else{
				SCI_TRACE_LOW("--SG_GPRS_GetData ERR EwouldblockCnt %d",EwouldblockCnt);
				EwouldblockCnt =0;
				goto err;
			}
			return;
		}
		else{
			SCI_TRACE_LOW("--SG_GPRS_GetData: tcp recv error:GPRS_RxHead:%d ,GPRS_WRIndex:%d,e = %d",GPRS_RxHead,GPRS_WRIndex,e);
			goto err;
		}
	}
	else if(len == 0){
			SCI_TRACE_LOW("--SG_GPRS_GetData: tcp recv error:len = 0,GPRS_RxHead:%d ,GPRS_WRIndex:%d",GPRS_RxHead,GPRS_WRIndex);
			goto err;
	}

	GPRS_WRIndex += len;
	EwouldblockCnt = 0;
	SG_CreateSignal(SG_RCV_BUF,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����������
	return;
err:
	g_state_info.lasterr = SG_ERR_RCV_DATA;
	GPRS_RxHead  = 0;
	SG_Clear_Rcv();
	SG_Reconnect_Socket();
	return;
}



/****************************************************************
  ��������	SG_GPRS_SEND_ITEM
  ��  ��  ��ͨ��GPRS��ʽ���ͱ���
  �����������
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void SG_GPRS_SEND_ITEM(int SignalCode)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	#if (__cwt_)
	if(temp_s_udpconn == NULL) // UDP������ʱֱ�ӷ���
	{
		SCI_TRACE_LOW("--The Udp is Unavailable!!");
		return;
	}

	#else
	if(g_state_info.socket == -1) // SOCKET������ʱֱ�ӷ���
	{
		return;
	}
	#endif

	switch(SignalCode)
	{
		case SG_SEND_MSG:  //  �нڵ㱻���������
			SG_sndmsgby_gprs();
			break;
		case SG_ACK_MSG:	// gprs���յ�Ӧ��֮��
			//�ͷŽڵ�
			GprsSndTimer =0;
			SG_Send_Free_Item(&msgItem);
			SG_sndmsgby_gprs();
			break;
		case SG_SEND_MSG_TO: // gprs���ͳ�ʱ
			//����3�Σ��ж������Ѿ��Ͽ�
			if(++gprs_snd_tmout_cnt > 2)
			{

				SCI_TRACE_LOW("@_@ ----use sms to send time out alarmItem!----");
				gprs_snd_tmout_cnt = 0;// gprs���ͳ�ʱ��������������
				//send by sms and free alarmItem
#if (0)
				if(g_set_info.nNetType == MSG_NET_UNION){// ��Ϸ�ʽ�Ķ��ŷ���
					SCI_MEMSET(smsbuf,0,sizeof(smsbuf));
					smsbuflen = msgItem->len;
					SCI_MEMCPY(smsbuf,msgItem->msg,smsbuflen);
					SG_Send_Free_Item(&msgItem);
//					SG_CreateSignal(SG_SEND_SMS,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					SendMsgReqToMN((uint8*)g_set_info.sCenterNo,(uint8*)smsbuf,smsbuflen,MN_SMS_8_BIT_ALPHBET);
				}
#endif /* (0) */
				SG_Reconnect_Socket();
			}
			else
			{
				SCI_TRACE_LOW("----use sms to send time out  %d!",gprs_snd_tmout_cnt);
				SG_sndmsgby_gprs(); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
			}
			break;
		default:
			SCI_TRACE_LOW("<<<<<<receive other signal !!! %x",SignalCode);
			return;
			break;
	}
}


/****************************************************************
  ��������	SG_SMS_SEND_ITEM
  ��  ��  ��ͨ�����ŷ�ʽ����һ������
  �����������
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void SG_SMS_SEND_ITEM(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;
	
	if(g_set_info.bRegisted != 1)
		return;
#if (0)
	// ����buf��Ϊ��
	if(smsbuflen > 0 && smsbuf[0])
	{
		SCI_TRACE_LOW("@_@1 ----get SMS task(smsbuf) to snd len %d!----",smsbuflen);
		// here construct the sms data struct and send
		if (0 == SendMsgReqToMN((uint8*)g_set_info.sCenterNo,(uint8*)smsbuf,smsbuflen,MN_SMS_8_BIT_ALPHBET))
		{
			// ���buf
			SCI_MEMSET(smsbuf,0,sizeof(smsbuf));
			smsbuflen = 0;
		}
		else
		{
		}
		return;
	}
#endif /* (0) */

	if(g_set_info.nNetType){
		if((g_set_info.nNetType == 1)||((g_gprs_t.state == GPRS_HANDUP)&&(g_set_info.bProxyEnable == 0))||(g_gprs_t.state == GPRS_RXCONTENT)||(g_gprs_t.state == GPRS_RXHEAD))
		{
            SG_CreateSignal(SG_SEND_MSG,0, &signal_ptr); 
			SCI_TRACE_LOW("======SG_SMS_SEND_ITEM: return");
			return ; //gprs ģʽ
		}
	}

	//�����alarm������ȡ�õĽڵ㲻Ϊ��
	if (msgItem !=NULL)
	{
#if (0)
		if(g_set_info.nNetType > 0)
		{
			if(msgItem->type ==2 || msgItem->type == 3)
			{
				SG_Send_Put_New_Item(msgItem->type,msgItem->msg,msgItem->len);
				SCI_FREE(msgItem);
				msgItem = NULL;
				return;
			}
		}
#endif /* (0) */
		SCI_TRACE_LOW("@_@2 ----get SMS task(type %d) to snd len %d!----",msgItem->type,msgItem->len);
		SCI_MEMSET(smsbuf,0,sizeof(smsbuf));
		SCI_MEMCPY(smsbuf,msgItem->msg,msgItem->len);
		smsbuflen = msgItem->len;
		SendMsgReqToMN((uint8*)g_set_info.sCenterNo,(uint8*)msgItem->msg,msgItem->len,MN_SMS_8_BIT_ALPHBET);
		SG_Send_Free_Item(&msgItem);
		return;
	}
	//���ȶ�ȡ���ͱ�������
	else if(msgItem == NULL)
	{
		msgItem = SG_Send_Get_Alarm_Item();
		if (msgItem == NULL)
		{
			//if(g_set_info.nNetType > 0)
				//msgItem = SG_Send_Get_New_Item_type();
			//else
			msgItem = SG_Send_Get_New_Item();
		}

		if(msgItem !=NULL)
		{
			SCI_TRACE_LOW("@_@3 ----get SMS task(type %d) to snd len %d!----",msgItem->type,msgItem->len);
			SCI_MEMSET(smsbuf,0,sizeof(smsbuf));
			SCI_MEMCPY(smsbuf,msgItem->msg,msgItem->len);
			smsbuflen = msgItem->len;
			SendMsgReqToMN((uint8*)g_set_info.sCenterNo,(uint8*)msgItem->msg,msgItem->len,MN_SMS_8_BIT_ALPHBET);
			SG_Send_Free_Item(&msgItem);
			return;
		}
		else
		{
			SCI_TRACE_LOW("======  msgItem==NULL");
		}
	}

}


/****************************************************************
  ��������	SG_Net_Disconnect
  ��  ��  ��GPRS��������
  �����������
  �����������
  ����ֵ  ��0 : �ɹ� -1 : ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2007-9-2
****************************************************************/
void SG_Net_Disconnect(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	#if (__cwt_)
		SG_Close_an_Udp();
	#else
		socket_close_an_tcp();
		g_state_info.socket = -1;
	#endif
	SG_CreateSignal(SG_DEACTIVE_PDP,0, &signal_ptr);
}



// GPRS�������ݴ���
void SG_Do_Rcv_Tcp(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;


	switch(GPRS_RxHead)
	{
		case 0://���յ�http���ò���
		{
			char *out = NULL;
			int outlen = 0;

			{
				int i=0;
				SCI_TRACE_LOW("--SG_Do_Rcv_Tcp:rx head %d,wr index %d,rxlen %d",GPRS_RxHead,GPRS_WRIndex,GPRS_RxLen);
#if (0)
				for(i=0;i<GPRS_WRIndex;i++)
					SCI_TRACE_LOW("--SG_Do_Rcv_Tcp:%d 0x%02x",i,GPRS_RxBuf[i]);
#endif /* (0) */
			}
			if(strstr(GPRS_RxBuf,"403"))
			{
				SCI_TRACE_LOW("--SG_Do_Rcv_Tcp: MsgWatchHanleUp 403 Err \r\n");
				g_state_info.lasterr = SG_ERR_PROXY_NOTFOUND;
				goto err;
			}
			else if(strstr(GPRS_RxBuf,"404"))
			{
				SCI_TRACE_LOW("--SG_Do_Rcv_Tcp: MsgWatchHanleUp 404 Err \r\n");
				g_state_info.lasterr = SG_ERR_PROXY_DENY;
				goto err;
			}
			else
			{
				//������Ҫ�������ֱ���
				//���Ĳ��ܸ���ID�ж�����

				GPRS_RxHead = 1;
				if (MsgWatchHanleUp(1, (MsgUChar**)&out, (MsgInt*)&outlen) != MSG_TRUE)
				{
					SCI_TRACE_LOW("--SG_Do_Rcv_Tcp: MsgWatchHanleUp() err\r\n");
					goto err;
				}

				if(socket_tcp_client_send(out, outlen)<0)
				{
					SCI_FREE(out);
					SCI_TRACE_LOW("--SG_Do_Rcv_Tcp: MsgWatchHanleUp snd Err \r\n");
					g_state_info.lasterr = SG_ERR_SEND_DATA;
					goto err;
				}

				SCI_FREE(out);

				g_gprs_t.state = GPRS_HANDUP;
				g_gprs_t.timeout = 1;
				g_state_info.nHandupConCnt++;
			}

			GPRS_WRIndex = 0;
			SCI_MEMSET(GPRS_RxBuf,0,GPRS_RXBUF_SIZE);
		}

			break;
		case 1:	//���հ�ͷ(6�ֽ�)
			//��ӡ�յ��ı���
			{
				int i;
				SCI_TRACE_LOW("--SG_Do_Rcv_Tcp:rx head %d,wr index %d,rxlen %d",GPRS_RxHead,GPRS_WRIndex,GPRS_RxLen);
				//for(i=0;i<GPRS_WRIndex;i++)
				//SCI_TRACE_LOW("--SG_Do_Rcv_Tcp:%d,0x%02x ",i,GPRS_RxBuf[i]);
			}

			g_gprs_t.state = GPRS_RXHEAD;
			g_gprs_t.timeout=1;
			g_state_info.Rcv_Data_To = 1;

			while(GPRS_WRIndex > 0){
				if(memcmp(GPRS_RxBuf,"\x7e\x80\x00\x00\x00\x00",6)==0){
					if(bhangup == -1){
						SCI_TRACE_LOW("--SG_Do_Rcv_Tcp: Rev HandleUp ans--");
						bhangup = 1;  // ���ֳɹ�
						ReConCnt =0;
						ReDeactiveCnt =0;
						reConsocket =0;
						SG_CreateSignal(SG_SEND_MSG,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					}
					else
					{
						SG_CreateSignal(SG_ACK_MSG,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
					}
					SG_Send_Net_State(1);
					SG_Send_OptrRexlev(0);

					gprs_snd_tmout_cnt = 0; //gprs���ͳ�ʱ����(�ۼ����γ���90s�Ͽ�����)

					GPRS_WRIndex = ((GPRS_WRIndex >6) ? (GPRS_WRIndex-6):0);
					if(GPRS_WRIndex >0){
						SCI_MEMCPY(&GPRS_RxBuf[0],&GPRS_RxBuf[6],GPRS_WRIndex);
						continue;
					}
					else{
						SCI_MEMSET(&GPRS_RxBuf[0],0,sizeof(GPRS_RxBuf));
						break;
					}
				}
				//���ձ��ĳ���
				else if(GPRS_RxBuf[0] == 0x7e && GPRS_RxBuf[1] == 0x00){

					//��ȡ��Ҫ�����ܳ���
					SCI_MEMCPY((char *)&GPRS_RxLen,GPRS_RxBuf+2,4);
					g_state_info.GpsEnable = 1;
//					SCI_TRACE_LOW("--SG_Do_Rcv_Tcp: 1 GPRS_RxLen=%d",GPRS_RxLen);

					// ��������֧�ֵĳ��ȵİ�(1024*10)���账��ֱ�ӷ���
					//Ŀǰ�� ֧��1k���ȵı���
					if(GPRS_WRIndex >= GPRS_RXBUF_SIZE || GPRS_RxLen > GPRS_RXBUF_SIZE/2 || GPRS_RxLen <= 0){
						GPRS_RxHead  = 1;
						SG_Clear_Rcv();
						break;
					}
					//���Ĵ���
					ReConCnt = 0;
					g_gprs_t.state = GPRS_RXCONTENT;
					g_gprs_t.timeout=1;

//					SCI_TRACE_LOW("--SG_Do_Rcv_Tcp: 2 GPRS_WRIndex=%d",GPRS_WRIndex);


					if(GPRS_WRIndex >= GPRS_RxLen+6){// �Ѿ��������еı��ģ����н���
						//ֱ�ӷ���ACK��
						if(SG_Net_Ack() == -1){
							goto err;
						}

						SG_Rcv_Handle_Msg((unsigned char*)&GPRS_RxBuf[6],GPRS_RxLen);
						GPRS_RxLen += 6;
						GPRS_RxHead  = 1;

						if(GPRS_WRIndex > GPRS_RxLen){
//							SCI_TRACE_LOW("-- WR INDEX %d RX LEN %d",GPRS_WRIndex,GPRS_RxLen);
							GPRS_WRIndex -= GPRS_RxLen;
							SCI_MEMCPY(&GPRS_RxBuf[0],&GPRS_RxBuf[GPRS_RxLen],GPRS_WRIndex);
							GPRS_RxLen   = 0;
							continue;
						}
						else{
							SG_Clear_Rcv();
							break;
						}
					}
				}
				else{//��������
					SCI_TRACE_LOW("--SG_Do_Rcv_Tcp:rev other");
					GPRS_RxHead  = 1;
					SG_Clear_Rcv();
					break;
				}
				if(g_state_info.Rcv_Data_To > 30)
				{
					SCI_TRACE_LOW("--SG_Do_Rcv_Tcp:receive data timeout!!");
					g_state_info.Rcv_Data_To = 0;
					break;
				}
			}
			break;
		default:
			goto err;
			break;
	}
	g_state_info.GpsEnable = 0;

	return;

err:
	g_state_info.GpsEnable = 0;
	GPRS_RxHead  = 0;
	SG_Clear_Rcv();
	SG_Reconnect_Socket();
	return;
}





#if (__cwt_)

void SG_Do_Rcv_Udp(xSignalHeaderRec   *receiveSignal)
{
	xSignalHeaderRec      *signal_ptr = NULL;
	char rxbuf[1000] = "";
	int  buflen = 0;

	memcpy((char *)&buflen,((char*)receiveSignal + 16),4);
	memcpy(rxbuf,((char*)receiveSignal + 20),buflen);

	SCI_TRACE_LOW("--SG_Do_Rcv_Gprs_Udp:buflen %d",buflen);
#if (0)
	{
		int i;
		SCI_TRACE_LOW("--&&&&&&&&&SG_Do_Rcv_Gprs_Udp: %s",GPRS_RxBuf);
		for(i=0;i<GPRS_RxLen;i++)
		{
			SCI_TRACE_LOW("==%02x",GPRS_RxBuf[i]);
		}
	}
#endif /* (0) */


	// δע�ᣬ����¼Ӧ���Ƿ��յ�
	if(g_set_info.bRegisted == 0)
	{
		if(SG_Check_login(rxbuf,buflen) == 0)
		{
			SCI_TRACE_LOW("--SG_Do_Rcv_Gprs_Udp: HANG UP OK\n");
			bhangup = 1;  // ���ֳɹ�
			ReConCnt =0;
			ReDeactiveCnt =0;
			reConsocket =0;
			SG_CreateSignal(SG_SEND_MSG,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
			g_gprs_t.state = GPRS_RXCONTENT;
			g_state_info.nKeepingTime = GetTimeOut(1); // ��ʼ����������
			g_gprs_t.timeout=1;
		}

	}
	// �������ά��Ӧ���� ÿ5�뷢��һ��������������15��û��Ӧ�����ؽ�GPRS����
	else if(SG_Check_Keeping(rxbuf,buflen) == 0)
	{
		SCI_TRACE_LOW("--SG_Do_Rcv_Gprs_Udp: Check Keeping Msg OK!!!\n");
		ReConCnt = 0;
		g_gprs_t.state = GPRS_RXCONTENT;
		g_gprs_t.timeout=1;
		g_state_info.nHeartTotalCnt ++;

	}
	// ��ͨ�·�����
	else
	{
		SG_Rcv_Handle_Msg((unsigned char*)rxbuf,buflen);
	}

	return;
}

#endif


void SG_Do_Rcv_Gprs(xSignalHeaderRec   *receiveSignal)
{
	#if (__cwt_)
	SG_Do_Rcv_Udp(receiveSignal);
	#else
	SG_Do_Rcv_Tcp();
	#endif

}

/****************************************************************
  ��������	SG_Net_Ack
  ��  ��  ��GPRS��������
  �����������
  �����������
  ����ֵ  ��0 : �ɹ� -1 : ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2007-9-2
****************************************************************/
int SG_Net_Ack(void)
{
	uint8 msg[7] = "\x7e\x80\x00\x00\x00\x00";
	int len = 0;
	int ret;

	len = sci_sock_send(g_state_info.socket,(char*)msg, 6, 0);
	if(len < 6)
	{
		ret = sci_sock_errno(g_state_info.socket);
		SCI_TRACE_LOW("--SG_Net_Ack: error:%d",ret);
		return -1;
	}

	SCI_TRACE_LOW("--SG_Net_Ack successful len:%d",len);

	return 0;
}
#if (__cwt_)
//��ȡά�����ӱ���
int SG_Get_ChkKeeping_Msg(){
//C{space}R{space} OEMCODE:COMMADDR|K{space}VERFYCODE\r\n
	unsigned short cksum= 0;
	char scksum[9] = {0};
	char buf[63] = {0};
	int i=0;

	if(g_state_info.sChkKeepMsg[0])
	{
		return strlen(g_state_info.sChkKeepMsg);
	}

	SCI_TRACE_LOW("oem %s, commaddr %s\n",g_set_info.sOemCode,g_set_info.sCommAddr);
	sprintf(buf,"%s:%s|K ",g_set_info.sOemCode,g_set_info.sCommAddr);
//	printf("keep: %s\n",buf);
	while(buf[i]){
		cksum += buf[i];
		i++;
	}

	sprintf(scksum,"%x",cksum);
	SCI_TRACE_LOW("ckum: %s\n",scksum);

	for(i=0;i<8;i++)
		scksum[i] = toupper(scksum[i]);

//	printf("ckum: %s\n",scksum);

	sprintf(g_state_info.sChkKeepMsg,"C R %s%s\r\n",buf,scksum);


//	sprintf(g_state_info.sChkKeepMsg,"C R %s%s\r\n",buf,scksum);
	SCI_TRACE_LOW("keep: %s",g_state_info.sChkKeepMsg);

#if (0)
	{
		int i;
		for(i=0;i<strlen(g_state_info.sChkKeepMsg);i++)
		{
			SCI_TRACE_LOW("@_@%02x",g_state_info.sChkKeepMsg[i]);
		}
	}
#endif /* (0) */
	return strlen(g_state_info.sChkKeepMsg);
}




int SG_Check_Keeping(char* msg,int len)
{
//C{space}R{space} OEMCODE:COMMADDR|K{space}VERFYCODE\r\n
//	char* strmsg = NULL;

	if(msg == NULL)
	{
		SCI_TRACE_LOW("<<<<<<msg is NULL!!");
		return 1;
	}
	if(strlen(msg) != SG_Get_ChkKeeping_Msg()){

		SCI_TRACE_LOW("<<<<<<len not match msg len %d, len%d ,strlen cmpmsg %d\n",strlen(msg),len,strlen(g_state_info.sChkKeepMsg));
		return 1;
	}

	if(memcmp(&msg[0],g_state_info.sChkKeepMsg,strlen(msg))){
		SCI_TRACE_LOW("<<<<<<msg not match\n");
		return 1;
	}
	SCI_TRACE_LOW("SG_Check_Keeping:OK\n");
	return 0;
}

int SG_Check_OemCode(char* OemCode){
	if(!OemCode || !g_set_info.sOemCode[0])
		return 1;

	if(strcmp(OemCode,g_set_info.sOemCode))
		return 1;
	return 0;
}

int SG_Check_CommAddr(char* commaddr){
	if(!commaddr || !g_set_info.sCommAddr[0])
		return 1;
	if(strcmp(commaddr,g_set_info.sCommAddr))
		return 1;
	return 0;
}




//����login���ĵķ����Ƿ���ȷ����0 �ɹ���1 ʧ��
int SG_Check_login(char* msg,int len)
{
//C{space}R{space} OEMCODE:COMMADDR|N{space}VERFYCODE\r\n

	char* strmsg = NULL;

	if(len < 10 || msg == NULL)
		return 1;
	if(memcmp(&msg[0],"C R ",4)){
		return 1;
	}

	strmsg = strtok(&msg[4],":");
	if(SG_Check_OemCode(strmsg)){
		return 1;
	}

	strmsg = strtok(NULL,"|");
	if(SG_Check_CommAddr(strmsg)){
		return 1;
	}

	strmsg = strtok(NULL," ");
	if(strmsg ==NULL)
		return 1;
	if(strmsg[0] != 'N'){
		return 1;
	}

	strmsg = strtok(NULL,"\r");

//	if(strcmp(strmsg,"2C6"))
//		return 1;
	SG_Set_Cal_CheckSum();
//	g_set_info.bKeeping = 3;
	g_set_info.bRegisted =1;
	SG_Set_Save();
	return 0;
}
#endif


void SG_Net_Err_Handle(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	if(SecTimer == 60 && (PLMN_NO_SERVICE == g_state_info.plmn_status || PLMN_EMERGENCY_ONLY == g_state_info.plmn_status))
	{
		MN_PLMN_T     select_plmn = {0,0,2};

	    MNPHONE_SelectPLMN(FALSE,select_plmn);
		SCI_TRACE_LOW("%%%%%%%%Auto Search Network!!!!");
		NetSearchCount ++;
	}
	// ������ʱÿ��180����һ�����磬�����Զ���������
	if((g_state_info.plmn_status == PLMN_NO_SERVICE || g_state_info.plmn_status == PLMN_EMERGENCY_ONLY)  && CheckTimeOut(NetSearchTimer))
	{
		MN_PLMN_T     select_plmn = {0,0,2};

	    MNPHONE_SelectPLMN(FALSE,select_plmn);
		NetSearchTimer = 0xffffffff;
		SCI_TRACE_LOW("%%%%%%%%Auto Search Network!!!!");
		NetSearchCount ++;
		if(NetSearchCount == 6)// �����������磬����
		{
//			SG_Soft_Reset(1);
		}
	}

	if(SecTimer%100 == 0)
	{
		char sig[2] = "";
		static char plmn = 0;
		char net_status = 0;
		net_status = SG_Net_Judge();

		switch(net_status)
		{
			case NO_SIM_CARD:
				sig[0] = ERR_NO_SIM_CARD;
				SG_Send_Net_Err(sig[0]);
				SCI_TRACE_LOW("$$$$$$$$$$$$ IMSI is NULL !!!!");
				break;
			case NO_NET_WORK:
				sig[0] = ERR_NO_NETWORK;
				SG_Send_Net_Err(sig[0]);
				SCI_TRACE_LOW("$$$$$$$$$$$$ No Network!!!!");
				break;
			default:
				if(plmn != g_state_info.plmn_status)
				{
					SG_Send_OptrRexlev(0);
					plmn = g_state_info.plmn_status;
				}
				break;
		}

		if((g_set_info.nNetType != 0))
		{
			SCI_TRACE_LOW("$$$$$$$$$$$$ g_gprs_t.state = %d",g_gprs_t.state);
			if((g_state_info.plmn_status == PLMN_NO_SERVICE || g_state_info.plmn_status == PLMN_EMERGENCY_ONLY)
				&&(g_gprs_t.state != GPRS_RXHEAD || g_gprs_t.state != GPRS_RXCONTENT))
			{
				MN_PLMN_T     select_plmn = {0,0,2};

		   		MNPHONE_SelectPLMN(FALSE,select_plmn);
				SCI_TRACE_LOW("============Auto Search Network!!!!");
			}
			else if((g_gprs_t.state == GPRS_DISCONNECT)&&(g_state_info.SimFlag == 1))
			{
				SG_CreateSignal(SG_ACTIVE_PDP,0, &signal_ptr); // �����źŵ���ѭ��Ҫ��������ACTIVE PDP
			}
		}


	}

#if (1)
	if(SecTimer%60 == 0)
	{
		if(g_gprs_t.state == GPRS_RXHEAD || g_gprs_t.state == GPRS_RXCONTENT)
		{
			SG_Send_Net_State(1);
		}
		else
		{
			SG_Send_Net_State(0);
		}
		//SG_CreateSignal(SG_SEND_NETSTAT,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
	}
#endif /* (0) */

}

int  SG_Net_Judge(void)
{
	char ret  = NETWORK_OK;

	if(g_state_info.SimFlag == 0 && (g_state_info.plmn_status == PLMN_NO_SERVICE || g_state_info.plmn_status == PLMN_EMERGENCY_ONLY) )
	{
		if(g_gprs_t.state != GPRS_SOCKET_CONNECT && g_gprs_t.state != GPRS_PORXY && g_gprs_t.state != GPRS_HANDUP
			&& g_gprs_t.state != GPRS_RXHEAD && g_gprs_t.state != GPRS_RXCONTENT && g_gprs_t.state != GPRS_EWOULDBLOCK)
		{
			ret =  NO_SIM_CARD;
		}
	}
	else if(PLMN_NO_SERVICE == g_state_info.plmn_status)
	{
		if(g_gprs_t.state != GPRS_SOCKET_CONNECT && g_gprs_t.state != GPRS_PORXY && g_gprs_t.state != GPRS_HANDUP
			&& g_gprs_t.state != GPRS_RXHEAD && g_gprs_t.state != GPRS_RXCONTENT && g_gprs_t.state != GPRS_EWOULDBLOCK)
		{
			ret = NO_NET_WORK;
		}
	}
	else if(PLMN_NORMAL_GSM_ONLY == g_state_info.plmn_status)
	{
		ret = GSM_ONLY;

		if(g_state_info.SimFlag != 1)
		{
			g_state_info.SimFlag = 1;
		}
	}
	else if(PLMN_NORMAL_GPRS_ONLY == g_state_info.plmn_status)
	{
		ret = GPRS_ONLY;

		if(g_state_info.SimFlag != 1)
		{
			g_state_info.SimFlag = 1;
		}
	}
	else if(PLMN_NORMAL_GSM_GPRS_BOTH == g_state_info.plmn_status)
	{
		ret = NETWORK_OK;

		if(g_state_info.SimFlag != 1)
		{
			g_state_info.SimFlag = 1;
		}
	}
	else
	{
		ret = NETWORK_OK;
	}
	SCI_TRACE_LOW("++++SG_Net_Judge: %d",ret);
	return ret;

}

void SG_Send_Heart_Beat(void)
{
	unsigned char *out = NULL;
	int outlen;

	if(g_set_info.Heart_Switch == 0)
		return;

	if(g_gprs_t.state != GPRS_RXCONTENT && g_gprs_t.state != GPRS_RXHEAD)
		return;

	if((g_gprs_t.timeout %g_set_info.Heart_Interval == 0) && (g_gprs_t.timeout != 0))
	{
//		SCI_TRACE_LOW("TIME %d  INTERVAL");
		GPRS_RxHead =1;

		if (MsgWatchHanleUp(1, &out, &outlen) != MSG_TRUE)
		{
			SCI_TRACE_LOW("--SG_Do_Handup: MsgWatchHanleUp() \r\n");
			SG_Reconnect_Socket();

		}

		if(socket_tcp_client_send((char*)out, outlen)<0)
		{

			SCI_TRACE_LOW("--SG_Do_Handup: MsgWatchHanleUp snd Err \r\n");
			g_state_info.lasterr = SG_ERR_SEND_DATA;
			SG_Reconnect_Socket();
		}
		SCI_FREE(out);
		//g_gprs_t.timeout = 1;
	}


}
