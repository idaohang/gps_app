/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_Call.c
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-7-23
  内容描述：
  修改记录： 加入简单的MT自动接听的测试函数 2007/8/17
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SG_Set.h"
#include "SG_GPS.h"
#include "SG_Send.h"
#include "SG_Receive.h"
#include "SG_Sms.h"
#include "SG_Call.h"
#include "mn_events.h"
#include "dal_audio.h"
#include "msg.h"
#include "SG_Hh.h"
#include "SG_Ext_Dev.h"

uint8 CurrCallId = 0;  // 当前拨打的电话的id数
uint8 listen = 0;
uint8 CallInListen = 0;
extern uint8 ListenFlag;
uint8 ListenTimer;
extern const AUD_RING_DATA_INFO_T g_normal_ring_table[];
/*************************************************************************
******************************函数体定义**********************************
*************************************************************************/


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
						 )
{

    uint8   offset = 0;
	uint8	len = 0;

	//check the param
    SCI_TRACE_LOW("<<<<<<enter function MMI_GenDispNumber()");
    SCI_PASSERT(NULL != tele_num, ("<<<<<<MMI_GenDispNumber() tele_num == NULL"));
	SCI_PASSERT(NULL != party_num, ("<<<<<<MMI_GenDispNumber() party_num == NULL"));
	SCI_ASSERT( max_tele_len > 2 );

	if(MN_NUM_TYPE_ALPHANUMERIC == number_type)
    {
        MMI_MEMCPY(tele_num, max_tele_len, party_num, party_len, party_len);
        return MIN(max_tele_len, party_len);
    }

    //check the tele_num is internation tele
    if( MN_NUM_TYPE_INTERNATIONAL == number_type )
    {
        offset = 1;
        tele_num[0] = '+';
    }

    if( 0 == party_len )
    {
        SCI_TRACE_LOW("<<<<<<MMI_GenDispNumber: called_num->num_len = 0");
        return (offset);
    }

	len = party_len << 1;
	SCI_ASSERT( len <= (max_tele_len - offset - 1));
    MMI_BcdToStr(PACKED_LSB_FIRST ,party_num, len, (char*)( tele_num + offset ) );

    return (uint8)strlen((char*)tele_num);
}



/****************************************************************
  函数名：	SG_Call_In_Judge
  功  能  ：呼入限制的判断
  输入参数：tele_num 电话号码 call_id 电话ID
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/9/02
****************************************************************/
static int SG_Call_In_Judge(char *tele_num,int call_id)
{
	// 没有呼入限制，直接退出函数
	if(g_set_info.bCallInDisable == 0)
	{
		SCI_TRACE_LOW("<<<<<<SG_Call_In_Judge:No Call in disable!! %s",tele_num);
		return 1;
	}

	// 呼入限制列表为空，所有电话都拒绝
	if((0 == strcmp(g_state_info.sAllowAnswer,"")) || (&tele_num == 0))
	{
		SCI_TRACE_LOW("<<<<<<SG_Call_In_Judge:All The Number is been reject:%s",tele_num);
		// 直接挂断
		if(MN_RETURN_SUCCESS != MNCALL_DisconnectCall(call_id, PNULL))
		{
			SCI_TRACE_LOW("<<<<<<SG_Call_In_Judge: MNCALL_DisconnectCall return failure");
		}
		return 0;
	}

	// 呼入的号码在列表中，则允许接通，否则直接挂断
	if(NULL == strstr(g_state_info.sAllowAnswer,(char*)tele_num) || (char *)tele_num == "")
	{
		SCI_TRACE_LOW("<<<<<<SG_Call_In_Judge:The Number is been reject:%s",tele_num);
		// 直接挂断
		if(MN_RETURN_SUCCESS != MNCALL_DisconnectCall(call_id, PNULL))
		{
			SCI_TRACE_LOW("<<<<<<SG_Call_In_Judge: MNCALL_DisconnectCall return failure");
		}
		return 0;
	}
	else
	{
		SCI_TRACE_LOW("<<<<<<SG_Call_In_Judge:The Number is allow to answer!! %s",tele_num);
		return 1;

	}

}



