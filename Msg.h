/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2008 版权所有
  文件名 ：Msg.h
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2005-9-26
  内容描述：SG2000 数据报文协议定义          
  修改记录：
***************************************************************/
#ifndef _MSG_H_
#define _MSG_H_

typedef char  MsgBool;
typedef char  MsgChar;
typedef short  MsgShort;
typedef int  MsgInt;
typedef unsigned char  MsgUChar;
typedef unsigned long  MsgULong;

#define MSG_TRUE 		1
#define MSG_FALSE 		0
#define MSG_NULL 		(void*)0

#define MSG_NET_SMS 	0x00	//短信通信方式
#define MSG_NET_GPRS 	0x01	//IP通信方式
#define MSG_NET_UNION	0x02	//混合通信方式

#define MSG_SET_OK		0x00	//成功
#define MSG_SET_FAIL	0x01	//失败

//报文头
#define MSG_HEAD 		"&&" 
#define SMS_HEAD        "55"
#define ALLSMS_HEAD     "55*"

#define MSG_HEAD_LEN	 	2 		//报文头长度
#define MSG_OWN_NO_LEN 	6 		//本机号长度
#define MSG_ANS_LEN 		1 		//响应代码长度
#define MSG_CRC_LEN 		2		//校验代码长度
#define MSG_INIT_LEN 		1024*10	//预分配内存长度
#define MSG_NO_LEN 			4		//报文序列号长度
#define MSG_SIGN_LEN 		2		//命令标识长度
#define MSG_GPS_LEN 		31		//GPS数据包长度
//#define MSG_TAG 			"\015"	

//命令类型分类
#define MSG_TYPE_SET 			0x01 //参数设置和参数查询	0x01
#define MSG_TYPE_WATCH 		0x02 //定位监控	0x02
#define MSG_TYPE_MUSTER 		0x03 //调度	0x03
#define MSG_TYPE_SAFETY 		0x04 //安防	0x04
#define MSG_TYPE_FARE 			0x05 //计价器数据采集	0x05
#define MSG_TYPE_INFO 			0x06 //信息点播	0x06
#define MSG_TYPE_NAVIGATION 	0x07 //导航和位置查询	0x07
#define MSG_TYPE_KEY		 	0x08 //密钥交换命令集	0x08
#define MSG_TYPE_CAMERA            0x09 //摄像头
#define MSG_TYPE_EXT_DEV		0x0A//外设命令
//#define MSG_TYPE_VOIVE                0x0B //语音芯片



//操作响应代码
#define MSG_ANS_OK 				0x20	//成功
#define MSG_ANS_OK_NO_ENCRYPT 	0x30	//成功，无压缩无加密
#define MSG_ANS_ZLIB_DES_ERR 	0x91	//解压解密失败
#define MSG_CRC_ERR 			0x92	//CRC校验不通过
#define MSG_DB_ERR 				0x93	//数据库忙或无法打开数据库或操作数据库失败
#define MSG_TERMINAL_ERR 		0x94	//终端验证失败！即终端还未登记过，也就是数据库里没有此终端号的记录
#define MSG_OTHER_ERR 			0x95	//其它未知错误
#define MSG_CONNECT_ERR		0x96	//重新连接中心
#define MSG_USE_DEFAULT_KEY	0xFF	//使用固定密钥

