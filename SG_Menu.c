/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Menu.c
  �汾   ��1.00
  ������ �����ɷ�
  ����ʱ�䣺2005-9-23
  ����������SG2000 �ֱ��˵����ܴ��� 
  �޸ļ�¼��
***************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>



#include "SG_DynamicMenu.h"
#include "tb_hal.h"

#include "SG_GPS.h"
#include "SG_Set.h"

#include "SG_Send.h"
#include "SG_Receive.h"
#include "SG_Hh.h"
#include "SG_Menu.h"
#include "SG_Net.h"
#include "SG_MsgHandle.h"
#include "SG_Call.h"
#include "SG_Camera.h"
#include "SG_Ext_Dev.h"
#include "sio.h"


extern uint8 tryCount;
static MENULIST* gpMenuList = NULL;
static MENULIST* gpSetMenuList = NULL;
static MENULIST* gpCurMenuList = NULL;
extern int gn_muster_particular_list;
extern void * temp_s_udpconn;
extern SG_CAMERA CurrCamera;

extern SG_MUSTER_LIST *gp_muster_particular_list;


// �����жϳ��������Ƿ�ͨ��
static  int   superlogin = 0;

void SG_Menu_Clear_SuperLogin()
{

	superlogin = 0;
}


static  int SG_Check_Superpwd(MENULIST  *menuList, char *menuid, void *s1)
{
	if(superlogin)
		return 1;

	if(strlen(g_set_info.sOwnNo) == 0)
	{
		superlogin = 1;
		return 1;
	}
	
	//�״ε���
	if (menuList->item->count == 0)
	{
		if (menuList->item->data != NULL)
		{
			SCI_FREE(menuList->item->data);
			menuList->item->data = NULL;
		}

		menuList->item->count = 1;
		
		SG_DynMenu_Put_InputBox ((unsigned char*)menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 0;
	}
	//��ȡ������Ϣ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{	
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) != 0)
		{
			menuList->item->count = 0;			
			SG_DynMenu_Put_MsgBox ((unsigned char*)menuid, "�����������!");
		}
		else
		{	
			superlogin = 1;
			return 1;
		}
	}
	return 0;
}
#ifdef _DUART
/****************************************************************�ع�����****************************************************************/
static int SG_Proc_Pro_Lock(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{	
	MENULIST *menuList = NULL;	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);	
	if (menuList == NULL)		
		return 0;			
	SCI_TRACE_LOW( "SG_Proc_Pro_Lock: %d\r\n", menuList->item->count);	
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))		
		return 0;	
	if (s1 == NULL && s2 == NULL)			
		menuList->item->count = 0;			
	//�״ε���	
	if (menuList->item->count == 0)	
	{		
		menuList->item->count = 1;		
		if(g_state_info.bNewUart4Alarm&UT_PROLOCK)			
			SG_DynMenu_Put_ChooseBox (menuid, "�ر��ع������?");		
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "���ع�����? ");					
		return 1;	
	}	
	//����ȷ��	
	else if ((menuList->item->count == 1) && (s1 != NULL))	
	{		
		if (*(char*)s1 == INPUT_CHOOSE_YES)		
		{			
			menuList->item->count = 2;					
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);					
			return 1;		
		}		
		else		
		{			
			menuList->item->count = 0;			
			return 0;		
		}	
	}	
	//��ȡ����	
	else if ((menuList->item->count == 2) && (s1 != NULL))		
	{			
		menuList->item->count = 0;			
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)		
		{			
			if(!(g_state_info.bNewUart4Alarm&UT_PROLOCK))			
			{				
				if(g_state_info.bNewUart4Alarm & 0x7fff)			
				{					
					menuList->item->count = 0;							
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");					
					return 1;				
				}						
				
				g_state_info.bNewUart4Alarm|=UT_PROLOCK;				
				SG_DynMenu_Put_MsgBox (menuid, "�ع�������!");			
				g_state_info.bRelease1 = DEV_ENABLE;
			}			
			else			
			{				
				g_state_info.bNewUart4Alarm&=~UT_PROLOCK;				
				SG_DynMenu_Put_MsgBox (menuid, "�ع������ر�!");
				g_state_info.bRelease1 = 0;

			}	
		}		
		else		
		{			
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");		
		}		
		return 1;
	}	
	else
	{		
		menuList->item->count = 0;				
		SG_DynMenu_Put_MsgBox (menuid, "�������!");		
		return 1;	
	}
}

#endif

/****************************************************************
mini
****************************************************************/
#if (0)
static int SG_Proc_MINI_DVR(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_MINI_DVR: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_MINI)
			SG_DynMenu_Put_ChooseBox (menuid, "�ر�MINI?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "��MINI?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_MINI))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_MINI))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_MINI;
				SG_DynMenu_Put_MsgBox (menuid, "MINI��!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_MINI;
				SG_DynMenu_Put_MsgBox (menuid, "MINI�ر�!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}
#endif /* (0) */
/****************************************************************
�ǹ�����ͷ
****************************************************************/
static int SG_Proc_SetCom_XgVideo(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_XgVideo: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_XGCAMERA)
			SG_DynMenu_Put_ChooseBox (menuid, "�ر�����ͷ?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "������ͷ?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_XGCAMERA))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_XGCAMERA))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_XGCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "����ͷ��!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_XGCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "����ͷ�ر�!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}

/****************************************************************
������ľ����ͷ
****************************************************************/
static int SG_Proc_SetCom_QqzmVideo(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_XgVideo: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_QQCAMERA)
			SG_DynMenu_Put_ChooseBox (menuid, "�ر�����ͷ?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "������ͷ?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_QQCAMERA))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_QQCAMERA))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_QQCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "����ͷ��!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_QQCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "����ͷ�ر�!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}

/****************************************************************
���������
****************************************************************/
static int SG_Proc_SetCom_Drive(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_XgVideo: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_set_info.bNewUart4Alarm&UT_DRIVE)
			SG_DynMenu_Put_ChooseBox (menuid, "�رռ��������?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "�򿪼��������?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_DRIVE))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_DRIVE))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_DRIVE;
				SG_DynMenu_Put_MsgBox (menuid, "�����������!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_DRIVE;
				SG_DynMenu_Put_MsgBox (menuid, "����������ر�!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}

/****************************************************************
��������SG_Proc_SetCom_Bus_Dev
���ܣ�������չ�����豸�˵��ص�����
˵����
���룺
���أ�
�޸ļ�¼���������º���,2006-7-5
****************************************************************/
static int SG_Proc_SetCom_Bus_Dev(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_Bus_Dev: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_BUS)
			SG_DynMenu_Put_ChooseBox (menuid, "�رչ�����վ��?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "�򿪹�����վ��?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_BUS))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_BUS))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_BUS;
				SG_DynMenu_Put_MsgBox (menuid, "������վ����!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_BUS;
				SG_DynMenu_Put_MsgBox (menuid, "������վ���ر�!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}


/****************************************************************
�Ե���
****************************************************************/
static int SG_Proc_SetCom_GPS(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_GPS: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_GPS)
			SG_DynMenu_Put_ChooseBox (menuid, "�ر��Ե���?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "���Ե���?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_GPS))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_GPS))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}

				SG_DynMenu_Put_InputBox (menuid, "������:1~2", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
				menuList->item->count = 3;

			}
			else
			{
				menuList->item->count = 0;
				g_state_info.bNewUart4Alarm&=~UT_GPS;
				SG_DynMenu_Put_MsgBox (menuid, "�Ե����ر�!");
				g_state_info.GpsDevType = 0;
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			menuList->item->count = 0;
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}
		

		return 1;
	}
	else if((menuList->item->count == 3) && (s1 != NULL))
	{
		char type = 0;
		
		menuList->item->count = 0;
		type = atoi(s1);
		if(type !=1 && type !=2 )
		{
			
			SG_DynMenu_Put_MsgBox (menuid, "�������!");

			return 1;
		}
		
		if(1 == type)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ZB��������!");
			g_state_info.GpsDevType = 1;
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "YH��������!");
			g_state_info.GpsDevType = 2;
		}
		g_state_info.bNewUart4Alarm|=UT_GPS;
		
		g_state_info.bRelease1 = DEV_ENABLE;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}

/****************************************************************
  ��������SG_Proc_Ver_Ask
  ��  ��  ���汾��ѯ�˵��ص�����
  ���������
  ������������ص�ǰ�汾
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Ver_Ask(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	char title[300] = "";

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	// APP VERSION
	strcpy(title, "����汾\r\n");
			
	strcat(title, g_set_info.sVersion);
				
	strcat(title, "\r\n");
	
	#ifdef _DUART
	strcat(title,"Ӳ���ͺ�:G01C\r\n");
#endif

#ifdef _SUART
#ifdef _TRACK
	strcat(title,"Ӳ���ͺ�:G01B\r\n");
#else
	strcat(title,"Ӳ���汾:");
	sprintf(title+strlen(title),"%x\r\n",g_state_info.hardware_ver);
	strcat(title,"Ӳ���ͺ�:G01A\r\n");
#endif
#endif



	strcat(title,"�û�����:");

	if(g_state_info.user_type == COMMON_VER)
	{
		strcat(title,"��ͨ�汾\r\n");
	}
	else if(g_state_info.user_type == PRIVATE_CAR_VER)
	{
		strcat(title,"˽�ҳ��汾\r\n");	
	}
	
	SG_DynMenu_Put_MsgBox (menuid, title);
				
	return 1;

}


/****************************************************************
  ��������SG_Proc_Net_State
  ��  ��  ��������ϲ�ѯ�ص�����
  ���������
  ������������� ��GPS OK \r\nGSM VERSION xxx��\r\nAPP VERSION xxxx��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ���ӹ��ϲ�ѯ��Ϣ�����ɷ壬2005-12-09
  GPS��λ��ѯ��Ϊ��⵱ǰ״̬�����ɷ壬2006-01-05
****************************************************************/
static int SG_Proc_Net_State(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	char title[200] = "";

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	// GPRS��ǰ����
	strcpy(title, "GPRS���Ž׶�:\r\n");
	
	switch(g_gprs_t.state)
	{
		case GPRS_ACTIVE_PDP:
			strcat(title,"Active PDP������\n");
			break;
		case GPRS_DEACTIVE_PDP:
			strcat(title,"DeActive PDP������\n");
			break;	
		case GPRS_SOCKET_CONNECT:
			strcat(title,"����SOCKET\n");
			break;
		case GPRS_PORXY:
			strcat(title,"���Ӵ���\n");
			break;
		case GPRS_HANDUP:
			strcat(title,"����\n");
			break;
		case GPRS_RXHEAD:
		case GPRS_RXCONTENT:
			strcat(title,"������������\n");
			break;
		case GPRS_DISCONNECT:
			strcat(title,"���ӶϿ�\n");
			break;
		default:
			break;
			
	}
	
	// �ϴη����Ĵ���
	strcat(title,"\r�������:\r\n");
	
	switch(g_state_info.lasterr)
	{
		case SG_ERR_NONE:
			strcat(title,"δ��������");
			break;
		case SG_ERR_CREAT_SOCKET:
			strcat(title,"����Socket");
			break;
		case SG_ERR_PROXY_CONNET:
			strcat(title,"����Proxy");
			break;
		case SG_ERR_PROXY_NOTFOUND:
			strcat(title,"��������");
			break;
		case SG_ERR_PROXY_DENY:
			strcat(title,"Proxy �ܾ�");
			break;
		case SG_ERR_CONNET_CENTER:
			strcat(title,"���Ĵ���");
			break;
		case SG_ERR_RCV_HANDUP:
			strcat(title, "����Ӧ��");
			break;
		case SG_ERR_RCV_DATA:
			strcat(title,"��������");
			break;
		case SG_ERR_SEND_DATA:
			strcat(title, "��������");
			break;
		case SG_ERR_RCV_ANS:
			strcat(title, "����Ӧ��");
			break;
		case SG_ERR_TIME_OUT:
			strcat(title, "��ʱ����\n");
			switch(g_state_info.nNetTimeOut)
			{
				case GPRS_ACTIVE_PDP:
					strcat(title,"Active PDP������");
					break;
				case GPRS_DEACTIVE_PDP:
					strcat(title,"DeActive PDP������");
					break;	
				case GPRS_PORXY:
					strcat(title,"���Ӵ���");
					break;
				case GPRS_HANDUP:
					strcat(title,"����");
					break;
				case GPRS_RXHEAD:
				case GPRS_RXCONTENT:
					strcat(title,"���ձ�������");
					break;
				case GPRS_SOCKET_CONNECT:
					strcat(title,"����SOCKET");
					break;
			}
			strcat(title, "��ʱ!\r\n");
			break;
			
		case SG_ERR_CLOSE_SOCKET:
			strcat(title, "�ر�SOCKET");
			break;
	
		case SG_ERR_HOST_PARSE:
			strcat(title, "������ַ����");
			break;
		case SG_ERR_ACTIVE_PDP:
			strcat(title, "����PDP������");
			break;
		case SG_ERR_DEACTIVE_PDP:
			strcat(title, "ȥ��PDP������");
			break;
		case SG_ERR_SET_OPT:
			strcat(title, "��������");
			break;
		default:
			break;
	}

	strcat(title, "\r\n");

	// ��ǰ�ۼ���������
	sprintf(title+strlen(title), "�ۼ���������:%ld��\r\n",ReConCnt);
	sprintf(title+strlen(title), "����״̬:%d\r\n",g_state_info.plmn_status);
	

	SG_DynMenu_Put_MsgBox (menuid, title);
				
	return 1;

}




