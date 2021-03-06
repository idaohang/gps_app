#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "os_api.h"
#include "app_tcp_if.h"
#include "dal_time.h"
#include "msg.h"
#include "SG_Set.h"
#include "arm_reg.h"
#include "SG_GPS.h"
#include "tb_hal.h"
#include "SG_MsgHandle.h"
#include "SG_Camera.h"
#include "SG_DynamicMenu.h"


gps_data GpsCurrent[GPS_DATA_LEN];
gps_data GpsFull[GPS_DATA_LEN];
gps_data GpsReport[GPS_DATA_LEN];
gps_data GpsOverPlace[GPS_DATA_LEN];
gps_data GpsRedeem[GPS_DATA_LEN];



SG_INFO_MUSTER	*gp_info_interest_piont 	= NULL;	//兴趣点点链表(1个内容n点)
SG_INFO_LIST 	*gp_info_list 				= NULL; //兴趣点内容链表
SG_INFO_LINE	*gp_info_line 				= NULL;	//路线链表
SG_INFO_POINT	*gp_info_point 				= NULL;	//兴趣点
SG_INFO_REGION	*gp_info_region 			= NULL;	//区域链表
static SG_INFO_LINE 	*last_line 			= NULL;	//最后一次所在路线
static SG_INFO_REGION   *lastlogin 			= NULL;	//最后一次所在区域
static SG_INFO_MUSTER 	*plast              = NULL;

int alarm_timer ;
int bNeed = 0; //0:不需要报警1:通过报警队列报警2:通过监控队列报警
int gps_cnt = 0;
int bfirst = 1; //开机定位
long AlarmTime = 0;

/****************************************************************
  函数名：GetTimeOut
  功  能  ：计算获取超时时间(秒)
  输入参数：tn:设定的超时时间(秒)
  输出参数：超时到达的时间(秒)
  编写者  ：李松峰
  修改记录：创建，2004-12-13
****************************************************************/
uint32 GetTimeOut (uint32 tn)
{
	uint32 t;
	t = GPS_Timer;
	return t+tn;
}

/****************************************************************
  函数名：CheckTimeOut
  功  能  ：检测当前时间是否超时
  输入参数：tn:超时到达的时间(秒)
  输出参数：1:已经超时
  				0:未超时
  编写者  ：李松峰
  修改记录：创建，2004-12-13
****************************************************************/
uint32 CheckTimeOut (uint32 tn)
{
	uint32 t;
	t = GPS_Timer;
	if (t >= tn)
		return 1;
	else
		return 0;
}

//根据三角形三个边长计算一边对应的角度
//c为角度对应的边
double CalcTriangleAngle(double a, double b, double c)
{
	//i = arccos((a*a+b*b-c*c)/(2*a*b));
  	//Result=(i/pi)*180;

	double i;
	double result;

	i = acos((a*a + b*b -c*c) / (2*a*b));
	result = (i / PAI) *180;
	
	return result;
}
/***************************************************************
计算当前点的经纬度范围(300米)
****************************************************************/
int  SG_GPS_Line_Scope(int  lat,int lng,int *xmin,int *xmax,int *ymin,int *ymax) 
{
#define S (6000*180/(PAI*SG_EARTH_R))
#define SG_OVERPLACE 320
	double x1, y1, x0, y0;
	
	
	x1 = (double)lat;
	y1 =(double)lng;

	//以在中国为例
	//计算纬度范围(xmin~xmax)
	x0=SG_OVERPLACE*S;
	*xmin=(x1-x0);
	*xmax=(x1+x0);
	//计算经度范围(ymin~ymax)
	x1 = (x1*PAI)/1080000;
	y0=SG_OVERPLACE*S/(cos(x1));
	*ymin=y1-y0;
	*ymax=y1+y0;
	return 1;
}


/*************************************************************
插入链表
***************************************************************/
void insert_tab_line(SG_INFO_LINE * item)
{

	SG_INFO_LINE *p1=NULL,*p0=NULL,*p2=NULL;

	p1=gp_info_line;
	p0=item;
	
	if(gp_info_line==NULL)
	{	
		gp_info_line=p0;
		p0->next=NULL;
	}
	else
	{
		while((p0->nLat_min>p1->nLat_min)&&(p1->next!=NULL))
		{
			p2=p1;
			p1=p1->next;
		}
		if(p0->nLat_min<=p1->nLat_min)
		{
			if(gp_info_line==p1)
				gp_info_line=p0;
			else
				p2->next=p0;
			p0->next=p1;
		}
		else
		{
			p1->next=p0;
			p0->next=NULL;
		}
	}

}


/************************************************************
//更新链表
****************************************************************/
int	renew_line_tab(unsigned char *para,int len)
{
	short num=0;
	int i;
	int lat0=0,long0=0,lat1=0,long1=0;
	char slong[4]={0};
	char slat[4]={0};
	int x0min=0,x0max=0,y0min=0,y0max=0,x1min=0,x1max=0,y1min=0,y1max=0;
	SG_INFO_LINE  *item=NULL,*p0=NULL,*lastitem=NULL;
	int  lineID=0;
	int  line_speed=0,line_time=0;
	int point =0;
#if 1
{
	SCI_TRACE_LOW("@_@ renew_line_tab: \n");

}	
#endif
//	SG_Set_Lock();

	last_line = NULL;

	if(gp_info_line)
	{	
		while(gp_info_line)
		{
			p0= gp_info_line->next;
			SCI_FREE(gp_info_line);
			gp_info_line=p0;
		}
		gp_info_line=NULL;
	}
#if 0	
	for(i =0; i<len;i++)
		SCI_TRACE_LOW("@_@ %02x ",para[i]);
	SCI_TRACE_LOW("@_@ \n");
#endif
	if (para == NULL || len < 3)
	{
		SCI_TRACE_LOW("@_@ renew_line_tab PARA ERR");
		
		SG_Set_Cal_CheckSum();
		g_set_info.nLinePiont = 0;
		SG_Set_Save();
		MAX_LIST_NUM = g_set_info.EfsMax-g_set_info.nRegionPiont-g_set_info.nLinePiont;
		if(MAX_LIST_NUM >g_set_info.EfsMax)
			MAX_LIST_NUM = g_set_info.EfsMax;
		else if(MAX_LIST_NUM<30)
			MAX_LIST_NUM = 30;
			
		return 1;
	}

	while(len>0)
	{	
		if(len >= 3){
			lineID=para[0];
			SCI_MEMCPY((char*)&num, para+1, 2);
		}
		else
			return 1;
//		SCI_TRACE_LOW("@_@ num %d \n",num);
		point += num;

		if(g_set_info.bnewSpeedLine == 1){
			len -= num*8+5;
			if(len > 0){
				line_speed=para[num*8+3];
				line_time=para[num*8+4];
			}
		}
		else
			len -= num*8+3;

		if(len < 0)
			return 1;
//		SCI_TRACE_LOW("@_@ len %d num %d\n",len,num);
	
		lastitem=NULL;
		for(i=0;i<num-1;i++)
		{	
			item =(SG_INFO_LINE *) SCI_ALLOC(sizeof(SG_INFO_LINE));
			if(item){
				SCI_MEMSET(item,0,sizeof(SG_INFO_LINE));
			}

	//		SCI_TRACE_LOW("@_@ i %d num %d %p\n",i,num,item);

			if (i == 0)
			{	
				SCI_MEMCPY(slong, para+1+2+i*8, 4);
				long0=slong[0]*6000+slong[1]*100+slong[2];
				SCI_MEMCPY(slat, para+1+2+i*8+4, 4);						
				lat0= slat[0]*6000+slat[1]*100+slat[2];
				SG_GPS_Line_Scope(lat0,long0,&x0min,&x0max,&y0min,&y0max);
			}
			SCI_MEMCPY(slong, para+1+2+(i+1)*8, 4);					
			long1=slong[0]*6000+slong[1]*100+slong[2];
	//		SCI_TRACE_LOW("@_@ long1 %d slong %d %d %d \n",long1,slong[0]*6000,slong[1]*100,slong[2]);
			SCI_MEMCPY(slat, para+1+2+(i+1)*8+4, 4);						
			lat1= slat[0]*6000+slat[1]*100+slat[2];
	//		SCI_TRACE_LOW("@_@ lat1 %d slat %d %d %d \n",lat1,slat[0]*6000,slat[1]*100,slat[2]);
			SG_GPS_Line_Scope(lat1,long1,&x1min,&x1max,&y1min,&y1max);
			item->front_lat=(double)lat0/6000;
			item->front_long=(double)long0/6000;
			item->back_lat=(double)lat1/6000;
			item->back_long=(double)long1/6000;
			item->nLat_min=x0min>=x1min? x1min:x0min;
			item->nlat_max=x0max>=x1max? x0max:x1max;
			item->nLon_min=y0min>=y1min? y1min:y0min;
			item->nlon_max=y0max>=y1max? y0max:y1max;
			item->line_id=lineID;
			item->nspeed=line_speed;
			item->ntime=line_time;
		       if(lastitem)
		       {
			       lastitem->back = item;
			       item->front = lastitem;
		       }
	//		SCI_TRACE_LOW("@_@ INSERT id %d\n",item->line_id);
			insert_tab_line(item);
	//		SCI_TRACE_LOW("@_@ INSERT id %d OK\n",item->line_id);
			lastitem =item;
			lat0=lat1;
			long0=long1;
			x0min=x1min;
			x0max=x1max;
			y0min=y1min;
			y0max=y1max;
		}
		if(g_set_info.bnewSpeedLine == 1)
			para += num*8+5;
		else
			para += num*8+3;
	}
#if  1
	{	
		SG_INFO_LINE *p1=NULL;
		p1=gp_info_line;
		SCI_TRACE_LOW("@_@ gp_info_line %p\r\n",p1);
		while(p1)
		{	
//			SCI_TRACE_LOW("@_@ p1 %p\r\n",p1);
//			SCI_TRACE_LOW("@_@ line id %d ",p1->line_id);
//			SCI_TRACE_LOW("@_@ nLat_min %d ",p1->nLat_min);
//			SCI_TRACE_LOW("@_@ nlat_max %d ",p1->nlat_max);
//			SCI_TRACE_LOW("@_@ nLon_min %d ",p1->nLon_min);
//			SCI_TRACE_LOW("@_@ nlon_max %d ",p1->nlon_max);
//			SCI_TRACE_LOW("@_@ front_lat %f ",p1->front_lat);
//			SCI_TRACE_LOW("@_@ front_long %f ",p1->front_long);
//			SCI_TRACE_LOW("@_@ back_lat %f ",p1->back_lat);
//			SCI_TRACE_LOW("@_@ back_long %f ",p1->back_long);
			p1=p1->next;
		}
//		SCI_TRACE_LOW("@_@ p1 %p\r\n",p1);
	}
#endif
//	SG_Set_UnLock();
	SCI_TRACE_LOW("@_@ renew_line_tab return OK");
	SG_Set_Cal_CheckSum();
	g_set_info.nLinePiont = point;
	SG_Set_Save();

	MAX_LIST_NUM = g_set_info.EfsMax-g_set_info.nRegionPiont-g_set_info.nLinePiont;
	if(MAX_LIST_NUM >g_set_info.EfsMax)
		MAX_LIST_NUM = g_set_info.EfsMax;
	else if(MAX_LIST_NUM<30)
		MAX_LIST_NUM = 30;
		
	return 1;
}






/****************************************************************
  函数：GetGPSData
  功能：获取GPS数据，存储缓冲区
  输入：para:参数  len:长度	   
  输出：无
  返回：1:正确  0:错误		 
  编写：陈海华
  记录：创建，2007-7-16
 
****************************************************************/

int GetGPSData(void)
{
	int ch;
	
	while((ch=SIO_GetChar(COM_USER)) >= 0)
	{
		//GPS升级
		if(g_state_info.GpsDownloadFlag == 1)
		{
			HAL_DumpPutChar(COM_DEBUG, (char)ch);
		}
		else
		{
			if((WRGPSPutBuf>=GPS_PUT_BUF_SIZE)||(WRGPSPutBuf>=GPS_PUT_BUF_SIZE))
			{
				WRGPSPutBuf=0;
				RDGPSPutBuf=0;
			}

			if(ch == '$')
			{
				WRGPSPutBuf=0;
				SCI_MEMSET(GPSPutBuf,0,sizeof(GPSPutBuf));
				GPSPutBuf[WRGPSPutBuf++]=(char)ch;
			}
			else
				GPSPutBuf[WRGPSPutBuf++]=(char)ch;


			// 	长度大于9，第二个字符为'G'，最后两个字符为回车换行
			if((WRGPSPutBuf>0x09) && (GPSPutBuf[1]=='G'||GPSPutBuf[1]=='P') && (GPSPutBuf[WRGPSPutBuf-2]==0x0D) && (GPSPutBuf[WRGPSPutBuf-1]==0x0A))
			{
				
				
				GPSPutBuf[WRGPSPutBuf]='\0';
				GPSTimer=0;
	//			SCI_TRACE_LOW("@_@---%d, %s",WRGPSPutBuf,GPSPutBuf);
				return WRGPSPutBuf;
			}
		}

	}

	return 0;
}


/****************************************************************
  函数名：SG_GPS_Check
  功  能  ：判断是否是正确的GPS数据
			Input: "$............*"
			Checksum is computed from the '$' to the '*', but not including
			these two characters.  It is an 8-bit Xor of the characters
			specified, encoded in hexadecimal format.
  输入参数：type:0183报文类型
  输出参数：无
  编写者  ：李松峰
  修改记录：创建，2005-10-20
****************************************************************/
int SG_GPS_Check(char* type, char *nmea)
{
	int i;
	int sum = 0;
	int right;
	int left;
	char convert[17] = "0123456789ABCDEF";
	char checksum[3] = "";

	if (type == NULL || nmea == NULL || strncmp(type, nmea, strlen(type)) != 0)
	{
//		SCI_TRACE_LOW( "SG_GPS_Check: NOT FIND %s HEAD\r\nNMEA %s \r\n", type, nmea);
		return 0;
	}	

	for (i = 1; i < ((int)strlen(nmea) - 5); i++) 
	{
	    sum = sum ^ nmea[i];
	}

	right = sum % 16;
	left = (sum / 16) % 16;

	checksum[0] = convert[left];
	checksum[1] = convert[right];

	if (strncmp(checksum, &nmea[strlen(nmea) - 4],2) != 0)
	{
		SCI_TRACE_LOW( "SG_GPS_Check: CHECKSUM ERR, TYPE:%s\r\nNMEA %s \r\n", type, nmea);	
		return 0;
	}	
	else
		return 1;
}

