/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2008 ��Ȩ����
  �ļ��� ��SG_Set.h
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-7-23
  ����������SG2000 ϵͳ������Ϣ���û�����
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_SET_H_
#define _SG_SET_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "sci_types.h"
#include "mn_type.h"
#include "SG_Send.h"
#include "Dal_time.h"
#include "cmddef.h"
#include "mn_events.h"

#define DPARAM         void*
#define uint32 unsigned long
#define uint16 unsigned short
#define uint8  unsigned char
//#define MAX_UINT32 0xffffffff

//piaotq changed 20050104
#define MMI_MEMCPY(_DEST_PTR, _DEST_LEN, _SRC_PTR, _SRC_LEN, _SIZE) \
        SCI_PASSERT((_SIZE) <= (_DEST_LEN), ("MMI_MEMCPY: the _DEST_LEN is too small!")); \
        SCI_PASSERT((_SIZE) <= (_SRC_LEN),  ("MMI_MEMCPY: the _SRC_LEN is too small!")); \
        SCI_MEMCPY((_DEST_PTR), (_SRC_PTR), (_SIZE))


// ��֧����

#define __cwt_ (FALSE)

#define __camera_debug_ (FALSE)

#define __gps_debug_ (FALSE)


//�汾��֧
#define COMMON_VER 1
#define PRIVATE_CAR_VER 2

#ifdef _DUART
#define SG2000_VER "SG2KV6.10_build at "__DATE__" "__TIME__
#endif
#ifdef _SUART
#ifdef _TRACK
#define SG2000_VER "SG2KV5.11_build at "__DATE__" "__TIME__
#else
#define SG2000_VER "SG2KV4.10_build at "__DATE__" "__TIME__
#endif
#endif

#define HH_ENABLE 0x01  //�ֱ�
#define DEBUG_ENABLE 0x02  //LOG
#define DEV_ENABLE 0x04   // ��Դ���ڵ�����
#define LED2_ENABLE 0x08  //������ڴ���2



// �źŶ���
#define SG_QQZM_PICTURE			0xfc7
#define SG_SMS_YH				0xfc8
#define SG_SMS_HH               0xfc9
#define SG_CPU_DD               0xfca
#define SG_LONGMSG_HANDLE		0xfcb
#define SG_PIC_UP				0xfcc // �ϴ�ͼƬ
#define SG_SMS_ZB				0xfcd
#define SG_SMS_CHECK            0xfce
#define SG_PRO_VO               0xfcf
#define SG_COM2_BUF				0xfd0
#define SG_PRO_ACK              0xfd1
#define SG_PRO_LOCK             0xfd2
#define SG_MSG_HH               0xfd3
#define SG_CHANGE_BAUD          0xfd4
#define SG_CALL_LISTEN          0xfd5
#define SG_CAMERA_UPLOAD		0xfd6
#define SG_SEND_HH				0xfd7
#define SG_ACK_HH				0xfd8
#define SG_SEND_HH_TO			0xfd9
#define SG_CPU_JJQ              0xfda
#define SG_CPU_QY               0xfdb
#define SG_CPU_CMKJ             0xfdc
#define SG_CPU_SDBJ             0xfdd
#define SG_SEND_SPEAKVOL        0xfde
#define SG_SEND_NETSTAT			0xfdf
#define SG_SEND_LONGSMS			0xfe0
#define SG_TTS_COMBIN			0xfe1
#define SG_SEND_CALLOFF			0xfe2
#define SG_SEND_TIME			0xfe3
#define SG_MSG_HANDLE			0xfe4
#define SG_CAMERA_RESET			0xfe5
#define SG_CAMERA_MSGMAKE		0xfe6
#define SG_CAMERA_TIMEOUT		0xfe7
#define SG_CAMERA_WAKEUP		0xfe8 //new cmd
#define SG_CAMERA_ONE_ASK		0xfe9
#define SG_CAMERA_MORE_ASK		0xfea
#define SG_CAMERA_REST_ASK		0xfeb
#define SG_CAMERA_STOP_ASK		0xfec
#define SG_WAITACK				0xfee
#define SG_GPIO_CALL			0xfef
#define SG_ANS_CALL				0xff0
#define SG_CHECK_REBOOT			0xff1
#define SG_DEACTIVE_PDP			0xff2
#define SG_HANDUP				0xff3
#define SG_CHECK_SOCKET_CONNECT	0xff4
#define SG_CREATE_SOCKET		0xff5
#define SG_ACTIVE_PDP			0xff6
#define SG_SEND_SMS				0xff7
#define SG_ACK_MSG				0xff8
#define SG_SEND_MSG_TO			0xff9
#define SG_RCV_BUF				0xffa
#define SG_SEND_MSG				0xffb
#define SG_TIME_OUT				0xffc
#define SG_GPS_BUF				0xffd
#define SG_HH_BUF				0xffe

//hardware
#ifdef _DUART
#define PWR_SB 			9
#define PWR_HH          26
#define HF_MUTE 		46
#define CPU_CMKG_IN 	11	//���ż��,�͵�ƽ��Ч
#define CPU_LED 		18
#define CPU_ACC_IN 		19
#define CPU_XIHUO_OUT	16
#define CPU_JJQ_IN 		17
#define PIC_TXD 		20
#define PIC_ACK 		8
#define VCAR_PFO		10
#define PWREN_GPS		1
#define CPU_SDBJ_IN		6	//�ֶ����� ,�͵�ƽ��Ч
#define CPU_WDT			4
#endif

