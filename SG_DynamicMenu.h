/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_DynamicMenu.h
  �汾   ��1.00
  ������ �����ɷ�
  ����ʱ�䣺2005-9-15
  ����������SG2000�������ֱ�֮�䶯̬�˵�Э��        
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_DYNAMICMENU_H_
#define _SG_DYNAMICMENU_H_

#ifdef __cplusplus
extern "C" {
#endif

//���� �ֶ����ն�
#define TYPE_MENU_LIST         0x01 //�˵��б�
#define TYPE_MENU_ITEM        0x02 //�˵���
#define TYPE_MSG_BOX            0x03 //��Ϣ��
#define TYPE_VOICE_BOX         0x04 //���⹦�ܴ�����������еȣ�
#define TYPE_AUTOSHOW_BOX 0x05 //�Զ�������ʾ��Ϣ��
#define TYPE_AUTOSHOW_BOX_T1 0x06 //�Զ�������ʾ��Ϣ���ڹ涨�����󷵻�������

#define SIGN_INPUT_BOX   0x01 //������־
#define DECRYPT_BUF         0x00 //����
#define ENCRYPT_BUF         0x01 //����
#define ANYCHAR                0x00 //�����ַ�
#define PURE_NUM              0x01 //������
#define PURE_ENG              0x02 //��Ӣ��
#define PURE_CHN              0x03 //������
#define ANY_UPLOW           0x00 //��Сд������
#define PURE_UP                0x01 //�޴�д
#define PURE_LOW             0x02 //��Сд
#define NOCARD                 0x00 //����ˢ��
#define CARD                      0x01 //����ˢ��

#define SIGN_CHOOSE_BOX       0x02 //ѡ����־


//����  �ն����ֶ�
#define TYPE_GET_MENU_LIST  0x01 //����˵����б�/�
#define TYPE_MENU_PARA         0x02 //�˵�������������
#define TYPE_SHORTCUT_KEY    0x03 //��ݼ�����
#define TYPE_MUSTER_ACK	  0x04 //����Ӧ��

#define SIGN_GET_CHILD_MENU     0x01 //�����Ӳ˵����б�/�
#define SIGN_GET_FATHER_MENU  0x02 //���󸸲˵����б�
#define SIGN_GET_MAIN_MENU      0x03 //�������˵�

#define SIGN_INPUT_BOX_PARA     0x01 //������������
#define SIGN_CHOOSE_BOX_PARA  0x02 //ѡ����������
#define SIGN_MSG_BOX_PARA  	0x03 //ѡ����������

#define INPUT_CHOOSE_YES	0x00 //�����/ѡ���ȷ������
#define INPUT_CHOOSE_NO  	0x01 //�����/ѡ���ȡ������

//��ݼ�ID
#define SHORTCUT_MEDICAL  	0x01 //ҽ��������ݼ�
#define SHORTCUT_SERVICE  	0x02 //ά��������ݼ�
#define SHORTCUT_INFO	  	0x00 //��Ϣ������ݼ�


//����
#define SEPARATOR       '\0' //�ָ���
#define END                   '\0' //������
#define MENUID_LEN            (sizeof(int)) //�˵�ID����4�ֽ�
#define INPUT_BOX_TITLE_LEN   15 //�������ʾ˵��ͷ��󳤶�
#define INPUT_BOX_TAIL_LEN     4 //�������ʾ˵��β��󳤶�
#define FIRST_MENUID 0x00000000 //���˵���ID���ֱ����ݴ��ж��Ƿ����˵�
#define MENUID_SINGLE 0xff123456 //������̬�˵���ID

//�˵���ṹ
typedef struct _MenuItem_
{
	unsigned char menuid[MENUID_LEN]; //�˵�ID
	char *s; //�˵�˵��
	int (*proc)(unsigned char menuid[MENUID_LEN], void *s1, void *s2); //�ص�����:����ֵ0��ʾ��Ҫ���Ͳ˵�
	int count; //���ò˵��ļ���
	void *data; //��������ָ��(�ص�������������)
} MENUITEM;

//�˵�����ṹ
typedef struct _MenuList_
{
	MENUITEM *item; //�˵���
	struct _MenuList_ *pre; //��һ���˵�
	struct _MenuList_ *next; //��һ���˵�
	struct _MenuList_ *father; //���˵�
	struct _MenuList_ *child; //�Ӳ˵��б�
} MENULIST;

int SG_DynMenu_Put_MenuList (MENULIST* list);
int SG_DynMenu_Put_InputBox (unsigned char menuid[MENUID_LEN], char *title, char *tail, char password, short maxLen, char charType, char upLow, char card);
int SG_DynMenu_Put_ChooseBox (unsigned char menuid[MENUID_LEN], char *title);
int SG_DynMenu_Put_MsgBox (unsigned char menuid[MENUID_LEN], char *title);
int SG_DynMenu_Put_VoiceBox (unsigned char menuid[MENUID_LEN], char *title, char *phoneNum, char *phoneName);
int SG_DynMenu_Put_AutoShowBox (unsigned char menuid[MENUID_LEN], char *title);
int SG_DynMenu_Put_AutoShowBox_t1 (char tm,unsigned char menuid[MENUID_LEN]);
MENULIST* SG_DynMenu_Get_MenuList (MENULIST *list, unsigned char menuid[MENUID_LEN]);
int SG_DynMenu_HandleMsg (unsigned char *msg);
int SG_DynMenu_Put_AutoShowBox_t1 (char tm,unsigned char menuid[MENUID_LEN]);

#ifdef __cplusplus
}
#endif

#endif //_SG_DYNAMICMENU_H_


