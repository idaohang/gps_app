/**************************************************************** 版权 
：福建星网锐捷通讯有限公司 2002-2005 版权所有 文件名 ：SG_DynamicMenu.c 版本 
：1.00 创建者 ：李松峰 创建时间：2005-9-15 
内容描述：SG2000主机和手柄之间动态菜单协议        修改记录： 
***************************************************************/



#include <string.h>

#include "SG_DynamicMenu.h"
#include "SG_GPS.h"
#include "SG_Set.h"
#include "SG_Menu.h"
#include "SG_MsgHandle.h"
#include "SG_Send.h"
#include "sci_types.h"
#include "SG_Hh.h"


extern int gMusterTel;
extern unsigned char gMuster_gbk[SG_MUSTER_LEN+1];
extern unsigned char gMuster_phone[SG_PHONE_LEN+1];
extern int gMusterCnt;

/****************************************************************
  函数名：SG_DynMenu_Put_MenuList
  功  能  ：发送动态菜单列表
  输入参数：
  				list:菜单列表
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  			增加菜单总数参数项，2005-10-24，李松峰
****************************************************************/
int SG_DynMenu_Put_MenuList (MENULIST* list)
{
 	int len = 0, num = 0;
	char *menulist = NULL;
	int p=0;
	MENULIST *tmp = NULL, *head = NULL;

	if (list == NULL)
	{
		len += 1 + 1 + MENUID_LEN*num + 1*num;
		menulist = SCI_ALLOC(len+1);
		SCI_MEMSET(menulist,0,len+1);
		if (menulist == NULL)
		{
			SCI_TRACE_LOW("SG_DynMenu_Put_MenuList: CALLOC: %d\r\n", len+1);	
			return 0;
		}	

		//类型
		menulist[p] = TYPE_MENU_LIST;
		p += 1;

		//菜单总数
		menulist[p] = num;
		p += 1;
	}
	else
	{
		//找到同级第一个菜单
		head = list;
		while(head->pre)
			head = head->pre;

		//菜单总数和数据长度	
		tmp = head;
		do
		{
			len += strlen(tmp->item->s);
			num ++;
			tmp = tmp->next;
		}
		while (tmp);
		
		len += 1 + 1 + MENUID_LEN*num + 1*num;
		menulist = SCI_ALLOC(len+1);
		SCI_MEMSET(menulist,0,len+1);
		if (menulist == NULL)
		{
			SCI_TRACE_LOW("SG_DynMenu_Put_MenuList: CALLOC: %d\r\n", len+1);	
			return 0;
		}	
		
		//类型设计技
		p = 0;
		menulist[p] = TYPE_MENU_LIST;
		p += 1;
	
		//菜单总数{}
		menulist[p] = num;
		p += 1;

		//当前菜单作为菜单列表第一个
		tmp = list;
		while (tmp)
		{
			if (tmp->item != NULL)
			{
				SCI_MEMCPY(&menulist[p], tmp->item->menuid, MENUID_LEN);
				p += MENUID_LEN;
				strcpy(&menulist[p], tmp->item->s);
				p += strlen(tmp->item->s);
				menulist[p] = SEPARATOR;
				p += 1;
			}
			tmp = tmp->next;
		}

		//添加当前菜单之前的菜单
		tmp = head;
		while (tmp)
		{
			//当前菜单之后已经添加
			if (tmp == list)
				break;
			
			if (tmp->item != NULL)
			{
				SCI_MEMCPY(&menulist[p], tmp->item->menuid, MENUID_LEN);
				p += MENUID_LEN;
				strcpy(&menulist[p], tmp->item->s);
				p += strlen(tmp->item->s);
				menulist[p] = SEPARATOR;
				p += 1;
			}
			tmp = tmp->next;
		}
	}

	SCI_TRACE_LOW( "SG_DynMenu_Put_MenuList: LEN: %d\r\n", len);

	//发送	
	if (SG_Net_DynamicMenu_Snd_Msg((unsigned char*)menulist, len) != 1)
	{
		SCI_FREE(menulist);
		return 0;
	}
	else
	{
		SCI_FREE(menulist);
		return 1;
	}
}

