/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Call.c
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-7-23
  ����������
  �޸ļ�¼�� ����򵥵�MT�Զ������Ĳ��Ժ��� 2007/8/17
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

uint8 CurrCallId = 0;  // ��ǰ����ĵ绰��id��
uint8 listen = 0;
uint8 CallInListen = 0;
extern uint8 ListenFlag;
uint8 ListenTimer;
extern const AUD_RING_DATA_INFO_T g_normal_ring_table[];
/*************************************************************************
******************************�����嶨��**********************************
*************************************************************************/


/****************************************************************
  ��������	MMI_GenDispNumber
  ��  ��  ��������ʾ�ĵ绰����(bcd to str)
  ���������MN_NUMBER_TYPE_E	number_type // ���������
  			uint8             	party_len	// ��ת����bcd����ĳ���
			uint8             	*party_num  // ��ת����bcd������׵�ַ
			uint8				*tele_num	// ת����ϴ洢str���׵�ַ
  			uint8				max_tele_len //!!!!!!!!!����ʾ�ĺ������󳤶� + 2(/0 �ַ�����������)
  �����������
  ��д��  ��������
  �޸ļ�¼������ 2007/8/15
****************************************************************/
uint8 MMI_GenDispNumber(
                         MN_NUMBER_TYPE_E  number_type,
                         uint8             party_len,
                         uint8             *party_num,
						 uint8				*tele_num,
						 uint8				max_tele_len//!!!!!!!!!����ʾ�ĺ������󳤶� + 2(/0 �ַ�����������)
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
  ��������	SG_Call_In_Judge
  ��  ��  ���������Ƶ��ж�
  ���������tele_num �绰���� call_id �绰ID
  �����������
  ��д��  ��������
  �޸ļ�¼������ 2008/9/02
****************************************************************/
static int SG_Call_In_Judge(char *tele_num,int call_id)
{
	// û�к������ƣ�ֱ���˳�����
	if(g_set_info.bCallInDisable == 0)
	{
		SCI_TRACE_LOW("<<<<<<SG_Call_In_Judge:No Call in disable!! %s",tele_num);
		return 1;
	}

	// ���������б�Ϊ�գ����е绰���ܾ�
	if((0 == strcmp(g_state_info.sAllowAnswer,"")) || (&tele_num == 0))
	{
		SCI_TRACE_LOW("<<<<<<SG_Call_In_Judge:All The Number is been reject:%s",tele_num);
		// ֱ�ӹҶ�
		if(MN_RETURN_SUCCESS != MNCALL_DisconnectCall(call_id, PNULL))
		{
			SCI_TRACE_LOW("<<<<<<SG_Call_In_Judge: MNCALL_DisconnectCall return failure");
		}
		return 0;
	}

	// ����ĺ������б��У��������ͨ������ֱ�ӹҶ�
	if(NULL == strstr(g_state_info.sAllowAnswer,(char*)tele_num) || (char *)tele_num == "")
	{
		SCI_TRACE_LOW("<<<<<<SG_Call_In_Judge:The Number is been reject:%s",tele_num);
		// ֱ�ӹҶ�
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
  ��������	CC_CallSetupInd
  ��  ��  ���յ������¼��Ĵ���ĺ���
  ���������DPARAM param
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void CC_CallSetupInd(DPARAM param)
{
	APP_MN_SETUP_IND_T 	*pSig = (APP_MN_SETUP_IND_T*)param;
	uint8          		tele_num[CC_MAX_TELE_NUM_LEN + 2]= {0};
    //xSignalHeaderRec *signal_ptr = NULL;
	// CHECK THE PARA
    SCI_PASSERT(PNULL != pSig, ("<<<<<<CC_CallSetupInd: pSig == PNULL"));


	// bcd��ʽ�ĺ���ת��Ϊstr��ʽ
	MMI_GenDispNumber( pSig->calling_num.number_type,
				pSig->calling_num.num_len,
				(uint8*)pSig->calling_num.party_num,
				tele_num ,
				CC_MAX_TELE_NUM_LEN + 2);

	SCI_TRACE_LOW("<<<<<<Print The Incoming Number:--%s---, g_set_info.sListenNo = %s",tele_num,g_set_info.sListenNo);
	

    // �����������
    if (strlen(g_set_info.sListenNo) != 0         // ������Ϊ�յ�����²��������ģʽ�� 2010.11.03 cojone
		&& 0 == strcmp((char *)tele_num, g_set_info.sListenNo))
    {
        SCI_TRACE_LOW("<<<<<<We are connecting the listen number:%s", g_set_info.sListenNo);
        CallInListen = 1;
        AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
        AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
		AUD_EnableVoiceCodec(1);
        CurrCallId = pSig->call_id;
        SCI_TRACE_LOW("<<<<<Current Call ID is %d>>>>>>>>", CurrCallId);
        ConnectPhoneTimer = 1;
        //SG_CreateSignal(SG_ANS_CALL, 0, &signal_ptr);
        return;
    }
	if(SG_Call_In_Judge((char *)tele_num,pSig->call_id) == 0)
		return;

	SG_Uart_Send_Msg(tele_num,strlen((char*)tele_num),REV_TEL);  // ���ֱ���������֪ͨ


	if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 2)// �ѺƵ�����
	{
		GpsDev_SendIncoming(tele_num);
	}

	if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 1)// չ��������
	{
		ZBGpsDev_SendIncoming(tele_num);
	}

	if(g_set_info.bAutoAnswerDisable) // ��ֹ�Զ�����
	{

		AUD_SetDevMode(AUD_DEV_MODE_HANDHOLD);
		AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_GENERIC_TONE, g_set_info.speaker_vol); // ��������Ϊ�ֱ�ͨ��ʱ����
	}
	else
	{
		AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
		AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_GENERIC_TONE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
	}
#ifndef	_TRACK
	GPIO_SetValue(HF_MUTE,SCI_FALSE);
#endif
	AUD_EnableVoiceCodec( 1 );
	AUD_PlayRing(AUD_SINGLE_TONE_RING,g_normal_ring_table[0].data_len,g_normal_ring_table[0].data_ptr,1000,PNULL);

}


