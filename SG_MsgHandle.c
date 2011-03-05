/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2007-2008 版权所有
  文件名 ：SG_MsgHandle.c
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2005-7-23
  内容描述：SG2000  报文发送和处理
  修改记录：

***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os_api.h"

#include "msg.h"
#include "SG_GPS.h"
#include "SG_Send.h"
#include "SG_Set.h"
#include "SG_CRC16.h"
#include "SG_MsgHandle.h"
#include "SG_Receive.h"
#include "SG_Camera.h"

extern unsigned char*Camera_Buf;
extern int Camera_BufCnt ;


/*****************************************************************
函数说明:  ParseGPS

输入参数:   gps   	指向需要解析的 GPS 结构体
         	data  	指向封装后的内容
        	pos  	指向从 data 开始存储的位置
输出:   	从 data+*pos 开始存储解析出的内容
   			pos 指针移到缓存区的结尾
修改记录:
			添加4个字节总里程统计，陈海华，2006-5-29
*****************************************************************/
void  ParseGPS(gps_data *gps, char *data, int * pos)
{
	int p = *pos;
	MsgChar tt[5] = "";
	MsgShort nAltitude;
	unsigned long alarmState;
	SCI_DATE_T	currDate={0};
	SCI_TIME_T	currTime={0xff};

	//状态报警字
	alarmState = htonl(g_state_info.alarmState);
	SCI_MEMCPY(&data[p],&alarmState,4);
	p += 4;
	//日期、时间
	tt[0] = gps->sDate[4];
	tt[1] = gps->sDate[5];
	data[p] = (char)atoi(tt);
	if(data[p] > 99 || data[p] < 7){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currDate.year == 0)
				TM_GetSysDate(&currDate);

			data[p] = currDate.year -2000;
		}
	}

	p += 1;
	tt[0] = gps->sDate[2];
	tt[1] = gps->sDate[3];
	data[p] = (char)atoi(tt);
	if(data[p] > 12 || data[p] < 1){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currDate.year == 0)
				TM_GetSysDate(&currDate);

			data[p] = currDate.mon;
		}
	}

	p += 1;
	tt[0] = gps->sDate[0];
	tt[1] = gps->sDate[1];
	data[p] = (char)atoi(tt);
	if(data[p] > 31 || data[p] < 1){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currDate.year == 0)
				TM_GetSysDate(&currDate);

			data[p] = currDate.mday;
		}
	}

	p += 1;
	tt[0] = gps->sTime[0];
	tt[1] = gps->sTime[1];
	data[p] = (char)atoi(tt);
	if(data[p] > 23/* || data[p] < 0*/){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currTime.hour == 0xff)
				TM_GetSysTime(&currTime);

			data[p] = currTime.hour;
		}
	}

	p += 1;
	tt[0] = gps->sTime[2];
	tt[1] = gps->sTime[3];
	data[p] = (char)atoi(tt);
	if(data[p] > 59/* || data[p] < 0*/){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currTime.hour == 0xff)
				TM_GetSysTime(&currTime);

			data[p] = currTime.min;
		}
	}

	p += 1;
	tt[0] = gps->sTime[4];
	tt[1] = gps->sTime[5];
	data[p] = (char)atoi(tt);
	if(data[p] > 59/* || data[p] < 0*/){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currTime.hour == 0xff)
				TM_GetSysTime(&currTime);

			data[p] = currTime.sec;
		}
	}

	p += 1;
	//经度
	SCI_MEMCPY(tt, gps->sLongitude, 3);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLongitude[3], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLongitude[6], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLongitude[8], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	//纬度
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, gps->sLatitude, 2);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLatitude[2], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLatitude[5], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLatitude[7], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	//速度
	data[p] = (char)atoi(gps->sSpeed);
	p += 1;
	//方向
	data[p] = (char)((atoi(gps->sAngle)+1)/2);
	p += 1;
	//加速度
	data[p] = gps->nAcc;
	p += 1;
	//行驶距离
	//big-endian

	if(((gps_data *)(g_state_info.pGpsCurrent))->status)
	{
		data[p] = (MsgUChar)(*((MsgUChar*)(&gps->nDistance) + 1));
		data[p+1] = (MsgUChar)(*((MsgUChar*)(&gps->nDistance)));
	}
	else
	{
		data[p] = 0;
		data[p+1] = 0;
	}
	p += 2;
	//高度
	//big-endian
	nAltitude = (MsgShort)atoi(gps->sAltitude);
	if (nAltitude > 30000 || nAltitude < -10000)
		nAltitude = 0;
	data[p] = (MsgUChar)(*((MsgUChar*)(&nAltitude) + 1));
	data[p+1] = (MsgUChar)(*((MsgUChar*)(&nAltitude)));
	p += 2;
	//星况
	data[p] = gps->status;
	p += 1;
	//星数
	data[p] = gps->nNum;
	p += 1;
	//总里程统计
	data[p] = (char)((g_state_info.nGetTotalDistance >> 24)&0xff);
	data[p+1] = (char)((g_state_info.nGetTotalDistance >> 16)&0xff);
	data[p+2] = (char)((g_state_info.nGetTotalDistance >> 8)&0xff);
	data[p+3] = (char)g_state_info.nGetTotalDistance & 0xff;
	p +=4;

	*pos = p;
}


