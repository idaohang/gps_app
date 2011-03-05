/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_GPS.h
  版本   ：1.50
  创建者 ：李松峰
  创建时间：2007-7-23
  内容描述：SG2000  报文发送和处理  
  修改记录：
***************************************************************/
#ifndef _SG_GPS_H_
#define _SG_GPS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "sci_types.h"
#include "SG_Set.h"
#define PAI 3.14159265358


//NMEA信息类型

#define NMEA_GPRMC "$GPRMC"
#define NMEA_GPGGA "$GPGGA"
#define NMEA_GPRMC_FITER "RMC"
#define NMEA_GPGGA_FITER "GGA"


#define   GPS_FULL_GGA    2
#define   GPS_FULL_RMC    1
#define   GPS_FULL_OK      4

#define GPS_DATA_LEN sizeof(gps_data) 

typedef struct _GPS_DATA_{
int nFull; //信息是否完整,GGA接收到(置0),RMC接收到(置1),前后0和1匹配(置2)才完整
//取自GGA或者RMC
char status; //GPS状态
char sTime[SG_GPS_TIME_LEN + 1]; // 当前位置的格林尼治时间，格式为hhmmss.ss/hhmmss.sss
char sLatitude[SG_GPS_LAT_LEN + 1]; //纬度, 格式为ddmm.mmmm/ddmm.mmmmm
char sSouthNorth[2]; //标明南北半球, N 为北半球、S为南半球
char sLongitude[SG_GPS_LONG_LEN + 1]; //径度，格式为dddmm.mmmm/dddmm.mmmmm 
char sEastWest[2]; //标明东西半球，E为东半球、W为西半球
//取自GGA
char sAltitude[SG_GPS_ALTITUDE_LEN + 1]; //海拔高度
char nNum; //可见卫星数目，范围为0到 12
//取自RMC
char sSpeed[SG_GPS_SPEED_LEN + 1]; //地面上的速度，范围为0.0到999.9
char sAngle[SG_GPS_ANGLE_LEN + 1]; //方位角，范围为000.0到 359.9 度
char sDate[SG_GPS_DATE_LEN + 1]; // 日期, 格式为ddmmyy 
//计算获得的数据
char nAcc; //单位是0.1米/秒2，有符号数
short nDistance; //指与上一次回传之间的行驶距离，单位是米
float fHDOP; // 水平精度因子

}gps_data;

// 点结构
typedef struct  
{
    double x;
    double y;
} POINT; 

// 多边形
typedef struct  
{
    // 顶点数
    int count;
    // 指向含有count+1个顶点的数组
    // 多保存一个顶点是为了方便处理环形向量
    // 最后一个点与第一个点重合
    POINT *acmes;
} POLY;


typedef struct _INFO_LINE_
{   
	int  nLat_min;   //最小纬度
	int  nLon_min;  //最小经度
	int  nlat_max;   //最大纬度
	int  nlon_max;  //最大经度
	int  line_id;       //路线号
	int  nspeed;      //线路最高速度
	int  ntime;	   //超速持续时间
	double front_lat; //前点纬度
	double front_long; //前点经度
	double back_lat;   //后点纬度
	double back_long; //后点经度
	
	struct  _INFO_LINE_  *front;
	struct  _INFO_LINE_  *back;
	struct  _INFO_LINE_  *next;
}SG_INFO_LINE;

typedef  struct _INFO_POINT_
{
	double flat;
	double flong;
	struct _INFO_POINT_ *next;
}SG_INFO_POINT;

typedef struct _INFO_REGION_
{
	int  nLatmin;   //最小纬度
	int  nLonmin;  //最小经度
	int  nlatmax;   //最大纬度
	int  nlonmax;  //最大经度
	int  num;
	int  myID;
	int  type;
	int gsmOperate; //对gsm模块的操作
	unsigned long   alarmstu;
	char  msg[30]; //时间段
	SG_INFO_POINT  *point;
	struct  _INFO_REGION_ *next;
}SG_INFO_REGION;




int SG_GPS_Watch(gps_data *gps); 
int GetGPSData(void);
int SG_GPS_Decode_RMC(char *data, gps_data *gps);
int SG_GPS_Decode_GGA(char *data, gps_data *gps);
int SG_GPS_Check(char* type, char *nmea);
void SG_GPS_Main(gps_data *gps);
int SG_GPS_Init(void);
void SG_Check_IO(xSignalHeaderRec   *receiveSignal);
int SG_GPS_Alarm(gps_data *gps) ;
int  Renew_Region_Tab(unsigned char *para,int len);
uint32 GetTimeOut (uint32 tn);
uint32 CheckTimeOut (uint32 tn);



#endif //_SG_GPS_H_

