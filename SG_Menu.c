/****************************************************************
  °æÈ¨   £º¸£½¨ĞÇÍøÈñ½İÍ¨Ñ¶ÓĞÏŞ¹«Ë¾ 2002-2005 °æÈ¨ËùÓĞ
  ÎÄ¼şÃû £ºSG_Menu.c
  °æ±¾   £º1.00
  ´´½¨Õß £ºÀîËÉ·å
  ´´½¨Ê±¼ä£º2005-9-23
  ÄÚÈİÃèÊö£ºSG2000 ÊÖ±ú²Ëµ¥¹¦ÄÜ´¦Àí 
  ĞŞ¸Ä¼ÇÂ¼£º
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


// ÓÃÓÚÅĞ¶Ï³¬¼¶ÃÜÂëÊÇ·ñÍ¨¹ı
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
	
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		if (menuList->item->data != NULL)
		{
			SCI_FREE(menuList->item->data);
			menuList->item->data = NULL;
		}

		menuList->item->count = 1;
		
		SG_DynMenu_Put_InputBox ((unsigned char*)menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 0;
	}
	//»ñÈ¡ÃÜÂëĞÅÏ¢
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{	
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) != 0)
		{
			menuList->item->count = 0;			
			SG_DynMenu_Put_MsgBox ((unsigned char*)menuid, "³¬¼¶ÃÜÂë´íÎó!");
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
/****************************************************************ÖØ¹¤Ëø³µ****************************************************************/
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
	//Ê×´Îµ÷ÓÃ	
	if (menuList->item->count == 0)	
	{		
		menuList->item->count = 1;		
		if(g_state_info.bNewUart4Alarm&UT_PROLOCK)			
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕÖØ¹¤Ëø³µ·?");		
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ªÖØ¹¤Ëø³µ? ");					
		return 1;	
	}	
	//²Ù×÷È·ÈÏ	
	else if ((menuList->item->count == 1) && (s1 != NULL))	
	{		
		if (*(char*)s1 == INPUT_CHOOSE_YES)		
		{			
			menuList->item->count = 2;					
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);					
			return 1;		
		}		
		else		
		{			
			menuList->item->count = 0;			
			return 0;		
		}	
	}	
	//»ñÈ¡ÃÜÂë	
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
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");					
					return 1;				
				}						
				
				g_state_info.bNewUart4Alarm|=UT_PROLOCK;				
				SG_DynMenu_Put_MsgBox (menuid, "ÖØ¹¤Ëø³µ´ò¿ª!");			
				g_state_info.bRelease1 = DEV_ENABLE;
			}			
			else			
			{				
				g_state_info.bNewUart4Alarm&=~UT_PROLOCK;				
				SG_DynMenu_Put_MsgBox (menuid, "ÖØ¹¤Ëø³µ¹Ø±Õ!");
				g_state_info.bRelease1 = 0;

			}	
		}		
		else		
		{			
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");		
		}		
		return 1;
	}	
	else
	{		
		menuList->item->count = 0;				
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");		
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_MINI)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕMINI?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ªMINI?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_MINI;
				SG_DynMenu_Put_MsgBox (menuid, "MINI´ò¿ª!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_MINI;
				SG_DynMenu_Put_MsgBox (menuid, "MINI¹Ø±Õ!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}
#endif /* (0) */
/****************************************************************
ĞÇ¹âÉãÏñÍ·
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_XGCAMERA)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕÉãÏñÍ·?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ªÉãÏñÍ·?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_XGCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "ÉãÏñÍ·´ò¿ª!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_XGCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "ÉãÏñÍ·¹Ø±Õ!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}

/****************************************************************
ÇàÇà×ÓÄ¾ÉãÏñÍ·
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_QQCAMERA)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕÉãÏñÍ·?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ªÉãÏñÍ·?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_QQCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "ÉãÏñÍ·´ò¿ª!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_QQCAMERA;
				SG_DynMenu_Put_MsgBox (menuid, "ÉãÏñÍ·¹Ø±Õ!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}

/****************************************************************
¼İÅà¹ÜÀíÆ÷
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_set_info.bNewUart4Alarm&UT_DRIVE)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õ¼İÅà¹ÜÀíÆ÷?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ª¼İÅà¹ÜÀíÆ÷?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_DRIVE;
				SG_DynMenu_Put_MsgBox (menuid, "¼İÅà¹ÜÀíÆ÷´ò¿ª!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_DRIVE;
				SG_DynMenu_Put_MsgBox (menuid, "¼İÅà¹ÜÀíÆ÷¹Ø±Õ!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}

/****************************************************************
º¯ÊıÃû£ºSG_Proc_SetCom_Bus_Dev
¹¦ÄÜ£º´®¿ÚÀ©Õ¹¹«½»Éè±¸²Ëµ¥»Øµ÷º¯Êı
ËµÃ÷£º
ÊäÈë£º
·µ»Ø£º
ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬³Âº£»ª,2006-7-5
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_BUS)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õ¹«½»±¨Õ¾Æ÷?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ª¹«½»±¨Õ¾Æ÷?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}
				
				g_state_info.bNewUart4Alarm|=UT_BUS;
				SG_DynMenu_Put_MsgBox (menuid, "¹«½»±¨Õ¾Æ÷´ò¿ª!");
				g_state_info.bRelease1 = DEV_ENABLE;
				

			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_BUS;
				SG_DynMenu_Put_MsgBox (menuid, "¹«½»±¨Õ¾Æ÷¹Ø±Õ!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif

			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}


/****************************************************************
×Ôµ¼º½
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_GPS)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õ×Ôµ¼º½?");		
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ª×Ôµ¼º½?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(!(g_state_info.bNewUart4Alarm&UT_GPS))
			{
				if(SG_Ext_Dev_Enable_Judge(UT_GPS))
				{
					menuList->item->count = 0;
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}

				SG_DynMenu_Put_InputBox (menuid, "´ò¿ªÀàĞÍ:1~2", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
				menuList->item->count = 3;

			}
			else
			{
				menuList->item->count = 0;
				g_state_info.bNewUart4Alarm&=~UT_GPS;
				SG_DynMenu_Put_MsgBox (menuid, "×Ôµ¼º½¹Ø±Õ!");
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
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
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
			
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

			return 1;
		}
		
		if(1 == type)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ZBµ¼º½ÆÁ´ò¿ª!");
			g_state_info.GpsDevType = 1;
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "YHµ¼º½ÆÁ´ò¿ª!");
			g_state_info.GpsDevType = 2;
		}
		g_state_info.bNewUart4Alarm|=UT_GPS;
		
		g_state_info.bRelease1 = DEV_ENABLE;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Ver_Ask
  ¹¦  ÄÜ  £º°æ±¾²éÑ¯²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º·µ»Øµ±Ç°°æ±¾
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
****************************************************************/
static int SG_Proc_Ver_Ask(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	char title[300] = "";

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	// APP VERSION
	strcpy(title, "Èí¼ş°æ±¾\r\n");
			
	strcat(title, g_set_info.sVersion);
				
	strcat(title, "\r\n");
	
	#ifdef _DUART
	strcat(title,"Ó²¼şĞÍºÅ:G01C\r\n");
#endif

#ifdef _SUART
#ifdef _TRACK
	strcat(title,"Ó²¼şĞÍºÅ:G01B\r\n");
#else
	strcat(title,"Ó²¼ş°æ±¾:");
	sprintf(title+strlen(title),"%x\r\n",g_state_info.hardware_ver);
	strcat(title,"Ó²¼şĞÍºÅ:G01A\r\n");
#endif
#endif



	strcat(title,"ÓÃ»§ÀàĞÍ:");

	if(g_state_info.user_type == COMMON_VER)
	{
		strcat(title,"ÆÕÍ¨°æ±¾\r\n");
	}
	else if(g_state_info.user_type == PRIVATE_CAR_VER)
	{
		strcat(title,"Ë½¼Ò³µ°æ±¾\r\n");	
	}
	
	SG_DynMenu_Put_MsgBox (menuid, title);
				
	return 1;

}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Net_State
  ¹¦  ÄÜ  £ºÍøÂç¹ÊÕÏ²éÑ¯»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º·µ»Ø ¡°GPS OK \r\nGSM VERSION xxx¡¡\r\nAPP VERSION xxxx¡±
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Ôö¼Ó¹ÊÕÏ²éÑ¯ĞÅÏ¢£¬ÀîËÉ·å£¬2005-12-09
  GPS¶¨Î»²éÑ¯¸ÄÎª¼ì²âµ±Ç°×´Ì¬£¬ÀîËÉ·å£¬2006-01-05
****************************************************************/
static int SG_Proc_Net_State(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	char title[200] = "";

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	// GPRSµ±Ç°¶¯×÷
	strcpy(title, "GPRS²¦ºÅ½×¶Î:\r\n");
	
	switch(g_gprs_t.state)
	{
		case GPRS_ACTIVE_PDP:
			strcat(title,"Active PDPÉÏÏÂÎÄ\n");
			break;
		case GPRS_DEACTIVE_PDP:
			strcat(title,"DeActive PDPÉÏÏÂÎÄ\n");
			break;	
		case GPRS_SOCKET_CONNECT:
			strcat(title,"Á¬½ÓSOCKET\n");
			break;
		case GPRS_PORXY:
			strcat(title,"Á¬½Ó´úÀí\n");
			break;
		case GPRS_HANDUP:
			strcat(title,"ÎÕÊÖ\n");
			break;
		case GPRS_RXHEAD:
		case GPRS_RXCONTENT:
			strcat(title,"Á´½ÓÖĞĞÄÕı³£\n");
			break;
		case GPRS_DISCONNECT:
			strcat(title,"Á¬½Ó¶Ï¿ª\n");
			break;
		default:
			break;
			
	}
	
	// ÉÏ´Î·¢ÉúµÄ´íÎó
	strcat(title,"\r×î½ü´íÎó:\r\n");
	
	switch(g_state_info.lasterr)
	{
		case SG_ERR_NONE:
			strcat(title,"Î´·¢Éú´íÎó");
			break;
		case SG_ERR_CREAT_SOCKET:
			strcat(title,"½¨Á¢Socket");
			break;
		case SG_ERR_PROXY_CONNET:
			strcat(title,"Á¬½ÓProxy");
			break;
		case SG_ERR_PROXY_NOTFOUND:
			strcat(title,"Á¬½ÓÖĞĞÄ");
			break;
		case SG_ERR_PROXY_DENY:
			strcat(title,"Proxy ¾Ü¾ø");
			break;
		case SG_ERR_CONNET_CENTER:
			strcat(title,"ÖĞĞÄ´íÎó");
			break;
		case SG_ERR_RCV_HANDUP:
			strcat(title, "ÎÕÊÖÓ¦´ğ");
			break;
		case SG_ERR_RCV_DATA:
			strcat(title,"½ÓÊÕÊı¾İ");
			break;
		case SG_ERR_SEND_DATA:
			strcat(title, "·¢ËÍÊı¾İ");
			break;
		case SG_ERR_RCV_ANS:
			strcat(title, "½ÓÊÕÓ¦´ğ");
			break;
		case SG_ERR_TIME_OUT:
			strcat(title, "³¬Ê±´íÎó£¬\n");
			switch(g_state_info.nNetTimeOut)
			{
				case GPRS_ACTIVE_PDP:
					strcat(title,"Active PDPÉÏÏÂÎÄ");
					break;
				case GPRS_DEACTIVE_PDP:
					strcat(title,"DeActive PDPÉÏÏÂÎÄ");
					break;	
				case GPRS_PORXY:
					strcat(title,"Á¬½Ó´úÀí");
					break;
				case GPRS_HANDUP:
					strcat(title,"ÎÕÊÖ");
					break;
				case GPRS_RXHEAD:
				case GPRS_RXCONTENT:
					strcat(title,"½ÓÊÕ±¨ÎÄÄÚÈİ");
					break;
				case GPRS_SOCKET_CONNECT:
					strcat(title,"Á¬½ÓSOCKET");
					break;
			}
			strcat(title, "³¬Ê±!\r\n");
			break;
			
		case SG_ERR_CLOSE_SOCKET:
			strcat(title, "¹Ø±ÕSOCKET");
			break;
	
		case SG_ERR_HOST_PARSE:
			strcat(title, "Ö÷»úµØÖ·½âÎö");
			break;
		case SG_ERR_ACTIVE_PDP:
			strcat(title, "¼¤»îPDPÉÏÏÂÎÄ");
			break;
		case SG_ERR_DEACTIVE_PDP:
			strcat(title, "È¥»îPDPÉÏÏÂÎÄ");
			break;
		case SG_ERR_SET_OPT:
			strcat(title, "²ÎÊıÉèÖÃ");
			break;
		default:
			break;
	}

	strcat(title, "\r\n");

	// µ±Ç°ÀÛ¼ÆÖØÁ¬´ÎÊı
	sprintf(title+strlen(title), "ÀÛ¼ÆÖØÁ¬´ÎÊı:%ld´Î\r\n",ReConCnt);
	sprintf(title+strlen(title), "ÍøÂç×´Ì¬:%d\r\n",g_state_info.plmn_status);
	

	SG_DynMenu_Put_MsgBox (menuid, title);
				
	return 1;

}




/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Check_Device
  ¹¦  ÄÜ  £º³µÁ¾×´Ì¬²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º·µ»Ø ¡°GPS OK \r\nGSM VERSION xxx¡¡\r\nAPP VERSION xxxx¡±
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Ôö¼Ó¹ÊÕÏ²éÑ¯ĞÅÏ¢£¬ÀîËÉ·å£¬2005-12-09
  GPS¶¨Î»²éÑ¯¸ÄÎª¼ì²âµ±Ç°×´Ì¬£¬ÀîËÉ·å£¬2006-01-05
****************************************************************/
static int SG_Proc_Check_Device(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	char title[500] = "";
	char net_status = 0;

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	// £Ç£Ğ£ÓÄ£¿éºòÑ¡×´Ì¬£ºÒÑ¶¨Î»¡¡Õı³£¡¡Òì³£
	if(((gps_data *)(g_state_info.pGpsCurrent))->status)// ÒÑ¾­¶¨Î»
	{
		strcpy(title, "GPS ÒÑ¶¨Î»!\r\n");
	}
	else
	{
		strcpy(title, "GPS Î´¶¨Î»!\r\n");
	}

	sprintf(title+strlen(title), "µ±Ç°ĞÇÊı: %d\r\n",((gps_data *)(g_state_info.pGpsCurrent))->nNum);
	
					
	if (g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF )
		strcat(title, "Ö÷µçÆ½µôµç!\r");

	else if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE )
		strcat(title, "Ö÷µçÆ½Ç·Ñ¹!\r");
		
	strcat(title, "³µÁ¾×´Ì¬:  ");
	
	if(g_state_info.alarmState & ~(GPS_CAR_STATU_POS|GPS_CAR_STATU_ACC))
	{
		sprintf(title+strlen(title), "0x%08x\r",(int)g_state_info.alarmState);
	}
	else
		strcat(title, "Õı³£.\r");

	if(TRUE == SG_Set_Check_CheckSum()){
		strcat(title, "ÅäÖÃÕı³£\r");
	}
	else{
		strcat(title, "ÅäÖÃÒì³£\r");
	}

	if(g_set_info.bAutoAnswerDisable)
	{
		strcat(title,"Ê¹ÓÃÊÖ±úÍ¨»°\r");
	}
	else
	{
		strcat(title,"Ê¹ÓÃÃâÌáÍ¨»°\r");
	}



	if (g_set_info.bRegisted == 0)
		strcat(title, "ÖÕ¶ËÎ´×¢²á!\r");
	
	sprintf(title+strlen(title), "ÀÛ¼ÆÖØÆô´ÎÊı:\r\nAll %d´Î\r\nFormal %d´Î\r\n",g_set_info.nResetCnt,g_set_info.nReConCnt);
	sprintf(title+strlen(title), "ÉÏ´ÎÖØÆôÔ­Òò %d\r\n",Restart_Reason);
	sprintf(title+strlen(title), "ÔËÓªÉÌ %d\r\nĞÅºÅ %d\r\n",g_state_info.opter,g_state_info.rxLevel);
	strcat(title, "IMEI:");
	strcat(title, g_state_info.IMEI);
	strcat(title, "\r\n");
	//sprintf(title+strlen(title), "IMSI %s\r\n",g_state_info.imsi);
	
	net_status = SG_Net_Judge();

	switch(net_status)
	{
		case NO_SIM_CARD:
			strcat(title,"ÎŞSIM¿¨\r\n");
			break;
		case NO_NET_WORK:
			strcat(title,"ÎŞÍøÂç\r\n");
			break;
		case GSM_ONLY:
			strcat(title,"ONLY GSM\r\n");	
			break;
		case GPRS_ONLY:
			strcat(title,"ONLY GPRS\r\n");	
			break;
		case NETWORK_OK:
			strcat(title,"ÍøÂçÕı³£\r\n");	
			break;
		default:
			break;
	}

	if(g_set_info.bNewUart4Alarm & UT_GPS)
	{
		if(g_set_info.GpsDevType == 1)
		{
			
			strcat(title,"ÆôÓÃZBµ¼º½ÆÁ");
		}
		else if(g_set_info.GpsDevType == 2)
		{
			
			strcat(title,"ÆôÓÃYHµ¼º½ÆÁ");			
		}
	}
#if (0)
	{
		int space;
		space = EFS_GetFreeSpace();
		sprintf(title+strlen(title),"ÎÄ¼şÏµÍ³Ê£Óà¿Õ¼ä %d,",space);
		space = EFS_GetUsedSpace();
		sprintf(title+strlen(title),"ÒÑ¾­Ê¹ÓÃ¿Õ¼ä %d\r\n",space);
	}
#endif /* (0) */

	SG_DynMenu_Put_MsgBox (menuid, title);
				
	return 1;

}



/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Theft
  ¹¦  ÄÜ  £º·ÀµÁ¹¦ÄÜ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Æû³µµã»ğÊ±½ûÖ¹ ´ò¿ª·ÀµÁ¹¦ÄÜ£¬ÀîËÉ·å£¬2005-12-15
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count < 2)
	{
		menuList->item->count = 2;

		if (g_set_info.nTheftState == 0)
		{
					
			if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
			{
				menuList->item->count = 0;			
				SG_DynMenu_Put_MsgBox (menuid, "ÇëÏÈÏ¨»ğÔÙÊ¹ÓÃ·ÀµÁ¹¦ÄÜ!");	
				return 1;

			}
		
			if (g_set_info.bTheftDisable == 1)
			{
				menuList->item->count = 0;			
				SG_DynMenu_Put_MsgBox (menuid, "·ÀµÁ¹¦ÄÜ±»½ûÓÃ£¬ÇëÁªÏµÖĞĞÄ¿ªÍ¨!");	
				return 1;
			}	
			
			 if(g_set_info.bTheftDisable == 0)
				SG_DynMenu_Put_ChooseBox (menuid, "¿ªÆôµã»ğ·ÀµÁ¹¦ÄÜ?");
				
		}	
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õ·ÀµÁ¹¦ÄÜ?");

		return 1;
	}

	//²Ù×÷È·ÈÏ
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
					SG_DynMenu_Put_ChooseBox (menuid, "³É¹¦´ò¿ªµã»ğ·ÀµÁ¹¦ÄÜ");
				}	
			}	
			else
			{
				if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
				{
					menuList->item->count = 0;			
					SG_DynMenu_Put_MsgBox (menuid, "ÇëÏÈÏ¨»ğÔÙÊ¹ÓÃ·ÀµÁ¹¦ÄÜ!");	
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
				SG_DynMenu_Put_MsgBox (menuid, "·ÀµÁ³É¹¦¹Ø±Õ!");			

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
  º¯ÊıÃû£ºSG_Proc_Medical_Service
  ¹¦  ÄÜ  £ºÒ½ÁÆ·şÎñ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  ÔÚ²Ëµ¥ÖĞ·¢ËÍÈÎÒâÊ±¼ä³öÏÖµÄ²Ëµ¥ÀàĞÍÊ±ĞèÒªÏÈ·¢¿Õ²Ëµ¥£¬ÀîËÉ·å£¬2005-12-08
****************************************************************/
static int SG_Proc_Medical_Service(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	
	if (s1 != NULL || s2 != NULL)
		return 0;

//	SG_DynMenu_Put_MenuList (NULL);							

	SG_DynMenu_Put_VoiceBox(menuid, "²¦´òÒ½ÁÆÇóÖúºÅÂë?",  g_set_info.sMedicalNo,  "Ò½ÁÆÇóÖú");
	
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Help_Service
  ¹¦  ÄÜ  £ºÎ¬ĞŞ·şÎñ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  ÔÚ²Ëµ¥ÖĞ·¢ËÍÈÎÒâÊ±¼ä³öÏÖµÄ²Ëµ¥ÀàĞÍÊ±ĞèÒªÏÈ·¢¿Õ²Ëµ¥£¬ÀîËÉ·å£¬2005-12-08  
****************************************************************/
static int SG_Proc_Help_Service(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	if (s1 != NULL || s2 != NULL)
		return 0;
	
//	SG_DynMenu_Put_MenuList (NULL);							
	SG_DynMenu_Put_VoiceBox(menuid, "²¦´òÎ¬ĞŞÇóÖúºÅÂë?",  g_set_info.sServiceNo,  "Î¬ĞŞÇóÖú");


	
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Hot_Service
  ¹¦  ÄÜ  £º·şÎñÈÈÏß²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  ÔÚ²Ëµ¥ÖĞ·¢ËÍÈÎÒâÊ±¼ä³öÏÖµÄ²Ëµ¥ÀàĞÍÊ±ĞèÒªÏÈ·¢¿Õ²Ëµ¥£¬ÀîËÉ·å£¬2005-12-08
****************************************************************/
static int SG_Proc_Hot_Service(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	if (s1 != NULL || s2 != NULL)
		return 0;
	
//	SG_DynMenu_Put_MenuList (NULL);							
	SG_DynMenu_Put_VoiceBox(menuid, "²¦´ò·şÎñÈÈÏß?",  g_set_info.sHelpNo,  "·şÎñÈÈÏß");


	return 1;
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_GPS_Info
  ¹¦  ÄÜ  £ºGPSĞÅÏ¢²éÑ¯²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Ôö¼Óµ±Ç°ÎŞ¶¨Î»ÌáÊ¾£¬ÏÔÊ¾×î½ü¶¨Î»ĞÅÏ¢£¬
  ÀîËÉ·å£¬2005-12-02
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
		strcpy(title, "µ±Ç°GPSÎŞ¶¨Î»\r×î½ü¶¨Î»ĞÅÏ¢:\r\n");
	}	
	else
	{
		strcpy(title,"GPSÒÑ¶¨Î»\rµ±Ç°¶¨Î»ĞÅÏ¢:\r\n");
	}
	strcat(title, "gps ËÙ¶È:");
	sscanf(gps->sSpeed,"%f",&m);
	m = m*1.852;
	sprintf(buf,"%0.2f",m);
	strcat(title,buf);
	strcat(title, " km/h\r\n");

	strcat(title, "¿É¼ûĞÇÊı:");
	sprintf(title+strlen(title), "%d ¿Å", gps->nNum);
	strcat(title, "\r\n");

	
	strcat(title, "½Ç¶È:");
	strcat(title, gps->sAngle);
	strcat(title, "\r\n");

	strcat(title, "¾­¶È:");
	strcat(title, gps->sLongitude);
	strcat(title, gps->sSouthNorth);
	strcat(title, "\r\n");
	
	strcat(title, "Î³¶È:");
	strcat(title, gps->sLatitude);
	strcat(title, gps->sEastWest);
	strcat(title, "\r\n");


	strcat(title, "¸ß¶È:");
	strcat(title, gps->sAltitude);
	strcat(title, " Ã×\r\n");


	strcat(title, "UTCÊ±¼ä:");
	strcat(title, gps->sTime);
	strcat(title, "\r\n");

	strcat(title, "UTCÈÕÆÚ:");
	strcat(title, gps->sDate);
	strcat(title, "\r\n");
	
	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Cacular_Total_Distance
  ¹¦  ÄÜ  £º×ÜÀï³ÌÍ³¼Æ
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £º³Âº£»ª
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2006-5-23
****************************************************************/
static int SG_Proc_Cacular_Total_Distance(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;
	sprintf(str,"%.3f¹«Àï\r\n",(double)g_state_info.nGetTotalDistance/1000);	

	SG_DynMenu_Put_MsgBox (menuid, str);
	
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Total_Distance
  ¹¦  ÄÜ  £º×ÜÀï³ÌÍ³¼ÆÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
		SG_DynMenu_Put_InputBox (menuid, "×ÜÀï³Ì:", NULL, DECRYPT_BUF, 9, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		g_watch_info.nGetTotalDistance = atol(s1);
		g_state_info.nGetTotalDistance = g_watch_info.nGetTotalDistance;
		
		SG_Watch_Save();

		SG_Net_Disconnect();
	
		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		SG_Menu_Clear_SuperLogin();

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Cacular_Distance1
  ¹¦  ÄÜ  £ºÀï³Ì1  µÄÍ³¼Æ
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £º³Âº£»ª
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2006-5-23
****************************************************************/
static int SG_Proc_Cacular_Distance1(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;

	sprintf(str,"%.3f¹«Àï",(double)g_state_info.nGetDistance[0]/1000);	
	SG_DynMenu_Put_MsgBox (menuid, str);

	return 1;
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Cacular_Distance2
  ¹¦  ÄÜ  £ºÀï³Ì2  µÄÍ³¼Æ
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £º³Âº£»ª
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2006-5-23
****************************************************************/
static int SG_Proc_Cacular_Distance2(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	sprintf(str,"%.3f¹«Àï",(double)g_state_info.nGetDistance[1]/1000);	
	SG_DynMenu_Put_MsgBox (menuid, str);
	return 1;
}
/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Clr_Distance1
  ¹¦  ÄÜ  £ºÀï³Ì1  µÄÍ³¼ÆÇåÁã
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £º³Âº£»ª
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2006-5-23
****************************************************************/
static int SG_Proc_Clr_Distance1(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;

	g_watch_info.nGetDistance[0]=0;
	g_state_info.nGetDistance[0]=0;
	SG_Watch_Save();

	sprintf(str,"Àï³Ì1ÒÑÇåÁã");	
	SG_DynMenu_Put_MsgBox (menuid, str);
	
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Clr_Distance2
  ¹¦  ÄÜ  £ºÀï³Ì2  µÄÍ³¼ÆÇåÁã
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £º³Âº£»ª
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2006-5-23
****************************************************************/
static int SG_Proc_Clr_Distance2(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char str[100];

	if (s1 != NULL || s2 != NULL)
		return 0;
	
	g_watch_info.nGetDistance[1]=0;
	g_state_info.nGetDistance[1]=0;
	SG_Watch_Save();

	sprintf(str,"Àï³Ì2ÒÑÇåÁã");	
	SG_DynMenu_Put_MsgBox (menuid, str);
	
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_OneGPS
  ¹¦  ÄÜ  £ºÒ»¼üµ¼º½ÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  ÊÖ¶¯ÉèÖÃ±¾»úºÅºó»½ĞÑºóÌ¨½ÓÊÕºÍÃÜÔ¿½»»»Ïß³Ì£¬ÀîËÉ·å£¬2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "Ò»¼üµ¼º½ºÅÂë:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sOneGPSNo, s1);
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_AlarmSms
  ¹¦  ÄÜ  £º¶ÌĞÅ±¨¾¯ºÅÂëÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  ÊÖ¶¯ÉèÖÃ±¾»úºÅºó»½ĞÑºóÌ¨½ÓÊÕºÍÃÜÔ¿½»»»Ïß³Ì£¬ÀîËÉ·å£¬2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "¶ÌĞÅ±¨¾¯ºÅÂë1:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		memset(g_set_info.sSmsAlarmNo1,0,sizeof(g_set_info.sSmsAlarmNo1));
		strcpy(g_set_info.sSmsAlarmNo1, s1);
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_AlarmSms2
  ¹¦  ÄÜ  £º¶ÌĞÅ±¨¾¯ºÅÂëÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  ÊÖ¶¯ÉèÖÃ±¾»úºÅºó»½ĞÑºóÌ¨½ÓÊÕºÍÃÜÔ¿½»»»Ïß³Ì£¬ÀîËÉ·å£¬2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "¶ÌĞÅ±¨¾¯ºÅÂë2:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		memset(g_set_info.sSmsAlarmNo2, 0, sizeof(g_set_info.sSmsAlarmNo2));
		strcpy(g_set_info.sSmsAlarmNo2, s1);
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Call_Out
  ¹¦  ÄÜ  £º½ûÖ¹ºô³ö²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.bCallOutDisable == 1)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õºô³öÏŞÖÆ?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "¿ªÆôºô³öÏŞÖÆ?");

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			if (g_set_info.bCallOutDisable == 1)
			{
				g_set_info.bCallOutDisable = 0;
				SG_DynMenu_Put_MsgBox (menuid, "ºô³öÏŞÖÆÒÑ¹Ø±Õ!");
			}
			else
			{
				g_set_info.bCallOutDisable = 1;
				SG_DynMenu_Put_MsgBox (menuid, "ºô³öÏŞÖÆÆôÓÃ!");			
			}	

			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Call_In
  ¹¦  ÄÜ  £º½ûÖ¹ºôÈë²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.bCallInDisable == 1)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕºôÈëÏŞÖÆ?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "¿ªÆôºôÈëÏŞÖÆ?");

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			
			if (g_set_info.bCallInDisable == 1)
			{
				g_set_info.bCallInDisable = 0;
				SG_DynMenu_Put_MsgBox (menuid, "ºôÈëÏŞÖÆÒÑ¹Ø±Õ!");
			}
			else
			{
				g_set_info.bCallInDisable = 1;
				SG_DynMenu_Put_MsgBox (menuid, "ºôÈëÏŞÖÆÆôÓÃ!");			
			}
			
			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Auto_Call
  ¹¦  ÄÜ  £º×Ô¶¯½ÓÌıÉèÖÃ
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.GpsAutoCall == 0)
			SG_DynMenu_Put_ChooseBox (menuid, "¿ªÆô×Ô¶¯½ÓÌı?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õ×Ô¶¯½ÓÌı?");

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			
			if (g_set_info.GpsAutoCall == 0)
			{
				g_set_info.GpsAutoCall = 1;
				SG_DynMenu_Put_MsgBox (menuid, "×Ô¶¯½ÓÌıÒÑÆôÓÃ!");
			}
			else
			{
				g_set_info.GpsAutoCall = 0;
				SG_DynMenu_Put_MsgBox (menuid, "×Ô¶¯½ÓÌı¹Ø±Õ!");			
			}
			
			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Heart_Switch
  ¹¦  ÄÜ  £ºÉèÖÃĞÄÌø¿ª¹Ø
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.Heart_Switch == 0)
			SG_DynMenu_Put_ChooseBox (menuid, "¿ªÆôĞÄÌø¹¦ÄÜ?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕĞÄÌø¹¦ÄÜ?");

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			SG_Set_Cal_CheckSum();
			
			if (g_set_info.Heart_Switch == 0)
			{
				g_set_info.Heart_Switch = 1;
				SG_DynMenu_Put_MsgBox (menuid, "ĞÄÌø¹¦ÄÜÆôÓÃ!");
			}
			else
			{
				g_set_info.Heart_Switch = 0;
				SG_DynMenu_Put_MsgBox (menuid, "ĞÄÌø¹¦ÄÜ¹Ø±Õ!");			
			}
			
			SG_Set_Save();
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_ShortCut
  ¹¦  ÄÜ  £º¿ì½İ¼ü»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£ºkey:¼üID
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Ôö¼ÓÇÀ´ğĞÅÏ¢·¢ËÍ½á¹ûÌáÊ¾£¬ÀîËÉ·å£¬2005-12-05
  Ôö¼ÓÇÀ´ğ½ûÖ¹ÅĞ¶Ï£¬ÀîËÉ·å£¬2006-01-11
****************************************************************/
int SG_Proc_ShortCut(unsigned char key)
{
	MsgUChar *msg = NULL;
	MsgInt msgLen;
	int menuid = MENUID_SINGLE;

	switch (key)
	{
		case SHORTCUT_MEDICAL:

			SG_DynMenu_Put_VoiceBox((unsigned char*)(&menuid), "²¦´òÒ½ÁÆÇóÖúºÅÂë?",  g_set_info.sMedicalNo,  "Ò½ÁÆÇóÖú");

			break;
			
		case SHORTCUT_SERVICE:
			
			SG_DynMenu_Put_VoiceBox((unsigned char*)(&menuid), "²¦´òÎ¬ĞŞÇóÖúºÅÂë?",  g_set_info.sServiceNo,  "Î¬ĞŞÇóÖú");
		
			break;

		//µçÕÙÇÀ´ğ	
		case SHORTCUT_INFO:
#if (0)
			if(g_set_info.bQuickAnswer==1)
			{
				g_set_info.ttsflag=1;
				SG_DynMenu_Put_AutoShowBox((unsigned char*)&menuid, "²¥±¨ĞÅÏ¢·¢ËÍ³É¹¦!µÈ´ıÏìÓ¦...");	
				g_set_info.ttsflag=0;
				SG_Net_Voice_Snd_Msg(MSG_SND_VOICE,MSG_VOICE_SET_COMBIN,g_tts_buf,g_tts_buflen);
				break;
			}
			
			if (g_set_info.bMusterDisable == 1)
			{
 				SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "ÇÀ´ğÒÑ±»½ûÖ¹!");
				break;
			}

			if (MsgMusterFirst(&msg, &msgLen) == MSG_TRUE)
			{
				SG_Send_Put_New_Item(1, msg, msgLen);
				SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "ÇÀ´ğĞÅÏ¢·¢ËÍ³É¹¦!µÈ´ıÏìÓ¦...");				
			}
			else
			{
				SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "ÇÀ´ğĞÅÏ¢·¢ËÍÊ§°Ü!");
			}
#endif /* (0) */
			SG_DynMenu_Put_VoiceBox((unsigned char*)(&menuid), "²¦´ò·şÎñÈÈÏß?",  g_set_info.sHelpNo,  "·şÎñÈÈÏß");

			break;
	
		default:
			break;
	}
	
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Ver
  ¹¦  ÄÜ  :°æ±¾ÇĞ»»
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Í¨Ñ¶·½Ê½SMS¸Ä³ÉGPRSÊ±¶ÏÏßÖØÁ¬£¬ÀîËÉ·å£¬2005-12-20
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_state_info.user_type == 1) 
			SG_DynMenu_Put_ChooseBox (menuid, "ÇĞ»»ÎªË½¼Ò³µ°æ±¾");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "ÇĞ»»ÎªÆÕÍ¨°æ±¾");

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{

			if (g_state_info.user_type == 1) // ½ûÖ¹×Ô¶¯½ÓÌı
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
				SG_DynMenu_Put_MsgBox (menuid, "ÆôÓÃË½¼Ò³µ°æ±¾!");	
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
				SG_DynMenu_Put_MsgBox (menuid, "ÆôÓÃÆÕÍ¨°æ±¾!");			
			}
			SG_Soft_Reset(2);
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}

}



