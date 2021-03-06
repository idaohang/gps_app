/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_Menu.c
  版本   ：1.00
  创建者 ：李松峰
  创建时间：2005-9-23
  内容描述：SG2000 手柄菜单功能处理 
  修改记录：
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


// 用于判断超级密码是否通过
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
	
	//首次调用
	if (menuList->item->count == 0)
	{
		if (menuList->item->data != NULL)
		{
			SCI_FREE(menuList->item->data);
			menuList->item->data = NULL;
		}

		menuList->item->count = 1;
		
		SG_DynMenu_Put_InputBox ((unsigned char*)menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 0;
	}
	//获取密码信息
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{	
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) != 0)
		{
			menuList->item->count = 0;			
			SG_DynMenu_Put_MsgBox ((unsigned char*)menuid, "超级密码错误!");
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
/****************************************************************重工锁车****************************************************************/
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
	//首次调用	
	if (menuList->item->count == 0)	
	{		
		menuList->item->count = 1;		
		if(g_state_info.bNewUart4Alarm&UT_PROLOCK)			
			SG_DynMenu_Put_ChooseBox (menuid, "关闭重工锁车�?");		
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "打开重工锁车? ");					
		return 1;	
	}	
	//操作确认	
	else if ((menuList->item->count == 1) && (s1 != NULL))	
	{		
		if (*(char*)s1 == INPUT_CHOOSE_YES)		
		{			
			menuList->item->count = 2;					
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);					
			return 1;		
		}		
		else		
		{			
			menuList->item->count = 0;			
			return 0;		
		}	
	}	
	//获取密码	
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
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");					
					return 1;				
				}						
				
				g_state_info.bNewUart4Alarm|=UT_PROLOCK;				
				SG_DynMenu_Put_MsgBox (menuid, "重工锁车打开!");			
				g_state_info.bRelease1 = DEV_ENABLE;
			}			
			else			
			{				
				g_state_info.bNewUart4Alarm&=~UT_PROLOCK;				
				SG_DynMenu_Put_MsgBox (menuid, "重工锁车关闭!");
				g_state_info.bRelease1 = 0;

			}	
		}		
		else		
		{			
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");		
		}		
		return 1;
	}	
	else
	{		
		menuList->item->count = 0;				
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");		
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_MINI)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭MINI?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开MINI?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_MINI;
				SG_DynMenu_Put_MsgBox (menuid, "MINI打开!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_MINI;
				SG_DynMenu_Put_MsgBox (menuid, "MINI关闭!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}
#endif /* (0) */
/****************************************************************
星光摄像头
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_XGCAMERA)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭摄像头?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开摄像头?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_XGCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "摄像头打开!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_XGCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "摄像头关闭!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}

/****************************************************************
青青子木摄像头
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_QQCAMERA)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭摄像头?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开摄像头?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_QQCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "摄像头打开!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_QQCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "摄像头关闭!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}

/****************************************************************
驾培管理器
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_set_info.bNewUart4Alarm&UT_DRIVE)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭驾培管理器?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开驾培管理器?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_DRIVE;
				SG_DynMenu_Put_MsgBox (menuid, "驾培管理器打开!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_DRIVE;
				SG_DynMenu_Put_MsgBox (menuid, "驾培管理器关闭!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}

/****************************************************************
函数名：SG_Proc_SetCom_Bus_Dev
功能：串口扩展公交设备菜单回调函数
说明：
输入：
返回：
修改记录：创建，陈海华,2006-7-5
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_BUS)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭公交报站器?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开公交报站器?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_BUS;
				SG_DynMenu_Put_MsgBox (menuid, "公交报站器打开!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_BUS;
				SG_DynMenu_Put_MsgBox (menuid, "公交报站器关闭!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}


/****************************************************************
自导航
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_GPS)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭自导航?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开自导航?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_GPS))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_GPS))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}

				SG_DynMenu_Put_InputBox (menuid, "打开类型:1~2", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
				menuList->item->count = 3;

			}
			else
			{
				menuList->item->count = 0;
				g_state_info.bNewUart4Alarm&=~UT_GPS;
				SG_DynMenu_Put_MsgBox (menuid, "自导航关闭!");
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
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
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
			
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

			return 1;
		}
		
		if(1 == type)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ZB导航屏打开!");
			g_state_info.GpsDevType = 1;
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "YH导航屏打开!");
			g_state_info.GpsDevType = 2;
		}
		g_state_info.bNewUart4Alarm|=UT_GPS;
		
		g_state_info.bRelease1 = DEV_ENABLE;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}

/****************************************************************
  函数名：SG_Proc_Ver_Ask
  功  能  ：版本查询菜单回调函数
  输入参数：
  输出参数：返回当前版本
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
static int SG_Proc_Ver_Ask(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	char title[300] = "";

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	// APP VERSION
	strcpy(title, "软件版本\r\n");
			
	strcat(title, g_set_info.sVersion);
				
	strcat(title, "\r\n");
	
	#ifdef _DUART
	strcat(title,"硬件型号:G01C\r\n");
#endif

#ifdef _SUART
#ifdef _TRACK
	strcat(title,"硬件型号:G01B\r\n");
#else
	strcat(title,"硬件版本:");
	sprintf(title+strlen(title),"%x\r\n",g_state_info.hardware_ver);
	strcat(title,"硬件型号:G01A\r\n");
#endif
#endif



	strcat(title,"用户类型:");

	if(g_state_info.user_type == COMMON_VER)
	{
		strcat(title,"普通版本\r\n");
	}
	else if(g_state_info.user_type == PRIVATE_CAR_VER)
	{
		strcat(title,"私家车版本\r\n");	
	}
	
	SG_DynMenu_Put_MsgBox (menuid, title);
				
	return 1;

}


/****************************************************************
  函数名：SG_Proc_Net_State
  功  能  ：网络故障查询回调函数
  输入参数：
  输出参数：返回 “GPS OK \r\nGSM VERSION xxx　\r\nAPP VERSION xxxx”
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  增加故障查询信息，李松峰，2005-12-09
  GPS定位查询改为检测当前状态，李松峰，2006-01-05
****************************************************************/
static int SG_Proc_Net_State(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	char title[200] = "";

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	// GPRS当前动作
	strcpy(title, "GPRS拨号阶段:\r\n");
	
	switch(g_gprs_t.state)
	{
		case GPRS_ACTIVE_PDP:
			strcat(title,"Active PDP上下文\n");
			break;
		case GPRS_DEACTIVE_PDP:
			strcat(title,"DeActive PDP上下文\n");
			break;	
		case GPRS_SOCKET_CONNECT:
			strcat(title,"连接SOCKET\n");
			break;
		case GPRS_PORXY:
			strcat(title,"连接代理\n");
			break;
		case GPRS_HANDUP:
			strcat(title,"握手\n");
			break;
		case GPRS_RXHEAD:
		case GPRS_RXCONTENT:
			strcat(title,"链接中心正常\n");
			break;
		case GPRS_DISCONNECT:
			strcat(title,"连接断开\n");
			break;
		default:
			break;
			
	}
	
	// 上次发生的错误
	strcat(title,"\r最近错误:\r\n");
	
	switch(g_state_info.lasterr)
	{
		case SG_ERR_NONE:
			strcat(title,"未发生错误");
			break;
		case SG_ERR_CREAT_SOCKET:
			strcat(title,"建立Socket");
			break;
		case SG_ERR_PROXY_CONNET:
			strcat(title,"连接Proxy");
			break;
		case SG_ERR_PROXY_NOTFOUND:
			strcat(title,"连接中心");
			break;
		case SG_ERR_PROXY_DENY:
			strcat(title,"Proxy 拒绝");
			break;
		case SG_ERR_CONNET_CENTER:
			strcat(title,"中心错误");
			break;
		case SG_ERR_RCV_HANDUP:
			strcat(title, "握手应答");
			break;
		case SG_ERR_RCV_DATA:
			strcat(title,"接收数据");
			break;
		case SG_ERR_SEND_DATA:
			strcat(title, "发送数据");
			break;
		case SG_ERR_RCV_ANS:
			strcat(title, "接收应答");
			break;
		case SG_ERR_TIME_OUT:
			strcat(title, "超时错误，\n");
			switch(g_state_info.nNetTimeOut)
			{
				case GPRS_ACTIVE_PDP:
					strcat(title,"Active PDP上下文");
					break;
				case GPRS_DEACTIVE_PDP:
					strcat(title,"DeActive PDP上下文");
					break;	
				case GPRS_PORXY:
					strcat(title,"连接代理");
					break;
				case GPRS_HANDUP:
					strcat(title,"握手");
					break;
				case GPRS_RXHEAD:
				case GPRS_RXCONTENT:
					strcat(title,"接收报文内容");
					break;
				case GPRS_SOCKET_CONNECT:
					strcat(title,"连接SOCKET");
					break;
			}
			strcat(title, "超时!\r\n");
			break;
			
		case SG_ERR_CLOSE_SOCKET:
			strcat(title, "关闭SOCKET");
			break;
	
		case SG_ERR_HOST_PARSE:
			strcat(title, "主机地址解析");
			break;
		case SG_ERR_ACTIVE_PDP:
			strcat(title, "激活PDP上下文");
			break;
		case SG_ERR_DEACTIVE_PDP:
			strcat(title, "去活PDP上下文");
			break;
		case SG_ERR_SET_OPT:
			strcat(title, "参数设置");
			break;
		default:
			break;
	}

	strcat(title, "\r\n");

	// 当前累计重连次数
	sprintf(title+strlen(title), "累计重连次数:%ld次\r\n",ReConCnt);
	sprintf(title+strlen(title), "网络状态:%d\r\n",g_state_info.plmn_status);
	

	SG_DynMenu_Put_MsgBox (menuid, title);
				
	return 1;

}




/****************************************************************
  函数名：SG_Proc_Check_Device
  功  能  ：车辆状态菜单回调函数
  输入参数：
  输出参数：返回 “GPS OK \r\nGSM VERSION xxx　\r\nAPP VERSION xxxx”
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  增加故障查询信息，李松峰，2005-12-09
  GPS定位查询改为检测当前状态，李松峰，2006-01-05
****************************************************************/
static int SG_Proc_Check_Device(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	char title[500] = "";
	char net_status = 0;

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	// ＧＰＳ模块候选状态：已定位　正常　异常
	if(((gps_data *)(g_state_info.pGpsCurrent))->status)// 已经定位
	{
		strcpy(title, "GPS 已定位!\r\n");
	}
	else
	{
		strcpy(title, "GPS 未定位!\r\n");
	}

	sprintf(title+strlen(title), "当前星数: %d\r\n",((gps_data *)(g_state_info.pGpsCurrent))->nNum);
	
					
	if (g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF )
		strcat(title, "主电平掉电!\r");

	else if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE )
		strcat(title, "主电平欠压!\r");
		
	strcat(title, "车辆状态:  ");
	
	if(g_state_info.alarmState & ~(GPS_CAR_STATU_POS|GPS_CAR_STATU_ACC))
	{
		sprintf(title+strlen(title), "0x%08x\r",(int)g_state_info.alarmState);
	}
	else
		strcat(title, "正常.\r");

	if(TRUE == SG_Set_Check_CheckSum()){
		strcat(title, "配置正常\r");
	}
	else{
		strcat(title, "配置异常\r");
	}

	if(g_set_info.bAutoAnswerDisable)
	{
		strcat(title,"使用手柄通话\r");
	}
	else
	{
		strcat(title,"使用免提通话\r");
	}



	if (g_set_info.bRegisted == 0)
		strcat(title, "终端未注册!\r");
	
	sprintf(title+strlen(title), "累计重启次数:\r\nAll %d次\r\nFormal %d次\r\n",g_set_info.nResetCnt,g_set_info.nReConCnt);
	sprintf(title+strlen(title), "上次重启原因 %d\r\n",Restart_Reason);
	sprintf(title+strlen(title), "运营商 %d\r\n信号 %d\r\n",g_state_info.opter,g_state_info.rxLevel);
	strcat(title, "IMEI:");
	strcat(title, g_state_info.IMEI);
	strcat(title, "\r\n");
	//sprintf(title+strlen(title), "IMSI %s\r\n",g_state_info.imsi);
	
	net_status = SG_Net_Judge();

	switch(net_status)
	{
		case NO_SIM_CARD:
			strcat(title,"无SIM卡\r\n");
			break;
		case NO_NET_WORK:
			strcat(title,"无网络\r\n");
			break;
		case GSM_ONLY:
			strcat(title,"ONLY GSM\r\n");	
			break;
		case GPRS_ONLY:
			strcat(title,"ONLY GPRS\r\n");	
			break;
		case NETWORK_OK:
			strcat(title,"网络正常\r\n");	
			break;
		default:
			break;
	}

	if(g_set_info.bNewUart4Alarm & UT_GPS)
	{
		if(g_set_info.GpsDevType == 1)
		{
			
			strcat(title,"启用ZB导航屏");
		}
		else if(g_set_info.GpsDevType == 2)
		{
			
			strcat(title,"启用YH导航屏");			
		}
	}
#if (0)
	{
		int space;
		space = EFS_GetFreeSpace();
		sprintf(title+strlen(title),"文件系统剩余空间 %d,",space);
		space = EFS_GetUsedSpace();
		sprintf(title+strlen(title),"已经使用空间 %d\r\n",space);
	}
#endif /* (0) */

	SG_DynMenu_Put_MsgBox (menuid, title);
				
	return 1;

}



/****************************************************************
  函数名：SG_Proc_Theft
  功  能  ：防盗功能菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  汽车点火时禁止 打开防盗功能，李松峰，2005-12-15
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
		
	//首次调用
	if (menuList->item->count < 2)
	{
		menuList->item->count = 2;

		if (g_set_info.nTheftState == 0)
		{
					
			if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
			{
				menuList->item->count = 0;			
				SG_DynMenu_Put_MsgBox (menuid, "请先熄火再使用防盗功能!");	
				return 1;

			}
		
			if (g_set_info.bTheftDisable == 1)
			{
				menuList->item->count = 0;			
				SG_DynMenu_Put_MsgBox (menuid, "防盗功能被禁用，请联系中心开通!");	
				return 1;
			}	
			
			 if(g_set_info.bTheftDisable == 0)
				SG_DynMenu_Put_ChooseBox (menuid, "开启点火防盗功能?");
				
		}	
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "关闭防盗功能?");

		return 1;
	}

	//操作确认
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
					SG_DynMenu_Put_ChooseBox (menuid, "成功打开点火防盗功能");
				}	
			}	
			else
			{
				if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
				{
					menuList->item->count = 0;			
					SG_DynMenu_Put_MsgBox (menuid, "请先熄火再使用防盗功能!");	
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
				SG_DynMenu_Put_MsgBox (menuid, "防盗成功关闭!");			

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
  函数名：SG_Proc_Medical_Service
  功  能  ：医疗服务菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  在菜单中发送任意时间出现的菜单类型时需要先发空菜单，李松峰，2005-12-08
****************************************************************/
static int SG_Proc_Medical_Service(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	if (s1 != NULL || s2 != NULL)
		return 0;

//	SG_DynMenu_Put_MenuList (NULL);							

	SG_DynMenu_Put_VoiceBox(menuid, "拨打医疗求助号码?",  g_set_info.sMedicalNo,  "医疗求助");
	
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Help_Service
  功  能  ：维修服务菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  在菜单中发送任意时间出现的菜单类型时需要先发空菜单，李松峰，2005-12-08  
****************************************************************/
static int SG_Proc_Help_Service(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	if (s1 != NULL || s2 != NULL)
		return 0;
	
//	SG_DynMenu_Put_MenuList (NULL);							
	SG_DynMenu_Put_VoiceBox(menuid, "拨打维修求助号码?",  g_set_info.sServiceNo,  "维修求助");


	
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Hot_Service
  功  能  ：服务热线菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  在菜单中发送任意时间出现的菜单类型时需要先发空菜单，李松峰，2005-12-08
****************************************************************/
static int SG_Proc_Hot_Service(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	if (s1 != NULL || s2 != NULL)
		return 0;
	
//	SG_DynMenu_Put_MenuList (NULL);							
	SG_DynMenu_Put_VoiceBox(menuid, "拨打服务热线?",  g_set_info.sHelpNo,  "服务热线");


	return 1;
}


/****************************************************************
  函数名：SG_Proc_GPS_Info
  功  能  ：GPS信息查询菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  增加当前无定位提示，显示最近定位信息，
  李松峰，2005-12-02
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
		strcpy(title, "当前GPS无定位\r最近定位信息:\r\n");
	}	
	else
	{
		strcpy(title,"GPS已定位\r当前定位信息:\r\n");
	}
	strcat(title, "gps 速度:");
	sscanf(gps->sSpeed,"%f",&m);
	m = m*1.852;
	sprintf(buf,"%0.2f",m);
	strcat(title,buf);
	strcat(title, " km/h\r\n");

	strcat(title, "可见星数:");
	sprintf(title+strlen(title), "%d 颗", gps->nNum);
	strcat(title, "\r\n");

	
	strcat(title, "角度:");
	strcat(title, gps->sAngle);
	strcat(title, "\r\n");

	strcat(title, "经度:");
	strcat(title, gps->sLongitude);
	strcat(title, gps->sSouthNorth);
	strcat(title, "\r\n");
	
	strcat(title, "纬度:");
	strcat(title, gps->sLatitude);
	strcat(title, gps->sEastWest);
	strcat(title, "\r\n");


	strcat(title, "高度:");
	strcat(title, gps->sAltitude);
	strcat(title, " 米\r\n");


	strcat(title, "UTC时间:");
	strcat(title, gps->sTime);
	strcat(title, "\r\n");

	strcat(title, "UTC日期:");
	strcat(title, gps->sDate);
	strcat(title, "\r\n");
	
	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}


/****************************************************************
  函数名：SG_Proc_Cacular_Total_Distance
  功  能  ：总里程统计
  输入参数：
  输出参数：
  编写者  ：陈海华
  修改记录：创建，2006-5-23
****************************************************************/
static int SG_Proc_Cacular_Total_Distance(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;
	sprintf(str,"%.3f公里\r\n",(double)g_state_info.nGetTotalDistance/1000);	

	SG_DynMenu_Put_MsgBox (menuid, str);
	
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_Total_Distance
  功  能  ：总里程统计设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
		SG_DynMenu_Put_InputBox (menuid, "总里程:", NULL, DECRYPT_BUF, 9, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		g_watch_info.nGetTotalDistance = atol(s1);
		g_state_info.nGetTotalDistance = g_watch_info.nGetTotalDistance;
		
		SG_Watch_Save();

		SG_Net_Disconnect();
	
		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		SG_Menu_Clear_SuperLogin();

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Cacular_Distance1
  功  能  ：里程1  的统计
  输入参数：
  输出参数：
  编写者  ：陈海华
  修改记录：创建，2006-5-23
****************************************************************/
static int SG_Proc_Cacular_Distance1(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;

	sprintf(str,"%.3f公里",(double)g_state_info.nGetDistance[0]/1000);	
	SG_DynMenu_Put_MsgBox (menuid, str);

	return 1;
}


/****************************************************************
  函数名：SG_Proc_Cacular_Distance2
  功  能  ：里程2  的统计
  输入参数：
  输出参数：
  编写者  ：陈海华
  修改记录：创建，2006-5-23
****************************************************************/
static int SG_Proc_Cacular_Distance2(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	sprintf(str,"%.3f公里",(double)g_state_info.nGetDistance[1]/1000);	
	SG_DynMenu_Put_MsgBox (menuid, str);
	return 1;
}
/****************************************************************
  函数名：SG_Proc_Clr_Distance1
  功  能  ：里程1  的统计清零
  输入参数：
  输出参数：
  编写者  ：陈海华
  修改记录：创建，2006-5-23
****************************************************************/
static int SG_Proc_Clr_Distance1(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;

	g_watch_info.nGetDistance[0]=0;
	g_state_info.nGetDistance[0]=0;
	SG_Watch_Save();

	sprintf(str,"里程1已清零");	
	SG_DynMenu_Put_MsgBox (menuid, str);
	
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Clr_Distance2
  功  能  ：里程2  的统计清零
  输入参数：
  输出参数：
  编写者  ：陈海华
  修改记录：创建，2006-5-23
****************************************************************/
static int SG_Proc_Clr_Distance2(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	g_watch_info.nGetDistance[1]=0;
	g_state_info.nGetDistance[1]=0;
	SG_Watch_Save();

	sprintf(str,"里程2已清零");	
	SG_DynMenu_Put_MsgBox (menuid, str);
	
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_OneGPS
  功  能  ：一键导航设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  手动设置本机号后唤醒后台接收和密钥交换线程，李松峰，2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "一键导航号码:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sOneGPSNo, s1);
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_AlarmSms
  功  能  ：短信报警号码设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  手动设置本机号后唤醒后台接收和密钥交换线程，李松峰，2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "短信报警号码1:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		memset(g_set_info.sSmsAlarmNo1,0,sizeof(g_set_info.sSmsAlarmNo1));
		strcpy(g_set_info.sSmsAlarmNo1, s1);
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_AlarmSms2
  功  能  ：短信报警号码设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  手动设置本机号后唤醒后台接收和密钥交换线程，李松峰，2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "短信报警号码2:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		memset(g_set_info.sSmsAlarmNo2, 0, sizeof(g_set_info.sSmsAlarmNo2));
		strcpy(g_set_info.sSmsAlarmNo2, s1);
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Call_Out
  功  能  ：禁止呼出菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.bCallOutDisable == 1)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭呼出限制?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "开启呼出限制?");

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			if (g_set_info.bCallOutDisable == 1)
			{
				g_set_info.bCallOutDisable = 0;
				SG_DynMenu_Put_MsgBox (menuid, "呼出限制已关闭!");
			}
			else
			{
				g_set_info.bCallOutDisable = 1;
				SG_DynMenu_Put_MsgBox (menuid, "呼出限制启用!");			
			}	

			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}

/****************************************************************
  函数名：SG_Proc_Call_In
  功  能  ：禁止呼入菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.bCallInDisable == 1)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭呼入限制?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "开启呼入限制?");

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			
			if (g_set_info.bCallInDisable == 1)
			{
				g_set_info.bCallInDisable = 0;
				SG_DynMenu_Put_MsgBox (menuid, "呼入限制已关闭!");
			}
			else
			{
				g_set_info.bCallInDisable = 1;
				SG_DynMenu_Put_MsgBox (menuid, "呼入限制启用!");			
			}
			
			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}

/****************************************************************
  函数名：SG_Proc_Auto_Call
  功  能  ：自动接听设置
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.GpsAutoCall == 0)
			SG_DynMenu_Put_ChooseBox (menuid, "开启自动接听?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "关闭自动接听?");

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			
			if (g_set_info.GpsAutoCall == 0)
			{
				g_set_info.GpsAutoCall = 1;
				SG_DynMenu_Put_MsgBox (menuid, "自动接听已启用!");
			}
			else
			{
				g_set_info.GpsAutoCall = 0;
				SG_DynMenu_Put_MsgBox (menuid, "自动接听关闭!");			
			}
			
			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}


/****************************************************************
  函数名：SG_Proc_Set_Heart_Switch
  功  能  ：设置心跳开关
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.Heart_Switch == 0)
			SG_DynMenu_Put_ChooseBox (menuid, "开启心跳功能?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "关闭心跳功能?");

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			
			if (g_set_info.Heart_Switch == 0)
			{
				g_set_info.Heart_Switch = 1;
				SG_DynMenu_Put_MsgBox (menuid, "心跳功能启用!");
			}
			else
			{
				g_set_info.Heart_Switch = 0;
				SG_DynMenu_Put_MsgBox (menuid, "心跳功能关闭!");			
			}
			
			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}

/****************************************************************
  函数名：SG_Proc_ShortCut
  功  能  ：快捷键回调函数
  输入参数：key:键ID
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  增加抢答信息发送结果提示，李松峰，2005-12-05
  增加抢答禁止判断，李松峰，2006-01-11
****************************************************************/
int SG_Proc_ShortCut(unsigned char key)
{
	MsgUChar *msg = NULL;
	MsgInt msgLen;
	int menuid = MENUID_SINGLE;

	switch (key)
	{
		case SHORTCUT_MEDICAL:

			SG_DynMenu_Put_VoiceBox((unsigned char*)(&menuid), "拨打医疗求助号码?",  g_set_info.sMedicalNo,  "医疗求助");

			break;
			
		case SHORTCUT_SERVICE:
			
			SG_DynMenu_Put_VoiceBox((unsigned char*)(&menuid), "拨打维修求助号码?",  g_set_info.sServiceNo,  "维修求助");
		
			break;

		//电召抢答	
		case SHORTCUT_INFO:
#if (0)
			if(g_set_info.bQuickAnswer==1)
			{
				g_set_info.ttsflag=1;
				SG_DynMenu_Put_AutoShowBox((unsigned char*)&menuid, "播报信息发送成功!等待响应...");	
				g_set_info.ttsflag=0;
				SG_Net_Voice_Snd_Msg(MSG_SND_VOICE,MSG_VOICE_SET_COMBIN,g_tts_buf,g_tts_buflen);
				break;
			}
			
			if (g_set_info.bMusterDisable == 1)
			{
 				SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "抢答已被禁止!");
				break;
			}

			if (MsgMusterFirst(&msg, &msgLen) == MSG_TRUE)
			{
				SG_Send_Put_New_Item(1, msg, msgLen);
				SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "抢答信息发送成功!等待响应...");				
			}
			else
			{
				SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "抢答信息发送失败!");
			}
#endif /* (0) */
			SG_DynMenu_Put_VoiceBox((unsigned char*)(&menuid), "拨打服务热线?",  g_set_info.sHelpNo,  "服务热线");

			break;
	
		default:
			break;
	}
	
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Ver
  功  能  :版本切换
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  通讯方式SMS改成GPRS时断线重连，李松峰，2005-12-20
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_state_info.user_type == 1) 
			SG_DynMenu_Put_ChooseBox (menuid, "切换为私家车版本");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "切换为普通版本");

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{

			if (g_state_info.user_type == 1) // 禁止自动接听
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
				SG_DynMenu_Put_MsgBox (menuid, "启用私家车版本!");	
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
				SG_DynMenu_Put_MsgBox (menuid, "启用普通版本!");			
			}
			SG_Soft_Reset(2);
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}

}