/****************************************************************
  函数名：	CC_CallSetupInd
  功  能  ：收到来电事件的处理的函数
  输入参数：DPARAM param
  输出参数：无
  编写者  ：陈海华
  修改记录：创建 2007/8/15
****************************************************************/
void CC_CallSetupInd(DPARAM param)
{
	APP_MN_SETUP_IND_T 	*pSig = (APP_MN_SETUP_IND_T*)param;
	uint8          		tele_num[CC_MAX_TELE_NUM_LEN + 2]= {0};
    //xSignalHeaderRec *signal_ptr = NULL;
	// CHECK THE PARA
    SCI_PASSERT(PNULL != pSig, ("<<<<<<CC_CallSetupInd: pSig == PNULL"));


	// bcd格式的号码转换为str格式
	MMI_GenDispNumber( pSig->calling_num.number_type,
				pSig->calling_num.num_len,
				(uint8*)pSig->calling_num.party_num,
				tele_num ,
				CC_MAX_TELE_NUM_LEN + 2);

	SCI_TRACE_LOW("<<<<<<Print The Incoming Number:--%s---, g_set_info.sListenNo = %s",tele_num,g_set_info.sListenNo);
	

    // 呼入监听功能
    if (strlen(g_set_info.sListenNo) != 0         // 监听号为空的情况下不进入监听模式是 2010.11.03 cojone
		&& 0 == strcmp((char *)tele_num, g_set_info.sListenNo))
    {
        SCI_TRACE_LOW("<<<<<<We are connecting the listen number:%s", g_set_info.sListenNo);
        CallInListen = 1;
        AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
        AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // 设置音量为免提时音量
		AUD_EnableVoiceCodec(1);
        CurrCallId = pSig->call_id;
        SCI_TRACE_LOW("<<<<<Current Call ID is %d>>>>>>>>", CurrCallId);
        ConnectPhoneTimer = 1;
        //SG_CreateSignal(SG_ANS_CALL, 0, &signal_ptr);
        return;
    }
	if(SG_Call_In_Judge((char *)tele_num,pSig->call_id) == 0)
		return;

	SG_Uart_Send_Msg(tele_num,strlen((char*)tele_num),REV_TEL);  // 向手柄发出来电通知


	if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 2)// 友浩导航屏
	{
		GpsDev_SendIncoming(tele_num);
	}

	if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 1)// 展博导航屏
	{
		ZBGpsDev_SendIncoming(tele_num);
	}

	if(g_set_info.bAutoAnswerDisable) // 禁止自动接听
	{

		AUD_SetDevMode(AUD_DEV_MODE_HANDHOLD);
		AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_GENERIC_TONE, g_set_info.speaker_vol); // 设置音量为手柄通话时音量
	}
	else
	{
		AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
		AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_GENERIC_TONE, g_set_info.speaker_vol); // 设置音量为免提时音量
	}
#ifndef	_TRACK
	GPIO_SetValue(HF_MUTE,SCI_FALSE);
#endif
	AUD_EnableVoiceCodec( 1 );
	AUD_PlayRing(AUD_SINGLE_TONE_RING,g_normal_ring_table[0].data_len,g_normal_ring_table[0].data_ptr,1000,PNULL);

}


/****************************************************************
  函数名：	CC_CallAlertingInd
  功  能  ：CC模块的初始化函数
  输入参数：无
  输出参数：无
  编写者  ：陈海华
  修改记录：创建 2007/8/15
****************************************************************/
void CC_Init_Global(void)
{
	SCI_TRACE_LOW("<<<<<<ccapp.c:enter function CC_Init_Global<<<<<<");
	// 待扩展中

}