/****************************************************************
  º¯ÊıÃû£ºSG_Proc_AutoAnswer
  ¹¦  ÄÜ  £º×Ô¶¯½ÓÌı²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £º¶­¼ÑÖ¦
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2006-2-12
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		if (g_set_info.bAutoAnswerDisable == 0) // ÔÊĞí×Ô¶¯½ÓÌı
			SG_DynMenu_Put_ChooseBox (menuid, "Ê¹ÓÃÊÖ±úÍ¨»°?");			
		else
			SG_DynMenu_Put_ChooseBox (menuid, "Ê¹ÓÃÃâÌáÍ¨»°?");

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{

			SG_Set_Cal_CheckSum();
			if (g_set_info.bAutoAnswerDisable == 1) // ½ûÖ¹×Ô¶¯½ÓÌı
			{
				g_set_info.bAutoAnswerDisable = 0;
				SG_DynMenu_Put_MsgBox (menuid, "ÆôÓÃÃâÌáÍ¨»°!");
			}
			else
			{
				g_set_info.bAutoAnswerDisable = 1;
				SG_DynMenu_Put_MsgBox (menuid, "ÆôÓÃÊÖ±úÍ¨»°!");			
			}
			
			SG_Set_Save();
			//SG_Notify_Hh();
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Test_TTS
  ¹¦  ÄÜ  £º²âÊÔÓïÒôĞ¾Æ¬TTS
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
		SG_DynMenu_Put_MsgBox (menuid, "Çë¿ªÆôTTS");
		return 1;
	}

	SG_Voice_Snd_Msg(MSG_VOICE_SET_COMBIN, TTSMsg, 4);
	SG_DynMenu_Put_MsgBox (menuid, "ÇëÊÔÌı..........");
	return 1;
	
}

static int SG_Proc_Check_AD(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[50]="";

	if (s1 != NULL || s2 != NULL)
		return 0;

	sprintf(title,"µ±Ç°ÓÍÏäAD Öµ:\r\n %d ",g_state_info.CurrAD/100);

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}

//ÅÄÉãÍ¼Ïñ°²×°µ÷ÊÔ
static int SG_Proc_Pictrue_Catch(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[200]="";

	if (s1 != NULL || s2 != NULL)
		return 0;
	if(!(g_set_info.bNewUart4Alarm & (UT_XGCAMERA|UT_QQCAMERA)))
	{
		
		strcat(title,"ÉãÏñÍ·Î´¿ªÆô£¬Çë¿ªÆôÉãÏñÍ·!!");
	}
	else
	{
		char sig[3];
		
		xSignalHeaderRec      *signal_ptr = NULL;
		sig[0] = 3;
		sig[1] = CAMERA_ONE_REQ;
		sig[2] = PIC_NOT_SAVE;
		SG_CreateSignal_Para(SG_CAMERA_ONE_ASK,3, &signal_ptr,sig); // ·¢ËÍĞÅºÅµ½Ö÷Ñ­»·ÒªÇó´¦Àí·¢ËÍ¶ÓÁĞÖĞµÄ±¨ÎÄ
		strcat(title,"²âÊÔÅÄÉãÒ»ÕÅÍ¼Æ¬!!\r\n");
		sprintf(title + strlen(title),"³É¹¦ÅÄÉã%dÕÅ!!",g_state_info.photo_catch_cnt);		
	
	}

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Test_LED
  ¹¦  ÄÜ  £º²âÊÔLED
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
		SG_DynMenu_Put_MsgBox (menuid, "Çë¿ªÆôLED");
		return 1;
	}
	else
	{
		char ledmsg[64]={0};
		int ledlen=0;
		ledmsg[0]=0x0a;
		strcpy(&ledmsg[1],"$$00S0FF0102ÎÒ±»´ò½Ù,Çë±¨¾¯&&");
		ledlen=strlen(ledmsg);
		ledmsg[ledlen]=0x0d;
		
		if(g_set_info.bNewUart4Alarm & UT_LED)
			SIO_WriteFrame(COM_DEBUG,ledmsg,ledlen+1);	
		else if(g_set_info.bNewUart4Alarm & UT_LED)
			SIO_WriteFrame(COM2,ledmsg,ledlen+1);	
		
		SG_DynMenu_Put_MsgBox (menuid, "ÎÒ±»´ò½Ù,Çë±¨¾¯");
		return 1;
	}

}


/******************************************************************
º¯ÊıÃû: SG_Proc_Test_OpenLock
¹¦ÄÜ:²âÊÔÖĞ¿ØËø¿ªËø
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
		SG_DynMenu_Put_MsgBox (menuid, "Çë¿ªÆôÖĞ¿ØËø");
		return 1;
	}
	g_state_info.screen=0x00;
	g_state_info.set=0x00;
	
	SG_DynMenu_Put_MsgBox (menuid, "²Ù×÷Íê³É");
	return 1;
}
/********************************************************************
º¯ÊıÃû:SG_Proc_Test_CloseLock
¹¦ÄÜ:	    ²âÊÔÖĞ¿ØËø±ÕËø
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
		SG_DynMenu_Put_MsgBox (menuid, "Çë¿ªÆôÖĞ¿ØËø");
		return 1;
	}
	g_state_info.screen=0x00;
	g_state_info.set=0x80;
	SG_DynMenu_Put_MsgBox (menuid, "²Ù×÷Íê³É");
	return 1;
}



/****************************************************************
  º¯ÊıÃû£ºSG_Proc_SpeakVolChg
  ¹¦  ÄÜ  £ºÒôÆµµ÷Õû²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £º¶­¼ÑÖ¦
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2006-2-12
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
		SG_DynMenu_Put_InputBox (menuid, "ÒôÁ¿ÉèÖÃ(1~7):", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int vol = 0;
				
		menuList->item->count = 0;		

		vol = atoi(s1);

		if(vol < 1 || vol > 7)
		{
			menuList->item->count = 0;
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
			return 1;		
		}
		
		SG_Set_Cal_CheckSum();
		g_set_info.speaker_vol = vol;
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");
				
	return 1;
	}	
	else
	{
		menuList->item->count = 0;

		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}


				


				
/****************************************************************
  º¯ÊıÃû£ºSG_Proc_IO_Check_State
  ¹¦  ÄÜ  £º¼ì²âIO¼°¸÷½ÓÏß×´Ì¬
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º·µ»ØAcc×´Ì¬ ¼Æ¼Û×´Ì¬ ³µÃÅ×´Ì¬ ÊÖ¶¯±¨¾¯×´Ì¬ Ï¨»ğ²âÊÔ 
  ±àĞ´Õß  £º¹ù±ÌÁ«
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨ 2007/10/10 
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
		
		strcpy(title,"ÊÊÅäÆ÷IO:");

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
		strcpy(title,"ÖĞ¿ØËø: Î´¿ªÆô");	
		strcat(title,"\r\n");
	#endif
	
	//sdbj
	if(g_state_info.alarmState & GPS_CAR_STATU_HIJACK)
	{
		sprintf(buf, "%s%s\r\n", "ÊÖ¶¯±¨¾¯:", "¿ª");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "ÊÖ¶¯±¨¾¯:", "¹Ø");
		strcat(title, buf);
	}
	//cmkg
	if(!(g_set_info.alarmSet & GPS_CAR_STATU_IO1))// ×Ô¶¨Òå±¨¾¯¸ßµçÆ½´¥·¢
	{
		strcat(title,"×Ô¶¨Òå±¨¾¯¶Ë¿Ú:\r\n¸ßµçÆ½´¥·¢\r\n");
	}
	else
	{
		strcat(title,"×Ô¶¨Òå±¨¾¯¶Ë¿Ú:\r\nµÍµçÆ½´¥·¢\r\n");
	}

	if(g_state_info.alarmState & GPS_CAR_STATU_IO1)
	{
		strcat(title,"×Ô¶¨Òå±¨¾¯¶Ë¿Ú:¿ª\r\n");
	}
	else
	{
		strcat(title,"×Ô¶¨Òå±¨¾¯¶Ë¿Ú:¹Ø\r\n");
	}

#if (0)
	if(g_state_info.alarmState & GPS_CAR_STATU_DOOR)
	{	
		sprintf(buf, "%s%s\r\n", "³µÃÅ±¨¾¯:", "¿ª");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "³µÃÅ±¨¾¯:", "¹Ø");
		strcat(title, buf);
	}
#endif /* (0) */
	if(g_set_info.bBatteryCheckEnable)
	{
		strcat(title,"¼Æ¼ÛÆ÷ĞÅºÅ×öµôµç¼ì²â\r\n");	
		if(g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF)
		{
			sprintf(buf, "%s%s\r\n", "¼Æ¼ÛÆ÷ĞÅºÅ:", "¿ª");
			strcat(title, buf);
		}
		else
		{
			sprintf(buf, "%s%s\r\n", "¼Æ¼ÛÆ÷ĞÅºÅ:", "¹Ø");
			strcat(title, buf);
		}
	}
	else
	{
		strcat(title,"¼Æ¼ÛÆ÷ĞÅºÅ×öÆÕÍ¨Ê¹ÓÃ\r\n");	
		if(g_state_info.alarmState & GPS_CAR_STATU_JJQ)
		{
			sprintf(buf, "%s%s\r\n", "¼Æ¼ÛÆ÷ĞÅºÅ:", "¿ª");
			strcat(title, buf);
		}
		else
		{
			sprintf(buf, "%s%s\r\n", "¼Æ¼ÛÆ÷ĞÅºÅ:", "¹Ø");
			strcat(title, buf);
		}
	}
	//jjq	
	if(g_set_info.JJQLev == 0)// ×Ô¶¨Òå±¨¾¯¸ßµçÆ½´¥·¢
	{
		strcat(title,"¼Æ¼ÛÊäÈëĞÅºÅ:\r\n¸ßµçÆ½´¥·¢\r\n");
	}
	else
	{
		strcat(title,"¼Æ¼ÛÊäÈëĞÅºÅ:\r\nµÍµçÆ½´¥·¢\r\n");
	}

