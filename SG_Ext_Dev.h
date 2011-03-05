/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2008 版权所有
  文件名 ：SG_Hh.h
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-7-23
  内容描述：SG2000  报文发送和处理  
  修改记录：
***************************************************************/
#ifndef _SG_Ext_Dev_H_
#define _SG_Ext_Dev_H_


#ifdef __cplusplus
extern "C" {
#endif
//#include "sci_types.h"

// 宏定义



//ZB导航屏相关的宏定义
#define ZB_GPS_START    	0x7E  // 帧头 
#define ZB_GPS_END       	0x7F  // 帧尾 
#define ZB_GPS_SPE		    0x7D  // 

#define CMD_ZB_ACK  	    0x00  // 车台发送状态信息给显示屏
#define CMD_GPS_SEND        0xF0  //GPS数据
#define CMD_CENTER_UP       0x11  //中心导航
#define CMD_SAFETY_SET      0x1E  //PND安全中心设置
#define CMD_CALL_CENTER     0x10  //拨打中心电话（导航屏）
#define CMD_CALL_OUT   		0x30  //拨打电话（导航屏）
#define CMD_CALL_RING   	0x31  //来电振铃(车载终端)
#define CMD_CALL_REV    	0x32  //电话接听(导航屏幕=)
#define CMD_CALL_OFF   	    0x33  //电话挂机(导航屏)
#define CMD_CALL_SPEAK  	0x34  //通知导航屏接通(车载终端)
#define CMD_CALL_END    	0x35  //通知导航屏挂机(车载终端)
#define CMD_CALL_DTMF    	0x36  //拨打分机
#define CMD_CALL_VOL   	    0x37  //音量调节(导航屏)

#define CMD_LETTER_UP  	    0x41  // 中心下发文字信息
#define CMD_SMS_OUT  		0x42   // 向手机发送短消息(导航屏)
#define CMD_SMS_UP       	0x43  // 车台收到手机短信
#define CMD_SMS_TNC       	0x44  // TNC格式短信

#define CMD_STATE_UP 	    0xF3  // 车台发送状态信息给显示屏


// 通话功能相关命令
#define CMD_ZB_DIALNUM     	0X30  //  拨打电话（导航屏->终端）
#define CMD_ZB_INCOMING    	0X31  //  来电振铃（终端->导航屏）
#define CMD_ZB_ANSCALL    	0X32  //  电话接听(导航屏幕->终端) 
#define CMD_ZB_CALLOFF    	0X33  //  电话挂机(导航屏->终端)
#define CMD_ZB_MOANS     	0X34  //  通知导航屏接通(车载终端->导航屏) 
#define CMD_ZB_MOOFF     	0X35  //  通知导航屏挂机(车载终端)
#define CMD_ZB_SENDDTMF     0X36  //  DTMF发送
#define CMD_ZB_SENDVOL     	0X37  //  音量调节(导航屏)
#define CMD_ZB_SENDMICVOL   0X38  //  麦克风增益调节(导航屏)
#define CMD_ZB_SENDMICVOL   0X38  //  麦克风增益调节(导航屏)

// 信息功能 

#define CMD_ZB_HEART 		0x31  // 车台与显示屏定时握手
#define CMD_MSG_UP 		0x32  // 车台发送中文信息到显示屏
#define CMD_DST_UP 		0x11  // 车台发送导航目的地经纬度信息到显示屏
//#define CMD_STATE_UP 	0x34  // 车台发送状态信息给显示屏





#define CMD_RCV_ACK 	0x7A  // 显示屏发送固定短语给车台



// YH导航屏相关宏定义
//编码	信息类型	
#define CMD_YH_SINBYTE		0x01//01H	单字节操作命令	 		双向
#define CMD_YH_DIALNUM		0x02//02H	拨号命令	        		手持终端向车台发送
#define CMD_YH_INCOMING		0x03//03H	送显号码（来电显示）		车台向手持终端发送
#define CMD_YH_STATE		0x05//05H	标志信息				车台向手持终端发送
#define CMD_YH_RCVCENTMSG	0x06//06H	来自中心的短信			车台向手持终端发送
#define CMD_YH_SENDDTMF		0xA2//A2HDTMF					手持终端向车台发送
#define CMD_YH_RCVSMS		0xA6//A6H	一般短信				车台向手持终端发送
#define CMD_YH_SENDSMS		0xA4//A4H	发送短消息				手持终端向车台发送
#define CMD_YH_HEART       	0x0D//心跳包					车台向手持终端发送 3秒钟一次

//单字节指令：01H
// 	手持终端向车台：
#define SINBYTEUP_HEART		0x01//01H	终端工作正常
#define SINBYTEUP_ANSCALL	0x02//02H	终端摘机（接听来电）
#define SINBYTEUP_CALLOFF	0x03//03H	终端挂机（挂断来电）
#define SINBYTEUP_MEDREQ	0x04//04H	医疗服务请求
#define SINBYTEUP_FALREQ	0x05//05H	故障服务请求
#define SINBYTEUP_INFOREQ	0x06//06H	信息服务请求
#define SINBYTEUP_HANDFREE	0x07//07H	耳机与免提切换通话
#define SINBYTEUP_VOLUP		0x09//09H	音量增大
#define SINBYTEUP_VOLDN		0x0A//0AH	音量减少
#define SINBYTEUP_CONNECT	0x0B//0BH	终端连接中
// 	车台向手持终端：
#define SINBYTEDN_INCOMING	0x03//03H	来电显示
#define SINBYTEDN_RING		0x11//11H	振铃
#define SINBYTEDN_MOOFF		0x12//12H	车台在挂机状态（去电，对方主动挂机）
#define SINBYTEDN_MOANS		0x13//13H	车台在摘机状态（去电，对方接听）
#define SINBYTEDN_VIBRATE	0x14//14H	蜂鸣器响两声
#define SINBYTEDN_DISABLE	0x19//19H	禁止呼叫
#define SINBYTEDN_CALLONLY	0x1A//1AH	呼叫受限
#define SINBYTEDN_SENDOK	0x17//17H	短信发送成功
#define SINBYTEDN_SENDERR	0x18//18H	短信发送失败

// 串口通信的帧头和帧尾
#define YH_GPS_START        0x0F // 新导航屏的帧头
#define YH_GPS_END       	0xF0 // 新导航屏的帧尾
#define YH_GPS_END2       	0xFF // 新导航屏的帧尾再尾
// 函数声明

void SG_Ext_Dev_Main(void);
unsigned char * FindChar(unsigned char *buf,int len,unsigned char c);
void UnicodeBE2LE(unsigned char *str,int len);
void Pro_Time_Handle(void);

#ifdef __cplusplus
}
#endif

#endif //_SG_HH_H_


