/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2007-2008 ��Ȩ����
  �ļ��� ��SG_Send.c
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2005-7-23
  ����������SG2000  ���ķ��ͺʹ���  
  �޸ļ�¼��

***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Msg.h"
#include "SG_GPS.h"
#include "SG_Send.h"
#include "SG_Set.h"
#include "os_api.h"
#include "SG_Timer.h"
#include "SG_Net.h"

short MAX_LIST_NUM = 200; //���洢��Ŀ
short MAX_ALARM_NUM = 50; 	//�����������洢��Ŀ

sg_send_list *gp_new_list = NULL; //�´������Ķ���
sg_send_list *gp_alarm_list = NULL; //��������
int gn_new_list = 0; //�´������Ķ��нڵ���Ŀ
int gn_alarm_list = 0; //�������нڵ���Ŀ

/****************************************************************
  ��������SG_Send_Free_Item_noLock
  ��  ��  ���ͷŽڵ�
  ���������pitem:�ڵ�ָ���ָ��
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static void SG_Send_Free_Item_noLock(sg_send_list ** pitem)
{

	if (pitem == NULL)
	{
		SCI_TRACE_LOW( "SG_Send_Free_Item: PARA\r\n");	
		return;
	}	
	
	if ((*pitem) && (*pitem)->msg)
	{
		SCI_FREE((*pitem)->msg);
		(*pitem)->msg = NULL;
	}
	
	if (*pitem)
	{
		SCI_FREE(*pitem);
		*pitem = NULL;
	}
}

/****************************************************************
  ��������SG_Send_Free_Item
  ��  ��  ���ͷŽڵ�
  ���������pitem:�ڵ�ָ���ָ��
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
void SG_Send_Free_Item(sg_send_list ** pitem)
{

//	SCI_TRACE_LOW( "SG_Send_Free_Item: %p %d %s",*pitem,(*pitem)->len,(*pitem)->msg);				
	if (pitem == NULL)
	{
		SCI_TRACE_LOW( "SG_Send_Free_Item: PARA\r\n");				
		return;
	}	
	
	if ((*pitem) && (*pitem)->msg)
	{
		SCI_FREE((*pitem)->msg);
		(*pitem)->msg = NULL;
	}
	
	if (*pitem)
	{
		SCI_FREE(*pitem);
		*pitem = NULL;
	}		

}


/****************************************************************
  ��������SG_Send_Clear_List
  ��  ��  ��������ж���
  ���������type: 
  					0:������Ϣ
  					1:������ͨ��Ϣ(����Կ������Ϣ����Ϣ)
  					2:������Կ������Ϣ
  					3:�����Ϣ
  					4:������Ϣ
  					5:���зǱ�����Ϣ
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ������������Ϣ���У����������Ϣ���У�  ���ɷ壬2005-12-05
  �޸��ͷŶ������ͷָ��ָ���BUG��  ���ɷ壬2005-12-06
  �޸�ifΪwhile��  ���ɷ壬2005-12-06
  ����������зǱ�����Ϣѡ����ɷ壬2005-12-31
****************************************************************/
void SG_Send_Clear_List(int type)
{
	sg_send_list *item = NULL, *itemfree = NULL;

	switch (type)
	{
	case 0:
		item = gp_new_list;

		while (item)
		{
			itemfree = item;
			item = item->next;
 			SG_Send_Free_Item_noLock(&itemfree);
		}

		gn_new_list = 0;
		gp_new_list = NULL;
		
		item = gp_alarm_list;

		while (item)
		{
			itemfree = item;
			item = item->next;
			SG_Send_Free_Item_noLock(&itemfree);
		}

		gn_alarm_list = 0;
		gp_alarm_list = NULL;	
		
		break;

	case 1:			
		item = gp_new_list;
		if(gp_new_list){
			while (item->next)
			{
				itemfree = item->next;

				if (itemfree->type != 2)
				{
					item->next = itemfree->next;
					SG_Send_Free_Item_noLock(&itemfree);
					gn_new_list --;				
				}
				else
					item = item->next;
			}
			if(gp_new_list->type != 2){
				itemfree = gp_new_list;
				gp_new_list = gp_new_list->next;
				SG_Send_Free_Item_noLock(&itemfree);
				gn_new_list --;				
			}
		}
		break;

	case 2:
		item = gp_new_list;
		if(gp_new_list){
			while (item->next)
			{
				itemfree = item->next;

				if (itemfree->type == 2)
				{
					item->next = itemfree->next;
					SG_Send_Free_Item_noLock(&itemfree);
					gn_new_list --;				
				}
				else
					item = item->next;
			}
			if(gp_new_list->type == 2){
				itemfree = gp_new_list;
				gp_new_list = gp_new_list->next;
				SG_Send_Free_Item_noLock(&itemfree);
				gn_new_list --;				
			}
		}
		break;

	case 3:
		item = gp_new_list;
		if(gp_new_list){
			while (item->next)
			{
				itemfree = item->next;
			
				if (itemfree->type == 3)
				{
					item->next = itemfree->next;
					SG_Send_Free_Item_noLock(&itemfree);
					gn_new_list --;				
				}
				else
					item = item->next;
			}
			if(gp_new_list->type == 3){
				itemfree = gp_new_list;
				gp_new_list = gp_new_list->next;
				SG_Send_Free_Item_noLock(&itemfree);
				gn_new_list --;				
			}		
		}
		break;
	case 4:
		item = gp_alarm_list;

		while (item)
		{
			itemfree = item;
			item = item->next;
			SG_Send_Free_Item_noLock(&itemfree);
		}

		gn_alarm_list = 0;
		gp_alarm_list = NULL;	
		
		break;
		
	case 5:
		item = gp_new_list;

		while (item)
		{
			itemfree = item;
			item = item->next;
 			SG_Send_Free_Item_noLock(&itemfree);
		}

		gn_new_list = 0;
		gp_new_list = NULL;
		
		break;

	default:
		break;
	}

	SCI_TRACE_LOW( "SG_Send_Clear_List: TYPE: %d, NEW LIST NO: %d, ALARM LIST NO: %d\r\n", type, gn_new_list, gn_alarm_list);			

}

 /****************************************************************
  ��������SG_Send_Put_Item
  ��  ��  ����һ���±��ķ��͵��´�������
  ���������msg, len: �±���
  				list: ����ͷָ���ָ��
  				num: ���нڵ���Ŀ��ָ��
  �����������
  ����ֵ��0:ʧ��1:�ɹ�
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  	���ϻ��ѹ��ܣ����ɷ壬2005-11-16
****************************************************************/
int SG_Send_Put_Item(int type, void *msg, int len, sg_send_list **list, int *num)
{
	sg_send_list *item = NULL, *tail = NULL, *first = NULL;
//	int count;
	int MaxNum = 0;

	if (msg == NULL || len <= 0 || list == NULL || num == NULL)
	{
		SCI_TRACE_LOW( "SG_Send_Put_Item: PARA\r\n");	
		return 0;
	}	

	//�����ڵ�
	item = SCI_ALLOC(sizeof(sg_send_list));
	if (item == NULL)
	{
		SCI_TRACE_LOW( "SG_Send_Put_Item: CALLOC: %d\r\n", sizeof(sg_send_list));	
		return 0;
	}	

	SCI_MEMSET(item,0, sizeof(sg_send_list));
	
	item->msg = msg;
	item->len = len;
	item->type = type;
//	SCI_TRACE_LOW( "SG_Send_Put_Item: %p %d %s",item,item->len,item->msg);	

	//�������������Ŀ
	//β������һ����ͷɾ��һ��
	if(type == 4)
	{
		MaxNum = MAX_ALARM_NUM;
	}
	else
	{
		MaxNum = MAX_LIST_NUM;
	}

	if (*num < MaxNum)
	{
		*num += 1;
	}
	else
	{
		first = *list;
		if(first->type >= 2){
			*list = first->next;
			tail = first;
		}
		else{
			while(first->next){
				if(first->next->type >= 2){
					tail = first->next;
					first->next = first->next->next;
					break;
				}
				first = first->next;
			}
		}
		if(tail == NULL){
			tail = *list;
			*list = (*list)->next;
		}	
		
		SG_Send_Free_Item_noLock(&tail);
		tail = NULL;
	}

	//�����Ϣ��ӵ�β��
	if (item->type >= 2)
	{
		tail = *list;
		
		while (tail && tail->next)
		{
			tail = tail->next;
		}
		
		if (tail)
			tail->next = item;
		else
			*list = item;
	}
	else //���ú���Կ��Ϣ��ӵ�ͷ��
	{
		item->next = *list;
		*list = item;
	}

	return 1;
}
static sg_send_list *SG_Send_Get_Item_type(sg_send_list **list, int *num,int type)
{
	sg_send_list *item = NULL,*ret = NULL;
	

	//��ͷ����ȡ
	if (list && (*list))
	{
		item = *list;
		if(item->type == type){
			ret = item;
			*list = (*list)->next;
			*num -= 1;
			return ret;
		}
		while(item->next){
			if(item->next->type == type){
				ret = item->next;
				item->next = ret->next;
				*num -= 1;
				return ret;
			}
			item = item->next;
		}
	}
	return ret;
}
/****************************************************************
  ��������SG_Send_Get_Item
  ��  ��  ���Ӵ������л�ȡһ�������͵Ľڵ�
  ���������list: ����ͷָ���ָ��
  				num: ���нڵ���Ŀ��ָ��
  ��������������͵Ľڵ�
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
 	 �����޷��͹���� �������ɷ壬2005-11-8
****************************************************************/
sg_send_list *SG_Send_Get_Item(sg_send_list **list, int *num)
{
	sg_send_list *item = NULL;


	//��ͷ����ȡ
	if (list && (*list))
	{
		item = *list;
		*list = (*list)->next;
		*num -= 1;
	}

/*
	if (item == NULL)
	{
		if (gn_alarm_list == 0 && gn_new_list == 0)
		{
			//�������С��¶�������Ϣ�����Թ���ϵͳ���ȴ�
			//�źſ���һ�β�������Ϊ���˺ܶ�Σ���ѯ����
			//������߼�������
		//	SCI_TRACE_LOW( "SG_Send_Get_New_Item: NO MSG, WAIT!!!\r\n");	

		}
	}
*/	
	return item;
}

 /****************************************************************
  ��������SG_Send_Put_New_Item
  ��  ��  ����һ���±��ķ��͵��´�������
  ����������±���
  �����������
  ����ֵ��0:ʧ��1:�ɹ�
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
int SG_Send_Put_New_Item(int type, void *msg, int len)
{

	xSignalHeaderRec      *signal_ptr = NULL;
	
	if (SG_Send_Put_Item(type, msg, len, &gp_new_list, &gn_new_list) == 1)
	{		
		SCI_TRACE_LOW( "SG_Send_Put_New_Item: MSG LEN: %d, NEW LIST NO: %d", len, gn_new_list);
		if(bhangup == 1 && g_set_info.nNetType > MSG_NET_SMS)
			SG_CreateSignal(SG_SEND_MSG,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
		return 1;
	}
	else
	{		
		if(msg)
			SCI_FREE(msg);
		SCI_TRACE_LOW( "SG_Send_Put_New_Item: MSG LEN: %d, NEW LIST NO: %d", len, gn_new_list);
		return 0;
	}
}
sg_send_list *SG_Send_Get_New_Item_type(){
	sg_send_list *item = NULL;

	item = SG_Send_Get_Item_type(&gp_new_list, &gn_new_list,1);
	
	if (item)
	{
		SCI_TRACE_LOW( "SG_Send_Get_New_Item_type: MSG LEN: %d, NEW LIST NO: %d TYPE %d\r\n", item->len, gn_new_list,item->type);
	}
	return item;
}

/****************************************************************
  ��������SG_Send_Get_New_Item
  ��  ��  �����´������л�ȡһ�������͵Ľڵ�
  �����������
  ��������������͵Ľڵ�
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
    	�������߹��ܣ����ɷ壬2005-11-16
****************************************************************/
sg_send_list *SG_Send_Get_New_Item()
{
	sg_send_list *item = NULL;

	item = SG_Send_Get_Item(&gp_new_list, &gn_new_list);
	
	if (item)
	{
		SCI_TRACE_LOW( "SG_Send_Get_New_Item: MSG LEN: %d, NEW LIST NO: %d\r\n", item->len, gn_new_list);
	}
#if 0	
	else
	{
		if(pSg2k->firstTime > 1103760000)
		{
			time_t curr;
			time_t interval;

			time(&curr);

			if(curr > 1103760000)
			{
				interval = curr - pSg2k->firstTime;

				if(interval > 2*24*3600)
				{
					if(interval < 3*24*3600)
					{
						if((curr%(24*3600)) < 3600*5)
						{
							SCI_TRACE_LOW("curr %s, first %s, reset!! \n",ctime(&curr),ctime(&pSg2k->firstTime));
							SG_Soft_Reset(0);
						}

					}
					else
					{
						SCI_TRACE_LOW("curr %s, first %s, reset!!! \n",ctime(&curr),ctime(&pSg2k->firstTime));
						SG_Soft_Reset(0);
					}
				}
			}
		}
	}
#endif
	return item;
}

 /****************************************************************
  ��������SG_Send_Put_Alarm_Item
  ��  ��  ����һ���������ķ��͵���������
  �����������������
  �����������
  ����ֵ��0:ʧ��1:�ɹ�
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
int SG_Send_Put_Alarm_Item(void *msg, int len)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	
	if (SG_Send_Put_Item(4, msg, len, &gp_alarm_list, &gn_alarm_list) == 1)
	{		
		SCI_TRACE_LOW( "SG_Send_Put_Alarm_Item: MSG LEN: %d, ALARM LIST NO: %d \r\n", len, gn_alarm_list);
		if(g_set_info.nNetType > MSG_NET_SMS && bhangup ==1)
			SG_CreateSignal(SG_SEND_MSG,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
		
		return 1;
	}
	else
	{		
		if(msg)
			SCI_FREE(msg);
		
		SCI_TRACE_LOW( "SG_Send_Put_Alarm_Item: MSG LEN: %d, ALARM LIST NO: %d\r\n", len, gn_alarm_list);
		return 0;
	}
}
 
/****************************************************************
  ��������SG_Send_Get_Alarm_Item
  ��  ��  ���ӱ������л�ȡһ�������͵Ľڵ�
  �����������
  ��������������͵Ľڵ�
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
sg_send_list *SG_Send_Get_Alarm_Item()
{
	sg_send_list *item = NULL;

	item = SG_Send_Get_Item(&gp_alarm_list, &gn_alarm_list);
	
	if (item)
		SCI_TRACE_LOW( "SG_Send_Get_Alarm_Item: MSG LEN: %d, ALARM LIST NO: %d\r\n", item->len, gn_alarm_list);

	return item;	
}


// ���������ͼƬ������(���ȴ���1k����Ϊ��ͼƬ)
int SG_Send_Cal_Pic_Item(void)
{
	sg_send_list *item = NULL;
	int Total = 0;
	
	item = gp_new_list;

	while(item)
	{
		if(item->len > 1000)
			Total ++;
		item = item->next;
	}

	SCI_TRACE_LOW("<<<<<< %d Items in List!!!",Total);
	return Total;

}