#if (0)
	if(g_state_info.alarmState & GPS_CAR_STATU_JJQ)
	{
		sprintf(buf, "%s%s\r\n", "¼Æ¼ÛÆ÷ĞÅºÅ:", "¿ª");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "¼Æ¼ÛÆ÷ĞÅºÅ:", "¹Ø");
		strcat(title, buf);
	}
#endif /* (0) */

	//acc
	if(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)
	{
		sprintf(buf, "%s%s\r\n", "ACCĞÅºÅ:", "¿ª");
		strcat(title, buf);
	}
	else 
	{
		sprintf(buf, "%s%s\r\n", "ACCĞÅºÅ:", "¹Ø");
		strcat(title, buf);
	}
		
	//VCAR_PFO
	if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE)
	{
		sprintf(buf, "%s%s\r\n", "µçÆ¿:", "Ç·Ñ¹");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "µçÆ¿:", "Õı³£");
		strcat(title, buf);
	}
			
			
	SG_DynMenu_Put_MsgBox (menuid, title);
	return 1;

}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_IO_Level
  ¹¦  ÄÜ  £ºÉèÖÃ×Ô¶¨Òå±¨¾¯io¿ÚµÄ´¥·¢µçÆ½	
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  	£º¹ù±ÌÁ«
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2007/10/11 
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(!(g_set_info.alarmSet & GPS_CAR_STATU_IO1))
			SG_DynMenu_Put_ChooseBox (menuid, "ÉèÖÃÎªµÍµçÆ½´¥·¢ ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "ÉèÖÃÎª¸ßµçÆ½´¥·¢ ?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
				SG_DynMenu_Put_MsgBox (menuid, "µ±Ç°Îª¸ßµçÆ½´¥·¢!");
				
			}
			else
			{
				SG_Set_Cal_CheckSum();
				g_set_info.alarmSet |= GPS_CAR_STATU_IO1;
				SG_Set_Save();
				SG_DynMenu_Put_MsgBox (menuid, "µ±Ç°ÎªµÍµçÆ½´¥·¢!");
			}

		}
		else
		{
			menuList->item->count = 0;
			
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_JJQ_Lev
  ¹¦  ÄÜ  £ºÉèÖÃ¼Æ¼ÛÊäÈëµÄ´¥·¢µçÆ½	
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  	£º¹ù±ÌÁ«
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2007/10/11 
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_set_info.JJQLev == 0)
			SG_DynMenu_Put_ChooseBox (menuid, "ÉèÖÃÎªµÍµçÆ½´¥·¢ ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "ÉèÖÃÎª¸ßµçÆ½´¥·¢ ?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
				SG_DynMenu_Put_MsgBox (menuid, "µ±Ç°Îª¸ßµçÆ½´¥·¢!");
				
			}
			else
			{
				SG_Set_Cal_CheckSum();
				g_set_info.JJQLev = 1;
				SG_Set_Save();
				SG_DynMenu_Put_MsgBox (menuid, "µ±Ç°ÎªµÍµçÆ½´¥·¢!");
			}

		}
		else
		{
			menuList->item->count = 0;
			
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}