/****************************************************************
  函数名：SG_GPS_Decode_RMC
  功  能  ：GPS模块RMC数据分析处理格式化
  输入参数：data:0183 RMC信息数据流
  输出参数：无
  返回值：0:失败1:成功
  编写者  ：李松峰
  修改记录：创建，2005-10-19
  	UBLOX输出长度不同，增加对不同长度数据处理
  	李松峰，2005-11-5
   	增加和GGA	匹配的处理，李松峰，2005-11-11 
****************************************************************/
int SG_GPS_Decode_RMC(char *data, gps_data *gps) 
{
	//$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>*hh
	//<1> 当前位置的格林尼治时间，格式为hhmmss.sss
	//<2> 状态, A 为有效位置, V为非有效接收警告，即当前天线视野上方的卫星个数少于3颗。
	//<3> 纬度, 格式为ddmm.mmmm
	//<4> 标明南北半球, N 为北半球、S为南半球
	//<5> 经度，格式为dddmm.mmmm 
	//<6> 标明东西半球，E为东半球、W为西半球
	//<7> 地面上的速度，范围为0.0到999.9
	//<8> 方位角，范围为000.0到 359.9 度
	//<9> 日期, 格式为ddmmyy 
	//<10> 地磁变化，从000.0到 180.0 度
	//<11> 地磁变化方向，为E 或 W
	//从GPS接收机中获得的时间信息为格林尼治时间，
	//因此需要在获取时间上加8小时才为我国标准时间。
	
	char *temp_ptr = NULL, *temp_ptr0 = NULL;
	char sOldTime[SG_GPS_TIME_LEN + 1]={0};
	int i =0;
	
	if ( (data == NULL) || (gps == NULL))
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_RMC: PARA\r\n");
		return 0;
	}	
	if (strncmp(data, NMEA_GPRMC, strlen(NMEA_GPRMC)) != 0)
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_RMC: NOT FIND \"%s\" HEAD\r\n", NMEA_GPRMC);
		return 0;
	}	

	g_state_info.nRMCCnt++;
	
	if(strlen(data) < 37)
	{
		goto myret;
	}

	if(strchr(data,',') == NULL)
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_RMC: NOT FIND ','\r\n");
		goto myret;
	}	

	//时间
	temp_ptr = strchr(data, ',');
	if (temp_ptr == NULL)
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_RMC: NOT FIND ','\r\n");
		goto myret;
	}	
		
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;

	//状态
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL)
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_RMC: NOT FIND ','\r\n");
		goto myret;
	}	
	
	temp_ptr += 1;
	
	if (temp_ptr-temp_ptr0-1 > SG_GPS_TIME_LEN)
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_RMC: TIME TOO LONG\r\n");
		goto myret;
	}	

	strcpy(sOldTime, gps->sTime);
	SCI_MEMSET(gps->sTime, 0, SG_GPS_TIME_LEN+1);
	strncpy(gps->sTime, temp_ptr0, temp_ptr-temp_ptr0-1);
	
	if (temp_ptr[0] == 'A')
	{
		gps->status = 1;
		g_state_info.nACnt++;
	}	
	else
	{
		g_state_info.nVCnt++;
		gps->status = 0;
		for(i =0;i<7 && temp_ptr;i++){
			temp_ptr = strchr(temp_ptr, ',');	
			temp_ptr +=1;
			temp_ptr0 = temp_ptr;
		}
		if(i<7 || temp_ptr == NULL || temp_ptr0 == NULL)
			goto myret;
		
		temp_ptr = strchr(temp_ptr, ',');	
		if(temp_ptr == NULL)
			goto myret;
		
		if (temp_ptr-temp_ptr0 != SG_GPS_DATE_LEN)
		{
			goto myret;
		}	

		SCI_MEMSET(gps->sDate, 0, SG_GPS_DATE_LEN+1);
		strncpy(gps->sDate, temp_ptr0, temp_ptr-temp_ptr0);

		goto myret;
//		pSg2k->gps_parse_rmc--;
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_RMC: STATUS: %c\r\n", temp_ptr[0]);				
	//	return 0;
	}	

	//纬度
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL || temp_ptr[4+1] != '.')
	{
		goto myret;
	}	

	temp_ptr += 1;
	temp_ptr0 = temp_ptr;

	//N-S
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL)
	{
		goto myret;
	}	
	
	temp_ptr += 1;
	
	if (temp_ptr-temp_ptr0-1 > SG_GPS_LAT_LEN)
	{
		goto myret;
	}	
	
	SCI_MEMSET(gps->sLatitude, 0, SG_GPS_LAT_LEN+1);
	strncpy(gps->sLatitude, temp_ptr0, temp_ptr-temp_ptr0-1);
	
	if (gps->sLatitude[8] == ',')
		gps->sLatitude[8] = '\0';
	
	gps->sSouthNorth[0] = toupper((int)temp_ptr[0]);
	gps->sSouthNorth[1] = '\0';
	
	if(gps->sSouthNorth[0] != 'N' && gps->sSouthNorth[0] != 'S')
	{
		goto myret;
	}	
	
	//经度
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL || temp_ptr[5+1] != '.')
	{
		goto myret;
	}	
	
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;

	//E-W
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL)
	{
		goto myret;
	}	
	
	temp_ptr += 1;
	
	if (temp_ptr-temp_ptr0-1 > SG_GPS_LONG_LEN)
	{
		goto myret;
	}	
	
	SCI_MEMSET(gps->sLongitude, 0, SG_GPS_LONG_LEN+1);
	strncpy(gps->sLongitude, temp_ptr0, temp_ptr-temp_ptr0-1);
	
	if (gps->sLongitude[9] == ',')
		gps->sLongitude[9] = '\0';

	gps->sEastWest[0] = toupper((int)temp_ptr[0]);
	gps->sEastWest[1] = '\0';
	
	if (gps->sEastWest[0]  != 'E' && gps->sEastWest[0]  != 'W')
	{
		goto myret;
	}	
	
	//速度
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == 0)
	{
		goto myret;
	}	
	
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;

	//角度
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL)  
	{
		goto myret;
	}	
	
	temp_ptr += 1;
	
	if (temp_ptr-temp_ptr0-1 > SG_GPS_SPEED_LEN)
	{
		goto myret;
	}	
	
	SCI_MEMSET(gps->sSpeed, 0, SG_GPS_SPEED_LEN+1);
	strncpy(gps->sSpeed, temp_ptr0, temp_ptr-temp_ptr0-1);



	temp_ptr0 = temp_ptr;

	//日期
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL)
	{
		goto myret;
	}	

	temp_ptr += 1;
	
	if (temp_ptr-temp_ptr0-1 > SG_GPS_ANGLE_LEN)
	{
		goto myret;
	}	
	
	SCI_MEMSET(gps->sAngle, 0, SG_GPS_ANGLE_LEN+1);
	strncpy(gps->sAngle, temp_ptr0, temp_ptr-temp_ptr0-1);
//
	temp_ptr0 = temp_ptr;
	
	//地磁变化
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL)
	{
		goto myret;
	}	

	temp_ptr += 1;

	if (temp_ptr-temp_ptr0-1 != SG_GPS_DATE_LEN)
	{
		goto myret;
	}	

	SCI_MEMSET(gps->sDate, 0, SG_GPS_DATE_LEN+1);
	strncpy(gps->sDate, temp_ptr0, temp_ptr-temp_ptr0-1);
	
myret:
	//前一条是GGA并且时间相等,说明是匹配的两条
	if (gps->nFull & GPS_FULL_GGA)
	{
		if(sOldTime[0]){
			if(strcmp(sOldTime, gps->sTime) == 0)
				gps->nFull |= GPS_FULL_OK;
			else
				gps->nFull  =0;
		}
	}
	else if(gps->nFull & GPS_FULL_RMC)
	{
//		pSg2k->gps_e ++;
		gps->nFull |= GPS_FULL_OK;
	}

	gps->nFull |= GPS_FULL_RMC;	

//	pSg2k->gps_parse_rmc--;
	return 1;
}







//GPS DOP 和活动卫星 
//$GPGSA、<1>、<2>、<3>、<3>、<3>、<3>、<3>、<3>、<3>、<3>、<3>、<3>、<3>、<3>、<4>、<5>、<6>、-*hh<CR><LF> 
//<1>模式M-手动，A-自动 
//<2>当前状态 1-无定位信息，2-2D 3-3D 
//<3>PRN号01~32 
//<4>位置精度 
//<5>垂直精度 
//<6>水平精度 

//当前GPS卫星状态（GSV） 
//$GPGSV、<1>、<2>、<3>、<4>、<5>、<6>、<7><4>、<5>、<6>、<7>*hh<CR><LF> 
//<1>GSV语句的总数目 
//<2>当前GSV语句数目 
//<3>显示卫星的总数目00~12 
//<4>卫星的PRV号星号 
//<5>卫星 仰角 
//<6>卫星 旋角 
//<7>信操比 
//语句共两条，第条最多包括4颗星的处所。每个星有4个数据，即<4>-星号 
//<5>-仰角
//<6>-方位
//<7>-信噪比 

//带有LORAN ID的地理信息命令 
//LCGLL 报告位置信息 
//$LCGLL、<1>、<2>、<3>、<4>，<5> <CR><LF> 
///1、 经度，ddmm .mm格式 
///2、 经度方向 N或S 
///3、 纬度，dddmm .mm格式 
///4、 纬度方向 E或W 
///5、 UTC时间（定位点）hhmmss格式 

//VTG速度相对正北的方向 
//$GPVTG、<1>、T、<2>、M、<3>、N、<4>K*hh<CR><LF> 
//<1>真实方向<2>、相对方向<3>步长<4>速率 

/****************************************************************
  函数名：SG_GPS_Decode_GGA
  功  能  ：GPS模块GGA数据分析处理格式化
  输入参数：data:0183 GGA信息数据流
  输出参数：无
  返回值：0:失败1:成功
  编写者  ：李松峰
  修改记录：创建，2005-10-19
  	UBLOX输出长度不同，增加对不同长度数据处理
  	李松峰，2005-11-5
  	增加和RMC	匹配的处理，李松峰，2005-11-11
****************************************************************/
int SG_GPS_Decode_GGA(char *data, gps_data *gps) 
{
	//位置信息（GGA） 
	//$GPGGA、<1>、<2>、<3>、<4>、<5>、<6>、<7>、<8>、<9>、<10>,    <11>、<12>*hh<CR><LF> 
	//<1>UTC时间，hh mm ss格式（定位它的） 
	//<2>纬度dd mm mmmm 格式（非0） 
	//<3>纬度方向 N或S 
	//<4>经度ddd mm mmmm 格式（非0） 
	//<5>经度方向E或W 
	//<6>GPS状态批示0-未定位 1-无差分定位信息 2-带差分定位信息 
	//<7>可见卫星数目（00~12） 
	//<8>精度百分比 
	//<9>海平面高度 
	//<10>*大地随球面相对海平面的高度 
	//<11>差分GPS信息 
	//<12>差分站ID号 0000-123 

	char *temp_ptr = NULL, *temp_ptr0 = NULL;
	char sNum[5] = "";
	char sOldTime[SG_GPS_TIME_LEN + 1]={0};

	if ( (data == NULL) || (gps == NULL))
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: PARA\r\n");
		return 0;
	}	
	if (strncmp(data, NMEA_GPGGA, strlen(NMEA_GPGGA)) != 0)
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: NOT FIND \"%s\" HEAD\r\n", NMEA_GPGGA);
		return 0;
	}	

	g_state_info.nGGACnt++;

//	pSg2k->gps_parse_gga++;

//	SCI_TRACE_LOW("@_@ %s\n",data);
	strcpy(sOldTime, gps->sTime);

	if(strlen(data) < 37)
	{
		goto myret;
	}
	
	if(strchr(data,',') == NULL)
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: NOT FIND ','\r\n");
		goto myret;
	}	
	
	//时间
	temp_ptr = strchr(data, ',');
	if (temp_ptr == NULL)
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: NOT FIND ','\r\n");
		goto myret;
	}	
		
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;
	
	//纬度
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL || temp_ptr[4+1] != '.')
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: LATTITUDE\r\n");
		goto myret;
	}	
	
	temp_ptr += 1;
	
	if (temp_ptr-temp_ptr0-1 > SG_GPS_TIME_LEN)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: TIME TOO LONG\r\n");
		goto myret;
	}	
	
	SCI_MEMSET(gps->sTime, 0, SG_GPS_TIME_LEN+1);
	strncpy(gps->sTime, temp_ptr0, temp_ptr-temp_ptr0-1);
	
	temp_ptr0 = temp_ptr;

	//N-S
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: NOT FIND ','\r\n");
		goto myret;
	}	
	
	temp_ptr += 1;
	
	if (temp_ptr-temp_ptr0-1 > SG_GPS_LAT_LEN)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: LATTITUDE TOO LONG\r\n");
		goto myret;
	}	
	
	SCI_MEMSET(gps->sLatitude, 0, SG_GPS_LAT_LEN+1);
	strncpy(gps->sLatitude, temp_ptr0, temp_ptr-temp_ptr0-1);
	
	if (gps->sLatitude[8] == ',')
		gps->sLatitude[8] = '\0';
	
	gps->sSouthNorth[0] = toupper((int)temp_ptr[0]);
	gps->sSouthNorth[1] = '\0';
	
	if(gps->sSouthNorth[0] != 'N' && gps->sSouthNorth[0] != 'S')
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: LATTITUDE NOT 'N' OR 'S'\r\n");
		goto myret;
	}	
	
	//经度
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL || temp_ptr[5+1] != '.')
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: LONGTITUDE TOO LONG\r\n");
		goto myret;
	}	
	
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;

	//E-W
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: NOT FIND ','\r\n");
		goto myret;
	}	
	
	temp_ptr += 1;
	
	if (temp_ptr-temp_ptr0-1 > SG_GPS_LONG_LEN)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: LONGTITUDE TOO LONG\r\n");
		goto myret;
	}	
	
	SCI_MEMSET(gps->sLongitude, 0, SG_GPS_LONG_LEN+1);
	strncpy(gps->sLongitude, temp_ptr0, temp_ptr-temp_ptr0-1);
	
	if (gps->sLongitude[9] == ',')
		gps->sLongitude[9] = '\0';

	gps->sEastWest[0] = toupper((int)temp_ptr[0]);
	gps->sEastWest[1] = '\0';
	
	if (gps->sEastWest[0]  != 'E' && gps->sEastWest[0]  != 'W')
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: LONGTITUDE NOT 'E' OR 'W'\r\n");
		goto myret;
	}	
	
	//状态
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: NOT FIND ','\r\n");
		goto myret;
	}	
	
	temp_ptr += 1;

	if (temp_ptr[0] == '0')
	{
//		pSg2k->gps_parse_gga--;
		gps->status = 0;
//		SCI_TRACE_LOW("@_@ SG_GPS_Decode_GGA: STATUS: %d\r\n", temp_ptr[0]);		
//		return 0;
	}	
	else
	{
		gps->status = 1;
	}	
	
	//可见卫星数目, 00 - 12
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == 0)
	{
		goto myret;
	}	

	temp_ptr += 1;
	temp_ptr0 = temp_ptr;

	//精度百分比 
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL) 
	{
		goto myret;
	}	

	temp_ptr += 1;

	if (temp_ptr-temp_ptr0-1 > 2)
	{
		goto myret;
	}	

	strncpy(sNum, temp_ptr0, temp_ptr-temp_ptr0-1);
	gps->nNum = atoi(sNum);
	
	//水平精度因子
