/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Menu.h
  �汾   ��1.00
  ������ �����ɷ�
  ����ʱ�䣺2005-9-23
  ����������SG2000 �ֱ��˵����ܴ��� 
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_MENU_H_
#define _SG_MENU_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
*******************************��������****************************
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