/****************************************************************
  函数名  ：MsgReportGps
  功  能  ：GPS信息汇报(定时、定距、定次、报警)
  输入参数：
  				gps: gps信息结构
  				bCompress: 是否压缩发送0不压缩1压缩
  				msgType:命令类型
  				msgCode:命令代码
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
  去除压缩发送时压缩完才发送的限制，2005-11-23
  方向单位是2度，李松峰，2005-12-14
  增加加速度和行驶距离数值，李松峰，2005-12-14
  (char)atoi(gps->sAngle/2改为(char)(atoi(gps->sAngle)/2)，李松峰，2005-12-17
  海拔的值超大时归0，李松峰，2005-12-28
****************************************************************/
static MsgShort MsgReportGps(gps_data *gps, MsgChar bCompress, MsgUChar msgType, MsgUChar msgCode, MsgUChar **out, MsgInt *outlen)
{
	MsgUChar* data = NULL;
	MsgInt len, p = 0;//, i;
	int temp;

	if (gps == NULL || out == NULL || outlen == NULL)
	{
		SCI_TRACE_LOW("<<<<<<MsgReportGps: PARA ERR, TYPE: 0x%02x, CODE: 0x%02x", msgType, msgCode);
		return MSG_FALSE;
	}

	len = MSG_GPS_LEN;

	data = SCI_ALLOC( len);
	if (data == NULL)
	{
		SCI_TRACE_LOW("<<<<<<MsgReportGps: CALLOC: %d", len);
		return MSG_FALSE;
	}

	SCI_MEMSET(data,0,len);

	//参数内容:GPS数据包
	ParseGPS(gps,(char*)data,(int*)&p);

//	附加GPS数据
//附加数据标记（4）+n*（附加数据类型代码（1）+附加数据长度（1）+附加数据体4）
//其中，附加数据标志为4个字节的十六进制，值固定为FFFFFFFF
//速度小数部分	3	速度的小数部分（4），单位0.0001 海里/时

//start
	data[p++]=0xff;
	data[p++]=0xff;
	data[p++]=0xff;
	data[p++]=0xff;

	if(g_set_info.bNewUart4Alarm & UT_LOCK)// 中控锁开启时才显示此附加数据
	{
	data[p++]=0x01;  //附加数据类型
	data[p++]=0x08;  //附件数据长度

	//需要上传的当前油量
	data[p] = (char)((g_state_info.upOilQuantity>> 24)&0xff);
	data[p+1] = (char)((g_state_info.upOilQuantity >> 16)&0xff);
	data[p+2] = (char)((g_state_info.upOilQuantity >> 8)&0xff);
	data[p+3] = (char)g_state_info.upOilQuantity & 0xff;
	p+=4;

	//需要上传的当前AD值
	data[p] = (char)((g_state_info.upAD>> 24)&0xff);
	data[p+1] = (char)((g_state_info.upAD >> 16)&0xff);
	data[p+2] = (char)((g_state_info.upAD >> 8)&0xff);
	data[p+3] = (char)g_state_info.upAD & 0xff;
	p+=4;
	}

	data[p++]=0x03;  //附加数据类型
	data[p++]=0x04;  //附件数据长度

	temp = (atof(gps->sSpeed) - atoi(gps->sSpeed))*10000;

	SCI_MEMCPY(&data[p],&temp,4);
	p += 4;

	if (MsgCreatMsg(msgType, msgCode, data, p, out, outlen) == MSG_FALSE)
	{
		if (data)
			SCI_FREE(data);
		return MSG_FALSE;
	}
	else
	{
		if (data)
			SCI_FREE(data);
		return MSG_TRUE;
	}
}

/****************************************************************
  函数名  ：MsgSetApn
  功  能  ：配置中心APN地址
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetApn(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_APN, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetApn
  功  能  ：设置移动中心号码
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/

MsgShort MsgSetMoveCen(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_MOVE_CEN, NULL, 0, out, outlen);
}


/****************************************************************
  函数名  ：MsgCameraRestAck
  功  能  ：摄像头休眠应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：陈海华
  修改记录：创建，2006-3-23
****************************************************************/

MsgShort MsgCameraRestAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_CAMERA, MSG_CAMERA_REST_ACK, msg, len, out, outlen);
}


/****************************************************************
  函数名  ：MsgCameraStartAck
  功  能  ：摄像头停止应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：陈海华
  修改记录：创建，2006-3-23
****************************************************************/
MsgShort MsgCameraStartAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_CAMERA, MSG_CAMERA_START_ACK, msg, len, out, outlen);
}


/****************************************************************
  函数名  ：MsgSetHelpNo
  功  能  ：设置求助号码
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetHelpNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_HELP_NO, NULL, 0, out, outlen);
}


/****************************************************************
  函数名  ：MsgSetMedicalNo
  功  能  ：设置医疗救护电话号码
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetMedicalNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_MEDICAL_NO, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetServiceNo
  功  能  ：设置维修电话号码
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetServiceNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_SERVICE_NO, NULL, 0, out, outlen);
}




/****************************************************************
  函数名  ：MsgSetTempRange
  功  能  ：设置温度上下值应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  修改记录：创建，2006-9-18   陈利
****************************************************************/
MsgShort MsgEditSmsUpload(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_MUSTER,MSG_SMS_UPLOAD,msg,len,out,outlen);
}




