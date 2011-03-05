/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��mmigprs.h
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-7-23
  ����������SG2000 ���ձ��ĺʹ���  
  �޸ļ�¼��
***************************************************************/
#ifndef _MMIGPRS_H_
#define _MMIGPRS_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
*******************************�궨��********************************
********************************************************************/
#define SMS_TO_GPRS 		0x01	//����ģʽ�л���GPRS
#define GPRS_TO_SMS 		0x02	//GPRSģʽ�л�������

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



// SG_Net_Judge��������ֵ
#define NO_SIM_CARD 		0x00
#define NO_NET_WORK 		0x01
#define GSM_ONLY 			0x02
#define GPRS_ONLY 			0x03
#define NETWORK_OK 			0x04

/********************************************************************
*******************************�ṹ�嶨��****************************
********************************************************************/

typedef struct
{
	BOOLEAN is_all;
	uint32 pdp_id[11];
}MMI_PDP_ACT_REQ_T;

extern int bhangup; 		// 0 û�н������� -1 �ȴ�����Ӧ�� 1 ���ֳɹ� 


/********************************************************************
*******************************��������****************************
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
void SG_Do_GprsLink(void); // GPRS��ʱ����
void SG_Reconnect_Socket(void);
void SG_Send_Item_Timeout_Handle(void);
void SG_Close_an_Udp(void);


#endif //_MMIGPRS_H_