//参数设置和查询命令集（分类代码0x01）
//下行数据
#define MSG_SET_POLICE_N0 					0x01	//设置报警号码
#define MSG_SET_CALL_LIMIT 					0x02 	//设置呼叫限制
#define MSG_SET_ALARM 						0x03 	//配置报警器
#define MSG_SET_HELP_NO 					0x04 	//设置求助号码
#define MSG_SET_SAMPLING_INTERVAL 		0x05 	//设置黑匣子采样间隔
#define MSG_SET_CONFIG_QUERY 				0x06 	//车载终端配置参数查询
#define MSG_SET_DISCONNECT_GPRS 			0x07 	//断开GPRS连接
#define MSG_SET_INIT_GPRS 					0x08 	//设置GPRS初始参数
#define MSG_SET_APN 						0x09 	//配置中心APN地址
#define MSG_SET_USER_PSW 					0x0A 	//配置用户名和密码
#define MSG_SET_TCP 						0x0B 	//配置中心TCP地址
#define MSG_SET_UDP 						0x0C 	//配置中心UDP地址
#define MSG_SET_VCC 						0x0D 	//设置VCC号码
#define MSG_SET_OVERSPEED 					0x0E 	//设置超速
#define MSG_SET_REGION 					0x0F 	//设置区域
#define MSG_SET_RESET 						0x10 	//复位
#define MSG_SET_FARE_UP_TIME 				0x11 	//设置计价器数据上传时间
#define MSG_SET_HAND_INTERVAL 			0x12 	//设置握手间隔时间
#define MSG_SET_FARE_UP_TYPE 				0x13 	//设置计价器数据上传方式
#define MSG_SET_EMPTY_UP_INTERVAL 		0x14 	//设置空重车上报间隔
#define MSG_SET_CENTER_NO 					0x15	//设置调度汇报中心号
#define MSG_SET_PHONE_NO					0x16	//设置电话号码初始参数
#define MSG_SET_LISTEN_NO 					0x17	//设置中心监听录音电话号码
#define MSG_SET_OWN_NO 					0x18	//设置车台本机号
#define MSG_SET_MEDICAL_NO 				0x19 	//设置医疗救护电话号码
#define MSG_SET_SERVICE_NO 				0x1A 	//设置维修电话号码
#define MSG_SET_COMMUNICATION_TYPE 		0x1B 	//设置通讯方式
#define MSG_SET_LINE 						0x1C 	//设置行车路线
#define MSG_SET_CHECK_ITSELF				0x1D 	//自检
#define MSG_SET_PROXY 						0x1E 	//设置代理地址和端口
#define MSG_SET_PROXY_ENABLE				0x1F 	//设置代理使能
#define MSG_SET_UPDATE_APP				0x24 	//升级终端软件
#define MSG_SET_STOP_TIMEOUT				0x21 	//设置超时停车报警应答
#define MSG_SET_DRIVE_TIMEOUT				0x22 	//设置超时驾驶报警应答
#define MSG_SET_CALL_TIMEOUT				0x23 	//设置通话时间限制应答
#define MSG_SET_BLIND_ENABLE				0x24 	//通讯盲区补偿设置
#define MSG_SET_UPDATE_INIT				0x25	//直接升级命令
#define MSG_SET_UPDATE_DATA				0x26	//直接升级报文发送
#define MSG_SET_VER_ASK					0x27	//版本信息查询
#define MSG_SET_SIGN_REPORT				0x29	//设置签到汇报
#define MSG_SET_MOVE_CEN					0x28 // 设置移动中心号码
#define MSG_SET_SELF_DEFINE_ALARM 			0x2C 	//设置自定义报警
#define MSG_SET_SMS_INTERVAL					0x2b		//设置段心间隔
#define MSG_SET_TOTAL_DISTANSE                           0x2D             //修改总里程
#define MSG_SET_LINE_SPEED						0x2e//设置行车路线带超速报警
#define MSG_SET_TEMP_RANGE					0x2f //设置温度上下范围
#define MSG_GET_CURR_TEMP						0x3a //获取当前温度值
#define MSG_SET_INFO_MUSTER					0x3b //设置兴趣点集合
#define MSG_DEL_INFO_MUSTER					0x3c//删除兴趣点
#define MSG_SET_REGION_NEW					0x3d //设置区域报警
#define MSG_SET_ALARM_AD                    0x3e//设置油门报警阀值
#define MSG_SET_OIL_CONSULT                 0x3f//配置油量检测参考点
#define MSG_SET_ONEGPS_NUM                 0x43//配置一键导航号码
#define MSG_SET_SAFETY                     0X44//设置安全中心
#define MSG_SET_PND                        0X46//设置本地导航开关
#define MSG_SET_CHECK_DRIVE				   0x48//驾配查询
#define MSG_CALL_BACK_TO			        0x4f//回拨坐席电话