/****************************************************************
  ��������SG_Proc_Check_Device
  ��  ��  ������״̬�˵��ص�����
  ���������
  ������������� ��GPS OK \r\nGSM VERSION xxx��\r\nAPP VERSION xxxx��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ���ӹ��ϲ�ѯ��Ϣ�����ɷ壬2005-12-09
  GPS��λ��ѯ��Ϊ��⵱ǰ״̬�����ɷ壬2006-01-05
****************************************************************/
static int SG_Proc_Check_Device(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	char title[500] = "";
	char net_status = 0;

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	// �ǣУ�ģ���ѡ״̬���Ѷ�λ���������쳣
	if(((gps_data *)(g_state_info.pGpsCurrent))->status)// �Ѿ���λ
	{
		strcpy(title, "GPS �Ѷ�λ!\r\n");
	}
	else
	{
		strcpy(title, "GPS δ��λ!\r\n");
	}

	sprintf(title+strlen(title), "��ǰ����: %d\r\n",((gps_data *)(g_state_info.pGpsCurrent))->nNum);
	
					
	if (g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF )
		strcat(title, "����ƽ����!\r");

	else if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE )
		strcat(title, "����ƽǷѹ!\r");
		
	strcat(title, "����״̬:  ");
	
	if(g_state_info.alarmState & ~(GPS_CAR_STATU_POS|GPS_CAR_STATU_ACC))
	{
		sprintf(title+strlen(title), "0x%08x\r",(int)g_state_info.alarmState);
	}
	else
		strcat(title, "����.\r");

	if(TRUE == SG_Set_Check_CheckSum()){
		strcat(title, "��������\r");
	}
	else{
		strcat(title, "�����쳣\r");
	}

	if(g_set_info.bAutoAnswerDisable)
	{
		strcat(title,"ʹ���ֱ�ͨ��\r");
	}
	else
	{
		strcat(title,"ʹ������ͨ��\r");
	}



	if (g_set_info.bRegisted == 0)
		strcat(title, "�ն�δע��!\r");
	
	sprintf(title+strlen(title), "�ۼ���������:\r\nAll %d��\r\nFormal %d��\r\n",g_set_info.nResetCnt,g_set_info.nReConCnt);
	sprintf(title+strlen(title), "�ϴ�����ԭ�� %d\r\n",Restart_Reason);
	sprintf(title+strlen(title), "��Ӫ�� %d\r\n�ź� %d\r\n",g_state_info.opter,g_state_info.rxLevel);
	strcat(title, "IMEI:");
	strcat(title, g_state_info.IMEI);
	strcat(title, "\r\n");
	//sprintf(title+strlen(title), "IMSI %s\r\n",g_state_info.imsi);
	
	net_status = SG_Net_Judge();

	switch(net_status)
	{
		case NO_SIM_CARD:
			strcat(title,"��SIM��\r\n");
			break;
		case NO_NET_WORK:
			strcat(title,"������\r\n");
			break;
		case GSM_ONLY:
			strcat(title,"ONLY GSM\r\n");	
			break;
		case GPRS_ONLY:
			strcat(title,"ONLY GPRS\r\n");	
			break;
		case NETWORK_OK:
			strcat(title,"��������\r\n");	
			break;
		default:
			break;
	}

	if(g_set_info.bNewUart4Alarm & UT_GPS)
	{
		if(g_set_info.GpsDevType == 1)
		{
			
			strcat(title,"����ZB������");
		}
		else if(g_set_info.GpsDevType == 2)
		{
			
			strcat(title,"����YH������");			
		}
	}
#if (0)
	{
		int space;
		space = EFS_GetFreeSpace();
		sprintf(title+strlen(title),"�ļ�ϵͳʣ��ռ� %d,",space);
		space = EFS_GetUsedSpace();
		sprintf(title+strlen(title),"�Ѿ�ʹ�ÿռ� %d\r\n",space);
	}
#endif /* (0) */

	SG_DynMenu_Put_MsgBox (menuid, title);
				
	return 1;

}



/****************************************************************
  ��������SG_Proc_Theft
  ��  ��  ���������ܲ˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  �������ʱ��ֹ �򿪷������ܣ����ɷ壬2005-12-15
****************************************************************/
static int SG_Proc_Theft(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
	{
		menuList->item->count = 0;
		SG_Menu_Clear_SuperLogin();
	}
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
		
	//�״ε���
	if (menuList->item->count < 2)
	{
		menuList->item->count = 2;

		if (g_set_info.nTheftState == 0)
		{
					
			if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
			{
				menuList->item->count = 0;			
				SG_DynMenu_Put_MsgBox (menuid, "����Ϩ����ʹ�÷�������!");	
				return 1;

			}
		
			if (g_set_info.bTheftDisable == 1)
			{
				menuList->item->count = 0;			
				SG_DynMenu_Put_MsgBox (menuid, "�������ܱ����ã�����ϵ���Ŀ�ͨ!");	
				return 1;
			}	
			
			 if(g_set_info.bTheftDisable == 0)
				SG_DynMenu_Put_ChooseBox (menuid, "��������������?");
				
		}	
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "�رշ�������?");

		return 1;
	}

	//����ȷ��
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			
			if (g_set_info.nTheftState == 0)
			{
				menuList->item->count = 0;

				if (g_set_info.bTheftDisable == 0)
				{
					SG_Set_Cal_CheckSum();
					g_set_info.nTheftState = 1;
					SG_Set_Save();									
					SG_DynMenu_Put_ChooseBox (menuid, "�ɹ��򿪵���������");
				}	
			}	
			else
			{
				if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
				{
					menuList->item->count = 0;			
					SG_DynMenu_Put_MsgBox (menuid, "����Ϩ����ʹ�÷�������!");	
					return 1;
				}
				SG_Set_Cal_CheckSum();
				g_set_info.nTheftState = 0;
				if(g_state_info.alarmState & GPS_CAR_STATU_ROB_ALARM)
				{
					g_state_info.alarmState &= ~GPS_CAR_STATU_ROB_ALARM;
					g_set_info.alarmState &= ~GPS_CAR_STATU_ROB_ALARM;
				}
				SG_Set_Save();		
				Pic_Send(0xc5); 
				SG_DynMenu_Put_MsgBox (menuid, "�����ɹ��ر�!");			

			}
			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}


}


/****************************************************************
  ��������SG_Proc_Medical_Service
  ��  ��  ��ҽ�Ʒ���˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  �ڲ˵��з�������ʱ����ֵĲ˵�����ʱ��Ҫ�ȷ��ղ˵������ɷ壬2005-12-08
****************************************************************/
static int SG_Proc_Medical_Service(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	if (s1 != NULL || s2 != NULL)
		return 0;

//	SG_DynMenu_Put_MenuList (NULL);							

	SG_DynMenu_Put_VoiceBox(menuid, "����ҽ����������?",  g_set_info.sMedicalNo,  "ҽ������");
	
	return 1;
}

/****************************************************************
  ��������SG_Proc_Help_Service
  ��  ��  ��ά�޷���˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  �ڲ˵��з�������ʱ����ֵĲ˵�����ʱ��Ҫ�ȷ��ղ˵������ɷ壬2005-12-08  
****************************************************************/
static int SG_Proc_Help_Service(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	if (s1 != NULL || s2 != NULL)
		return 0;
	
//	SG_DynMenu_Put_MenuList (NULL);							
	SG_DynMenu_Put_VoiceBox(menuid, "����ά����������?",  g_set_info.sServiceNo,  "ά������");


	
	return 1;
}

/****************************************************************
  ��������SG_Proc_Hot_Service
  ��  ��  ���������߲˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  �ڲ˵��з�������ʱ����ֵĲ˵�����ʱ��Ҫ�ȷ��ղ˵������ɷ壬2005-12-08
****************************************************************/
static int SG_Proc_Hot_Service(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	if (s1 != NULL || s2 != NULL)
		return 0;
	
//	SG_DynMenu_Put_MenuList (NULL);							
	SG_DynMenu_Put_VoiceBox(menuid, "�����������?",  g_set_info.sHelpNo,  "��������");


	return 1;
}


/****************************************************************
  ��������SG_Proc_GPS_Info
  ��  ��  ��GPS��Ϣ��ѯ�˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ���ӵ�ǰ�޶�λ��ʾ����ʾ�����λ��Ϣ��
  ���ɷ壬2005-12-02
****************************************************************/
static int SG_Proc_GPS_Info(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	gps_data *gps = (gps_data*)g_state_info.pGpsFull;
	char  title[512] = "";
	char buf[128]="";
	int len=0;
	MENULIST *menuList = NULL;
	float m=0;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	
	if (menuList == NULL)
		return 0;

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
		
	memset(title,0,sizeof(title));

	memset(buf,0,sizeof(buf));
		
	if (((gps_data*)g_state_info.pGpsCurrent)->status == 0)
	{
		strcpy(title, "��ǰGPS�޶�λ\r�����λ��Ϣ:\r\n");
	}	
	else
	{
		strcpy(title,"GPS�Ѷ�λ\r��ǰ��λ��Ϣ:\r\n");
	}
	strcat(title, "gps �ٶ�:");
	sscanf(gps->sSpeed,"%f",&m);
	m = m*1.852;
	sprintf(buf,"%0.2f",m);
	strcat(title,buf);
	strcat(title, " km/h\r\n");

	strcat(title, "�ɼ�����:");
	sprintf(title+strlen(title), "%d ��", gps->nNum);
	strcat(title, "\r\n");

	
	strcat(title, "�Ƕ�:");
	strcat(title, gps->sAngle);
	strcat(title, "\r\n");

	strcat(title, "����:");
	strcat(title, gps->sLongitude);
	strcat(title, gps->sSouthNorth);
	strcat(title, "\r\n");
	
	strcat(title, "γ��:");
	strcat(title, gps->sLatitude);
	strcat(title, gps->sEastWest);
	strcat(title, "\r\n");


	strcat(title, "�߶�:");
	strcat(title, gps->sAltitude);
	strcat(title, " ��\r\n");


	strcat(title, "UTCʱ��:");
	strcat(title, gps->sTime);
	strcat(title, "\r\n");

	strcat(title, "UTC����:");
	strcat(title, gps->sDate);
	strcat(title, "\r\n");
	
	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}


/****************************************************************
  ��������SG_Proc_Cacular_Total_Distance
  ��  ��  �������ͳ��
  ���������
  ���������
  ��д��  ���º���
  �޸ļ�¼��������2006-5-23
****************************************************************/
static int SG_Proc_Cacular_Total_Distance(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;
	sprintf(str,"%.3f����\r\n",(double)g_state_info.nGetTotalDistance/1000);	

	SG_DynMenu_Put_MsgBox (menuid, str);
	
	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_Total_Distance
  ��  ��  �������ͳ�����ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Set_Total_Distance(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Total_Distance: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	

	if(SG_Check_Superpwd(menuList,(char *)menuid,s1) == 0)
		return 1;
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "�����:", NULL, DECRYPT_BUF, 9, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		g_watch_info.nGetTotalDistance = atol(s1);
		g_state_info.nGetTotalDistance = g_watch_info.nGetTotalDistance;
		
		SG_Watch_Save();

		SG_Net_Disconnect();
	
		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		SG_Menu_Clear_SuperLogin();

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}

	return 1;
}

/****************************************************************
  ��������SG_Proc_Cacular_Distance1
  ��  ��  �����1  ��ͳ��
  ���������
  ���������
  ��д��  ���º���
  �޸ļ�¼��������2006-5-23
****************************************************************/
static int SG_Proc_Cacular_Distance1(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;

	sprintf(str,"%.3f����",(double)g_state_info.nGetDistance[0]/1000);	
	SG_DynMenu_Put_MsgBox (menuid, str);

	return 1;
}


/****************************************************************
  ��������SG_Proc_Cacular_Distance2
  ��  ��  �����2  ��ͳ��
  ���������
  ���������
  ��д��  ���º���
  �޸ļ�¼��������2006-5-23
****************************************************************/
static int SG_Proc_Cacular_Distance2(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	sprintf(str,"%.3f����",(double)g_state_info.nGetDistance[1]/1000);	
	SG_DynMenu_Put_MsgBox (menuid, str);
	return 1;
}
/****************************************************************
  ��������SG_Proc_Clr_Distance1
  ��  ��  �����1  ��ͳ������
  ���������
  ���������
  ��д��  ���º���
  �޸ļ�¼��������2006-5-23
****************************************************************/
static int SG_Proc_Clr_Distance1(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;

	g_watch_info.nGetDistance[0]=0;
	g_state_info.nGetDistance[0]=0;
	SG_Watch_Save();

	sprintf(str,"���1������");	
	SG_DynMenu_Put_MsgBox (menuid, str);
	
	return 1;
}

