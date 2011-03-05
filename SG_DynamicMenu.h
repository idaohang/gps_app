/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_DynamicMenu.h
  版本   ：1.00
  创建者 ：李松峰
  创建时间：2005-9-15
  内容描述：SG2000主机和手柄之间动态菜单协议        
  修改记录：
***************************************************************/
#ifndef _SG_DYNAMICMENU_H_
#define _SG_DYNAMICMENU_H_

#ifdef __cplusplus
extern "C" {
#endif

//下行 局端至终端
#define TYPE_MENU_LIST         0x01 //菜单列表
#define TYPE_MENU_ITEM        0x02 //菜单项
#define TYPE_MSG_BOX            0x03 //消息框
#define TYPE_VOICE_BOX         0x04 //特殊功能处理框（语音呼叫等）
#define TYPE_AUTOSHOW_BOX 0x05 //自动滚动显示消息框
#define TYPE_AUTOSHOW_BOX_T1 0x06 //自动滚动显示消息框并在规定秒数后返回主界面

#define SIGN_INPUT_BOX   0x01 //输入框标志
#define DECRYPT_BUF         0x00 //明文
#define ENCRYPT_BUF         0x01 //密文
#define ANYCHAR                0x00 //任意字符
#define PURE_NUM              0x01 //纯数字
#define PURE_ENG              0x02 //纯英文
#define PURE_CHN              0x03 //纯汉字
#define ANY_UPLOW           0x00 //大小写不限制
#define PURE_UP                0x01 //限大写
#define PURE_LOW             0x02 //限小写
#define NOCARD                 0x00 //不能刷卡
#define CARD                      0x01 //可以刷卡

#define SIGN_CHOOSE_BOX       0x02 //选择框标志


//上行  终端至局端
#define TYPE_GET_MENU_LIST  0x01 //请求菜单（列表/项）
#define TYPE_MENU_PARA         0x02 //菜单操作参数回送
#define TYPE_SHORTCUT_KEY    0x03 //快捷键回送
#define TYPE_MUSTER_ACK	  0x04 //电召应答

#define SIGN_GET_CHILD_MENU     0x01 //请求子菜单（列表/项）
#define SIGN_GET_FATHER_MENU  0x02 //请求父菜单（列表）
#define SIGN_GET_MAIN_MENU      0x03 //请求主菜单

#define SIGN_INPUT_BOX_PARA     0x01 //输入框参数回送
#define SIGN_CHOOSE_BOX_PARA  0x02 //选择框参数回送
#define SIGN_MSG_BOX_PARA  	0x03 //选择框参数回送

#define INPUT_CHOOSE_YES	0x00 //输入框/选择框确定参数
#define INPUT_CHOOSE_NO  	0x01 //输入框/选择框取消参数

//快捷键ID
#define SHORTCUT_MEDICAL  	0x01 //医疗求助快捷键
#define SHORTCUT_SERVICE  	0x02 //维修求助快捷键
#define SHORTCUT_INFO	  	0x00 //信息求助快捷键


//常量
#define SEPARATOR       '\0' //分隔符
#define END                   '\0' //结束符
#define MENUID_LEN            (sizeof(int)) //菜单ID长度4字节
#define INPUT_BOX_TITLE_LEN   15 //输入框提示说明头最大长度
#define INPUT_BOX_TAIL_LEN     4 //输入框提示说明尾最大长度
#define FIRST_MENUID 0x00000000 //主菜单首ID，手柄根据此判断是否主菜单
#define MENUID_SINGLE 0xff123456 //独立动态菜单项ID

//菜单项结构
typedef struct _MenuItem_
{
	unsigned char menuid[MENUID_LEN]; //菜单ID
	char *s; //菜单说明
	int (*proc)(unsigned char menuid[MENUID_LEN], void *s1, void *s2); //回调函数:返回值0表示需要回送菜单
	int count; //调用菜单的级数
	void *data; //附件参数指针(回调函数存数据用)
} MENUITEM;

//菜单链表结构
typedef struct _MenuList_
{
	MENUITEM *item; //菜单项
	struct _MenuList_ *pre; //上一个菜单
	struct _MenuList_ *next; //下一个菜单
	struct _MenuList_ *father; //父菜单
	struct _MenuList_ *child; //子菜单列表
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