/****************************************************************
  函数名：	CC_CallAlertingInd
  功  能  ：收到振铃提示事件的处理函数
  输入参数：DPARAM param
  输出参数：无
  编写者  ：陈海华
  修改记录：创建 2007/8/15
****************************************************************/
void CC_CallAlertingInd(DPARAM param)
{
    APP_MN_ALERTING_IND_T *pSig = (APP_MN_ALERTING_IND_T*)param;
    unsigned char buf[2];

	//check the param
    SCI_TRACE_LOW("<<<<<<ccapp.c:enter function CC_CallAlertingInd(), call_id = %d, alerting_type = %d", pSig->call_id, pSig->alerting_type);
    SCI_PASSERT(PNULL != pSig, ("<<<<<<ccapp.c: CC_CallAlertingInd() pSig == PNULL"));
	CurrCallId = pSig->call_id;

	buf[0] = 0;
	SG_Uart_Send_Msg(buf, 1, RTN_AUTO_REV);

    switch( pSig->alerting_type )
    {
    	case MN_CALL_IN_BAND_AVAILABLE:
        	break;

        //MO,收到对端发送来的ALERT消息
    	case MN_CALL_REMOTE_USER_ALERTING:
        	break;

        //MT,indicate alerting
    	case MN_CALL_LOCAL_USER_ALERTING:
            SCI_TRACE_LOW("<<<<<<MN_CALL_LOCAL_USER_ALERTING<<<<<<");
                MNCALL_IndicateAlert( pSig->call_id, PNULL );
      		break;

    default:
        SCI_TRACE_LOW("<<<<<<ccapp.c:CC_CallAlertingInd() alerting_type = %d", pSig->alerting_type);
        break;
    }

}

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
                              )
{
    uint8 volume = 0;
    AUD_DEV_MODE_E  mode = AUD_DEV_MODE_HANDFREE;

    SCI_TRACE_LOW("<<<<<<ccapp.c: CC_OpenAudioTunnel() is_input_on = %d, is_output_on = %d",is_input_on,is_output_on);
    //check the current state of microphone and speaker

    volume = 5;

	AUD_SetVolume(mode, AUD_TYPE_VOICE, (uint32)volume); // set the volume

	AUD_EnableVoiceCodec( 1 ); // set voice codec enable

}

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
							 )
{
    uint8   offset   = 0;
    uint8*  tele_num = PNULL;

	SCI_TRACE_LOW("<<<<<<MMI_GenPartyNumber<<<<<<");

	//check the param
	SCI_ASSERT( NULL != tele_num_ptr );
	SCI_ASSERT( NULL != party_num );


    if (0 == tele_len)
    {
        party_num->num_len = 0;
        SCI_TRACE_LOW("<<<<<<MMI_GenPartyNumber  length = 0\n");
        return FALSE;
    }

    tele_num = (uint8*)SCI_ALLOC(tele_len + 1);
    SCI_MEMSET(tele_num, 0, (tele_len + 1));
    MMI_MEMCPY(
        tele_num,
        tele_len + 1,
        tele_num_ptr,
        tele_len,
        tele_len);

    if('+' == tele_num[0])
    {
        party_num->number_type = MN_NUM_TYPE_INTERNATIONAL;
        offset++;
    }
    else
    {
        party_num->number_type = MN_NUM_TYPE_UNKNOW;
    }

    party_num->num_len = ( tele_len - offset + 1 ) /2;
	SCI_ASSERT( party_num->num_len <=  MN_MAX_ADDR_BCD_LEN);
    MMI_StrToBcd( PACKED_LSB_FIRST,
    			(char*)( tele_num + offset),
    			(uint8 *)&(party_num->bcd_num) );

    SCI_FREE(tele_num);

    return TRUE;

}

/****************************************************************
  函数名：	CC_ConnectCall
  功  能  ：向外拨打电话的处理函数
  输入参数：char* src_tele_num-----str格式的电话号码
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2007/8/20
****************************************************************/
CC_RESULT_E CC_ConnectCall(char* src_tele_num)
{
	MN_CALL_TYPE_E	  		call_type = MN_CALL_TYPE_NORMAL;
    MMI_PARTY_NUMBER_T 		party_num;
	MN_CALLED_NUMBER_T		want_to_call_num;

//	uint8 i =0;//,*ptr;//,*p;

	//check the param
    SCI_PASSERT(PNULL != src_tele_num, ("<<<<<<ccapp.c: CC_ConnectCall() tele_num == PNULL"));

    SCI_TRACE_LOW("<<<<<<ccapp.c:CC_ConnectCall() tele_num= %s\n", src_tele_num);

	// 监听呼出时，关闭 HF_MUTE 端口
	if(0 == strcmp(src_tele_num,g_state_info.sListenOutNo))
	{
		SCI_TRACE_LOW("<<<<<<We are connecting the listen number:%s",g_state_info.sListenOutNo);
#ifndef _TRACK
		GPIO_SetValue(HF_MUTE,SCI_TRUE);
#endif
		listen = 1;
//		memset(g_state_info.sListenOutNo,0,sizeof(g_state_info.sListenOutNo));
	}
	else// 拨打普通电话，打开 HF_MUTE 端口
	{
#ifndef _TRACK
		GPIO_SetValue(HF_MUTE,SCI_FALSE);
#endif
	}


    //convert str to BCD
    if( TRUE != MMI_GenPartyNumber((uint8*)src_tele_num, strlen(src_tele_num), &party_num)	)
    {

#ifdef _TRACK
       	if(g_state_info.TestFlag == 1)
		{
			SG_PC_CallOff(0);
		}
#endif /* _TRACK */

        SCI_TRACE_LOW("<<<<<<ccapp.c: CC_GenPartyNumber return failure");
		if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 1)
		{
			SG_ZBGPS_ACK(CMD_CALL_OUT, 0x30);
		}
        return CC_RESULT_ERROR;
    }

	SCI_MEMCPY(
		want_to_call_num.party_num,
		party_num.bcd_num,
		6
		);

	want_to_call_num.number_plan=0;
	want_to_call_num.number_type=0;
	want_to_call_num.num_len = party_num.num_len;

	// connect the call
	if (MN_RETURN_FAILURE == MNCALL_StartCall( MN_CALL_TYPE_NORMAL,
        &want_to_call_num,
        PNULL,
        MN_CALL_CLIR_NETWORK_DECIDE,
        PNULL  ))
    {

#ifdef _TRACK
		if(g_state_info.TestFlag == 1)
		{
			SG_PC_CallOff(0);
		}
#endif /* _TRACK */


		if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 1)
		{
			SG_ZBGPS_ACK(CMD_CALL_OUT, 0x30);
		}
		SCI_TRACE_LOW("<<<<<<ccapp.c:CC_ConnectCall() MNCALL_StartCall return failure");
		return CC_RESULT_ERROR;
	}
	else
	{
		SCI_TRACE_LOW("<<<<<<ccapp.c:CC_ConnectCall():success");
		SG_Send_CallStatus(CALL_OUT);
		return CC_RESULT_SUCCESS;
	}

	return CC_RESULT_SUCCESS;

}