//上行数据
#define MSG_SET_ANS_POLICE_N0 			0x81	//设置缺省报警手机号应答
#define MSG_SET_ANS_EMPTY_UP_INTERVAL 	0x82	//设置空重车上报间隔
#define MSG_SET_ANS_CALL_LIMIT 			0x83	//设置呼叫限制应答
#define MSG_SET_ANS_ALARM 				0x84	//配置报警器应答
#define MSG_SET_ANS_HELP_NO 				0x85	//设置求助号码应答
#define MSG_SET_ANS_SAMPLING_INTERVAL 	0x86	//设置黑匣子采样间隔应答
#define MSG_SET_ANS_INIT_GPRS 				0x87	//设置GPRS初始参数应答
#define MSG_SET_ANS_APN 					0x88	//配置中心APN地址应答
#define MSG_SET_ANS_USER_PSW 				0x89	//配置用户名和密码应答
#define MSG_SET_ANS_TCP 					0x8A	//配置中心TCP地址应答
#define MSG_SET_ANS_UDP 					0x8B	//配置中心UDP地址应答
#define MSG_SET_ANS_VCC 					0x8C	//设置VCC号码应答
#define MSG_SET_ANS_OVERSPEED 			0x8D	//设置超速应答
#define MSG_SET_ANS_REGION 				0x8E	//设置区域应答
#define MSG_SET_ANS_RESET 					0x8F	//复位应答
#define MSG_SET_ANS_FARE_UP_TIME 			0x90	//设置计价器数据上传时间应答
#define MSG_SET_ANS_CONFIG_QUERY 		0x91	//车载终端配置参数查询应答
#define MSG_SET_ANS_CHECK 					0x92	//自检
#define MSG_SET_ANS_HAND_INTERVAL 		0x93	//设置握手间隔时间应答
#define MSG_SET_ANS_FARE_UP_TYPE 			0x94	//设置计价器数据上传方式应答
#define MSG_SET_ANS_CENTER_NO 			0x95	//设置调度汇报中心号应答
#define MSG_SET_ANS_PHONE_NO				0x96	//设置电话号码初始参数应答
#define MSG_SET_ANS_LISTEN_NO 			0x97	//设置中心监听录音电话号码
#define MSG_SET_ANS_OWN_NO 				0x98	//设置车台本机号应答
#define MSG_SET_ANS_MEDICAL_NO 			0x99	//设置医疗救护电话号码应答
#define MSG_SET_ANS_SERVICE_NO 			0x9A	//设置维修电话号码应答
#define MSG_SET_ANS_LINE 					0x9B	//设置行车路线应答
#define MSG_SET_ANS_COMMUNICATION_TYPE 	0x9C	//设置通讯方式应答
#define MSG_SET_ANS_DISCONNECT_GPRS 		0x9D 	//断开GPRS连接应答
#define MSG_SET_ANS_PROXY 					0x9E 	//设置代理地址和端口
#define MSG_SET_ANS_PROXY_ENABLE			0x9F 	//设置代理使能
#define MSG_SET_ANS_UPDATE_APP			0xAF 	//升级终端软件
#define MSG_SET_ANS_BLIND_ENABLE			0xA1 	//通讯盲区补偿设置
#define MSG_SET_ANS_STOP_TIMEOUT			0xB1 	//设置超时停车报警应答
#define MSG_SET_ANS_DRIVE_TIMEOUT		0xB2 	//设置超时驾驶报警应答
#define MSG_SET_ANS_CALL_TIMEOUT			0xB3 	//设置通话时间限制应答
#define MSG_SET_ANS_VER_ACK				0xAD	//查询版本信息的应答
#define MSG_SET_ANS_UPDATE_DATA				0xAE	//直接升级软件内容请求
#define MSG_SET_ANS_UPDATE_END				0xAF	//升级终端软件结果汇报
#define MSG_SET_ANS_SIGN_REPORT			0xB5	//签到汇报应答
#define MSG_SET_ANS_MOVE_CEN				0xb4 //设置移动中心号码
#define MSG_SET_ANS_SELF_DEFINE_ALARM	0xBC	//自定义报警
#define MSG_SET_ANS_SMS_INTERVAL			0xab		// 短信间隔 
#define MSG_SET_ANS_TOTAL_DISTANSE            0xBD         //修改总里程应答
#define MSG_SET_ANS_LINE_SPEED			0xbe     //新设置线路报警带超速
#define MSG_SET_ANS_TEMP_RANGE			0xbf 	//应答设置温度上下范围
#define MSG_GET_ANS_CURR_TEMP				0xca	//应答当前温度
#define MSG_SET_ANS_INFO_MUSTER				0xcb //设置兴趣点集合应答
#define MSG_DEL_ANS_INFO_MUSTER			0xcc //删除兴趣点应答
#define MSG_SET_ANS_REGION_NEW			0xcd //应答新的区域报警
#define MSG_SET_ANS_ALARM_AD                      0xce //应答报警阀值设置
#define MSG_SET_ANS_OIL_CONSULT                 0xcf //应答油箱参考点设置
#define MSG_SET_ANS_ONEGPS_NUM                 0xd4 //应答一键导航通话设置
#define MSG_SET_ANS_SAFETY                0xd5 //应答安全中心设置
#define MSG_SET_ANS_PND                   0xd6 //应答本地导航
#define MSG_SET_ANS_DRIVE 				  0xda	//驾培查询应答
#define MSG_SET_ANS_CALL_BACK_TO          0xdf	//回拨坐席