/****************************************************************
  ��������SG_Proc_Clr_Distance2
  ��  ��  �����2  ��ͳ������
  ���������
  ���������
  ��д��  ���º���
  �޸ļ�¼��������2006-5-23
****************************************************************/
static int SG_Proc_Clr_Distance2(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	g_watch_info.nGetDistance[1]=0;
	g_state_info.nGetDistance[1]=0;
	SG_Watch_Save();

	sprintf(str,"���2������");	
	SG_DynMenu_Put_MsgBox (menuid, str);
	
	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_OneGPS
  ��  ��  ��һ���������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  �ֶ����ñ����ź��Ѻ�̨���պ���Կ�����̣߳����ɷ壬2006-01-11
****************************************************************/
static int SG_Proc_Set_OneGPS(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_CenterNo: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;

	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "һ����������:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sOneGPSNo, s1);
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_AlarmSms
  ��  ��  �����ű����������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  �ֶ����ñ����ź��Ѻ�̨���պ���Կ�����̣߳����ɷ壬2006-01-11
****************************************************************/
static int SG_Proc_Set_AlarmSms(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_CenterNo: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;

	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "���ű�������1:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		memset(g_set_info.sSmsAlarmNo1,0,sizeof(g_set_info.sSmsAlarmNo1));
		strcpy(g_set_info.sSmsAlarmNo1, s1);
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_AlarmSms2
  ��  ��  �����ű����������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  �ֶ����ñ����ź��Ѻ�̨���պ���Կ�����̣߳����ɷ壬2006-01-11
****************************************************************/
static int SG_Proc_Set_AlarmSms2(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_CenterNo: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;

	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "���ű�������2:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		memset(g_set_info.sSmsAlarmNo2, 0, sizeof(g_set_info.sSmsAlarmNo2));
		strcpy(g_set_info.sSmsAlarmNo2, s1);
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Call_Out
  ��  ��  ����ֹ�����˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Call_Out(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Call_Out: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.bCallOutDisable == 1)
			SG_DynMenu_Put_ChooseBox (menuid, "�رպ�������?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "������������?");

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			if (g_set_info.bCallOutDisable == 1)
			{
				g_set_info.bCallOutDisable = 0;
				SG_DynMenu_Put_MsgBox (menuid, "���������ѹر�!");
			}
			else
			{
				g_set_info.bCallOutDisable = 1;
				SG_DynMenu_Put_MsgBox (menuid, "������������!");			
			}	

			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}

/****************************************************************
  ��������SG_Proc_Call_In
  ��  ��  ����ֹ����˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Call_In(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Call_In: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.bCallInDisable == 1)
			SG_DynMenu_Put_ChooseBox (menuid, "�رպ�������?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "������������?");

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			
			if (g_set_info.bCallInDisable == 1)
			{
				g_set_info.bCallInDisable = 0;
				SG_DynMenu_Put_MsgBox (menuid, "���������ѹر�!");
			}
			else
			{
				g_set_info.bCallInDisable = 1;
				SG_DynMenu_Put_MsgBox (menuid, "������������!");			
			}
			
			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}

/****************************************************************
  ��������SG_Proc_Auto_Call
  ��  ��  ���Զ���������
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Auto_Call(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Call_In: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.GpsAutoCall == 0)
			SG_DynMenu_Put_ChooseBox (menuid, "�����Զ�����?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "�ر��Զ�����?");

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			
			if (g_set_info.GpsAutoCall == 0)
			{
				g_set_info.GpsAutoCall = 1;
				SG_DynMenu_Put_MsgBox (menuid, "�Զ�����������!");
			}
			else
			{
				g_set_info.GpsAutoCall = 0;
				SG_DynMenu_Put_MsgBox (menuid, "�Զ������ر�!");			
			}
			
			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}


/****************************************************************
  ��������SG_Proc_Set_Heart_Switch
  ��  ��  ��������������
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Set_Heart_Switch(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Heart_Switch: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.Heart_Switch == 0)
			SG_DynMenu_Put_ChooseBox (menuid, "������������?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "�ر���������?");

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			
			if (g_set_info.Heart_Switch == 0)
			{
				g_set_info.Heart_Switch = 1;
				SG_DynMenu_Put_MsgBox (menuid, "������������!");
			}
			else
			{
				g_set_info.Heart_Switch = 0;
				SG_DynMenu_Put_MsgBox (menuid, "�������ܹر�!");			
			}
			
			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}

/****************************************************************
  ��������SG_Proc_ShortCut
  ��  ��  ����ݼ��ص�����
  ���������key:��ID
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ����������Ϣ���ͽ����ʾ�����ɷ壬2005-12-05
  ���������ֹ�жϣ����ɷ壬2006-01-11
****************************************************************/
int SG_Proc_ShortCut(unsigned char key)
{
	MsgUChar *msg = NULL;
	MsgInt msgLen;
	int menuid = MENUID_SINGLE;

	switch (key)
	{
		case SHORTCUT_MEDICAL:

			SG_DynMenu_Put_VoiceBox((unsigned char*)(&menuid), "����ҽ����������?",  g_set_info.sMedicalNo,  "ҽ������");

			break;
			
		case SHORTCUT_SERVICE:
			
			SG_DynMenu_Put_VoiceBox((unsigned char*)(&menuid), "����ά����������?",  g_set_info.sServiceNo,  "ά������");
		
			break;

		//��������	
		case SHORTCUT_INFO:
#if (0)
			if(g_set_info.bQuickAnswer==1)
			{
				g_set_info.ttsflag=1;
				SG_DynMenu_Put_AutoShowBox((unsigned char*)&menuid, "������Ϣ���ͳɹ�!�ȴ���Ӧ...");	
				g_set_info.ttsflag=0;
				SG_Net_Voice_Snd_Msg(MSG_SND_VOICE,MSG_VOICE_SET_COMBIN,g_tts_buf,g_tts_buflen);
				break;
			}
			
			if (g_set_info.bMusterDisable == 1)
			{
 				SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "�����ѱ���ֹ!");
				break;
			}

			if (MsgMusterFirst(&msg, &msgLen) == MSG_TRUE)
			{
				SG_Send_Put_New_Item(1, msg, msgLen);
				SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "������Ϣ���ͳɹ�!�ȴ���Ӧ...");				
			}
			else
			{
				SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "������Ϣ����ʧ��!");
			}
#endif /* (0) */
			SG_DynMenu_Put_VoiceBox((unsigned char*)(&menuid), "�����������?",  g_set_info.sHelpNo,  "��������");

			break;
	
		default:
			break;
	}
	
	return 1;
}

/****************************************************************
  ��������SG_Proc_Ver
  ��  ��  :�汾�л�
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ͨѶ��ʽSMS�ĳ�GPRSʱ�������������ɷ壬2005-12-20
****************************************************************/
static int SG_Proc_Ver(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_AutoAnswer: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_state_info.user_type == 1) 
			SG_DynMenu_Put_ChooseBox (menuid, "�л�Ϊ˽�ҳ��汾");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "�л�Ϊ��ͨ�汾");

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{

			if (g_state_info.user_type == 1) // ��ֹ�Զ�����
			{
#if (1)
				SG_Set_Cal_CheckSum();
				//g_set_info.GPSEnable = 1;
				g_set_info.bAutoAnswerDisable = 0;
				g_set_info.speaker_vol = 3;	
				SG_Set_Save();
#endif /* (0) */

				g_state_info.user_type = 2;
				MNNV_WriteItem(537, sizeof(char), &g_state_info.user_type);
				SG_DynMenu_Put_MsgBox (menuid, "����˽�ҳ��汾!");	
			}
			else
			{
#if (0)
				SG_Set_Cal_CheckSum();
				g_set_info.GPSEnable = 0;
				SG_Set_Save();
#endif /* (0) */
				SG_Set_Cal_CheckSum();
				g_set_info.bAutoAnswerDisable = 1;
				g_set_info.speaker_vol = 5;	
				SG_Set_Save();
				g_state_info.user_type = 1;
				MNNV_WriteItem(537, sizeof(char), &g_state_info.user_type);
				SG_DynMenu_Put_MsgBox (menuid, "������ͨ�汾!");			
			}
			SG_Soft_Reset(2);
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}

}



/****************************************************************
  ��������SG_Proc_AutoAnswer
  ��  ��  ���Զ������˵��ص�����
  ���������
  ���������
  ��д��  ������֦
  �޸ļ�¼��������2006-2-12
****************************************************************/
static int SG_Proc_AutoAnswer(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_AutoAnswer: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.bAutoAnswerDisable == 0) // �����Զ�����
			SG_DynMenu_Put_ChooseBox (menuid, "ʹ���ֱ�ͨ��?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "ʹ������ͨ��?");

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{

			SG_Set_Cal_CheckSum();
			if (g_set_info.bAutoAnswerDisable == 1) // ��ֹ�Զ�����
			{
				g_set_info.bAutoAnswerDisable = 0;
				SG_DynMenu_Put_MsgBox (menuid, "��������ͨ��!");
			}
			else
			{
				g_set_info.bAutoAnswerDisable = 1;
				SG_DynMenu_Put_MsgBox (menuid, "�����ֱ�ͨ��!");			
			}
			
			SG_Set_Save();
			//SG_Notify_Hh();
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}

/****************************************************************
  ��������SG_Proc_Test_TTS
  ��  ��  ����������оƬTTS
****************************************************************/
static int SG_Proc_Test_TTS(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	unsigned char TTSMsg[]={0x33,0x32,0x31,0x00};

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	if(!(g_state_info.bNewUart4Alarm & UT_TTS)){
		SG_DynMenu_Put_MsgBox (menuid, "�뿪��TTS");
		return 1;
	}

	SG_Voice_Snd_Msg(MSG_VOICE_SET_COMBIN, TTSMsg, 4);
	SG_DynMenu_Put_MsgBox (menuid, "������..........");
	return 1;
	
}

static int SG_Proc_Check_AD(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[50]="";

	if (s1 != NULL || s2 != NULL)
		return 0;

	sprintf(title,"��ǰ����AD ֵ:\r\n %d ",g_state_info.CurrAD/100);

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}

//����ͼ��װ����
static int SG_Proc_Pictrue_Catch(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[200]="";

	if (s1 != NULL || s2 != NULL)
		return 0;
	if(!(g_set_info.bNewUart4Alarm & (UT_XGCAMERA|UT_QQCAMERA)))
	{
		
		strcat(title,"����ͷδ�������뿪������ͷ!!");
	}
	else
	{
		char sig[3];
		
		xSignalHeaderRec      *signal_ptr = NULL;
		sig[0] = 3;
		sig[1] = CAMERA_ONE_REQ;
		sig[2] = PIC_NOT_SAVE;
		SG_CreateSignal_Para(SG_CAMERA_ONE_ASK,3, &signal_ptr,sig); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
		strcat(title,"��������һ��ͼƬ!!\r\n");
		sprintf(title + strlen(title),"�ɹ�����%d��!!",g_state_info.photo_catch_cnt);		
	
	}

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}

/****************************************************************
  ��������SG_Proc_Test_LED
  ��  ��  ������LED
****************************************************************/
static int SG_Proc_Test_LED(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	if(!(g_state_info.bNewUart4Alarm & (UT_LED|UT_LED2))){
		SG_DynMenu_Put_MsgBox (menuid, "�뿪��LED");
		return 1;
	}
	else
	{
		char ledmsg[64]={0};
		int ledlen=0;
		ledmsg[0]=0x0a;
		strcpy(&ledmsg[1],"$$00S0FF0102�ұ����,�뱨��&&");
		ledlen=strlen(ledmsg);
		ledmsg[ledlen]=0x0d;
		
		if(g_set_info.bNewUart4Alarm & UT_LED)
			SIO_WriteFrame(COM_DEBUG,ledmsg,ledlen+1);	
		else if(g_set_info.bNewUart4Alarm & UT_LED)
			SIO_WriteFrame(COM2,ledmsg,ledlen+1);	
		
		SG_DynMenu_Put_MsgBox (menuid, "�ұ����,�뱨��");
		return 1;
	}

}


/******************************************************************
������: SG_Proc_Test_OpenLock
����:�����п�������
*******************************************************************/
static int SG_Proc_Test_OpenLock(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{	
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	if(!(g_set_info.bNewUart4Alarm&UT_LOCK))
	{
		SG_DynMenu_Put_MsgBox (menuid, "�뿪���п���");
		return 1;
	}
	g_state_info.screen=0x00;
	g_state_info.set=0x00;
	
	SG_DynMenu_Put_MsgBox (menuid, "�������");
	return 1;
}
/********************************************************************
������:SG_Proc_Test_CloseLock
����:	    �����п�������
**********************************************************************/
static int SG_Proc_Test_CloseLock(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	if(!(g_set_info.bNewUart4Alarm&UT_LOCK))
	{
		SG_DynMenu_Put_MsgBox (menuid, "�뿪���п���");
		return 1;
	}
	g_state_info.screen=0x00;
	g_state_info.set=0x80;
	SG_DynMenu_Put_MsgBox (menuid, "�������");
	return 1;
}



/****************************************************************
  ��������SG_Proc_SpeakVolChg
  ��  ��  ����Ƶ�����˵��ص�����
  ���������
  ���������
  ��д��  ������֦
  �޸ļ�¼��������2006-2-12
****************************************************************/
static int SG_Proc_SpeakVolChg(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SpeakVolChg: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "��������(1~7):", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int vol = 0;
				
		menuList->item->count = 0;		

		vol = atoi(s1);

		if(vol < 1 || vol > 7)
		{
			menuList->item->count = 0;
			SG_DynMenu_Put_MsgBox (menuid, "�������!");
			return 1;		
		}
		
		SG_Set_Cal_CheckSum();
		g_set_info.speaker_vol = vol;
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");
				
	return 1;
	}	
	else
	{
		menuList->item->count = 0;

		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}


				


				
/****************************************************************
  ��������SG_Proc_IO_Check_State
  ��  ��  �����IO��������״̬
  ���������
  �������������Acc״̬ �Ƽ�״̬ ����״̬ �ֶ�����״̬ Ϩ����� 
  ��д��  ��������
  �޸ļ�¼������ 2007/10/10 
****************************************************************/
static int SG_Proc_IO_Check_State(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	#define DEFAULT_CHK_CNT 	1000

	char  title[512] = "";
	char  buf[128]={0};
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	
	if (menuList == NULL)
		return 0;
				
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
			
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;

	#ifdef _DUART
	if(g_set_info.bNewUart4Alarm & UT_LOCK)
	{
		int j;
		
		strcpy(title,"������IO:");

		if(g_state_info.lockState&0x80)
			strcat(title,"1,");
		else
			strcat(title,"0,");
		
		for(j=0;j<5;j++)
		{
			if((g_state_info.lockState<<(j+3))&0x80)
				strcat(title,"1");
			else
				strcat(title,"0");
		}
	}
	else
		strcpy(title,"�п���: δ����");	
		strcat(title,"\r\n");
	#endif
	
	//sdbj
	if(g_state_info.alarmState & GPS_CAR_STATU_HIJACK)
	{
		sprintf(buf, "%s%s\r\n", "�ֶ�����:", "��");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "�ֶ�����:", "��");
		strcat(title, buf);
	}
	//cmkg
	if(!(g_set_info.alarmSet & GPS_CAR_STATU_IO1))// �Զ��屨���ߵ�ƽ����
	{
		strcat(title,"�Զ��屨���˿�:\r\n�ߵ�ƽ����\r\n");
	}
	else
	{
		strcat(title,"�Զ��屨���˿�:\r\n�͵�ƽ����\r\n");
	}

	if(g_state_info.alarmState & GPS_CAR_STATU_IO1)
	{
		strcat(title,"�Զ��屨���˿�:��\r\n");
	}
	else
	{
		strcat(title,"�Զ��屨���˿�:��\r\n");
	}

#if (0)
	if(g_state_info.alarmState & GPS_CAR_STATU_DOOR)
	{	
		sprintf(buf, "%s%s\r\n", "���ű���:", "��");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "���ű���:", "��");
		strcat(title, buf);
	}
#endif /* (0) */
	if(g_set_info.bBatteryCheckEnable)
	{
		strcat(title,"�Ƽ����ź���������\r\n");	
		if(g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF)
		{
			sprintf(buf, "%s%s\r\n", "�Ƽ����ź�:", "��");
			strcat(title, buf);
		}
		else
		{
			sprintf(buf, "%s%s\r\n", "�Ƽ����ź�:", "��");
			strcat(title, buf);
		}
	}
	else
	{
		strcat(title,"�Ƽ����ź�����ͨʹ��\r\n");	
		if(g_state_info.alarmState & GPS_CAR_STATU_JJQ)
		{
			sprintf(buf, "%s%s\r\n", "�Ƽ����ź�:", "��");
			strcat(title, buf);
		}
		else
		{
			sprintf(buf, "%s%s\r\n", "�Ƽ����ź�:", "��");
			strcat(title, buf);
		}
	}
	//jjq	
	if(g_set_info.JJQLev == 0)// �Զ��屨���ߵ�ƽ����
	{
		strcat(title,"�Ƽ������ź�:\r\n�ߵ�ƽ����\r\n");
	}
	else
	{
		strcat(title,"�Ƽ������ź�:\r\n�͵�ƽ����\r\n");
	}

#if (0)
	if(g_state_info.alarmState & GPS_CAR_STATU_JJQ)
	{
		sprintf(buf, "%s%s\r\n", "�Ƽ����ź�:", "��");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "�Ƽ����ź�:", "��");
		strcat(title, buf);
	}
#endif /* (0) */

	//acc
	if(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)
	{
		sprintf(buf, "%s%s\r\n", "ACC�ź�:", "��");
		strcat(title, buf);
	}
	else 
	{
		sprintf(buf, "%s%s\r\n", "ACC�ź�:", "��");
		strcat(title, buf);
	}
		
	//VCAR_PFO
	if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE)
	{
		sprintf(buf, "%s%s\r\n", "��ƿ:", "Ƿѹ");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "��ƿ:", "����");
		strcat(title, buf);
	}
			
			
	SG_DynMenu_Put_MsgBox (menuid, title);
	return 1;

}