/****************************************************************
  函数名：SG_DynMenu_Put_InputBox
  功  能  ：发送输入框菜单项
  输入参数：
  				menuid:菜单ID(可以是调用的父菜单的ID)
  				title:提示说明头
  				tail:提示说明尾（如时间单位等）
  				password:明文/密文标志
  				maxLen:输入最大长度
  				charType:数字/字符选择
  				upLow:大小写
  				card:刷卡标志
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
int SG_DynMenu_Put_InputBox (unsigned char menuid[MENUID_LEN], char *title, char *tail, char password, short maxLen, char charType, char upLow, char card)
{
 	int len = 0;
	char *menuItem = NULL;
	int p;

	if (title == NULL || maxLen <= 0)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_InputBox: PARA\r\n");	
		return 0;
	}	

	len = 1 + 1 + MENUID_LEN + 1 + 2 + 1 + 1 + 1 + INPUT_BOX_TITLE_LEN + 1 + INPUT_BOX_TAIL_LEN + 1; 
	menuItem = SCI_ALLOC( len+1);
	SCI_MEMSET(menuItem,0,len+1);
	if (menuItem == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_InputBox: CALLOC: %d\r\n", len);	
		return 0;
	}	

	p = 0;
	menuItem[p] = TYPE_MENU_ITEM;
	p += 1;
	menuItem[p] = SIGN_INPUT_BOX;
	p += 1;
	SCI_MEMCPY(&menuItem[p], menuid, MENUID_LEN);
	p += MENUID_LEN;
	menuItem[p] = password;
	p += 1;
	//BIG-ENDIAN存储方式
	memcpy(&menuItem[p],(char*)&maxLen,2);
	p += 2;
	menuItem[p] = charType;
	p += 1;
	menuItem[p] = upLow;
	p += 1;
	menuItem[p] = card;
	p += 1;
	strncpy(&menuItem[p], title, INPUT_BOX_TITLE_LEN);
	p += INPUT_BOX_TITLE_LEN;
	menuItem[p] = SEPARATOR;
	p += 1;
	if (tail != NULL)
		strncpy(&menuItem[p], tail, INPUT_BOX_TAIL_LEN);
	p += INPUT_BOX_TAIL_LEN;
	menuItem[p] = END;
	p += 1;

	SCI_TRACE_LOW( "SG_DynMenu_Put_InputBox: LEN: %d\r\n", len);
	
	//发送	
	if (SG_Net_DynamicMenu_Snd_Msg((unsigned char*)menuItem, len) != 1)
	{
		SCI_FREE(menuItem);
		return 0;
	}
	else
	{
		SCI_FREE(menuItem);
		return 1;
	}
}

/****************************************************************
  函数名：SG_DynMenu_Put_ChooseBox
  功  能  ：发送选择框菜单项
  输入参数：
  				menuid:菜单ID(可以是调用的父菜单的ID)
  				title:提示说明
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
int SG_DynMenu_Put_ChooseBox (unsigned char menuid[MENUID_LEN], char *title)
{
 	int len = 0;
	char *menuItem = NULL;
	int p;

	if (title == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_ChooseBox: PARA\r\n");	
		return 0;
	}	

	len = 1 + 1 + MENUID_LEN + strlen(title) + 1; 
	menuItem = SCI_ALLOC( len+1);
	SCI_MEMSET(menuItem,0,len+1);
	if (menuItem == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_ChooseBox: CALLOC: %d\r\n", len);	
		return 0;
	}	

	p = 0;
	menuItem[p] = TYPE_MENU_ITEM;
	p += 1;
	menuItem[p] = SIGN_CHOOSE_BOX;
	p += 1;
	SCI_MEMCPY(&menuItem[p], menuid, MENUID_LEN);
	p += MENUID_LEN;
	strcpy(&menuItem[p], title);
	p += strlen(title);
	menuItem[p] = END;
	p += 1;

	SCI_TRACE_LOW( "SG_DynMenu_Put_ChooseBox: LEN: %d \r\n", len);
	
	//发送	
	if (SG_Net_DynamicMenu_Snd_Msg((unsigned char*)menuItem, len) != 1)
	{
		SCI_FREE(menuItem);
		return 0;
	}
	else
	{
		SCI_FREE(menuItem);
		return 1;
	}
}

/****************************************************************
  函数名：SG_DynMenu_Put_MsgBox
  功  能  ：发送消息框
  输入参数：
  				menuid:菜单ID(可以是调用的父菜单的ID)
  				title:提示说明
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
int SG_DynMenu_Put_MsgBox (unsigned char menuid[MENUID_LEN], char *title)
{
 	int len = 0;
	char *menuItem = NULL;
	int p;

	if (title == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_MsgBox: PARA\r\n");	
		return 0;
	}	

	len = 1 + MENUID_LEN + strlen(title) + 1; 
	menuItem = SCI_ALLOC( len+1);
	SCI_MEMSET(menuItem,0,len+1);
	if (menuItem == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_MsgBox: CALLOC: %d\r\n", len);	
		return 0;
	}	

	p = 0;
	menuItem[p] = TYPE_MSG_BOX;
	p += 1;
	SCI_MEMCPY(&menuItem[p], menuid, MENUID_LEN);
	p += MENUID_LEN;
	strcpy(&menuItem[p], title);
	p += strlen(title);
	menuItem[p] = END;
	p += 1;

	SCI_TRACE_LOW( "SG_DynMenu_Put_MsgBox: LEN: %d \r\n", len);
	
	//发送	
	if (SG_Net_DynamicMenu_Snd_Msg((unsigned char*)menuItem, len) != 1)
	{
		SCI_FREE(menuItem);
		return 0;
	}
	else
	{
		SCI_FREE(menuItem);
		return 1;
	}
}

/****************************************************************
  函数名：SG_DynMenu_Put_VoiceBox
  功  能  ：发送语音呼叫框
  输入参数：
  				menuid:菜单ID(可以是调用的父菜单的ID)
  				title:提示说明
  				phoneNum:电话号码
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  		增加所拨号码名称，2005-10-24，李松峰
****************************************************************/
int SG_DynMenu_Put_VoiceBox (unsigned char menuid[MENUID_LEN], char *title, char *phoneNum, char *phoneName)
{
 	int len = 0;
	char *menuItem = NULL;
	int p;

	if (title == NULL || phoneNum == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_VoiceBox: PARA\r\n");	
		return 0;
	}	

	len = 1 + MENUID_LEN + strlen(title) + 1 + strlen(phoneNum) + 1 + strlen(phoneName) + 1; 
	menuItem = SCI_ALLOC( len+1);
	SCI_MEMSET(menuItem,0,len+1);
	if (menuItem == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_VoiceBox: CALLOC: %d\r\n", len);	
		return 0;
	}	

	p = 0;
	menuItem[p] = TYPE_VOICE_BOX;
	p += 1;
	SCI_MEMCPY(&menuItem[p], menuid, MENUID_LEN);
	p += MENUID_LEN;
	strcpy(&menuItem[p], title);
	p += strlen(title);
	menuItem[p] = SEPARATOR;
	p += 1;
	strcpy(&menuItem[p], phoneNum);
	p += strlen(phoneNum);
	menuItem[p] = SEPARATOR;
	p += 1;
	strcpy(&menuItem[p], phoneName);
	p += strlen(phoneName);
	menuItem[p] = END;
	p += 1;

	SCI_TRACE_LOW( "SG_DynMenu_Put_VoiceBox: LEN: %d \r\n", len);
	
	//发送	
	if (SG_Net_DynamicMenu_Snd_Msg((unsigned char*)menuItem, len) != 1)
	{
		SCI_FREE(menuItem);
		return 0;
	}
	else
	{
		SCI_FREE(menuItem);
		return 1;
	}
}