/****************************************************************
  函数名：SG_Proc_AutoAnswer
  功  能  ：自动接听菜单回调函数
  输入参数：
  输出参数：
  编写者  ：董佳枝
  修改记录：创建，2006-2-12
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.bAutoAnswerDisable == 0) // 允许自动接听
			SG_DynMenu_Put_ChooseBox (menuid, "使用手柄通话?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "使用免提通话?");

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{

			SG_Set_Cal_CheckSum();
			if (g_set_info.bAutoAnswerDisable == 1) // 禁止自动接听
			{
				g_set_info.bAutoAnswerDisable = 0;
				SG_DynMenu_Put_MsgBox (menuid, "启用免提通话!");
			}
			else
			{
				g_set_info.bAutoAnswerDisable = 1;
				SG_DynMenu_Put_MsgBox (menuid, "启用手柄通话!");			
			}
			
			SG_Set_Save();
			//SG_Notify_Hh();
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}

/****************************************************************
  函数名：SG_Proc_Test_TTS
  功  能  ：测试语音芯片TTS
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
		SG_DynMenu_Put_MsgBox (menuid, "请开启TTS");
		return 1;
	}

	SG_Voice_Snd_Msg(MSG_VOICE_SET_COMBIN, TTSMsg, 4);
	SG_DynMenu_Put_MsgBox (menuid, "请试听..........");
	return 1;
	
}

static int SG_Proc_Check_AD(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[50]="";

	if (s1 != NULL || s2 != NULL)
		return 0;

	sprintf(title,"当前油箱AD 值:\r\n %d ",g_state_info.CurrAD/100);

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}

//拍摄图像安装调试
static int SG_Proc_Pictrue_Catch(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[200]="";

	if (s1 != NULL || s2 != NULL)
		return 0;
	if(!(g_set_info.bNewUart4Alarm & (UT_XGCAMERA|UT_QQCAMERA)))
	{
		
		strcat(title,"摄像头未开启，请开启摄像头!!");
	}
	else
	{
		char sig[3];
		
		xSignalHeaderRec      *signal_ptr = NULL;
		sig[0] = 3;
		sig[1] = CAMERA_ONE_REQ;
		sig[2] = PIC_NOT_SAVE;
		SG_CreateSignal_Para(SG_CAMERA_ONE_ASK,3, &signal_ptr,sig); // 发送信号到主循环要求处理发送队列中的报文
		strcat(title,"测试拍摄一张图片!!\r\n");
		sprintf(title + strlen(title),"成功拍摄%d张!!",g_state_info.photo_catch_cnt);		
	
	}

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Test_LED
  功  能  ：测试LED
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
		SG_DynMenu_Put_MsgBox (menuid, "请开启LED");
		return 1;
	}
	else
	{
		char ledmsg[64]={0};
		int ledlen=0;
		ledmsg[0]=0x0a;
		strcpy(&ledmsg[1],"$$00S0FF0102我被打劫,请报警&&");
		ledlen=strlen(ledmsg);
		ledmsg[ledlen]=0x0d;
		
		if(g_set_info.bNewUart4Alarm & UT_LED)
			SIO_WriteFrame(COM_DEBUG,ledmsg,ledlen+1);	
		else if(g_set_info.bNewUart4Alarm & UT_LED)
			SIO_WriteFrame(COM2,ledmsg,ledlen+1);	
		
		SG_DynMenu_Put_MsgBox (menuid, "我被打劫,请报警");
		return 1;
	}

}


/******************************************************************
函数名: SG_Proc_Test_OpenLock
功能:测试中控锁开锁
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
		SG_DynMenu_Put_MsgBox (menuid, "请开启中控锁");
		return 1;
	}
	g_state_info.screen=0x00;
	g_state_info.set=0x00;
	
	SG_DynMenu_Put_MsgBox (menuid, "操作完成");
	return 1;
}
/********************************************************************
函数名:SG_Proc_Test_CloseLock
功能:	    测试中控锁闭锁
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
		SG_DynMenu_Put_MsgBox (menuid, "请开启中控锁");
		return 1;
	}
	g_state_info.screen=0x00;
	g_state_info.set=0x80;
	SG_DynMenu_Put_MsgBox (menuid, "操作完成");
	return 1;
}



/****************************************************************
  函数名：SG_Proc_SpeakVolChg
  功  能  ：音频调整菜单回调函数
  输入参数：
  输出参数：
  编写者  ：董佳枝
  修改记录：创建，2006-2-12
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
		SG_DynMenu_Put_InputBox (menuid, "音量设置(1~7):", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int vol = 0;
				
		menuList->item->count = 0;		

		vol = atoi(s1);

		if(vol < 1 || vol > 7)
		{
			menuList->item->count = 0;
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
			return 1;		
		}
		
		SG_Set_Cal_CheckSum();
		g_set_info.speaker_vol = vol;
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");
				
	return 1;
	}	
	else
	{
		menuList->item->count = 0;

		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}


				


				
/****************************************************************
  函数名：SG_Proc_IO_Check_State
  功  能  ：检测IO及各接线状态
  输入参数：
  输出参数：返回Acc状态 计价状态 车门状态 手动报警状态 熄火测试 
  编写者  ：郭碧莲
  修改记录：创建 2007/10/10 
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
		
		strcpy(title,"适配器IO:");

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
		strcpy(title,"中控锁: 未开启");	
		strcat(title,"\r\n");
	#endif
	
	//sdbj
	if(g_state_info.alarmState & GPS_CAR_STATU_HIJACK)
	{
		sprintf(buf, "%s%s\r\n", "手动报警:", "开");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "手动报警:", "关");
		strcat(title, buf);
	}
	//cmkg
	if(!(g_set_info.alarmSet & GPS_CAR_STATU_IO1))// 自定义报警高电平触发
	{
		strcat(title,"自定义报警端口:\r\n高电平触发\r\n");
	}
	else
	{
		strcat(title,"自定义报警端口:\r\n低电平触发\r\n");
	}

	if(g_state_info.alarmState & GPS_CAR_STATU_IO1)
	{
		strcat(title,"自定义报警端口:开\r\n");
	}
	else
	{
		strcat(title,"自定义报警端口:关\r\n");
	}

#if (0)
	if(g_state_info.alarmState & GPS_CAR_STATU_DOOR)
	{	
		sprintf(buf, "%s%s\r\n", "车门报警:", "开");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "车门报警:", "关");
		strcat(title, buf);
	}
#endif /* (0) */
	if(g_set_info.bBatteryCheckEnable)
	{
		strcat(title,"计价器信号做掉电检测\r\n");	
		if(g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF)
		{
			sprintf(buf, "%s%s\r\n", "计价器信号:", "开");
			strcat(title, buf);
		}
		else
		{
			sprintf(buf, "%s%s\r\n", "计价器信号:", "关");
			strcat(title, buf);
		}
	}
	else
	{
		strcat(title,"计价器信号做普通使用\r\n");	
		if(g_state_info.alarmState & GPS_CAR_STATU_JJQ)
		{
			sprintf(buf, "%s%s\r\n", "计价器信号:", "开");
			strcat(title, buf);
		}
		else
		{
			sprintf(buf, "%s%s\r\n", "计价器信号:", "关");
			strcat(title, buf);
		}
	}
	//jjq	
	if(g_set_info.JJQLev == 0)// 自定义报警高电平触发
	{
		strcat(title,"计价输入信号:\r\n高电平触发\r\n");
	}
	else
	{
		strcat(title,"计价输入信号:\r\n低电平触发\r\n");
	}