/****************************************************************
  函数名  ElectronDevReport
  功  能  ：电子设备报告
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
 修改记录:创建 2006-8-8   陈利
****************************************************************/
MsgShort ElectronDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_EXT_DEV, MSG_EXT_DEV_REPORT, msg, len, out, outlen);

}




/****************************************************************
  函数名  ：MsgCameraAck
  功  能  ：摄像头设置请求确认
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：陈海华
  修改记录：创建，2006-3-14
  	去掉图像数据前面4个字节图像长度,陈海华,2006-6-6
****************************************************************/
MsgShort MsgCameraAck(MsgUChar * data, MsgInt len, MsgUChar **out, MsgInt *outlen,int packNo,int packCnt)
{
	int  pos = 0;
	unsigned short pic_Tx_Len = 0;
	static unsigned char *para =NULL;
	int nCameraID = 1;
	gps_data* gps = (gps_data*)g_state_info.pGpsFull;

#if (0)
	{
		strcpy(gps->sLongitude,"119.2953");
		strcpy(gps->sLatitude,"26.0827");
		strcpy(gps->sSpeed,"100");
		gps->nAcc = 15;
		strcpy(gps->sEastWest,"E");
		strcpy(gps->sSouthNorth,"N");
		gps->status = 1;
		gps->nNum = 9;
		strcpy(gps->sDate,"240408");
		strcpy(gps->sTime,"112233");
	}
#endif /* (0) */


	//青青子木摄像头
	if(g_set_info.bNewUart4Alarm&UT_QQCAMERA)
	{
		if(packCnt>1)
		{
			para = data;
		}
		else
		{
			para = data +2;
		}
	}
	else if(g_set_info.bNewUart4Alarm&UT_XGCAMERA)
	{
		para = data;
	}


	//data结构:图像数据
	//para结构:应答类型（1）+ 摄像头ID（1） +GPS长度(2)+GPS报文(31)
	//+ 图像数据大小（2）+图像数据


    // 超过15k大小的图片采用分包协议
    //para结构:应答类型（1）+摄像头ID（1）+GPS报文大小（2）+GPS报文（31）
    //+图象分片序号（1）+图象分片总数（1）+图象数据大小（2）+图象数据（N）


	//应答类型
	para[pos] = 0X00;
	pos ++;

	// 摄像头ID
	para[pos] = nCameraID;  // 要加入对cameraid的判断
	pos ++;

	//gps报文长度
	para[pos] = ((MSG_GPS_LEN>>8)&0xff);
	pos ++;
	para[pos] = (MSG_GPS_LEN&0xff);
	pos ++;

	//拷贝gps报文
	ParseGPS(g_state_info.pGpsFull, (char*)para, &pos);

	if(packCnt>1)
	{
		para[pos] = packNo;
		pos ++;
		para[pos] = packCnt;
		pos ++;

	}

	//拷贝图像长度
	pic_Tx_Len = len;
	memcpy(&para[pos],(char *)&pic_Tx_Len,2);

	pos +=2;

	//拷贝图像

	pos += len;
#if (0)
	{
		int i;
		for(i=pos - 10;i<pos;i++)
		{
			SCI_TRACE_LOW("== %02x",para[i]);
		}
	}
#endif /* (0) */


	memcpy(g_state_info.sMsgNo, g_set_info.sCameraMsgNo, SG_MSG_NO_LEN);

	if(packCnt > 1)
	{
		return MsgCreatMsg(MSG_TYPE_CAMERA, MSG_CAMERA_SET_PACKAGE_ACK, para, pos, out, outlen);
	}
	else
	{
	return MsgCreatMsg(MSG_TYPE_CAMERA, MSG_CAMERA_SET_ACK, para, pos, out, outlen);
	}


	return 1;
}




