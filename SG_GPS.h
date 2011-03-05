/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_GPS.h
  �汾   ��1.50
  ������ �����ɷ�
  ����ʱ�䣺2007-7-23
  ����������SG2000  ���ķ��ͺʹ���  
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_GPS_H_
#define _SG_GPS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "sci_types.h"
#include "SG_Set.h"
#define PAI 3.14159265358


//NMEA��Ϣ����

#define NMEA_GPRMC "$GPRMC"
#define NMEA_GPGGA "$GPGGA"
#define NMEA_GPRMC_FITER "RMC"
#define NMEA_GPGGA_FITER "GGA"


#define   GPS_FULL_GGA    2
#define   GPS_FULL_RMC    1
#define   GPS_FULL_OK      4

#define GPS_DATA_LEN sizeof(gps_data) 

typedef struct _GPS_DATA_{
int nFull; //��Ϣ�Ƿ�����,GGA���յ�(��0),RMC���յ�(��1),ǰ��0��1ƥ��(��2)������
//ȡ��GGA����RMC
char status; //GPS״̬
char sTime[SG_GPS_TIME_LEN + 1]; // ��ǰλ�õĸ�������ʱ�䣬��ʽΪhhmmss.ss/hhmmss.sss
char sLatitude[SG_GPS_LAT_LEN + 1]; //γ��, ��ʽΪddmm.mmmm/ddmm.mmmmm
char sSouthNorth[2]; //�����ϱ�����, N Ϊ������SΪ�ϰ���
char sLongitude[SG_GPS_LONG_LEN + 1]; //���ȣ���ʽΪdddmm.mmmm/dddmm.mmmmm 
char sEastWest[2]; //������������EΪ������WΪ������
//ȡ��GGA
char sAltitude[SG_GPS_ALTITUDE_LEN + 1]; //���θ߶�
char nNum; //�ɼ�������Ŀ����ΧΪ0�� 12
//ȡ��RMC
char sSpeed[SG_GPS_SPEED_LEN + 1]; //�����ϵ��ٶȣ���ΧΪ0.0��999.9
char sAngle[SG_GPS_ANGLE_LEN + 1]; //��λ�ǣ���ΧΪ000.0�� 359.9 ��
char sDate[SG_GPS_DATE_LEN + 1]; // ����, ��ʽΪddmmyy 
//�����õ�����
char nAcc; //��λ��0.1��/��2���з�����
short nDistance; //ָ����һ�λش�֮�����ʻ���룬��λ����
float fHDOP; // ˮƽ��������

}gps_data;

// ��ṹ
typedef struct  
{
    double x;
    double y;
} POINT; 

// �����
typedef struct  
{
    // ������
    int count;
    // ָ����count+1�����������
    // �ౣ��һ��������Ϊ�˷��㴦��������
    // ���һ�������һ�����غ�
    POINT *acmes;
} POLY;


typedef struct _INFO_LINE_
{   
	int  nLat_min;   //��Сγ��
	int  nLon_min;  //��С����
	int  nlat_max;   //���γ��
	int  nlon_max;  //��󾭶�
	int  line_id;       //·�ߺ�
	int  nspeed;      //��·����ٶ�
	int  ntime;	   //���ٳ���ʱ��
	double front_lat; //ǰ��γ��
	double front_long; //ǰ�㾭��
	double back_lat;   //���γ��
	double back_long; //��㾭��
	
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
	int  nLatmin;   //��Сγ��
	int  nLonmin;  //��С����
	int  nlatmax;   //���γ��
	int  nlonmax;  //��󾭶�
	int  num;
	int  myID;
	int  type;
	int gsmOperate; //��gsmģ��Ĳ���
	unsigned long   alarmstu;
	char  msg[30]; //ʱ���
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