#if (0)
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL) 
	{
		goto myret;
	}	

	temp_ptr += 1;
#endif /* (0) */
	temp_ptr0 = temp_ptr;

	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL) 
	{
		goto myret;
	}	

	temp_ptr += 1;

	if (temp_ptr-temp_ptr0-1 > 4)
	{
		goto myret;
	}	

	SCI_MEMSET(sNum,0,sizeof(sNum));
	SCI_MEMCPY(sNum, temp_ptr0, temp_ptr-temp_ptr0-1);
	gps->fHDOP = atof(sNum);

	
	//海拔高度
	temp_ptr = strchr(temp_ptr, ',');
	if (temp_ptr == NULL) 
	{
		goto myret;
	}	

	temp_ptr += 1;

	if (temp_ptr-temp_ptr0-1 > SG_GPS_ALTITUDE_LEN)
	{
		goto myret;
	}	

	SCI_MEMSET(gps->sAltitude, 0, SG_GPS_ALTITUDE_LEN+1);
	strncpy(gps->sAltitude, temp_ptr0, temp_ptr-temp_ptr0-1);

myret:
	//前一条是RMC并且时间相等,说明是匹配的两条
	if (gps->nFull & GPS_FULL_RMC)
	{
		if(sOldTime[0]){
			if(strcmp(sOldTime, gps->sTime) == 0)
				gps->nFull |= GPS_FULL_OK;
			else
				gps->nFull = 0;
		}
	}
	else if(gps->nFull & GPS_FULL_GGA)
	{
//		pSg2k->gps_e ++;
		gps->nFull |= GPS_FULL_OK;
	}

	gps->nFull |= GPS_FULL_GGA;	
//	pSg2k->gps_parse_gga--;
//	print_gps(gps);
	return 1;
}
/****************************************************************
  函数名：SG_GPS_Compare_Time
  功  能  ：比较两个时间相差几秒
  输入参数：sTime0:被比较的时间
  				sTime1:比较的时间
  输出参数：
  返回值:
  编写者  ：李松峰
  修改记录：创建，2005-10-19
  时间越过0点时计算错误，李松峰，2005-12-13
****************************************************************/
static int SG_GPS_Compare_Time(char sTime0[SG_GPS_TIME_LEN + 1], char sTime1[SG_GPS_TIME_LEN + 1]) 
{
	char hh0[3] = "", mm0[3] = "", ss0[3] = "";
	char hh1[3] = "", mm1[3] = "", ss1[3] = "";
	int t0=0, t1=0, t24;
	
	hh0[0] = sTime0[0];
	hh0[1] = sTime0[1];
	mm0[0] = sTime0[2];
	mm0[1] = sTime0[3];
	ss0[0] = sTime0[4];
	ss0[1] = sTime0[5];
	
	hh1[0] = sTime1[0];
	hh1[1] = sTime1[1];
	mm1[0] = sTime1[2];
	mm1[1] = sTime1[3];
	ss1[0] = sTime1[4];
	ss1[1] = sTime1[5];

//	待实现
	t0 = atoi(hh0)*3600 + atoi(mm0)*60 + atoi(ss0);
	t1 = atoi(hh1)*3600 + atoi(mm1)*60 + atoi(ss1);
	t24 = 24*3600 + 0*60 + 0; 
	
	//两种情况
	// 1. t1 >= t0, t0 09:00:00 t1 23:00:00 没有越过0点，相差t1-t0
	// 2. t1 < t0, t0 23:00:00 t1 01:00:00 越过了0点，相差(t24-t0) + t1
	// 3. t1 < t0, t0 23:50:17 t1 00:00:00 越过了0点，相差(t24-t0) + t1
	
	if (t1 >= t0)
		return (t1 - t0);
	else
		return ((t24 - t0) + t1);
}


/****************************************************************
  函数名：SG_GPS_Calc_Place
  功  能  ：计算两点距离
  输入参数：gps0:被比较的点
  				gps1:比较的点
  输出参数：
  返回值：距离(米)
  编写者  ：李松峰
  修改记录：创建，2005-10-19
****************************************************************/
static double SG_GPS_Calc_Place(gps_data *gps0, gps_data *gps1) 
{
	//d(x1,y1,x2,y2)=r*arccos(sin(x1)*sin(x2)+cos(x1)*cos(x2)*cos(y1-y2))
	//x1,y1是纬度\经度的弧度单位，r为地球半径
			
	char dd0[2+1] = "", mm_mmmm0[SG_GPS_LAT_LEN-2+1] = "";
	char ddd1[3+1] = "", mm_mmmm1[SG_GPS_LONG_LEN-3+1] = "";
	double x1, x2, y1, y2, d;

	if (gps0 == NULL || gps1 == NULL)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Calc_Place: PARA\r\n");
		return 0xFFFFFFFF;
	}	
		
	SCI_MEMCPY(dd0, gps0->sLatitude, 2);
	strcpy(mm_mmmm0, gps0->sLatitude+2);
	
	x1 = atof(dd0) + atof(mm_mmmm0) / 60;
	
	SCI_MEMCPY(ddd1, gps0->sLongitude, 3);
	strcpy(mm_mmmm1, gps0->sLongitude+3);
	
	y1 = atof(ddd1) + atof(mm_mmmm1) / 60;

	SCI_MEMCPY(dd0, gps1->sLatitude, 2);
	strcpy(mm_mmmm0, gps1->sLatitude+2);
	
	x2 = atof(dd0) + atof(mm_mmmm0) / 60;
	
	SCI_MEMCPY(ddd1, gps1->sLongitude, 3);
	strcpy(mm_mmmm1, gps1->sLongitude+3);
	
	y2 = atof(ddd1) + atof(mm_mmmm1) / 60;

	//角度转成弧度
	x1 = (x1*PAI)/180;
	y1 = (y1*PAI)/180;
	x2 = (x2*PAI)/180;
	y2 = (y2*PAI)/180;
	
	d = SG_EARTH_R*acos( (sin(x1)*sin(x2)+cos(x1)*cos(x2)*cos(y1-y2)) );

	return d;
}

/****************************************************************
  函数名：SG_GPS_Check_Redeem
  功  能  ：检查是否需要拐点补偿
  输入参数：gps:gps数据
  输出参数：
  返回值:
  			0:不需要补偿汇报
  			1:需要
  编写者  ：李松峰
  修改记录：创建，2005-10-19
****************************************************************/
static int SG_GPS_Check_Redeem(gps_data *gps,gps_data *gpsRedeem,int nInflex) 
{
	float nAngle0, nAngle1;
	int  nInflexionRedeem = 0;
	gps_data *gpsR = NULL;
	
	if(nInflex)
	{
		nInflexionRedeem = nInflex;
		gpsR = gpsRedeem;
	}
	else
	{
		nInflexionRedeem = g_set_info.nInflexionRedeem;
		gpsR = (gps_data*)g_state_info.pGpsRedeem;
	}
	//0x00－不采集
	if (g_set_info.nWatchType == MSG_WATCH_CANCLE)
		return 0;
	
	//拐点不补偿
	if (nInflexionRedeem == 0)
		return 0;

	//未定位信息，不检查
	if (gps == NULL || gps->status != 1)
		return 0;

	//非完整信息或RMC信息，GGA无角度，不比较
	if ((gps->nFull & GPS_FULL_RMC)== 0)
		return 0;
	
	//被比较数据非有效时不报警
	//例如第一个数据时
	if (gpsR->status == 0)
		return 0;

	/////////////////////////////////////////////////////////////
	//GPS角度不准，考虑用三个点连线角度判断
	nAngle0 = atof(gps->sAngle);
	nAngle1 = atof(gpsR->sAngle);
	
	if (abs(nAngle0 - nAngle1) < nInflexionRedeem)	
		return 0;

	SCI_TRACE_LOW("@_@ SG_GPS_Check_Redeem: ANGLE0: %f, ANGLEf: %d\r\n", nAngle0, nAngle1);

	return 1;
}

int   setGPSTime(gps_data *gps)
{
//  待填充
	//使用系统提供的函数借口
	 SCI_DATE_T  sys_date,currDate;
	 SCI_TIME_T  sys_time,curTime;
	 int tmp =0;

	 sys_date.wday = 0;
	if(gps->status == 1 && (gps->nFull & GPS_FULL_OK)){

		sscanf(gps->sTime,"%2d",&tmp);
		sys_time.hour = tmp;
		sscanf(gps->sTime+2,"%2d",&tmp);
		sys_time.min = tmp;		
		sscanf(gps->sTime+4,"%2d",&tmp);
		sys_time.sec = tmp;
		sscanf(gps->sDate,"%2d",&tmp);
		sys_date.mday = tmp;
		sscanf(gps->sDate+2,"%2d",&tmp);
		sys_date.mon = tmp;
		sscanf(gps->sDate+4,"%2d",&tmp);
		sys_date.year =tmp;

		if(/*sys_date.year > 0 && */sys_date.year < 49)
			sys_date.year += 2000;
		else
			return -1;
		
		if(sys_date.mon <1 || sys_date.mon > 12)
			return -1;
		if(sys_date.mday <1 || sys_date.mday > 31)
			return -1;
		if(/*sys_time.hour < 0 ||*/ sys_time.hour > 23)
			return -1;
		if(sys_time.min > 59 /*|| sys_time.min < 0*/)
			return -1;
		if(sys_time.sec > 59/* || sys_time.sec < 0*/)
			return -1;

		TM_GetSysDate(&currDate);
		TM_GetSysTime(&curTime);
		
		if(sys_date.year == currDate.year && sys_date.mon == currDate.mon && sys_date.mday == currDate.mday \
		&&	sys_time.hour == curTime.hour && sys_time.min == curTime.min)
			return 0;
		if(TM_SetSysTime(sys_time) == ERR_TM_NONE){
			if(TM_SetSysDate(sys_date) == ERR_TM_NONE){
				SCI_TRACE_LOW("@_@ setGPSTime: set systime success!!!");
				GPS_Timer = TM_GetTotalSeconds();
				return 0;
			}			
		}
		
		SCI_TRACE_LOW("@_@ setGPSTime: set systime fail!!!");
	}
	return -1;
}