/****************************************************************
  函数名  ：MsgSetPoliceNo
  功  能  ：设置报警号码
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetPoliceNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_POLICE_N0, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetCallLimit
  功  能  ：设置呼叫限制
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetCallLimit(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_CALL_LIMIT, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetDisconnectGprs
  功  能  ：断开GPRS连接
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetDisconnectGprs(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_DISCONNECT_GPRS, NULL, 0, out, outlen);
}


/****************************************************************
  函数名  ：MsgSetUserPsw
  功  能  ：配置用户名和密码
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetUserPsw(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_USER_PSW, out, outlen);
}


/****************************************************************
  函数名  ：MsgSetTcp
  功  能  ：配置中心TCP地址
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetTcp(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_TCP, out, outlen);
}


/****************************************************************
  函数名  ：MsgSetOverSpeed
  功  能  ：设置超速
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetOverSpeed(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_OVERSPEED, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetRegion
  功  能  ：设置区域
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetRegion(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_REGION, out, outlen);
}


/****************************************************************
  函数名  ：MsgSetOneGPSNum
  功  能  ：设置一键导航通话应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetOneGPSNum(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_ONEGPS_NUM, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetReset
  功  能  ：复位
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetReset(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_RESET, NULL, 0, out, outlen);
}


/****************************************************************
  函数名  ：MsgSetCommunicationType
  功  能  ：设置通讯方式
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetCommunicationType(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_COMMUNICATION_TYPE, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetADAlarm
  功  能  ：油量报警阀值应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetADAlarm(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_ALARM_AD, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetPND
  功  能  ：设置本地导航应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：林兆剑
  修改记录：创建，2009-3-6
****************************************************************/
MsgShort MsgSetPND(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_PND, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetConsult
  功  能  ：配置检测参考点
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetConsult(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_OIL_CONSULT, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetLine
  功  能  ：设置行车路线
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetLine(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_LINE, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetLine
  功  能  ：设置行车路线带超速报警
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetLineSpeed(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_LINE_SPEED, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetProxy
  功  能  ：配置代理地址
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetProxy(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_PROXY, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetProxyEnable
  功  能  ：配置代理使能
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetProxyEnable(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_PROXY_ENABLE, out, outlen);
}


/****************************************************************
  函数名  ：MsgSetBlindEnable
  功  能  ：设置通讯超时停车
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetStopTout(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_STOP_TIMEOUT, out, outlen);
}


/****************************************************************
  函数名  ：MsgSetDrvTout
  功  能  ：设置通讯超时驾驶
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetDrvTout(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_DRIVE_TIMEOUT, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetCallTout
  功  能  ：设置通讯通话限时
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetCallTout(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_CALL_TIMEOUT, out, outlen);
}

/****************************************************************
函数名  ：MsgListenAck
功  能  ：监听应答
输入参数 ：
输出参数 ：
			out:输出报文
			outlen:输出长度
			MSG_TRUE :成功
			MSG_FALSE:失败
编写者  ：谢科迪
修改记录 ：创建，2010-05-05
****************************************************************/
MsgShort MsgSetListenOk(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_LISTEN_NO, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgIntervalWtachByTime
  功  能  ：设置定时监控
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgIntervalWtachByTime(gps_data *gps,int bCompress, MsgUChar **out, MsgInt *outlen)
{
	return MsgReportGps(gps, bCompress, MSG_TYPE_WATCH, MSG_WATCH_ANS_INTERVALCHECK, out, outlen);
}


/****************************************************************
  函数名  ：MsgWatchIntervalStop
  功  能  ：停止轮询监控
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgWatchIntervalStop(MsgUChar type, MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_ANS_INTERVALSTOP, &type, 1, out, outlen);
}

/****************************************************************
  函数名  ：MsgWatchCancle
  功  能  ：监控取消
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgWatchCancle(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_ANS_CANCLE, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgWatchHanleDown
  功  能  ：握手(中心发起)
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgWatchHanleDown(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_ANS_HAND_DOWN, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgSafetyRegionCancle
  功  能  ：区域报警取消
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSafetyRegionCancle(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_REGION_CANCLE, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgSafetyLineCancle
  功  能  ：路线报警取消
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSafetyLineCancle(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_LINE_CANCLE, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgSafetyOverspeedCancle
  功  能  ：超速报警取消
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSafetyOverspeedCancle(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_OVERSPEED_CANCLE, NULL, 0, out, outlen);
}

/****************************************************************
函数名  ：MsgListenAck
功  能  ：监听应答
输入参数：
输出参数：
			out:输出报文
			outlen:输出长度
			MSG_TRUE :成功
			MSG_FALSE:失败
编写者  ：李松峰
修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgListenAck(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_LISTEN, NULL, 0, out, outlen);
}


/****************************************************************
  函数名  ：MsgSafetyOverspeedCancle
  功  能  ：重设超级密码
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSafetyChgPasswd(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_CHGPASSWD, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetHelpNo
  功  能  ：回拨坐席号码	
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgCallBackToNo(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_CALL_BACK_TO, msg,len, out, outlen);
}

/****************************************************************
  函数名  ：MsgBlackBoxAck
  功  能  ：黑匣子请求应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：陈海华
  修改记录：创建，2006-5-28
****************************************************************/
MsgShort MsgBlackBoxAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_BLACK_BOX_ACK, msg, len, out, outlen);
}

/****************************************************************
  函数名  ：MsgBlackBoxStopAck
  功  能  ：黑匣子停止应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：陈海华
  修改记录：创建，2006-5-28
****************************************************************/
MsgShort MsgBlackBoxStopAck(MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_BLACK_BOX_STOP_ACK, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  MsgSelfDefineAlarmAck
  功  能  ：自定义报警
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：陈海华
  修改记录：创建，2006-9-8
****************************************************************/
MsgShort MsgSelfDefineAlarmAck(MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_SELF_DEFINE_ALARM, NULL,0, out, outlen);

}

/****************************************************************
  函数名  ：MsgSetOwnNo
  功  能  ：设置车台本机号
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetOwnNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_OWN_NO, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetInitGprs
  功  能  ：设置GPRS初始参数
  输入参数：out:输出报文
  			outlen:输出长度
  输出参数：MSG_TRUE :成功
  			MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetInitGprs(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_INIT_GPRS, out, outlen);
}


/****************************************************************
  函数名  ：MsgReportSetOk
  功  能  ：创建设置成功响应报文(仅设置类型用)
  输入参数：
  				code:命令代码
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgReportSetOk(MsgUChar msgCode, MsgUChar **out, MsgInt *outlen)
{
	MsgUChar data[1];

	data[0] = MSG_SET_OK;

	return MsgCreatMsg(MSG_TYPE_SET, msgCode, data, 1, out, outlen);
}


/****************************************************************
  函数名  ：MsgCreatMsg
  功  能  ：创建数据报文
  输入参数：
  				msgType:命令类型
  				msgCode:命令代码
  				data:数据区
  				dataLen:数据区长度
  输出参数：
  				out:输出报文
  				outLen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
  	未交换密钥时只能创建可以用固定密钥的报文
  	其他返回失败，2005-11-9，李松峰
  	增加电召报文序列号区分，报警汇报、信息点播请求、
  	导航请求、密钥请求序列号为0，李松峰，2005-12-15
  	BUG:监控汇报使用监控序列号，李松峰，2005-12-16
  	本机号设置应答只有在第一次设置时使用固定密钥，李松峰，2005-12-18
****************************************************************/
MsgShort MsgCreatMsg(MsgUChar msgType, MsgUChar msgCode, MsgUChar *data, MsgInt dataLen, MsgUChar **out, MsgInt *outLen)
{
	MsgUChar* msg = NULL;
	MsgInt len, p = 0;
	MsgUChar ans=0x30;
	MsgChar key;


	SCI_TRACE_LOW( "******MsgCreatMsg");

	if (out == NULL || outLen == NULL || dataLen < 0)
	{
		SCI_TRACE_LOW( "MsgCreatMsg: PARA ERR, TYPE: 0x%02x, CODE: 0x%02x, DATALEN: %d\r\n", msgType, msgCode, dataLen);
		return MSG_FALSE;
	}

	if (data == NULL && dataLen > 0)
	{
		SCI_TRACE_LOW( "MsgCreatMsg: DATA ERR, TYPE: 0x%02x, CODE: 0x%02x, DATALEN: %d\r\n", msgType, msgCode, dataLen);
		return MSG_FALSE;
	}

	len = MSG_NO_LEN + MSG_SIGN_LEN + dataLen;
	msg = SCI_ALLOC( len);
	if (msg == NULL)
	{
		SCI_TRACE_LOW( "MsgCreatMsg: CALLOC: %d \r\n", len);
		return MSG_FALSE;
	}

	SCI_MEMSET(msg,0,len);

	if (msgType == MSG_TYPE_MUSTER)
	{
		SCI_MEMCPY(msg, g_set_info.sMusterMsgNo, SG_MSG_NO_LEN);
		p += SG_MSG_NO_LEN;
	}
	else if (	(msgType == MSG_TYPE_SAFETY && msgCode == MSG_SAFETY_ANS_POLICE)
		|| (msgType == MSG_TYPE_INFO && (msgCode == MSG_INFO_ASK_MENU || msgCode == MSG_INFO_ASK || msgCode == MSG_INFO_CANCLE))
		|| (msgType == MSG_TYPE_NAVIGATION && (msgCode == MSG_NAVIGATION_QUERY_CURRENT_LOCATION || msgCode == MSG_NAVIGATION_ASK_INFO_MENU || msgCode == MSG_NAVIGATION_QUERY_INFO_ROUND))
		|| (msgType == MSG_TYPE_KEY && msgCode == MSG_KEY_ASK)
		|| (msgType == MSG_TYPE_SAFETY && msgCode == MSG_SAFETY_ANS_ALARM))
	{
		SCI_MEMSET(msg, 0, SG_MSG_NO_LEN);
		p += SG_MSG_NO_LEN;
	}
	else if(msgType==MSG_TYPE_EXT_DEV){
		memcpy(msg, g_set_info.sExtMsgNo, SG_MSG_NO_LEN);
		p += SG_MSG_NO_LEN;
	}
	else
	{
		SCI_MEMCPY(msg, g_state_info.sMsgNo, SG_MSG_NO_LEN);
		p += SG_MSG_NO_LEN;
	}

	//命令标识
	msg[p] = msgType;
	p += 1;
	msg[p] = msgCode;
	p += 1;

	//数据区
	if (data != NULL && dataLen > 0)
	{
		SCI_MEMCPY(&msg[p], data, dataLen);
		p += dataLen;
	}

	if (MsgMakeMsg(key, msg, len, ans, out, outLen) == MSG_FALSE)
	{
		if (msg)
			SCI_FREE(msg);
		SCI_TRACE_LOW( "MsgCreatMsg: TYPE: 0x%02x, CODE: 0x%02x, DATALEN: %d\r\n", msgType, msgCode, dataLen);
		return MSG_FALSE;
	}
	else
	{
		if (msg)
			SCI_FREE(msg);

		SCI_TRACE_LOW( "MsgCreatMsg: TYPE: 0x%02x, CODE: 0x%02x, DATALEN: %d\r\n", msgType, msgCode, dataLen);
		return MSG_TRUE;
	}
}


/****************************************************************
  函数名  ：MsgMakeMsg
  功  能  ：组合创建数据报文
  输入参数：
  				data: 报文内容(未压缩加密)
  				len: 报文内容长度o
  				ans: 操作响应代码
  				key:0:固定密钥1: 工作密钥
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
  将预分配固定空间改为实际使用空间，李松峰，2005-12-27
****************************************************************/
MsgShort MsgMakeMsg(MsgChar key, MsgUChar* data, MsgInt len, MsgUChar ans, MsgUChar **out, MsgInt *outlen)
{
	MsgInt p = 0;
	MsgUChar *msg = MSG_NULL;//, *msgdesed = MSG_NULL;
	MsgInt msglen = 0;//, msgdesedlen = 0;
	unsigned short CRC;
	MsgChar sASCIICenterNo[32 + 1] = "", sBCDCenterNo[MSG_OWN_NO_LEN] = "";

	SCI_TRACE_LOW( "********MsgMakeMsg: PARA %d\r\n",len);

	if (out == NULL || outlen == NULL)
	{
		SCI_TRACE_LOW( "MsgMakeMsg: PARA \r\n");
		return MSG_FALSE;
	}


	//预分配输出报文空间
	msglen = MSG_HEAD_LEN + MSG_OWN_NO_LEN + 1 + len + MSG_CRC_LEN + 1;
	msg = SCI_ALLOC( msglen);
	if (msg == MSG_NULL)
	{
		SCI_TRACE_LOW( "MsgMakeMsg: CALLOC: %d \r\n", msglen);
		return MSG_FALSE;
	}

	SCI_MEMSET(msg,0,msglen);

	//报文帧头（不压缩不加密，定长2位）
	SCI_MEMCPY(&msg[p], MSG_HEAD, MSG_HEAD_LEN);
	p += MSG_HEAD_LEN;

	//终端号部分（不压缩不加密，15位ASCII码转换定长6位BCD码）
	strcpy(sASCIICenterNo, g_set_info.sOwnNo);
	MsgAscii2Bcd(sASCIICenterNo, sBCDCenterNo);
	SCI_MEMCPY(&msg[p], sBCDCenterNo, MSG_OWN_NO_LEN);
	p += MSG_OWN_NO_LEN;

	//操作响应代码（不压缩不加密，定长1位）
	SCI_MEMCPY(&msg[p], &ans, 1);
	p += 1;

	//数据区
	if (data != MSG_NULL && len > 0)
	{
		SCI_MEMCPY(&msg[p],data,len);
		p += len;
	}

	//CRC校验（不压缩不加密）
	CRC = Msg_Get_CRC16(msg, p);

	msg[p] = CRC>>8;
	msg[p+1] = CRC;
	p += MSG_CRC_LEN;

	//输出
	*out = msg;
	*outlen = p;

	return MSG_TRUE;
}


/****************************************************************
  函数名  ：MsgAscii2Bcd
  功  能  ：ASCII码转换为BCD码(内存空间在外部申请)
  输入参数：
  				ascii: 输入ASCII码字符串
  输出参数：
  				bcd:输出BCD码
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-23
  	奇数总数位数据转换错误，李松峰，2005-11-17
****************************************************************/
MsgShort MsgAscii2Bcd(char *ascii, char *bcd)
{
	MsgInt len, i = 0, j = 0;
	unsigned char c, c1, c2;

	if (ascii == NULL || bcd == NULL)
	{
		SCI_TRACE_LOW( "MsgAscii2Bcd: PARA \r\n");
		return MSG_FALSE;
	}

	len = strlen(ascii);

	for (i = 0; i < len; i++)
	{
		c1 = ascii[i++];

		if(i < len)
			c2 = ascii[i];
		else
			c2 = 0;

		c1 <<= 4;
		c2 &= 0x0f;
		c = (char)(c1 | c2);

		bcd[j++] = c;
	}

	return MSG_TRUE;
}

MsgShort MsgSafetyAlarm(gps_data *gps, MsgUChar **out, MsgInt *outlen)
{
	return MsgAlarmGps(gps, MSG_TYPE_SAFETY, MSG_SAFETY_ANS_ALARM, out, outlen);
}


/****************************************************************
  函数名  ：MsgWatchByDistance
  功  能  ：定距汇报
  输入参数：
  				gps: gps信息结构
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
    去除压缩发送参数，全部设为可压缩，2005-11-23
****************************************************************/
MsgShort MsgWatchByDistance(gps_data *gps, MsgUChar **out, MsgInt *outlen)
{
	return MsgReportGps(gps, 1, MSG_TYPE_WATCH, MSG_WATCH_ANS_DISTANCE, out, outlen);
}


/****************************************************************
  函数名  ：MsgWatchByTime
  功  能  ：定时汇报
  输入参数：
  				gps: gps信息结构
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
  去除压缩发送参数，全部设为可压缩，2005-11-23
****************************************************************/
MsgShort MsgWatchByTime(gps_data *gps, MsgUChar **out, MsgInt *outlen)
{
	return MsgReportGps(gps, 1, MSG_TYPE_WATCH, MSG_WATCH_ANS_TIME, out, outlen);
}

/****************************************************************
  函数名  ：MsgWatchByAmount
  功  能  ：定次汇报
  输入参数：
  				gps: gps信息结构
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
    去除压缩发送参数，全部设为可压缩，2005-11-23
****************************************************************/
MsgShort MsgWatchByAmount(gps_data *gps, MsgUChar **out, MsgInt *outlen)
{
	return MsgReportGps(gps, 1, MSG_TYPE_WATCH, MSG_WATCH_ANS_AMOUNT, out, outlen);
}


/****************************************************************
  函数名  ：MsgWatchHanleUp
  功  能  ：握手(终端发起)
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgWatchHanleUp(MsgUChar type, MsgUChar **out, MsgInt *outlen)
{
	MsgUChar  pdu[4] = {0};

	SCI_TRACE_LOW("*******MsgWatchHanleUp");
	pdu[0] = type;
	pdu[2] = 2;
	pdu[3] = 98;
	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_HAND_UP, pdu, 4, out, outlen);
}


/****************************************************************
  函数名  ：MsgSetListenNo
  功  能  ：设置中心监听录音电话号码
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetListenNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_LISTEN_NO, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetTotalDist
  功  能  ：设置修改总里程应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  修改记录：创建，2006-9-18   陈利
****************************************************************/
MsgShort MsgSetTotalDist(MsgUChar **out, MsgInt *outlen)
{
	static unsigned char para[100]={0};
	int pos=0;

	ParseGPS(g_state_info.pGpsFull, (char*)para, &pos);
	SCI_TRACE_LOW( "MsgSetTotalDist:MsgLen:%d\r\n",pos);
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_TOTAL_DISTANSE,para,pos,out,outlen);
}


/****************************************************************
  函数名  ：MsgSetCheckItself
  功  能  ：自检
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetCheckItself(MsgUChar *data, MsgInt len, MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_CHECK, data, len, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetCheckDrive
  功  能  ：驾培
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetCheckDrive(MsgUChar *data, MsgInt len, MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_DRIVE, data, len, out, outlen);
}

/****************************************************************
  函数名  ：MsgVerAck
  功  能  ：版本信息查询应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：陈海华
  修改记录：创建，2006-3-21
****************************************************************/
MsgShort MsgVerAck(MsgUChar * data, MsgInt len, MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_VER_ACK, data, len, out, outlen);
}

/****************************************************************
  函数名  ：MsgSetRegionNew
  功  能  ：设置区域
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetRegionNew(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_REGION_NEW, out, outlen);
}


MsgShort MsgSmsInterval(MsgUChar **out, MsgInt *outlen,unsigned char ret)
{
	unsigned char c = (unsigned char)ret;

	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_SMS_INTERVAL, &c, 1, out, outlen);
}


/****************************************************************
  函数名  ：MsgDelInterestPiont
  功  能  ：删除兴趣点应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  修改记录：创建，2006-9-18   陈利
****************************************************************/
MsgShort MsgDelInterestPiont(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_SET, MSG_DEL_ANS_INFO_MUSTER, msg, len, out, outlen);

}
/****************************************************************
  函数名  ：MsgSetInfoMuster
  功  能  ：设置兴趣点应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  修改记录：创建，2006-9-18   陈利
****************************************************************/
MsgShort MsgSetInterestPiont(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_INFO_MUSTER, msg, len, out, outlen);

}