/****************************************************************
  函数名：SG_DynMenu_Put_AutoShowBox
  功  能  ：发送自动滚动显示消息框
  输入参数：
  				menuid:菜单ID(可以是调用的父菜单的ID)
  				title:提示说明
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
int SG_DynMenu_Put_AutoShowBox (unsigned char menuid[MENUID_LEN], char *title)
{
 	int len = 0;
	char *menuItem = NULL;
	int p;
	char  *test=NULL;
	short TTSVoice = 0;

	test = SCI_ALLOC(1024);
	SCI_MEMSET(test,0,1024);
	if(test==NULL)
		SCI_TRACE_LOW("SG_DynMenu_Put_AutoShowBox:calloc error\r\n");
	if (title == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_AutoShowBox: PARA\r\n");	
		return 0;
	}	

	len = 1 + MENUID_LEN + strlen(title) + 1; 
	menuItem = SCI_ALLOC( len+1);
	SCI_MEMSET(menuItem,0,len+1);
	if (menuItem == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_AutoShowBox: CALLOC: %d\r\n", len);	
		return 0;
	}	

	p = 0;
	menuItem[p] = TYPE_AUTOSHOW_BOX;
	p += 1;
	SCI_MEMCPY(&menuItem[p], menuid, MENUID_LEN);
	p += MENUID_LEN;
	strcpy(&menuItem[p], title);
	p += strlen(title);
	menuItem[p] = END;
	p += 1;
	
	SCI_TRACE_LOW( "SG_DynMenu_Put_AutoShowBox: LEN: %d \r\n", len);
	
	//发送	
#ifndef _TRACK 
	SG_Net_DynamicMenu_Snd_Msg((unsigned char*)menuItem, len);
#endif
	SCI_FREE(menuItem);




	//发送语音
	if(g_set_info.bNewUart4Alarm & (UT_TTS | UT_TTS2))
	{
		//发送语音
		SG_Voice_Snd_Msg(MSG_VOICE_SET_COMBIN, title, strlen(title));
	}

	
//	TTSVoice = 8;
	
//	sprintf(test,"[v%d]",TTSVoice);
//	strcat(test,title);
//	SG_Net_Voice_Snd_Msg(MSG_SND_VOICE,MSG_VOICE_SET_COMBIN,test,strlen(test));

	SCI_FREE(test);
	return 1;
}

/****************************************************************
  函数名：SG_DynMenu_Put_AutoShowBox_t1
  功  能  ：发送自动滚动显示消息框并自动消失
  输入参数：
  				menuid:菜单ID(可以是调用的父菜单的ID)
  				title:提示说明
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
int SG_DynMenu_Put_AutoShowBox_t1 (char tm,unsigned char menuid[MENUID_LEN])
{
 	int len = 0;
	char *menuItem = NULL;
	int p;

	len = 1 + MENUID_LEN + 1+1; 
	menuItem = SCI_ALLOC( len+1);
	SCI_MEMSET(menuItem,0,len+1);
	if (menuItem == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Put_AutoShowBox: CALLOC: %d\r\n", len);	
		return 0;
	}	

	p = 0;
	menuItem[p] = TYPE_AUTOSHOW_BOX_T1;
	p += 1;
	SCI_MEMCPY(&menuItem[p], menuid, MENUID_LEN);
	p += MENUID_LEN;
	menuItem[p] = tm;
	p++;
	menuItem[p] = END;
	p += 1;
	
	SCI_TRACE_LOW( "SG_DynMenu_Put_AutoShowBox: LEN: %d \r\n", len);
	
	//发送	
	if (SG_Net_DynamicMenu_Snd_Msg((unsigned char*)menuItem, len) != 1)
	{
		SCI_FREE(menuItem);
		return 0;
	}
	else
	{
		SCI_FREE(menuItem);
		return 1;
	}
}

//递归寻找
static void SG_SearchChild(MENULIST *list, unsigned char menuid[MENUID_LEN], MENULIST **result)
{
	if (list == NULL || result == NULL)
	{
		SCI_TRACE_LOW( "SG_SearchChild: PARA\r\n");	
		return;
	}	

	if (memcmp(list->item->menuid, menuid, MENUID_LEN) == 0)
	{
		*result = list;
	}
	
	if (list->child != NULL)
		SG_SearchChild(list->child, menuid, result);
	
	if (list->next != NULL)
		SG_SearchChild(list->next, menuid, result);
}

/****************************************************************
  函数名：SG_DynMenu_Get_MenuList
  功  能  ：获取指定菜单ID的菜单结构指针
  输入参数：
  				menuid:菜单ID
  				list:菜单链表头节点指针
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
MENULIST* SG_DynMenu_Get_MenuList (MENULIST *list, unsigned char menuid[MENUID_LEN])
{
	MENULIST *tmp, *result = NULL;
	
	if (list == NULL)
	{
		SCI_TRACE_LOW( "SG_DynMenu_Get_MenuList: PARA\r\n");	
		return 0;
	}	

	tmp = list;

	SG_SearchChild(tmp, menuid, &result);

	if (result == NULL)
		SCI_TRACE_LOW( "SG_DynMenu_Get_MenuList: RETURN NULL\r\n");
	
	return result;
}

/****************************************************************
  函数名：SG_DynMenu_HandleMsg
  功  能  ：接收和处理串口返回的消息
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  	增加快捷键处理，李松峰，2005-11-5
  	增加对回调函数返回值处理，判断是否发送菜单
  	李松峰， 2005-11-8
****************************************************************/
int SG_DynMenu_HandleMsg (unsigned char *msg)
{
	char *buf;
	char type, sign, result;
	unsigned char menuId[MENUID_LEN];
	MENULIST *list = NULL, *menuList = NULL;
 	int len = 0;
	int p=0;

	// 获取当前菜单
	if ((list = SG_Menu_Get_MenuList()) == NULL)
	{
		SCI_TRACE_LOW("<<<<<<GET A NULL LIST!!");
		return 0;
	}
	
	menuList = NULL;
	
	//类型+标志
	type = msg[p];	//类型

	p += 1;

	switch (type)
	{
		//请求菜单（列表/项）
		case TYPE_GET_MENU_LIST: 
	
		sign = msg[p];	//标志
		p += 1;
	
		SCI_TRACE_LOW( "<<<<<<SG_DynMenu_GetMsg: TYPE: 0x%02x; SIGN: 0x%02x\r\n", type, sign);
	
		switch (sign)
		{
			//请求子菜单（列表/项）
			case SIGN_GET_CHILD_MENU: 
		
				SCI_MEMCPY(menuId, &msg[p], MENUID_LEN);

				//a.取子菜单列表发送
				//b.已经是功能菜单，回调函数，发送结果信息
				menuList = SG_DynMenu_Get_MenuList(list, menuId);
				if (menuList == NULL)
					SG_DynMenu_Put_MenuList (NULL);				

				if (menuList->child != NULL)
					SG_DynMenu_Put_MenuList (menuList->child);				
				else if (menuList->item->proc != NULL)
				{
				//回调函数都为NULL判断为首次调用
					if (menuList->item->proc(menuId, NULL, NULL) == 0)
						SG_DynMenu_Put_MenuList (menuList);				
				}
			
				break;

			//请求父菜单（列表）
			case SIGN_GET_FATHER_MENU: 
	
				SCI_MEMCPY(&menuId, &msg[p], MENUID_LEN);
		
				menuList = SG_DynMenu_Get_MenuList(list, menuId);

				if (menuList && menuList->father != NULL)
					SG_DynMenu_Put_MenuList (menuList->father);	
				else
					SG_DynMenu_Put_MenuList (NULL);				
		
			break;

			//请求主菜单
			case SIGN_GET_MAIN_MENU: 
				SG_Menu_Clear_SuperLogin();

				SG_DynMenu_Put_MenuList (list);	

				break;
		}
		break;
	
	//菜单操作参数回送	
	case TYPE_MENU_PARA: 
	
		sign = msg[p];
		p += 1;
		SCI_MEMCPY(&menuId, &msg[p], MENUID_LEN);
		p += MENUID_LEN;
		result = msg[p];
		p += 1;

		SCI_TRACE_LOW( "SG_DynMenu_GetMsg: TYPE: 0x%02x; SIGN: 0x%02x\r\n", type, sign);

		menuList = SG_DynMenu_Get_MenuList(list, menuId);

		//找不到菜单列表回送空列表
		//例如在独立消息框等情况
		if (menuList == NULL)
			SG_DynMenu_Put_MenuList (NULL);				
		
		switch (sign)
		{
			//输入框参数回送
			case SIGN_INPUT_BOX_PARA: 
	
				len = strchr((char*)&msg[p], END)- (char*)(&msg[p]);
				buf = SCI_ALLOC( len+1);
				SCI_MEMSET(buf,0,len+1);
				if (buf == NULL)
					break;
				SCI_MEMCPY(buf, &msg[p], len);

				//输入取消或者错误，回送菜单列表
				if (result == INPUT_CHOOSE_NO || len <= 0)
				{
					SG_DynMenu_Put_MenuList (menuList);
				}
				else
				{
					//输入正确
					//输入框给回调函数判断是否需要处理
					//返回0表示不需要处理，返回菜单列表
					if (menuList->item->proc == NULL || menuList->item->proc(menuId, buf, NULL) == 0)
						SG_DynMenu_Put_MenuList (menuList);				
				}
        			
				SCI_FREE(buf);
				buf = NULL;
		
				break;

			 //选择框参数回送
			case SIGN_CHOOSE_BOX_PARA:
			
				//选择框给回调函数判断是否需要处理
				//返回0表示不需要处理，返回菜单列表
				if (menuList->item->proc == NULL || menuList->item->proc(menuId, &result, NULL) == 0)
					SG_DynMenu_Put_MenuList (menuList);				
				
				break;
    		
			//消息框参数回送
			case SIGN_MSG_BOX_PARA: 
    		
				//消息框给回调函数判断是否需要处理
				//返回0表示不需要处理，返回菜单列表
				//若无菜单列表,手柄显示消息框前的画面
				if (menuList->item->proc == NULL)
					SG_DynMenu_Put_MenuList (NULL);	
				else if (menuList->item->proc(menuId, &result, NULL) == 0)					
					SG_DynMenu_Put_MenuList (menuList);				
				break;
		}
		break;
	
	//快捷键回送
	case TYPE_SHORTCUT_KEY: 
	
		sign = msg[p];
		p += 1;

		SCI_TRACE_LOW( "SG_DynMenu_GetMsg: TYPE_SHORTCUT_KEY: 0x%02x\r\n", sign);

		//调用快捷键处理函数			
		SG_Proc_ShortCut(sign);

		break;
	//处理电召应答
#if (0)
case TYPE_MUSTER_ACK:
	if(gMusterTel) 
	{
		//无人应答
		if(msg[1] ==3)
		{
			gMusterCnt ++;					
			if(gMusterCnt < 4)
			{
				//动态菜单滚动消息框
				SG_DynMenu_Put_VoiceBox (menuid, gMuster_gbk, gMuster_phone, gMuster_phone);
			}
			else
			{
				gMusterCnt =0;
				if(MsgMusterAck(&msg[1], 1,&pOut,&outLen) == MSG_TRUE)
				{
					SG_Send_Put_New_Item(1, pOut, outLen);
					//动态菜单滚动消息框,电召超时
					SG_DynMenu_Put_AutoShowBox(menuid, gbk);
					gMusterTel = 0;		
				}
			}
		}
		else	//确认或拒接
		{
			gMusterCnt = 0;
			if(MsgMusterAck(&msg[1], 1,&pOut,&outLen) == MSG_TRUE)
			{
				SG_Send_Put_New_Item(1, pOut, outLen);
				gMusterTel = 0;
			}
		}
		
	}
	break;
#endif /* (0) */

	default:
	
		SCI_TRACE_LOW( "SG_DynMenu_GetMsg: DEFAULT\r\n");
	
		break;
	}

	return 0;
//	goto reread;
}