/****************************************************************
  函数名：SG_GPS_Watch
  功  能  ：检查是否需要定时定距定次汇报
  输入参数：gps:gps数据
  输出参数：
  返回值:
  			0:不需要汇报
  			1:需要
  编写者  ：李松峰
  修改记录：创建，2005-10-19
	GPS监控时间间隔判断采用GPS定位时间加
	系统时间综合判断，李松峰，2005-11-8
	修改定距监控无汇报时已监控时间不累积的错误，
	将定距监控时间累积放在距离判断外累加，
	李松峰，2006-01-10
	将监控报文改为最近一次的有效报文, 2006-02-20
****************************************************************/
int SG_GPS_Watch(gps_data *gps) 
{
	unsigned char *msg = NULL;
	int len;
	uint32 time = 0;
	double d = 0.0;

	
	//第一次定位时主动汇报一条
	if(bfirst && (gps_cnt >= g_set_info.nGpstTime) && (gps->status == 1))
	{

		//由于系统使用rtc时钟因此仅在系统定位的时候设置一次!!!!
		if(setGPSTime(gps) == 0){
			HhTimeCorrectTimer = GetTimeOut(0);
			bfirst = 0;
			SCI_TRACE_LOW("@_@ SG_GPS_Watch: first locate,and update system time !!!!");
			SCI_MEMCPY(g_state_info.sMsgNo, g_set_info.sWatchMsgNo,sizeof(g_state_info.sMsgNo));
			if (MsgWatchByTime(g_state_info.pGpsFull, &msg, &len) == MSG_TRUE)
			{
				SG_Send_Put_New_Item(3, msg, len);
			}
		}
	}

	//0x00－不采集
	if (g_set_info.nWatchType == MSG_WATCH_CANCLE){
	//	SCI_TRACE_LOW("@_@ SG_GPS_Watch: MSG_WATCH_CANCLE return");
		return 0;
	}

	
	switch (g_set_info.nWatchType)
	{
	
	//0x01－定时采集
	case MSG_WATCH_TIME:
		//SCI_TRACE_LOW("@_@ SG_GPS_Watch: MSG_WATCH_TIME %d",GetTimeOut(0));
		
		//已定位信息采用GPS时间比较
		if (gps->status == 1 && strlen(((gps_data*)g_state_info.pGpsReport)->sTime) != 0)
		{
			time = SG_GPS_Compare_Time(((gps_data*)g_state_info.pGpsReport)->sTime, gps->sTime);
			//SCI_TRACE_LOW("@_@ SG_GPS_Watch: time %d %s %s",time,((gps_data*)g_state_info.pGpsReport)->sTime,gps->sTime);
		}	
		else
		//未定位信息采用系统时间比较		
		{
			//首次汇报
			if (g_watch_info.nLastReportSystemTime == 0)
			{
				g_watch_info.nLastReportSystemTime = GetTimeOut(0);
				SG_Watch_Save();
				time = 0;
			}
			else
			{
				uint32 time1 = GetTimeOut(0);
				if(time1 >= g_watch_info.nLastReportSystemTime)
					time = time1 - g_watch_info.nLastReportSystemTime;
				else{
					time = 0;
					g_watch_info.nLastReportSystemTime = time1;
					SG_Watch_Save();
				}
				SCI_TRACE_LOW("@_@ SG_GPS_Watch: time %d time1 %d last %d",time,time1,g_watch_info.nLastReportSystemTime);
			}
		}	

	
		//定时时间到
		if (time >= g_set_info.nWatchInterval/* || g_watch_info.nWatchedTime == 0*/ )
		{	
			//重新设置比较时间
			g_watch_info.nLastReportSystemTime = GetTimeOut(0);
			SG_Watch_Save();

			//累积监控时间
			g_watch_info.nWatchedTime += time;
			SG_Watch_Save();
			
			//停车不需要汇报
			if(g_set_info.bStopReport == 0)
			{
				//无信号或熄火不汇报
				if(gps->status != 1 || ( g_state_info.alarmState & (GPS_CAR_STATU_GAS_ALARM)) == 0){
					SCI_TRACE_LOW("@_@ SG_GPS_Watch:gps->status %d return",gps->status);
					return 0;
				}
				
				//判断距离，停车不汇报
				d = SG_GPS_Calc_Place( (gps_data*)g_state_info.pGpsReport, gps);
				if(d < SG_DEFAULT_STOP){
					SCI_TRACE_LOW("@_@ SG_GPS_Watch:d  %d < SG_DEFAULT_STOP%d return",d, SG_DEFAULT_STOP);
					return 0;
				}
			}
			if(gps->status == 1)                //增加这样子的判断，为了发送最近的OK 报文给中心
			{
				SCI_MEMCPY(g_state_info.pGpsReport, gps, sizeof(gps_data));	
				SCI_MEMCPY(g_state_info.pGpsRedeem, gps, sizeof(gps_data));				
			}
			SCI_TRACE_LOW("@_@ SG_GPS_Watch:interval %d  time %d nLastReportSystemTime %d ,nWatchedTime %d",g_watch_info.nLastReportSystemTime,g_watch_info.nWatchedTime);
		}
		//拐点补偿
		else if (SG_GPS_Check_Redeem(gps,NULL,0) == 1)
		{
			//停车不需要拐点补偿
			d = SG_GPS_Calc_Place( (gps_data*)g_state_info.pGpsReport, gps);
			if(d < SG_DEFAULT_STOP){
				SCI_TRACE_LOW("@_@ SG_GPS_Watch:d  %d < SG_DEFAULT_STOP%d return",d, SG_DEFAULT_STOP);
				return 0;
			}

			if(gps->status == 1)                //增加这样子的判断，为了发送最近的OK 报文给中心
				SCI_MEMCPY(g_state_info.pGpsRedeem, gps, sizeof(gps_data));		//这里是否也需要判断一下 GPS 是否有效呢澹�					
		}
		else
		{
		//	SCI_TRACE_LOW("@_@ SG_GPS_Watch: @@@@@@@@@@!!!!");
			return 0;
		}
		
		SCI_MEMCPY(g_state_info.sMsgNo, g_set_info.sWatchMsgNo,sizeof(g_state_info.sMsgNo));
				
		if (MsgWatchByTime(g_state_info.pGpsFull, &msg, &len) != MSG_TRUE)
			return 0;

		break;

	//0x03－定次采集
	case MSG_WATCH_AMOUNT:

		//已定位信息采用GPS时间比较
		if (gps->status == 1 && strlen(((gps_data*)g_state_info.pGpsReport)->sTime) != 0)
		{
			time = SG_GPS_Compare_Time(((gps_data*)g_state_info.pGpsReport)->sTime, gps->sTime);
			SCI_TRACE_LOW("@_@ SG_GPS_Watch: time %d %s %s",time,((gps_data*)g_state_info.pGpsReport)->sTime,gps->sTime);		
		}	
		else
		//未定位信息采用系统时间比较		
		{
			//首次汇报
			if (g_watch_info.nLastReportSystemTime == 0)
			{
				g_watch_info.nLastReportSystemTime = GetTimeOut(0);
				SG_Watch_Save();
				time = g_set_info.nWatchInterval;
			}
			else
			{
				uint32 time1 = GetTimeOut(0);
				if(time1 >= g_watch_info.nLastReportSystemTime)
					time = time1 - g_watch_info.nLastReportSystemTime;
				else{
					time = 0;
					g_watch_info.nLastReportSystemTime = time1;
					SG_Watch_Save();
				}
			}
		}	

		//定时时间到
		if (time >= g_set_info.nWatchInterval)
		{
			//重新设置比较时间
			g_watch_info.nLastReportSystemTime = GetTimeOut(0);
			SG_Watch_Save();
			
			//停车不需要汇报
			if(g_set_info.bStopReport == 0)
			{
				//无信号或熄火不汇报
				if(gps->status != 1 || ( g_state_info.alarmState & (GPS_CAR_STATU_GAS_ALARM)) == 0){
					SCI_TRACE_LOW("@_@ SG_GPS_Watch:gps->status %d return",gps->status);
					return 0;
				}

				//判断距离，停车不汇报
				d = SG_GPS_Calc_Place( (gps_data*)g_state_info.pGpsReport, gps);
				if(d < SG_DEFAULT_STOP){
					SCI_TRACE_LOW("@_@ SG_GPS_Watch:d  %d < SG_DEFAULT_STOP%d return",d, SG_DEFAULT_STOP);
					return 0;
				}
			}
			//累积监控次数
			g_watch_info.nWatchedTime += 1;
			SG_Watch_Save();

			if(gps->status == 1)
			{
				SCI_MEMCPY(g_state_info.pGpsReport, gps, sizeof(gps_data));	
				SCI_MEMCPY(g_state_info.pGpsRedeem, gps, sizeof(gps_data));							
			}
		}
		//拐点补偿
		else if (SG_GPS_Check_Redeem(gps,NULL,0) == 1)
		{
			//无信号或熄火不汇报
			if(gps->status != 1/* || ( g_state_info.alarmState & (GPS_CAR_STATU_GAS_ALARM|GPS_CAR_STATU_MOVE_ALARM)) == 0*/){
				SCI_TRACE_LOW("@_@ SG_GPS_Watch:gps->status %d return",gps->status);
				return 0;
			}
			//停车不需要拐点补偿
			d = SG_GPS_Calc_Place( (gps_data*)g_state_info.pGpsReport, gps);
			if(d < SG_DEFAULT_STOP){
				SCI_TRACE_LOW("@_@ SG_GPS_Watch:d  %d < SG_DEFAULT_STOP%d return",d, SG_DEFAULT_STOP);
				return 0;
			}

			if(gps->status == 1)
				SCI_MEMCPY(g_state_info.pGpsRedeem, gps, sizeof(gps_data));							
		}
		else
		{
			return 0;
		}

		SCI_MEMCPY(g_state_info.sMsgNo, g_set_info.sWatchMsgNo,sizeof(g_state_info.sMsgNo));

		if (MsgWatchByAmount(g_state_info.pGpsFull, &msg, &len) != MSG_TRUE)
			return 0;
		
		break;

	default:
		return 0;
		break;
	}

		
	//发送报文到发送队列
	SG_Send_Put_New_Item(3, msg, len);
	
	//监控时间到，设置未监控取消状态
	if ((g_set_info.nWatchTime != 0) && (g_watch_info.nWatchedTime >= g_set_info.nWatchTime/* - g_set_info.nWatchInterval*/))
	{
		SG_Set_Cal_CheckSum();

		g_set_info.nWatchType = MSG_WATCH_CANCLE;
		g_set_info.nWatchTime = 0;
		g_watch_info.nWatchedTime = 0;	
		g_watch_info.nLastReportSystemTime = 0;	
		SG_Watch_Save();
		SG_Set_Save();
		
		SCI_TRACE_LOW("@_@ SG_GPS_Watch: WATCH TIME OUT, CANCLE!!!\r\n");	
	}

	return 1;
}
// 判断点 pt 在有向线段 p1p2 的哪一侧)
// 返回值  1 左侧、0 在直线上、-1 右侧
static int SideOfLine(POINT p1, POINT p2, POINT pt)
{
    enum POS {RR =-1, ON = 0, LL = 1}; // 右侧、线上、左侧 

    // 叉积的两个子项
    double c1 = (p2.x - pt.x) * (pt.y - p1.y);
    double c2 = (p2.y - pt.y) * (pt.x - p1.x);

    return c1 > c2 ? LL : (c1 < c2 ? RR : ON);
}

// 判断给定点 pt 是否在多边形 poly 内
// 返回 0 在内部，-1 在外面
// 返回 > 0 表示点在第几条有向线段上 
int PtInPoly(POINT pt, POLY *poly)
{
    int i;
    int status, lastStauts;
    int cnt = 0;
    int pos, temp;

    lastStauts = (poly->acmes[0].y > pt.y) ? 1
        : ((poly->acmes[0].y == pt.y) ? 0 : -1);

    for(i=1; i<=poly->count; ++i)
    {
        status = (poly->acmes[i].y > pt.y) ? 1
            : ((poly->acmes[i].y < pt.y) ? -1 : 0);
        temp = status - lastStauts;
        lastStauts = status;

        pos = SideOfLine(poly->acmes[i-1], poly->acmes[i], pt);
        
        // 点在有向线段上
        if(pos == 0
            && ((poly->acmes[i-1].x <= pt.x &&  pt.x <= poly->acmes[i].x)
                || (poly->acmes[i-1].x >= pt.x && pt.x >= poly->acmes[i].x))
            && ((poly->acmes[i-1].y <= pt.y &&  pt.y <= poly->acmes[i].y)
                || (poly->acmes[i-1].y >= pt.y && pt.y >= poly->acmes[i].y))
          )
            return i;

        // 跨越
        if((temp > 0 && pos == 1) || (temp < 0 && pos == -1))
                cnt += temp;
    }

    return cnt == 0 ? -1 : 0;
}

//时间段比较
int SG_Get_TimeStamp_Valid(gps_data* gps,char *sdate){
	int ret = 0,rd=-1;
	char sDate[SG_GPS_DATE_LEN+1] = {0};

	if(gps == NULL || sdate == NULL){
	//	SCI_TRACE_LOW("@_@ SG_Get_TimeStamp_Valid: para err\n");
		return 1;
	}
//	SCI_MEMCPY(sDate,sdate,24);	
//	SCI_TRACE_LOW("@_@ date %s,time %s, dst %s\n",gps->sDate,gps->sTime,sDate);
//	SCI_MEMSET(sDate,0,sizeof(sDate));
	// 如果年月日均为0，则表示每日循环，仅比较时间部分
	if(memcmp(&sDate[0],&sdate[0],6)){
		sDate[0] = gps->sDate[4];
		sDate[1] = gps->sDate[5];
		sDate[2] = gps->sDate[2];
		sDate[3] = gps->sDate[3];
		sDate[4] = gps->sDate[0];
		sDate[5] = gps->sDate[1];

		ret = memcmp(&sDate[0],&sdate[0],6);//比较起始日期
		if(ret < 0){
//			SCI_TRACE_LOW("@_@ SG_Get_TimeStamp_Valid: early ,0-6@ %d %s\n",ret,sDate);
			return -1;
		}
		else if(ret == 0){
			rd = 1; // == start
		}
		if(ret >= 0){
			ret = memcmp(&sDate[0],&sdate[12],6);//结束日期
			if(ret > 0){
//				SCI_TRACE_LOW("@_@ SG_Get_TimeStamp_Valid: late,12@ %d %s\n",ret,sDate);
				return 1;
			}
			else if(ret == 0){
				if(rd == 1)
					rd = 0; // == start==end
				else
					rd = 2; // ==end
			}
		}	
	}
	ret = memcmp(&gps->sTime[0],&sdate[6],6);//起始时间
	if(ret < 0){
		if(rd == 0 || rd == 1){
//			SCI_TRACE_LOW("@_@ SG_Get_TimeStamp_Valid: early ,6@ %d %s\n",ret,sDate);
			return -1;
		}
	}
	if(ret >= 0){
		if(memcmp(&gps->sTime[0],&sdate[18],6) > 0)//结束时间
		{
			if(rd == 2 || rd ==0){
//				SCI_TRACE_LOW("@_@ SG_Get_TimeStamp_Valid: late,18@ %d %s\n",ret,sDate);
				return 1;
			}
		}
	}
	return 0;

}

static int SG_GPS_Check_Pt_In_Region(gps_data *gps)
{
	POLY poly;
	POINT pt;	
	int nlat0=0,nlong0=0;
	double lat0,long0;
	char dd0[2+1] = "", mm_mmmm0[SG_GPS_LAT_LEN-2+1] = "";
	char ddd1[3+1] = "", mm_mmmm1[SG_GPS_LONG_LEN-3+1] = "";
	int i, result;
	int myID = -1, regionType = 0;
	unsigned long alarmst;
	int  tstamp=0,tmstamp = 1;
	SG_INFO_REGION  *p0=NULL;
	SG_INFO_POINT  *p1=NULL,*p2=NULL;
	static int regionfilter;    //用来过滤漂移导致的区域出入
	static int out  = 0;
//	int menuid = MENUID_SINGLE;
//	static char buf[128] ={0}; 
//	int currID = -1;
//	int bOutRegionAlarm = 0;
	//区域号（1）+ 区域性质（1）+ 区域轨迹点数目（2）+ 经度1（4）+纬度1（4）
	//+ 经度2（4）+ 纬度2（4）+…+ 经度N（4）+ 纬度N（4）	
	//区域性质：0：离开区域；1：进入区域
	
	if (gp_info_region== NULL || gps == NULL ||gps->status == 0)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Check_Pt_In_Region: PARA\r\n");
		return -1;
	}	
//	SCI_TRACE_LOW("@_@ # SG_GPS_Check_Pt_In_Region: %s %s\n",gps->sDate,gps->sTime);
	//当前点经纬度
	SCI_MEMCPY(dd0, gps->sLatitude, 2);
	strcpy(mm_mmmm0, gps->sLatitude+2);
	lat0 = atof(dd0) + atof(mm_mmmm0) / 60;
	nlat0=lat0*6000;
	SCI_MEMCPY(ddd1, gps->sLongitude, 3);
	strcpy(mm_mmmm1, gps->sLongitude+3);
	long0 = atof(ddd1) + atof(mm_mmmm1) / 60;
	nlong0=long0*6000;

	pt.x = long0;
	pt.y = lat0;
