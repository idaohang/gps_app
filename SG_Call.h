/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_Call.h
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-7-23
  内容描述： 
  修改记录：
***************************************************************/
#ifndef _SG_CALL_H_
#define _SG_CALL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sci_types.h"
#include "mn_type.h"

/********************************************************************
*******************************宏定义********************************
********************************************************************/
#define CC_MAX_AUOT_NUM                        10
#define CC_MAX_CALL_NUM      				7 //最多同时存在的CALL的数量
#define CC_MAX_TELE_NUM_LEN					40//最大电话号码的长度
#define CC_EVERY_MINUTE_HINT_NUMBER			55//用来计算是否要发提示音的中间数值
#define CC_INVALID_INDEX					255//非法索引
#define CC_INVALID_ID						255//非法ID
#define CC_TIMER_OUT						1000//通话计时TIMER周期
#define CC_DIAL_TIMER_OUT					10000//拨号TIMER周期
#define CC_DISCONNECT_TIMER_OUT				3000//DISCONNECT TIMER周期
#define CC_VIBRATE_TIMER_OUT                3000    //振动
#define CC_ADJUSTVOLUME_TIMER_OUT           3000    //调节音量
#define CC_HOUR_TO_SECOND					3600//小时换算为秒
#define CC_MINUTE_TO_SECOND					60//分钟换算为秒
#define CC_DISPLAY_TIME_LEN					9//显示通话时间字串的长度
#define CC_INVALID_DTMF_VALUE				0//非法DTMF值
#define CC_EMERGENCY_CALL_NUM				1//紧急呼叫号码个数
#define CC_UNKNOWN_NUM						"Unknown Number"//未知号码字符串
#define CC_UNKNOWN_NUM_LEN					14//未知号码字符串长度
#define CC_MISSED_CALL_LEN					12//" missed call"的长度
#define CC_MISSED_CALL_NUM_LEN				4//missed call的最大数量的字符长度，如:1000个，长度为4

#define CC_MO_CALL_TYPE                     0
#define CC_MT_CALL_TYPE                     1

extern uint8 CurrCallId;
/********************************************************************
*******************************结构体定义****************************
********************************************************************/
typedef struct
{
    BOOLEAN				is_dtmf;//if there is dtmf
    uint8				dtmf_len;//dtmf string len
	uint8				dtmf_str[CC_MAX_TELE_NUM_LEN]; //用来储存分机号码
}CC_DTMF_STATE_T;//DTMF的状态

typedef struct
{
	BOOLEAN			exist_incoming_call;//是否存在INCOMING的CALL
	BOOLEAN			exist_outgoing_call;//是否存在准备呼出的CALL
	BOOLEAN			exist_mpty;//是否存在MPTY
	uint8			exist_active_call_num;//存在ACTIVE的CALL的数量
	uint8			exist_hold_call_num;//存在HOLD的CALL的数量
}CC_EXIST_CALL_T; //当前存在的所有CALL的状态

typedef enum
{
	OTHER_OPERATE,//其它操作
	RELEASE_ALL_ACTIVE_AND_ACCEPT,//释放所有ACTIVE的CALL并接来电
	HOLD_ALL_ACTIVE_AND_ACCEPT,//HOLD所有ACTIVE的CALL并接来电
    BUILD_MPTY_AND_ACCEPT
}CC_OPERATE_WAITING_CALL_E; //用户对来电的操作

typedef struct
{
	BOOLEAN				is_want_mo_call;//是否有想要呼出的CALL
	MN_CALLED_NUMBER_T 	address;//号码的信息
	uint8   			name[CC_MAX_TELE_NUM_LEN + 1];//本通CALL相应的名字
    BOOLEAN             is_name_ucs2;
    uint8               name_len;
	//uint8		number[CC_MAX_TELE_NUM_LEN + 1];//想要呼出的号码
}CC_WANT_MO_CALL_T; //想要呼出的CALL的信息，用于通话中拨新号码

typedef struct
{
    BOOLEAN speaker;            //If the speaker is open
    BOOLEAN microphone;            //If the microphone is open
    BOOLEAN  midi_state;		//saved the state of midi
}CC_AUDIO_STATE_T;