/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Battery_Check
  ¹¦  ÄÜ  £º¼Æ¼Û¸´ÓÃ
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  	£º¹ù±ÌÁ«
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2007/10/11 
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
			SG_DynMenu_Put_ChooseBox (menuid, "Ê¹ÓÃµôµç±¨¾¯?");					
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õµôµç±¨¾¯?");
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

			SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

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
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	return 1;

	
}
#ifdef _DUART
/****************************************************************
  º¯ÊıÃû£ºSG_Proc_VOLT_Lock
  ¹¦  ÄÜ  £ºµçÑ¹½âËø		
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  	£ºÁÖÕ×½£
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2008/11/21
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
		SG_DynMenu_Put_MsgBox (menuid, "Çë¿ªÆôÖØ¹¤Ëø³µ");
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
	//SG_DynMenu_Put_MsgBox (menuid, "½âËø³É¹¦");

	g_pro_lock.nType = PRO_VOLT_UNLOCK;
	//g_pro_lock.nCarNum = 0x01;
	//g_pro_lock.timeout = 0;
	//g_pro_lock.ack = 0;
	g_pro_lock.VoltTime = 1;
	SG_Rcv_Safety_VOLT_UnLock();
		SG_DynMenu_Put_MsgBox (menuid, "½âËø³É¹¦");
	//g_pro_lock.state = SG_MENU_CTRL;
	
	menuList->item->count = 0;
	
	return 1;
	
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_VOLT_Lock
  ¹¦  ÄÜ  £ºµçÑ¹Ëø³µ		
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  	£ºÁÖÕ×½£
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2008/11/21
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
		SG_DynMenu_Put_MsgBox (menuid, "Çë¿ªÆôÖØ¹¤Ëø³µ");
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
	//SG_DynMenu_Put_MsgBox (menuid, "Ëø³µ³É¹¦");

	g_pro_lock.nType = PRO_VOLT_LOCK;
	//g_pro_lock.nCarNum = 0x01;
	//g_pro_lock.timeout = 0;app
	//g_pro_lock.ack = 0;
	g_pro_lock.VoltTime = 1;
	SG_Rcv_Safety_VOLT_Lock();
	SG_DynMenu_Put_MsgBox (menuid, "Ëø³µ³É¹¦");
	//g_pro_lock.state = SG_MENU_CTRL;
	
	menuList->item->count = 0;
	
	return 1;
	
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_One_Lock
  ¹¦  ÄÜ  £ºÒ»¼¶Ëø³µ		
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  	£ºÁÖÕ×½£
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2008/11/11
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
		SG_DynMenu_Put_MsgBox (menuid, "Çë¿ªÆôÖØ¹¤Ëø³µ");
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
	SG_DynMenu_Put_MsgBox (menuid, "Ò»¼¶Ëø³µ,µÈ´ıÖĞ...");

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
  º¯ÊıÃû£ºSG_Proc_Two_Lock
  ¹¦  ÄÜ  £º¶ş¼¶Ëø³µ		
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  	£ºÁÖÕ×½£
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2008/11/11
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
		SG_DynMenu_Put_MsgBox (menuid, "Çë¿ªÆôÖØ¹¤Ëø³µ");
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
	SG_DynMenu_Put_MsgBox (menuid, "¶ş¼¶Ëø³µ,µÈ´ıÖĞ...");

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
  º¯ÊıÃû£ºSG_Proc_Unioce_Lock
  ¹¦  ÄÜ  £º½âËø		
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  	£ºÁÖÕ×½£
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2008/11/11
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
		SG_DynMenu_Put_MsgBox (menuid, "Çë¿ªÆôÖØ¹¤Ëø³µ");
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
	SG_DynMenu_Put_MsgBox (menuid, "½âËø,µÈ´ıÖĞ...");

	g_pro_lock.nType = PRO_UNCOIL_LOCK;
	g_pro_lock.nCarNum = 0x01;
	g_pro_lock.timeout = 0;
	g_pro_lock.ack = 0;
	g_pro_lock.state = SG_MENU_CTRL;
	SG_Rcv_Safety_Uncoil_Lock();
	//g_xih_ctrl.XIH_State = SG_MENU_CTRL;

	//tryCount = 3;
	//Pic_Send(0xa6); // Ï¨»ğ£¬¹Ø±ÕÓÍÂ·
	menuList->item->count = 0;
	
	return 1;
	
}
		
#endif

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_IO_Level
  ¹¦  ÄÜ  £ºÏ¨»ğ		
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  	£º¹ù±ÌÁ«
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2007/10/11 
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
	SG_DynMenu_Put_MsgBox (menuid, "Ï¨»ğ,µÈ´ıÖĞ...");
	g_xih_ctrl.XIH_State = SG_MENU_CTRL;

	tryCount = 3;
	Pic_Send(0xa6); // Ï¨»ğ£¬¹Ø±ÕÓÍÂ·
	menuList->item->count = 0;
	
	return 1;
	
}
			
/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_IO_Level
  ¹¦  ÄÜ  £ºÏ¨»ğ»Ö¸´		
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  	£º¹ù±ÌÁ«
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2007/10/11 
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
	SG_DynMenu_Put_MsgBox (menuid, "Ï¨»ğ»Ö¸´,µÈ´ıÖĞ...");
	tryCount = 3;
	g_xih_ctrl.XIH_State = SG_MENU_CTRL;
	Pic_Send(0xc5); // Ï¨»ğ»Ö¸´£¬´ò¿ªÓÍÂ·
	menuList->item->count = 0;
	
	return 1;
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Muster_Particular_Ask
  ¹¦  ÄÜ  £ºµ÷¶ÈĞÅÏ¢»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºchenhaihua
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  				 ¸Ä±äµ÷¶ÈĞÅÏ¢Êä³ö·½Ê½ ³ÂÀû 2006-09-06
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

	//Ê×´Îµ÷ÓÃ,²Ù×÷
	if (menuList->item->count == 0)
	{
		if(gn_muster_particular_list<=0){
			sprintf(ch,"ÎŞĞÅÏ¢");
			SG_DynMenu_Put_MsgBox (menuid, ch);
			return 1;

		}
		else{
		menuList->item->count = 1;
		sprintf(ch,"ÊäÈëĞÅÏ¢ºÅ0-%d:",gn_muster_particular_list-1);
		SG_DynMenu_Put_InputBox (menuid, ch, NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);

		}

		return 1;
	}
	//»ñÈ¡ĞÅÏ¢ºÅ
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
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈëĞÅÏ¢ºÅ³¬³ö");
		}

		return 1;
	}

	return 1;

}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Show_Device
  ¹¦  ÄÜ  £ºÖÕ¶Ë×´Ì¬²éÑ¯²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
****************************************************************/
static int SG_Proc_Show_Device(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char *title = NULL;

	if (s1 != NULL || s2 != NULL)
		return 0;

	title = SCI_ALLOC( 1024);
		
	if (g_set_info.bRegisted == 0)
		strcpy(title, "Î´×¢²á!\r");
	else
		strcpy(title, "ÒÑ×¢²á!\r");
	
	if (g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF )
		strcat(title, "Ö÷µçÆ½µôµç!\r");

	else if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE )
		strcat(title, "Ö÷µçÆ½Ç·Ñ¹!\r");
	
	#if (__cwt_)
	if (temp_s_udpconn == NULL)
		strcat(title, "GPRS¶Ï!\r");
	else
		strcat(title, "GPRSÍ¨!\r");
	#else
	if (g_state_info.socket <= 0)
		strcat(title, "GPRS¶Ï!\r");
	else
		strcat(title, "GPRSÍ¨!\r");
	#endif		
	strcat(title, "±¾»úºÅ:\r");
	strcat(title, g_set_info.sOwnNo);
	strcat(title, "\r");
	#if(__cwt_)
	strcat(title, "COMMAND ADDR:\r");
	strcat(title, g_set_info.sCommAddr);
	strcat(title, "\r");
	
	strcat(title, "ÊÚÈ¨Âë:\r");
	strcat(title, g_set_info.sOemCode);
	strcat(title, "\r");
	#endif
	strcat(title, "ÖĞĞÄºÅ:\r");
	strcat(title, g_set_info.sCenterNo);
	strcat(title, "\r");

	strcat(title, "Í¨Ñ¶·½Ê½:");
	if (g_set_info.nNetType == MSG_NET_SMS)
		strcat(title, "¶ÌĞÅ");
	else if (g_set_info.nNetType == MSG_NET_GPRS)
		strcat(title, "Êı¾İ");
	else if (g_set_info.nNetType == MSG_NET_UNION)
		strcat(title, "»ìºÏ");
	strcat(title, "\r");
	
	strcat(title, "ÖĞĞÄIP:\r");
	strcat(title, g_set_info.sCenterIp);
	strcat(title, " ");
	sprintf(title+strlen(title), "%d", g_set_info.nCenterPort);
	strcat(title, "\r");
	
	strcat(title, "´úÀíIP:\r");
	strcat(title, g_set_info.sProxyIp);
	strcat(title, " ");
	sprintf(title+strlen(title), "%d", g_set_info.nProxyPort);
	strcat(title, "\r");

	if (g_set_info.bProxyEnable==1)
		strcat(title, "(Ê¹ÓÃ´úÀí)\r");
	else
		strcat(title, "(²»Ê¹ÓÃ´úÀí)\r");

	strcat(title, "APN:");
	strcat(title, g_set_info.sAPN);
	strcat(title, "\r");	
	strcat(title, "USER:");	
	strcat(title, g_set_info.sGprsUser);
	strcat(title, "\r");	
	strcat(title, "PSW:");
	strcat(title, g_set_info.sGprsPsw);	
	strcat(title, "\r");

	strcat(title, "¼à¿Ø·½Ê½:");
	if (g_set_info.nWatchType == MSG_WATCH_TIME)
		strcat(title, "¶¨Ê±");
	else if (g_set_info.nWatchType == MSG_WATCH_AMOUNT)
		strcat(title, "¶¨´Î");
	else if (g_set_info.nWatchType == MSG_WATCH_DISTANCE)
		strcat(title, "¶¨¾à");
	else
		strcat(title, "È¡Ïû");
	strcat(title, "\r");	
	
	if (g_set_info.bCompress == 0)
		strcat(title, "(²»Ñ¹Ëõ");
	else
		strcat(title, "(ÓĞÑ¹Ëõ");	

	if (g_set_info.nInflexionRedeem == 0)
		strcat(title, ",ÎŞ¹Õµã²¹³¥");
	else
		strcat(title, ",ÓĞ¹Õµã²¹³¥");		
	
	if (g_set_info.bStopReport== 0)
		strcat(title, ",Í£³µ²»»ã±¨)\r");
	else
		strcat(title, ")\r");		

	sprintf(title+strlen(title), "¼à¿Ø¼ä¸ô:%d\r", g_set_info.nWatchInterval);
	if(g_set_info.nWatchType == MSG_WATCH_AMOUNT)
	{

		sprintf(title+strlen(title), "¼à¿Ø´ÎÊı:%ld\r", g_set_info.nWatchTime);
		if(g_set_info.nWatchTime == 0)
		{
			sprintf(title+strlen(title), "ÒÑ¼à¿Ø´ÎÊı:%ld\r", g_watch_info.nWatchedTime);
		}
		else
		{
			sprintf(title+strlen(title), "ÒÑ¼à¿Ø´ÎÊı:%ld\r", g_watch_info.nWatchedTime);
		}
	}
	else
 	{

		sprintf(title+strlen(title), "¼à¿ØÊ±¼ä:%ld\r", g_set_info.nWatchTime);
		if(g_set_info.nWatchTime == 0)
		{
			sprintf(title+strlen(title), "ÒÑ¼à¿ØÊ±¼ä:%ld\r", g_watch_info.nWatchedTime);
		}
		else
		{
			sprintf(title+strlen(title), "ÒÑ¼à¿ØÊ±¼ä:%ld\r", g_watch_info.nWatchedTime);
		}
	}
	

	if(g_set_info.bRegionEnable)
	{
		sprintf(title+strlen(title),"ÇøÓò±¨¾¯¿ª\t");
	}
	
	if(g_set_info.bLineEnable)
	{
		sprintf(title+strlen(title), "Â·Ïß±¨¾¯¿ª\r\n");
	}	
 


	sprintf(title+strlen(title),"À©Õ¹´®¿Ú:  0x%x \r ",g_set_info.bNewUart4Alarm);

	
	SG_DynMenu_Put_MsgBox (menuid, title);
	
	if (title)
		SCI_FREE(title);

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_OwnNo
  ¹¦  ÄÜ  £º±¾»úºÅÂëÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  ÊÖ¶¯ÉèÖÃ±¾»úºÅºó»½ĞÑºóÌ¨½ÓÊÕºÍÃÜÔ¿½»»»Ïß³Ì£¬ÀîËÉ·å£¬2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "±¾»úºÅÂë:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}	

	//±£´æ±¾»úºÅÂë
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
		else//ÉèÖÃÖÕ¶ËÒÑ¾­µÇ¼Ç
		{
			g_set_info.bRegisted = 1;
		}
			SG_Set_Save();
		#endif
		g_state_info.randtime = (g_set_info.sOwnNo[9]-'0')*10+ (g_set_info.sOwnNo[10]-'0'); //Ëæ»úÊı
		SG_Net_Disconnect();
	
		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	return 0;
}