//	SG_Set_Lock();	

	if(g_state_info.loginID >= 0 && lastlogin)
	{
		if(lastlogin->myID == g_state_info.loginID)
		{
			if(g_set_info.bnewRegion)
			{
				// get time stamp and compare
				// tstamp =0 在时间段内<0比起始时间早>0比结束时间晚
				tstamp = SG_Get_TimeStamp_Valid(gps,lastlogin->msg);
				if( tstamp)
				{
					out = 0;
					SCI_TRACE_LOW("@_@ SG_GPS_Check_Pt_In_Region: 0 before ID %d, type %d, tstamp %d, E %lx,RE %lx S %lx,login %d out %d\r\n",lastlogin->myID,lastlogin->type,tstamp,g_set_info.alarmEnable,g_set_info.ReserveAlarmEnable,g_state_info.alarmState,g_state_info.loginID,g_state_info.logoutID);
					SG_Set_Cal_CheckSum();

					g_set_info.alarmEnable = g_set_info.ReserveAlarmEnable;

					g_state_info.alarmState &= g_set_info.ReserveAlarmEnable|GPS_CAR_STATU_POS|GPS_CAR_STATU_GAS_ALARM;

					if(lastlogin->gsmOperate& 0xe0){//带gsm模块操作
						;
					}
					SG_Set_Save();

					g_state_info.loginID = -1;
					lastlogin = NULL;
	
		//			SCI_TRACE_LOW("@_@ SG_GPS_Check_Pt_In_Region: 0 after ID %d, type %d, tstamp %d, E %lx,RE %lx S %lx,login %d out %d\r\n",lastlogin->myID,lastlogin->type,tstamp,g_set_info.alarmEnable,g_set_info.ReserveAlarmEnable,g_state_info.alarmState,g_state_info.loginID,g_state_info.logoutID);
		//			SCI_TRACE_LOW("@_@ 0 SG_GPS_Check_Pt_In_Region: tstamp %d date %s\n",tstamp,lastlogin->msg);
		//			SG_Set_UnLock();
					return 0; 
				}
			}

			poly.count = lastlogin->num;
			myID=lastlogin->myID;
			regionType=lastlogin->type;
			poly.acmes = (POINT*)SCI_ALLOC((sizeof(POINT) * (poly.count+1)));
			if(poly.acmes){
				SCI_MEMSET(poly.acmes,0,(sizeof(POINT) * (poly.count+1)));
				p2=lastlogin->point;
				for(i=0;i<lastlogin->num;i++)
				{
					poly.acmes[i].x=p2->flong;
					poly.acmes[i].y=p2->flat;
					p2=p2->next;
				}
				poly.acmes[i].x=lastlogin->point->flong;
				poly.acmes[i].y=lastlogin->point->flat;
				result = PtInPoly(pt, &poly);
				SCI_FREE(poly.acmes);
			}
	//		else
	//			return 0;
			
			if(result && (regionfilter++ >= 3))// 返回 0 在内部，-1 在外面 > 0 表示点在第几条有向线段上 ,
			                                                 //出区域达到 3 次才认为正常出区域
			{
				g_state_info.loginID = -1;
				
				if(g_set_info.bnewRegion > 0)
				{
					SCI_TRACE_LOW("@_@ SG_GPS_Check_Pt_In_Region: 1 before ID %d, type %d, tstamp %d, E %lx,RE %lx S %lx,login %d out %d\r\n",lastlogin->myID,lastlogin->type,tstamp,g_set_info.alarmEnable,g_set_info.ReserveAlarmEnable,g_state_info.alarmState,g_state_info.loginID,g_state_info.logoutID);
					SG_Set_Cal_CheckSum();
					g_set_info.alarmEnable = g_set_info.ReserveAlarmEnable;
					g_state_info.alarmState &= g_set_info.ReserveAlarmEnable |GPS_CAR_STATU_POS|GPS_CAR_STATU_GAS_ALARM;
					if(lastlogin->gsmOperate & 0xe0){//带gsm模块操作
					;//	SG_update_NetType(0);
					}
					SG_Set_Save();
			//		SCI_TRACE_LOW("@_@ SG_GPS_Check_Pt_In_Region: 1 after ID %d, type %d, tstamp %d, E %lx,RE %lx S %lx,login %d out %d\r\n",lastlogin->myID,lastlogin->type,tstamp,g_set_info.alarmEnable,g_set_info.ReserveAlarmEnable,g_state_info.alarmState,g_state_info.loginID,g_state_info.logoutID);
				}

				if(lastlogin->type == 0 || lastlogin->type == 2)  //区域性质：0：离开区域；1：进入区域 2: 出入区域
				{
					out = 1;
					g_state_info.logoutID = myID;
//					lastlogout = lastlogin;
	//				SCI_TRACE_LOW("@_@ 1 SG_GPS_Check_Pt_In_Region: outside %d \n",g_state_info.logoutID);
					lastlogin = NULL;
					return 1;
				}
				
				lastlogin = NULL;
		//		SG_Set_UnLock();
		//		SCI_TRACE_LOW("@_@ 0 SG_GPS_Check_Pt_In_Region: ret normal 0\n");
				return 0;  
			}
			else
			{
			//	SG_Set_UnLock();
//				SCI_TRACE_LOW("@_@ SG_GPS_Check_Pt_In_Region: inside %d\n",g_state_info.loginID);
				out = 0;
				if(lastlogin->type > 0){
				//	if((lastlogin->gsmOperate & 0xe0) && (gWlCurrStatus->gsmOperate != lastlogin->gsmOperate))
						//SG_update_NetType(lastlogin->gsmOperate);	
						
	//				SCI_TRACE_LOW("@_@ 2 SG_GPS_Check_Pt_In_Region: inside %d\n",g_state_info.loginID);	
					return 2;
				}
	//			SCI_TRACE_LOW("@_@ 0 SG_GPS_Check_Pt_In_Region: ret normal 1\n");	
				return 0;	
			}
		}		
	}

	p0=gp_info_region;
	while(p0)
	{
		if((p0->nLatmin<nlat0)&&(p0->nlonmax>nlat0))
		{
			if((p0->nLonmin<nlong0)&&(p0->nlonmax>nlong0))
			{
				if(g_set_info.bnewRegion)
				{
					tstamp = SG_Get_TimeStamp_Valid(gps,p0->msg);
					if(tstamp)
					{
						p0=p0->next;
						continue;
					}
				}
				
				poly.count = p0->num;
				myID=p0->myID;
				regionType=p0->type;
				// 分配 count+1 个顶点的空间
				poly.acmes = (POINT*)SCI_ALLOC((sizeof(POINT) * (poly.count+1)));
				if(poly.acmes){
					SCI_MEMSET(poly.acmes,0,(sizeof(POINT) * (poly.count+1)));
					p1=p0->point;
					for(i=0;i<p0->num;i++)
					{
						poly.acmes[i].x=p1->flong;
						poly.acmes[i].y=p1->flat;
						p1=p1->next;
					}
					poly.acmes[i].x=p0->point->flong;
					poly.acmes[i].y=p0->point->flat;
					result = PtInPoly(pt, &poly);
					SCI_FREE(poly.acmes);
				}
				
				if(result == 0)  // 返回 0 在内部，-1 在外面 > 0 表示点在第几条有向线段上 
				{
					out =0;
					regionfilter = 0;
					g_state_info.loginID = myID;
					lastlogin = p0;
						
					if(g_set_info.bnewRegion >  0)
					{
						if(p0->alarmstu & GPS_CAR_REGION_ALARM_NEED)
						{
							alarmst = p0->alarmstu;
							alarmst &= ~GPS_CAR_STATU_POS;

						//	SCI_TRACE_LOW("@_@ SG_GPS_Check_Pt_In_Region: 2 before ID %d, type %d, tstamp %d, E %lx,RE %lx S %lx,login %d out %d\r\n",p0->myID,p0->type,tstamp,g_set_info.alarmEnable,g_set_info.ReserveAlarmEnable,g_state_info.alarmState,g_state_info.loginID,g_state_info.logoutID);
							SG_Set_Cal_CheckSum();

							g_set_info.alarmEnable = alarmst |GPS_CAR_STATU_REGION_IN;
							g_state_info.alarmState  &= alarmst | GPS_CAR_STATU_POS |GPS_CAR_STATU_GAS_ALARM ;
							SG_Set_Save();
						//	SCI_TRACE_LOW("@_@ SG_GPS_Check_Pt_In_Region: 2 after ID %d, type %d, tstamp %d, E %lx,RE %lx S %lx,login %d out %d\r\n",p0->myID,p0->type,tstamp,g_set_info.alarmEnable,g_set_info.ReserveAlarmEnable,g_state_info.alarmState,g_state_info.loginID,g_state_info.logoutID);
						}						
					}
					if(regionType > 0)  //区域性质：0：离开区域；1：进入区域  2: 出入区域
					{
						g_state_info.logoutID = -1;
//						lastlogout = NULL;
					//	SG_Set_UnLock();
		//				if(lastlogin->gsmOperate & 0xe0)
		//					SG_update_NetType(lastlogin->gsmOperate);						
		//				SCI_TRACE_LOW("@_@ 2 SG_GPS_Check_Pt_In_Region: inside %d %x \n",g_state_info.loginID,lastlogin->gsmOperate);
						return 2;
					}

				//	SG_Set_UnLock();
		//			SCI_TRACE_LOW("@_@ 0 SG_GPS_Check_Pt_In_Region: normal 2\n",g_state_info.loginID);
					return 0;
				}
				else if(regionType ==0 || regionType == 2){
					out = 1;
					tmstamp = 0;
				}
			}
		}
		if(p0->type == 0 || p0->type == 2){
			tstamp = SG_Get_TimeStamp_Valid(gps,p0->msg);
	//		SCI_TRACE_LOW("@_@ @SG_GPS_Check_Pt_In_Region:%d %s \n",tstamp,p0->msg);
			if(tstamp == 0){
				tmstamp = 0;
				out = 1;
			}
		}
		p0=p0->next;
	}
	
//	SG_Set_UnLock();

//	if(gWlCurrStatus->gsmOperate & 0xe0)
//		SG_update_NetType(0);	
	if(out && (tmstamp == 0)){
//		SCI_TRACE_LOW("@_@ 1 SG_GPS_Check_Pt_In_Region: out %d t %d \n",out,tmstamp);
		return 1;
	}
//	SCI_TRACE_LOW("@_@ 0 SG_GPS_Check_Pt_In_Region: ret normal 3\n");
	return 0;
}


static int SG_GPS_Check_Over_Region(gps_data *gps) 
{
	int result;
	
	if (gps == NULL)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Region: PARA\r\n");
		return -1;
	}	
	
	if (g_set_info.bRegionEnable == 0 || g_state_info.pRegion == NULL)
	{
		return 0;
	}	

	//未定位信息，不检查
	if (gps->status != 1)
	{
		return -1;
	}		


	result = SG_GPS_Check_Pt_In_Region(g_state_info.pGpsFull);    //将入口参数改为 Full, 避免漂移导致区域判断错误
	

	return result;
}



int sg_check_Line_Cell(SG_INFO_LINE *p0,double lat0,double long0)
{	
	double long1=0, lat1=0, long2=0, lat2=0;
	double a=0, b=0, c=0,  angleC,angleB, cz;

	if(p0 == NULL || lat0 < 0 || long0 < 0){
		SCI_TRACE_LOW("@_@ sg_check_Line_Cell:PARA ERR %p,%f,%f\n",p0,lat0,long0);
		return -1;
	}

	lat1=p0->front_lat*PAI/180;
	long1=p0->front_long*PAI/180;
	lat2=p0->back_lat*PAI/180;
	long2=p0->back_long*PAI/180;
	
//	SCI_TRACE_LOW("@_@ @sg_check_Line_Cell: id%d, F lat %f, long %f B lat %f, long %f \n",p0->line_id,p0->front_lat,p0->front_long,p0->back_lat,p0->back_long);
//	SCI_TRACE_LOW("@_@ @sg_check_Line_Cell: lat1 %f, long1 %f  lat2 %f, long2 %f \n",lat1,long1,lat2,long2);
	
	//当前点到前一轨迹点的线段长
	b = SG_EARTH_R*acos( (sin(lat1)*sin(lat0)+cos(lat1)*cos(lat0)*cos(long1-long0)) );
//	SCI_TRACE_LOW("@_@ sg_check_Line_Cell: id%d,b %f\n",p0->line_id,b);
	if(b <= SG_DEFAULT_OVERLINE){
//		SCI_TRACE_LOW("@_@ sg_check_Line_Cell: id%d,b %f\n",p0->line_id,b);
		return 0;
	}
		
	//当前点到后一轨迹点的线段长
	c = SG_EARTH_R*acos( (sin(lat0)*sin(lat2)+cos(lat0)*cos(lat2)*cos(long0-long2)) );
//	SCI_TRACE_LOW("@_@ sg_check_Line_Cell: id%d,c %f\n",p0->line_id,c);
	if(c <= SG_DEFAULT_OVERLINE){
//		SCI_TRACE_LOW("@_@ sg_check_Line_Cell: id%d,c %f\n",p0->line_id,c);
		return 0;
	}

		//两个轨迹点的线段长
	a = SG_EARTH_R*acos( (sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(long1-long2)) );
//	SCI_TRACE_LOW("@_@ sg_check_Line_Cell: id%d,a %f\n",p0->line_id,a);

	//步骤2
	//判断角度是否锐角
	angleC = CalcTriangleAngle(a, b, c);
	angleB = CalcTriangleAngle(a, c, b);
	
	//步骤3
	//判断垂直距离是否越界
	cz = c * sin(angleC * (PAI/180));
//	SCI_TRACE_LOW("@_@ sg_check_Line_Cell: id%d,cz %f\n",p0->line_id,cz);
	if(angleB <= 90 && angleC <= 90){
		if (cz <= SG_DEFAULT_OVERLINE)
		{
//			SCI_TRACE_LOW("@_@ sg_check_Line_Cell: id%d,cz %f\n",p0->line_id,cz);
			return 0;
		}
		return 1;
	}
	return -1;	
}



/*****************************************************************
检查 是否偏移路线报警
*******************************************************************/
static int SG_GPS_Check_Over_Line(gps_data *gps,int*speed,int*overtm,int*line_id)  
{
	double long0=0, lat0=0;
	char dd0[2+1] = "", mm_mmmm0[SG_GPS_LAT_LEN-2+1] = "";
	char ddd1[3+1] = "", mm_mmmm1[SG_GPS_LONG_LEN-3+1] = "";
	int nlat0,nlong0;
	int result = -1;
	SG_INFO_LINE *p0=NULL;



	//未定位信息，不检查
	if (gps == NULL || gps->status != 1)
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Line: PARA ERR\n");
		return 0;
	}		

	if(g_state_info.loginID >= 0)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Line: IN REGION %d\n",g_state_info.loginID);
		return 0;
	}
	
//	SG_Set_Lock();	
	
	if (g_set_info.bLineEnable == 0 || gp_info_line== NULL)
	{
//		SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Line: disable %d %p\n",g_set_info.bLineEnable,gp_info_line);
		goto back;
	}
	
//	SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Line: gps sLongitude %s sLatitude%s\n",gps->sLongitude,gps->sLatitude);
	//当前点经纬度转成弧度
	SCI_MEMCPY(dd0, gps->sLatitude, 2);
	strcpy(mm_mmmm0, gps->sLatitude+2);
	nlat0=atof(dd0)*6000+atof(mm_mmmm0) *100;
	lat0 = (atof(dd0) + atof(mm_mmmm0) / 60) * (PAI/180);
//	SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Line: nlat0 %d lat0 %f\n",nlat0,lat0);
	SCI_MEMCPY(ddd1, gps->sLongitude, 3);
	strcpy(mm_mmmm1, gps->sLongitude+3);
	nlong0=atof(ddd1)*6000+atof(mm_mmmm1)*100;
	long0 = (atof(ddd1) + atof(mm_mmmm1) / 60) * (PAI/180);