#ifdef _SUART
#ifdef _TRACK
#define PWR_SB 			32
#define CPU_CMKG_IN 	11	//���ż��,�͵�ƽ��Ч
#define CPU_LED 		18
#define CPU_ACC_IN 		19   //ACC
#define CPU_XIHUO_OUT	16
#define CPU_JJQ_IN 		17   //�Ƽ���
#define PIC_TXD 		20
#define PIC_ACK 		8
#define VCAR_PFO		10   //Ƿѹ
#define PWREN_GPS		1
#define CPU_WDT			4
#define CPU_SDBJ_IN		42   //�ֶ�����
#define CPU_DD_IN 		26   //����
#define CPU_LINE_IN 	46   //����
#else
#define PWR_SB 			32
#define HF_MUTE 		46
#define CPU_CMKG_IN 	11	//���ż��,�͵�ƽ��Ч
#define CPU_LED 		18
#define CPU_ACC_IN 		19
#define CPU_XIHUO_OUT	16
#define CPU_JJQ_IN 		17
#define PIC_TXD 		20
#define PIC_ACK 		8
#define VCAR_PFO		10
#define PWREN_GPS		1
#define CPU_SDBJ_IN		6	//�ֶ����� ,�͵�ƽ��Ч
#define CPU_WDT			4
#endif
#endif



#define PACKED_LSB_FIRST  0  // LSB first (1234 = 0x21, 0x43)
#define PACKED_MSB_FIRST  1  // MSB first (1234 = 0x12, 0x34)
#define UNPACKED          2  // unpacked  (1234 = 0x01, 0x02, 0x03, 0x04)


#define SG_GPS_TIME_LEN 15
#define SG_GPS_DATE_LEN 6
#define SG_GPS_LAT_LEN 15
#define SG_GPS_LONG_LEN 15
#define SG_GPS_SPEED_LEN 10
#define SG_GPS_ANGLE_LEN 10
#define SG_GPS_ALTITUDE_LEN 10
#define SG_EARTH_R 6371004
#define SG_ALARM_TIME 60
#define SG_TTS_TIME 33

#define SG_CALLNUM_SIZE 170


// ����buf����
#define UART_READ_BUF     2048
#define UART_SEND_BUF     256
#define UART_PARSE_BUF    400
#define MSG_EXT_DEV_LEN   256
#define UART_RED_BUF 	  256
#define TTS_SEND_BUF      1024

//�ع�����
#define PRO_ONE_LOCK      0x01
#define PRO_TWO_LOCK      0x02
#define PRO_UNCOIL_LOCK   0x03
#define PRO_VOLT_LOCK     0x04
#define PRO_VOLT_UNLOCK   0x05
#define PRO_LOCK_ACK      0x46
#define PRO_UNCOIL_ACK    0x47

//
//#define NO_CONNECT_TIME 300 //5����
//#define CONNETCT_DELAY  60  //60
#define DEFAULT_CHECK_WATCHDOG	30 //30s
#define CAMERA_DEFAULT_TIMEOUT	2
#define SG_VERSION_LEN 		20
#define SG_PHONE_LEN 		15
#define SG_MUSTER_LEN 		512
#define SG_IP_LEN 			12 	//IP��ַÿһ��ռ3λ�������ǰ��"0"
#define SG_PORT_LEN			5
#define SG_PSW_LEN 			8
#define PSW_SHA1_LEN   		20	//����sha_1ժҪ��λ��
#define SG_APN_LEN 			20
#define SG_GPRS_USER_LEN	16
#define SG_GPRS_PSW_LEN		16
#define SG_USER_NUM 		10
#define SG_MSG_NO_LEN 		4 	//�������кų���
#define SG_TCP_TIME_OUT		10	//TCP��ʱʱ��5��
#define SG_RCV_TIME_OUT		40	//TCP��ʱʱ��5��
#define SG_SEND_TIME_OUT	60	//GPRS���ͳ�ʱʱ��30��
#define SG_SMS_TIME_OUT		40	//������Ӧ��ʱʱ��40��
#define SG_KEY_TIME_OUT		120	//��Կ����ʱʱ��120��
#define SG_DTMF_TIME_OUT	180	//DTMF���ͱ�����Ϣ��ʱʱ��180��
#define SG_SUSPEND_TIME		15	//���߻��ѵȴ�ʱ��
#define SG_COMPRESS_NUM		10	//GPSѹ�����͵���Ŀ
#define SG_ALALM_SMS_NUM	1	//ʹ�ö��ŷ��ͱ���һ�η��͵ĸ���


#define SG_UART_NONE         	0
#define SG_UART_NAV            	1
#define SG_UART_THEFT        	2
#define SG_UART_VIDEO        	3
#define SG_UART_VIDEO2      	4
#define SG_UART_BUS_DEV    		5
#define SG_UART_LOCK	     	6
#define SG_UART_ELECTRON_DEV  	7
#define SG_UART_TEMP	     	8
#define SG_UART_VOICE 	     	9

//#define SG_TEMP_NODEF       32000   //��Ч���¶�ֵ

//#define SG_SET_FILE_NAME		"sg2000.set"		//g_set_info �ṹ�屣���ļ�
#define REGION_FILE_PATH		"region.set"		//�������ñ����ļ�
#define SG_SET_LINE_FILE 		"line.set"			//·�����ñ����ļ�
#define CONSULT_FILE_PATH       "consult.set"		//���������ļ�
#define CALLIN_FILE_FILE        "callin.set"  //�������Ʊ����ļ�
#define CALLOUT_FILE_FILE       "callout.set"  //�������Ʊ����ļ�

