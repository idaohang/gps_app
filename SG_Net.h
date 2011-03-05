/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：mmigprs.h
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-7-23
  内容描述：SG2000 接收报文和处理  
  修改记录：
***************************************************************/
#ifndef _MMIGPRS_H_
#define _MMIGPRS_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
*******************************宏定义********************************
********************************************************************/
#define SMS_TO_GPRS 		0x01	//短信模式切换到GPRS
#define GPRS_TO_SMS 		0x02	//GPRS模式切换到短信

#define	ACTIVE_OVERTIME_1	60		
#define	ACTIVE_OVERTIME_2	140		
#define	PORXY_OVERTIME		120		
#define	HANDUP_OVERTIME		40	
#define	DEACTIVE_OVERTIME	60		
#define	RCVDATA_OVERTIME	300		
#define	RCVDATA_OVERTIME_CWT	60		
#define	SOCKCON_OVERTIME	50		
#define GPRS_RCVANS_TIMEOUT 30


#define DEFAULT_KEEPLINE_TIME 20



// SG_Net_Judge函数返回值
#define NO_SIM_CARD 		0x00
#define NO_NET_WORK 		0x01
#define GSM_ONLY 			0x02
#define GPRS_ONLY 			0x03
#define NETWORK_OK 			0x04

/********************************************************************
*******************************结构体定义****************************
********************************************************************/

typedef struct
{
	BOOLEAN is_all;
	uint32 pdp_id[11];
}MMI_PDP_ACT_REQ_T;

extern int bhangup; 		// 0 没有进行握手 -1 等待握手应答 1 握手成功 


/********************************************************************
*******************************函数声明****************************
********************************************************************/
BOOLEAN SG_GPRS_DeactivePdpContext(void);
BOOLEAN SG_GPRS_SetAndActivePdpContex(void);
void SG_GPRS_GetData(             
				const uint32 so, // socket
            	int32 fin        // socket fin flag, 1 - socket closed	
            	);
void MnGPRS_EventCallback(
                                uint32 task_id, //task ID
                                uint32 event_id, //event ID
                                void *param_ptr
                                );
void SG_Net_Disconnect(void);
int SG_Net_Ack(void);
int socket_tcp_client_send(char * pchar, int length);
void SG_GPRS_SEND_ITEM(int SignalCode);
void SG_SMS_SEND_ITEM(void);
BOOLEAN SG_Check_Socket_State(void);
void SG_Do_Handup(void);
void SG_Do_GprsLink(void); // GPRS超时处理
void SG_Reconnect_Socket(void);
void SG_Send_Item_Timeout_Handle(void);
void SG_Close_an_Udp(void);


#endif //_MMIGPRS_H_

