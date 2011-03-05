/**************************************************************** ��Ȩ 
�������������ͨѶ���޹�˾ 2002-2005 ��Ȩ���� �ļ��� ��SG_DynamicMenu.c �汾 
��1.00 ������ �����ɷ� ����ʱ�䣺2005-9-15 
����������SG2000�������ֱ�֮�䶯̬�˵�Э��        �޸ļ�¼�� 
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
  ��������SG_DynMenu_Put_MenuList
  ��  ��  �����Ͷ�̬�˵��б�
  ���������
  				list:�˵��б�
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  			���Ӳ˵����������2005-10-24�����ɷ�
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

		//����
		menulist[p] = TYPE_MENU_LIST;
		p += 1;

		//�˵�����
		menulist[p] = num;
		p += 1;
	}
	else
	{
		//�ҵ�ͬ����һ���˵�
		head = list;
		while(head->pre)
			head = head->pre;

		//�˵����������ݳ���	
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
		
		//������Ƽ�
		p = 0;
		menulist[p] = TYPE_MENU_LIST;
		p += 1;
	
		//�˵�����{}
		menulist[p] = num;
		p += 1;

		//��ǰ�˵���Ϊ�˵��б��һ��
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

		//��ӵ�ǰ�˵�֮ǰ�Ĳ˵�
		tmp = head;
		while (tmp)
		{
			//��ǰ�˵�֮���Ѿ����
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

	//����	
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
  ��������SG_DynMenu_Put_InputBox
  ��  ��  �����������˵���
  ���������
  				menuid:�˵�ID(�����ǵ��õĸ��˵���ID)
  				title:��ʾ˵��ͷ
  				tail:��ʾ˵��β����ʱ�䵥λ�ȣ�
  				password:����/���ı�־
  				maxLen:������󳤶�
  				charType:����/�ַ�ѡ��
  				upLow:��Сд
  				card:ˢ����־
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
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
	//BIG-ENDIAN�洢��ʽ
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
	
	//����	
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
  ��������SG_DynMenu_Put_ChooseBox
  ��  ��  ������ѡ���˵���
  ���������
  				menuid:�˵�ID(�����ǵ��õĸ��˵���ID)
  				title:��ʾ˵��
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
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
	
	//����	
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
  ��������SG_DynMenu_Put_MsgBox
  ��  ��  ��������Ϣ��
  ���������
  				menuid:�˵�ID(�����ǵ��õĸ��˵���ID)
  				title:��ʾ˵��
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
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
	
	//����	
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
  ��������SG_DynMenu_Put_VoiceBox
  ��  ��  �������������п�
  ���������
  				menuid:�˵�ID(�����ǵ��õĸ��˵���ID)
  				title:��ʾ˵��
  				phoneNum:�绰����
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  		���������������ƣ�2005-10-24�����ɷ�
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
	
	//����	
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
  ��������SG_DynMenu_Put_AutoShowBox
  ��  ��  �������Զ�������ʾ��Ϣ��
  ���������
  				menuid:�˵�ID(�����ǵ��õĸ��˵���ID)
  				title:��ʾ˵��
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
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
	
	//����	
#ifndef _TRACK 
	SG_Net_DynamicMenu_Snd_Msg((unsigned char*)menuItem, len);
#endif
	SCI_FREE(menuItem);




	//��������
	if(g_set_info.bNewUart4Alarm & (UT_TTS | UT_TTS2))
	{
		//��������
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
  ��������SG_DynMenu_Put_AutoShowBox_t1
  ��  ��  �������Զ�������ʾ��Ϣ���Զ���ʧ
  ���������
  				menuid:�˵�ID(�����ǵ��õĸ��˵���ID)
  				title:��ʾ˵��
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
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
	
	//����	
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

//�ݹ�Ѱ��
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
  ��������SG_DynMenu_Get_MenuList
  ��  ��  ����ȡָ���˵�ID�Ĳ˵��ṹָ��
  ���������
  				menuid:�˵�ID
  				list:�˵�����ͷ�ڵ�ָ��
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
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
  ��������SG_DynMenu_HandleMsg
  ��  ��  �����պʹ����ڷ��ص���Ϣ
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  	���ӿ�ݼ��������ɷ壬2005-11-5
  	���ӶԻص���������ֵ�����ж��Ƿ��Ͳ˵�
  	���ɷ壬 2005-11-8
****************************************************************/
int SG_DynMenu_HandleMsg (unsigned char *msg)
{
	char *buf;
	char type, sign, result;
	unsigned char menuId[MENUID_LEN];
	MENULIST *list = NULL, *menuList = NULL;
 	int len = 0;
	int p=0;

	// ��ȡ��ǰ�˵�
	if ((list = SG_Menu_Get_MenuList()) == NULL)
	{
		SCI_TRACE_LOW("<<<<<<GET A NULL LIST!!");
		return 0;
	}
	
	menuList = NULL;
	
	//����+��־
	type = msg[p];	//����

	p += 1;

	switch (type)
	{
		//����˵����б�/�
		case TYPE_GET_MENU_LIST: 
	
		sign = msg[p];	//��־
		p += 1;
	
		SCI_TRACE_LOW( "<<<<<<SG_DynMenu_GetMsg: TYPE: 0x%02x; SIGN: 0x%02x\r\n", type, sign);
	
		switch (sign)
		{
			//�����Ӳ˵����б�/�
			case SIGN_GET_CHILD_MENU: 
		
				SCI_MEMCPY(menuId, &msg[p], MENUID_LEN);

				//a.ȡ�Ӳ˵��б���
				//b.�Ѿ��ǹ��ܲ˵����ص����������ͽ����Ϣ
				menuList = SG_DynMenu_Get_MenuList(list, menuId);
				if (menuList == NULL)
					SG_DynMenu_Put_MenuList (NULL);				

				if (menuList->child != NULL)
					SG_DynMenu_Put_MenuList (menuList->child);				
				else if (menuList->item->proc != NULL)
				{
				//�ص�������ΪNULL�ж�Ϊ�״ε���
					if (menuList->item->proc(menuId, NULL, NULL) == 0)
						SG_DynMenu_Put_MenuList (menuList);				
				}
			
				break;

			//���󸸲˵����б�
			case SIGN_GET_FATHER_MENU: 
	
				SCI_MEMCPY(&menuId, &msg[p], MENUID_LEN);
		
				menuList = SG_DynMenu_Get_MenuList(list, menuId);

				if (menuList && menuList->father != NULL)
					SG_DynMenu_Put_MenuList (menuList->father);	
				else
					SG_DynMenu_Put_MenuList (NULL);				
		
			break;

			//�������˵�
			case SIGN_GET_MAIN_MENU: 
				SG_Menu_Clear_SuperLogin();

				SG_DynMenu_Put_MenuList (list);	

				break;
		}
		break;
	
	//�˵�������������	
	case TYPE_MENU_PARA: 
	
		sign = msg[p];
		p += 1;
		SCI_MEMCPY(&menuId, &msg[p], MENUID_LEN);
		p += MENUID_LEN;
		result = msg[p];
		p += 1;

		SCI_TRACE_LOW( "SG_DynMenu_GetMsg: TYPE: 0x%02x; SIGN: 0x%02x\r\n", type, sign);

		menuList = SG_DynMenu_Get_MenuList(list, menuId);

		//�Ҳ����˵��б���Ϳ��б�
		//�����ڶ�����Ϣ������
		if (menuList == NULL)
			SG_DynMenu_Put_MenuList (NULL);				
		
		switch (sign)
		{
			//������������
			case SIGN_INPUT_BOX_PARA: 
	
				len = strchr((char*)&msg[p], END)- (char*)(&msg[p]);
				buf = SCI_ALLOC( len+1);
				SCI_MEMSET(buf,0,len+1);
				if (buf == NULL)
					break;
				SCI_MEMCPY(buf, &msg[p], len);

				//����ȡ�����ߴ��󣬻��Ͳ˵��б�
				if (result == INPUT_CHOOSE_NO || len <= 0)
				{
					SG_DynMenu_Put_MenuList (menuList);
				}
				else
				{
					//������ȷ
					//�������ص������ж��Ƿ���Ҫ����
					//����0��ʾ����Ҫ�������ز˵��б�
					if (menuList->item->proc == NULL || menuList->item->proc(menuId, buf, NULL) == 0)
						SG_DynMenu_Put_MenuList (menuList);				
				}
        			
				SCI_FREE(buf);
				buf = NULL;
		
				break;

			 //ѡ����������
			case SIGN_CHOOSE_BOX_PARA:
			
				//ѡ�����ص������ж��Ƿ���Ҫ����
				//����0��ʾ����Ҫ�������ز˵��б�
				if (menuList->item->proc == NULL || menuList->item->proc(menuId, &result, NULL) == 0)
					SG_DynMenu_Put_MenuList (menuList);				
				
				break;
    		
			//��Ϣ���������
			case SIGN_MSG_BOX_PARA: 
    		
				//��Ϣ����ص������ж��Ƿ���Ҫ����
				//����0��ʾ����Ҫ�������ز˵��б�
				//���޲˵��б�,�ֱ���ʾ��Ϣ��ǰ�Ļ���
				if (menuList->item->proc == NULL)
					SG_DynMenu_Put_MenuList (NULL);	
				else if (menuList->item->proc(menuId, &result, NULL) == 0)					
					SG_DynMenu_Put_MenuList (menuList);				
				break;
		}
		break;
	
	//��ݼ�����
	case TYPE_SHORTCUT_KEY: 
	
		sign = msg[p];
		p += 1;

		SCI_TRACE_LOW( "SG_DynMenu_GetMsg: TYPE_SHORTCUT_KEY: 0x%02x\r\n", sign);

		//���ÿ�ݼ�������			
		SG_Proc_ShortCut(sign);

		break;
	//�������Ӧ��
#if (0)
case TYPE_MUSTER_ACK:
	if(gMusterTel) 
	{
		//����Ӧ��
		if(msg[1] ==3)
		{
			gMusterCnt ++;					
			if(gMusterCnt < 4)
			{
				//��̬�˵�������Ϣ��
				SG_DynMenu_Put_VoiceBox (menuid, gMuster_gbk, gMuster_phone, gMuster_phone);
			}
			else
			{
				gMusterCnt =0;
				if(MsgMusterAck(&msg[1], 1,&pOut,&outLen) == MSG_TRUE)
				{
					SG_Send_Put_New_Item(1, pOut, outLen);
					//��̬�˵�������Ϣ��,���ٳ�ʱ
					SG_DynMenu_Put_AutoShowBox(menuid, gbk);
					gMusterTel = 0;		
				}
			}
		}
		else	//ȷ�ϻ�ܽ�
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