#if (0)
	if(g_state_info.alarmState & GPS_CAR_STATU_JJQ)
	{
		sprintf(buf, "%s%s\r\n", "计价器信号:", "开");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "计价器信号:", "关");
		strcat(title, buf);
	}
#endif /* (0) */

	//acc
	if(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)
	{
		sprintf(buf, "%s%s\r\n", "ACC信号:", "开");
		strcat(title, buf);
	}
	else 
	{
		sprintf(buf, "%s%s\r\n", "ACC信号:", "关");
		strcat(title, buf);
	}
		
	//VCAR_PFO
	if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE)
	{
		sprintf(buf, "%s%s\r\n", "电瓶:", "欠压");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "电瓶:", "正常");
		strcat(title, buf);
	}
			
			
	SG_DynMenu_Put_MsgBox (menuid, title);
	return 1;

}


/****************************************************************
  函数名：SG_Proc_Set_IO_Level
  功  能  ：设置自定义报警io口的触发电平	
  输入参数：
  输出参数：
  编写者  	：郭碧莲
  修改记录：创建，2007/10/11 
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(!(g_set_info.alarmSet & GPS_CAR_STATU_IO1))
			SG_DynMenu_Put_ChooseBox (menuid, "设置为低电平触发 ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "设置为高电平触发 ?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
				SG_DynMenu_Put_MsgBox (menuid, "当前为高电平触发!");
				
			}
			else
			{
				SG_Set_Cal_CheckSum();
				g_set_info.alarmSet |= GPS_CAR_STATU_IO1;
				SG_Set_Save();
				SG_DynMenu_Put_MsgBox (menuid, "当前为低电平触发!");
			}

		}
		else
		{
			menuList->item->count = 0;
			
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}


/****************************************************************
  函数名：SG_Proc_Set_JJQ_Lev
  功  能  ：设置计价输入的触发电平	
  输入参数：
  输出参数：
  编写者  	：郭碧莲
  修改记录：创建，2007/10/11 
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_set_info.JJQLev == 0)
			SG_DynMenu_Put_ChooseBox (menuid, "设置为低电平触发 ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "设置为高电平触发 ?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
				SG_DynMenu_Put_MsgBox (menuid, "当前为高电平触发!");
				
			}
			else
			{
				SG_Set_Cal_CheckSum();
				g_set_info.JJQLev = 1;
				SG_Set_Save();
				SG_DynMenu_Put_MsgBox (menuid, "当前为低电平触发!");
			}

		}
		else
		{
			menuList->item->count = 0;
			
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}



/****************************************************************
  函数名：SG_Proc_Set_Battery_Check
  功  能  ：计价复用
  输入参数：
  输出参数：
  编写者  	：郭碧莲
  修改记录：创建，2007/10/11 
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
			SG_DynMenu_Put_ChooseBox (menuid, "使用掉电报警?");					
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "关闭掉电报警?");
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

			SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

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
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	return 1;

	
}
#ifdef _DUART
/****************************************************************
  函数名：SG_Proc_VOLT_Lock
  功  能  ：电压解锁		
  输入参数：
  输出参数：
  编写者  	：林兆剑
  修改记录：创建，2008/11/21
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
		SG_DynMenu_Put_MsgBox (menuid, "请开启重工锁车");
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
	//SG_DynMenu_Put_MsgBox (menuid, "解锁成功");

	g_pro_lock.nType = PRO_VOLT_UNLOCK;
	//g_pro_lock.nCarNum = 0x01;
	//g_pro_lock.timeout = 0;
	//g_pro_lock.ack = 0;
	g_pro_lock.VoltTime = 1;
	SG_Rcv_Safety_VOLT_UnLock();
		SG_DynMenu_Put_MsgBox (menuid, "解锁成功");
	//g_pro_lock.state = SG_MENU_CTRL;
	
	menuList->item->count = 0;
	
	return 1;
	
}

/****************************************************************
  函数名：SG_Proc_VOLT_Lock
  功  能  ：电压锁车		
  输入参数：
  输出参数：
  编写者  	：林兆剑
  修改记录：创建，2008/11/21
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
		SG_DynMenu_Put_MsgBox (menuid, "请开启重工锁车");
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
	//SG_DynMenu_Put_MsgBox (menuid, "锁车成功");

	g_pro_lock.nType = PRO_VOLT_LOCK;
	//g_pro_lock.nCarNum = 0x01;
	//g_pro_lock.timeout = 0;app
	//g_pro_lock.ack = 0;
	g_pro_lock.VoltTime = 1;
	SG_Rcv_Safety_VOLT_Lock();
	SG_DynMenu_Put_MsgBox (menuid, "锁车成功");
	//g_pro_lock.state = SG_MENU_CTRL;
	
	menuList->item->count = 0;
	
	return 1;
	
}

/****************************************************************
  函数名：SG_Proc_One_Lock
  功  能  ：一级锁车		
  输入参数：
  输出参数：
  编写者  	：林兆剑
  修改记录：创建，2008/11/11
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
		SG_DynMenu_Put_MsgBox (menuid, "请开启重工锁车");
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
	SG_DynMenu_Put_MsgBox (menuid, "一级锁车,等待中...");

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
  函数名：SG_Proc_Two_Lock
  功  能  ：二级锁车		
  输入参数：
  输出参数：
  编写者  	：林兆剑
  修改记录：创建，2008/11/11
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
		SG_DynMenu_Put_MsgBox (menuid, "请开启重工锁车");
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
	SG_DynMenu_Put_MsgBox (menuid, "二级锁车,等待中...");

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
  函数名：SG_Proc_Unioce_Lock
  功  能  ：解锁		
  输入参数：
  输出参数：
  编写者  	：林兆剑
  修改记录：创建，2008/11/11
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
		SG_DynMenu_Put_MsgBox (menuid, "请开启重工锁车");
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
	SG_DynMenu_Put_MsgBox (menuid, "解锁,等待中...");

	g_pro_lock.nType = PRO_UNCOIL_LOCK;
	g_pro_lock.nCarNum = 0x01;
	g_pro_lock.timeout = 0;
	g_pro_lock.ack = 0;
	g_pro_lock.state = SG_MENU_CTRL;
	SG_Rcv_Safety_Uncoil_Lock();
	//g_xih_ctrl.XIH_State = SG_MENU_CTRL;

	//tryCount = 3;
	//Pic_Send(0xa6); // 熄火，关闭油路
	menuList->item->count = 0;
	
	return 1;
	
}
		
#endif

/****************************************************************
  函数名：SG_Proc_Set_IO_Level
  功  能  ：熄火		
  输入参数：
  输出参数：
  编写者  	：郭碧莲
  修改记录：创建，2007/10/11 
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
	SG_DynMenu_Put_MsgBox (menuid, "熄火,等待中...");
	g_xih_ctrl.XIH_State = SG_MENU_CTRL;

	tryCount = 3;
	Pic_Send(0xa6); // 熄火，关闭油路
	menuList->item->count = 0;
	
	return 1;
	
}
			
/****************************************************************
  函数名：SG_Proc_Set_IO_Level
  功  能  ：熄火恢复		
  输入参数：
  输出参数：
  编写者  	：郭碧莲
  修改记录：创建，2007/10/11 
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
	SG_DynMenu_Put_MsgBox (menuid, "熄火恢复,等待中...");
	tryCount = 3;
	g_xih_ctrl.XIH_State = SG_MENU_CTRL;
	Pic_Send(0xc5); // 熄火恢复，打开油路
	menuList->item->count = 0;
	
	return 1;
}


/****************************************************************
  函数名：SG_Proc_Muster_Particular_Ask
  功  能  ：调度信息回调函数
  输入参数：
  输出参数：
  编写者  ：chenhaihua
  修改记录：创建，2005-9-20
  				 改变调度信息输出方式 陈利 2006-09-06
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

	//首次调用,操作
	if (menuList->item->count == 0)
	{
		if(gn_muster_particular_list<=0){
			sprintf(ch,"无信息");
			SG_DynMenu_Put_MsgBox (menuid, ch);
			return 1;

		}
		else{
		menuList->item->count = 1;
		sprintf(ch,"输入信息号0-%d:",gn_muster_particular_list-1);
		SG_DynMenu_Put_InputBox (menuid, ch, NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);

		}

		return 1;
	}
	//获取信息号
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
			SG_DynMenu_Put_MsgBox (menuid, "输入信息号超出");
		}

		return 1;
	}

	return 1;

}

/****************************************************************
  函数名：SG_Proc_Show_Device
  功  能  ：终端状态查询菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
****************************************************************/
static int SG_Proc_Show_Device(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char *title = NULL;

	if (s1 != NULL || s2 != NULL)
		return 0;

	title = SCI_ALLOC( 1024);
		
	if (g_set_info.bRegisted == 0)
		strcpy(title, "未注册!\r");
	else
		strcpy(title, "已注册!\r");
	
	if (g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF )
		strcat(title, "主电平掉电!\r");

	else if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE )
		strcat(title, "主电平欠压!\r");
	
	#if (__cwt_)
	if (temp_s_udpconn == NULL)
		strcat(title, "GPRS断!\r");
	else
		strcat(title, "GPRS通!\r");
	#else
	if (g_state_info.socket <= 0)
		strcat(title, "GPRS断!\r");
	else
		strcat(title, "GPRS通!\r");
	#endif		
	strcat(title, "本机号:\r");
	strcat(title, g_set_info.sOwnNo);
	strcat(title, "\r");
	#if(__cwt_)
	strcat(title, "COMMAND ADDR:\r");
	strcat(title, g_set_info.sCommAddr);
	strcat(title, "\r");
	
	strcat(title, "授权码:\r");
	strcat(title, g_set_info.sOemCode);
	strcat(title, "\r");
	#endif
	strcat(title, "中心号:\r");
	strcat(title, g_set_info.sCenterNo);
	strcat(title, "\r");

	strcat(title, "通讯方式:");
	if (g_set_info.nNetType == MSG_NET_SMS)
		strcat(title, "短信");
	else if (g_set_info.nNetType == MSG_NET_GPRS)
		strcat(title, "数据");
	else if (g_set_info.nNetType == MSG_NET_UNION)
		strcat(title, "混合");
	strcat(title, "\r");
	
	strcat(title, "中心IP:\r");
	strcat(title, g_set_info.sCenterIp);
	strcat(title, " ");
	sprintf(title+strlen(title), "%d", g_set_info.nCenterPort);
	strcat(title, "\r");
	
	strcat(title, "代理IP:\r");
	strcat(title, g_set_info.sProxyIp);
	strcat(title, " ");
	sprintf(title+strlen(title), "%d", g_set_info.nProxyPort);
	strcat(title, "\r");

	if (g_set_info.bProxyEnable==1)
		strcat(title, "(使用代理)\r");
	else
		strcat(title, "(不使用代理)\r");

	strcat(title, "APN:");
	strcat(title, g_set_info.sAPN);
	strcat(title, "\r");	
	strcat(title, "USER:");	
	strcat(title, g_set_info.sGprsUser);
	strcat(title, "\r");	
	strcat(title, "PSW:");
	strcat(title, g_set_info.sGprsPsw);	
	strcat(title, "\r");

	strcat(title, "监控方式:");
	if (g_set_info.nWatchType == MSG_WATCH_TIME)
		strcat(title, "定时");
	else if (g_set_info.nWatchType == MSG_WATCH_AMOUNT)
		strcat(title, "定次");
	else if (g_set_info.nWatchType == MSG_WATCH_DISTANCE)
		strcat(title, "定距");
	else
		strcat(title, "取消");
	strcat(title, "\r");	
	
	if (g_set_info.bCompress == 0)
		strcat(title, "(不压缩");
	else
		strcat(title, "(有压缩");	

	if (g_set_info.nInflexionRedeem == 0)
		strcat(title, ",无拐点补偿");
	else
		strcat(title, ",有拐点补偿");		
	
	if (g_set_info.bStopReport== 0)
		strcat(title, ",停车不汇报)\r");
	else
		strcat(title, ")\r");		

	sprintf(title+strlen(title), "监控间隔:%d\r", g_set_info.nWatchInterval);
	if(g_set_info.nWatchType == MSG_WATCH_AMOUNT)
	{

		sprintf(title+strlen(title), "监控次数:%ld\r", g_set_info.nWatchTime);
		if(g_set_info.nWatchTime == 0)
		{
			sprintf(title+strlen(title), "已监控次数:%ld\r", g_watch_info.nWatchedTime);
		}
		else
		{
			sprintf(title+strlen(title), "已监控次数:%ld\r", g_watch_info.nWatchedTime);
		}
	}
	else
 	{

		sprintf(title+strlen(title), "监控时间:%ld\r", g_set_info.nWatchTime);
		if(g_set_info.nWatchTime == 0)
		{
			sprintf(title+strlen(title), "已监控时间:%ld\r", g_watch_info.nWatchedTime);
		}
		else
		{
			sprintf(title+strlen(title), "已监控时间:%ld\r", g_watch_info.nWatchedTime);
		}
	}
	

	if(g_set_info.bRegionEnable)
	{
		sprintf(title+strlen(title),"区域报警开\t");
	}
	
	if(g_set_info.bLineEnable)
	{
		sprintf(title+strlen(title), "路线报警开\r\n");
	}	
 


	sprintf(title+strlen(title),"扩展串口:  0x%x \r ",g_set_info.bNewUart4Alarm);

	
	SG_DynMenu_Put_MsgBox (menuid, title);
	
	if (title)
		SCI_FREE(title);

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_OwnNo
  功  能  ：本机号码设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  手动设置本机号后唤醒后台接收和密钥交换线程，李松峰，2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "本机号码:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}	

	//保存本机号码
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
		else//设置终端已经登记
		{
			g_set_info.bRegisted = 1;
		}
			SG_Set_Save();
		#endif
		g_state_info.randtime = (g_set_info.sOwnNo[9]-'0')*10+ (g_set_info.sOwnNo[10]-'0'); //随机数
		SG_Net_Disconnect();
	
		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	return 0;
}