#if (__cwt_)
/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_OemCode
  ¹¦  ÄÜ  £º±¾»úºÅÂëÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  ÊÖ¶¯ÉèÖÃ±¾»úºÅºó»½ĞÑºóÌ¨½ÓÊÕºÍÃÜÔ¿½»»»Ïß³Ì£¬ÀîËÉ·å£¬2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "ÊÚÈ¨Âë:", NULL, DECRYPT_BUF, SG_PHONE_LEN, ANYCHAR, ANY_UPLOW, NOCARD);
	}

	//±£´æ±¾»úºÅÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sOemCode, s1);
		memset(g_state_info.sKeepMsg,0,sizeof(g_state_info.sKeepMsg));
		memset(g_state_info.sChkKeepMsg,0,sizeof(g_state_info.sChkKeepMsg));
		SG_Set_Save();
		SG_Net_Disconnect();
		
		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		
	return 1;
}

#endif
		
/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_CenterNo
  ¹¦  ÄÜ  £ºÖĞĞÄºÅÂëÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  ÊÖ¶¯ÉèÖÃ±¾»úºÅºó»½ĞÑºóÌ¨½ÓÊÕºÍÃÜÔ¿½»»»Ïß³Ì£¬ÀîËÉ·å£¬2006-01-11
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
		SG_DynMenu_Put_InputBox (menuid, "ÖĞĞÄºÅÂë:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sCenterNo, s1);
		
		SG_Set_Save();

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Communication_Type
  ¹¦  ÄÜ  £ºÍ¨Ñ¶·½Ê½ÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Í¨Ñ¶·½Ê½SMS¸Ä³ÉGPRSÊ±¶ÏÏßÖØÁ¬£¬ÀîËÉ·å£¬2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "0¶ÌĞÅ1Êı¾İ2»ìºÏ", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char /*mode = 0, */type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type != MSG_NET_UNION && type != MSG_NET_GPRS && type != MSG_NET_SMS)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
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

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Watch_Type
  ¹¦  ÄÜ  £º¼à¿Ø·½Ê½ÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Í¨Ñ¶·½Ê½SMS¸Ä³ÉGPRSÊ±¶ÏÏßÖØÁ¬£¬ÀîËÉ·å£¬2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "1¶¨Ê±3¶¨´Î4È¡Ïû", NULL, DECRYPT_BUF, 1, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char /*mode = 0, */type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type != MSG_WATCH_CANCLE && type != MSG_WATCH_AMOUNT && type != MSG_WATCH_TIME)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
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

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Watch_Interval
  ¹¦  ÄÜ  £º¼à¿Ø¼ä¸ôÊ±¼äÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Í¨Ñ¶·½Ê½SMS¸Ä³ÉGPRSÊ±¶ÏÏßÖØÁ¬£¬ÀîËÉ·å£¬2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "ÉèÖÃ·¶Î§1-65535Ãë", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type == 0 || type >65535)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
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

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_GPS_Interval
  ¹¦  ÄÜ  £ºGPS¹ıÂËÊ±¼ä¼ä¸ôÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Í¨Ñ¶·½Ê½SMS¸Ä³ÉGPRSÊ±¶ÏÏßÖØÁ¬£¬ÀîËÉ·å£¬2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "ÉèÖÃ·¶Î§1-60Ãë", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type == 0 || type >60)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
			return 1;
		}
		
		if (g_set_info.nGpstTime!= (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.nGpstTime= type;				
			SG_Set_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Heart_Interval
  ¹¦  ÄÜ  £ºĞÄÌøÊ±¼ä¼ä¸ôÉèÖÃ²Ëµ¥  Ä¬ÈÏÎª120Ãë
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Í¨Ñ¶·½Ê½SMS¸Ä³ÉGPRSÊ±¶ÏÏßÖØÁ¬£¬ÀîËÉ·å£¬2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "ÉèÖÃ·¶Î§10-300", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type < 10 || type >300)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
			return 1;
		}
		
		if (g_set_info.Heart_Interval != (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.Heart_Interval = type;				
			SG_Set_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_GPS_Dist
  ¹¦  ÄÜ  £ºGPSÆ¯ÒÆ¹ıÂË¾àÀëÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Í¨Ñ¶·½Ê½SMS¸Ä³ÉGPRSÊ±¶ÏÏßÖØÁ¬£¬ÀîËÉ·å£¬2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "ÉèÖÃ·¶Î§5-50Ã×", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type < 5 || type >50)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
			return 1;
		}
		
		if (g_set_info.nGpstDist != (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.nGpstDist = type;				
			SG_Set_Save();
		}

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Efs_Max
  ¹¦  ÄÜ  £ºGPSÆ¯ÒÆ¹ıÂË¾àÀëÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Í¨Ñ¶·½Ê½SMS¸Ä³ÉGPRSÊ±¶ÏÏßÖØÁ¬£¬ÀîËÉ·å£¬2005-12-20
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
		SG_DynMenu_Put_InputBox (menuid, "ÉèÖÃ·¶Î§10-200", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		int type = 0;
				
		menuList->item->count = 0;		

		type = atoi(s1);
		
		if (type < 10 || type > 200)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
			return 1;
		}
		
		if (g_set_info.EfsMax!= (int)type)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.EfsMax = type;				
			SG_Set_Save();
			MAX_LIST_NUM = g_set_info.EfsMax-g_set_info.nRegionPiont-g_set_info.nLinePiont;
		}

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_CenterIp
  ¹¦  ÄÜ  £ºÖĞĞÄµØÖ·ÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  	IPÊäÈë¸ö¸ÄÎªAAABBBCCCDDD¸ñÊ½£¬ÀîËÉ·å£¬2005-11-17
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
		SG_DynMenu_Put_InputBox (menuid, "ÖĞĞÄµØÖ·:", NULL, DECRYPT_BUF, SG_IP_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char sIp[3+1] = "";
		int nIp;
		char *s = (char*)s1;
		
		menuList->item->count = 0;		
		
		if (strlen(s1) != SG_IP_LEN)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!\r\nAAABBBCCCDDD,¹²12Î»,Ã¿¶Î3Î»,²»×ãµÄÇ°Ãæ²¹0");
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
		
		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		

	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_CenterPort
  ¹¦  ÄÜ  £ºÖĞĞÄ¶Ë¿ÚÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
		SG_DynMenu_Put_InputBox (menuid, "ÖĞĞÄ¶Ë¿Ú:", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		g_set_info.nCenterPort = atoi(s1);
		
		SG_Set_Save();

		SG_Net_Disconnect();
	
		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}

	return 1;
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_ProxyIp
  ¹¦  ÄÜ  £º´úÀíµØÖ·ÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  	IPÊäÈë¸ö¸ÄÎªAAABBBCCCDDD¸ñÊ½£¬ÀîËÉ·å£¬2005-11-17  
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
		SG_DynMenu_Put_InputBox (menuid, "´úÀíµØÖ·:", NULL, DECRYPT_BUF, SG_IP_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		char sIp[3+1] = "";
		int nIp;
		char *s = (char*)s1;
		
		menuList->item->count = 0;		
		
		if (strlen(s1) != SG_IP_LEN)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!\r\nAAABBBCCCDDD,¹²12Î»,Ã¿¶Î3Î»,²»×ãµÄÇ°Ãæ²¹0");
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
	
		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_ProxyPort
  ¹¦  ÄÜ  £º´úÀí¶Ë¿ÚÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
		SG_DynMenu_Put_InputBox (menuid, "´úÀí¶Ë¿Ú:", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		g_set_info.nProxyPort = atoi(s1);
		
		SG_Set_Save();
		
		SG_Net_Disconnect();

		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_ProxyEnable
  ¹¦  ÄÜ  £º´úÀíÊ¹ÄÜÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
			SG_DynMenu_Put_ChooseBox (menuid, "Ê¹ÓÃ´úÀíÁ¬½Ó?");					
		else			
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õ´úÀíÁ¬½Ó?");
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

			SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

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
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	return 1;
}


#if (__cwt_)
/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Edit_SMS
  ¹¦  ÄÜ  £º×Ô±à¼­¶ÌĞÅ»Ø¸´
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º0: ¿ÉÒÔÔÙ½øÀ´1:·µ»Ø
  ±àĞ´Õß  £ºchenhaihua
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2007-7-27
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
		SG_DynMenu_Put_InputBox (menuid, "±à¼­¶ÌĞÅ:", NULL, DECRYPT_BUF, 256, ANYCHAR, ANY_UPLOW, NOCARD);
	}

	//·¢ËÍ¶ÌĞÅĞÅÏ¢
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		//gbk to unicode
		unicodelen= GBK2Unicode(unicode,s1);

		//¸ßµÍ×Ö½Ú×ª»»
		UnicodeBE2LE(unicode,unicodelen);

		//unicode to base64
		basecodelen= base64_encode(basecode,unicodelen,unicode);
		
		MsgEditUp(basecode,basecodelen);
		
		SG_DynMenu_Put_MsgBox (menuid, "·¢ËÍ³É¹¦!");
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "·¢ËÍÊ§°Ü!");
	}		
	return 1;

}

#else


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Edit_SMS
  ¹¦  ÄÜ  £ºÉÏ´«ÎÄ×ÖÏûÏ¢
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º0: ¿ÉÒÔÔÙ½øÀ´1:·µ»Ø
  ±àĞ´Õß  £ºchenhaihua
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2007-7-27
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
		SG_DynMenu_Put_InputBox (menuid, "±à¼­¶ÌĞÅ:", NULL, DECRYPT_BUF, 256, ANYCHAR, ANY_UPLOW, NOCARD);
	}

	//·¢ËÍ¶ÌĞÅĞÅÏ¢
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		//gbk to unicode
		unicodelen= GBK2Unicode(unicode,s1);

		//¸ßµÍ×Ö½Ú×ª»»
	//	UnicodeBE2LE(unicode,unicodelen);

		if(MsgEditSmsUpload(unicode,unicodelen,&msg,&msgLen) != MSG_FALSE){
		
			SG_Send_Put_New_Item(1, msg, msgLen);

			SG_DynMenu_Put_MsgBox (menuid, "ÒÑ·¢ËÍ!");
		}
		else{
			menuList->item->count = 0;
			
			SG_DynMenu_Put_MsgBox (menuid, "·¢ËÍÊ§°Ü!");
		}
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "·¢ËÍÊ§°Ü!");
	}		
	return 1;

}

#endif
/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_APN
  ¹¦  ÄÜ  £ºAPNÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
	
		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Exit
  ¹¦  ÄÜ  £ºÍË³öÅäÖÃ²Ëµ¥²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
  º¯ÊıÃû£ºSG_Proc_Set_User
  ¹¦  ÄÜ  £ºgprsÓÃ»§ÃûÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
		SG_DynMenu_Put_InputBox (menuid, "ÓÃ»§Ãû:", NULL, DECRYPT_BUF, SG_GPRS_USER_LEN, PURE_ENG, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sGprsUser, s1);
		
		SG_Set_Save();
		
		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	return 1;
}