//#define MUSTER_FILE_PATH   		"interest.set"		//��Ȥ�����ñ����ļ�
#define NEW_LIST_FILE_NAME		"newlist.set"		//��ͨ���Ķ���
#define ALARM_LIST_FILE_NAME	"alarmlist.set"	//�������Ķ���
#define MUSTER_LIST_FILE_NAME		"musterlist.set"		//������Ϣ
#define CALLIN_LIST_FILE_NEME   "callin.set"  //�������Ʊ����ļ�
#define CALLOUT_LIST_FILE_NEME   "callout.set"  //�������Ʊ����ļ�

// ���ش洢ͼƬ�����ļ�
#define PICTURE_1_PATH   "pic0.set"  //ͼƬ1
#define PICTURE_2_PATH   "pic1.set"  //ͼƬ2
#define PICTURE_3_PATH   "pic2.set"  //ͼƬ3
#define PICTURE_4_PATH   "pic3.set"  //ͼƬ4
#define PICTURE_5_PATH   "pic4.set"  //ͼƬ5


#if (0)

#define DES_DEFAULT_KEY "23@44#55#55&*$@56%fg&key"	//�̶���Կ
#define SG_DEFAULT_CALL_PSW "00000000"				//Ĭ��ͨ������
#define SG_DEFAULT_THEFT_PSW "00000000"				//Ĭ�Ϸ�������
#define SG_DEFAULT_MANAGER_PSW "00000000"			//Ĭ�Ϲ���Ա����
#define SG_DEFAULT_SUPER_PSW "46387827"				//Ĭ�ϳ����û�����
#define SG_MY_SUPER_PSW "83842600"					//Ĭ�ϳ����û�����
#define SG_DEFAULT_PIN "1234"						//Ĭ��PIN
#endif /* (0) */
#define SG_DEFAULT_OVERSPEED 	100					//Ĭ�ϳ����ٶ�
#define SG_DEFAULT_OVERPLACE 	300					//Ĭ��λ�Ʊ�������
#define SG_OVERPLACE_CHECK_TIME 10					//Ĭ��λ�Ʊ�������������(��ֹGPSƯ��)
#define SG_DEFAULT_OVERLINE 	300					//Ĭ��·��ƫ�Ʊ�������
#define SG_DEFAULT_STOP  		30					//Ĭ���ж�ͣ���ľ���
#define SG_JUDGE_CAR_STOP_TIME  60					//�ж�ͣ��ʱ��
#define SG_JUDGE_ACC_ON_TIME    60					//�ж�ACC����ʱ��

//#define SG_PICTURE_SAVE_MAX_FILE 5					//ͼƬ�洢�ļ�

//��������״̬λ
#define	GPS_CAR_REGION_ALARM_NEED		0x00000001	//��Ҫ���±�����
#define	GPS_CAR_STATU_POS				0x00000001	//��λ״̬��0��δ��λ����0����λ��
#define	GPS_CAR_STATU_REPORT			0x00000001	//���ÿ��أ����ڱ������ʱҪ���ϴ�һ������
#define	GPS_CAR_STATU_HIJACK			0x00000002	//�پ���0����������0�������������ն˰�ť����������
#define	GPS_CAR_STATU_JJQ				0x0000000c	//�Ƽ���
#define	GPS_CAR_STATU_HELP				0x00000010	//���������ֱ��ϰ�ť������������
#define	GPS_CAR_STATU_MOVE_ALARM		0x00000020	//λ�Ʊ��������ݼ����������
#define	GPS_CAR_STATU_STOP_TIMEOUT		0x00000040	//��ʱͣ������
#define	GPS_CAR_STAT_DRIVE_TIMEOUT		0x00000080	//��ʱ��ʻ����
#define	GPS_CAR_STATU_LESS_PRESSURE		0x00000100	//Ƿѹ�����ݼ����������
#define	GPS_CAR_STATU_POWER_OFF			0x00000200	//���磨���ݼ����������
#define	GPS_CAR_STATU_REGION_IN			0x00000400	//�����򣨸����������򱨾����������ڼ��״̬�ű�����
#define	GPS_CAR_STATU_REGION_OUT		0x00000800	//�����򣨸����������򱨾����������ڼ��״̬�ű�����
#define	GPS_CAR_STATU_SPEED_ALARM		0x00001000	//���٣����������ٶȱ������������ڼ��״̬�ű�����
#define	GPS_CAR_STATU_PATH_ALRAM		0x00002000	//ƫ��·�ߣ���������·�߱������������ڼ��״̬�ű�����
#define	GPS_CAR_STATU_GAS_ALARM			0x00004000	//��·���������ݼ����������
#define	GPS_CAR_STATU_ROB_ALARM			0x00008000	//�������������ݼ����������
#define	GPS_CAR_STATU_ACC				0x00010000	//���
#define	GPS_CAR_STATU_DOOR				0x00020000	//����
#define	GPS_CAR_STATU_WINDOW			0x00040000	//����
#define	GPS_CAR_STATU_FRONT_COVER		0x00080000	//ǰ��
#define	GPS_CAR_STATU_BACK_COVER		0x00100000	//���
#define	GPS_CAR_STATU_DETECT			0x00200000	//̽��
#define	GPS_CAR_STATU_OILBOX          	0x00400000 	//����
#define	GPS_CAR_STATU_BRAKE          	0x00800000 	//ɲ��
#define	GPS_CAR_STATU_IO1		       	0x01000000 	//��չIO ��
#define	GPS_CAR_STATU_IOS_MASK          0x3e000000 	//��չ����������
#define	GPS_CAR_STATU_TEMP           	0x02000000	//�¶�
#define GPS_CAR_DUMP_STATUE				0x40000000  //������ײ����


