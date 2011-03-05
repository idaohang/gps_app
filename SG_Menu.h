/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_Menu.h
  版本   ：1.00
  创建者 ：李松峰
  创建时间：2005-9-23
  内容描述：SG2000 手柄菜单功能处理 
  修改记录：
***************************************************************/
#ifndef _SG_MENU_H_
#define _SG_MENU_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
*******************************函数声明****************************
********************************************************************/

int SG_Proc_ShortCut(unsigned char key);
int SG_Init_MenuList (void);
void* SG_Menu_Get_MenuList(void);
void SG_Menu_Set_MenuList(int type);
void SG_Menu_Clear_SuperLogin(void);
void* SG_Menu_Main(void* data);
int  SG_Net_DynamicMenu_Snd_Msg (unsigned char *msg, int len);
#ifdef __cplusplus
}
#endif

#endif //_SG_MENU_H_