//	SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Line: nlong0 %d long0 %f\n",nlong0,long0);

	if(last_line){
		result = sg_check_Line_Cell(last_line,lat0,long0);
		if(result == 0){
			if(nlat0 > last_line->nlat_max || nlat0 < last_line->nLat_min || nlong0 < last_line->nLon_min || nlong0 > last_line->nlon_max){
				;
/*				SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Line:(%d,%d),min(%d,%d),max(%d,%d),p1(%lf,%lf),p2(%lf,%lf)\n",\
				nlat0,nlong0,last_line->nLat_min,last_line->nlat_max,last_line->nLon_min,last_line->nlon_max,last_line->front_lat,last_line->front_long,last_line->back_lat,last_line->back_long);
				SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Line:(%s,%s),p1(%d.%d,%d.%d),p2(%d.%d,%d.%d)\n",\
				gps->sLatitude,gps->sLongitude,(int)(last_line->front_lat),((last_line->front_lat-((int)last_line->front_lat))*60),\
				(int)(last_line->front_long),((last_line->front_long-((int)last_line->front_long))*60),\
				(int)(last_line->back_lat),((last_line->back_lat-((int)last_line->back_lat))*60),\
				(int)(last_line->back_long),((last_line->back_long-((int)last_line->back_long))*60));*/
			}

			p0 = last_line;
			goto back ;
		}
		
		if(last_line->front){
			p0 = last_line->front;
			result = sg_check_Line_Cell(p0,lat0,long0);
			if(result== 0) //in
			{
				last_line = p0;
				goto back ;
			}
		}
		
		if(last_line->back){
			p0 = last_line->back;
			result = sg_check_Line_Cell(p0,lat0,long0);
			if(result == 0) //in
			{
				last_line = p0;
				goto back ;
			}
		}

		last_line = NULL;
	}

	
	p0=gp_info_line;
	while(p0)
	{	
	//	*line_id=p0->line_id;
		 if((nlat0<p0->nLat_min)||(nlat0>p0->nlat_max))
		 {
		 	p0=p0->next;
		 	continue;
		 }

		if((p0->nLon_min<=nlong0)&&(p0->nlon_max>=nlong0))
		{
			result = sg_check_Line_Cell(p0,lat0,long0) ;
			if(result== 0)
			{
				last_line = p0;
				goto back ;
			}
		}
		
		p0=p0->next;
	}
//	p0 = NULL;
	result = 1; // out
	*line_id = -1;
	
back:
	if(p0){
		*line_id = p0->line_id;
		if(g_set_info.bnewSpeedLine == 1)
		{
			*speed = p0->nspeed;
			*overtm = p0->ntime;		
		}
	}
	if(result == 0){
		SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Line: IN LINE %d\r\n",*line_id);
	}
	else if(result == 1){
		SCI_TRACE_LOW("@_@ SG_GPS_Check_Over_Line: OVER LINE %d\r\n",*line_id);
	}
//	SG_Set_UnLock();
	return result;
}


int SG_GPS_Alarm(gps_data *gps) 
{
	unsigned char *msg = NULL;
	int len;
	int menuid = MENUID_SINGLE;
	static unsigned char statusn = 0;
	static int speed = 0,overtm = 0,line_id =-1;//上一次比较的数值
	int nowSpeed =0,nowTm =0,nowId=-1;//当前获取的数值
	char buf[128] ={0}; 
	
	if (gps == NULL)
	{
		SCI_TRACE_LOW("@_@ SG_GPS_Alarm: PARA\r\n");
		return 0;
	}

	SCI_TRACE_LOW("@_@ SG_GPS_Alarm: bOverSpeedEnable %d status %d nfull%d alarmState %lx alarm enable %lx   HDOP %f\n",g_set_info.bOverSpeedEnable,gps->status,gps->nFull,g_state_info.alarmState,g_set_info.alarmEnable,gps->fHDOP);

#ifdef _TRACK
	//统计ACC开启时间
	if(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)
	{
		static int ACCCheckTime;
		if(CheckTimeOut(ACCCheckTime)) 
		{
			g_state_info.AccOnTime += 1;
			ACCCheckTime = GetTimeOut(SG_JUDGE_ACC_ON_TIME);
		}
	}
#endif




	
	//判断无定位报警
	if (gps->status != 1)
	{
		if (g_state_info.alarmState & GPS_CAR_STATU_POS)
		{
			if(statusn >= 10){
//				bNeed = 2;
				SCI_TRACE_LOW("@_@ SG_GPS_Alarm: GPS STATUS\r\n");										
				statusn = 0;
				g_state_info.alarmState &= ~GPS_CAR_STATU_POS;
			}
			else 
				statusn ++;
		}
		else
			statusn = 0;

		if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
		{
			g_state_info.GPSTime++;
			SCI_TRACE_LOW("====g_state_info.GPSTime:%d",g_state_info.GPSTime);		
			if(g_state_info.GPSTime >= 1200)
			{
				g_state_info.GPSTime = 0;
#ifdef _TRACK
				HAL_SetGPIOVal(PWREN_GPS,1);				// 设置PWR_GPS引脚高
#else
				HAL_SetGPIOVal(PWREN_GPS,0);				// 设置PWR_GPS引脚低
#endif
				Gps_PwrOff_Timeout = GetTimeOut(5);
				SCI_TRACE_LOW("====GPS OFF");		
			}
		}

		
	}
	else
	{
#ifdef _TRACK
		g_state_info.GPSTime = 0;
#endif
		if (gps_cnt >= g_set_info.nGpstTime)
		{
			statusn = 0;
			g_state_info.alarmState |= GPS_CAR_STATU_POS;
		}
	}
#if 0
	//判断位移报警
	if (SG_GPS_Check_Over_Place(gps) == 1)
	{
		if ((g_state_info.alarmState & GPS_CAR_STATU_MOVE_ALARM) == 0)
		{
			bNeed = 1;
		}
		
		g_state_info.alarmState |= GPS_CAR_STATU_MOVE_ALARM;
	}
	else if(g_state_info.alarmState & GPS_CAR_STATU_MOVE_ALARM)
	{
		bNeed = 2;
		g_state_info.alarmState &= ~GPS_CAR_STATU_MOVE_ALARM;
	}
#endif
	if((g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM) && (gps_cnt>=g_set_info.nGpstTime) && g_state_info.GpsEnable == 0)
	{
		switch(SG_GPS_Check_Over_Region(gps))
		{
	  		//出区域报警
			case 1:
				if (((g_state_info.alarmState & GPS_CAR_STATU_REGION_OUT) == 0) && (g_set_info.alarmEnable& GPS_CAR_STATU_REGION_OUT))
				{
					bNeed = 1;
				}
				SCI_TRACE_LOW("@_@ SG_GPS_Alarm:SG_GPS_Check_Over_Region return outside\n");
				g_state_info.alarmState |= GPS_CAR_STATU_REGION_OUT;
				g_state_info.alarmState &= ~GPS_CAR_STATU_REGION_IN;
				break;
	  		//入区域报警
			case 2:
				if (((g_state_info.alarmState & GPS_CAR_STATU_REGION_IN) == 0) && (g_set_info.alarmEnable& GPS_CAR_STATU_REGION_IN))
				{
					bNeed = 1;
				}
				SCI_TRACE_LOW("@_@ SG_GPS_Alarm:SG_GPS_Check_Over_Region return inside\n");
				
				g_state_info.alarmState |= GPS_CAR_STATU_REGION_IN;
				g_state_info.alarmState &= ~GPS_CAR_STATU_REGION_OUT;			
				break;

			case 0:
	//			SCI_TRACE_LOW("@_@ SG_GPS_Alarm:SG_GPS_Check_Over_Region return normal %x\n",g_state_info.alarmState);
				if(g_state_info.alarmState & (GPS_CAR_STATU_REGION_OUT|GPS_CAR_STATU_REGION_IN))
				{
					if(bNeed !=1)
						bNeed = 2;
					g_state_info.alarmState &= ~(GPS_CAR_STATU_REGION_OUT|GPS_CAR_STATU_REGION_IN);						
				}
				break;
				
			default:
				break;
		}

		
		//判断路线报警
		if (SG_GPS_Check_Over_Line(gps,&nowSpeed,&nowTm,&nowId) == 1)
		{
			if (((g_state_info.alarmState & GPS_CAR_STATU_PATH_ALRAM) == 0) && (g_set_info.alarmEnable& GPS_CAR_STATU_PATH_ALRAM))
			{
					bNeed = 1;
					g_state_info.alarmState |= GPS_CAR_STATU_PATH_ALRAM;
			}
		}
		else if(g_state_info.alarmState & GPS_CAR_STATU_PATH_ALRAM)
		{
			if(bNeed !=1)
				bNeed = 2;
			g_state_info.alarmState &= ~GPS_CAR_STATU_PATH_ALRAM;
		}
			


		//判断超速和超速持续时间
		if (gps->nFull & GPS_FULL_RMC) 
		{
			static int speedAlarm = 0;


			if(g_set_info.bOverSpeedEnable == 0)
			{
				speedAlarm = 0;
				SCI_MEMSET(g_state_info.sSpeedTime,0,sizeof(g_state_info.sSpeedTime));
			}
			
			if(g_set_info.bOverSpeedEnable == 1 && (gps->status != 0) && (g_set_info.alarmEnable & GPS_CAR_STATU_SPEED_ALARM))
			{

				if(speed != g_set_info.nOverSpeed)
					speed = g_set_info.nOverSpeed;
				if(overtm != g_set_info.nOverSpeedTime)	
					overtm = g_set_info.nOverSpeedTime;
		
				
				SCI_TRACE_LOW("@_@ SG_GPS_Alarm: speed %d  %d %lx\n",speed,atoi(gps->sSpeed),g_state_info.alarmState);

				if(speed > 0 && atoi(gps->sSpeed) > speed)
				{
					
					int tmptime;
					
					//初始记录超速开始时间
					if (strlen( g_state_info.sSpeedTime) == 0)
					{
						strcpy( g_state_info.sSpeedTime, gps->sTime);
//						SCI_TRACE_LOW("@_@ SG_GPS_Alarm:line %d speed %d,tm %d,%s\n",line_id,speed,overtm, g_state_info.sSpeedTime);
					}
					else
					{
						//超速持续时间超过设定，发送超速报警		
						if(overtm > 5)
						{
							tmptime = overtm - (overtm%5);
						}
						else
						{
							tmptime = overtm;        //持续时间小于 10 不做这样的处理
						}
							
						if (SG_GPS_Compare_Time( g_state_info.sSpeedTime, gps->sTime) >= tmptime)
						{
							if (SG_GPS_Compare_Time( g_state_info.sSpeedTime, gps->sTime) < 300) //间隔时间仅在300秒内有效
							{

								int  needAlarm = 0;
								
								if(speedAlarm == 1)
								{
									bNeed = 1;
									g_state_info.alarmState |= GPS_CAR_STATU_SPEED_ALARM;
									SCI_TRACE_LOW("@_@@@@@@@@@@ SG_GPS_Alarm: OVER SPEED,speedAlarm %d %d\r\n",tmptime,speed);
								}
								SCI_TRACE_LOW("==========overtm: %d   speedAlarm: %d",overtm,speedAlarm);
								if((overtm <= 5) || (overtm % 5) == 0)
								{
									if(speedAlarm == 0)
										needAlarm = 1;
								}
								else             //每隔持续时间* 倍数重复本地警告一次
								{
									if((speedAlarm % (overtm % 5)) == 0)
										needAlarm = 1;
								}
			
								if(needAlarm)    
								{
									sprintf(buf,"车速%d, 超过当前限速%d!",(int)(atoi(gps->sSpeed)*1.852),(int)(speed*1.852));
									SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, buf);	
		
								}
								
								speedAlarm++;

							}
								
							SCI_MEMSET(g_state_info.sSpeedTime,0,sizeof(g_state_info.sSpeedTime));
						}
					}
					
				}
				else
				{
					SCI_MEMSET(g_state_info.sSpeedTime,0,sizeof(g_state_info.sSpeedTime));
					speedAlarm = 0;
					
					if(g_state_info.alarmState & GPS_CAR_STATU_SPEED_ALARM)
					{
						if(g_set_info.alarmEnable & GPS_CAR_STATU_SPEED_ALARM)
						{
							if(bNeed !=1)
								bNeed = 2;
						}
						g_state_info.alarmState &= ~GPS_CAR_STATU_SPEED_ALARM;
					}
				}
			}
			else if(g_state_info.alarmState & GPS_CAR_STATU_SPEED_ALARM)
			{
				SCI_MEMSET(g_state_info.sSpeedTime,0,sizeof(g_state_info.sSpeedTime));
				speedAlarm = 0;
				if(g_set_info.alarmEnable & GPS_CAR_STATU_SPEED_ALARM)
				{
					if(bNeed !=1)
						bNeed = 2;
				}
				g_state_info.alarmState &= ~GPS_CAR_STATU_SPEED_ALARM;
			}
		}

		//里程统计
		if(gps->status == 1)
		{
			static gps_data lastPos = {0};
			static  int lastCheckTime = 0;
			double d;
			static int   keeptime = 0;

			if(lastPos.status != 1 || lastCheckTime == 0)
			{
				SCI_MEMCPY(&lastPos,gps,sizeof(gps_data));
				lastCheckTime = GetTimeOut(0);
				keeptime = lastCheckTime;
			}
			else if(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)
			{
				if(CheckTimeOut(lastCheckTime))      //每隔3 秒检测一次
				{
					lastCheckTime = GetTimeOut(3);
					d = SG_GPS_Calc_Place( (gps_data*)&lastPos, gps);  
					if(d >= g_set_info.nGpstDist)
					{
						static int saveD = 0;
						 //仅累计300 公里/小时以下的里程
						 keeptime = lastCheckTime-keeptime-3;
						 if(keeptime > 0){//不必要
							 if(d/keeptime < 200)  //每秒小于200 米
							 {
								g_state_info.nGetTotalDistance += d;
								g_state_info.nGetDistance[0] += d;
								g_state_info.nGetDistance[1] += d;
								SCI_TRACE_LOW("========TOTAL Distance %d, d1 %d d2 %d",g_state_info.nGetTotalDistance
									,g_state_info.nGetDistance[0],g_state_info.nGetDistance[1]);
			//					sprintf(buf,"LC:%f,%d\n",d,g_set_info.nGetTotalDistance);
			//					SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, buf);	
							}
							SCI_MEMCPY(&lastPos,gps,sizeof(gps_data));
						 }
						keeptime = lastCheckTime-3;
						// 更改为5公里保存一次
						if(g_state_info.nGetTotalDistance >= (saveD + 5000))
						{
							g_watch_info.nGetTotalDistance = g_state_info.nGetTotalDistance;
							g_watch_info.nGetDistance[0] = g_state_info.nGetDistance[0];
							g_watch_info.nGetDistance[1] = g_state_info.nGetDistance[1];
							SCI_TRACE_LOW("@_@ SG_Alarm:  save TotalDistance %d\r\n",g_watch_info.nGetTotalDistance);
							SG_Watch_Save();
							saveD = g_watch_info.nGetTotalDistance;
						}
					}
				}
			}
		}

	}
	else if(g_state_info.alarmState & GPS_CAR_STATU_SPEED_ALARM)
		g_state_info.alarmState &= ~GPS_CAR_STATU_SPEED_ALARM;