/*bNewUart4Alarm
byte 0
bit 0 0x01  camera
bit 1 0x02   oldcamera
bit 2 0x04   gps navi
bit 3 0x08	  ������
bit 4 0x10    :��������
bit 5 0x20   :�п���
bit 6 0x40   tts
bit 7 0x80   temp
byte 1
bit 1 	0x01	electric
*/
#define UT_CAMERA		0x01
#define UT_QQCAMERA 	0x02
#define UT_GPS			0x04
#define UT_THREF		0x08
#define UT_BUS			0x10
#define UT_LOCK			0x20
#define UT_TTS			0x40
#define UT_TEMP			0x80
#define UT_DRIVE      	0x0100 //
#define UT_HBLED    	0x0200
#define UT_LED       	0x0400
#define UT_JJQ       	0x0800
//#define UT_PRINT      	0x1000
#define UT_PROLOCK     	0x1000  //�ع�����
#define UT_XGCAMERA   	0x2000
#define UT_WATCH      	0x4000
#define UT_LED2       	0x8000
#define UT_TTS2       	0x00010000


typedef  enum  SG_XIH_STATE
{
	SG_MENU_CTRL = 0,
	SG_REMOTE_CTRL
}sg_xih_ctrlstate;

typedef struct _SG_XIH_CTRL_
{
	int XIH_State;                   //���糬ʱ
	unsigned char menuid[4];
	unsigned char sMsgNo[SG_MSG_NO_LEN];				//�������к�
	unsigned char para[2];
}sg_xih_ctrl;

#ifdef _DUART
typedef struct _SG_PRO_LOCK_
{
	int state;           //״̬
	unsigned char menuid[4];
	unsigned char sMsgNum[SG_MSG_NO_LEN];				//�������к�
	int nType;		//��������
	int nCarNum;	//��������
	int time;       //��ʱ
	int VoltTime;
	int timeout;	//���ͳ�ʱ����
	int ack;
	unsigned char para[2];

}sg_pro_lock;
#endif

typedef struct _IO_STATUS_T
{
	uint8 sdbj_cnt;	//�ֶ�����
	uint8 sdbj_cancel_cnt;	//�ֶ�����
	uint8 cmjc_cnt;	//���ż��
	uint8 cmjc_cancel_cnt;	//���ż��
	uint8 jjq_cnt;	//�Ƽ���
	uint8 jjq_cancel_cnt;	//�Ƽ���
	uint8 dd_cnt; //����
	uint8 dd_cancel_cnt;	//����
	uint8 acc_cnt;	//acc
	uint8 acc_cancel_cnt;	//acc
	uint8 qy_cnt;	//Ƿѹ
	uint8 qy_cancel_cnt;	//Ƿѹ
	uint8 gpstx_cnt; //GPS����
	uint8 gpstx_cancel_cnt;

}IO_STATUS_T;

extern IO_STATUS_T g_io_status;


enum cfg_type
{
	REGION,
	LINE,
	MUSTER,
	CONSULT,
	CALLIN,
	CALLOUT
};

typedef struct _INFO_LIST_
{
	int  len;
	char msg[256];
	struct  _INFO_LIST_ *next;
}SG_INFO_LIST;		//��Ȥ�����ݽṹ��


typedef struct _INFO_MUSTER_
{
	int  nLatitude;		//γ��
	int  nLongitude;	//����
	int  musterID;  	//ID��
	int  nradiu;        //�뾶
	SG_INFO_LIST *text;
	struct  _INFO_MUSTER_ *next;
}SG_INFO_MUSTER;	// ��Ȥ��ṹ��


typedef  enum  SG_ERRNO
{
	SG_ERR_NONE = 0,
	SG_ERR_CREAT_SOCKET,
	SG_ERR_PROXY_CONNET,
	SG_ERR_PROXY_NOTFOUND,
	SG_ERR_PROXY_DENY,
	SG_ERR_CONNET_CENTER,
	SG_ERR_RCV_HANDUP,
	SG_ERR_SEND_DATA,
	SG_ERR_RCV_DATA,
	SG_ERR_RCV_ANS,
	SG_ERR_TIME_OUT,
	SG_ERR_CLOSE_SOCKET,
	SG_ERR_SET_OPT,
	SG_ERR_HOST_PARSE,
	SG_ERR_ACTIVE_PDP,
	SG_ERR_DEACTIVE_PDP,
	SG_ERR_OPEN_UDP,
	SG_ERR_SEND_UDP

}sg_errno;

typedef struct _SG_CAMERA_SET_
{
	int bCaptureWay;						//����ʽ
	int nCameraID;						//����ͷ ID
	int nInterval;						//���
	int nImageQuality;					//ͼ������
	char nImageBright;					//����
	char nContrast;						//�Աȶ�
	char nSaturation;						//���Ͷ�
	char nChroma;							//ɫ��
	unsigned short nCount;							//����
	short  setCount;					//���õĲ�����
	int   bSetted;					//�Ƿ������ù�?
	int   reserved[2];
}CAMERA;