#if (__cwt_)
/****************************************************************
  函数名：SG_Proc_Set_OemCode
  功  能  ：本机号码设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  手动设置本机号后唤醒后台接收和密钥交换线程，李松峰，2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "授权码:", NULL, DECRYPT_BUF, SG_PHONE_LEN, ANYCHAR, ANY_UPLOW, NOCARD);
	}

	//保存本机号码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sOemCode, s1);
		memset(g_state_info.sKeepMsg,0,sizeof(g_state_info.sKeepMsg));
		memset(g_state_info.sChkKeepMsg,0,sizeof(g_state_info.sChkKeepMsg));
		SG_Set_Save();
		SG_Net_Disconnect();
		
		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		
	return 1;
}

#endif
		
/****************************************************************
  函数名：SG_Proc_Set_CenterNo
  功  能  ：中心号码设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  手动设置本机号后唤醒后台接收和密钥交换线程，李松峰，2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "中心号码:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sCenterNo, s1);
		
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_Communication_Type
  功  能  ：通讯方式设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  通讯方式SMS改成GPRS时断线重连，李松峰，2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "0短信1数据2混合", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char /*mode = 0, */type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type != MSG_NET_UNION && type != MSG_NET_GPRS && type != MSG_NET_SMS)
		{
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
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

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_Watch_Type
  功  能  ：监控方式设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  通讯方式SMS改成GPRS时断线重连，李松峰，2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "1定时3定次4取消", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char /*mode = 0, */type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type != MSG_WATCH_CANCLE && type != MSG_WATCH_AMOUNT && type != MSG_WATCH_TIME)
		{
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
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

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_Watch_Interval
  功  能  ：监控间隔时间设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  通讯方式SMS改成GPRS时断线重连，李松峰，2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "设置范围1-65535秒", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type == 0 || type >65535)
		{
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
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

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_GPS_Interval
  功  能  ：GPS过滤时间间隔设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  通讯方式SMS改成GPRS时断线重连，李松峰，2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "设置范围1-60秒", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type == 0 || type >60)
		{
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
			return 1;
		}
		
		if (g_set_info.nGpstTime!= (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.nGpstTime= type;				
			SG_Set_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}


/****************************************************************
  函数名：SG_Proc_Set_Heart_Interval
  功  能  ：心跳时间间隔设置菜单  默认为120秒
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  通讯方式SMS改成GPRS时断线重连，李松峰，2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "设置范围10-300", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type < 10 || type >300)
		{
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
			return 1;
		}
		
		if (g_set_info.Heart_Interval != (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.Heart_Interval = type;				
			SG_Set_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_GPS_Dist
  功  能  ：GPS漂移过滤距离设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  通讯方式SMS改成GPRS时断线重连，李松峰，2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "设置范围5-50米", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type < 5 || type >50)
		{
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
			return 1;
		}
		
		if (g_set_info.nGpstDist != (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.nGpstDist = type;				
			SG_Set_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_Efs_Max
  功  能  ：GPS漂移过滤距离设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  通讯方式SMS改成GPRS时断线重连，李松峰，2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "设置范围10-200", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type < 10 || type > 200)
		{
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
			return 1;
		}
		
		if (g_set_info.EfsMax!= (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.EfsMax = type;				
			SG_Set_Save();
			MAX_LIST_NUM = g_set_info.EfsMax-g_set_info.nRegionPiont-g_set_info.nLinePiont;
		}

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_CenterIp
  功  能  ：中心地址设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  	IP输入个改为AAABBBCCCDDD格式，李松峰，2005-11-17
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
		SG_DynMenu_Put_InputBox (menuid, "中心地址:", NULL, DECRYPT_BUF, SG_IP_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char sIp[3+1] = "";
		int nIp;
		char *s = (char*)s1;
		
		menuList->item->count = 0;		
		
		if (strlen(s1) != SG_IP_LEN)
		{
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!\r\nAAABBBCCCDDD,共12位,每段3位,不足的前面补0");
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
		
		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		

	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_CenterPort
  功  能  ：中心端口设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
		SG_DynMenu_Put_InputBox (menuid, "中心端口:", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		g_set_info.nCenterPort = atoi(s1);
		
		SG_Set_Save();

		SG_Net_Disconnect();
	
		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}

	return 1;
}


/****************************************************************
  函数名：SG_Proc_Set_ProxyIp
  功  能  ：代理地址设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  	IP输入个改为AAABBBCCCDDD格式，李松峰，2005-11-17  
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
		SG_DynMenu_Put_InputBox (menuid, "代理地址:", NULL, DECRYPT_BUF, SG_IP_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char sIp[3+1] = "";
		int nIp;
		char *s = (char*)s1;
		
		menuList->item->count = 0;		
		
		if (strlen(s1) != SG_IP_LEN)
		{
			SG_DynMenu_Put_MsgBox (menuid, "输入错误!\r\nAAABBBCCCDDD,共12位,每段3位,不足的前面补0");
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
	
		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_ProxyPort
  功  能  ：代理端口设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
		SG_DynMenu_Put_InputBox (menuid, "代理端口:", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		g_set_info.nProxyPort = atoi(s1);
		
		SG_Set_Save();
		
		SG_Net_Disconnect();

		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_ProxyEnable
  功  能  ：代理使能设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
			SG_DynMenu_Put_ChooseBox (menuid, "使用代理连接?");					
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "关闭代理连接?");
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

			SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

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
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	return 1;
}


#if (__cwt_)
/****************************************************************
  函数名：SG_Proc_Edit_SMS
  功  能  ：自编辑短信回复
  输入参数：
  输出参数：0: 可以再进来1:返回
  编写者  ：chenhaihua
  修改记录：创建，2007-7-27
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
		SG_DynMenu_Put_InputBox (menuid, "编辑短信:", NULL, DECRYPT_BUF, 256, ANYCHAR, ANY_UPLOW, NOCARD);
	}

	//发送短信信息
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		//gbk to unicode
		unicodelen= GBK2Unicode(unicode,s1);

		//高低字节转换
		UnicodeBE2LE(unicode,unicodelen);

		//unicode to base64
		basecodelen= base64_encode(basecode,unicodelen,unicode);
		
		MsgEditUp(basecode,basecodelen);
		
		SG_DynMenu_Put_MsgBox (menuid, "发送成功!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "发送失败!");
	}		
	return 1;

}

#else


/****************************************************************
  函数名：SG_Proc_Edit_SMS
  功  能  ：上传文字消息
  输入参数：
  输出参数：0: 可以再进来1:返回
  编写者  ：chenhaihua
  修改记录：创建，2007-7-27
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
		SG_DynMenu_Put_InputBox (menuid, "编辑短信:", NULL, DECRYPT_BUF, 256, ANYCHAR, ANY_UPLOW, NOCARD);
	}

	//发送短信信息
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		//gbk to unicode
		unicodelen= GBK2Unicode(unicode,s1);

		//高低字节转换
	//	UnicodeBE2LE(unicode,unicodelen);

		if(MsgEditSmsUpload(unicode,unicodelen,&msg,&msgLen) != MSG_FALSE){
		
			SG_Send_Put_New_Item(1, msg, msgLen);

			SG_DynMenu_Put_MsgBox (menuid, "已发送!");
		}
		else{
			menuList->item->count = 0;
			
			SG_DynMenu_Put_MsgBox (menuid, "发送失败!");
		}
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "发送失败!");
	}		
	return 1;

}

#endif
/****************************************************************
  函数名：SG_Proc_Set_APN
  功  能  ：APN设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
	
		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Set_Exit
  功  能  ：退出配置菜单菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
  函数名：SG_Proc_Set_User
  功  能  ：gprs用户名设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
		SG_DynMenu_Put_InputBox (menuid, "用户名:", NULL, DECRYPT_BUF, SG_GPRS_USER_LEN, PURE_ENG, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sGprsUser, s1);
		
		SG_Set_Save();
		
		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	return 1;
}

/****************************************************************
函数名：SG_Proc_SetCom_Lock
功能：串口扩展中控锁设备菜单回调函数
说明：
输入：
返回：
修改记录：创建，陈海华,2006-7-10
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_LOCK)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭中控锁?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开中控锁?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_LOCK;
				SG_DynMenu_Put_MsgBox (menuid, "中控锁打开!");
				g_state_info.bRelease1 = DEV_ENABLE;
			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_LOCK;
				SG_DynMenu_Put_MsgBox (menuid, "中控锁关闭!");
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
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}

/****************************************************************
函数名：SG_Proc_SetCom_TTS
功能：TTS 设备菜单回调函数
说明：
输入：
返回：
修改记录：创建，陈利,2006-8-7
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_TTS)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭TTS ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开TTS ?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}

				
				g_state_info.bNewUart4Alarm |= UT_TTS;
				SG_DynMenu_Put_MsgBox (menuid, "TTS 打开!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
				
			}
			else
			{
				g_state_info.bNewUart4Alarm &=~UT_TTS;
				SG_DynMenu_Put_MsgBox (menuid, "TTS 关闭!");
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
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}

/****************************************************************
函数名：SG_Proc_SetCom_Debug
功能：打开DEBUG菜单回调函数
说明：
输入：
返回：
修改记录：创建，陈利,2006-8-7
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bRelease1 == DEBUG_ENABLE)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭DEBUG ?");	
		else 
			SG_DynMenu_Put_ChooseBox (menuid, "打开DEBUG ?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(g_state_info.bRelease1 != DEBUG_ENABLE)
			{
				g_state_info.bRelease1 = DEBUG_ENABLE;
				SG_DynMenu_Put_MsgBox (menuid, "DEBUG 打开!");
			}
			else
			{
				#ifdef _DUART
				if(g_set_info.bNewUart4Alarm & 0x7fff)
				{
					g_state_info.bRelease1 = DEV_ENABLE;					
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG关闭并启用外设!");
				}
				else
				{
					g_state_info.bRelease1 = 0;
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG 关闭!");
				}
				#else
				if(g_state_info.bNewUart4Alarm)
				{
					g_state_info.bRelease1 = DEV_ENABLE;
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG关闭并启用外设!");
				}
				else
				{
					g_state_info.bRelease1 = HH_ENABLE;
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG关闭并启用手柄!");
				}
				#endif
			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}




/****************************************************************
函数名：SG_Proc_SetCom_Start
功能：启用当前设置回调函数
说明：
输入：
返回：
修改记录：创建，陈利,2006-8-7
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
		
	//首次调用

	 if (menuList->item->count == 0)
	{

		menuList->item->count = 1;
		
		SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 1;
	
	}
	//获取密码
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			char title[200] = "当前设置已启用!\r\n";

			SG_Set_Cal_CheckSum();
			g_set_info.bRelease1 = g_state_info.bRelease1;
			g_set_info.bRelease2 = g_state_info.bRelease2;
			g_set_info.bNewUart4Alarm = g_state_info.bNewUart4Alarm;
			if(g_set_info.bNewUart4Alarm &UT_LOCK) // 中控锁开启则下次重启进行中控锁测试 
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
					strcat(title,"串口2的LED.");
				}
				if(g_set_info.bNewUart4Alarm & UT_TTS2)
				{
					strcat(title,"串口2的TTS.");
				}
					
			}
			
			if(g_set_info.bRelease1 == DEBUG_ENABLE)
			{
				strcat(title,"开启DEBUG模式.");
			}
			else
			{
				if(g_set_info.bNewUart4Alarm & UT_LOCK)
				{
					strcat(title,"中控锁.");
				}
				if(g_set_info.bNewUart4Alarm & UT_TTS)
				{
					strcat(title,"TTS.");
				}
				if(g_set_info.bNewUart4Alarm & UT_LED)
				{
					strcat(title,"有源串口的LED.");
				}
				if(g_set_info.bNewUart4Alarm & UT_HBLED)
				{
					strcat(title,"鞍山海博LED.");
				}
				if(g_set_info.bNewUart4Alarm & UT_XGCAMERA)
				{
					strcat(title,"星光摄像头.");
				}
				if(g_set_info.bNewUart4Alarm & UT_QQCAMERA)
				{
					strcat(title,"青青子木摄像头.");
				}
				if(g_set_info.bNewUart4Alarm & UT_PROLOCK)
				{					
					strcat(title,"重工锁车.");			
				}
				if(g_set_info.bNewUart4Alarm & UT_WATCH)
				{
					strcat(title,"透传设备.");
				}
				if(g_set_info.bNewUart4Alarm &UT_DRIVE)
				{
					strcat(title,"驾培管理器.");
				}
				if(g_set_info.bNewUart4Alarm & UT_BUS)
				{
					strcat(title,"公交报站器.");
				}
				if(g_set_info.bNewUart4Alarm & UT_GPS)
				{
					strcat(title,"自导航.");
					if(g_set_info.GpsDevType == 1)
					{
						strcat(title,"ZB导航屏.");
					}
					else if(g_set_info.GpsDevType == 2)
					{
						strcat(title,"YH导航屏.");
					}
						
				}	
				if(g_set_info.bNewUart4Alarm  == 0)
				{
					
					strcat(title,"无连接外设");
				}
			
			}
			SG_DynMenu_Put_MsgBox (menuid, title);

			SG_Soft_Reset(5);
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}


/****************************************************************
  函数名：SG_Proc_Set_Lock_Out_Level
  功  能  ：设置中控锁常态电平
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
			SG_DynMenu_Put_InputBox (menuid, "中控锁设置电平:", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		strcpy(level,s1);
		
		if(strlen(level) < 5)
		{
			menuList->item->count = 0;
				
			SG_DynMenu_Put_MsgBox (menuid, "输入长度不够!");
				
			return 1;
		}
		for(i = 0;i < 5;i ++) // 设置值不为0和1时提示错误
		{
			if(level[i] != '0' && level[i] != '1')
			{
				menuList->item->count = 0;
				
				SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
				
				return 1;
			}
		}		
		SG_Set_Cal_CheckSum();
		g_set_info.alarmSet = 0;
		
		for(i = 0;i < 5;i ++)
		{
			if(level[i] == '1') // 设定的电平值为1
			{
				temp |= 0x01<<(5-i);
			}
		}
		g_set_info.alarmSet |= temp;
		g_set_info.alarmSet = temp<< 24;
		SG_Set_Save();

		SG_Net_Disconnect();
		
		strcpy(title,"设置成功!");
		SG_DynMenu_Put_MsgBox (menuid, title);

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}

	return 1;
}


/****************************************************************
函数名：SG_Proc_SetCom_Deliver
功能：LED 设备菜单回调函数
说明：
输入：
返回：
修改记录：创建，陈利,2006-8-7
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_LED)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭有源串口的LED显示屏?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开有源串口的LED显示屏?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_LED;
				SG_DynMenu_Put_MsgBox (menuid, "有源串口的显示屏已启用!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_LED;
				SG_DynMenu_Put_MsgBox (menuid, "有源串口的显示屏已关闭!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}



/****************************************************************
函数名：SG_Proc_SetCom_HBLED
功能：LED 设备菜单回调函数
说明：
输入：
返回：
修改记录：创建，陈利,2006-8-7
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_HBLED)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭鞍山海博LED显示屏?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开鞍山海博LED显示屏?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_HBLED;
				SG_DynMenu_Put_MsgBox (menuid, "鞍山海博的显示屏已启用!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_HBLED;
				SG_DynMenu_Put_MsgBox (menuid, "鞍山海博的显示屏已关闭!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}





#ifdef _DUART
/****************************************************************
函数名：SG_Proc_SetCom_Deliver
功能：LED 设备菜单回调函数
说明：
输入：
返回：
修改记录：创建，陈利,2006-8-7
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_LED2)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭串口2的LED显示屏?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开串口2的LED显示屏?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;
				}
				
				g_state_info.bNewUart4Alarm|=UT_LED2;
				SG_DynMenu_Put_MsgBox (menuid, "串口2的显示屏已启用!");
				g_state_info.bRelease2 = LED2_ENABLE;
			}
			else
			{
				g_state_info.bNewUart4Alarm &=~UT_LED2;
				SG_DynMenu_Put_MsgBox (menuid, "串口2的显示屏已关闭!");
				g_state_info.bRelease2 = HH_ENABLE;
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}


/****************************************************************
函数名：SG_Proc_SetCom_Deliver
功能：TTS2 设备菜单回调函数
说明：
输入：
返回：
修改记录：创建，陈利,2006-8-7
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_TTS2)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭串口2的TTS?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开串口2的TTS");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;
				}


				g_state_info.bNewUart4Alarm|=UT_TTS2;
				SG_DynMenu_Put_MsgBox (menuid, "串口2的TTS启用!");
				g_state_info.bRelease2 = LED2_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_TTS2;
				SG_DynMenu_Put_MsgBox (menuid, "串口2的TTS已关闭!");
				g_state_info.bRelease2 = HH_ENABLE;
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}
#endif




/****************************************************************
函数名：SG_Proc_SetCom_Deliver
功能：透传设备开关菜单回调函数
说明：
输入：
返回：
修改记录：创建，陈利,2006-8-7
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_WATCH)
			SG_DynMenu_Put_ChooseBox (menuid, "关闭透传设备?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "打开透传设备?");			

		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
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
		
					SG_DynMenu_Put_MsgBox (menuid, "已开启其他设备，请先关闭其他设备!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_WATCH;
				SG_DynMenu_Put_MsgBox (menuid, "透传设备已启用!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_WATCH;
				SG_DynMenu_Put_MsgBox (menuid, "透传设备已关闭!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}
}




/********************************************************************************
设置透传设备类型
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
		SG_DynMenu_Put_InputBox (menuid, "1 计价器 2 称重传感器", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		char value=-1;
		 
		menuList->item->count = 0;	
		
		value=atoi(s1);
		if(value != 1 && value != 2)
		{
			menuList->item->count = 0;
		
			SG_DynMenu_Put_MsgBox (menuid, "设置错误!");

		}
		SG_Set_Cal_CheckSum();
		g_set_info.watchid=value;
		SG_Set_Save();
		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");
		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	return 1;

}



/***********************************************************************
设置透传设备超时时间
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		SG_DynMenu_Put_ChooseBox (menuid, "是否设置超时时间?");
		
		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "设置参数(1~60):", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else

		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		var=atoi(s1);
		if((var>=1)&&(var<=60))
		{	
			SG_Set_Cal_CheckSum();
			g_set_info.nSetTime= var;
			SG_Set_Save();
			strcpy(title,"所设置参数:");
			sprintf(title+strlen(title),"%d",var);
			SG_DynMenu_Put_MsgBox(menuid,title);
		}
		else
			SG_DynMenu_Put_MsgBox (menuid, "参数设置错误!");
			
		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	return 1;
}


/************************************************************************
设置透传ID
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
		SG_DynMenu_Put_InputBox (menuid, "默认ID(0~255):", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
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
		sprintf(strmsg,"设置成功,设备\r\n默认ID:0x%x",g_set_info.devID);
		SG_DynMenu_Put_MsgBox (menuid, strmsg);
		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	return 1;

}

/************************************************************************
设置透传波特率
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
		
	//首次调用
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		SG_DynMenu_Put_ChooseBox (menuid, "是否设置串口波特率?");
		
		return 1;
	}
	//操作确认
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "设置参数:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//获取密码
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		var=atoi(s1);

		SG_Set_Cal_CheckSum();
		g_set_info.watchbd = var;
		SG_Set_Save();
		strcpy(title,"所设置参数:");
		sprintf(title+strlen(title),"%d",var);
		SG_DynMenu_Put_MsgBox(menuid,title);


		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}
	return 1;
}


/****************************************************************
  函数名：SG_Proc_Set_Psw
  功  能  ：gprs密码设置菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
		SG_DynMenu_Put_InputBox (menuid, "密码:", NULL, ENCRYPT_BUF, SG_GPRS_PSW_LEN, PURE_ENG, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sGprsPsw, s1);
		
		SG_Set_Save();
		
		SG_DynMenu_Put_MsgBox (menuid, "设置成功!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}

	return 1;
}


//显示 log 信息
static int SG_Proc_SysLog(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[600] = "";
	
	if (s1 != NULL || s2 != NULL)
		return 0;

	

	sprintf(title+strlen(title), "系统调试1:R %d,GGA %d, RMC %d, A %d, V %d, H %d, P %d, ac %d ,de %d\r\n",
			g_set_info.nReConCnt,g_state_info.nGGACnt,g_state_info.nRMCCnt,g_state_info.nACnt,g_state_info.nVCnt,g_state_info.nHandupConCnt,g_state_info.nPorxyConCnt,g_state_info.nActiveCnt,g_state_info.nDeactiveCnt);	
	sprintf(title+strlen(title), "err:gps %d\r\n",g_state_info.nGPSErr);
	sprintf(title+strlen(title), "alarmset %x\r\nalarmstate %lx\r\nalarmenable %lx alarmRepeat %lx\r\n",g_set_info.alarmSet,g_state_info.alarmState,g_set_info.alarmEnable,g_set_info.alarmRepeat);
	sprintf(title+strlen(title), "reboot %d\r\nnormal reset %d\r\n",g_set_info.nResetCnt,g_set_info.nReConCnt);
	sprintf(title+strlen(title), "g_set_info length %d\r\n",sizeof(g_set_info));
	sprintf(title+strlen(title), "GPS定位过滤时间为:%d\r", g_set_info.nGpstTime);
	sprintf(title+strlen(title), "GPS过滤距离为:%d\r", g_set_info.nGpstDist);
	sprintf(title+strlen(title), "盲区补偿的点为:%d\r", MAX_LIST_NUM);
	sprintf(title+strlen(title), "随机数为:%d \r", g_state_info.randtime);
	if(g_set_info.Heart_Switch)
		sprintf(title+strlen(title), "心跳开 间隔:%d \r", g_set_info.Heart_Interval);
	else
		sprintf(title+strlen(title), "心跳关 间隔:%d \r", g_set_info.Heart_Interval);
		

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}



//显示 log 信息
static int SG_Proc_SetCom_Check_It_Self(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[600] = "";
	int count = 0;

	if (s1 != NULL || s2 != NULL)
		return 0;

	sprintf(title+strlen(title), "Ext Device:\r\nbRelease1 %d brelease 2 %d\r\nbNewUart4Alarm %02x\r\n",g_set_info.bRelease1,g_set_info.bRelease2,g_set_info.bNewUart4Alarm);

	if(g_set_info.bNewUart4Alarm & (UT_QQCAMERA|UT_XGCAMERA))
	{
		sprintf(title+strlen(title), "摄像头:监控标志%x; 实时0x%x 路,间隔 %d ; 条件0x%x 路, 间隔%d\r\n"
			,g_set_info.nCameraAlarm, g_set_info.sg_camera[0].nCameraID,g_set_info.sg_camera[0].nInterval,g_set_info.sg_camera[1].nCameraID,g_set_info.sg_camera[1].nInterval) ;
	
		sprintf(title+strlen(title),"图片保存标志: %x  图片保存索引 %d\r\n",g_set_info.bPictureSave,g_set_info.nPictureSaveIndex);
		sprintf(title+strlen(title),"文件0-4保存图片: %d %d %d %d %d\r\n",g_set_info.camera_save_list[0].pic_save,
			g_set_info.camera_save_list[1].pic_save,g_set_info.camera_save_list[2].pic_save,
			g_set_info.camera_save_list[3].pic_save,g_set_info.camera_save_list[4].pic_save);
		sprintf(title+strlen(title),"摄像头当前状态: %d",CurrCamera.currst);
		sprintf(title+strlen(title),"断线次数: %d \r\n",g_state_info.nReConnect);
		count = SG_Send_Cal_Pic_Item();

		sprintf(title+strlen(title),"队列中图片包个数: %d",count);
		sprintf(title+strlen(title),"已经拍摄图片: %d",g_state_info.photo_catch_cnt);
	}

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}

/****************************************************************
  函数名：SG_Proc_Psw_Super
  功  能  ：超级密码菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  增加超级管理员密码也能修改管理员密码，李松峰，2005-12-06
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
	
	//首次调用
	if (menuList->item->count == 0)
	{
		if (menuList->item->data != NULL)
		{
			SCI_FREE(menuList->item->data);
			menuList->item->data = NULL;
		}

		menuList->item->count = 1;
		
		SG_DynMenu_Put_InputBox (menuid, "原超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 1;
	}
	//获取原密码信息
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) != 0)
		{
			menuList->item->count = 0;
		
			SG_DynMenu_Put_MsgBox (menuid, "超级密码错误!");
		}
		else
		{
			menuList->item->count = 2;

			SG_DynMenu_Put_InputBox (menuid, "新超级密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
		}

		return 1;
	}
	//获取新密码信息
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 3;
		menuList->item->data = SCI_ALLOC(12);
		SCI_MEMSET(menuList->item->data,0,12);
		strcpy(menuList->item->data,s1);
		SG_DynMenu_Put_InputBox (menuid, "再次输入密码:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 1;
	}
	//再次获取新密码信息
	else if ((menuList->item->count == 3) && (s1 != NULL) && (menuList->item->data != NULL))
	{
		menuList->item->count = 0;
	
		if (strcmp(menuList->item->data, s1) != 0)
		{
			SG_DynMenu_Put_MsgBox (menuid, "密码不匹配!");
		}
		else
		{
			SG_Set_Cal_CheckSum();
			//保存密码
			strcpy((char *)g_set_info.sSuperPsw,(const char*)s1);
			
			SG_Set_Save();
			
			SG_DynMenu_Put_MsgBox (menuid, "密码修改成功!");
		}
		
		SCI_FREE(menuList->item->data);
		menuList->item->data = NULL;

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");

		return 1;
	}		
}



/****************************************************************
  函数名：SG_Proc_Reset_Default
  功  能  ：恢复出厂菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
			SG_DynMenu_Put_ChooseBox (menuid, "重启系统?");
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			SG_DynMenu_Put_MsgBox (menuid, "重启中...");
			SG_Soft_Reset(2);
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		
	return 1;
}


/****************************************************************
  函数名：SG_Proc_Reset_Default
  功  能  ：恢复出厂菜单回调函数
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
			SG_DynMenu_Put_ChooseBox (menuid, "恢复出厂设置?");
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			SG_DynMenu_Put_MsgBox (menuid, "出厂设置成功!");
			SG_Set_Reset();
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "输入错误!");
	}		
	return 1;
}

/****************************************************************
  函数名：SG_Net_DynamicMenu_Snd_Msg
  功  能  ：动态菜单发送消息队列
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
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
  函数名：SG_Init_MenuList
  功  能  ：创建菜单列表
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2005-9-20
  	添加里程统计菜单，陈海华，2006-5-23
****************************************************************/
int SG_Init_MenuList ()
{
	MENUITEM *pMenuItem = NULL;
	MENULIST *pMenuList = NULL, *father = NULL;
	int nMenuId = FIRST_MENUID;
//	int i=0;
/*
主机功能菜单

	|--模块检测	|--版本查询
	|			|--车辆状态
	|			|--网络故障查询
	|                                 
	|--安装调试	|--IO及接线状态检测(未设置本机号时可以进入)
	|         	|--熄火
	|			|--熄火恢复
	|
	|--串口扩展	|--透传设备		|--开关LED显示屏功能
	|
	|        	|--中控锁		|--开关中控锁功能
	|                        		|--中控锁常态电平设置
	|          	|--开关TTS功能
	|			|--开关DEBUG
	|			|--启用当前设置
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

	// {模块检测
	pMenuList->item = pMenuItem;
	SCI_MEMCPY(pMenuItem->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuItem->s = "版本查询";  //  查看版本号
	pMenuItem->proc = SG_Proc_Ver_Ask;


//安装调试

//	|--安装调试		|
//	|				|
//	|				|--IO及接线状态检测
//	|				|
//	|				|--熄火
//	|				|
//	|				|--熄火恢复
//	|				|


	

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "安装调试";
	pMenuList->item->proc = NULL;
	
	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "IO及接线状态检测";
	pMenuList->item->proc =SG_Proc_IO_Check_State;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "自定义1触发电平";
	pMenuList->item->proc = SG_Proc_Set_IO_level;

#ifndef _TRACK
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "计价输入触发电平";
	pMenuList->item->proc = SG_Proc_Set_JJQ_Lev;

	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "设置计价输入复用";
	pMenuList->item->proc = SG_Proc_Set_Battery_Check;
#endif



	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("手柄与免提切换");
	pMenuList->item->proc = SG_Proc_AutoAnswer;
	
	#ifdef _DUART // 外设安装调试
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("LED报警测试");
	pMenuList->item->proc = SG_Proc_Test_LED;

	pMenuList->next = SCI_ALLOC(  sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC(  sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "中控锁开关锁";
	pMenuList->item->proc = NULL;
//{
	father = pMenuList;

	pMenuList->child = SCI_ALLOC(  sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC(  sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "开锁";
	pMenuList->item->proc =SG_Proc_Test_OpenLock;

	pMenuList->next = SCI_ALLOC(  sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC(  sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "闭锁";
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
	pMenuList->item->s = ("TTS设置调试");
	pMenuList->item->proc = SG_Proc_Test_TTS;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("油量AD 值");
	pMenuList->item->proc = SG_Proc_Check_AD;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("图像拍摄");
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
	pMenuList->item->s = "熄火";
	pMenuList->item->proc = SG_Proc_Set_XIH_Off;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "熄火恢复";
	pMenuList->item->proc = SG_Proc_Set_XIH_On;
	}


    pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("版本切换");
	pMenuList->item->proc = SG_Proc_Ver;


	pMenuList = father;	
	father = pMenuList->father;




// 安装调试end

//模块检测{
#if (__cwt_)
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "自编译短信";
	pMenuList->item->proc = SG_Proc_Edit_SMS;
#endif /* (0) */

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "模块检测";
	pMenuList->item->proc = NULL;


	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "车辆状态";
	pMenuList->item->proc =SG_Proc_Check_Device;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "网络故障查询";
	pMenuList->item->proc = SG_Proc_Net_State;
	pMenuList = father;	
	father = pMenuList->father;
//}模块检测菜单结束

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "防盗功能";
	pMenuList->item->proc = SG_Proc_Theft;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "快捷拨号";
	pMenuList->item->proc = NULL;

//	{ 	快捷拨号

	father = pMenuList;


	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "服务热线";
	pMenuList->item->proc = SG_Proc_Hot_Service;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("医疗求助");
	pMenuList->item->proc = SG_Proc_Medical_Service;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("维修求助");
	pMenuList->item->proc = SG_Proc_Help_Service;
	
	pMenuList = father;


//}快捷拨号结束

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("位置查询");
	pMenuList->item->proc = NULL;

//	{	位置查询

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("GPS信息");
	pMenuList->item->proc = SG_Proc_GPS_Info;


	pMenuList = father;

//	}	位置查询

//   {	里程统计菜单－begin
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("里程统计");
	pMenuList->item->proc = NULL;

//   		{	子菜单第1层－begin
	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("总里程统计");
	pMenuList->item->proc = SG_Proc_Cacular_Total_Distance;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("里程1统计");
	pMenuList->item->proc = SG_Proc_Cacular_Distance1;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("里程2统计");
	pMenuList->item->proc = SG_Proc_Cacular_Distance2;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("里程1清零");
	pMenuList->item->proc = SG_Proc_Clr_Distance1;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("里程2清零");
	pMenuList->item->proc = SG_Proc_Clr_Distance2;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("总里程设置");
	pMenuList->item->proc = SG_Proc_Set_Total_Distance;	
//   		}	子菜单第1层－end

	pMenuList = father;
//   }	里程统计菜单－end

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("通话设置");
	pMenuList->item->proc = NULL;
	
	

//		{	通话设置

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("呼入限制");
	pMenuList->item->proc = SG_Proc_Call_In;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);

	pMenuList->item->s = ("呼出限制");
	pMenuList->item->proc = SG_Proc_Call_Out;

	if (g_state_info.user_type == PRIVATE_CAR_VER)
	{
		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = "设置一键导航号码";
		pMenuList->item->proc = SG_Proc_Set_OneGPS;

		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = "短信报警号码1";
		pMenuList->item->proc = SG_Proc_Set_AlarmSms;

		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = "短信报警号码2";
		pMenuList->item->proc = SG_Proc_Set_AlarmSms2;

		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->father = father;
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = ("自动接听");
		pMenuList->item->proc = SG_Proc_Auto_Call;
	}

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("通话音量调整");
	pMenuList->item->proc = SG_Proc_SpeakVolChg;

	pMenuList = father;	

	father = pMenuList -> father;

//		}	通话设置

	
// 调度信息查询
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "调度信息查询";
	pMenuList->item->proc = SG_Proc_Muster_Particular_Ask;


// 串口扩展{
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "串口扩展";
	pMenuList->item->proc = NULL;

	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "透传设备";
	pMenuList->item->proc = NULL;

	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "设置开关";
	pMenuList->item->proc = SG_Proc_SetCom_Watch;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "设置透传ID";
	pMenuList->item->proc = SG_Proc_SetCom_ID;

#if (1)
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "设置透传波特率";
	pMenuList->item->proc = SG_Proc_SetCom_Baud;
#endif /* (0) */


	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "设置超时时间";
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
	pMenuList->item->s = "有源串口的LED";
	pMenuList->item->proc = SG_Proc_SetCom_LED;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "鞍山海博LED";
	pMenuList->item->proc = SG_Proc_SetCom_HBLED;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "串口2的LED";
	pMenuList->item->proc = SG_Proc_SetCom_LED2;
	#else
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "开关LED显示屏";
	pMenuList->item->proc = SG_Proc_SetCom_LED;	
	#endif

	

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "中控锁";
	pMenuList->item->proc = NULL;

	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "开关中控锁";
	pMenuList->item->proc = SG_Proc_SetCom_Lock;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "中控锁常态电平设置";
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
	pMenuList->item->s = "开关TTS";
	pMenuList->item->proc = SG_Proc_SetCom_TTS;

#ifdef _DUART
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "开关串口2的TTS";
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
	pMenuList->item->s = "开关自导航";
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
	pMenuList->item->s = "星光摄像头";
	pMenuList->item->proc = SG_Proc_SetCom_XgVideo;
#endif /* (0) */

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "青青子木摄像头";
	pMenuList->item->proc = SG_Proc_SetCom_QqzmVideo;
	

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "驾培管理器";
	pMenuList->item->proc = SG_Proc_SetCom_Drive;
	
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "开关DEBUG";
	pMenuList->item->proc = SG_Proc_SetCom_Debug;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "启用当前设置";
	pMenuList->item->proc = SG_Proc_SetCom_Start;	
	
	#ifdef _DUART
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "串口设备查看";
	pMenuList->item->proc = SG_Proc_SetCom_Check_It_Self;	
	#endif
	
	pMenuList = father;	
	father = pMenuList->father;
//串口扩展end
	
	

/*
终端设置隐藏菜单

	|--配置查询
	|--本机号码
	|--中心号码
	|--通讯方式
	|--中心地址
	|--中心端口
	|--GPRS设置  	|--代理地址
				|--代理端口
				|--代理使用
				|--APN
				|--用户名
				|--密码
	|--系统LOG
	|--恢复出厂
	|--重启系统
	|--退出配置
	
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
	pMenuList->item->s = "配置查询";
	pMenuList->item->proc = SG_Proc_Show_Device;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "本机号码";
	pMenuList->item->proc = SG_Proc_Set_OwnNo;
	#if (__cwt_)
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "授权码";
	pMenuList->item->proc = SG_Proc_Set_OemCode;
	#endif
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "中心号码";
	pMenuList->item->proc = SG_Proc_Set_CenterNo;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "通讯方式";
	pMenuList->item->proc = SG_Proc_Set_Communication_Type;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "中心地址";
	pMenuList->item->proc = SG_Proc_Set_CenterIp;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "中心端口";
	pMenuList->item->proc = SG_Proc_Set_CenterPort;

//GPRS 设置
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "GPRS设置";
	pMenuList->item->proc = NULL;

	father = pMenuList;
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "代理使用";
	pMenuList->item->proc = SG_Proc_Set_ProxyEnable;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList->next->father = father;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "代理地址";
	pMenuList->item->proc = SG_Proc_Set_ProxyIp;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "代理端口";
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
	pMenuList->item->s = "用户名";
	pMenuList->item->proc = SG_Proc_Set_User;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList->next->father = father;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "密码";
	pMenuList->item->proc = SG_Proc_Set_Psw;

	pMenuList = father;
	father = pMenuList->father;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "监控设置";
	pMenuList->item->proc = NULL;

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "监控方式";
	pMenuList->item->proc = SG_Proc_Set_Watch_Type;

	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "监控间隔时间";
	pMenuList->item->proc = SG_Proc_Set_Watch_Interval;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "GPS过滤时间";
	pMenuList->item->proc = SG_Proc_Set_GPS_Interval;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "里程统计最小距离";
	pMenuList->item->proc = SG_Proc_Set_GPS_Dist;

    pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "盲区补偿最大条数";
	pMenuList->item->proc = SG_Proc_Set_Efs_Max;

	pMenuList = father;
	father = pMenuList->father;


	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "心跳设置";
	pMenuList->item->proc = NULL;

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "心跳开关";
	pMenuList->item->proc = SG_Proc_Set_Heart_Switch;

	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "心跳间隔时间";
	pMenuList->item->proc = SG_Proc_Set_Heart_Interval;

	pMenuList = father;
	father = pMenuList->father;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "系统LOG";
	pMenuList->item->proc = SG_Proc_SysLog;

#if (0) // 暂时屏蔽超级密码修改
// 超级密码修改
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "超级密码修改";
	pMenuList->item->proc = SG_Proc_Psw_Super;

#endif /* (0) */

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "恢复出厂";
	pMenuList->item->proc = SG_Proc_Reset_Default;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "重启系统";
	pMenuList->item->proc = SG_Proc_Reset;


	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "退出配置";
	pMenuList->item->proc = SG_Proc_Set_Exit;

//   }	配置菜单－end

	SCI_TRACE_LOW("SG_Init_MenuList \r\n");
	return 1;
}
/****************************************************************
  函数名：SG_Menu_Set_MenuList
  功  能  ：设置当前菜单
  输入参数：type: 0:主菜单 1:设置菜单
  输出参数：无
  编写者  ：李松峰
  修改记录：创建，2005-11-27
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
  函数名：SG_Menu_Get_MenuList
  功  能  ：获取当前菜单
  输入参数：无
  输出参数：无
  编写者  ：李松峰
  修改记录：创建，2005-11-27
****************************************************************/
void* SG_Menu_Get_MenuList()
{
	return (void*)gpCurMenuList;
}