/****************************************************************
  函数名  ：MsgSetAlarm
  功  能  ：设置报警器
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetAlarm(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_ALARM, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  WatchDevReport
  功  能  ：透传设备报告
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
 修改记录:创建 2006-8-8   陈利
****************************************************************/
MsgShort WatchDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_EXT_DEV, MSG_EXT_DEV_REPORT, msg, len, out, outlen);
}



/****************************************************************
  函数名  ：MsgReportGps
  功  能  ：GPS信息汇报(定时、定距、定次、报警)
  输入参数：
  				gps: gps信息结构
  				bCompress: 是否压缩发送0不压缩1压缩
  				msgType:命令类型
  				msgCode:命令代码
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
  去除压缩发送时压缩完才发送的限制，2005-11-23
  方向单位是2度，李松峰，2005-12-14
  增加加速度和行驶距离数值，李松峰，2005-12-14
  (char)atoi(gps->sAngle/2改为(char)(atoi(gps->sAngle)/2)，李松峰，2005-12-17
  海拔的值超大时归0，李松峰，2005-12-28
****************************************************************/
MsgShort MsgAlarmGps(gps_data *gps, MsgUChar msgType, MsgUChar msgCode, MsgUChar **out, MsgInt *outlen)
{
	MsgUChar* data = NULL;
	MsgInt len, p = 0;
	char alarmBuf[24]={0};
	unsigned short Len_alarmBuf = 0;
	unsigned short alarmlen = 0;
	unsigned long alarmstate = 0;

	if (gps == NULL || out == NULL || outlen == NULL)
	{
		SCI_TRACE_LOW( "MsgReportGps: PARA ERR, TYPE: 0x%02x, CODE: 0x%02x\r\n", msgType, msgCode);
		return MSG_FALSE;
	}

	if(msgType != MSG_TYPE_SAFETY || msgCode != MSG_SAFETY_ANS_ALARM){
		return MSG_FALSE;
	}

	len = MSG_GPS_LEN +2;

	if(g_state_info.alarmState & GPS_CAR_STATU_REGION_IN){
		alarmstate = GPS_CAR_STATU_REGION_IN;
		SCI_MEMCPY(&alarmBuf[alarmlen],(char*)&alarmstate,4);
		alarmBuf[4+alarmlen] = 0x01;
		alarmBuf[5+alarmlen] = g_state_info.loginID;
		alarmlen +=6;
	}
	if(g_state_info.alarmState & GPS_CAR_STATU_REGION_OUT){
		alarmstate = GPS_CAR_STATU_REGION_OUT;
		SCI_MEMCPY(&alarmBuf[alarmlen],(char*)&alarmstate,4);
		alarmBuf[4+alarmlen] = 0x01;
		alarmBuf[5+alarmlen] = g_state_info.logoutID;
		alarmlen +=6;
	}
	if(g_state_info.alarmState & GPS_CAR_STATU_PATH_ALRAM){
		alarmstate = GPS_CAR_STATU_PATH_ALRAM;
		SCI_MEMCPY(&alarmBuf[alarmlen],(char*)&alarmstate,4);
		alarmBuf[4+alarmlen] = 0x01;
		alarmBuf[5+alarmlen] = 0x00;
		alarmlen +=6;
	}

	len+= 2+alarmlen;

	data = SCI_ALLOC(len);
	if (data == NULL)
	{
		SCI_TRACE_LOW( "MsgReportGps: CALLOC: %d \r\n", len);
		return MSG_FALSE;
	}

	SCI_MEMSET(data,0,len);

	p =2;

	Len_alarmBuf = MSG_GPS_LEN;
	SCI_MEMCPY(data,(char*)&Len_alarmBuf,2);

	//参数内容:GPS数据包
	ParseGPS(gps,(char*)data,&p);

	if(alarmstate)
	{
		Len_alarmBuf = alarmlen;
		SCI_MEMCPY(&data[p],(char*)&Len_alarmBuf,2);
		p+=2;
		SCI_MEMCPY(&data[p],alarmBuf,alarmlen);
		p+=alarmlen;
	}

	if (MsgCreatMsg(msgType, msgCode, data, len, out, outlen) == MSG_FALSE)
	{
		if (data)
			SCI_FREE(data);
		return MSG_FALSE;
	}
	else
	{
		if (data)
			SCI_FREE(data);
		return MSG_TRUE;
	}
}