/****************************************************************
  ��������SG_Proc_Set_IO_Level
  ��  ��  �������Զ��屨��io�ڵĴ�����ƽ	
  ���������
  ���������
  ��д��  	��������
  �޸ļ�¼��������2007/10/11 
****************************************************************/
static int SG_Proc_Set_IO_level(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(!(g_set_info.alarmSet & GPS_CAR_STATU_IO1))
			SG_DynMenu_Put_ChooseBox (menuid, "����Ϊ�͵�ƽ���� ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "����Ϊ�ߵ�ƽ���� ?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(g_set_info.alarmSet&GPS_CAR_STATU_IO1)
			{
				SG_Set_Cal_CheckSum();
				g_set_info.alarmSet &= ~GPS_CAR_STATU_IO1;
				SG_Set_Save();
				SG_DynMenu_Put_MsgBox (menuid, "��ǰΪ�ߵ�ƽ����!");
				
			}
			else
			{
				SG_Set_Cal_CheckSum();
				g_set_info.alarmSet |= GPS_CAR_STATU_IO1;
				SG_Set_Save();
				SG_DynMenu_Put_MsgBox (menuid, "��ǰΪ�͵�ƽ����!");
			}

		}
		else
		{
			menuList->item->count = 0;
			
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}


/****************************************************************
  ��������SG_Proc_Set_JJQ_Lev
  ��  ��  �����üƼ�����Ĵ�����ƽ	
  ���������
  ���������
  ��д��  	��������
  �޸ļ�¼��������2007/10/11 
****************************************************************/
static int SG_Proc_Set_JJQ_Lev(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_set_info.JJQLev == 0)
			SG_DynMenu_Put_ChooseBox (menuid, "����Ϊ�͵�ƽ���� ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "����Ϊ�ߵ�ƽ���� ?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(g_set_info.JJQLev == 1)
			{
				SG_Set_Cal_CheckSum();
				g_set_info.JJQLev = 0;
				SG_Set_Save();
				SG_DynMenu_Put_MsgBox (menuid, "��ǰΪ�ߵ�ƽ����!");
				
			}
			else
			{
				SG_Set_Cal_CheckSum();
				g_set_info.JJQLev = 1;
				SG_Set_Save();
				SG_DynMenu_Put_MsgBox (menuid, "��ǰΪ�͵�ƽ����!");
			}

		}
		else
		{
			menuList->item->count = 0;
			
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}



/****************************************************************
  ��������SG_Proc_Set_Battery_Check
  ��  ��  ���Ƽ۸���
  ���������
  ���������
  ��д��  	��������
  �޸ļ�¼��������2007/10/11 
****************************************************************/
static int SG_Proc_Set_Battery_Check(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
	{
		menuList->item->count = 0;
		SG_Menu_Clear_SuperLogin();
	}
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		
		if (g_set_info.bBatteryCheckEnable== 0)
			SG_DynMenu_Put_ChooseBox (menuid, "ʹ�õ��籨��?");					
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "�رյ��籨��?");
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.bBatteryCheckEnable = !g_set_info.bBatteryCheckEnable;
		
			SG_Set_Save();
			if(g_set_info.bBatteryCheckEnable == 1)
			{
				if(g_state_info.alarmState & GPS_CAR_STATU_JJQ)
				{
					g_state_info.alarmState &= ~GPS_CAR_STATU_JJQ;
				}
			}

			SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

			return 1;
		}
		else
		{
			return 0;
		}
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	return 1;

	
}
#ifdef _DUART
/****************************************************************
  ��������SG_Proc_VOLT_Lock
  ��  ��  ����ѹ����		
  ���������
  ���������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/21
****************************************************************/
static int SG_Proc_VOLT_Uncoil(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if(!(g_state_info.bNewUart4Alarm & UT_PROLOCK))
	{
		SG_DynMenu_Put_MsgBox (menuid, "�뿪���ع�����");
		return 1;
	}
	
	if (s1 == NULL && s2 == NULL)	
	{
		menuList->item->count = 0;
		SG_Menu_Clear_SuperLogin();
	}
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	//memcpy(g_pro_lock.menuid,menuid,4);
	//SG_DynMenu_Put_MsgBox (menuid, "�����ɹ�");

	g_pro_lock.nType = PRO_VOLT_UNLOCK;
	//g_pro_lock.nCarNum = 0x01;
	//g_pro_lock.timeout = 0;
	//g_pro_lock.ack = 0;
	g_pro_lock.VoltTime = 1;
	SG_Rcv_Safety_VOLT_UnLock();
		SG_DynMenu_Put_MsgBox (menuid, "�����ɹ�");
	//g_pro_lock.state = SG_MENU_CTRL;
	
	menuList->item->count = 0;
	
	return 1;
	
}

/****************************************************************
  ��������SG_Proc_VOLT_Lock
  ��  ��  ����ѹ����		
  ���������
  ���������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/21
****************************************************************/
static int SG_Proc_VOLT_Lock(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if(!(g_state_info.bNewUart4Alarm & UT_PROLOCK))
	{
		SG_DynMenu_Put_MsgBox (menuid, "�뿪���ع�����");
		return 1;
	}
	
	if (s1 == NULL && s2 == NULL)	
	{
		menuList->item->count = 0;
		SG_Menu_Clear_SuperLogin();
	}
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	//memcpy(g_pro_lock.menuid,menuid,4);
	//SG_DynMenu_Put_MsgBox (menuid, "�����ɹ�");

	g_pro_lock.nType = PRO_VOLT_LOCK;
	//g_pro_lock.nCarNum = 0x01;
	//g_pro_lock.timeout = 0;app
	//g_pro_lock.ack = 0;
	g_pro_lock.VoltTime = 1;
	SG_Rcv_Safety_VOLT_Lock();
	SG_DynMenu_Put_MsgBox (menuid, "�����ɹ�");
	//g_pro_lock.state = SG_MENU_CTRL;
	
	menuList->item->count = 0;
	
	return 1;
	
}

/****************************************************************
  ��������SG_Proc_One_Lock
  ��  ��  ��һ������		
  ���������
  ���������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/11
****************************************************************/
static int SG_Proc_One_Lock(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if(!(g_state_info.bNewUart4Alarm & UT_PROLOCK))
	{
		SG_DynMenu_Put_MsgBox (menuid, "�뿪���ع�����");
		return 1;
	}
	
	if (s1 == NULL && s2 == NULL)	
	{
		menuList->item->count = 0;
		SG_Menu_Clear_SuperLogin();
	}
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	memcpy(g_pro_lock.menuid,menuid,4);
	SG_DynMenu_Put_MsgBox (menuid, "һ������,�ȴ���...");

	g_pro_lock.nType = PRO_ONE_LOCK;
	g_pro_lock.nCarNum = 0x01;
	g_pro_lock.timeout = 0;
	g_pro_lock.ack = 0;
	SG_Rcv_Safety_One_Lock();
	g_pro_lock.state = SG_MENU_CTRL;
	
	menuList->item->count = 0;
	
	return 1;
	
}

/****************************************************************
  ��������SG_Proc_Two_Lock
  ��  ��  ����������		
  ���������
  ���������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/11
****************************************************************/
static int SG_Proc_Two_Lock(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if(!(g_state_info.bNewUart4Alarm & UT_PROLOCK))
	{
		SG_DynMenu_Put_MsgBox (menuid, "�뿪���ع�����");
		return 1;
	}
	
	if (s1 == NULL && s2 == NULL)	
	{
		menuList->item->count = 0;
		SG_Menu_Clear_SuperLogin();
	}
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	memcpy(g_pro_lock.menuid,menuid,4);
	SG_DynMenu_Put_MsgBox (menuid, "��������,�ȴ���...");

	g_pro_lock.nType = PRO_TWO_LOCK;
	g_pro_lock.nCarNum = 0x01;
	g_pro_lock.timeout = 0;
	g_pro_lock.ack = 0;
	g_pro_lock.state = SG_MENU_CTRL;
	SG_Rcv_Safety_Two_Lock();

	menuList->item->count = 0;
	
	return 1;
	
}

/****************************************************************
  ��������SG_Proc_Unioce_Lock
  ��  ��  ������		
  ���������
  ���������
  ��д��  	�����׽�
  �޸ļ�¼��������2008/11/11
****************************************************************/
static int SG_Proc_Uncoil_Lock(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if(!(g_state_info.bNewUart4Alarm & UT_PROLOCK))
	{
		SG_DynMenu_Put_MsgBox (menuid, "�뿪���ع�����");
		return 1;
	}
	
	if (s1 == NULL && s2 == NULL)	
	{
		menuList->item->count = 0;
		SG_Menu_Clear_SuperLogin();
	}
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	memcpy(g_pro_lock.menuid,menuid,4);
	SG_DynMenu_Put_MsgBox (menuid, "����,�ȴ���...");

	g_pro_lock.nType = PRO_UNCOIL_LOCK;
	g_pro_lock.nCarNum = 0x01;
	g_pro_lock.timeout = 0;
	g_pro_lock.ack = 0;
	g_pro_lock.state = SG_MENU_CTRL;
	SG_Rcv_Safety_Uncoil_Lock();
	//g_xih_ctrl.XIH_State = SG_MENU_CTRL;

	//tryCount = 3;
	//Pic_Send(0xa6); // Ϩ�𣬹ر���·
	menuList->item->count = 0;
	
	return 1;
	
}
		
#endif

/****************************************************************
  ��������SG_Proc_Set_IO_Level
  ��  ��  ��Ϩ��		
  ���������
  ���������
  ��д��  	��������
  �޸ļ�¼��������2007/10/11 
****************************************************************/
static int SG_Proc_Set_XIH_Off(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
	{
		menuList->item->count = 0;
		SG_Menu_Clear_SuperLogin();
	}
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	memcpy(g_xih_ctrl.menuid,menuid,4);
	SG_DynMenu_Put_MsgBox (menuid, "Ϩ��,�ȴ���...");
	g_xih_ctrl.XIH_State = SG_MENU_CTRL;

	tryCount = 3;
	Pic_Send(0xa6); // Ϩ�𣬹ر���·
	menuList->item->count = 0;
	
	return 1;
	
}
			
/****************************************************************
  ��������SG_Proc_Set_IO_Level
  ��  ��  ��Ϩ��ָ�		
  ���������
  ���������
  ��д��  	��������
  �޸ļ�¼��������2007/10/11 
****************************************************************/
static int SG_Proc_Set_XIH_On(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
	{
		menuList->item->count = 0;
		SG_Menu_Clear_SuperLogin();
	}
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	memcpy(g_xih_ctrl.menuid,menuid,4);
	SG_DynMenu_Put_MsgBox (menuid, "Ϩ��ָ�,�ȴ���...");
	tryCount = 3;
	g_xih_ctrl.XIH_State = SG_MENU_CTRL;
	Pic_Send(0xc5); // Ϩ��ָ�������·
	menuList->item->count = 0;
	
	return 1;
}


/****************************************************************
  ��������SG_Proc_Muster_Particular_Ask
  ��  ��  ��������Ϣ�ص�����
  ���������
  ���������
  ��д��  ��chenhaihua
  �޸ļ�¼��������2005-9-20
  				 �ı������Ϣ�����ʽ ���� 2006-09-06
****************************************************************/
static int SG_Proc_Muster_Particular_Ask(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
 	int index;
	SG_MUSTER_LIST *list = NULL;
	char ch[100];

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW("SG_Proc_Muster_Particular_Ask: %d\r\n", menuList->item->count);
	
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;

	//�״ε���,����
	if (menuList->item->count == 0)
	{
		if(gn_muster_particular_list<=0){
			sprintf(ch,"����Ϣ");
			SG_DynMenu_Put_MsgBox (menuid, ch);
			return 1;

		}
		else{
		menuList->item->count = 1;
		sprintf(ch,"������Ϣ��0-%d:",gn_muster_particular_list-1);
		SG_DynMenu_Put_InputBox (menuid, ch, NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);

		}

		return 1;
	}
	//��ȡ��Ϣ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		menuList->item->count = 0;

		index = atoi(s1);
		
		if (index<gn_muster_particular_list)
		{
			list = gp_muster_particular_list;
			while(index ++<(gn_muster_particular_list-1) && list != NULL)
			{
				list = list->next;		
			}
			if(list !=NULL)
			{
				SG_DynMenu_Put_MsgBox (menuid, list->msg);
			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "������Ϣ�ų���");
		}

		return 1;
	}

	return 1;

}

