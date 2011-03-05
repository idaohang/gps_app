/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2007-2008 版权所有
  文件名 ：SG_Send.c
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2005-7-23
  内容描述：SG2000  报文发送和处理  
  修改记录：

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

short MAX_LIST_NUM = 200; //最大存储数目
short MAX_ALARM_NUM = 50; 	//报警队列最大存储数目

sg_send_list *gp_new_list = NULL; //新待发报文队列
sg_send_list *gp_alarm_list = NULL; //报警队列
int gn_new_list = 0; //新待发报文队列节点数目
int gn_alarm_list = 0; //报警队列节点数目

/****************************************************************
  函数名：SG_Send_Free_Item_noLock
  功  能  ：释放节点
  输入参数：pitem:节点指针的指针
  输出参数：无
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
  函数名：SG_Send_Free_Item
  功  能  ：释放节点
  输入参数：pitem:节点指针的指针
  输出参数：无
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
  函数名：SG_Send_Clear_List
  功  能  ：清空所有队列
  输入参数：type: 
  					0:所有信息
  					1:所有普通信息(除密钥请求信息外信息)
  					2:所有密钥请求信息
  					3:监控信息
  					4:报警信息
  					5:所有非报警信息
  输出参数：无
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  增加清除监控信息队列，清除报警信息队列，  李松峰，2005-12-05
  修改释放队列项后头指针指向的BUG，  李松峰，2005-12-06
  修改if为while，  李松峰，2005-12-06
  增加清除所有非报警信息选项，李松峰，2005-12-31
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
  函数名：SG_Send_Put_Item
  功  能  ：将一个新报文发送到新待发队列
  输入参数：msg, len: 新报文
  				list: 队列头指针的指针
  				num: 队列节点数目的指针
  输出参数：无
  返回值：0:失败1:成功
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  	加上唤醒功能，李松峰，2005-11-16
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

	//创建节点
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

	//超过缓冲最大数目
	//尾部增加一个开头删除一个
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

	//监控信息添加到尾部
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
	else //设置和密钥信息添加到头部
	{
		item->next = *list;
		*list = item;
	}

	return 1;
}
static sg_send_list *SG_Send_Get_Item_type(sg_send_list **list, int *num,int type)
{
	sg_send_list *item = NULL,*ret = NULL;
	

	//从头部获取
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
  函数名：SG_Send_Get_Item
  功  能  ：从待发队列获取一个待发送的节点
  输入参数：list: 队列头指针的指针
  				num: 队列节点数目的指针
  输出参数：待发送的节点
  编写者  ：李松峰
  修改记录：创建，2005-9-20
 	 增加无发送挂起的 处理，李松峰，2005-11-8
****************************************************************/
sg_send_list *SG_Send_Get_Item(sg_send_list **list, int *num)
{
	sg_send_list *item = NULL;


	//从头部获取
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
			//报警队列、新队列无消息，尝试挂起系统，等待
			//信号可能一次不挂起，因为送了很多次，轮询几次
			//后挂起，逻辑无问题
		//	SCI_TRACE_LOW( "SG_Send_Get_New_Item: NO MSG, WAIT!!!\r\n");	

		}
	}
*/	
	return item;
}

 /****************************************************************
  函数名：SG_Send_Put_New_Item
  功  能  ：将一个新报文发送到新待发队列
  输入参数：新报文
  输出参数：无
  返回值：0:失败1:成功
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
int SG_Send_Put_New_Item(int type, void *msg, int len)
{

	xSignalHeaderRec      *signal_ptr = NULL;
	
	if (SG_Send_Put_Item(type, msg, len, &gp_new_list, &gn_new_list) == 1)
	{		
		SCI_TRACE_LOW( "SG_Send_Put_New_Item: MSG LEN: %d, NEW LIST NO: %d", len, gn_new_list);
		if(bhangup == 1 && g_set_info.nNetType > MSG_NET_SMS)
			SG_CreateSignal(SG_SEND_MSG,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文
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
  函数名：SG_Send_Get_New_Item
  功  能  ：从新待发队列获取一个待发送的节点
  输入参数：无
  输出参数：待发送的节点
  编写者  ：李松峰
  修改记录：创建，2005-9-20
    	加上休眠功能，李松峰，2005-11-16
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
  函数名：SG_Send_Put_Alarm_Item
  功  能  ：将一个报警报文发送到报警队列
  输入参数：报警报文
  输出参数：无
  返回值：0:失败1:成功
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
int SG_Send_Put_Alarm_Item(void *msg, int len)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	
	if (SG_Send_Put_Item(4, msg, len, &gp_alarm_list, &gn_alarm_list) == 1)
	{		
		SCI_TRACE_LOW( "SG_Send_Put_Alarm_Item: MSG LEN: %d, ALARM LIST NO: %d \r\n", len, gn_alarm_list);
		if(g_set_info.nNetType > MSG_NET_SMS && bhangup ==1)
			SG_CreateSignal(SG_SEND_MSG,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文
		
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
  函数名：SG_Send_Get_Alarm_Item
  功  能  ：从报警队列获取一个待发送的节点
  输入参数：无
  输出参数：待发送的节点
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
sg_send_list *SG_Send_Get_Alarm_Item()
{
	sg_send_list *item = NULL;

	item = SG_Send_Get_Item(&gp_alarm_list, &gn_alarm_list);
	
	if (item)
		SCI_TRACE_LOW( "SG_Send_Get_Alarm_Item: MSG LEN: %d, ALARM LIST NO: %d\r\n", item->len, gn_alarm_list);

	return item;	
}


// 计算队列中图片的数量(长度大于1k则认为是图片)
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