typedef struct _SG_INTERVAL_WATCH_INFO_
{
	unsigned long startTime;
	unsigned long endTime;
	int interval; //������ڵ�λΪ��
	int bRedeem; //�յ㲹����0����������1��180�������Ƕȡ�
	int bCompress; //0x00�����δ��ͣ�0x01��ѹ������
	int bStopReport; //0x00�����㱨��0x01���㱨
}sg_intervalwatch_info;


typedef struct _SG_STATE_INFO_
{
	int nNetTimeOut;                   //���糬ʱ
	int nReConnect;                    // ��ʱ��������������
	int nGPSErr;						//
	int nGGACnt;						//
	int nRMCCnt;						//
	int nACnt;							//
	int nVCnt;							//
	int nHandupConCnt;					//
	int nPorxyConCnt;					//
	int nActiveCnt;						//
	int nDeactiveCnt;					//
	int nUdpRcvCnt;
	int nUdpSendCnt;
	int nGPSDevErrCnt;
	int nGPSDevHandupAckCnt;
	int nGPSDevOtherAckCnt;
	int nGPSDevMsgDownCnt;
	int nKeepingTime;
	int nHeartTotalCnt;
	int nBusCnt;
	int rxLevel; 					//�ź�ǿ��
	int rxLevelnum;                 //�źŵȼ�
	int opter;	//��Ӫ�̱���
	uint8 imsi[8];		//imsi
	int  lasterr;
	int plmn_status;    // ����״̬:0������
	int  socket;
	int  loginID;							//��ǰ���������
	int logoutID;
	int nLockCnt;
	int nLockErr;
	int nLedCnt;
	int nLedErr;
	int WatchError;
	char ActiveErr; // acitive ʱ�����Ĵ���
	unsigned char screen;	//��/����������
	unsigned char set;		//��/����
	unsigned char  lockState;                  //������IO��״̬
	char sListenOutNo[SG_PHONE_LEN + 1]; 	//���ļ��������绰����

	int photo_catch_cnt;
//	unsigned char sSuperPsw[12];	//�����û�����
	uint32  alarmState;			//״̬������
	uint32 ioState;
	unsigned char sMsgNo[SG_MSG_NO_LEN];				//�������к�
	int bNewUart4Alarm;					//�±�ʾ������ʾ��Դ�����豸����
	uint32 nGetTotalDistance;				//�����ͳ��ֵ
	int nGetDistance[2];				//0���ͳ��ֵ��1���2ͳ��ֵ
	uint32 WatchFlag;          // ͸���豸�ϴ���ر�־
	int WatchStatus; // ͸���豸״̬
	char bRelease1;// ����1������
	char bRelease2;// ����2������
	char sSpeedTime[SG_GPS_TIME_LEN + 1]; // ���ٿ�ʼʱ�䣬��ʽΪhhmmss.ss/hhmmss.sss
	char CameraOffset;                    //

	char *pConsult; 		            //����ο����б�:����(4)+�ο����б�ṹ
	char *pRegion; 						//������б�:����(4) + ������б�ṹ
	char *pLine;						//�г�·�ߵ��б�:����(4) + ·�ߵ��б�ṹ
	void *pGpsCurrent; 					//��ǰGPS����(�Ѿ��ֽ�Ľṹ)
	void *pGpsFull; 					//����ƥ��GGA��RMC��ɵ�GPS����(�Ѿ��ֽ�Ľṹ)
	void *pGpsReport;					//��һ�λ㱨��GPS����(�Ѿ��ֽ�Ľṹ)
	void *pGpsOverPlace;				//λ�Ʊ�����ʱGPS����(�Ѿ��ֽ�Ľṹ)
	void *pGpsRedeem;					//��һ�ιյ㲹��GPS����(�Ѿ��ֽ�Ľṹ)

	//����ͳ��
	int upOilQuantity; 				 //�ϴ�����
	int upAD;						//ʵʱ�ϴ�ʱ��ADֵ
	int CurrAD;						//��ǰ����ADֵ
	int AD[11];						//����
	int test;
	int F;

	int sdbj;
	int cmjc;
	int qy;
	int jjq;
	int dd;
	int acc;
	int gpstx;

	int drvTime;						//���ڳ�ʱ��ʻ�жϵ��ۻ�ʱ��
	int drvstopTime;					//���ڳ�ʱ��ʻͣ���ۻ�ʱ��(��)
	int stopTime;						//���ڳ�ʱͣ���ۻ�ʱ��(��)
	int lastCheckTime;                //�ϴ��ж�ͣ����ʱ��

	int randtime;                     //�����
	int line;
	int TestFlag;
	int GpsEnable;                     //GPS���ݽ���ʹ��
	int SimFlag;               //SIM����־  0:��  1:��
	int SendSMSFlag;           //���ż���־
	int AccOnTime;             // ACC����ʱ��ͳ��
	int LedState;              //LED״̬: 0��  1��  2����
	int LedTimeCount;          //LED��ʱ�ж�
	int LedOverTime;           //LED��ʱ����
	int SendSMSCount;          //���ͱ������ŵ�����
	int SendSMSType;           //���ͱ������ŵ�����: 1����  2����
	int GPSTime;               //GPS����λʱ��
	int GpsDownloadFlag;       //GPS������־

	char sAllowAnswer[SG_CALLNUM_SIZE];
	char sAllowCall[SG_CALLNUM_SIZE];   //���������б�
	char sUserNo[SG_PHONE_LEN + 5]; 	//�û�����
	char sCallBackToNo[SG_PHONE_LEN + 1];     //�ز���ϯ�绰����
	char IMEI[20];


	int GpsDevType;  // 1 ΪZB������ 2ΪYH������
	short hardware_ver; // Ӳ���汾 (����1.54)
	char user_type; // �û�����(1:��ͨ�汾  2: ˽�ҳ��汾)
	char Rcv_Data_To;// �������ݳ�ʱ
	uint8 ZB_Num;

}sg_state_info;

