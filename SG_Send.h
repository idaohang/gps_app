/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_Send.h
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-7-23
  内容描述：SG2000  报文发送和处理  
  修改记录：
***************************************************************/
#ifndef _SG_SEND_H_
#define _SG_SEND_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "sci_types.h"

//结构体声明
typedef struct _SG_SEND_LIST_
{
	void *msg;
	int len;
	int type; // 1:设置信息2:密钥交换信息3:监控信息4 报警报文
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