typedef struct
{
	BOOLEAN   is_remote_mpty;//被对方加入一个MPTY
	BOOLEAN   is_remote_split;//被对方SPLIT
	BOOLEAN   is_remote_hold;//被对方HOLD
}CC_OPERATED_BY_REMOTE_T; //被对方的操作

typedef enum
{
	CC_INIT_STATE, //初始状态
	CC_CALLING_STATE,//呼叫状态
	CC_CALL_STATE,//通话状态
	CC_INCOMING_CALL_STATE, //来电状态
	CC_HOLD_STATE,//保持状态
    CC_WAITING_STATE   //@Louis wei 2003-12-18
} CC_CALL_STATE_E; //每通CALL的状态

typedef enum
{
    CC_RELEASE_ALL_NORMAL,      //关闭call的窗口，但不关闭其他窗口，显示释放提示及通话时间
    CC_RELEASE_ALL_PROMPT,      //退回idle，显示释放提示及通话时间
    CC_RELEASE_ALL_NOPROMPT    //关闭call的窗口，但不关闭其他窗口，不显示释放提示及通话时间
}CC_RELEASE_ALL_TYPE_E;

typedef enum
{
    CC_NO_OPER,
    CC_HOLD_OPER, //hold the call
    CC_RETRIEVE_OPER,
    CC_SHUTTLE_OPER,
    CC_BUILDMPTY_OPER, //build mpty
    CC_SPLIT_OPER, //split call
    CC_CONNECT_OPER, //connect call
    CC_RELEASE_OPER
}CC_OPERATED_BY_USER_E;


typedef struct
{
    uint32				start_time;//开始通话时间
    uint32				stop_time;//结束通话时间
}CC_CALL_TIME_T;//通话时间

//char **CC_CALL_RESTRICTION; // 呼出限制列表

typedef enum
{
    CC_RESULT_SUCCESS,//函数调用成功
	CC_RESULT_ERROR//函数调用错误
}CC_RESULT_E;//CC函数的返回结果

typedef struct
{	
	uint8   						call_id;//本通CALL的ID
//wenming feng prd2749 begin
//    BOOLEAN                         is_mtfw_reject; //本通CALL是否正被mtfw中断
    BOOLEAN 				        is_Tch_Assigned;// Tch assigned or not when receive MT call 
	BOOLEAN 				        is_MT_Connected;// MMI send connect or not 
//wenming feng prd2749 end 
	BOOLEAN   						is_in_mpty;//本通CALL是否在一个MPTY中
    uint8                           call_type;
	uint8   						name[CC_MAX_TELE_NUM_LEN + 1];//本通CALL相应的名字
	BOOLEAN					        is_name_ucs2;
	uint8						    name_len;
    uint8                           group_id;   //本通CALL相应的group id，在pb中设置，默认为0
	CC_OPERATED_BY_REMOTE_T   		operated_by_remote;//被对方的操作
	CC_CALL_STATE_E   				call_state;//本通CALL的状态
    CC_OPERATED_BY_USER_E           operated_by_user;
    BOOLEAN                         is_release_all;     //是否是挂断所有电话
    CC_RELEASE_ALL_TYPE_E          release_all_type; //挂断所有电话的原因
	MN_CALLED_NUMBER_T   			address;//号码的信息
	CC_CALL_TIME_T					call_time;//通话起止时间
    uint32                          start_call_time_count;  //起始的定时器时间
}CC_CALL_INFO_T; //每通CALL的信息

typedef struct
{
	uint8							      count_time_id;
	uint8							  	  dial_time_id;
	uint8   								call_number;//当前CALL的总数,1-7
	uint8   								current_call;//当前显示的CALL,0-6
    //@用于记录incoming Call在call_info中的index
    uint8                                   incoming_call_index;//@Louis.wei 2003-12-18
    BOOLEAN                                 need_dtmf;
	uint32									missed_call_num;//未接来电的数量
	uint32									call_time_count;//用来记录定时器循环的次数
	CC_DTMF_STATE_T							dtmf_state;//DTMF的状态
	CC_EXIST_CALL_T   						exist_call;//当前存在的所有CALL的状态
	CC_OPERATE_WAITING_CALL_E				operate_waiting_call;//用户对来电的操作
	CC_WANT_MO_CALL_T						want_mo_call;//想要呼出的CALL的信息，用于通话中拨新号码
	CC_AUDIO_STATE_T						audio_state;//声音信道的状态
	CC_CALL_INFO_T   						call_info[CC_MAX_CALL_NUM];//每通CALL的信息
}CC_CALL_CONTEXT_T;//当前CC模块的总体情况