typedef struct _SG_WATCH_INFO_
{
	uint32 nGetTotalDistance;				//�����ͳ��ֵ
	int nGetDistance[2];				//0���ͳ��ֵ��1���2ͳ��ֵ
	uint32 nWatchedTime;				//�Ѿ����ʱ��
	uint32 nLastReportSystemTime;		//�ϴλ㱨ϵͳʱ��
	int AccOnTime;             // ACC����ʱ��ͳ��
	char sLatitude[SG_GPS_LAT_LEN + 1]; //γ��, ��ʽΪddmm.mmmm/ddmm.mmmmm
	char sLongitude[SG_GPS_LONG_LEN + 1]; //���ȣ���ʽΪdddmm.mmmm/dddmm.mmmmm
	char Reserve[44];
}sg_watch_info;


typedef struct _CAMERA_SAVE_
{
//	unsigned char path[20];
	int pic_save;
}sg_camera_save;

//������ʱ����
typedef struct _SMS_SAVE_
{
	unsigned char flag;            //�����־
	unsigned char alphabet_type;   //���뷽ʽ
	unsigned short len;            //����
	unsigned char msg[200];		       //����
	unsigned char telenum[20];     //����

}sg_sms_save;

typedef struct _SG_SET_INFO_
{

	char sOwnNo[SG_PHONE_LEN + 1]; 		//��̨������(�ն˺�/�����ֻ���)
	char sCenterNo[SG_PHONE_LEN + 1]; 	//���ĺ���
	char sPoliceNo[SG_PHONE_LEN + 1]; 	//��������
	char sHelpNo[SG_PHONE_LEN + 1]; 		//��������
	char sListenNo[SG_PHONE_LEN + 1]; 	//���ļ���¼���绰����
	char sOneGPSNo[SG_PHONE_LEN + 1]; 	//һ������ͨ������
	char sMedicalNo[SG_PHONE_LEN + 1]; 	//ҽ�ƾȻ��绰����
	char sServiceNo[SG_PHONE_LEN + 1]; 	//ά�޵绰����
	char sCenterIp[SG_IP_LEN + 3 + 1]; 	//����IP������3��'.'
	char sProxyIp[SG_IP_LEN + 3 + 1]; 	//����IP������3��'.'

	int nCenterPort; 					//���Ķ˿�
	int nProxyPort; 					//����˿�

	char sAPN[SG_APN_LEN + 1];			//����APN��ַ
	char sGprsUser[SG_GPRS_USER_LEN+1];	//GPRS�û���
	char sGprsPsw[SG_GPRS_PSW_LEN+1];	//GPRS����
	char bRegisted;						//��̨�Ƿ��Ѿ�ע��Ǽ�0:δ�Ǽ�1:�ѵǼ�

	int nDisconnectTime;				//�Ͽ�һ��ʱ�����������
	int nResetCnt;

	char bProxyEnable; 					//����ʹ��1:ʹ�ô���0:��ʹ�ô���
	char nNetType; 						//ͨѶ��ʽ0:����1:GPRS
	char nSamplingInflexionRedeem; 		//��ϻ�Ӳ����յ㲹��0����������1��180�������Ƕ�
	char nSamplingType; 				//��ϻ�Ӳ�����ʽ0x00�����ɼ�0x01����ʱ�ɼ�0x10������ɼ�

	int nSamplingInterval;				//��ϻ�Ӳ������

	int nOverSpeed; 					//�����ٶ�
	int nOverSpeedTime; 				//���ٳ���ʱ��
	int nReConCnt;						//

	char bOverSpeedEnable;				//���ٱ�������1:����0��ֹ
	char bRegionEnable;					//���򱨾�����1:����0��ֹ
	char bLineEnable;					//·�߱�������1:����0��ֹ
	char bCallOutDisable;				//������ֹ0:����1:��ֹ

	char bCallInDisable;				//�����ֹ0:����1:��ֹ
	char nWatchType; 					//��ط�ʽ
	char nInflexionRedeem; 				//�յ㲹��0����������1��180�������Ƕ�
	char bCompress; 					//ѹ������1:ѹ��0��ѹ��

	int nWatchInterval; 				//�������
	uint32 nWatchTime; 					//���ʱ��

	int nGpstTime;                       //GPS��λ���˵�ʱ��
	int nGpstDist;                       //GPS���ͳ�Ƶľ���

//	char sAllowAnswer[SG_CALLNUM_SIZE];
//	char sAllowCall[SG_CALLNUM_SIZE];   //���������б�
	char bLockTest;
	char bRelease1;// ����1������
	char bRelease2;// ����2������


	uint32 alarmEnable;			//����������
	uint32 alarmRepeat;			//�㱨������

	uint32    nIoAlarmMask;               //IO �������
	int	bIoOutLevel;                    // IO �����ƽ: 0 ����, 1 ����
	int bnewSpeedLine;

	int bNewUart4Alarm;					//�±�ʾ������ʾ��Դ�����豸����
	int bnewRegion; 					//�����ۺ�
	uint32 ReserveAlarmEnable; 	//��������������
	uint32 alarmState;			//״̬������

	unsigned char sWatchMsgNo[SG_MSG_NO_LEN];			//��ػ㱨�ı������к�
	unsigned char sMusterMsgNo[SG_MSG_NO_LEN];			//��������ı������к�
	unsigned char sWatchIntervalMsgNo[SG_MSG_NO_LEN];	//��ѯ��صı������к�
	unsigned char sExtMsgNo[SG_MSG_NO_LEN];	//����led ��ر�����ˮ��

	char sVersion[60];	//�����汾��Ϣ
	unsigned char sCameraMsgNo[SG_MSG_NO_LEN];	//��ػ㱨�ı������к�


	int nRegionPiont;
	int nLinePiont;

	unsigned int alarmSet;
	uint32 nSmsInterval;					//���ż��
	uint32 nSmsInterval_data;					//���ģʽ���ż��

	int nCameraAlarm;					//����ͷ��ر�־λ
	CAMERA sg_camera[2];

	unsigned char sSafetyPsw[8];	// �������İ�ȫ��������
	unsigned char sSuperPsw[12];	//�����û�����
	unsigned char sUserPsw[12];     //�ֻ���������
	unsigned char sChangePsw[12];     //�ֻ����ð���
	char sUserNo[SG_PHONE_LEN + 1]; 	//�ֻ��û�����
	char sSmsAlarmNo1[SG_PHONE_LEN + 1]; 	//���ű�������1.
	char sSmsAlarmNo2[SG_PHONE_LEN + 1]; 	//���ű�������2

	int SetEnable;                    //�û��ֻ����ð� 1:�� 0:ȡ��
	int SmsEnable;                    //�û��ֻ������� 1:�� 0:ȡ��

	int devID;  // ͸���豸������֮��ͨ�ŵ������ʶID��
	int watchid; // ͸���豸ID�� 1 �Ƽ��� 2���ش�����
	int nSetTime; // ͸���豸͸����ʱʱ�� 1-60 ��
	int watchbd; // ͸���豸������
	int bAutoAnswerDisable;				//�Զ����� 0:�ֱ�ͨ�� 1:(Ĭ��)����ͨ��
	int speaker_vol;  // ͨ������
	//����
	int nTotalVolume;					 //�������ݻ�,��0.01Ϊ��λ
	int nAlarmVol; 					   //�����쳣�仯��ֵ
	int nkeepTime; 					  //����ʱ��
	int MaxAD; 							//AD���ֵ
	int MinAD; 				  //AD	��Сֵ
	int K_AD;				 //����ʱ�ĵ�ѹ�ϸ��ٷֱ�
	int upOilQuantity; 				 //�ϴ�����
	int upAD;						//ʵʱ�ϴ�ʱ��ADֵ
	int OilFlag;                //�������ͱ�־ 0:������ADֵ���� 1:������ADֵ����
	int nRingVol;           // ��������

	int bStopToutEnable;			//��ʱͣ��0:��ֹ1:����
	int nStopTout;					//�����澯ʱ��
	int bDrvToutEnable;					//��ʱ��ʻ
	int nDrvTout;						//��ʱ��ʻʱ��(��)

	int nTheftState;           //����״̬  0:��     1:��
	int bTheftDisable;         //����ʹ��  0:����    1:��ֹ
	int GPSEnable;             //��ؿ���  0:��   1:��
	int PNDEnable;             //��������  0:��   1:��
	int SmsSndTimer;
	int LedResetFlag;              //����2LED������־: 0������  1����
	int jdqFlag;                   //Ϩ��״̬:     0Ϩ��ָ�    1Ϩ��
	int ddjdqFlag;                  //�����Ƿ�Ϩ��:   0��   1��
	int ACCGPS;                    //ACC��ʱ�Ƿ���¾�γ��:  0��  1��
	int JDQ;                       //����ָ��:   0����״̬   1����״̬

	int drvTime;						//���ڳ�ʱ��ʻ�жϵ��ۻ�ʱ��
	int drvstopTime;					//���ڳ�ʱ��ʻͣ���ۻ�ʱ��(��)
	int stopTime;						//���ڳ�ʱͣ���ۻ�ʱ��(��)
	int lastCheckTime;                //�ϴ��ж�ͣ����ʱ��
	int nsetregion;
	int nsetline;
	int GpsDevType;        // 1 ΪZB������ 2ΪYH������
	int GpsAutoCall;       //�������Զ�����
	int JJQLev;             //�Ƽ���������ƽ  0:��  1:��
	int EfsFlag;           //�ļ���д��־
	int EfsMax;            //ä����������
	int bPictureSave;					//ͼ�񱣴��־
	int nPictureSaveIndex;				//ͼ�񱣴��ļ�������
	int Heart_Switch;          // �������أ�Ĭ�Ϲر�
	int Heart_Interval;       // ���������Ĭ��120s
	int SmsAlarmFlag;        //����ģʽ�£�GPRS��ͨ�����Ƿ��Ͷ���:  0��Ҫ  1Ҫ
	int GpsPowrFlag;         //GPS��Դ��־   0�ر�  1����
	sg_camera_save camera_save_list[5];


//	char IoDefineLev; 					//  ���ű�������Ϊ�Զ��屨�� ������ƽ:1 �ߵ�ƽ���� 0:�͵�ƽ����
	char bStopReport;					//�Ƿ�ͣ���㱨: 1: �㱨    0: ���㱨
	char bBatteryCheckEnable;  			// �Ƿ��ؼ����Ч 1:��Ч  0:��Ч

	char Reserve[220];
	unsigned short checksum;  // CRC16��checksum


}sg_set_info;


