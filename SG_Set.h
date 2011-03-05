/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2008 版权所有
  文件名 ：SG_Set.h
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-7-23
  内容描述：SG2000 系统设置信息和用户管理
  修改记录：
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


// 分支定义

#define __cwt_ (FALSE)

#define __camera_debug_ (FALSE)

#define __gps_debug_ (FALSE)


//版本分支
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

#define HH_ENABLE 0x01  //手柄
#define DEBUG_ENABLE 0x02  //LOG
#define DEV_ENABLE 0x04   // 有源串口的外设
#define LED2_ENABLE 0x08  //外设接在串口2



// 信号定义
#define SG_QQZM_PICTURE			0xfc7
#define SG_SMS_YH				0xfc8
#define SG_SMS_HH               0xfc9
#define SG_CPU_DD               0xfca
#define SG_LONGMSG_HANDLE		0xfcb
#define SG_PIC_UP				0xfcc // 上传图片
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
#define CPU_CMKG_IN 	11	//车门检测,低电平有效
#define CPU_LED 		18
#define CPU_ACC_IN 		19
#define CPU_XIHUO_OUT	16
#define CPU_JJQ_IN 		17
#define PIC_TXD 		20
#define PIC_ACK 		8
#define VCAR_PFO		10
#define PWREN_GPS		1
#define CPU_SDBJ_IN		6	//手动报警 ,低电平有效
#define CPU_WDT			4
#endif

#ifdef _SUART
#ifdef _TRACK
#define PWR_SB 			32
#define CPU_CMKG_IN 	11	//车门检测,低电平有效
#define CPU_LED 		18
#define CPU_ACC_IN 		19   //ACC
#define CPU_XIHUO_OUT	16
#define CPU_JJQ_IN 		17   //计价器
#define PIC_TXD 		20
#define PIC_ACK 		8
#define VCAR_PFO		10   //欠压
#define PWREN_GPS		1
#define CPU_WDT			4
#define CPU_SDBJ_IN		42   //手动报警
#define CPU_DD_IN 		26   //掉电
#define CPU_LINE_IN 	46   //天线
#else
#define PWR_SB 			32
#define HF_MUTE 		46
#define CPU_CMKG_IN 	11	//车门检测,低电平有效
#define CPU_LED 		18
#define CPU_ACC_IN 		19
#define CPU_XIHUO_OUT	16
#define CPU_JJQ_IN 		17
#define PIC_TXD 		20
#define PIC_ACK 		8
#define VCAR_PFO		10
#define PWREN_GPS		1
#define CPU_SDBJ_IN		6	//手动报警 ,低电平有效
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


// 串口buf长度
#define UART_READ_BUF     2048
#define UART_SEND_BUF     256
#define UART_PARSE_BUF    400
#define MSG_EXT_DEV_LEN   256
#define UART_RED_BUF 	  256
#define TTS_SEND_BUF      1024

//重工锁车
#define PRO_ONE_LOCK      0x01
#define PRO_TWO_LOCK      0x02
#define PRO_UNCOIL_LOCK   0x03
#define PRO_VOLT_LOCK     0x04
#define PRO_VOLT_UNLOCK   0x05
#define PRO_LOCK_ACK      0x46
#define PRO_UNCOIL_ACK    0x47

