/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_MsgHandle.h
  版本   ：1.50
  创建者 ：李松峰
  创建时间：2007-7-23
  内容描述：SG2000  报文发送和处理  
  修改记录：
***************************************************************/
#ifndef _SG_MSGHANDLE_H_
#define _SG_MSGHANDLE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "msg.h"


// External Variables 


/********************************************************************
*******************************函数声明******************************
********************************************************************/

//创建设置成功响应报文(仅设置类型用)
MsgShort MsgReportSetOk(MsgUChar msgCode, MsgUChar **out, MsgInt *outlen);

//组合创建数据报文
MsgShort MsgMakeMsg(MsgChar key, MsgUChar* data, MsgInt len, MsgUChar ans, MsgUChar **out, MsgInt *outlen);

//ASCII码转换为BCD码(内存空间在外部申请)
MsgShort MsgAscii2Bcd(char *ascii, char *bcd);

//创建数据报文
MsgShort MsgCreatMsg(MsgUChar msgType, MsgUChar msgCode, MsgUChar *data, MsgInt dataLen, MsgUChar **out, MsgInt *outLen);

//定时汇报
MsgShort MsgWatchByTime(gps_data *gps, MsgUChar **out, MsgInt *outlen);

//定次汇报
MsgShort MsgWatchByAmount(gps_data *gps, MsgUChar **out, MsgInt *outlen);
// External Prototypes

//MsgShort ElectronDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

//配置中心APN地址
MsgShort MsgSetMoveCen(MsgUChar **out, MsgInt *outlen);

//设置报警号码
MsgShort MsgSetPoliceNo(MsgUChar **out, MsgInt *outlen);

//设置呼叫限制
MsgShort MsgSetCallLimit(MsgUChar **out, MsgInt *outlen);

//断开GPRS连接	
MsgShort MsgSetDisconnectGprs(MsgUChar **out, MsgInt *outlen);

//设置GPRS初始参数
MsgShort MsgSetInitGprs(MsgUChar **out, MsgInt *outlen);
//回拨坐席电话
MsgShort MsgCallBackToNo(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// 配置用户名和密码	
MsgShort MsgSetUserPsw(MsgUChar **out, MsgInt *outlen);

// 功能:配置中心TCP地址
MsgShort MsgSetTcp(MsgUChar **out, MsgInt *outlen);

//设置超速
MsgShort MsgSetOverSpeed(MsgUChar **out, MsgInt *outlen);

//设置区域	
MsgShort MsgSetRegion(MsgUChar **out, MsgInt *outlen);

//复位
MsgShort MsgSetReset(MsgUChar **out, MsgInt *outlen);

//设置通讯方式
MsgShort MsgSetCommunicationType(MsgUChar **out, MsgInt *outlen);

//设置行车路线
MsgShort MsgSetLine(MsgUChar **out, MsgInt *outlen);

//设置行车路线带超速报警
MsgShort MsgSetLineSpeed(MsgUChar **out, MsgInt *outlen);

//配置代理地址
MsgShort MsgSetProxy(MsgUChar **out, MsgInt *outlen);

//配置代理使能
MsgShort MsgSetProxyEnable(MsgUChar **out, MsgInt *outlen);

//设置通讯超时驾驶
MsgShort MsgSetDrvTout(MsgUChar **out, MsgInt *outlen);

//设置通讯超时停车
MsgShort MsgSetStopTout(MsgUChar **out, MsgInt *outlen);


//设置通讯通话限时
MsgShort MsgSetCallTout(MsgUChar **out, MsgInt *outlen);

//设置定时监控
MsgShort MsgIntervalWtachByTime(gps_data *gps,int bCompress, MsgUChar **out, MsgInt *outlen);

//停止轮询监控
MsgShort MsgWatchIntervalStop(MsgUChar type, MsgUChar **out, MsgInt *outlen);

//监控取消
MsgShort MsgWatchCancle(MsgUChar **out, MsgInt *outlen);

//握手(中心发起)	
MsgShort MsgWatchHanleDown(MsgUChar **out, MsgInt *outlen);

//区域报警取消
MsgShort MsgSafetyRegionCancle(MsgUChar **out, MsgInt *outlen);

//路线报警取消
MsgShort MsgSafetyLineCancle(MsgUChar **out, MsgInt *outlen);

//超速报警取消
MsgShort MsgSafetyOverspeedCancle(MsgUChar **out, MsgInt *outlen);

//重设超级密码
MsgShort MsgSafetyChgPasswd(MsgUChar **out, MsgInt *outlen);

//黑匣子请求应答
MsgShort MsgBlackBoxAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

//黑匣子停止应答
MsgShort MsgBlackBoxStopAck(MsgUChar **out, MsgInt *outlen);

//监听应答
MsgShort MsgListenAck(MsgUChar **out, MsgInt *outlen);

//自定义报警
MsgShort MsgSelfDefineAlarmAck(MsgUChar **out, MsgInt *outlen);

MsgShort MsgSafetyAlarm(gps_data *gps, MsgUChar **out, MsgInt *outlen);
// 设置车台本机号
MsgShort MsgSetOwnNo(MsgUChar **out, MsgInt *outlen);

//  透传设备报告
MsgShort WatchDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// 短信间隔设置应答
MsgShort MsgSmsInterval(MsgUChar **out, MsgInt *outlen,unsigned char ret);

// 摄像头休眠应答
MsgShort MsgCameraRestAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// 摄像头设置请求确认
MsgShort MsgCameraAck(MsgUChar * data, MsgInt len, MsgUChar **out, MsgInt *outlen,int packNo,int packCnt);

// 摄像头停止应答
MsgShort MsgCameraStartAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// 设置报警器应答
MsgShort MsgSetAlarm(MsgUChar **out, MsgInt *outlen);

// 设置移动中心号码应答
MsgShort MsgSetMoveCen(MsgUChar **out, MsgInt *outlen);


// 电子设备报告
MsgShort ElectronDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// 上传文字消息
MsgShort MsgEditSmsUpload(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// 设置求助号码
MsgShort MsgSetHelpNo(MsgUChar **out, MsgInt *outlen);

// 设置医疗求助号码
MsgShort MsgSetMedicalNo(MsgUChar **out, MsgInt *outlen);

// 设置维修求助号码
MsgShort MsgSetServiceNo(MsgUChar **out, MsgInt *outlen);


MsgShort MsgWatchHanleUp(MsgUChar type, MsgUChar **out, MsgInt *outlen);
MsgShort MsgAlarmGps(gps_data *gps, MsgUChar msgType, MsgUChar msgCode, MsgUChar **out, MsgInt *outlen);
void  ParseGPS(gps_data *gps, char *data, int * pos);

MsgShort MsgBusDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

#ifdef __cplusplus
}
#endif

#endif //_SG_MSGHANDLE_H_