//add by chenhh
typedef struct _MMI_PARTY_NUMBER_T
{
    MN_NUMBER_TYPE_E  number_type;
    uint16			  num_len;
    uint8			  bcd_num[MN_MAX_ADDR_BCD_LEN];
} MMI_PARTY_NUMBER_T;






typedef enum
{
	GPRS_NORMAL_START,
	GPRS_ACTIVE_PDP,
	GPRS_DEACTIVE_PDP,
	GPRS_SOCKET_CONNECT,
	GPRS_PORXY,
	GPRS_HANDUP,
	GPRS_RXHEAD,
	GPRS_RXCONTENT,
	GPRS_EWOULDBLOCK,
	GPRS_DISCONNECT,
	GPRS_ERR
}GPRS_STATE_E;

typedef struct _GPRS_T
{
	uint8 state;
	uint8 nextstate;
	uint32 timeout;

}GPRS_T;

extern GPRS_T g_gprs_t;

//end chenhh

//External Variables
extern sg_set_info g_set_info;
extern sg_state_info g_state_info;
extern sg_watch_info g_watch_info;
extern sg_sms_save g_sms_save;

extern char PhoneFlag;
extern BOOLEAN PhoneAllow;
extern sg_xih_ctrl g_xih_ctrl;
#ifdef _DUART
extern sg_pro_lock g_pro_lock;
#endif
extern uint32 SecTimer;
extern uint32 ReConCnt;
extern uint32 HhTimeCorrectTimer; // �ֱ�Уʱ����

