/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Send.h
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-7-23
  ����������SG2000  ���ķ��ͺʹ���  
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_SEND_H_
#define _SG_SEND_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "sci_types.h"

//�ṹ������
typedef struct _SG_SEND_LIST_
{
	void *msg;
	int len;
	int type; // 1:������Ϣ2:��Կ������Ϣ3:�����Ϣ4 ��������
	struct _SG_SEND_LIST_ *next;
}sg_send_list;

// External Variables 


// Internal Prototypes
void SG_Send_Clear_List(int type);
int SG_Send_Put_New_Item(int type, void *msg, int len);
int SG_Send_Put_Alarm_Item(void *msg, int len);
sg_send_list *SG_Send_Get_Alarm_Item(void);
sg_send_list *SG_Send_Get_New_Item(void);
void SG_Send_Free_Item(sg_send_list ** pitem);
sg_send_list *SG_Send_Get_New_Item_type(void);
sg_send_list *SG_Send_Get_Item(sg_send_list **list, int *num);
int SG_Send_Put_Item(int type, void *msg, int len, sg_send_list **list, int *num);
int SG_Send_Cal_Pic_Item(void);


// External Prototypes


#ifdef __cplusplus
}
#endif

#endif //_SG_SEND_H_