/****************************************************************
  ��������SG_Proc_Show_Device
  ��  ��  ���ն�״̬��ѯ�˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Show_Device(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char *title = NULL;

	if (s1 != NULL || s2 != NULL)
		return 0;

	title = SCI_ALLOC( 1024);
		
	if (g_set_info.bRegisted == 0)
		strcpy(title, "δע��!\r");
	else
		strcpy(title, "��ע��!\r");
	
	if (g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF )
		strcat(title, "����ƽ����!\r");

	else if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE )
		strcat(title, "����ƽǷѹ!\r");
	
	#if (__cwt_)
	if (temp_s_udpconn == NULL)
		strcat(title, "GPRS��!\r");
	else
		strcat(title, "GPRSͨ!\r");
	#else
	if (g_state_info.socket <= 0)
		strcat(title, "GPRS��!\r");
	else
		strcat(title, "GPRSͨ!\r");
	#endif		
	strcat(title, "������:\r");
	strcat(title, g_set_info.sOwnNo);
	strcat(title, "\r");
	#if(__cwt_)
	strcat(title, "COMMAND ADDR:\r");
	strcat(title, g_set_info.sCommAddr);
	strcat(title, "\r");
	
	strcat(title, "��Ȩ��:\r");
	strcat(title, g_set_info.sOemCode);
	strcat(title, "\r");
	#endif
	strcat(title, "���ĺ�:\r");
	strcat(title, g_set_info.sCenterNo);
	strcat(title, "\r");

	strcat(title, "ͨѶ��ʽ:");
	if (g_set_info.nNetType == MSG_NET_SMS)
		strcat(title, "����");
	else if (g_set_info.nNetType == MSG_NET_GPRS)
		strcat(title, "����");
	else if (g_set_info.nNetType == MSG_NET_UNION)
		strcat(title, "���");
	strcat(title, "\r");
	
	strcat(title, "����IP:\r");
	strcat(title, g_set_info.sCenterIp);
	strcat(title, " ");
	sprintf(title+strlen(title), "%d", g_set_info.nCenterPort);
	strcat(title, "\r");
	
	strcat(title, "����IP:\r");
	strcat(title, g_set_info.sProxyIp);
	strcat(title, " ");
	sprintf(title+strlen(title), "%d", g_set_info.nProxyPort);
	strcat(title, "\r");

	if (g_set_info.bProxyEnable==1)
		strcat(title, "(ʹ�ô���)\r");
	else
		strcat(title, "(��ʹ�ô���)\r");

	strcat(title, "APN:");
	strcat(title, g_set_info.sAPN);
	strcat(title, "\r");	
	strcat(title, "USER:");	
	strcat(title, g_set_info.sGprsUser);
	strcat(title, "\r");	
	strcat(title, "PSW:");
	strcat(title, g_set_info.sGprsPsw);	
	strcat(title, "\r");

	strcat(title, "��ط�ʽ:");
	if (g_set_info.nWatchType == MSG_WATCH_TIME)
		strcat(title, "��ʱ");
	else if (g_set_info.nWatchType == MSG_WATCH_AMOUNT)
		strcat(title, "����");
	else if (g_set_info.nWatchType == MSG_WATCH_DISTANCE)
		strcat(title, "����");
	else
		strcat(title, "ȡ��");
	strcat(title, "\r");	
	
	if (g_set_info.bCompress == 0)
		strcat(title, "(��ѹ��");
	else
		strcat(title, "(��ѹ��");	

	if (g_set_info.nInflexionRedeem == 0)
		strcat(title, ",�޹յ㲹��");
	else
		strcat(title, ",�йյ㲹��");		
	
	if (g_set_info.bStopReport== 0)
		strcat(title, ",ͣ�����㱨)\r");
	else
		strcat(title, ")\r");		

	sprintf(title+strlen(title), "��ؼ��:%d\r", g_set_info.nWatchInterval);
	if(g_set_info.nWatchType == MSG_WATCH_AMOUNT)
	{

		sprintf(title+strlen(title), "��ش���:%ld\r", g_set_info.nWatchTime);
		if(g_set_info.nWatchTime == 0)
		{
			sprintf(title+strlen(title), "�Ѽ�ش���:%ld\r", g_watch_info.nWatchedTime);
		}
		else
		{
			sprintf(title+strlen(title), "�Ѽ�ش���:%ld\r", g_watch_info.nWatchedTime);
		}
	}
	else
 	{

		sprintf(title+strlen(title), "���ʱ��:%ld\r", g_set_info.nWatchTime);
		if(g_set_info.nWatchTime == 0)
		{
			sprintf(title+strlen(title), "�Ѽ��ʱ��:%ld\r", g_watch_info.nWatchedTime);
		}
		else
		{
			sprintf(title+strlen(title), "�Ѽ��ʱ��:%ld\r", g_watch_info.nWatchedTime);
		}
	}
	

	if(g_set_info.bRegionEnable)
	{
		sprintf(title+strlen(title),"���򱨾���\t");
	}
	
	if(g_set_info.bLineEnable)
	{
		sprintf(title+strlen(title), "·�߱�����\r\n");
	}	
 


	sprintf(title+strlen(title),"��չ����:  0x%x \r ",g_set_info.bNewUart4Alarm);

	
	SG_DynMenu_Put_MsgBox (menuid, title);
	
	if (title)
		SCI_FREE(title);

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_OwnNo
  ��  ��  �������������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  �ֶ����ñ����ź��Ѻ�̨���պ���Կ�����̣߳����ɷ壬2006-01-11
****************************************************************/
static int SG_Proc_Set_OwnNo(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
	
	SCI_TRACE_LOW( "SG_Proc_Set_OwnNo: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}	

	//���汾������
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		#if(__cwt_)
				SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sCommAddr,s1);
		memset(g_state_info.sKeepMsg,0,sizeof(g_state_info.sKeepMsg));
		memset(g_state_info.sChkKeepMsg,0,sizeof(g_state_info.sChkKeepMsg));
		strcpy(g_set_info.sOwnNo, s1);
		
		if(g_set_info.sOwnNo[0] == '0' && strlen(g_set_info.sOwnNo) == 1){
			memset(g_set_info.sOwnNo,0,sizeof(g_set_info.sOwnNo));
			memset(g_set_info.sCommAddr,0,sizeof(g_set_info.sCommAddr));
		}
		SG_Set_Save();
		#else
		SG_Set_Cal_CheckSum();
			strcpy(g_set_info.sOwnNo, s1);
		if(g_set_info.sOwnNo[0] == '0' && strlen(g_set_info.sOwnNo) == 1)
		{
			memset(g_set_info.sOwnNo,0,sizeof(g_set_info.sOwnNo));
			g_set_info.bRegisted = 0;
		}
		else//�����ն��Ѿ��Ǽ�
		{
			g_set_info.bRegisted = 1;
		}
			SG_Set_Save();
		#endif
		g_state_info.randtime = (g_set_info.sOwnNo[9]-'0')*10+ (g_set_info.sOwnNo[10]-'0'); //�����
		SG_Net_Disconnect();
	
		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	return 0;
}


#if (__cwt_)
/****************************************************************
  ��������SG_Proc_Set_OemCode
  ��  ��  �������������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  �ֶ����ñ����ź��Ѻ�̨���պ���Կ�����̣߳����ɷ壬2006-01-11
****************************************************************/
static int SG_Proc_Set_OemCode(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_OemCode: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char *)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "��Ȩ��:", NULL, DECRYPT_BUF, SG_PHONE_LEN, ANYCHAR, ANY_UPLOW, NOCARD);
	}

	//���汾������
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sOemCode, s1);
		memset(g_state_info.sKeepMsg,0,sizeof(g_state_info.sKeepMsg));
		memset(g_state_info.sChkKeepMsg,0,sizeof(g_state_info.sChkKeepMsg));
		SG_Set_Save();
		SG_Net_Disconnect();
		
		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		
	return 1;
}

#endif
		
/****************************************************************
  ��������SG_Proc_Set_CenterNo
  ��  ��  �����ĺ������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  �ֶ����ñ����ź��Ѻ�̨���պ���Կ�����̣߳����ɷ壬2006-01-11
****************************************************************/
static int SG_Proc_Set_CenterNo(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_CenterNo: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;

	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "���ĺ���:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sCenterNo, s1);
		
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_Communication_Type
  ��  ��  ��ͨѶ��ʽ���ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ͨѶ��ʽSMS�ĳ�GPRSʱ�������������ɷ壬2005-12-20
****************************************************************/
static int SG_Proc_Set_Communication_Type(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Communication_Type: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "0����1����2���", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char /*mode = 0, */type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type != MSG_NET_UNION && type != MSG_NET_GPRS && type != MSG_NET_SMS)
		{
			SG_DynMenu_Put_MsgBox (menuid, "�������!");
			return 1;
		}
		
		if (g_set_info.nNetType != (int)type)
		{
			SG_Set_Cal_CheckSum();
			if (g_set_info.nNetType == MSG_NET_SMS)
			{
				g_set_info.nNetType = type;
				SG_Net_Disconnect();				
			
			}	
			else
			{
				g_set_info.nNetType = type;
				if(g_set_info.nNetType == MSG_NET_SMS)
             	SG_Send_Clear_List(3);
				SG_Net_Disconnect();
			}
			
			SG_Set_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_Watch_Type
  ��  ��  ����ط�ʽ���ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ͨѶ��ʽSMS�ĳ�GPRSʱ�������������ɷ壬2005-12-20
****************************************************************/
static int SG_Proc_Set_Watch_Type(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Communication_Type: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "1��ʱ3����4ȡ��", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char /*mode = 0, */type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type != MSG_WATCH_CANCLE && type != MSG_WATCH_AMOUNT && type != MSG_WATCH_TIME)
		{
			SG_DynMenu_Put_MsgBox (menuid, "�������!");
			return 1;
		}
		
		if (g_set_info.nWatchType!= (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.nWatchType = type;				
			SG_Set_Save();
			g_watch_info.nWatchedTime = 0;
			g_watch_info.nLastReportSystemTime = 0;	
			SG_Watch_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_Watch_Interval
  ��  ��  ����ؼ��ʱ�����ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ͨѶ��ʽSMS�ĳ�GPRSʱ�������������ɷ壬2005-12-20
****************************************************************/
static int SG_Proc_Set_Watch_Interval(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Communication_Type: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "���÷�Χ1-65535��", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type == 0 || type >65535)
		{
			SG_DynMenu_Put_MsgBox (menuid, "�������!");
			return 1;
		}
		
		if (g_set_info.nWatchInterval != (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.nWatchInterval = type;				
			SG_Set_Save();
			g_watch_info.nWatchedTime = 0;
			g_watch_info.nLastReportSystemTime = 0;	
			SG_Watch_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_GPS_Interval
  ��  ��  ��GPS����ʱ�������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ͨѶ��ʽSMS�ĳ�GPRSʱ�������������ɷ壬2005-12-20
****************************************************************/
static int SG_Proc_Set_GPS_Interval(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Communication_Type: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "���÷�Χ1-60��", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type == 0 || type >60)
		{
			SG_DynMenu_Put_MsgBox (menuid, "�������!");
			return 1;
		}
		
		if (g_set_info.nGpstTime!= (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.nGpstTime= type;				
			SG_Set_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}


/****************************************************************
  ��������SG_Proc_Set_Heart_Interval
  ��  ��  ������ʱ�������ò˵�  Ĭ��Ϊ120��
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ͨѶ��ʽSMS�ĳ�GPRSʱ�������������ɷ壬2005-12-20
****************************************************************/
static int SG_Proc_Set_Heart_Interval(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Heart_Interval: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "���÷�Χ10-300", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type < 10 || type >300)
		{
			SG_DynMenu_Put_MsgBox (menuid, "�������!");
			return 1;
		}
		
		if (g_set_info.Heart_Interval != (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.Heart_Interval = type;				
			SG_Set_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_GPS_Dist
  ��  ��  ��GPSƯ�ƹ��˾������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ͨѶ��ʽSMS�ĳ�GPRSʱ�������������ɷ壬2005-12-20
****************************************************************/
static int SG_Proc_Set_GPS_Dist(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Communication_Type: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "���÷�Χ5-50��", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type < 5 || type >50)
		{
			SG_DynMenu_Put_MsgBox (menuid, "�������!");
			return 1;
		}
		
		if (g_set_info.nGpstDist != (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.nGpstDist = type;				
			SG_Set_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_Efs_Max
  ��  ��  ��GPSƯ�ƹ��˾������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ͨѶ��ʽSMS�ĳ�GPRSʱ�������������ɷ壬2005-12-20
****************************************************************/
static int SG_Proc_Set_Efs_Max(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Communication_Type: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "���÷�Χ10-200", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type < 10 || type > 200)
		{
			SG_DynMenu_Put_MsgBox (menuid, "�������!");
			return 1;
		}
		
		if (g_set_info.EfsMax!= (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.EfsMax = type;				
			SG_Set_Save();
			MAX_LIST_NUM = g_set_info.EfsMax-g_set_info.nRegionPiont-g_set_info.nLinePiont;
		}

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_CenterIp
  ��  ��  �����ĵ�ַ���ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  	IP�������ΪAAABBBCCCDDD��ʽ�����ɷ壬2005-11-17
****************************************************************/
static int SG_Proc_Set_CenterIp(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_CenterIp: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "���ĵ�ַ:", NULL, DECRYPT_BUF, SG_IP_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char sIp[3+1] = "";
		int nIp;
		char *s = (char*)s1;
		
		menuList->item->count = 0;		
		
		if (strlen(s1) != SG_IP_LEN)
		{
			SG_DynMenu_Put_MsgBox (menuid, "�������!\r\nAAABBBCCCDDD,��12λ,ÿ��3λ,�����ǰ�油0");
			return 1;
		}
		SG_Set_Cal_CheckSum();
		memset(g_set_info.sCenterIp, 0, sizeof(g_set_info.sCenterIp));
		memset(sIp, 0, sizeof(sIp));
		
		memcpy(sIp, &s[0], 3);
		nIp = atoi(sIp);
		sprintf(g_set_info.sCenterIp, "%d", nIp);
		strcat(g_set_info.sCenterIp, ".");
		
		memcpy(sIp, &s[3], 3);
		nIp = atoi(sIp);
		sprintf(g_set_info.sCenterIp+strlen(g_set_info.sCenterIp), "%d", nIp);
		strcat(g_set_info.sCenterIp, ".");
		
		memcpy(sIp, &s[6], 3);
		nIp = atoi(sIp);
		sprintf(g_set_info.sCenterIp+strlen(g_set_info.sCenterIp), "%d", nIp);
		strcat(g_set_info.sCenterIp, ".");

		memcpy(sIp, &s[9], 3);
		nIp = atoi(sIp);
		sprintf(g_set_info.sCenterIp+strlen(g_set_info.sCenterIp), "%d", nIp);
		
		SG_Set_Save();

		SG_Net_Disconnect();
		
		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		

	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_CenterPort
  ��  ��  �����Ķ˿����ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Set_CenterPort(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_CenterPort: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	

	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "���Ķ˿�:", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		g_set_info.nCenterPort = atoi(s1);
		
		SG_Set_Save();

		SG_Net_Disconnect();
	
		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}

	return 1;
}


/****************************************************************
  ��������SG_Proc_Set_ProxyIp
  ��  ��  �������ַ���ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  	IP�������ΪAAABBBCCCDDD��ʽ�����ɷ壬2005-11-17  
****************************************************************/
static int SG_Proc_Set_ProxyIp(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_ProxyIp: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "�����ַ:", NULL, DECRYPT_BUF, SG_IP_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char sIp[3+1] = "";
		int nIp;
		char *s = (char*)s1;
		
		menuList->item->count = 0;		
		
		if (strlen(s1) != SG_IP_LEN)
		{
			SG_DynMenu_Put_MsgBox (menuid, "�������!\r\nAAABBBCCCDDD,��12λ,ÿ��3λ,�����ǰ�油0");
			return 1;
		}
		SG_Set_Cal_CheckSum();
		memset(g_set_info.sProxyIp, 0, sizeof(g_set_info.sProxyIp));
		memset(sIp, 0, sizeof(sIp));
		
		memcpy(sIp, &s[0], 3);
		nIp = atoi(sIp);
		sprintf(g_set_info.sProxyIp, "%d", nIp);
		strcat(g_set_info.sProxyIp, ".");
		
		memcpy(sIp, &s[3], 3);
		nIp = atoi(sIp);
		sprintf(g_set_info.sProxyIp+strlen(g_set_info.sProxyIp), "%d", nIp);
		strcat(g_set_info.sProxyIp, ".");
		
		memcpy(sIp, &s[6], 3);
		nIp = atoi(sIp);
		sprintf(g_set_info.sProxyIp+strlen(g_set_info.sProxyIp), "%d", nIp);
		strcat(g_set_info.sProxyIp, ".");

		memcpy(sIp, &s[9], 3);
		nIp = atoi(sIp);
		sprintf(g_set_info.sProxyIp+strlen(g_set_info.sProxyIp), "%d", nIp);
		
		SG_Set_Save();

		SG_Net_Disconnect();
	
		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	
	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_ProxyPort
  ��  ��  ������˿����ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Set_ProxyPort(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_ProxyPort: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "����˿�:", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		g_set_info.nProxyPort = atoi(s1);
		
		SG_Set_Save();
		
		SG_Net_Disconnect();

		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_ProxyEnable
  ��  ��  ������ʹ�����ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Set_ProxyEnable(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_ProxyEnable: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		
		if (g_set_info.bProxyEnable == 0)
			SG_DynMenu_Put_ChooseBox (menuid, "ʹ�ô�������?");					
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "�رմ�������?");
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.bProxyEnable = !g_set_info.bProxyEnable;

			if(g_set_info.bProxyEnable)
			{
				if(g_set_info.nProxyPort == 0)
					g_set_info.nProxyPort = 80;

				if(strlen(g_set_info.sProxyIp) < 6)
					strcpy(g_set_info.sProxyIp,"10.0.0.172");					
			}
			
			SG_Set_Save();

			SG_Net_Disconnect();

			SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

			return 1;
		}
		else
		{
			return 0;
		}
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	return 1;
}


#if (__cwt_)
/****************************************************************
  ��������SG_Proc_Edit_SMS
  ��  ��  ���Ա༭���Żظ�
  ���������
  ���������0: �����ٽ���1:����
  ��д��  ��chenhaihua
  �޸ļ�¼��������2007-7-27
****************************************************************/
static int SG_Proc_Edit_SMS(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	unsigned char unicode[512] = "";
	unsigned int unicodelen = 0;
	unsigned char basecode[512] = "";
	unsigned int basecodelen = 0;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW("SG_Proc_Edit_SMS: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "�༭����:", NULL, DECRYPT_BUF, 256, ANYCHAR, ANY_UPLOW, NOCARD);
	}

	//���Ͷ�����Ϣ
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		//gbk to unicode
		unicodelen= GBK2Unicode(unicode,s1);

		//�ߵ��ֽ�ת��
		UnicodeBE2LE(unicode,unicodelen);

		//unicode to base64
		basecodelen= base64_encode(basecode,unicodelen,unicode);
		
		MsgEditUp(basecode,basecodelen);
		
		SG_DynMenu_Put_MsgBox (menuid, "���ͳɹ�!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "����ʧ��!");
	}		
	return 1;

}

#else


/****************************************************************
  ��������SG_Proc_Edit_SMS
  ��  ��  ���ϴ�������Ϣ
  ���������
  ���������0: �����ٽ���1:����
  ��д��  ��chenhaihua
  �޸ļ�¼��������2007-7-27
****************************************************************/
static int SG_Proc_Edit_SMS(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MsgUChar *msg = NULL;
	MsgInt msgLen;
	MENULIST *menuList = NULL;
	unsigned char unicode[512] = "";
	unsigned int unicodelen = 0;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Edit_SMS: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "�༭����:", NULL, DECRYPT_BUF, 256, ANYCHAR, ANY_UPLOW, NOCARD);
	}

	//���Ͷ�����Ϣ
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		//gbk to unicode
		unicodelen= GBK2Unicode(unicode,s1);

		//�ߵ��ֽ�ת��
	//	UnicodeBE2LE(unicode,unicodelen);

		if(MsgEditSmsUpload(unicode,unicodelen,&msg,&msgLen) != MSG_FALSE){
		
			SG_Send_Put_New_Item(1, msg, msgLen);

			SG_DynMenu_Put_MsgBox (menuid, "�ѷ���!");
		}
		else{
			menuList->item->count = 0;
			
			SG_DynMenu_Put_MsgBox (menuid, "����ʧ��!");
		}
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "����ʧ��!");
	}		
	return 1;

}

#endif
/****************************************************************
  ��������SG_Proc_Set_APN
  ��  ��  ��APN���ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Set_APN(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	int		userChange = 0;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_APN: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "APN:", NULL, DECRYPT_BUF, SG_APN_LEN, PURE_ENG, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sAPN, s1);
		
		if(strlen(g_set_info.sGprsPsw) == 0 && strlen(g_set_info.sGprsUser) == 0)
		{
			strcpy(g_set_info.sGprsUser, s1);	
			strcpy(g_set_info.sGprsPsw, s1);	
			userChange = 1;
		}
		
		SG_Set_Save();
	
		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	return 1;
}

/****************************************************************
  ��������SG_Proc_Set_Exit
  ��  ��  ���˳����ò˵��˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Set_Exit(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	SG_Menu_Clear_SuperLogin();
	setmenu_status = 0;
	SG_Menu_Set_MenuList(0);
	SG_DynMenu_Put_MenuList (gpMenuList);
	
	return 1;
}


/****************************************************************
  ��������SG_Proc_Set_User
  ��  ��  ��gprs�û������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Set_User(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_User: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "�û���:", NULL, DECRYPT_BUF, SG_GPRS_USER_LEN, PURE_ENG, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sGprsUser, s1);
		
		SG_Set_Save();
		
		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	return 1;
}

/****************************************************************
��������SG_Proc_SetCom_Lock
���ܣ�������չ�п����豸�˵��ص�����
˵����
���룺
���أ�
�޸ļ�¼���������º���,2006-7-10
****************************************************************/
static int SG_Proc_SetCom_Lock(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_Lock: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_LOCK)
			SG_DynMenu_Put_ChooseBox (menuid, "�ر��п���?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "���п���?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_LOCK))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_LOCK))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_LOCK;
				SG_DynMenu_Put_MsgBox (menuid, "�п�����!");
				g_state_info.bRelease1 = DEV_ENABLE;
			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_LOCK;
				SG_DynMenu_Put_MsgBox (menuid, "�п����ر�!");
				if(!(g_state_info.bNewUart4Alarm&UT_TTS))
				{
					#ifdef _DUART
					g_state_info.bRelease1 = 0;
					#else
					g_state_info.bRelease1 = HH_ENABLE;
					#endif
				}
				
			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}

/****************************************************************
��������SG_Proc_SetCom_TTS
���ܣ�TTS �豸�˵��ص�����
˵����
���룺
���أ�
�޸ļ�¼������������,2006-8-7
****************************************************************/
static int SG_Proc_SetCom_TTS(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_TTS: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_TTS)
			SG_DynMenu_Put_ChooseBox (menuid, "�ر�TTS ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "��TTS ?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_TTS))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_TTS))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}

				
				g_state_info.bNewUart4Alarm |= UT_TTS;
				SG_DynMenu_Put_MsgBox (menuid, "TTS ��!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
				
			}
			else
			{
				g_state_info.bNewUart4Alarm &=~UT_TTS;
				SG_DynMenu_Put_MsgBox (menuid, "TTS �ر�!");
				if(!(g_state_info.bNewUart4Alarm&UT_LOCK))
				{
					#ifdef _DUART
					g_state_info.bRelease1 = 0;
					#else
					g_state_info.bRelease1 = HH_ENABLE;
					#endif
				}

			}

		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}