void Call_Err_Handle(void)
{

	xSignalHeaderRec      *signal_ptr = NULL;

	PhoneFlag = 0;
	ConnectPhoneTimer = 0;
	AUD_StopRing();
	AUD_EnableVoiceCodec( 0);
#ifndef _TRACK
		GPIO_SetValue(HF_MUTE, SCI_TRUE);
#endif

	SG_CreateSignal(SG_SEND_CALLOFF,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文

	if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 2)
	{
		GpsDev_SendAnsCall();
	}

#ifdef _TRACK
	if(g_state_info.TestFlag == 1)
	{
		SG_PC_CallOff(2);
	}
#endif /* _TRACK */

}

void Call_Discon_Handle(void)
{

	xSignalHeaderRec      *signal_ptr = NULL;

	PhoneFlag = 0;
	ConnectPhoneTimer = 0;
	GPIO_SetValue(CPU_LED,SCI_FALSE);
	AUD_StopRing();
	AUD_EnableVoiceCodec( 0);
#ifndef _TRACK
		GPIO_SetValue(HF_MUTE, SCI_TRUE);
#endif
	SG_CreateSignal(SG_SEND_CALLOFF,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文

	if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 2)
	{
		GpsDev_SendCalloff();
	}
#ifdef _TRACK
	if(g_state_info.TestFlag == 1)
	{
		SG_PC_CallOff(1);
	}
#endif /* (0) */

}

void SG_Do_Call(){
	unsigned char buf[2];
    int ret =0;

	if(g_set_info.bAutoAnswerDisable == 1 && CallInListen == 0)
	{
		//使用手柄通话，不自动接听
		SCI_TRACE_LOW("--------------------------------------------NOT AUTO ANS------------------------");
		return;
	}

	if((g_state_info.user_type == 2) && (g_set_info.GpsAutoCall == 0))// 导航屏是否自动接听
	{
		return;
	}

	ConnectPhoneTimer = 0;
	// 10s之后接听电话，接听电话之前要stop铃声
	AUD_StopRing();
	AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // 设置音量为免提时音量
#ifndef _TRACK
		GPIO_SetValue(HF_MUTE,SCI_FALSE);
#endif
	AUD_EnableVoiceCodec( 1 );
	ret = MNCALL_ConnectCall(CurrCallId,PNULL);
    SCI_TRACE_LOW("----MNCALL_ConnectCall:RET %d",ret);
	buf[0] = 1;
	SG_Uart_Send_Msg(buf, 1, RTN_AUTO_REV);

}

void SG_Call_Listen()
{

	if(CC_RESULT_SUCCESS != CC_ConnectCall(g_state_info.sListenOutNo))
	{
		SCI_TRACE_LOW("<<<<<<CC_ConnectCall connect ListenNo Failure!!");
	}
	else
	{
		SCI_TRACE_LOW("<<<<<<CC_ConnectCall connect ListenNo Successful!!");
	}

	SCI_TRACE_LOW( "<<<<<<SG_Rcv_Safety_Listen: %s", g_state_info.sListenOutNo);
	memset(g_state_info.sListenOutNo,0,sizeof(g_state_info.sListenOutNo));

}