#define GPS_PUT_BUF_SIZE 128
extern uint8  GPSPutBuf[GPS_PUT_BUF_SIZE];
extern uint32 WRGPSPutBuf,RDGPSPutBuf;
extern uint32 GPSTimer;
extern uint32 LedTimer;
extern uint8  LedCnt;
extern uint8  DataLedCnt;
extern uint32 ResetTimer;
extern uint32 CameraWatchTimer;
extern uint32 CameraConditionTimer;
extern uint32 WatchDevTimer;

extern uint32 Gps_Rev_Timeout;
extern int Gps_PwrOff_Timeout;
extern uint32 Pwm_Start_Time;
extern uint32 Pwm_End_Time;

//extern uint8 bHandleFlag;
//extern char HandleBuf[1024];
//extern int HandleBufLen;

extern uint32 SmsSndTimer; 	//sms���Ͷ�ʱ��
extern uint32 ConnectPhoneTimer;
extern int Restart_Reason;

#define GPRS_RXBUF_SIZE (1024*10)			//
extern int GPRS_RDIndex,GPRS_WRIndex;
extern int GPRS_RxLen;
extern uint8 GPRS_RxHead;
extern char Sms_Long_Send_Flag;

extern SCI_TIME_T  g_reboot_time;
extern SCI_DATE_T g_reboot_date;


extern char PicAckFlag;
extern char PicAckTime;
extern char PicSndCnt;
extern char PicAckLowTime;
extern char PicAckHighTime;

extern char Log_Enable1;
extern char Log_Enable2;

extern short MAX_LIST_NUM;
extern short MAX_ALARM_NUM;
extern int WDT_Timer;
extern int GPS_Timer;

// add by dongjz
#define DEFAULT_SMS_INTERVAL	60*60	 //30min
#define DEFAULT_HEART_INTERVAL	90	 //2min

extern sg_send_list *gp_new_list; //�´������Ķ���
extern sg_send_list *gp_alarm_list; //��������
extern int gn_new_list; //�´������Ķ��нڵ���Ŀ
extern int gn_alarm_list; //�������нڵ���Ŀ
extern uint32 NetSearchTimer;
extern int NetSearchCount;

extern unsigned char Uart_Read_Buf[2048];
extern unsigned char Uart_Parse[1024];
extern unsigned char Uart_Snd_Buf[256];
extern unsigned char TTS_Snd_Buf[1024];
extern char PictureUpFlag;// 0 not up 1 up

extern int UartReadCnt ;
extern int UartParseCnt;

#ifdef _DUART
extern unsigned char Uart_Read_COM2[1024];
extern unsigned char Uart_Parse_COM2[1024];
extern unsigned char Uart_Snd_COM2[256];
extern int UartReadCnt_2 ;
extern int UartParseCnt_2;
#endif

//External Prototypes
void sg_set_cfg_init(uint8 type);
void SG_Soft_Reset(int delay);
int SG_Set_Save(void);
int SG_Set_Init(void);
int SG_Get_New_List_File(void);
int SG_Get_Alarm_List_File(void);
void Pic_Send (const char data);
int Pic_Receive_Ack(void);
void SG_Set_Reset(void);
int SG_Set_Cfg_Save(uint8 type,unsigned char *para,int len);
BOOLEAN SG_Set_Check_CheckSum(void);
void SG_Set_Cal_CheckSum(void);
void SG_Set_Default(void);
void SG_State_Init(void);


#endif //_SG_SET_H_