/****************************************************************
��������SG_Proc_SetCom_Debug
���ܣ���DEBUG�˵��ص�����
˵����
���룺
���أ�
�޸ļ�¼������������,2006-8-7
****************************************************************/
static int SG_Proc_SetCom_Debug(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_Debug: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bRelease1 == DEBUG_ENABLE)
			SG_DynMenu_Put_ChooseBox (menuid, "�ر�DEBUG ?");	
		else 
			SG_DynMenu_Put_ChooseBox (menuid, "��DEBUG ?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(g_state_info.bRelease1 != DEBUG_ENABLE)
			{
				g_state_info.bRelease1 = DEBUG_ENABLE;
				SG_DynMenu_Put_MsgBox (menuid, "DEBUG ��!");
			}
			else
			{
				#ifdef _DUART
				if(g_set_info.bNewUart4Alarm & 0x7fff)
				{
					g_state_info.bRelease1 = DEV_ENABLE;					
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG�رղ���������!");
				}
				else
				{
					g_state_info.bRelease1 = 0;
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG �ر�!");
				}
				#else
				if(g_state_info.bNewUart4Alarm)
				{
					g_state_info.bRelease1 = DEV_ENABLE;
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG�رղ���������!");
				}
				else
				{
					g_state_info.bRelease1 = HH_ENABLE;
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG�رղ������ֱ�!");
				}
				#endif
			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}




/****************************************************************
��������SG_Proc_SetCom_Start
���ܣ����õ�ǰ���ûص�����
˵����
���룺
���أ�
�޸ļ�¼������������,2006-8-7
****************************************************************/
static int SG_Proc_SetCom_Start(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_Start: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���

	 if (menuList->item->count == 0)
	{

		menuList->item->count = 1;
		
		SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 1;
	
	}
	//��ȡ����
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			char title[200] = "��ǰ����������!\r\n";

			SG_Set_Cal_CheckSum();
			g_set_info.bRelease1 = g_state_info.bRelease1;
			g_set_info.bRelease2 = g_state_info.bRelease2;
			g_set_info.bNewUart4Alarm = g_state_info.bNewUart4Alarm;
			if(g_set_info.bNewUart4Alarm &UT_LOCK) // �п����������´����������п������� 
			{
				g_set_info.bLockTest = 1;
			}
			if(g_set_info.bNewUart4Alarm &UT_GPS)
			{
				g_set_info.GpsDevType = g_state_info.GpsDevType;
			}
			else
			{
				g_set_info.GpsDevType = 0;				
			}
			
			SG_Set_Save();
			if(g_set_info.bRelease2 == LED2_ENABLE)
			{
				if(g_set_info.bNewUart4Alarm & UT_LED2)
				{
					strcat(title,"����2��LED.");
				}
				if(g_set_info.bNewUart4Alarm & UT_TTS2)
				{
					strcat(title,"����2��TTS.");
				}
					
			}
			
			if(g_set_info.bRelease1 == DEBUG_ENABLE)
			{
				strcat(title,"����DEBUGģʽ.");
			}
			else
			{
				if(g_set_info.bNewUart4Alarm & UT_LOCK)
				{
					strcat(title,"�п���.");
				}
				if(g_set_info.bNewUart4Alarm & UT_TTS)
				{
					strcat(title,"TTS.");
				}
				if(g_set_info.bNewUart4Alarm & UT_LED)
				{
					strcat(title,"��Դ���ڵ�LED.");
				}
				if(g_set_info.bNewUart4Alarm & UT_HBLED)
				{
					strcat(title,"��ɽ����LED.");
				}
				if(g_set_info.bNewUart4Alarm & UT_XGCAMERA)
				{
					strcat(title,"�ǹ�����ͷ.");
				}
				if(g_set_info.bNewUart4Alarm & UT_QQCAMERA)
				{
					strcat(title,"������ľ����ͷ.");
				}
				if(g_set_info.bNewUart4Alarm & UT_PROLOCK)
				{					
					strcat(title,"�ع�����.");			
				}
				if(g_set_info.bNewUart4Alarm & UT_WATCH)
				{
					strcat(title,"͸���豸.");
				}
				if(g_set_info.bNewUart4Alarm &UT_DRIVE)
				{
					strcat(title,"���������.");
				}
				if(g_set_info.bNewUart4Alarm & UT_BUS)
				{
					strcat(title,"������վ��.");
				}
				if(g_set_info.bNewUart4Alarm & UT_GPS)
				{
					strcat(title,"�Ե���.");
					if(g_set_info.GpsDevType == 1)
					{
						strcat(title,"ZB������.");
					}
					else if(g_set_info.GpsDevType == 2)
					{
						strcat(title,"YH������.");
					}
						
				}	
				if(g_set_info.bNewUart4Alarm  == 0)
				{
					
					strcat(title,"����������");
				}
			
			}
			SG_DynMenu_Put_MsgBox (menuid, title);

			SG_Soft_Reset(5);
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}


/****************************************************************
  ��������SG_Proc_Set_Lock_Out_Level
  ��  ��  �������п�����̬��ƽ
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Set_Lock_Out_Level(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	char title[200] = "";
	int i;//,j;
	char level[10] = "";
	char temp = 0;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Lock_Out_Level: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	

	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
			SG_DynMenu_Put_InputBox (menuid, "�п������õ�ƽ:", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		strcpy(level,s1);
		
		if(strlen(level) < 5)
		{
			menuList->item->count = 0;
				
			SG_DynMenu_Put_MsgBox (menuid, "���볤�Ȳ���!");
				
			return 1;
		}
		for(i = 0;i < 5;i ++) // ����ֵ��Ϊ0��1ʱ��ʾ����
		{
			if(level[i] != '0' && level[i] != '1')
			{
				menuList->item->count = 0;
				
				SG_DynMenu_Put_MsgBox (menuid, "�������!");
				
				return 1;
			}
		}		
		SG_Set_Cal_CheckSum();
		g_set_info.alarmSet = 0;
		
		for(i = 0;i < 5;i ++)
		{
			if(level[i] == '1') // �趨�ĵ�ƽֵΪ1
			{
				temp |= 0x01<<(5-i);
			}
		}
		g_set_info.alarmSet |= temp;
		g_set_info.alarmSet = temp<< 24;
		SG_Set_Save();

		SG_Net_Disconnect();
		
		strcpy(title,"���óɹ�!");
		SG_DynMenu_Put_MsgBox (menuid, title);

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}

	return 1;
}


/****************************************************************
��������SG_Proc_SetCom_Deliver
���ܣ�LED �豸�˵��ص�����
˵����
���룺
���أ�
�޸ļ�¼������������,2006-8-7
****************************************************************/
static int SG_Proc_SetCom_LED(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_LED: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_LED)
			SG_DynMenu_Put_ChooseBox (menuid, "�ر���Դ���ڵ�LED��ʾ��?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "����Դ���ڵ�LED��ʾ��?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_LED))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_LED))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_LED;
				SG_DynMenu_Put_MsgBox (menuid, "��Դ���ڵ���ʾ��������!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_LED;
				SG_DynMenu_Put_MsgBox (menuid, "��Դ���ڵ���ʾ���ѹر�!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}



/****************************************************************
��������SG_Proc_SetCom_HBLED
���ܣ�LED �豸�˵��ص�����
˵����
���룺
���أ�
�޸ļ�¼������������,2006-8-7
****************************************************************/
static int SG_Proc_SetCom_HBLED(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_LED: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_HBLED)
			SG_DynMenu_Put_ChooseBox (menuid, "�رհ�ɽ����LED��ʾ��?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "�򿪰�ɽ����LED��ʾ��?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_HBLED))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_HBLED))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_HBLED;
				SG_DynMenu_Put_MsgBox (menuid, "��ɽ��������ʾ��������!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_HBLED;
				SG_DynMenu_Put_MsgBox (menuid, "��ɽ��������ʾ���ѹر�!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}





#ifdef _DUART
/****************************************************************
��������SG_Proc_SetCom_Deliver
���ܣ�LED �豸�˵��ص�����
˵����
���룺
���أ�
�޸ļ�¼������������,2006-8-7
****************************************************************/
static int SG_Proc_SetCom_LED2(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_LED2: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_LED2)
			SG_DynMenu_Put_ChooseBox (menuid, "�رմ���2��LED��ʾ��?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "�򿪴���2��LED��ʾ��?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_LED2))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_LED2))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;
				}
				
				g_state_info.bNewUart4Alarm|=UT_LED2;
				SG_DynMenu_Put_MsgBox (menuid, "����2����ʾ��������!");
				g_state_info.bRelease2 = LED2_ENABLE;
			}
			else
			{
				g_state_info.bNewUart4Alarm &=~UT_LED2;
				SG_DynMenu_Put_MsgBox (menuid, "����2����ʾ���ѹر�!");
				g_state_info.bRelease2 = HH_ENABLE;
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}