void SG_Wait_ListenCall(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;
		//拨打监听电话
	if(ListenFlag == 1)
	{
		ListenTimer++;
		if(ListenTimer == 2)
		{
			ListenFlag = 0;
			ListenTimer = 0;
			SG_CreateSignal(SG_CALL_LISTEN,0, &signal_ptr);
		}
	}

}

void SG_Wait_Ans_Call(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	if(ConnectPhoneTimer)
	{
		ConnectPhoneTimer++;
		if(ConnectPhoneTimer >= 12)
		{
			ConnectPhoneTimer = 0;
			SG_CreateSignal(SG_ANS_CALL,0, &signal_ptr);
		}
	}

}

//回拨坐席电话
void SG_Call_Back_To()
{
	SG_Set_Cal_CheckSum();
	g_set_info.bAutoAnswerDisable = 0; //设置为免提通话 
	g_set_info.speaker_vol = 6;	
	SG_Set_Save();
	
	if(CC_RESULT_SUCCESS != CC_ConnectCall(g_state_info.sCallBackToNo))
	{
		SCI_TRACE_LOW("<<<<<<CC_ConnectCall connect CallBackNo Failure!!");
	}
	else
	{
		SCI_TRACE_LOW("<<<<<<CC_ConnectCall connect CallBackNo Successful!!");
	}

	SCI_TRACE_LOW( "<<<<<<SG_Call_Back_To: %s", g_state_info.sCallBackToNo);
	memset(g_state_info.sCallBackToNo,0,sizeof(g_state_info.sCallBackToNo));

}

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
								void* param_ptr
								)