//定位监控命令集（分类代码0x02）
//下行数据
#define MSG_WATCH_TIME 			0x01	//定时监控
#define MSG_WATCH_DISTANCE 		0x02	//定距监控
#define MSG_WATCH_AMOUNT 			0x03	//定次监控
#define MSG_WATCH_CANCLE 			0x04	//监控取消
#define MSG_WATCH_HAND_DOWN		0x05	//握手
#define MSG_WATCH_ANS_HAND_UP	0x06	//握手应答
#define MSG_WATCH_INTERVAL_CHCK	0x07	//轮询监控
#define MSG_WATCH_INTERVAL_STOP	0x08	//停止轮询监控
#define MSG_WATCH_BLACK_BOX_ASK  0x0B		//请求黑匣子数据上传
#define MSG_WATCH_BLACK_BOX_STOP_ASK  0x0C		//停止黑匣子数据上传

//上行数据
#define MSG_WATCH_ANS_TIME 		0x81	//定时监控应答
#define MSG_WATCH_ANS_DISTANCE 	0x82	//定距监控应答
#define MSG_WATCH_ANS_AMOUNT 	0x83	//定次监控应答
#define MSG_WATCH_ANS_CANCLE 	0x84	//监控取消应答
#define MSG_WATCH_ANS_HAND_DOWN 0x85	//握手应答
#define MSG_WATCH_HAND_UP 		0x86	//握手
#define MSG_WATCH_ANS_INTERVALCHECK 0x87 //轮询监控应答
#define MSG_WATCH_ANS_INTERVALSTOP 0x88 //停止轮询监控应答
#define MSG_WATCH_SIGN_REPORT	0x89	//签到汇报应答
#define MSG_WATCH_ANS_JJQ			0x8a	//出租车监控应答
#define MSG_WATCH_BLACK_BOX_ACK		0x8B 	//黑匣子数据应答
#define MSG_WATCH_BLACK_BOX_STOP_ACK		0x8C 	//黑匣子数据应答
#define MSG_WATCH_CMP_ANS_TIME		0x8d	//定时监控应答压缩
#define MSG_WATCH_CMP_ANS_DISTANCE	0x8e	//定距监控应答压缩
#define  MSG_WATCH_CMP_ANS_AMOUNT	0x8f	//定次监控应答压缩	
//调度命令集（分类代码0x03）
//下行数据
#define MSG_MUSTER_COMMON 			0x01	//普通信息
#define MSG_MUSTER_TEL 				0x02	//电召消息
#define MSG_MUSTER_PARTICULAR 		0x03	//详细调度消息
#define MSG_MUSTER_PREDEFINE 			0x04	//设置预存消息
#define MSG_MUSTER_CALL 				0x05	//拨打电话号码
#define MSG_ONE_GPS                0x0b      //一键导航 
#define MSG_ADVERTISING_PLAY       0x0c      //走马灯
#define MSG_SMS_PLAY               0x0d      //分流短信
//上行数据
#define MSG_MUSTER_ANS_COMMON 		0x81	//普通消息应答
#define MSG_MUSTER_ANS_TEL 			0x82	//电召消息应答
#define MSG_MUSTER_ANS_PARTICULAR  	0x83	//详细调度消息应答
#define MSG_MUSTER_ANS_FIRST 			0x84	//电召消息抢答
#define MSG_MUSTER_ANS_PREDEFINE 	0x85	//设置预存消息应答
#define MSG_MUSTER_PREDEFINE_UP		0x86	//预存消息上传
#define MSG_MUSTER_ANS_ACK			0x87	//电召应答
#define MSG_SMS_UPLOAD			0x89	//sms上传
#define MSG_ONE_GPS_ACK             0x8b    //一键导航应答
#define MSG_ADVERTISING_PLAY_ACK    0x8c      //走马灯应答
#define MSG_SMS_PLAY_ACK            0x8d      //分流短信应答

//安防命令集（分类代码0x04）
//下行数据
#define MSG_SAFETY_OIL_LOCK				0x01	//断/开油电路闭/开锁
#define MSG_SAFETY_LISTEN 					0x02	//监听
#define MSG_SAFETY_POLICE_CONFIRM 		0x03	//报警确认
#define MSG_SAFETY_CANCLE					0x04	//报警后定位信息发送取消
#define MSG_SAFETY_REGION_CANCLE			0x05	//区域取消
#define MSG_SAFETY_LINE_CANCLE			0x06	//路线取消
#define MSG_SAFETY_OVERSPEED_CANCLE		0x07	//超速取消
#define MSG_SAFETY_CHGPASSWD				0x0a	//修改超级用户密码
#define MSG_SAFETY_PROJECT_LOCK				0x10	//重工锁车
//#define MSG_SAFETY_DRIVE_TOUT_CANCLE		0x09	//超速驾驶取消