/****************************************************************
º¯ÊıÃû£ºSG_Proc_SetCom_Lock
¹¦ÄÜ£º´®¿ÚÀ©Õ¹ÖĞ¿ØËøÉè±¸²Ëµ¥»Øµ÷º¯Êı
ËµÃ÷£º
ÊäÈë£º
·µ»Ø£º
ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬³Âº£»ª,2006-7-10
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_LOCK)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕÖĞ¿ØËø?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ªÖĞ¿ØËø?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_LOCK;
				SG_DynMenu_Put_MsgBox (menuid, "ÖĞ¿ØËø´ò¿ª!");
				g_state_info.bRelease1 = DEV_ENABLE;
			}
			else
			{
				g_state_info.bNewUart4Alarm&=~UT_LOCK;
				SG_DynMenu_Put_MsgBox (menuid, "ÖĞ¿ØËø¹Ø±Õ!");
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
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}

/****************************************************************
º¯ÊıÃû£ºSG_Proc_SetCom_TTS
¹¦ÄÜ£ºTTS Éè±¸²Ëµ¥»Øµ÷º¯Êı
ËµÃ÷£º
ÊäÈë£º
·µ»Ø£º
ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬³ÂÀû,2006-8-7
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_TTS)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕTTS ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ªTTS ?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}

				
				g_state_info.bNewUart4Alarm |= UT_TTS;
				SG_DynMenu_Put_MsgBox (menuid, "TTS ´ò¿ª!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
				
			}
			else
			{
				g_state_info.bNewUart4Alarm &=~UT_TTS;
				SG_DynMenu_Put_MsgBox (menuid, "TTS ¹Ø±Õ!");
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
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}

/****************************************************************
º¯ÊıÃû£ºSG_Proc_SetCom_Debug
¹¦ÄÜ£º´ò¿ªDEBUG²Ëµ¥»Øµ÷º¯Êı
ËµÃ÷£º
ÊäÈë£º
·µ»Ø£º
ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬³ÂÀû,2006-8-7
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bRelease1 == DEBUG_ENABLE)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕDEBUG ?");	
		else 
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ªDEBUG ?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			if(g_state_info.bRelease1 != DEBUG_ENABLE)
			{
				g_state_info.bRelease1 = DEBUG_ENABLE;
				SG_DynMenu_Put_MsgBox (menuid, "DEBUG ´ò¿ª!");
			}
			else
			{
				#ifdef _DUART
				if(g_set_info.bNewUart4Alarm & 0x7fff)
				{
					g_state_info.bRelease1 = DEV_ENABLE;					
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG¹Ø±Õ²¢ÆôÓÃÍâÉè!");
				}
				else
				{
					g_state_info.bRelease1 = 0;
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG ¹Ø±Õ!");
				}
				#else
				if(g_state_info.bNewUart4Alarm)
				{
					g_state_info.bRelease1 = DEV_ENABLE;
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG¹Ø±Õ²¢ÆôÓÃÍâÉè!");
				}
				else
				{
					g_state_info.bRelease1 = HH_ENABLE;
					SG_DynMenu_Put_MsgBox (menuid, "DEBUG¹Ø±Õ²¢ÆôÓÃÊÖ±ú!");
				}
				#endif
			}
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}




/****************************************************************
º¯ÊıÃû£ºSG_Proc_SetCom_Start
¹¦ÄÜ£ºÆôÓÃµ±Ç°ÉèÖÃ»Øµ÷º¯Êı
ËµÃ÷£º
ÊäÈë£º
·µ»Ø£º
ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬³ÂÀû,2006-8-7
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
		
	//Ê×´Îµ÷ÓÃ

	 if (menuList->item->count == 0)
	{

		menuList->item->count = 1;
		
		SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 1;
	
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		menuList->item->count = 0;
		
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) == 0)
		{
			char title[200] = "µ±Ç°ÉèÖÃÒÑÆôÓÃ!\r\n";

			SG_Set_Cal_CheckSum();
			g_set_info.bRelease1 = g_state_info.bRelease1;
			g_set_info.bRelease2 = g_state_info.bRelease2;
			g_set_info.bNewUart4Alarm = g_state_info.bNewUart4Alarm;
			if(g_set_info.bNewUart4Alarm &UT_LOCK) // ÖĞ¿ØËø¿ªÆôÔòÏÂ´ÎÖØÆô½øĞĞÖĞ¿ØËø²âÊÔ 
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
					strcat(title,"´®¿Ú2µÄLED.");
				}
				if(g_set_info.bNewUart4Alarm & UT_TTS2)
				{
					strcat(title,"´®¿Ú2µÄTTS.");
				}
					
			}
			
			if(g_set_info.bRelease1 == DEBUG_ENABLE)
			{
				strcat(title,"¿ªÆôDEBUGÄ£Ê½.");
			}
			else
			{
				if(g_set_info.bNewUart4Alarm & UT_LOCK)
				{
					strcat(title,"ÖĞ¿ØËø.");
				}
				if(g_set_info.bNewUart4Alarm & UT_TTS)
				{
					strcat(title,"TTS.");
				}
				if(g_set_info.bNewUart4Alarm & UT_LED)
				{
					strcat(title,"ÓĞÔ´´®¿ÚµÄLED.");
				}
				if(g_set_info.bNewUart4Alarm & UT_HBLED)
				{
					strcat(title,"°°É½º£²©LED.");
				}
				if(g_set_info.bNewUart4Alarm & UT_XGCAMERA)
				{
					strcat(title,"ĞÇ¹âÉãÏñÍ·.");
				}
				if(g_set_info.bNewUart4Alarm & UT_QQCAMERA)
				{
					strcat(title,"ÇàÇà×ÓÄ¾ÉãÏñÍ·.");
				}
				if(g_set_info.bNewUart4Alarm & UT_PROLOCK)
				{					
					strcat(title,"ÖØ¹¤Ëø³µ.");			
				}
				if(g_set_info.bNewUart4Alarm & UT_WATCH)
				{
					strcat(title,"Í¸´«Éè±¸.");
				}
				if(g_set_info.bNewUart4Alarm &UT_DRIVE)
				{
					strcat(title,"¼İÅà¹ÜÀíÆ÷.");
				}
				if(g_set_info.bNewUart4Alarm & UT_BUS)
				{
					strcat(title,"¹«½»±¨Õ¾Æ÷.");
				}
				if(g_set_info.bNewUart4Alarm & UT_GPS)
				{
					strcat(title,"×Ôµ¼º½.");
					if(g_set_info.GpsDevType == 1)
					{
						strcat(title,"ZBµ¼º½ÆÁ.");
					}
					else if(g_set_info.GpsDevType == 2)
					{
						strcat(title,"YHµ¼º½ÆÁ.");
					}
						
				}	
				if(g_set_info.bNewUart4Alarm  == 0)
				{
					
					strcat(title,"ÎŞÁ¬½ÓÍâÉè");
				}
			
			}
			SG_DynMenu_Put_MsgBox (menuid, title);

			SG_Soft_Reset(5);
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Lock_Out_Level
  ¹¦  ÄÜ  £ºÉèÖÃÖĞ¿ØËø³£Ì¬µçÆ½
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
			SG_DynMenu_Put_InputBox (menuid, "ÖĞ¿ØËøÉèÖÃµçÆ½:", NULL, DECRYPT_BUF, 5, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		strcpy(level,s1);
		
		if(strlen(level) < 5)
		{
			menuList->item->count = 0;
				
			SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë³¤¶È²»¹»!");
				
			return 1;
		}
		for(i = 0;i < 5;i ++) // ÉèÖÃÖµ²»Îª0ºÍ1Ê±ÌáÊ¾´íÎó
		{
			if(level[i] != '0' && level[i] != '1')
			{
				menuList->item->count = 0;
				
				SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
				
				return 1;
			}
		}		
		SG_Set_Cal_CheckSum();
		g_set_info.alarmSet = 0;
		
		for(i = 0;i < 5;i ++)
		{
			if(level[i] == '1') // Éè¶¨µÄµçÆ½ÖµÎª1
			{
				temp |= 0x01<<(5-i);
			}
		}
		g_set_info.alarmSet |= temp;
		g_set_info.alarmSet = temp<< 24;
		SG_Set_Save();

		SG_Net_Disconnect();
		
		strcpy(title,"ÉèÖÃ³É¹¦!");
		SG_DynMenu_Put_MsgBox (menuid, title);

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}

	return 1;
}


/****************************************************************
º¯ÊıÃû£ºSG_Proc_SetCom_Deliver
¹¦ÄÜ£ºLED Éè±¸²Ëµ¥»Øµ÷º¯Êı
ËµÃ÷£º
ÊäÈë£º
·µ»Ø£º
ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬³ÂÀû,2006-8-7
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_LED)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕÓĞÔ´´®¿ÚµÄLEDÏÔÊ¾ÆÁ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ªÓĞÔ´´®¿ÚµÄLEDÏÔÊ¾ÆÁ?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_LED;
				SG_DynMenu_Put_MsgBox (menuid, "ÓĞÔ´´®¿ÚµÄÏÔÊ¾ÆÁÒÑÆôÓÃ!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_LED;
				SG_DynMenu_Put_MsgBox (menuid, "ÓĞÔ´´®¿ÚµÄÏÔÊ¾ÆÁÒÑ¹Ø±Õ!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}



/****************************************************************
º¯ÊıÃû£ºSG_Proc_SetCom_HBLED
¹¦ÄÜ£ºLED Éè±¸²Ëµ¥»Øµ÷º¯Êı
ËµÃ÷£º
ÊäÈë£º
·µ»Ø£º
ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬³ÂÀû,2006-8-7
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_HBLED)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õ°°É½º£²©LEDÏÔÊ¾ÆÁ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ª°°É½º£²©LEDÏÔÊ¾ÆÁ?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_HBLED;
				SG_DynMenu_Put_MsgBox (menuid, "°°É½º£²©µÄÏÔÊ¾ÆÁÒÑÆôÓÃ!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_HBLED;
				SG_DynMenu_Put_MsgBox (menuid, "°°É½º£²©µÄÏÔÊ¾ÆÁÒÑ¹Ø±Õ!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}





#ifdef _DUART
/****************************************************************
º¯ÊıÃû£ºSG_Proc_SetCom_Deliver
¹¦ÄÜ£ºLED Éè±¸²Ëµ¥»Øµ÷º¯Êı
ËµÃ÷£º
ÊäÈë£º
·µ»Ø£º
ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬³ÂÀû,2006-8-7
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_LED2)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õ´®¿Ú2µÄLEDÏÔÊ¾ÆÁ?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ª´®¿Ú2µÄLEDÏÔÊ¾ÆÁ?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;
				}
				
				g_state_info.bNewUart4Alarm|=UT_LED2;
				SG_DynMenu_Put_MsgBox (menuid, "´®¿Ú2µÄÏÔÊ¾ÆÁÒÑÆôÓÃ!");
				g_state_info.bRelease2 = LED2_ENABLE;
			}
			else
			{
				g_state_info.bNewUart4Alarm &=~UT_LED2;
				SG_DynMenu_Put_MsgBox (menuid, "´®¿Ú2µÄÏÔÊ¾ÆÁÒÑ¹Ø±Õ!");
				g_state_info.bRelease2 = HH_ENABLE;
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}


/****************************************************************
º¯ÊıÃû£ºSG_Proc_SetCom_Deliver
¹¦ÄÜ£ºTTS2 Éè±¸²Ëµ¥»Øµ÷º¯Êı
ËµÃ÷£º
ÊäÈë£º
·µ»Ø£º
ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬³ÂÀû,2006-8-7
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_TTS2)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±Õ´®¿Ú2µÄTTS?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ª´®¿Ú2µÄTTS");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;
				}


				g_state_info.bNewUart4Alarm|=UT_TTS2;
				SG_DynMenu_Put_MsgBox (menuid, "´®¿Ú2µÄTTSÆôÓÃ!");
				g_state_info.bRelease2 = LED2_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_TTS2;
				SG_DynMenu_Put_MsgBox (menuid, "´®¿Ú2µÄTTSÒÑ¹Ø±Õ!");
				g_state_info.bRelease2 = HH_ENABLE;
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}
#endif




/****************************************************************
º¯ÊıÃû£ºSG_Proc_SetCom_Deliver
¹¦ÄÜ£ºÍ¸´«Éè±¸¿ª¹Ø²Ëµ¥»Øµ÷º¯Êı
ËµÃ÷£º
ÊäÈë£º
·µ»Ø£º
ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬³ÂÀû,2006-8-7
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;
		if(g_state_info.bNewUart4Alarm&UT_WATCH)
			SG_DynMenu_Put_ChooseBox (menuid, "¹Ø±ÕÍ¸´«Éè±¸?");
		else
			SG_DynMenu_Put_ChooseBox (menuid, "´ò¿ªÍ¸´«Éè±¸?");			

		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
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
		
					SG_DynMenu_Put_MsgBox (menuid, "ÒÑ¿ªÆôÆäËûÉè±¸£¬ÇëÏÈ¹Ø±ÕÆäËûÉè±¸!");

					return 1;

				}
				g_state_info.bNewUart4Alarm|=UT_WATCH;
				SG_DynMenu_Put_MsgBox (menuid, "Í¸´«Éè±¸ÒÑÆôÓÃ!");
				
				g_state_info.bRelease1 = DEV_ENABLE;
			}

			else
			{
				g_state_info.bNewUart4Alarm &=~UT_WATCH;
				SG_DynMenu_Put_MsgBox (menuid, "Í¸´«Éè±¸ÒÑ¹Ø±Õ!");
				#ifdef _DUART
				g_state_info.bRelease1 = 0;
				#else
				g_state_info.bRelease1 = HH_ENABLE;
				#endif
			}
			
		}
		else
		{
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}
}




/********************************************************************************
ÉèÖÃÍ¸´«Éè±¸ÀàĞÍ
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
		SG_DynMenu_Put_InputBox (menuid, "1 ¼Æ¼ÛÆ÷ 2 ³ÆÖØ´«¸ĞÆ÷", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		char value=-1;
		 
		menuList->item->count = 0;	
		
		value=atoi(s1);
		if(value != 1 && value != 2)
		{
			menuList->item->count = 0;
		
			SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ´íÎó!");

		}
		SG_Set_Cal_CheckSum();
		g_set_info.watchid=value;
		SG_Set_Save();
		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");
		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	return 1;

}



/***********************************************************************
ÉèÖÃÍ¸´«Éè±¸³¬Ê±Ê±¼ä
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		SG_DynMenu_Put_ChooseBox (menuid, "ÊÇ·ñÉèÖÃ³¬Ê±Ê±¼ä?");
		
		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "ÉèÖÃ²ÎÊı(1~60):", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else

		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		var=atoi(s1);
		if((var>=1)&&(var<=60))
		{	
			SG_Set_Cal_CheckSum();
			g_set_info.nSetTime= var;
			SG_Set_Save();
			strcpy(title,"ËùÉèÖÃ²ÎÊı:");
			sprintf(title+strlen(title),"%d",var);
			SG_DynMenu_Put_MsgBox(menuid,title);
		}
		else
			SG_DynMenu_Put_MsgBox (menuid, "²ÎÊıÉèÖÃ´íÎó!");
			
		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	return 1;
}


/************************************************************************
ÉèÖÃÍ¸´«ID
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
		SG_DynMenu_Put_InputBox (menuid, "Ä¬ÈÏID(0~255):", NULL, DECRYPT_BUF, SG_PORT_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
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
		sprintf(strmsg,"ÉèÖÃ³É¹¦,Éè±¸\r\nÄ¬ÈÏID:0x%x",g_set_info.devID);
		SG_DynMenu_Put_MsgBox (menuid, strmsg);
		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	return 1;

}

/************************************************************************
ÉèÖÃÍ¸´«²¨ÌØÂÊ
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
		
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		menuList->item->count = 1;

		SG_DynMenu_Put_ChooseBox (menuid, "ÊÇ·ñÉèÖÃ´®¿Ú²¨ÌØÂÊ?");
		
		return 1;
	}
	//²Ù×÷È·ÈÏ
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			menuList->item->count = 2;
		
			SG_DynMenu_Put_InputBox (menuid, "ÉèÖÃ²ÎÊı:", NULL, DECRYPT_BUF, SG_PHONE_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

			return 1;
		}
		else
		{
			menuList->item->count = 0;

			return 0;
		}
	}
	//»ñÈ¡ÃÜÂë
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;
		var=atoi(s1);

		SG_Set_Cal_CheckSum();
		g_set_info.watchbd = var;
		SG_Set_Save();
		strcpy(title,"ËùÉèÖÃ²ÎÊı:");
		sprintf(title+strlen(title),"%d",var);
		SG_DynMenu_Put_MsgBox(menuid,title);


		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}
	return 1;
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Set_Psw
  ¹¦  ÄÜ  £ºgprsÃÜÂëÉèÖÃ²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
		SG_DynMenu_Put_InputBox (menuid, "ÃÜÂë:", NULL, ENCRYPT_BUF, SG_GPRS_PSW_LEN, PURE_ENG, ANY_UPLOW, NOCARD);
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		
		SG_Set_Cal_CheckSum();
		strcpy(g_set_info.sGprsPsw, s1);
		
		SG_Set_Save();
		
		SG_DynMenu_Put_MsgBox (menuid, "ÉèÖÃ³É¹¦!");

		return 1;
	}	
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}

	return 1;
}


//ÏÔÊ¾ log ĞÅÏ¢
static int SG_Proc_SysLog(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[600] = "";
	
	if (s1 != NULL || s2 != NULL)
		return 0;

	

	sprintf(title+strlen(title), "ÏµÍ³µ÷ÊÔ1:R %d,GGA %d, RMC %d, A %d, V %d, H %d, P %d, ac %d ,de %d\r\n",
			g_set_info.nReConCnt,g_state_info.nGGACnt,g_state_info.nRMCCnt,g_state_info.nACnt,g_state_info.nVCnt,g_state_info.nHandupConCnt,g_state_info.nPorxyConCnt,g_state_info.nActiveCnt,g_state_info.nDeactiveCnt);	
	sprintf(title+strlen(title), "err:gps %d\r\n",g_state_info.nGPSErr);
	sprintf(title+strlen(title), "alarmset %x\r\nalarmstate %lx\r\nalarmenable %lx alarmRepeat %lx\r\n",g_set_info.alarmSet,g_state_info.alarmState,g_set_info.alarmEnable,g_set_info.alarmRepeat);
	sprintf(title+strlen(title), "reboot %d\r\nnormal reset %d\r\n",g_set_info.nResetCnt,g_set_info.nReConCnt);
	sprintf(title+strlen(title), "g_set_info length %d\r\n",sizeof(g_set_info));
	sprintf(title+strlen(title), "GPS¶¨Î»¹ıÂËÊ±¼äÎª:%d\r", g_set_info.nGpstTime);
	sprintf(title+strlen(title), "GPS¹ıÂË¾àÀëÎª:%d\r", g_set_info.nGpstDist);
	sprintf(title+strlen(title), "Ã¤Çø²¹³¥µÄµãÎª:%d\r", MAX_LIST_NUM);
	sprintf(title+strlen(title), "Ëæ»úÊıÎª:%d \r", g_state_info.randtime);
	if(g_set_info.Heart_Switch)
		sprintf(title+strlen(title), "ĞÄÌø¿ª ¼ä¸ô:%d \r", g_set_info.Heart_Interval);
	else
		sprintf(title+strlen(title), "ĞÄÌø¹Ø ¼ä¸ô:%d \r", g_set_info.Heart_Interval);
		

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}



//ÏÔÊ¾ log ĞÅÏ¢
static int SG_Proc_SetCom_Check_It_Self(unsigned char menuid[MENUID_LEN], void *s1, void *s2)
{
	char title[600] = "";
	int count = 0;

	if (s1 != NULL || s2 != NULL)
		return 0;

	sprintf(title+strlen(title), "Ext Device:\r\nbRelease1 %d brelease 2 %d\r\nbNewUart4Alarm %02x\r\n",g_set_info.bRelease1,g_set_info.bRelease2,g_set_info.bNewUart4Alarm);

	if(g_set_info.bNewUart4Alarm & (UT_QQCAMERA|UT_XGCAMERA))
	{
		sprintf(title+strlen(title), "ÉãÏñÍ·:¼à¿Ø±êÖ¾%x; ÊµÊ±0x%x Â·,¼ä¸ô %d ; Ìõ¼ş0x%x Â·, ¼ä¸ô%d\r\n"
			,g_set_info.nCameraAlarm, g_set_info.sg_camera[0].nCameraID,g_set_info.sg_camera[0].nInterval,g_set_info.sg_camera[1].nCameraID,g_set_info.sg_camera[1].nInterval) ;
	
		sprintf(title+strlen(title),"Í¼Æ¬±£´æ±êÖ¾: %x  Í¼Æ¬±£´æË÷Òı %d\r\n",g_set_info.bPictureSave,g_set_info.nPictureSaveIndex);
		sprintf(title+strlen(title),"ÎÄ¼ş0-4±£´æÍ¼Æ¬: %d %d %d %d %d\r\n",g_set_info.camera_save_list[0].pic_save,
			g_set_info.camera_save_list[1].pic_save,g_set_info.camera_save_list[2].pic_save,
			g_set_info.camera_save_list[3].pic_save,g_set_info.camera_save_list[4].pic_save);
		sprintf(title+strlen(title),"ÉãÏñÍ·µ±Ç°×´Ì¬: %d",CurrCamera.currst);
		sprintf(title+strlen(title),"¶ÏÏß´ÎÊı: %d \r\n",g_state_info.nReConnect);
		count = SG_Send_Cal_Pic_Item();

		sprintf(title+strlen(title),"¶ÓÁĞÖĞÍ¼Æ¬°ü¸öÊı: %d",count);
		sprintf(title+strlen(title),"ÒÑ¾­ÅÄÉãÍ¼Æ¬: %d",g_state_info.photo_catch_cnt);
	}

	SG_DynMenu_Put_MsgBox (menuid, title);
	
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Psw_Super
  ¹¦  ÄÜ  £º³¬¼¶ÃÜÂë²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  Ôö¼Ó³¬¼¶¹ÜÀíÔ±ÃÜÂëÒ²ÄÜĞŞ¸Ä¹ÜÀíÔ±ÃÜÂë£¬ÀîËÉ·å£¬2005-12-06
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
	
	//Ê×´Îµ÷ÓÃ
	if (menuList->item->count == 0)
	{
		if (menuList->item->data != NULL)
		{
			SCI_FREE(menuList->item->data);
			menuList->item->data = NULL;
		}

		menuList->item->count = 1;
		
		SG_DynMenu_Put_InputBox (menuid, "Ô­³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 1;
	}
	//»ñÈ¡Ô­ÃÜÂëĞÅÏ¢
	else if ((menuList->item->count == 1) && (s1 != NULL))
	{
		if (strcmp((const char*)s1, (const char*)g_set_info.sSuperPsw) != 0)
		{
			menuList->item->count = 0;
		
			SG_DynMenu_Put_MsgBox (menuid, "³¬¼¶ÃÜÂë´íÎó!");
		}
		else
		{
			menuList->item->count = 2;

			SG_DynMenu_Put_InputBox (menuid, "ĞÂ³¬¼¶ÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);
		}

		return 1;
	}
	//»ñÈ¡ĞÂÃÜÂëĞÅÏ¢
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 3;
		menuList->item->data = SCI_ALLOC(12);
		SCI_MEMSET(menuList->item->data,0,12);
		strcpy(menuList->item->data,s1);
		SG_DynMenu_Put_InputBox (menuid, "ÔÙ´ÎÊäÈëÃÜÂë:", NULL, ENCRYPT_BUF, SG_PSW_LEN, PURE_NUM, ANY_UPLOW, NOCARD);

		return 1;
	}
	//ÔÙ´Î»ñÈ¡ĞÂÃÜÂëĞÅÏ¢
	else if ((menuList->item->count == 3) && (s1 != NULL) && (menuList->item->data != NULL))
	{
		menuList->item->count = 0;
	
		if (strcmp(menuList->item->data, s1) != 0)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÃÜÂë²»Æ¥Åä!");
		}
		else
		{
			SG_Set_Cal_CheckSum();
			//±£´æÃÜÂë
			strcpy((char *)g_set_info.sSuperPsw,(const char*)s1);
			
			SG_Set_Save();
			
			SG_DynMenu_Put_MsgBox (menuid, "ÃÜÂëĞŞ¸Ä³É¹¦!");
		}
		
		SCI_FREE(menuList->item->data);
		menuList->item->data = NULL;

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");

		return 1;
	}		
}



/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Reset_Default
  ¹¦  ÄÜ  £º»Ö¸´³ö³§²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
			SG_DynMenu_Put_ChooseBox (menuid, "ÖØÆôÏµÍ³?");
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			SG_DynMenu_Put_MsgBox (menuid, "ÖØÆôÖĞ...");
			SG_Soft_Reset(2);
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		
	return 1;
}


/****************************************************************
  º¯ÊıÃû£ºSG_Proc_Reset_Default
  ¹¦  ÄÜ  £º»Ö¸´³ö³§²Ëµ¥»Øµ÷º¯Êı
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
			SG_DynMenu_Put_ChooseBox (menuid, "»Ö¸´³ö³§ÉèÖÃ?");
	}
	else if ((menuList->item->count == 2) && (s1 != NULL))
	{
		menuList->item->count = 0;		

		if (*(char*)s1 == INPUT_CHOOSE_YES)
		{
			SG_DynMenu_Put_MsgBox (menuid, "³ö³§ÉèÖÃ³É¹¦!");
			SG_Set_Reset();
		}

		return 1;
	}
	else
	{
		menuList->item->count = 0;
		
		SG_DynMenu_Put_MsgBox (menuid, "ÊäÈë´íÎó!");
	}		
	return 1;
}

/****************************************************************
  º¯ÊıÃû£ºSG_Net_DynamicMenu_Snd_Msg
  ¹¦  ÄÜ  £º¶¯Ì¬²Ëµ¥·¢ËÍÏûÏ¢¶ÓÁĞ
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
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
  º¯ÊıÃû£ºSG_Init_MenuList
  ¹¦  ÄÜ  £º´´½¨²Ëµ¥ÁĞ±í
  ÊäÈë²ÎÊı£º
  Êä³ö²ÎÊı£º
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-9-20
  	Ìí¼ÓÀï³ÌÍ³¼Æ²Ëµ¥£¬³Âº£»ª£¬2006-5-23
****************************************************************/
int SG_Init_MenuList ()
{
	MENUITEM *pMenuItem = NULL;
	MENULIST *pMenuList = NULL, *father = NULL;
	int nMenuId = FIRST_MENUID;
//	int i=0;
/*
Ö÷»ú¹¦ÄÜ²Ëµ¥

	|--Ä£¿é¼ì²â	|--°æ±¾²éÑ¯
	|			|--³µÁ¾×´Ì¬
	|			|--ÍøÂç¹ÊÕÏ²éÑ¯
	|                                 
	|--°²×°µ÷ÊÔ	|--IO¼°½ÓÏß×´Ì¬¼ì²â(Î´ÉèÖÃ±¾»úºÅÊ±¿ÉÒÔ½øÈë)
	|         	|--Ï¨»ğ
	|			|--Ï¨»ğ»Ö¸´
	|
	|--´®¿ÚÀ©Õ¹	|--Í¸´«Éè±¸		|--¿ª¹ØLEDÏÔÊ¾ÆÁ¹¦ÄÜ
	|
	|        	|--ÖĞ¿ØËø		|--¿ª¹ØÖĞ¿ØËø¹¦ÄÜ
	|                        		|--ÖĞ¿ØËø³£Ì¬µçÆ½ÉèÖÃ
	|          	|--¿ª¹ØTTS¹¦ÄÜ
	|			|--¿ª¹ØDEBUG
	|			|--ÆôÓÃµ±Ç°ÉèÖÃ
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

	// {Ä£¿é¼ì²â
	pMenuList->item = pMenuItem;
	SCI_MEMCPY(pMenuItem->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuItem->s = "°æ±¾²éÑ¯";  //  ²é¿´°æ±¾ºÅ
	pMenuItem->proc = SG_Proc_Ver_Ask;


//°²×°µ÷ÊÔ

//	|--°²×°µ÷ÊÔ		|
//	|				|
//	|				|--IO¼°½ÓÏß×´Ì¬¼ì²â
//	|				|
//	|				|--Ï¨»ğ
//	|				|
//	|				|--Ï¨»ğ»Ö¸´
//	|				|


	

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "°²×°µ÷ÊÔ";
	pMenuList->item->proc = NULL;
	
	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "IO¼°½ÓÏß×´Ì¬¼ì²â";
	pMenuList->item->proc =SG_Proc_IO_Check_State;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "×Ô¶¨Òå1´¥·¢µçÆ½";
	pMenuList->item->proc = SG_Proc_Set_IO_level;

#ifndef _TRACK
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¼Æ¼ÛÊäÈë´¥·¢µçÆ½";
	pMenuList->item->proc = SG_Proc_Set_JJQ_Lev;

	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÉèÖÃ¼Æ¼ÛÊäÈë¸´ÓÃ";
	pMenuList->item->proc = SG_Proc_Set_Battery_Check;
#endif



	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("ÊÖ±úÓëÃâÌáÇĞ»»");
	pMenuList->item->proc = SG_Proc_AutoAnswer;
	
	#ifdef _DUART // ÍâÉè°²×°µ÷ÊÔ
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("LED±¨¾¯²âÊÔ");
	pMenuList->item->proc = SG_Proc_Test_LED;

	pMenuList->next = SCI_ALLOC(  sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC(  sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÖĞ¿ØËø¿ª¹ØËø";
	pMenuList->item->proc = NULL;
//{
	father = pMenuList;

	pMenuList->child = SCI_ALLOC(  sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC(  sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¿ªËø";
	pMenuList->item->proc =SG_Proc_Test_OpenLock;

	pMenuList->next = SCI_ALLOC(  sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC(  sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "±ÕËø";
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
	pMenuList->item->s = ("TTSÉèÖÃµ÷ÊÔ");
	pMenuList->item->proc = SG_Proc_Test_TTS;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("ÓÍÁ¿AD Öµ");
	pMenuList->item->proc = SG_Proc_Check_AD;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Í¼ÏñÅÄÉã");
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
	pMenuList->item->s = "Ï¨»ğ";
	pMenuList->item->proc = SG_Proc_Set_XIH_Off;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "Ï¨»ğ»Ö¸´";
	pMenuList->item->proc = SG_Proc_Set_XIH_On;
	}


    pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("°æ±¾ÇĞ»»");
	pMenuList->item->proc = SG_Proc_Ver;


	pMenuList = father;	
	father = pMenuList->father;




// °²×°µ÷ÊÔend

//Ä£¿é¼ì²â{
#if (__cwt_)
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "×Ô±àÒë¶ÌĞÅ";
	pMenuList->item->proc = SG_Proc_Edit_SMS;
#endif /* (0) */

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "Ä£¿é¼ì²â";
	pMenuList->item->proc = NULL;


	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "³µÁ¾×´Ì¬";
	pMenuList->item->proc =SG_Proc_Check_Device;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÍøÂç¹ÊÕÏ²éÑ¯";
	pMenuList->item->proc = SG_Proc_Net_State;
	pMenuList = father;	
	father = pMenuList->father;
//}Ä£¿é¼ì²â²Ëµ¥½áÊø

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	SCI_MEMCPY(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "·ÀµÁ¹¦ÄÜ";
	pMenuList->item->proc = SG_Proc_Theft;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¿ì½İ²¦ºÅ";
	pMenuList->item->proc = NULL;

//	{ 	¿ì½İ²¦ºÅ

	father = pMenuList;


	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "·şÎñÈÈÏß";
	pMenuList->item->proc = SG_Proc_Hot_Service;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Ò½ÁÆÇóÖú");
	pMenuList->item->proc = SG_Proc_Medical_Service;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Î¬ĞŞÇóÖú");
	pMenuList->item->proc = SG_Proc_Help_Service;
	
	pMenuList = father;


//}¿ì½İ²¦ºÅ½áÊø

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Î»ÖÃ²éÑ¯");
	pMenuList->item->proc = NULL;

//	{	Î»ÖÃ²éÑ¯

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("GPSĞÅÏ¢");
	pMenuList->item->proc = SG_Proc_GPS_Info;


	pMenuList = father;

//	}	Î»ÖÃ²éÑ¯

//   {	Àï³ÌÍ³¼Æ²Ëµ¥£­begin
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Àï³ÌÍ³¼Æ");
	pMenuList->item->proc = NULL;

//   		{	×Ó²Ëµ¥µÚ1²ã£­begin
	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("×ÜÀï³ÌÍ³¼Æ");
	pMenuList->item->proc = SG_Proc_Cacular_Total_Distance;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Àï³Ì1Í³¼Æ");
	pMenuList->item->proc = SG_Proc_Cacular_Distance1;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Àï³Ì2Í³¼Æ");
	pMenuList->item->proc = SG_Proc_Cacular_Distance2;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Àï³Ì1ÇåÁã");
	pMenuList->item->proc = SG_Proc_Clr_Distance1;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Àï³Ì2ÇåÁã");
	pMenuList->item->proc = SG_Proc_Clr_Distance2;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("×ÜÀï³ÌÉèÖÃ");
	pMenuList->item->proc = SG_Proc_Set_Total_Distance;	
//   		}	×Ó²Ëµ¥µÚ1²ã£­end

	pMenuList = father;
//   }	Àï³ÌÍ³¼Æ²Ëµ¥£­end

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Í¨»°ÉèÖÃ");
	pMenuList->item->proc = NULL;
	
	

//		{	Í¨»°ÉèÖÃ

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("ºôÈëÏŞÖÆ");
	pMenuList->item->proc = SG_Proc_Call_In;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);

	pMenuList->item->s = ("ºô³öÏŞÖÆ");
	pMenuList->item->proc = SG_Proc_Call_Out;

	if (g_state_info.user_type == PRIVATE_CAR_VER)
	{
		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = "ÉèÖÃÒ»¼üµ¼º½ºÅÂë";
		pMenuList->item->proc = SG_Proc_Set_OneGPS;

		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = "¶ÌĞÅ±¨¾¯ºÅÂë1";
		pMenuList->item->proc = SG_Proc_Set_AlarmSms;

		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = "¶ÌĞÅ±¨¾¯ºÅÂë2";
		pMenuList->item->proc = SG_Proc_Set_AlarmSms2;

		pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
		pMenuList->next->father = father;
		pMenuList->next->pre = pMenuList;
		pMenuList = pMenuList->next;
		pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
		nMenuId ++;
		memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
		pMenuList->item->s = ("×Ô¶¯½ÓÌı");
		pMenuList->item->proc = SG_Proc_Auto_Call;
	}

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = ("Í¨»°ÒôÁ¿µ÷Õû");
	pMenuList->item->proc = SG_Proc_SpeakVolChg;

	pMenuList = father;	

	father = pMenuList -> father;

//		}	Í¨»°ÉèÖÃ

	
// µ÷¶ÈĞÅÏ¢²éÑ¯
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "µ÷¶ÈĞÅÏ¢²éÑ¯";
	pMenuList->item->proc = SG_Proc_Muster_Particular_Ask;


// ´®¿ÚÀ©Õ¹{
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "´®¿ÚÀ©Õ¹";
	pMenuList->item->proc = NULL;

	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "Í¸´«Éè±¸";
	pMenuList->item->proc = NULL;

	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÉèÖÃ¿ª¹Ø";
	pMenuList->item->proc = SG_Proc_SetCom_Watch;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÉèÖÃÍ¸´«ID";
	pMenuList->item->proc = SG_Proc_SetCom_ID;

#if (1)
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÉèÖÃÍ¸´«²¨ÌØÂÊ";
	pMenuList->item->proc = SG_Proc_SetCom_Baud;
#endif /* (0) */


	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÉèÖÃ³¬Ê±Ê±¼ä";
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
	pMenuList->item->s = "ÓĞÔ´´®¿ÚµÄLED";
	pMenuList->item->proc = SG_Proc_SetCom_LED;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "°°É½º£²©LED";
	pMenuList->item->proc = SG_Proc_SetCom_HBLED;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "´®¿Ú2µÄLED";
	pMenuList->item->proc = SG_Proc_SetCom_LED2;
	#else
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¿ª¹ØLEDÏÔÊ¾ÆÁ";
	pMenuList->item->proc = SG_Proc_SetCom_LED;	
	#endif

	

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÖĞ¿ØËø";
	pMenuList->item->proc = NULL;

	father = pMenuList;

	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¿ª¹ØÖĞ¿ØËø";
	pMenuList->item->proc = SG_Proc_SetCom_Lock;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÖĞ¿ØËø³£Ì¬µçÆ½ÉèÖÃ";
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
	pMenuList->item->s = "¿ª¹ØTTS";
	pMenuList->item->proc = SG_Proc_SetCom_TTS;

#ifdef _DUART
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¿ª¹Ø´®¿Ú2µÄTTS";
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
	pMenuList->item->s = "¿ª¹Ø×Ôµ¼º½";
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
	pMenuList->item->s = "ĞÇ¹âÉãÏñÍ·";
	pMenuList->item->proc = SG_Proc_SetCom_XgVideo;
#endif /* (0) */

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÇàÇà×ÓÄ¾ÉãÏñÍ·";
	pMenuList->item->proc = SG_Proc_SetCom_QqzmVideo;
	

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¼İÅà¹ÜÀíÆ÷";
	pMenuList->item->proc = SG_Proc_SetCom_Drive;
	
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¿ª¹ØDEBUG";
	pMenuList->item->proc = SG_Proc_SetCom_Debug;
	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÆôÓÃµ±Ç°ÉèÖÃ";
	pMenuList->item->proc = SG_Proc_SetCom_Start;	
	
	#ifdef _DUART
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "´®¿ÚÉè±¸²é¿´";
	pMenuList->item->proc = SG_Proc_SetCom_Check_It_Self;	
	#endif
	
	pMenuList = father;	
	father = pMenuList->father;
//´®¿ÚÀ©Õ¹end
	
	

/*
ÖÕ¶ËÉèÖÃÒş²Ø²Ëµ¥

	|--ÅäÖÃ²éÑ¯
	|--±¾»úºÅÂë
	|--ÖĞĞÄºÅÂë
	|--Í¨Ñ¶·½Ê½
	|--ÖĞĞÄµØÖ·
	|--ÖĞĞÄ¶Ë¿Ú
	|--GPRSÉèÖÃ  	|--´úÀíµØÖ·
				|--´úÀí¶Ë¿Ú
				|--´úÀíÊ¹ÓÃ
				|--APN
				|--ÓÃ»§Ãû
				|--ÃÜÂë
	|--ÏµÍ³LOG
	|--»Ö¸´³ö³§
	|--ÖØÆôÏµÍ³
	|--ÍË³öÅäÖÃ
	
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
	pMenuList->item->s = "ÅäÖÃ²éÑ¯";
	pMenuList->item->proc = SG_Proc_Show_Device;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "±¾»úºÅÂë";
	pMenuList->item->proc = SG_Proc_Set_OwnNo;
	#if (__cwt_)
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÊÚÈ¨Âë";
	pMenuList->item->proc = SG_Proc_Set_OemCode;
	#endif
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÖĞĞÄºÅÂë";
	pMenuList->item->proc = SG_Proc_Set_CenterNo;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "Í¨Ñ¶·½Ê½";
	pMenuList->item->proc = SG_Proc_Set_Communication_Type;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÖĞĞÄµØÖ·";
	pMenuList->item->proc = SG_Proc_Set_CenterIp;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÖĞĞÄ¶Ë¿Ú";
	pMenuList->item->proc = SG_Proc_Set_CenterPort;

//GPRS ÉèÖÃ
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "GPRSÉèÖÃ";
	pMenuList->item->proc = NULL;

	father = pMenuList;
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "´úÀíÊ¹ÓÃ";
	pMenuList->item->proc = SG_Proc_Set_ProxyEnable;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList->next->father = father;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "´úÀíµØÖ·";
	pMenuList->item->proc = SG_Proc_Set_ProxyIp;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "´úÀí¶Ë¿Ú";
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
	pMenuList->item->s = "ÓÃ»§Ãû";
	pMenuList->item->proc = SG_Proc_Set_User;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList->next->father = father;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÃÜÂë";
	pMenuList->item->proc = SG_Proc_Set_Psw;

	pMenuList = father;
	father = pMenuList->father;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¼à¿ØÉèÖÃ";
	pMenuList->item->proc = NULL;

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¼à¿Ø·½Ê½";
	pMenuList->item->proc = SG_Proc_Set_Watch_Type;

	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "¼à¿Ø¼ä¸ôÊ±¼ä";
	pMenuList->item->proc = SG_Proc_Set_Watch_Interval;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "GPS¹ıÂËÊ±¼ä";
	pMenuList->item->proc = SG_Proc_Set_GPS_Interval;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "Àï³ÌÍ³¼Æ×îĞ¡¾àÀë";
	pMenuList->item->proc = SG_Proc_Set_GPS_Dist;

    pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "Ã¤Çø²¹³¥×î´óÌõÊı";
	pMenuList->item->proc = SG_Proc_Set_Efs_Max;

	pMenuList = father;
	father = pMenuList->father;


	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ĞÄÌøÉèÖÃ";
	pMenuList->item->proc = NULL;

	father = pMenuList;
	
	pMenuList->child = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->child->father = father;
	pMenuList = pMenuList->child;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ĞÄÌø¿ª¹Ø";
	pMenuList->item->proc = SG_Proc_Set_Heart_Switch;

	
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ĞÄÌø¼ä¸ôÊ±¼ä";
	pMenuList->item->proc = SG_Proc_Set_Heart_Interval;

	pMenuList = father;
	father = pMenuList->father;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÏµÍ³LOG";
	pMenuList->item->proc = SG_Proc_SysLog;

#if (0) // ÔİÊ±ÆÁ±Î³¬¼¶ÃÜÂëĞŞ¸Ä
// ³¬¼¶ÃÜÂëĞŞ¸Ä
	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->father = father;
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "³¬¼¶ÃÜÂëĞŞ¸Ä";
	pMenuList->item->proc = SG_Proc_Psw_Super;

#endif /* (0) */

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "»Ö¸´³ö³§";
	pMenuList->item->proc = SG_Proc_Reset_Default;

	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÖØÆôÏµÍ³";
	pMenuList->item->proc = SG_Proc_Reset;


	pMenuList->next = SCI_ALLOC( sizeof(MENULIST));
	pMenuList->next->pre = pMenuList;
	pMenuList = pMenuList->next;
	pMenuList->item = SCI_ALLOC( sizeof(MENUITEM));
	nMenuId ++;
	memcpy(pMenuList->item->menuid, (char*)&nMenuId, MENUID_LEN);
	pMenuList->item->s = "ÍË³öÅäÖÃ";
	pMenuList->item->proc = SG_Proc_Set_Exit;

//   }	ÅäÖÃ²Ëµ¥£­end

	SCI_TRACE_LOW("SG_Init_MenuList \r\n");
	return 1;
}
/****************************************************************
  º¯ÊıÃû£ºSG_Menu_Set_MenuList
  ¹¦  ÄÜ  £ºÉèÖÃµ±Ç°²Ëµ¥
  ÊäÈë²ÎÊı£ºtype: 0:Ö÷²Ëµ¥ 1:ÉèÖÃ²Ëµ¥
  Êä³ö²ÎÊı£ºÎŞ
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-11-27
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
  º¯ÊıÃû£ºSG_Menu_Get_MenuList
  ¹¦  ÄÜ  £º»ñÈ¡µ±Ç°²Ëµ¥
  ÊäÈë²ÎÊı£ºÎŞ
  Êä³ö²ÎÊı£ºÎŞ
  ±àĞ´Õß  £ºÀîËÉ·å
  ĞŞ¸Ä¼ÇÂ¼£º´´½¨£¬2005-11-27
****************************************************************/
void* SG_Menu_Get_MenuList()
{
	return (void*)gpCurMenuList;
}
