/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2008 版权所有
  文件名 ：SG_Hh.h
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-7-23
  内容描述：SG2000  报文发送和处理  
  修改记录：
***************************************************************/
#ifndef _SG_HH_H_
#define _SG_HH_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "sci_types.h"



/*串口收到的消息*/
#define REV_OK       		0x00	//	回应帧
#define SEND_BROWSER 		0x01	//	动态菜单
#define ASK_NAME_SINGAL 	0x02	// 	请求返回运营商名称、gsm信号强度
#define ASK_VAL 			0x03	// 	请求返回听筒音量
#define SEND_CALL			0x04	// 	拨打 
#define SEND_REV 			0x05	// 	接听 挂断 
#define SEND_DTMF			0x06	//	发送DTMF码
#define SEND_VAL			0x07	//	调节音量
#define SEND_SMS			0x08	//	发短消息
#define COPY_TEL_BOOK		0x09	// 	发送需要复制的电话号码
#define QUERY_TEL_BOOK		0x0a	// 	申请查询sim卡号码总数
#define GET_TEL_BOOK		0x0b	//	申请获取sim第n条号码
#define SEND_INSCH			0x0c	// 	中文手柄上电报告主机版本号
#define SEND_INIT			0x0d	// 	发送初始化命令
#define SEND_RESET			0x0e	// 	发送重启命令
#define SEND_MT				0x0f	// 	手柄/免提选择
#define SEND_GOOD			0x10	// 	发送手柄工作正常命令
#define PASS_KEY_MIANTI     0x11
#define SG2000_HH_EXIT      0x12	//	手柄退出
#define SG2000_MB_TEST      0x13
#define SG2000_MB_CONFIG    0x14	
#define MUSTER_OK			0x15
#define MUSTER_CANCEL		0x16
#define SEND_HANDFREE		0x10	// 	手柄/免提选择
// 主机与调度屏通信，上行消息

#define RTN_NAME_SINGAL 	0x02
#define RTN_VAL	  			0x03
#define REV_TEL				0x04
#define	REV_TELSTA			0x05 // 向手柄发送通话状态  0x00 呼叫丢失 0x01 呼出0x02 通话
#define SMS_RESULT			0x06
#define	REV_SMS				0x07
#define	REV_TEL_BOOK 		0x08
#define	NUM_TEL_BOOK		0x09
#define SET_TEL_BOOK 		0x0a
#define	RTN_TIME  			0x0b // 向手柄发送时间和日期
#define RTN_ERR  			0x0c
#define RTN_NWRPT			0x0e
#define RTN_AUTO_REV 	 	0x11

//和PC机通信
#define SG_SHOW_DEVICE      0x21    //配置查询
#define SG_SET_DEVICE       0x22    //参数配置
#define SG_SHOW_VER         0x23    //版本查询
#define SG_SHOW_STATE       0x24    //车辆状态
#define SG_SHOW_IO          0x25    //IO接线状态
#define SG_XIH_OFF          0x26    //熄火
#define SG_XIH_ON           0x27    //熄火恢复
#define SG_LISTEN_NOM       0x28    //监听
#define SG_SHOW_NET         0x29    //网络状态

#define SG_PC_FLAG          0x31    //生产测试
#define SG_PC_XHON          0x32    //熄火
#define SG_PC_XHOFF         0x33    //熄火恢复
#define SG_PC_IO            0x34    //IO检测
#define SG_PC_VER           0x35    //版本查询
#define SG_PC_CALL          0x36    //拨打电话
#define SG_PC_STATE         0x37    //车辆状态


// 0x00 丢失  0x02 呼出中 0x03 通话
#define CAll_OFF 	0x00 // 呼叫丢失
#define CALL_OUT  	0x02 // 向外呼出电话
#define CALL_SPEAK	0x03 // 通话



#define ERR_NO_SIM_CARD 0x01
#define ERR_NO_NETWORK  0x02
#define ERR_PIN_CODE    0x03
// 变量定义
extern uint8 setmenu_status;

//结构体声明
#define MAX_FRAME_BUF 580
typedef struct
{
	unsigned char type;      //类型
	unsigned short len;     //长度
	unsigned char dat[MAX_FRAME_BUF]; //数据
} SENDFRAME;  

//结构体声明
typedef struct _SG_HH_LIST_
{
	char *buf;
	int len;
	int com; // 串口号
	struct _SG_HH_LIST_ *next;
}sg_hh_list;


// External Variables 
extern BOOLEAN g_framesendok;
extern unsigned char g_sendframe1s;
extern SENDFRAME g_sendframe;
extern int checkhandset;
// Internal Prototypes



// SMS 相关定义
// 用户信息编码方式
#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS2		8
typedef struct

{

	unsigned char pSCA[20];			// 短消息服务中心号码(SMSC地址)

	unsigned char  pTPA[20];			// 目标号码或回复号码(TP-DA或TP-RA)

	unsigned  char TP_DCS;			// 用户信息编码方式(TP-DCS)

	unsigned char pTP_SCTS[20];		// 服务时间戳字符串(TP_SCTS), 接收时用到

	unsigned char  pTP_UD[255];		// 原始用户信息(编码前或解码后的TP-UD)

	int pDU_Len;                    // pdu的长度

} PDUSTRUCT;




// External Prototypes

// 函数声明
int  CheckSum( unsigned char*pdatas,int len) ;
void SG_Send_Net_State(char type);
void SG_HH_SEND_ITEM(int SignalCode);
int SG_HH_Put_New_Item(int type, int com, unsigned char *buf, int len);
int SG_HH_Put_Item(int type, int com, char *msg, int len, sg_hh_list **list, int *num);
sg_hh_list *SG_HH_Get_Item(sg_hh_list **list, int *num);
void DoUart(void);
void SG_Uart_Send_Msg(unsigned char*msg,short len,uint8 type);


#ifdef __cplusplus
}
#endif

#endif //_SG_HH_H_