#if 1
	if((gps->status == 1) && (gps_cnt>=g_set_info.nGpstTime))
	{
		static gps_data lastPos = {0};
		double d;
		if(CheckTimeOut(g_state_info.lastCheckTime))      //每隔60 秒检测一次
		{
			int   lastTime = 0;

   		//判断连续停车时间

			if(lastPos.status != 1)
			{
				SCI_MEMCPY(&lastPos,gps,sizeof(gps_data));
			}
			else
			{
				if(g_state_info.lastCheckTime)
					lastTime = GetTimeOut(SG_JUDGE_CAR_STOP_TIME)-g_state_info.lastCheckTime;
				else
					lastTime = 0;
				
				d = SG_GPS_Calc_Place( (gps_data*)&lastPos, gps);   //为避免飘移误差，60 秒内走 200米以下认定为停车
		
				if(d >= 200)
				{
					SCI_TRACE_LOW("@_@ SG_Alarm: Check Stop is FALSE.  %d\r\n",GetTimeOut(0));
					g_state_info.stopTime = 0;
					g_state_info.drvstopTime = 0;
					g_state_info.drvTime += lastTime;
				}
				else           
				{
					//SCI_TRACE_LOW("@_@ SG_Alarm: Check Stop is TRUE.  %d\r\n",GetTimeOut(0));
					if (g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)
						g_state_info.stopTime += lastTime;
					g_state_info.drvstopTime += lastTime;
				//	SCI_TRACE_LOW("==g_state_info.drvstopTime:%d\r\n",g_state_info.drvstopTime);
				//	SCI_TRACE_LOW("==g_state_info.stopTime:%d\r\n",g_state_info.stopTime);
				}
				SCI_MEMCPY(&lastPos,gps,sizeof(gps_data));
			}

		//超时驾驶
			if(g_set_info.bDrvToutEnable && g_set_info.nDrvTout > 0 )
			{
	//			int menuid = MENUID_SINGLE;
				
				if(g_state_info.drvTime >=  g_set_info.nDrvTout)
				{
					if((g_state_info.alarmState & GPS_CAR_STAT_DRIVE_TIMEOUT) == 0)
					{
						SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "超时驾驶,请注意安全!");	
						g_state_info.alarmState |= GPS_CAR_STAT_DRIVE_TIMEOUT;

						if (g_set_info.alarmEnable & GPS_CAR_STAT_DRIVE_TIMEOUT)
							bNeed = 1;
						
						SCI_TRACE_LOW("@_@ SG_Alarm: Driver Time out.\r\n");
					}
				}			
				else if(g_state_info.alarmState & GPS_CAR_STAT_DRIVE_TIMEOUT)
				{
					if(bNeed !=1)
						bNeed = 2;
					g_state_info.alarmState &= ~GPS_CAR_STAT_DRIVE_TIMEOUT;
				}
				
				if(g_state_info.drvstopTime >= 1200)     //如果停车超过20 分钟,驾驶累积时间为0
				{
					g_state_info.drvTime = 0;
					g_state_info.drvstopTime = 0;
					g_state_info.alarmState &= ~GPS_CAR_STAT_DRIVE_TIMEOUT;
				}
			}
			else
			{
				g_state_info.alarmState &= ~GPS_CAR_STAT_DRIVE_TIMEOUT;
			}

		//判断停车超时报警
			if(g_set_info.bStopToutEnable && g_set_info.nStopTout > 0)
			{
				if(g_state_info.stopTime >= g_set_info.nStopTout)
				{
					if((g_state_info.alarmState & GPS_CAR_STATU_STOP_TIMEOUT) == 0)
					{
			//			int menuid = MENUID_SINGLE;
						SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, "您超过了规定停车时间!");		
						g_state_info.alarmState |= GPS_CAR_STATU_STOP_TIMEOUT;

						if(g_set_info.alarmEnable & GPS_CAR_STATU_STOP_TIMEOUT)
							bNeed = 1;
						
						SCI_TRACE_LOW("@_@ SG_Alarm: Stop Time out.\r\n");
					}
				}
				else if(g_state_info.alarmState & GPS_CAR_STATU_STOP_TIMEOUT)
				{
					g_state_info.alarmState &= ~GPS_CAR_STATU_STOP_TIMEOUT;
					if(bNeed !=1)	
						bNeed = 2;
				}
			}
			else
				g_state_info.alarmState &= ~GPS_CAR_STATU_STOP_TIMEOUT;

			g_state_info.lastCheckTime = GetTimeOut(SG_JUDGE_CAR_STOP_TIME);
			
		}
	}
	else
	{
		if(CheckTimeOut(g_state_info.lastCheckTime))
		{
			if(!(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM))
			{
				g_state_info.stopTime += SG_JUDGE_CAR_STOP_TIME;
				g_state_info.drvstopTime += SG_JUDGE_CAR_STOP_TIME;
			}
			
			if(g_set_info.drvstopTime >= 1200)     //如果停车超过20 分钟,驾驶累积时间为0
			{
				g_state_info.drvTime = 0;
				g_state_info.drvstopTime = 0;
				g_state_info.alarmState &= ~GPS_CAR_STAT_DRIVE_TIMEOUT;
			}
		}
		g_state_info.lastCheckTime = GetTimeOut(SG_JUDGE_CAR_STOP_TIME);
	}
#endif
	//创建报警汇报报文
	if (bNeed == 1)
	{
		bNeed = 0;
		
		if(g_set_info.nNetType == 0) // 短信模式		
			AlarmTime = GetTimeOut(g_set_info.nSmsInterval);
		else
			AlarmTime = GetTimeOut(SG_ALARM_TIME);
		SCI_TRACE_LOW("@_@@@@@@@@@@@@@@@@@ SG_GPS_Alarm: g_state_info.alarmState %lx\n",g_state_info.alarmState);
		if((g_state_info.alarmState & GPS_CAR_STATU_PATH_ALRAM) && (g_state_info.alarmState & GPS_CAR_STATU_REGION_IN) && (speed&0x01)){
			//入区域和线路报警并存，不报线路报警
			g_state_info.alarmState &= ~GPS_CAR_STATU_PATH_ALRAM;
			SCI_TRACE_LOW("@_@ SG_GPS_Alarm: region in while path alarm g_state_info.alarmState %lx\n",g_state_info.alarmState);
		}

		if (MsgSafetyAlarm(g_state_info.pGpsFull,  &msg, &len) != MSG_TRUE)
			return 0;

		SG_Send_Put_Alarm_Item(msg,len);

		if(g_set_info.nNetType == 0) // 短信模式		
		{			
			xSignalHeaderRec     *signal_ptr = NULL;
			SG_CreateSignal(SG_SEND_SMS,0, &signal_ptr); 
			SCI_TRACE_LOW("============== send gps alarm sms==============");
		}

		return 1;
	}
	else if(bNeed == 2)   //报警取消也发送一条报告报文
	{
		bNeed = 0;
		if (MsgWatchByTime((gps_data*)g_state_info.pGpsFull,  &msg, &len) != MSG_TRUE)
			return 0;

		SG_Send_Put_New_Item(1, msg, len);
	}

	//IO报警状态(处于IO报警状态时需要发送监控数据)
	//DTMF响应超时:马上重发报警/等待60秒等发送其他报文
 	if (CheckTimeOut (AlarmTime) == 1)		
	{
		if(g_state_info.alarmState &  g_set_info.alarmRepeat)
		{
			if(g_set_info.nNetType == 0) // 短信模式
				AlarmTime = GetTimeOut(g_set_info.nSmsInterval);
			else
				AlarmTime = GetTimeOut(SG_ALARM_TIME);

			//报警已经发送:发送汇报信息
			//DTMF响应超时、系统重启:重发报警
			//处于正在发送中、SMS或DTMF等待响应时以及已重发报警时不重发报警		
//			if (g_set_info.nAlarmSendType != 0 && g_set_info.nAlarmSendType != 3 && g_set_info.nAlarmSendType != 5)
//				return 0;
#if (0)
			if(gps->status != 1)
				return 0;
#endif /* (0) */
			if(/*(speed%2)&& */(g_set_info.alarmRepeat & GPS_CAR_STATU_SPEED_ALARM)&&(g_state_info.alarmState & GPS_CAR_STATU_SPEED_ALARM)){
				sprintf(buf,"车速%d, 超过当前限速%d!",(int)(atoi(gps->sSpeed)*1.852),(int)(speed*1.852));
				SG_DynMenu_Put_AutoShowBox ((unsigned char*)&menuid, buf);	
			}	
			if (MsgSafetyAlarm(g_state_info.pGpsFull,  &msg, &len) != MSG_TRUE)
				return 0;
			SG_Send_Put_Alarm_Item(msg,len);
			
			if(g_set_info.nNetType == 0) // 短信模式		
			{			
				xSignalHeaderRec     *signal_ptr = NULL;
				SG_CreateSignal(SG_SEND_SMS,0, &signal_ptr); 
				SCI_TRACE_LOW("============== send gps alarm sms==============");
			}

		}

	}
		
	return 0;
}


int SG_GPS_Init(void){

	SCI_MEMSET(GpsCurrent,0,sizeof(GpsCurrent));
	SCI_MEMSET(GpsFull,0,sizeof(GpsFull));
	SCI_MEMSET(GpsReport,0,sizeof(GpsReport));
	SCI_MEMSET(GpsOverPlace,0,sizeof(GpsOverPlace));
	SCI_MEMSET(GpsRedeem,0,sizeof(GpsRedeem));



	while (g_state_info.pGpsCurrent == NULL)
	{
		g_state_info.pGpsCurrent = GpsCurrent;
	
		if (g_state_info.pGpsCurrent == NULL)
			SCI_TRACE_LOW("@_@ SG_GPS_Init: CALLOC: %d\r\n", sizeof(gps_data));
	}
	SCI_TRACE_LOW("@_@ SG_GPS_Init: pGpsCurrent: %p ok\r\n", g_state_info.pGpsCurrent);
	
	while (g_state_info.pGpsFull == NULL)
	{
		g_state_info.pGpsFull = GpsFull;
		if (g_state_info.pGpsFull == NULL)
			SCI_TRACE_LOW("@_@ SG_GPS_Init: CALLOC: %d\r\n", sizeof(gps_data));
	}
	SCI_TRACE_LOW("@_@ SG_GPS_Init: pGpsFull: %p ok\r\n", g_state_info.pGpsFull);
	
	while (g_state_info.pGpsReport == NULL)
	{
		g_state_info.pGpsReport = GpsReport;
		if (g_state_info.pGpsReport == NULL)
			SCI_TRACE_LOW("@_@ SG_GPS_Init: CALLOC: %d\r\n", sizeof(gps_data));
	}
	SCI_TRACE_LOW("@_@ SG_GPS_Init: pGpsReport: %p ok\r\n", g_state_info.pGpsReport);
	
	while (g_state_info.pGpsOverPlace == NULL)
	{
		g_state_info.pGpsOverPlace = GpsOverPlace;
		if (g_state_info.pGpsOverPlace == NULL)
			SCI_TRACE_LOW("@_@ SG_GPS_Init: CALLOC: %d\r\n", sizeof(gps_data));
	}	
	SCI_TRACE_LOW("@_@ SG_GPS_Init: pGpsOverPlace: %p ok\r\n", g_state_info.pGpsOverPlace);
	
	while (g_state_info.pGpsRedeem == NULL)
	{
		g_state_info.pGpsRedeem = GpsRedeem;
		if (g_state_info.pGpsRedeem == NULL)
			SCI_TRACE_LOW("@_@ SG_GPS_Init: CALLOC: %d\r\n", sizeof(gps_data));
	}	
	SCI_TRACE_LOW("@_@ SG_GPS_Init: pGpsRedeem: %p ok\r\n", g_state_info.pGpsRedeem);
	
	{
		gps_data *gps= (gps_data *)g_state_info.pGpsFull;

		SCI_MEMCPY(gps->sLatitude, g_watch_info.sLatitude, SG_GPS_LAT_LEN);
		SCI_MEMCPY(gps->sLongitude, g_watch_info.sLongitude, SG_GPS_LAT_LEN);
		SCI_MEMSET(g_watch_info.sLatitude, 0, SG_GPS_LAT_LEN+1);
		SCI_MEMSET(g_watch_info.sLongitude, 0, SG_GPS_LAT_LEN+1);
		SG_Watch_Save();
	}
	return 0;
}