{


   	SCI_ASSERT(PNULL != param_ptr);
	if( PNULL == param_ptr )
	{
		SCI_TRACE_HIGH("<<<<<<MnCall_EventCallback: the ponit of param is empty");
		return;
	}

	switch(event_id)
	{
	/*
	EV_MN_APP_CALL_START_IND_F = (MN_APP_CALL_SERVICE << 8),
	EV_MN_APP_ALERTING_IND_F,
	EV_MN_APP_SETUP_COMPLETE_IND_F,
	EV_MN_APP_SETUP_COMPLETE_CNF_F,
	EV_MN_APP_CALL_DISCONNECTED_IND_F,
	EV_MN_APP_CALL_ERR_IND_F,
	EV_MN_APP_SETUP_IND_F,
	EV_MN_APP_HELD_CNF_F,
	EV_MN_APP_HELD_IND_F,
	EV_MN_APP_START_DTMF_CNF_F,
	EV_MN_APP_STOP_DTMF_CNF_F,
	EV_MN_APP_SYNC_IND_F,
	EV_MN_APP_RETRIEVED_CNF_F,
	EV_MN_APP_RETRIEVED_IND_F,
	EV_MN_APP_ACMMAX_IND_F,
	EV_MN_APP_PROGRESS_IND_F,
	EV_MN_APP_USERTOUSER_IND_F,
	EV_MN_APP_MODIFY_CNF_F,
	EV_MN_APP_MODIFY_IND_F,
	EV_MN_APP_FORWARD_IND_F,
	EV_MN_APP_CALL_WAIT_IND_F,
	EV_MN_APP_CALL_BARRY_IND_F,
	EV_MN_APP_BUILD_MPTY_CNF_F,
	EV_MN_APP_BUILD_MPTY_IND_F,
	EV_MN_APP_SPLIT_MPTY_CNF_F,
	EV_MN_APP_SPLIT_MPTY_IND_F,
	EV_MN_APP_FORWARD_STATUS_IND_F,
	EV_MN_APP_CONNECTING_IND_F,
	EV_MN_APP_DISCONNECTING_IND_F,
	EV_MN_APP_HOLDING_IND_F,
	EV_MN_APP_RETRIEVING_IND_F,
	EV_MN_APP_BUILDING_MPTY_IND_F,
	EV_MN_APP_SPLITTING_MPTY_IND_F,
       EV_MN_APP_CALL_READY_IND_F,
       EV_MN_APP_DISC_INFO_IND_F,
	MAX_MN_APP_CALL_EVENTS_NUM,
	*/
    case EV_MN_APP_CALL_START_IND_F:
        //收到分配CALL ID事件的处理，判断是否在允许接入列表中
        SCI_TRACE_LOW("<<<<<<EV_MN_APP_CALL_START_IND_F<<<<<<");

		if(PhoneAllow==1)
		{
			PhoneAllow = 0;
			SG_Send_CallStatus(CAll_OFF);
			PhoneFlag = 0;
			if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 1)
			{
				SG_ZBGPS_ACK(CMD_CALL_OUT, 0x01);
			}
			return;
		}

		PhoneFlag = 1;
		g_state_info.SendSMSFlag = 1;

		if(listen == 1)
		{
			listen = 0;
			AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
			AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // 设置音量为免提时音量
		}
		else if(g_set_info.bAutoAnswerDisable) // 禁止自动接听
		{
			AUD_SetDevMode(AUD_DEV_MODE_HANDHOLD);
			AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // 设置音量为免提时音量
		}
		else
		{
			AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
			AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // 设置音量为免提时音量
		}

		if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 1)
		{
			SG_ZBGPS_ACK(CMD_CALL_OUT, 0x00);
		}

		AUD_EnableVoiceCodec( 1 );
        break;

    case EV_MN_APP_ALERTING_IND_F:
        //收到振铃提示事件的处理
		SCI_TRACE_LOW("<<<<<<EV_MN_APP_ALERTING_IND_F<<<<<<");
		CC_CallAlertingInd(param_ptr);
        break;

    case EV_MN_APP_SETUP_COMPLETE_IND_F:
        //收到建立连接完成事件（MT）的处理
  		SCI_TRACE_LOW("<<<<<<EV_MN_APP_SETUP_COMPLETE_IND_F<<<<<<");

        if (CallInListen == 1) // 呼入监听
        {
            CallInListen = 0;
            AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
			AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // 设置音量为免提时音量
			PhoneFlag = 2;
            break;
        }
        else if(g_set_info.bAutoAnswerDisable) // 禁止自动接听
		{
			AUD_SetDevMode(AUD_DEV_MODE_HANDHOLD);
			AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // 设置音量为免提时音量
		}
		else
		{
			AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
			AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // 设置音量为免提时音量
		}

		if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 2)
		{
			GpsDev_SendAnsCall();
		}
		SG_Send_CallStatus(CALL_SPEAK);

		if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 1)
		{
			ZBGpsDev_SendAnsCall();
		}
		PhoneFlag = 2;

        break;

    case EV_MN_APP_SYNC_IND_F:
        //收到信道分配事件的处理
        SCI_TRACE_LOW("<<<<<<EV_MN_APP_SYNC_IND_F<<<<<<");
        break;

    case EV_MN_APP_SETUP_COMPLETE_CNF_F:
        //收到建立连接完成事件（MO）的处理
        SCI_TRACE_LOW("<<<<<<EV_MN_APP_SETUP_COMPLETE_CNF_F<<<<<<");
		SG_Send_CallStatus(CALL_SPEAK); // 向手柄发送正在通话中的状态
		PhoneFlag = 2;

		if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 2)
		{
			GpsDev_SendAnsCall();
		}
		if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 1)
		{
			ZBGpsDev_SendAnsCall();
		}
        break;

    case EV_MN_APP_CALL_DISCONNECTED_IND_F:
        //收到CALL中断事件的处理
		SCI_TRACE_LOW("<<<<<<EV_MN_APP_CALL_DISCONNECTED_IND_F<<<<<<");
		Call_Discon_Handle();
		g_state_info.SendSMSFlag = 0;

        break;

    case EV_MN_APP_CALL_ERR_IND_F:
        //收到网络错误事件的处理

	    SCI_TRACE_LOW("<<<<<<EV_MN_APP_CALL_ERR_IND_F<<<<<<");
	    Call_Err_Handle();
		g_state_info.SendSMSFlag = 0;

        break;

    case EV_MN_APP_SETUP_IND_F:
		//收到来电事件的处理
    	SCI_TRACE_LOW("<<<<<<<EV_MN_APP_SETUP_IND_F<<<<<<");
		ConnectPhoneTimer = 1;
		PhoneFlag = 1;
		g_state_info.SendSMSFlag = 1;
		CC_CallSetupInd(param_ptr);

        break;

    case EV_MN_APP_HELD_CNF_F:
        //receive the cofirm about hold call
        SCI_TRACE_LOW("<<<<<<EV_MN_APP_HELD_CNF_F<<<<<<");
        break;
    default:
        SCI_TRACE_LOW("<<<<<<ccapp.c: CC_HandlePsMsg() CC isn't process, msg_id = %d", event_id);
        break;

    }

}