//
//#define NO_CONNECT_TIME 300 //5分钟
//#define CONNETCT_DELAY  60  //60
#define DEFAULT_CHECK_WATCHDOG	30 //30s
#define CAMERA_DEFAULT_TIMEOUT	2
#define SG_VERSION_LEN 		20
#define SG_PHONE_LEN 		15
#define SG_MUSTER_LEN 		512
#define SG_IP_LEN 			12 	//IP地址每一段占3位，不足的前补"0"
#define SG_PORT_LEN			5
#define SG_PSW_LEN 			8
#define PSW_SHA1_LEN   		20	//密码sha_1摘要后位数
#define SG_APN_LEN 			20
#define SG_GPRS_USER_LEN	16
#define SG_GPRS_PSW_LEN		16
#define SG_USER_NUM 		10
#define SG_MSG_NO_LEN 		4 	//报文序列号长度
#define SG_TCP_TIME_OUT		10	//TCP超时时间5秒
#define SG_RCV_TIME_OUT		40	//TCP超时时间5秒
#define SG_SEND_TIME_OUT	60	//GPRS发送超时时间30秒
#define SG_SMS_TIME_OUT		40	//短信响应超时时间40秒
#define SG_KEY_TIME_OUT		120	//密钥请求超时时间120秒
#define SG_DTMF_TIME_OUT	180	//DTMF发送报警信息超时时间180秒
#define SG_SUSPEND_TIME		15	//休眠唤醒等待时间
#define SG_COMPRESS_NUM		10	//GPS压缩发送的数目
#define SG_ALALM_SMS_NUM	1	//使用短信发送报警一次发送的个数


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

//#define SG_TEMP_NODEF       32000   //无效的温度值

//#define SG_SET_FILE_NAME		"sg2000.set"		//g_set_info 结构体保存文件
#define REGION_FILE_PATH		"region.set"		//区域设置保存文件
#define SG_SET_LINE_FILE 		"line.set"			//路线设置保存文件
#define CONSULT_FILE_PATH       "consult.set"		//油量保存文件
#define CALLIN_FILE_FILE        "callin.set"  //呼入限制保存文件
#define CALLOUT_FILE_FILE       "callout.set"  //呼出限制保存文件

//#define MUSTER_FILE_PATH   		"interest.set"		//兴趣点设置保存文件
#define NEW_LIST_FILE_NAME		"newlist.set"		//普通报文队列
#define ALARM_LIST_FILE_NAME	"alarmlist.set"	//报警报文队列
#define MUSTER_LIST_FILE_NAME		"musterlist.set"		//调度信息
#define CALLIN_LIST_FILE_NEME   "callin.set"  //呼入限制保存文件
#define CALLOUT_LIST_FILE_NEME   "callout.set"  //呼出限制保存文件

// 本地存储图片保存文件
#define PICTURE_1_PATH   "pic0.set"  //图片1
#define PICTURE_2_PATH   "pic1.set"  //图片2
#define PICTURE_3_PATH   "pic2.set"  //图片3
#define PICTURE_4_PATH   "pic3.set"  //图片4
#define PICTURE_5_PATH   "pic4.set"  //图片5


#if (0)

#define DES_DEFAULT_KEY "23@44#55#55&*$@56%fg&key"	//固定密钥
#define SG_DEFAULT_CALL_PSW "00000000"				//默认通话密码
#define SG_DEFAULT_THEFT_PSW "00000000"				//默认防盗密码
#define SG_DEFAULT_MANAGER_PSW "00000000"			//默认管理员密码
#define SG_DEFAULT_SUPER_PSW "46387827"				//默认超级用户密码
#define SG_MY_SUPER_PSW "83842600"					//默认超级用户密码
#define SG_DEFAULT_PIN "1234"						//默认PIN
#endif /* (0) */
#define SG_DEFAULT_OVERSPEED 	100					//默认超速速度
#define SG_DEFAULT_OVERPLACE 	300					//默认位移报警距离
#define SG_OVERPLACE_CHECK_TIME 10					//默认位移报警连续检查次数(防止GPS漂移)
#define SG_DEFAULT_OVERLINE 	300					//默认路线偏移报警距离
#define SG_DEFAULT_STOP  		30					//默认判断停车的距离
#define SG_JUDGE_CAR_STOP_TIME  60					//判断停车时间
#define SG_JUDGE_ACC_ON_TIME    60					//判断ACC开启时间

//#define SG_PICTURE_SAVE_MAX_FILE 5					//图片存储文件