//上行数据
#define MSG_SAFETY_ANS_OIL 				0x81	//断/开油电路闭/开锁应答
#define MSG_SAFETY_ANS_POLICE 			0x82	//报警
#define MSG_SAFETY_ANS_CANCLE 			0x83	//报警后定位信息发送取消应答
#define MSG_SAFETY_ANS_REGION_CANCLE	0x84	//区域取消应答
#define MSG_SAFETY_ANS_LINE_CANCLE		0x85	//路线取消应答
#define MSG_SAFETY_ANS_OVERSPEED_CANCLE	0x86	//超速取消应答
#define MSG_SAFETY_ANS_CHGPASSWD		0x8a	//修改超级用户密码应答
#define MSG_SAFETY_ANS_LISTEN		    0x8F	//监听应答
#define MSG_SAFETY_ANS_ALARM					0x8b	//新版本报警code 支持区域id
#define MSG_SAFETY_ANS_LOCK				0x90	//重工锁车应答
//#define MSG_SAFETY_ANS_STOP_TOUT_CANCLE		0x88	//超时停车取消
//#define MSG_SAFETY_ANS_DRIVE_TOUT_CANCLE	0x89	//超速驾驶取消

//信息点播命令集（分类代码0x06）
//下行数据
#define MSG_INFO_MENU 			0x01	//固定信息点播菜单内容
#define MSG_INFO_DATA 			0x02	//固定信息点播
#define MSG_INFO_BROADCAST 	0x03	//广播信息
#define MSG_INFO_CANCLE_ANS 	0x04	//信息点播取消成功应答
//上行数据
#define MSG_INFO_ASK_MENU 	0x81	//请求固定信息点播菜单内容
#define MSG_INFO_ASK 			0x82	//固定信息点播
#define MSG_INFO_CANCLE	 	0x83	//信息点播取消

//导航和位置查询命令集（分类代码0x07）
//下行数据
#define MSG_NAVIGATION_CURRENT_LOCATION 		0x01	//查询当前位置功能
#define MSG_NAVIGATION_INFO_MENU 				0x02	//周边信息分类菜单内容
#define MSG_NAVIGATION_INFO_ROUND 				0x03	//查询周边信息功能
//上行数据
#define MSG_NAVIGATION_QUERY_CURRENT_LOCATION 	0x81	//查询当前位置功能
#define MSG_NAVIGATION_ASK_INFO_MENU 			0x82	//请求周边信息分类菜单内容
#define MSG_NAVIGATION_QUERY_INFO_ROUND 		0x83	//查询周边信息功能

//密钥交换命令集（分类代码0x08）
//下行数据
#define MSG_KEY_WORK	0x01	//工作密钥
#define MSG_KEY_RESET	0x02	//重新交换工作密钥
//上行数据
#define MSG_KEY_ASK 	0x81	//请求工作密钥交换
#define MSG_KEY_ANS_OK 		0x82	//工作密钥成功交换响应


//摄像头命令集（分类代码0x09）
//下行数据
#define MSG_CAMERA_SET_ASK	0x01	//捕获图像帧
#define MSG_CAMERA_STOP_ASK	0x02	//停止传输或重新传输图像帧
#define MSG_CAMERA_REST_ASK	0x03	//摄像头休眠
#define MSG_CAMERA_UP_SAVE  0x04	//上传存储的图片
//上行数据
#define MSG_CAMERA_SET_PACKAGE_ACK	0x85	//捕获图像帧应答(分包传输的应答)
#define MSG_CAMERA_SET_ACK	0x84	//捕获图像帧应答
#define MSG_CAMERA_START_ACK	0x82	//停止或重新传输当前帧应答
#define MSG_CAMERA_REST_ACK	0x83	//摄像头休眠应答
#define MSG_CAMERA_UP_ANS  0x84	//上传存储的图片应答


//外设命令集(分类代码0x0A)
//下行数据
#define MSG_EXT_DEV_SET		0x01

//上行数据
#define MSG_EXT_DEV_REPORT	0x81		//
//语音芯片命令集(0x0B)
//下行数据
#define  MSG_VOICE_SET_COMBIN    0x01    //合成命令
/*
#define  MSG_VOICE_PAU_COMBIN    0X02    //暂停合成
#define  MSG_VOICE_RET_COMBIN    0X03            //恢复合成
#define  MSG_VOICE_STO_COMBIN    0x04    //停止合成
#define  MSG_VOICE_SLE_COMBIN     0X05     //休眠命令
*/
#endif //_MSG_H_