/********************************************************************
*******************************函数声明******************************
********************************************************************/

/****************************************************************
  函数名：	MnCall_EventCallback
  功  能  ：电话相关业务的callback函数
  输入参数：uint32 	event_id  事件的id
  			uint32 	task_id
  			void*	param_ptr
  输出参数：无
  编写者  ：陈海华
  修改记录：创建 2007/8/15
****************************************************************/
void MnCall_EventCallback( 
								uint32 task_id, //task ID
								uint32 event_id, //event ID
								void *param_ptr
								);

/****************************************************************
  函数名：	CC_CallSetupInd
  功  能  ：收到来电事件的处理的函数
  输入参数：DPARAM param
  输出参数：无
  编写者  ：陈海华
  修改记录：创建 2007/8/15
****************************************************************/
void CC_CallSetupInd(DPARAM param);


/****************************************************************
  函数名：	CC_CallAlertingInd
  功  能  ：CC模块的初始化函数
  输入参数：无
  输出参数：无
  编写者  ：陈海华
  修改记录：创建 2007/8/15
****************************************************************/
void CC_Init_Global(void);


/****************************************************************
  函数名：	CC_CallAlertingInd
  功  能  ：收到振铃提示事件的处理函数
  输入参数：DPARAM param 
  输出参数：无
  编写者  ：陈海华
  修改记录：创建 2007/8/15
****************************************************************/
void CC_CallAlertingInd(DPARAM param);


/****************************************************************
  函数名：	CC_ConnectCall
  功  能  ：向外拨打电话的处理函数
  输入参数：DPARAM param
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2007/8/20
****************************************************************/
CC_RESULT_E CC_ConnectCall(char* src_tele_num);


/****************************************************************
  函数名：	CC_OpenAudioTunnel
  功  能  ：open speaker and microphone
  输入参数：BOOLEAN is_input_on  
  			BOOLEAN is_output_on
  输出参数：无
  编写者  ：陈海华
  修改记录：创建 2007/8/15
****************************************************************/
void CC_OpenAudioTunnel( 
						BOOLEAN is_input_on, 
						BOOLEAN is_output_on
						);


/****************************************************************
  函数名：	MMI_GenPartyNumber
  功  能  ：由str格式的电话号码处理成为bcd格式的处理函数
  输入参数：uint8	*tele_num_ptr	str格式的电话号码的首地址
			int16	tele_len		str格式的电话号码的长度
  			MMI_PARTY_NUMBER_T	*party_num  生成的bcd码及类型存储的结构体首地址
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2007/8/25
****************************************************************/
BOOLEAN MMI_GenPartyNumber(
							 uint8				*tele_num_ptr,
							 int16				tele_len,
							 MMI_PARTY_NUMBER_T *party_num
							 );


/****************************************************************
  函数名：	MMI_GenDispNumber
  功  能  ：产生显示的电话号码(bcd to str)
  输入参数：MN_NUMBER_TYPE_E	number_type // 号码的类型
  			uint8             	party_len	// 待转换的bcd数组的长度
			uint8             	*party_num  // 待转换的bcd数组的首地址
			uint8				*tele_num	// 转换完毕存储str的首地址
  			uint8				max_tele_len //!!!!!!!!!可显示的号码的最大长度 + 2(/0 字符串结束符号)
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2007/8/15
****************************************************************/
uint8 MMI_GenDispNumber(
                         MN_NUMBER_TYPE_E  number_type,
                         uint8             party_len,
                         uint8             *party_num,
						 uint8				*tele_num,
						 uint8				max_tele_len//!!!!!!!!!可显示的号码的最大长度 + 2(/0 字符串结束符号)
						 );





//回拨坐席电话
void SG_Call_Back_To();

#endif //_SG_CALL_H_