//车辆报警状态位
#define	GPS_CAR_REGION_ALARM_NEED		0x00000001	//需要更新报警字
#define	GPS_CAR_STATU_POS				0x00000001	//定位状态（0：未定位，非0：定位）
#define	GPS_CAR_STATU_REPORT			0x00000001	//复用开关，用于报警清除时要求上传一条报文
#define	GPS_CAR_STATU_HIJACK			0x00000002	//劫警（0：正常，非0：报警）（按终端按钮触发报警）
#define	GPS_CAR_STATU_JJQ				0x0000000c	//计加器
#define	GPS_CAR_STATU_HELP				0x00000010	//求助（按手柄上按钮，触发报警）
#define	GPS_CAR_STATU_MOVE_ALARM		0x00000020	//位移报警（根据检测结果报警）
#define	GPS_CAR_STATU_STOP_TIMEOUT		0x00000040	//超时停车报警
#define	GPS_CAR_STAT_DRIVE_TIMEOUT		0x00000080	//超时驾驶报警
#define	GPS_CAR_STATU_LESS_PRESSURE		0x00000100	//欠压（根据检测结果报警）
#define	GPS_CAR_STATU_POWER_OFF			0x00000200	//掉电（根据检测结果报警）
#define	GPS_CAR_STATU_REGION_IN			0x00000400	//入区域（根据所设区域报警，车辆处于监控状态才报警）
#define	GPS_CAR_STATU_REGION_OUT		0x00000800	//出区域（根据所设区域报警，车辆处于监控状态才报警）
#define	GPS_CAR_STATU_SPEED_ALARM		0x00001000	//超速（根据所设速度报警，车辆处于监控状态才报警）
#define	GPS_CAR_STATU_PATH_ALRAM		0x00002000	//偏移路线（根据所设路线报警，车辆处于监控状态才报警）
#define	GPS_CAR_STATU_GAS_ALARM			0x00004000	//油路报警（根据检测结果报警）
#define	GPS_CAR_STATU_ROB_ALARM			0x00008000	//防盗报警（根据检测结果报警）
#define	GPS_CAR_STATU_ACC				0x00010000	//点火
#define	GPS_CAR_STATU_DOOR				0x00020000	//车门
#define	GPS_CAR_STATU_WINDOW			0x00040000	//车窗
#define	GPS_CAR_STATU_FRONT_COVER		0x00080000	//前盖
#define	GPS_CAR_STATU_BACK_COVER		0x00100000	//后盖
#define	GPS_CAR_STATU_DETECT			0x00200000	//探测
#define	GPS_CAR_STATU_OILBOX          	0x00400000 	//油箱
#define	GPS_CAR_STATU_BRAKE          	0x00800000 	//刹车
#define	GPS_CAR_STATU_IO1		       	0x01000000 	//扩展IO 口
#define	GPS_CAR_STATU_IOS_MASK          0x3e000000 	//扩展适配器掩码
#define	GPS_CAR_STATU_TEMP           	0x02000000	//温度
#define GPS_CAR_DUMP_STATUE				0x40000000  //汽车碰撞报警