/****************************************************************
  ��������	CC_CallAlertingInd
  ��  ��  ��CCģ��ĳ�ʼ������
  �����������
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void CC_Init_Global(void)
{
	SCI_TRACE_LOW("<<<<<<ccapp.c:enter function CC_Init_Global<<<<<<");
	// ����չ��

}


/****************************************************************
  ��������	CC_CallAlertingInd
  ��  ��  ���յ�������ʾ�¼��Ĵ�����
  ���������DPARAM param
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
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

        //MO,�յ��Զ˷�������ALERT��Ϣ
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
  ��������	CC_OpenAudioTunnel
  ��  ��  ��open speaker and microphone
  ���������BOOLEAN is_input_on
  			BOOLEAN is_output_on
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
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
  ��������	MMI_GenPartyNumber
  ��  ��  ����str��ʽ�ĵ绰���봦���Ϊbcd��ʽ�Ĵ�����
  ���������uint8	*tele_num_ptr	str��ʽ�ĵ绰������׵�ַ
			int16	tele_len		str��ʽ�ĵ绰����ĳ���
  			MMI_PARTY_NUMBER_T	*party_num  ���ɵ�bcd�뼰���ʹ洢�Ľṹ���׵�ַ
  �����������
  ��д��  ��������
  �޸ļ�¼������ 2007/8/25
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
  ��������	CC_ConnectCall
  ��  ��  �����Ⲧ��绰�Ĵ�����
  ���������char* src_tele_num-----str��ʽ�ĵ绰����
  �����������
  ��д��  ��������
  �޸ļ�¼������ 2007/8/20
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

	// ��������ʱ���ر� HF_MUTE �˿�
	if(0 == strcmp(src_tele_num,g_state_info.sListenOutNo))
	{
		SCI_TRACE_LOW("<<<<<<We are connecting the listen number:%s",g_state_info.sListenOutNo);
#ifndef _TRACK
		GPIO_SetValue(HF_MUTE,SCI_TRUE);
#endif
		listen = 1;
//		memset(g_state_info.sListenOutNo,0,sizeof(g_state_info.sListenOutNo));
	}
	else// ������ͨ�绰���� HF_MUTE �˿�
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

	SG_CreateSignal(SG_SEND_CALLOFF,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���

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
	SG_CreateSignal(SG_SEND_CALLOFF,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���

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
		//ʹ���ֱ�ͨ�������Զ�����
		SCI_TRACE_LOW("--------------------------------------------NOT AUTO ANS------------------------");
		return;
	}

	if((g_state_info.user_type == 2) && (g_set_info.GpsAutoCall == 0))// �������Ƿ��Զ�����
	{
		return;
	}

	ConnectPhoneTimer = 0;
	// 10s֮������绰�������绰֮ǰҪstop����
	AUD_StopRing();
	AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
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
		//��������绰
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

//�ز���ϯ�绰
void SG_Call_Back_To()
{
	SG_Set_Cal_CheckSum();
	g_set_info.bAutoAnswerDisable = 0; //����Ϊ����ͨ�� 
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
  ��������	MnCall_EventCallback
  ��  ��  ���绰���ҵ���callback����
  ���������uint32 	event_id  �¼���id
  			uint32 	task_id
  			void*	param_ptr
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
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
        //�յ�����CALL ID�¼��Ĵ����ж��Ƿ�����������б���
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
			AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
		}
		else if(g_set_info.bAutoAnswerDisable) // ��ֹ�Զ�����
		{
			AUD_SetDevMode(AUD_DEV_MODE_HANDHOLD);
			AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
		}
		else
		{
			AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
			AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
		}

		if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 1)
		{
			SG_ZBGPS_ACK(CMD_CALL_OUT, 0x00);
		}

		AUD_EnableVoiceCodec( 1 );
        break;

    case EV_MN_APP_ALERTING_IND_F:
        //�յ�������ʾ�¼��Ĵ���
		SCI_TRACE_LOW("<<<<<<EV_MN_APP_ALERTING_IND_F<<<<<<");
		CC_CallAlertingInd(param_ptr);
        break;

    case EV_MN_APP_SETUP_COMPLETE_IND_F:
        //�յ�������������¼���MT���Ĵ���
  		SCI_TRACE_LOW("<<<<<<EV_MN_APP_SETUP_COMPLETE_IND_F<<<<<<");

        if (CallInListen == 1) // �������
        {
            CallInListen = 0;
            AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
			AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
			PhoneFlag = 2;
            break;
        }
        else if(g_set_info.bAutoAnswerDisable) // ��ֹ�Զ�����
		{
			AUD_SetDevMode(AUD_DEV_MODE_HANDHOLD);
			AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
		}
		else
		{
			AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
			AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
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
        //�յ��ŵ������¼��Ĵ���
        SCI_TRACE_LOW("<<<<<<EV_MN_APP_SYNC_IND_F<<<<<<");
        break;

    case EV_MN_APP_SETUP_COMPLETE_CNF_F:
        //�յ�������������¼���MO���Ĵ���
        SCI_TRACE_LOW("<<<<<<EV_MN_APP_SETUP_COMPLETE_CNF_F<<<<<<");
		SG_Send_CallStatus(CALL_SPEAK); // ���ֱ���������ͨ���е�״̬
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
        //�յ�CALL�ж��¼��Ĵ���
		SCI_TRACE_LOW("<<<<<<EV_MN_APP_CALL_DISCONNECTED_IND_F<<<<<<");
		Call_Discon_Handle();
		g_state_info.SendSMSFlag = 0;

        break;

    case EV_MN_APP_CALL_ERR_IND_F:
        //�յ���������¼��Ĵ���

	    SCI_TRACE_LOW("<<<<<<EV_MN_APP_CALL_ERR_IND_F<<<<<<");
	    Call_Err_Handle();
		g_state_info.SendSMSFlag = 0;

        break;

    case EV_MN_APP_SETUP_IND_F:
		//�յ������¼��Ĵ���
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