/****************************************************************
��������SG_Proc_SetCom_Deliver
���ܣ�TTS2 �豸�˵��ص�����
˵����
���룺
���أ�
�޸ļ�¼������������,2006-8-7
****************************************************************/
static int SG_Proc_SetCom_TTS2(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_LED2: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_TTS2)
			SG_DynMenu_Put_ChooseBox (menuid, "�رմ���2��TTS?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "�򿪴���2��TTS");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_TTS2))
			{

				if(SG_Ext_Dev_Enable_Judge(UT_TTS2))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;
				}


				g_state_info.bNewUart4Alarm|=UT_TTS2;
				SG_DynMenu_Put_MsgBox (menuid, "����2��TTS����!");
				g_state_info.bRelease2 = LED2_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_TTS2;
				SG_DynMenu_Put_MsgBox (menuid, "����2��TTS�ѹر�!");
				g_state_info.bRelease2 = HH_ENABLE;
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}
#endif




/****************************************************************
��������SG_Proc_SetCom_Deliver
���ܣ�͸���豸���ز˵��ص�����
˵����
���룺
���أ�
�޸ļ�¼������������,2006-8-7
****************************************************************/
static int SG_Proc_SetCom_Watch(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;

	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_Watch: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_WATCH)
			SG_DynMenu_Put_ChooseBox (menuid, "�ر�͸���豸?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "��͸���豸?");			

		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_WATCH))
			{
				if(g_state_info.bNewUart4Alarm != 0)
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "�ѿ��������豸�����ȹر������豸!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_WATCH;
				SG_DynMenu_Put_MsgBox (menuid, "͸���豸������!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_WATCH;
				SG_DynMenu_Put_MsgBox (menuid, "͸���豸�ѹر�!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}
}




/********************************************************************************
����͸���豸����
********************************************************************************/
static int SG_Proc_Watch_ID(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;

	SCI_TRACE_LOW( "SG_Proc_SetCom_ID: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(menuList->item->count < 1)
	{
		menuList->item->count = 1;			
		SG_DynMenu_Put_InputBox (menuid, "1 �Ƽ��� 2 ���ش�����", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		char value=-1;
		 
		menuList->item->count = 0;	
		
		value=atoi(s1);
		if(value != 1 && value != 2)
		{
			menuList->item->count = 0;
		
			SG_DynMenu_Put_MsgBox (menuid, "���ô���!");

		}
		SG_Set_Cal_CheckSum();
		g_set_info.watchid=value;
		SG_Set_Save();
		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");
		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	return 1;

}



/***********************************************************************
����͸���豸��ʱʱ��
*************************************************************************/
static int SG_Proc_SetCom_Time(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	char title[100]="";
	int var=0;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_Time: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		SG_DynMenu_Put_ChooseBox (menuid, "�Ƿ����ó�ʱʱ��?");
		
		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "���ò���(1~60):", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else

		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		var=atoi(s1);
		if((var>=1)&&(var<=60))
		{	
			SG_Set_Cal_CheckSum();
			g_set_info.nSetTime= var;
			SG_Set_Save();
			strcpy(title,"�����ò���:");
			sprintf(title+strlen(title),"%d",var);
			SG_DynMenu_Put_MsgBox(menuid,title);
		}
		else
			SG_DynMenu_Put_MsgBox (menuid, "�������ô���!");
			
		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	return 1;
}


/************************************************************************
����͸��ID
****************************************************************************/
static int SG_Proc_SetCom_ID(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char strmsg[60]={0};
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_ID: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(menuList->item->count < 1)
	{
		menuList->item->count = 1;			
		SG_DynMenu_Put_InputBox (menuid, "Ĭ��ID(0~255):", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		int value=-1;
		 
		menuList->item->count = 0;	
		
		value=atoi(s1);
		SG_Set_Cal_CheckSum();
		if(value>=255)
			g_set_info.devID=255;
		else if(value<0)
			g_set_info.devID=0;
		else
			g_set_info.devID=value;
		
		SG_Set_Save();
		sprintf(strmsg,"���óɹ�,�豸\r\nĬ��ID:0x%x",g_set_info.devID);
		SG_DynMenu_Put_MsgBox (menuid, strmsg);
		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	return 1;

}

/************************************************************************
����͸��������
****************************************************************************/
static int SG_Proc_SetCom_Baud(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	char title[100]="";
	int var=0;
	
	menuList = SG_DynMenu_Get_MenuList(gpMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_SetCom_Baud: %d\r\n", menuList->item->count);
	
	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;

	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
		
	//�״ε���
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		SG_DynMenu_Put_ChooseBox (menuid, "�Ƿ����ô��ڲ�����?");
		
		return 1;
	}
	//����ȷ��
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "���ò���:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//��ȡ����
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		var=atoi(s1);

		SG_Set_Cal_CheckSum();
		g_set_info.watchbd = var;
		SG_Set_Save();
		strcpy(title,"�����ò���:");
		sprintf(title+strlen(title),"%d",var);
		SG_DynMenu_Put_MsgBox(menuid,title);


		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}
	return 1;
}


/****************************************************************
  ��������SG_Proc_Set_Psw
  ��  ��  ��gprs�������ò˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Set_Psw(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Set_Psw: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
		menuList->item->count = 2;			
		SG_DynMenu_Put_InputBox (menuid, "����:", NULL, ENCRYPT_BUF, SG_GPRS_PSW_LEN, PURE_ENG, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sGprsPsw, s1);
		
		SG_Set_Save();
		
		SG_DynMenu_Put_MsgBox (menuid, "���óɹ�!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}

	return 1;
}


//��ʾ log ��Ϣ
static int SG_Proc_SysLog(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[600] = "";
	
	if (s1 != NULL || s2 != NULL)
		return 0;

	

	sprintf(title+strlen(title), "ϵͳ����1:R %d,GGA %d, RMC %d, A %d, V %d, H %d, P %d, ac %d ,de %d\r\n",
			g_set_info.nReConCnt,g_state_info.nGGACnt,g_state_info.nRMCCnt,g_state_info.nACnt,g_state_info.nVCnt,g_state_info.nHandupConCnt,g_state_info.nPorxyConCnt,g_state_info.nActiveCnt,g_state_info.nDeactiveCnt);	
	sprintf(title+strlen(title), "err:gps %d\r\n",g_state_info.nGPSErr);
	sprintf(title+strlen(title), "alarmset %x\r\nalarmstate %lx\r\nalarmenable %lx alarmRepeat %lx\r\n",g_set_info.alarmSet,g_state_info.alarmState,g_set_info.alarmEnable,g_set_info.alarmRepeat);
	sprintf(title+strlen(title), "reboot %d\r\nnormal reset %d\r\n",g_set_info.nResetCnt,g_set_info.nReConCnt);
	sprintf(title+strlen(title), "g_set_info length %d\r\n",sizeof(g_set_info));
	sprintf(title+strlen(title), "GPS��λ����ʱ��Ϊ:%d\r", g_set_info.nGpstTime);
	sprintf(title+strlen(title), "GPS���˾���Ϊ:%d\r", g_set_info.nGpstDist);
	sprintf(title+strlen(title), "ä�������ĵ�Ϊ:%d\r", MAX_LIST_NUM);
	sprintf(title+strlen(title), "�����Ϊ:%d \r", g_state_info.randtime);
	if(g_set_info.Heart_Switch)
		sprintf(title+strlen(title), "������ ���:%d \r", g_set_info.Heart_Interval);
	else
		sprintf(title+strlen(title), "������ ���:%d \r", g_set_info.Heart_Interval);
		

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}



//��ʾ log ��Ϣ
static int SG_Proc_SetCom_Check_It_Self(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[600] = "";
	int count = 0;

	if (s1 != NULL || s2 != NULL)
		return 0;

	sprintf(title+strlen(title), "Ext Device:\r\nbRelease1 %d brelease 2 %d\r\nbNewUart4Alarm %02x\r\n",g_set_info.bRelease1,g_set_info.bRelease2,g_set_info.bNewUart4Alarm);

	if(g_set_info.bNewUart4Alarm & (UT_QQCAMERA|UT_XGCAMERA))
	{
		sprintf(title+strlen(title), "����ͷ:��ر�־%x; ʵʱ0x%x ·,��� %d ; ����0x%x ·, ���%d\r\n"
			,g_set_info.nCameraAlarm, g_set_info.sg_camera[0].nCameraID,g_set_info.sg_camera[0].nInterval,g_set_info.sg_camera[1].nCameraID,g_set_info.sg_camera[1].nInterval) ;
	
		sprintf(title+strlen(title),"ͼƬ�����־: %x  ͼƬ�������� %d\r\n",g_set_info.bPictureSave,g_set_info.nPictureSaveIndex);
		sprintf(title+strlen(title),"�ļ�0-4����ͼƬ: %d %d %d %d %d\r\n",g_set_info.camera_save_list[0].pic_save,
			g_set_info.camera_save_list[1].pic_save,g_set_info.camera_save_list[2].pic_save,
			g_set_info.camera_save_list[3].pic_save,g_set_info.camera_save_list[4].pic_save);
		sprintf(title+strlen(title),"����ͷ��ǰ״̬: %d",CurrCamera.currst);
		sprintf(title+strlen(title),"���ߴ���: %d \r\n",g_state_info.nReConnect);
		count = SG_Send_Cal_Pic_Item();

		sprintf(title+strlen(title),"������ͼƬ������: %d",count);
		sprintf(title+strlen(title),"�Ѿ�����ͼƬ: %d",g_state_info.photo_catch_cnt);
	}

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}

/****************************************************************
  ��������SG_Proc_Psw_Super
  ��  ��  ����������˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  ���ӳ�������Ա����Ҳ���޸Ĺ���Ա���룬���ɷ壬2005-12-06
****************************************************************/
static int SG_Proc_Psw_Super(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW("SG_Proc_Psw_Super: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	//�״ε���
	if (menuList->item->count == 0)
	{
		if (menuList->item->data != NULL)
		{
			SCI_FREE(menuList->item->data);
			menuList->item->data = NULL;
		}

		menuList->item->count = 1;
		
		SG_DynMenu_Put_InputBox (menuid, "ԭ��������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 1;
	}
	//��ȡԭ������Ϣ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) != 0)
		{
			menuList->item->count = 0;
		
			SG_DynMenu_Put_MsgBox (menuid, "�����������!");
		}
		else
		{
			menuList->item->count = 2;

			SG_DynMenu_Put_InputBox (menuid, "�³�������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
		}

		return 1;
	}
	//��ȡ��������Ϣ
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 3;
		menuList->item->data = SCI_ALLOC(12);
		SCI_MEMSET(menuList->item->data,0,12);
		strcpy(menuList->item->data,s1);
		SG_DynMenu_Put_InputBox (menuid, "�ٴ���������:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 1;
	}
	//�ٴλ�ȡ��������Ϣ
	else if ((menuList->item->count == 3) && (s1 != NULL) && (menuList->item->data != NULL))
	{
		menuList->item->count = 0;
	
		if (strcmp(menuList->item->data, s1) != 0)
		{
			SG_DynMenu_Put_MsgBox (menuid, "���벻ƥ��!");
		}
		else
		{
			SG_Set_Cal_CheckSum();
			//��������
			strcpy((char *)g_set_info.sSuperPsw,(const char*)s1);
			
			SG_Set_Save();
			
			SG_DynMenu_Put_MsgBox (menuid, "�����޸ĳɹ�!");
		}
		
		SCI_FREE(menuList->item->data);
		menuList->item->data = NULL;

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");

		return 1;
	}		
}



/****************************************************************
  ��������SG_Proc_Reset_Default
  ��  ��  ���ָ������˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Reset(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Reset_Default: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
			menuList->item->count = 2;			
			SG_DynMenu_Put_ChooseBox (menuid, "����ϵͳ?");
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			SG_DynMenu_Put_MsgBox (menuid, "������...");
			SG_Soft_Reset(2);
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		
	return 1;
}


/****************************************************************
  ��������SG_Proc_Reset_Default
  ��  ��  ���ָ������˵��ص�����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
static int SG_Proc_Reset_Default(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	MENULIST *menuList = NULL;
	
	menuList = SG_DynMenu_Get_MenuList(gpSetMenuList, menuid);
	if (menuList == NULL)
		return 0;
		
	SCI_TRACE_LOW( "SG_Proc_Reset_Default: %d\r\n", menuList->item->count);

	if (menuList->item->count == 0 && (s1 != NULL || s2 != NULL))
		return 0;
	
	if (s1 == NULL && s2 == NULL)	
		menuList->item->count = 0;
	
	if(SG_Check_Superpwd(menuList,(char*)menuid,s1) == 0)
		return 1;
	
	if(menuList->item->count < 2)
	{
			menuList->item->count = 2;			
			SG_DynMenu_Put_ChooseBox (menuid, "�ָ���������?");
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			SG_DynMenu_Put_MsgBox (menuid, "�������óɹ�!");
			SG_Set_Reset();
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "�������!");
	}		
	return 1;
}

/****************************************************************
  ��������SG_Net_DynamicMenu_Snd_Msg
  ��  ��  ����̬�˵�������Ϣ����
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
int  SG_Net_DynamicMenu_Snd_Msg (unsigned char *msg, int len)
{
	int i;

	g_sendframe.type = 0x01;
	g_sendframe.len = len;
	SCI_MEMCPY(g_sendframe.dat,msg,len);

#if (0)
	{
		char *p = (char*)&g_sendframe;
	for(i = 0; i< len ;i++)
 	{
			SCI_TRACE_LOW("SEND MSG<<%d,%02x>>",i,p[i]);
	 	}
 	}
#endif /* (0) */
	
	SendFrame(&g_sendframe);
	
 
	return 1;
 }


/****************************************************************
  ��������SG_Init_MenuList
  ��  ��  �������˵��б�
  ���������
  ���������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
  	������ͳ�Ʋ˵����º�����2006-5-23
****************************************************************/
int SG_Init_MenuList ()
{
	MENUITEM *pMenuItem = NULL;
	MENULIST *pMenuList = NULL, *father = NULL;
	int nMenuId = FIRST_MENUID;
//	int i=0;
/*
�������ܲ˵�

	|--ģ����	|--�汾��ѯ
	|			|--����״̬
	|			|--������ϲ�ѯ
	|                                 
	|--��װ����	|--IO������״̬���(δ���ñ�����ʱ���Խ���)
	|         	|--Ϩ��
	|			|--Ϩ��ָ�
	|
	|--������չ	|--͸���豸		|--����LED��ʾ������
	|
	|        	|--�п���		|--�����п�������
	|                        		|--�п�����̬��ƽ����
	|          	|--����TTS����
	|			|--����DEBUG
	|			|--���õ�ǰ����
*/

	pMenuItem = SCI_ALLOC(sizeof(MENUITEM));
	SCI_MEMSET(pMenuItem,0,sizeof(MENUITEM));
	if (pMenuItem == NULL)
	{
		SCI_TRACE_LOW( "SG_Init_MenuList: CALLOC: %d\r\n", sizeof(MENUITEM));	
		return 0;
	}	
	
	pMenuList = SCI_ALLOC(sizeof(MENULIST));
	SCI_MEMSET(pMenuList,0,sizeof(MENULIST));	
	if (pMenuList == NULL)
	{
		SCI_FREE(pMenuItem);
		SCI_TRACE_LOW("SG_Init_MenuList: CALLOC: %d\r\n", sizeof(MENULIST));	
		return 0;
	}	

	gpMenuList = pMenuList;

	// {ģ����
	pMenuList->item = pMenuItem;
	SCI_MEMCPY(pMenuItem->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuItem->s = "�汾��ѯ";  //  �鿴�汾��
	pMenuItem->proc = SG_Proc_Ver_Ask;


//��װ����

//	|--��װ����		|
//	|				|
//	|				|--IO������״̬���
//	|				|
//	|				|--Ϩ��
//	|				|
//	|				|--Ϩ��ָ�
//	|				|


	

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��װ����";
	pMenuList->item->proc = NULL;
	
	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "IO������״̬���";
	pMenuList->item->proc =SG_Proc_IO_Check_State;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�Զ���1������ƽ";
	pMenuList->item->proc = SG_Proc_Set_IO_level;

#ifndef _TRACK
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�Ƽ����봥����ƽ";
	pMenuList->item->proc = SG_Proc_Set_JJQ_Lev;

	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���üƼ����븴��";
	pMenuList->item->proc = SG_Proc_Set_Battery_Check;
#endif



	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("�ֱ��������л�");
	pMenuList->item->proc = SG_Proc_AutoAnswer;
	
	#ifdef _DUART // ���谲װ����
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("LED��������");
	pMenuList->item->proc = SG_Proc_Test_LED;

	pMenuList->next = SCI_ALLOC(  sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC(  sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�п���������";
	pMenuList->item->proc = NULL;
//{
	father = pMenuList;

	pMenuList->child = SCI_ALLOC(  sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC(  sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����";
	pMenuList->item->proc =SG_Proc_Test_OpenLock;

	pMenuList->next = SCI_ALLOC(  sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC(  sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����";
	pMenuList->item->proc = SG_Proc_Test_CloseLock;

	pMenuList = father;
	father = pMenuList->father;

	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("TTS���õ���");
	pMenuList->item->proc = SG_Proc_Test_TTS;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("����AD ֵ");
	pMenuList->item->proc = SG_Proc_Check_AD;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("ͼ������");
	pMenuList->item->proc = SG_Proc_Pictrue_Catch;

	
#endif 


	if( 0 == strcmp (g_set_info.sOwnNo,"")  )
	{
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "Ϩ��";
	pMenuList->item->proc = SG_Proc_Set_XIH_Off;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "Ϩ��ָ�";
	pMenuList->item->proc = SG_Proc_Set_XIH_On;
	}


    pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("�汾�л�");
	pMenuList->item->proc = SG_Proc_Ver;


	pMenuList = father;	
	father = pMenuList->father;




// ��װ����end

//ģ����{
#if (__cwt_)
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�Ա������";
	pMenuList->item->proc = SG_Proc_Edit_SMS;
#endif /* (0) */

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ģ����";
	pMenuList->item->proc = NULL;


	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����״̬";
	pMenuList->item->proc =SG_Proc_Check_Device;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "������ϲ�ѯ";
	pMenuList->item->proc = SG_Proc_Net_State;
	pMenuList = father;	
	father = pMenuList->father;
//}ģ����˵�����

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��������";
	pMenuList->item->proc = SG_Proc_Theft;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��ݲ���";
	pMenuList->item->proc = NULL;

//	{ 	��ݲ���

	father = pMenuList;


	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��������";
	pMenuList->item->proc = SG_Proc_Hot_Service;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("ҽ������");
	pMenuList->item->proc = SG_Proc_Medical_Service;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("ά������");
	pMenuList->item->proc = SG_Proc_Help_Service;
	
	pMenuList = father;


//}��ݲ��Ž���

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("λ�ò�ѯ");
	pMenuList->item->proc = NULL;

//	{	λ�ò�ѯ

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("GPS��Ϣ");
	pMenuList->item->proc = SG_Proc_GPS_Info;


	pMenuList = father;

//	}	λ�ò�ѯ

//   {	���ͳ�Ʋ˵���begin
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("���ͳ��");
	pMenuList->item->proc = NULL;

//   		{	�Ӳ˵���1�㣭begin
	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("�����ͳ��");
	pMenuList->item->proc = SG_Proc_Cacular_Total_Distance;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("���1ͳ��");
	pMenuList->item->proc = SG_Proc_Cacular_Distance1;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("���2ͳ��");
	pMenuList->item->proc = SG_Proc_Cacular_Distance2;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("���1����");
	pMenuList->item->proc = SG_Proc_Clr_Distance1;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("���2����");
	pMenuList->item->proc = SG_Proc_Clr_Distance2;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("���������");
	pMenuList->item->proc = SG_Proc_Set_Total_Distance;	
//   		}	�Ӳ˵���1�㣭end

	pMenuList = father;
//   }	���ͳ�Ʋ˵���end

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("ͨ������");
	pMenuList->item->proc = NULL;
	
	

//		{	ͨ������

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("��������");
	pMenuList->item->proc = SG_Proc_Call_In;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);

	pMenuList->item->s = ("��������");
	pMenuList->item->proc = SG_Proc_Call_Out;

	if (g_state_info.user_type == PRIVATE_CAR_VER)
	{
		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = "����һ����������";
		pMenuList->item->proc = SG_Proc_Set_OneGPS;

		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = "���ű�������1";
		pMenuList->item->proc = SG_Proc_Set_AlarmSms;

		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = "���ű�������2";
		pMenuList->item->proc = SG_Proc_Set_AlarmSms2;

		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->father = father;
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = ("�Զ�����");
		pMenuList->item->proc = SG_Proc_Auto_Call;
	}

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("ͨ����������");
	pMenuList->item->proc = SG_Proc_SpeakVolChg;

	pMenuList = father;	

	father = pMenuList -> father;

//		}	ͨ������

	
// ������Ϣ��ѯ
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "������Ϣ��ѯ";
	pMenuList->item->proc = SG_Proc_Muster_Particular_Ask;


// ������չ{
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "������չ";
	pMenuList->item->proc = NULL;

	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "͸���豸";
	pMenuList->item->proc = NULL;

	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���ÿ���";
	pMenuList->item->proc = SG_Proc_SetCom_Watch;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����͸��ID";
	pMenuList->item->proc = SG_Proc_SetCom_ID;

#if (1)
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����͸��������";
	pMenuList->item->proc = SG_Proc_SetCom_Baud;
#endif /* (0) */


	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���ó�ʱʱ��";
	pMenuList->item->proc = SG_Proc_SetCom_Time;
	
	pMenuList = father;	
	father = pMenuList->father;


	#ifdef _DUART
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��Դ���ڵ�LED";
	pMenuList->item->proc = SG_Proc_SetCom_LED;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��ɽ����LED";
	pMenuList->item->proc = SG_Proc_SetCom_HBLED;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����2��LED";
	pMenuList->item->proc = SG_Proc_SetCom_LED2;
	#else
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����LED��ʾ��";
	pMenuList->item->proc = SG_Proc_SetCom_LED;	
	#endif

	

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�п���";
	pMenuList->item->proc = NULL;

	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�����п���";
	pMenuList->item->proc = SG_Proc_SetCom_Lock;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�п�����̬��ƽ����";
	pMenuList->item->proc = SG_Proc_Set_Lock_Out_Level;
	
	pMenuList = father;	
	father = pMenuList->father;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����TTS";
	pMenuList->item->proc = SG_Proc_SetCom_TTS;

#ifdef _DUART
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���ش���2��TTS";
	pMenuList->item->proc = SG_Proc_SetCom_TTS2;
#endif
	
if (g_state_info.user_type == PRIVATE_CAR_VER)
{
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�����Ե���";
	pMenuList->item->proc = SG_Proc_SetCom_GPS;
}

	
#if (0)
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�ǹ�����ͷ";
	pMenuList->item->proc = SG_Proc_SetCom_XgVideo;
#endif /* (0) */

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "������ľ����ͷ";
	pMenuList->item->proc = SG_Proc_SetCom_QqzmVideo;
	

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���������";
	pMenuList->item->proc = SG_Proc_SetCom_Drive;
	
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����DEBUG";
	pMenuList->item->proc = SG_Proc_SetCom_Debug;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���õ�ǰ����";
	pMenuList->item->proc = SG_Proc_SetCom_Start;	
	
	#ifdef _DUART
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�����豸�鿴";
	pMenuList->item->proc = SG_Proc_SetCom_Check_It_Self;	
	#endif
	
	pMenuList = father;	
	father = pMenuList->father;
//������չend
	
	

/*
�ն��������ز˵�

	|--���ò�ѯ
	|--��������
	|--���ĺ���
	|--ͨѶ��ʽ
	|--���ĵ�ַ
	|--���Ķ˿�
	|--GPRS����  	|--�����ַ
				|--����˿�
				|--����ʹ��
				|--APN
				|--�û���
				|--����
	|--ϵͳLOG
	|--�ָ�����
	|--����ϵͳ
	|--�˳�����
	
*/
	pMenuItem = NULL;
	pMenuList = NULL;
	father = NULL;
	nMenuId = FIRST_MENUID;


	pMenuItem = SCI_ALLOC( sizeof(MENUITEM));
	if (pMenuItem == NULL)
	{
		SCI_TRACE_LOW( "SG_Init_MenuList: CALLOC: %d\r\n", sizeof(MENUITEM));	
		return 0;
	}	
	
	pMenuList = SCI_ALLOC( sizeof(MENULIST));
	if (pMenuList == NULL)
	{
		SCI_FREE (pMenuItem);
		SCI_TRACE_LOW( "SG_Init_MenuList: CALLOC: %d\r\n", sizeof(MENULIST));	
		return 0;
	}	

	gpSetMenuList = pMenuList;

	pMenuList->item = pMenuItem;
	memcpy(pMenuItem->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���ò�ѯ";
	pMenuList->item->proc = SG_Proc_Show_Device;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��������";
	pMenuList->item->proc = SG_Proc_Set_OwnNo;
	#if (__cwt_)
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��Ȩ��";
	pMenuList->item->proc = SG_Proc_Set_OemCode;
	#endif
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���ĺ���";
	pMenuList->item->proc = SG_Proc_Set_CenterNo;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ͨѶ��ʽ";
	pMenuList->item->proc = SG_Proc_Set_Communication_Type;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���ĵ�ַ";
	pMenuList->item->proc = SG_Proc_Set_CenterIp;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���Ķ˿�";
	pMenuList->item->proc = SG_Proc_Set_CenterPort;

//GPRS ����
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "GPRS����";
	pMenuList->item->proc = NULL;

	father = pMenuList;
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����ʹ��";
	pMenuList->item->proc = SG_Proc_Set_ProxyEnable;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList->next->father = father;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�����ַ";
	pMenuList->item->proc = SG_Proc_Set_ProxyIp;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����˿�";
	pMenuList->item->proc = SG_Proc_Set_ProxyPort;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList->next->father = father;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "APN";
	pMenuList->item->proc = SG_Proc_Set_APN;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList->next->father = father;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�û���";
	pMenuList->item->proc = SG_Proc_Set_User;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList->next->father = father;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����";
	pMenuList->item->proc = SG_Proc_Set_Psw;

	pMenuList = father;
	father = pMenuList->father;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�������";
	pMenuList->item->proc = NULL;

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��ط�ʽ";
	pMenuList->item->proc = SG_Proc_Set_Watch_Type;

	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��ؼ��ʱ��";
	pMenuList->item->proc = SG_Proc_Set_Watch_Interval;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "GPS����ʱ��";
	pMenuList->item->proc = SG_Proc_Set_GPS_Interval;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���ͳ����С����";
	pMenuList->item->proc = SG_Proc_Set_GPS_Dist;

    pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ä�������������";
	pMenuList->item->proc = SG_Proc_Set_Efs_Max;

	pMenuList = father;
	father = pMenuList->father;


	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��������";
	pMenuList->item->proc = NULL;

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "��������";
	pMenuList->item->proc = SG_Proc_Set_Heart_Switch;

	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�������ʱ��";
	pMenuList->item->proc = SG_Proc_Set_Heart_Interval;

	pMenuList = father;
	father = pMenuList->father;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ϵͳLOG";
	pMenuList->item->proc = SG_Proc_SysLog;

#if (0) // ��ʱ���γ��������޸�
// ���������޸�
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "���������޸�";
	pMenuList->item->proc = SG_Proc_Psw_Super;

#endif /* (0) */

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�ָ�����";
	pMenuList->item->proc = SG_Proc_Reset_Default;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "����ϵͳ";
	pMenuList->item->proc = SG_Proc_Reset;


	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "�˳�����";
	pMenuList->item->proc = SG_Proc_Set_Exit;

//   }	���ò˵���end

	SCI_TRACE_LOW("SG_Init_MenuList \r\n");
	return 1;
}
/****************************************************************
  ��������SG_Menu_Set_MenuList
  ��  ��  �����õ�ǰ�˵�
  ���������type: 0:���˵� 1:���ò˵�
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-11-27
****************************************************************/
void SG_Menu_Set_MenuList(int type)
{
	if (type == 0)
	{
		gpCurMenuList = gpMenuList;
		SCI_TRACE_LOW("SG_Menu_Set_MenuList: USE MAIN MENU\r\n");		
	}	
	else if (type == 1)
	{
		gpCurMenuList = gpSetMenuList;
		SCI_TRACE_LOW("SG_Menu_Set_MenuList: USE SET MENU\r\n");				
	}	
	else
	{
		gpCurMenuList = NULL;
		SCI_TRACE_LOW("SG_Menu_Set_MenuList: USE NULL MENU\r\n");						
	}	
}

/****************************************************************
  ��������SG_Menu_Get_MenuList
  ��  ��  ����ȡ��ǰ�˵�
  �����������
  �����������
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-11-27
****************************************************************/
void* SG_Menu_Get_MenuList()
{
	return (void*)gpCurMenuList;
}
