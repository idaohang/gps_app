/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2008 ��Ȩ����
  �ļ��� ��SG_Hh.h
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-7-23
  ����������SG2000  ���ķ��ͺʹ���  
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_HH_H_
#define _SG_HH_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "sci_types.h"



/*�����յ�����Ϣ*/
#define REV_OK       		0x00	//	��Ӧ֡
#define SEND_BROWSER 		0x01	//	��̬�˵�
#define ASK_NAME_SINGAL 	0x02	// 	���󷵻���Ӫ�����ơ�gsm�ź�ǿ��
#define ASK_VAL 			0x03	// 	���󷵻���Ͳ����
#define SEND_CALL			0x04	// 	���� 
#define SEND_REV 			0x05	// 	���� �Ҷ� 
#define SEND_DTMF			0x06	//	����DTMF��
#define SEND_VAL			0x07	//	��������
#define SEND_SMS			0x08	//	������Ϣ
#define COPY_TEL_BOOK		0x09	// 	������Ҫ���Ƶĵ绰����
#define QUERY_TEL_BOOK		0x0a	// 	�����ѯsim����������
#define GET_TEL_BOOK		0x0b	//	�����ȡsim��n������
#define SEND_INSCH			0x0c	// 	�����ֱ��ϵ籨�������汾��
#define SEND_INIT			0x0d	// 	���ͳ�ʼ������
#define SEND_RESET			0x0e	// 	������������
#define SEND_MT				0x0f	// 	�ֱ�/����ѡ��
#define SEND_GOOD			0x10	// 	�����ֱ�������������
#define PASS_KEY_MIANTI     0x11
#define SG2000_HH_EXIT      0x12	//	�ֱ��˳�
#define SG2000_MB_TEST      0x13
#define SG2000_MB_CONFIG    0x14	
#define MUSTER_OK			0x15
#define MUSTER_CANCEL		0x16
#define SEND_HANDFREE		0x10	// 	�ֱ�/����ѡ��
// �����������ͨ�ţ�������Ϣ

#define RTN_NAME_SINGAL 	0x02
#define RTN_VAL	  			0x03
#define REV_TEL				0x04
#define	REV_TELSTA			0x05 // ���ֱ�����ͨ��״̬  0x00 ���ж�ʧ 0x01 ����0x02 ͨ��
#define SMS_RESULT			0x06
#define	REV_SMS				0x07
#define	REV_TEL_BOOK 		0x08
#define	NUM_TEL_BOOK		0x09
#define SET_TEL_BOOK 		0x0a
#define	RTN_TIME  			0x0b // ���ֱ�����ʱ�������
#define RTN_ERR  			0x0c
#define RTN_NWRPT			0x0e
#define RTN_AUTO_REV 	 	0x11

//��PC��ͨ��
#define SG_SHOW_DEVICE      0x21    //���ò�ѯ
#define SG_SET_DEVICE       0x22    //��������
#define SG_SHOW_VER         0x23    //�汾��ѯ
#define SG_SHOW_STATE       0x24    //����״̬
#define SG_SHOW_IO          0x25    //IO����״̬
#define SG_XIH_OFF          0x26    //Ϩ��
#define SG_XIH_ON           0x27    //Ϩ��ָ�
#define SG_LISTEN_NOM       0x28    //����
#define SG_SHOW_NET         0x29    //����״̬

#define SG_PC_FLAG          0x31    //��������
#define SG_PC_XHON          0x32    //Ϩ��
#define SG_PC_XHOFF         0x33    //Ϩ��ָ�
#define SG_PC_IO            0x34    //IO���
#define SG_PC_VER           0x35    //�汾��ѯ
#define SG_PC_CALL          0x36    //����绰
#define SG_PC_STATE         0x37    //����״̬


// 0x00 ��ʧ  0x02 ������ 0x03 ͨ��
#define CAll_OFF 	0x00 // ���ж�ʧ
#define CALL_OUT  	0x02 // ��������绰
#define CALL_SPEAK	0x03 // ͨ��



#define ERR_NO_SIM_CARD 0x01
#define ERR_NO_NETWORK  0x02
#define ERR_PIN_CODE    0x03
// ��������
extern uint8 setmenu_status;

//�ṹ������
#define MAX_FRAME_BUF 580
typedef struct
{
	unsigned char type;      //����
	unsigned short len;     //����
	unsigned char dat[MAX_FRAME_BUF]; //����
} SENDFRAME;  

//�ṹ������
typedef struct _SG_HH_LIST_
{
	char *buf;
	int len;
	int com; // ���ں�
	struct _SG_HH_LIST_ *next;
}sg_hh_list;


// External Variables 
extern BOOLEAN g_framesendok;
extern unsigned char g_sendframe1s;
extern SENDFRAME g_sendframe;
extern int checkhandset;
// Internal Prototypes



// SMS ��ض���
// �û���Ϣ���뷽ʽ
#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS2		8
typedef struct

{

	unsigned char pSCA[20];			// ����Ϣ�������ĺ���(SMSC��ַ)

	unsigned char  pTPA[20];			// Ŀ������ظ�����(TP-DA��TP-RA)

	unsigned  char TP_DCS;			// �û���Ϣ���뷽ʽ(TP-DCS)

	unsigned char pTP_SCTS[20];		// ����ʱ����ַ���(TP_SCTS), ����ʱ�õ�

	unsigned char  pTP_UD[255];		// ԭʼ�û���Ϣ(����ǰ�������TP-UD)

	int pDU_Len;                    // pdu�ĳ���

} PDUSTRUCT;




// External Prototypes

// ��������
int  CheckSum( unsigned char*pdatas,int len) ;
void SG_Send_Net_State(char type);
void SG_HH_SEND_ITEM(int SignalCode);
int SG_HH_Put_New_Item(int type, int com, unsigned char *buf, int len);
int SG_HH_Put_Item(int type, int com, char *msg, int len, sg_hh_list **list, int *num);
sg_hh_list *SG_HH_Get_Item(sg_hh_list **list, int *num);
void DoUart(void);
void SG_Uart_Send_Msg(unsigned char*msg,short len,uint8 type);


#ifdef __cplusplus
}
#endif

#endif //_SG_HH_H_