/*bNewUart4Alarm
byte 0
bit 0 0x01  camera
bit 1 0x02   oldcamera
bit 2 0x04   gps navi
bit 3 0x08	  防盗器
bit 4 0x10    :沈阳公交
bit 5 0x20   :中控锁
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
#define UT_PROLOCK     	0x1000  //重工锁车
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
	int XIH_State;                   //网络超时
	unsigned char menuid[4];
	unsigned char sMsgNo[SG_MSG_NO_LEN];				//报文序列号
	unsigned char para[2];
}sg_xih_ctrl;

#ifdef _DUART
typedef struct _SG_PRO_LOCK_
{
	int state;           //状态
	unsigned char menuid[4];
	unsigned char sMsgNum[SG_MSG_NO_LEN];				//报文序列号
	int nType;		//命令类型
	int nCarNum;	//车辆类型
	int time;       //计时
	int VoltTime;
	int timeout;	//发送超时次数
	int ack;
	unsigned char para[2];

}sg_pro_lock;
#endif

typedef struct _IO_STATUS_T
{
	uint8 sdbj_cnt;	//手动报警
	uint8 sdbj_cancel_cnt;	//手动报警
	uint8 cmjc_cnt;	//车门检测
	uint8 cmjc_cancel_cnt;	//车门检测
	uint8 jjq_cnt;	//计价器
	uint8 jjq_cancel_cnt;	//计价器
	uint8 dd_cnt; //掉电
	uint8 dd_cancel_cnt;	//掉电
	uint8 acc_cnt;	//acc
	uint8 acc_cancel_cnt;	//acc
	uint8 qy_cnt;	//欠压
	uint8 qy_cancel_cnt;	//欠压
	uint8 gpstx_cnt; //GPS天线
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
}SG_INFO_LIST;		//兴趣点内容结构体


typedef struct _INFO_MUSTER_
{
	int  nLatitude;		//纬度
	int  nLongitude;	//经度
	int  musterID;  	//ID号
	int  nradiu;        //半径
	SG_INFO_LIST *text;
	struct  _INFO_MUSTER_ *next;
}SG_INFO_MUSTER;	// 兴趣点结构体


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
	int bCaptureWay;						//捕获方式
	int nCameraID;						//摄像头 ID
	int nInterval;						//间隔
	int nImageQuality;					//图像质量
	char nImageBright;					//亮度
	char nContrast;						//对比度
	char nSaturation;						//饱和度
	char nChroma;							//色度
	unsigned short nCount;							//次数
	short  setCount;					//设置的参数项
	int   bSetted;					//是否已配置过?
	int   reserved[2];
}CAMERA;

typedef struct _SG_INTERVAL_WATCH_INFO_
{
	unsigned long startTime;
	unsigned long endTime;
	int interval; //监控周期单位为秒
	int bRedeem; //拐点补偿：0，不补偿；1～180，补偿角度。
	int bCompress; //0x00：单次传送；0x01：压缩传送
	int bStopReport; //0x00：不汇报；0x01：汇报
}sg_intervalwatch_info;


typedef struct _SG_STATE_INFO_
{
	int nNetTimeOut;                   //网络超时
	int nReConnect;                    // 因超时而断线重连次数
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
	int rxLevel; 					//信号强度
	int rxLevelnum;                 //信号等级
	int opter;	//运营商编码
	uint8 imsi[8];		//imsi
	int  lasterr;
	int plmn_status;    // 网络状态:0无网络
	int  socket;
	int  loginID;							//当前进入的区域
	int logoutID;
	int nLockCnt;
	int nLockErr;
	int nLedCnt;
	int nLedErr;
	int WatchError;
	char ActiveErr; // acitive 时发生的错误
	unsigned char screen;	//开/闭锁屏蔽字
	unsigned char set;		//开/闭锁
	unsigned char  lockState;                  //适配器IO口状态
	char sListenOutNo[SG_PHONE_LEN + 1]; 	//中心监听呼出电话号码

	int photo_catch_cnt;
//	unsigned char sSuperPsw[12];	//超级用户密码
	uint32  alarmState;			//状态报警字
	uint32 ioState;
	unsigned char sMsgNo[SG_MSG_NO_LEN];				//报文序列号
	int bNewUart4Alarm;					//新标示用来表示有源串口设备类型
	uint32 nGetTotalDistance;				//总里程统计值
	int nGetDistance[2];				//0里程统计值；1里程2统计值
	uint32 WatchFlag;          // 透传设备上传监控标志
	int WatchStatus; // 透传设备状态
	char bRelease1;// 串口1的配置
	char bRelease2;// 串口2的配置
	char sSpeedTime[SG_GPS_TIME_LEN + 1]; // 超速开始时间，格式为hhmmss.ss/hhmmss.sss
	char CameraOffset;                    //

	char *pConsult; 		            //油箱参考点列表:长度(4)+参考点列表结构
	char *pRegion; 						//区域点列表:长度(4) + 区域点列表结构
	char *pLine;						//行车路线点列表:长度(4) + 路线点列表结构
	void *pGpsCurrent; 					//当前GPS数据(已经分解的结构)
	void *pGpsFull; 					//完整匹配GGA和RMC组成的GPS数据(已经分解的结构)
	void *pGpsReport;					//上一次汇报的GPS数据(已经分解的结构)
	void *pGpsOverPlace;				//位移报警打开时GPS数据(已经分解的结构)
	void *pGpsRedeem;					//上一次拐点补偿GPS数据(已经分解的结构)

	//油量统计
	int upOilQuantity; 				 //上传油量
	int upAD;						//实时上传时的AD值
	int CurrAD;						//当前油箱AD值
	int AD[11];						//测试
	int test;
	int F;

	int sdbj;
	int cmjc;
	int qy;
	int jjq;
	int dd;
	int acc;
	int gpstx;

	int drvTime;						//用于超时驾驶判断的累积时间
	int drvstopTime;					//用于超时驾驶停车累积时间(秒)
	int stopTime;						//用于超时停车累积时间(秒)
	int lastCheckTime;                //上次判定停车的时间

	int randtime;                     //随机数
	int line;
	int TestFlag;
	int GpsEnable;                     //GPS数据接收使能
	int SimFlag;               //SIM卡标志  0:无  1:有
	int SendSMSFlag;           //短信检测标志
	int AccOnTime;             // ACC开启时间统计
	int LedState;              //LED状态: 0关  1开  2故障
	int LedTimeCount;          //LED超时判断
	int LedOverTime;           //LED超时次数
	int SendSMSCount;          //发送报警短信的条数
	int SendSMSType;           //发送报警短信的类型: 1防盗  2报警
	int GPSTime;               //GPS不定位时间
	int GpsDownloadFlag;       //GPS升级标志

	char sAllowAnswer[SG_CALLNUM_SIZE];
	char sAllowCall[SG_CALLNUM_SIZE];   //呼出限制列表
	char sUserNo[SG_PHONE_LEN + 5]; 	//用户号码
	char sCallBackToNo[SG_PHONE_LEN + 1];     //回拨坐席电话号码
	char IMEI[20];


	int GpsDevType;  // 1 为ZB导航屏 2为YH导航屏
	short hardware_ver; // 硬件版本 (例如1.54)
	char user_type; // 用户类型(1:普通版本  2: 私家车版本)
	char Rcv_Data_To;// 接收数据超时
	uint8 ZB_Num;

}sg_state_info;

typedef struct _SG_WATCH_INFO_
{
	uint32 nGetTotalDistance;				//总里程统计值
	int nGetDistance[2];				//0里程统计值；1里程2统计值
	uint32 nWatchedTime;				//已经监控时间
	uint32 nLastReportSystemTime;		//上次汇报系统时间
	int AccOnTime;             // ACC开启时间统计
	char sLatitude[SG_GPS_LAT_LEN + 1]; //纬度, 格式为ddmm.mmmm/ddmm.mmmmm
	char sLongitude[SG_GPS_LONG_LEN + 1]; //径度，格式为dddmm.mmmm/dddmm.mmmmm
	char Reserve[44];
}sg_watch_info;


typedef struct _CAMERA_SAVE_
{
//	unsigned char path[20];
	int pic_save;
}sg_camera_save;

//短信临时保存
typedef struct _SMS_SAVE_
{
	unsigned char flag;            //保存标志
	unsigned char alphabet_type;   //编码方式
	unsigned short len;            //长度
	unsigned char msg[200];		       //数据
	unsigned char telenum[20];     //号码

}sg_sms_save;

typedef struct _SG_SET_INFO_
{

	char sOwnNo[SG_PHONE_LEN + 1]; 		//车台本机号(终端号/本机手机号)
	char sCenterNo[SG_PHONE_LEN + 1]; 	//中心号码
	char sPoliceNo[SG_PHONE_LEN + 1]; 	//报警号码
	char sHelpNo[SG_PHONE_LEN + 1]; 		//求助号码
	char sListenNo[SG_PHONE_LEN + 1]; 	//中心监听录音电话号码
	char sOneGPSNo[SG_PHONE_LEN + 1]; 	//一键导航通话号码
	char sMedicalNo[SG_PHONE_LEN + 1]; 	//医疗救护电话号码
	char sServiceNo[SG_PHONE_LEN + 1]; 	//维修电话号码
	char sCenterIp[SG_IP_LEN + 3 + 1]; 	//中心IP，加上3个'.'
	char sProxyIp[SG_IP_LEN + 3 + 1]; 	//代理IP，加上3个'.'

	int nCenterPort; 					//中心端口
	int nProxyPort; 					//代理端口

	char sAPN[SG_APN_LEN + 1];			//中心APN地址
	char sGprsUser[SG_GPRS_USER_LEN+1];	//GPRS用户名
	char sGprsPsw[SG_GPRS_PSW_LEN+1];	//GPRS密码
	char bRegisted;						//车台是否已经注册登记0:未登记1:已登记

	int nDisconnectTime;				//断开一段时间后重新连接
	int nResetCnt;

	char bProxyEnable; 					//代理使能1:使用代理0:不使用代理
	char nNetType; 						//通讯方式0:短信1:GPRS
	char nSamplingInflexionRedeem; 		//黑匣子采样拐点补偿0，不补偿；1～180，补偿角度
	char nSamplingType; 				//黑匣子采样方式0x00－不采集0x01－定时采集0x10－定距采集

	int nSamplingInterval;				//黑匣子采样间隔

	int nOverSpeed; 					//超速速度
	int nOverSpeedTime; 				//超速持续时间
	int nReConCnt;						//

	char bOverSpeedEnable;				//超速报警允许1:允许0禁止
	char bRegionEnable;					//区域报警允许1:允许0禁止
	char bLineEnable;					//路线报警允许1:允许0禁止
	char bCallOutDisable;				//呼出禁止0:允许1:禁止

	char bCallInDisable;				//呼入禁止0:允许1:禁止
	char nWatchType; 					//监控方式
	char nInflexionRedeem; 				//拐点补偿0，不补偿；1～180，补偿角度
	char bCompress; 					//压缩发送1:压缩0不压缩

	int nWatchInterval; 				//监控周期
	uint32 nWatchTime; 					//监控时间

	int nGpstTime;                       //GPS定位过滤的时间
	int nGpstDist;                       //GPS里程统计的距离

//	char sAllowAnswer[SG_CALLNUM_SIZE];
//	char sAllowCall[SG_CALLNUM_SIZE];   //呼出限制列表
	char bLockTest;
	char bRelease1;// 串口1的配置
	char bRelease2;// 串口2的配置


	uint32 alarmEnable;			//报警允许字
	uint32 alarmRepeat;			//汇报报警字

	uint32    nIoAlarmMask;               //IO 输出掩码
	int	bIoOutLevel;                    // IO 输出电平: 0 常低, 1 常高
	int bnewSpeedLine;

	int bNewUart4Alarm;					//新标示用来表示有源串口设备类型
	int bnewRegion; 					//区域综合
	uint32 ReserveAlarmEnable; 	//保留报警允许字
	uint32 alarmState;			//状态报警字

	unsigned char sWatchMsgNo[SG_MSG_NO_LEN];			//监控汇报的报文序列号
	unsigned char sMusterMsgNo[SG_MSG_NO_LEN];			//电召抢答的报文序列号
	unsigned char sWatchIntervalMsgNo[SG_MSG_NO_LEN];	//轮询监控的报文序列号
	unsigned char sExtMsgNo[SG_MSG_NO_LEN];	//外设led 监控报文流水号

	char sVersion[60];	//主机版本信息
	unsigned char sCameraMsgNo[SG_MSG_NO_LEN];	//监控汇报的报文序列号


	int nRegionPiont;
	int nLinePiont;

	unsigned int alarmSet;
	uint32 nSmsInterval;					//短信间隔
	uint32 nSmsInterval_data;					//混合模式短信间隔

	int nCameraAlarm;					//摄像头监控标志位
	CAMERA sg_camera[2];

	unsigned char sSafetyPsw[8];	// 导航屏的安全中心密码
	unsigned char sSuperPsw[12];	//超级用户密码
	unsigned char sUserPsw[12];     //手机配置密码
	unsigned char sChangePsw[12];     //手机配置暗码
	char sUserNo[SG_PHONE_LEN + 1]; 	//手机用户号码
	char sSmsAlarmNo1[SG_PHONE_LEN + 1]; 	//短信报警号码1.
	char sSmsAlarmNo2[SG_PHONE_LEN + 1]; 	//短信报警号码2

	int SetEnable;                    //用户手机配置绑定 1:绑定 0:取消
	int SmsEnable;                    //用户手机报警绑定 1:绑定 0:取消

	int devID;  // 透传设备和中心之间通信的外设标识ID号
	int watchid; // 透传设备ID号 1 计价器 2称重传感器
	int nSetTime; // 透传设备透传超时时间 1-60 秒
	int watchbd; // 透传设备波特率
	int bAutoAnswerDisable;				//自动接听 0:手柄通话 1:(默认)免提通道
	int speaker_vol;  // 通话音量
	//新增
	int nTotalVolume;					 //油量总容积,以0.01为单位
	int nAlarmVol; 					   //油量异常变化阀值
	int nkeepTime; 					  //持续时间
	int MaxAD; 							//AD最大值
	int MinAD; 				  //AD	最小值
	int K_AD;				 //发动时的电压上浮百分比
	int upOilQuantity; 				 //上传油量
	int upAD;						//实时上传时的AD值
	int OilFlag;                //油箱类型标志 0:油量和AD值反比 1:油量和AD值正比
	int nRingVol;           // 铃声音量

	int bStopToutEnable;			//超时停车0:禁止1:允许
	int nStopTout;					//触发告警时限
	int bDrvToutEnable;					//超时驾驶
	int nDrvTout;						//超时驾驶时间(秒)

	int nTheftState;           //防盗状态  0:关     1:开
	int bTheftDisable;         //防盗使能  0:允许    1:禁止
	int GPSEnable;             //监控开关  0:开   1:关
	int PNDEnable;             //导航开关  0:开   1:关
	int SmsSndTimer;
	int LedResetFlag;              //串口2LED重启标志: 0不重启  1重启
	int jdqFlag;                   //熄火状态:     0熄火恢复    1熄火
	int ddjdqFlag;                  //掉电是否熄火:   0否   1是
	int ACCGPS;                    //ACC关时是否更新经纬度:  0否  1是
	int JDQ;                       //锁车指令:   0解锁状态   1锁车状态

	int drvTime;						//用于超时驾驶判断的累积时间
	int drvstopTime;					//用于超时驾驶停车累积时间(秒)
	int stopTime;						//用于超时停车累积时间(秒)
	int lastCheckTime;                //上次判定停车的时间
	int nsetregion;
	int nsetline;
	int GpsDevType;        // 1 为ZB导航屏 2为YH导航屏
	int GpsAutoCall;       //导航屏自动接听
	int JJQLev;             //计价器触发电平  0:高  1:低
	int EfsFlag;           //文件读写标志
	int EfsMax;            //盲区保存条数
	int bPictureSave;					//图像保存标志
	int nPictureSaveIndex;				//图像保存文件名索引
	int Heart_Switch;          // 心跳开关，默认关闭
	int Heart_Interval;       // 心跳间隔，默认120s
	int SmsAlarmFlag;        //数据模式下，GPRS不通报警是否发送短信:  0不要  1要
	int GpsPowrFlag;         //GPS电源标志   0关闭  1开启
	sg_camera_save camera_save_list[5];


//	char IoDefineLev; 					//  车门报警更改为自定义报警 触发电平:1 高电平触发 0:低电平触发
	char bStopReport;					//是否停车汇报: 1: 汇报    0: 不汇报
	char bBatteryCheckEnable;  			// 是否电池检测有效 1:有效  0:无效

	char Reserve[220];
	unsigned short checksum;  // CRC16的checksum


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
extern uint32 HhTimeCorrectTimer; // 手柄校时修正

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

extern uint32 SmsSndTimer; 	//sms发送定时器
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

extern sg_send_list *gp_new_list; //新待发报文队列
extern sg_send_list *gp_alarm_list; //报警队列
extern int gn_new_list; //新待发报文队列节点数目
extern int gn_alarm_list; //报警队列节点数目
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