void SG_GPS_Main(gps_data *gps)
{

//	int len;//,i;
	gps_data *pGpsFull= (gps_data *)g_state_info.pGpsFull;

	if((gps->nFull & GPS_FULL_RMC) && gps->status == 1)
	{
		//计算加速度
		if (((gps_data*)g_state_info.pGpsCurrent)->status == 1)
			gps->nAcc = (char)((atof(gps->sSpeed) -atof(((gps_data*)g_state_info.pGpsCurrent)->sSpeed))/0.1);
		//计算行驶距离，，邋
		if (g_set_info.nInflexionRedeem > 0)
		{
			if (((gps_data*)g_state_info.pGpsRedeem)->status == 1)
				gps->nDistance = (short)SG_GPS_Calc_Place((gps_data*)g_state_info.pGpsRedeem, gps); 							
		}
		else
		{
			if (((gps_data*)g_state_info.pGpsReport)->status == 1)
				gps->nDistance = (short)SG_GPS_Calc_Place((gps_data*)g_state_info.pGpsReport, gps); 				
		}
		//过滤漂移
		if(atof(gps->sSpeed) >= 108)
		{
			gps->status = 0;
			SCI_MEMSET(gps->sSpeed,0,sizeof(gps->sSpeed));
			strcpy(gps->sSpeed,"0.0");
//			SCI_TRACE_LOW("@_@ SG_GPS_Main: gps->sSpeed :%s ,set status to 0!!!!",gps->sSpeed);
		}

	}
		//保存完整定位信息(非熄火或位移报警时保存)
		
	//私家车关闭监控时，将上传的经纬度置为0
	if(g_set_info.GPSEnable == 1)
	{
		SCI_MEMSET(gps->sLatitude, 0, SG_GPS_LAT_LEN + 1);
		SCI_MEMSET(gps->sLongitude, 0, SG_GPS_LAT_LEN + 1);
	}

	if(gps->status == 1){

		if(pGpsFull->status == 0)
		{
			gps_cnt = 1;
			pGpsFull->status = 1;
			
		}
		if((0 < gps_cnt) && (gps_cnt <= g_set_info.nGpstTime))
			gps_cnt++;

		//之前没保存过经纬度
		if((gps_cnt == g_set_info.nGpstTime) && (strlen(pGpsFull->sLatitude) == 0))
		{
			
			SCI_TRACE_LOW("==========g_watch_info.sLatitude is null");
			SCI_MEMCPY(pGpsFull, gps, sizeof(gps_data)); 
	
		}
		else if(gps_cnt == g_set_info.nGpstTime)
		{
			SCI_TRACE_LOW("==========g_watch_info.sLatitude is:%s",pGpsFull->sLatitude);
		}
		//默认ACC开才更新经纬度	
		if((gps_cnt >= g_set_info.nGpstTime) && ((g_set_info.ACCGPS == 1)||(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)))
		{
		
			SCI_MEMCPY(pGpsFull, gps, sizeof(gps_data)); 
			//SCI_TRACE_LOW("======ACC ON = ");
		}
		else
		{
			//熄火状态下仅更新时间，同时置速度为 0
			SCI_MEMCPY(pGpsFull->sTime,gps->sTime,sizeof(gps->sTime));
			SCI_MEMCPY(pGpsFull->sDate,gps->sDate,sizeof(gps->sDate));
			SCI_MEMSET(pGpsFull->sSpeed,0,sizeof(pGpsFull->sSpeed));
			strcpy(pGpsFull->sSpeed,"0.0");
			pGpsFull->nNum = gps->nNum;
		}
	}
	else
	{
	    gps_cnt = 0;
		pGpsFull->status = 0;
		SCI_MEMSET(pGpsFull->sSpeed,0,sizeof(pGpsFull->sSpeed));
		strcpy(pGpsFull->sSpeed,"0.0");
		pGpsFull->nNum = gps->nNum;
		if(bfirst == 0)
		{
			SCI_MEMCPY(pGpsFull->sTime,gps->sTime,sizeof(gps->sTime));
			SCI_MEMCPY(pGpsFull->sDate,gps->sDate,sizeof(gps->sDate));
		}

	}
	
	//保存当前GPS信息供其他模块使用
	//此数据可能不是完整有效已定位的GPS信息
	memcpy(g_state_info.pGpsCurrent, gps, sizeof(gps_data));

	SG_GPS_Alarm(gps);
#if 0
	//报警:位移/区域/路线/超速汇报
	if (SG_GPS_Alarm(gps) == 0)
	{
		//位移防盗功能时休眠在这里控制
		//熄火时休眠但是不关闭GPS电源
		if (g_set_info.bMoveDisable == 0)
		{
			if (SG_Set_Check_Suspend(0) == 1)
			{
				//停车休眠时不汇报和黑匣子保存
//				return;
			}	
		}
	}
#endif		
	
	//定时定距定次汇报
	SG_GPS_Watch(gps);
//	SCI_TRACE_LOW("@_@ SG_GPS_Watch: @@@@@@@@@@!!!!");
	SCI_MEMSET(gps, 0, sizeof(gps_data));	
#if 0	
 gps_continue:
 
	//完整信息汇报完毕
	//此信息对于下次匹配没有用处
	//清空并且从头继续获取

	//如果信息不全，则保留上次解析状态
	if ((gps->nFull & (GPS_FULL_OK|GPS_FULL_GGA|GPS_FULL_RMC)) != (GPS_FULL_OK|GPS_FULL_GGA|GPS_FULL_RMC))
	{
		gps->nFull &= ~GPS_FULL_OK;
	}
	else{
	//	SCI_TRACE_LOW("@_@ SG_GPS_Main: set gps to 0!!!!");
		SCI_MEMSET(gps, 0, sizeof(gps_data));		
	}
#endif	
	return ;
}
/****************************************************************
  函数：Insert_Interest_Piont_Msg
  功能：插入兴趣点内容链表
  输入：para
  输出：无
  返回：无
  编写：chenli
  记录：创建，2007-7-16
 
****************************************************************/
void Insert_Interest_Piont_Msg(SG_INFO_LIST * item)
{
	SG_INFO_LIST *p1=NULL,*p0=NULL;

	p1 = gp_info_list;
	p0 = item;
	
	if(gp_info_list==NULL)
	{	
		gp_info_list = p0;
		p0->next = NULL;
	}
	else
	{
		while(p1 && p1->next)
		{
			p1=p1->next;
		}
		p1->next=p0;
	}
	
}

/****************************************************************
  函数：Insert_Interest_Piont_Piont
  功能：插入兴趣点点链表
  输入：item兴趣点点链表
  输出：无
  返回：无
  编写：chenli
  记录：创建，2007-7-16
 
****************************************************************/
void Insert_Interest_Piont_Piont(SG_INFO_MUSTER * item)
{
	SG_INFO_MUSTER *p1=NULL,*p0=NULL,*p2=NULL;

	p1=gp_info_interest_piont;
	p0=item;
	
	if(gp_info_interest_piont==NULL)
	{	
		gp_info_interest_piont=p0;
		p0->next=NULL;
	}
	else
	{
		while((p0->nLatitude>p1->nLatitude)&&(p1->next!=NULL))
		{
			p2=p1;
			p1=p1->next;
		}
		if(p0->nLatitude<=p1->nLatitude)
		{
			if(gp_info_interest_piont==p1)
				gp_info_interest_piont=p0;
			else
				p2->next=p0;
			p0->next=p1;
		}
		else
		{
			p1->next=p0;
			p0->next=NULL;
		}
	}
}


/****************************************************************
  函数：Insert_Rgion_Point
  功能：每个区域节点插入一个点(n个点)
  输入：item区域里点节点
  输出：无
  返回：无
  编写：chenli
  记录：创建，2007-7-16
 
****************************************************************/
void Insert_Rgion_Point(SG_INFO_POINT * item)
{
	SG_INFO_POINT *p1=NULL,*p0=NULL;

	p1=gp_info_point;
	p0=item;
	if(gp_info_point==NULL)
	{	
		gp_info_point=p0;
		p0->next=NULL;
	}
	else
	{
		while(p1&&p1->next)
		{
			p1=p1->next;
		}
		p1->next=p0;
		p0->next = NULL;
	}
//	SCI_TRACE_LOW("--insert_tab_point gp %p,p0 :%p,p1:%p",gp_info_point,p0,p1);
	
}

/****************************************************************
  函数：Insert_Region_Item
  功能：插入一个区域节点
  输入：item区域节点
  输出：无
  返回：无
  编写：chenli
  记录：创建，2007-7-16
 
****************************************************************/
void Insert_Region_Item(SG_INFO_REGION * item)
{
	SG_INFO_REGION  *p1=NULL,*p0=NULL,*p2=NULL;

	p1=gp_info_region;
	p0=item;
	
	if(gp_info_region==NULL)
	{	
		gp_info_region=p0;
		p0->next=NULL;
	}
	else
	{
		while((p0->nLatmin>p1->nLatmin)&&(p1->next!=NULL))
		{
			p2=p1;
			p1=p1->next;
		}
		if(p0->nLatmin<=p1->nLatmin)
		{
			if(gp_info_region==p1)
				gp_info_region=p0;
			else
				p2->next=p0;
			p0->next=p1;
		}
		else
		{
			p1->next=p0;
			p0->next=NULL;
		}
	}
	
}


/****************************************************************
  函数：Renew_Interest_Piont_Tab
  功能：更新区域链表列表
  输入：para
  输出：无
  返回：无
  编写：chenli
  记录：创建，2007-7-16
 
****************************************************************/
int  Renew_Region_Tab(unsigned char *para,int len)
{	
	short num=0;
	int i=0,j=0,myID = -1, regionType = 0,gsmOperate = 0;
	int nlat,nlong;
	int minlat=0,maxlat=0,minlong=0,maxlong=0;
	char slong[5]={0};
	char slat[5]={0};
	SG_INFO_REGION *item=NULL,*p0=NULL;
	SG_INFO_POINT *item1=NULL,*p1=NULL;
	unsigned long alarmst=0;
	int point=0;
	
	g_state_info.loginID = -1;
	g_state_info.logoutID = -1;
	if(g_state_info.alarmState & GPS_CAR_STATU_REGION_IN)
		g_state_info.alarmState &= ~GPS_CAR_STATU_REGION_IN;
	if(g_state_info.alarmState & GPS_CAR_STATU_REGION_OUT)
		g_state_info.alarmState &= ~GPS_CAR_STATU_REGION_OUT;

	lastlogin = NULL;
	
	if(gp_info_region)
	{	
		while(gp_info_region)
		{
			p0= gp_info_region->next;
			while(gp_info_region->point)
			{
				p1=gp_info_region->point->next;
				SCI_FREE(gp_info_region->point);
				gp_info_region->point=p1;
				
//				SCI_TRACE_LOW("- gp %p",gp_info_region->point);
			}
			gp_info_region->point=NULL;
			SCI_FREE(gp_info_region);
			gp_info_region=p0;
//			SCI_TRACE_LOW("- gp %p",gp_info_region);
		}
		gp_info_region=NULL;
	}
	if (para == NULL || len < 4)
	{

		SG_Set_Cal_CheckSum();
		g_set_info.bRegionEnable = 0;
		if(g_set_info.bnewRegion)
			g_set_info.bnewRegion = 0;

		g_set_info.nRegionPiont = 0;

		MAX_LIST_NUM = g_set_info.EfsMax-g_set_info.nRegionPiont-g_set_info.nLinePiont;
		if(MAX_LIST_NUM > g_set_info.EfsMax)
			MAX_LIST_NUM = g_set_info.EfsMax;
		else if(MAX_LIST_NUM<30)
			MAX_LIST_NUM = 30;
		SG_Set_Save();
		return 0;
	}
	
	while(j<len)
	{	
		myID=para[j++];
		regionType=para[j] & 0x1f;
		gsmOperate = para[j++]&0xE0; 
		SCI_MEMCPY(&num,&para[j],2);

		j +=2;
		point += num;

//		SCI_TRACE_LOW("1--j=%d num=%d len=%d",j,num,len);
		
		item=(SG_INFO_REGION *)SCI_ALLOC(sizeof(SG_INFO_REGION));
		SCI_MEMSET(item,0,sizeof(SG_INFO_REGION));
		gp_info_point=NULL;
		for(i=0;i<num;i++)
		{	
			item1=(SG_INFO_POINT *)SCI_ALLOC(sizeof(SG_INFO_POINT));
			SCI_MEMSET(item1,0,sizeof(SG_INFO_POINT));
			SCI_MEMCPY(slong,&para[j+i*8],4);
			nlong=slong[0]*6000+slong[1]*100+slong[2];
			item1->flong=(double)nlong/6000;
			SCI_MEMCPY(slat,&para[j+i*8+4],4);
			nlat=slat[0]*6000+slat[1]*100+slat[2];
			item1->flat=(double)nlat/6000;
			
			Insert_Rgion_Point(item1);
			
//			SCI_TRACE_LOW("2--nlong=%d nlat=%d item1->flong=%f,item1->flat=%f",j,num,item1->flong,item1->flat);
			if(i==0){
				minlat=nlat;
				maxlat=nlat;
				minlong=nlong;
				maxlong=nlong;
			}
			else
			{
				if(minlat>nlat)
					minlat=nlat;
				else if(maxlat<nlat)
					maxlat=nlat;
					
				if(minlong>nlong)
					minlong=nlong;
				else if(maxlong<nlong)
					maxlong=nlong;
			}
			
//			SCI_TRACE_LOW("22--i=%d num=%d len=%d",i,num,len);
		}

		
//		SCI_TRACE_LOW("2222--j=%d num=%d len=%d",j,num,len);
		item->point=gp_info_point;
		item->nLatmin=minlat;
		item->nlatmax=maxlat;
		item->nLonmin=minlong;
		item->nlonmax=maxlong;
		item->num=num;
		item->myID=myID;
		item->type=regionType;
		item->gsmOperate = gsmOperate;
		if(g_set_info.bnewRegion)
		{
			SCI_MEMCPY((char *)&item->alarmstu,&para[j+num*8],4);
			item->alarmstu=(unsigned long)ntohl(item->alarmstu);
			SCI_MEMCPY(item->msg,&para[j+num*8+4],24);
		}
		
		Insert_Region_Item(item);

//		SCI_TRACE_LOW("3--=%d num=%d len=%d",j,num,len);
		j+=num*8;
		if(g_set_info.bnewRegion)
			j += 28;
//		SCI_TRACE_LOW("4--j=%d num=%d len=%d",j,num,len);
		
	}
	SG_Set_Cal_CheckSum();
	g_set_info.nRegionPiont = point;
	SG_Set_Save();
	MAX_LIST_NUM = g_set_info.EfsMax-g_set_info.nRegionPiont-g_set_info.nLinePiont;
	if(MAX_LIST_NUM >g_set_info.EfsMax)
		MAX_LIST_NUM = g_set_info.EfsMax;
	else if(MAX_LIST_NUM<30)
		MAX_LIST_NUM = 30;
	
	return 1;
}

//gps模块异常处理
void SG_Gps_Err_Handle(void)
{
	if(g_set_info.GpsPowrFlag == 0)
	{
		return;
	}

	if(g_state_info.GpsDownloadFlag == 1)
	{
		return;
	}


	if(CheckTimeOut(Gps_Rev_Timeout))
	{
		Gps_Rev_Timeout = GetTimeOut(60);
		Gps_PwrOff_Timeout = GetTimeOut(5);
		SCI_TRACE_LOW("<<<<<<GPS ERROR: POWER OFF!!!");
#ifdef _TRACK
	HAL_SetGPIOVal(PWREN_GPS,1);				// 设置PWR_GPS引脚高
#else
	HAL_SetGPIOVal(PWREN_GPS,0);				// 设置PWR_GPS引脚高
#endif
	
	}
	if(Gps_PwrOff_Timeout !=0 && CheckTimeOut(Gps_PwrOff_Timeout))
	{
		Gps_PwrOff_Timeout = 0;
#ifdef _TRACK
	HAL_SetGPIOVal(PWREN_GPS,0);				// 设置PWR_GPS引脚高
#else
	HAL_SetGPIOVal(PWREN_GPS,1);				// 设置PWR_GPS引脚高
#endif
		SCI_TRACE_LOW("<<<<<<GPS ERROR: POWER ON!!!");
 	}

}