/****************************************************************
  函数名  ：MsgSafetyCancle
  功  能  ：报警后定位信息发送取消
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSafetyCancle(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_CANCLE, NULL, 0, out, outlen);
}

/****************************************************************
  函数名  ：MsgSafetyOilLock
  功  能  ：断/开油电路闭/开锁
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSafetyOilLock(MsgUChar id[2], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_OIL, id, 2, out, outlen);
}
#ifdef _DUART
/****************************************************************
  函数名  ：MsgSafetyProLock
  功  能  ：重工锁车应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSafetyProLock(MsgUChar id[2], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_LOCK, id, 2, out, outlen);
}
#endif


/****************************************************************
  函数名  ：MsgSetSafety
  功  能  ：安全中心应答
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSetSafety(MsgUChar id[2], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_SAFETY, id, 2, out, outlen);
}


/****************************************************************
  函数名  ：MsgMusterParticular
  功  能  ：调度详细信息
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgMusterParticular(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_MUSTER, MSG_MUSTER_ANS_PARTICULAR, NULL, 0, out, outlen);
}


/****************************************************************
  函数名  ：MsgOneGpsACK
  功  能  ：调度详细信息
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgOneGpsACK(MsgUChar id[1], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_MUSTER, MSG_ONE_GPS_ACK, id, 1, out, outlen);
}

/****************************************************************
  函数名  ：MsgAdvertisingPlayACK
  功  能  ：调度详细信息
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgAdvertisingPlayACK(MsgUChar id[1], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_MUSTER, MSG_ADVERTISING_PLAY_ACK, id, 1, out, outlen);
}
/****************************************************************
  函数名  ：MsgOneGPS
  功  能  ：调度详细信息
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：李松峰
  修改记录：创建，2005-10-21
****************************************************************/
MsgShort MsgSmsPlayACK(MsgUChar id[1], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_MUSTER, MSG_SMS_PLAY_ACK, id, 1, out, outlen);
}

/****************************************************************
  函数名  MsgBusDevReport
  功  能  ：外部设备报告
  输入参数：
  输出参数：
  				out:输出报文
  				outlen:输出长度
  				MSG_TRUE :成功
  				MSG_FALSE:失败
  编写者  ：陈海华
  修改记录：创建，2006-7-5
****************************************************************/
MsgShort MsgBusDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_EXT_DEV, MSG_EXT_DEV_REPORT, msg, len, out, outlen);

}


