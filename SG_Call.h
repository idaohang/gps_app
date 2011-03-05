/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Call.h
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-7-23
  ���������� 
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_CALL_H_
#define _SG_CALL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sci_types.h"
#include "mn_type.h"

/********************************************************************
*******************************�궨��********************************
********************************************************************/
#define CC_MAX_AUOT_NUM                        10
#define CC_MAX_CALL_NUM      				7 //���ͬʱ���ڵ�CALL������
#define CC_MAX_TELE_NUM_LEN					40//���绰����ĳ���
#define CC_EVERY_MINUTE_HINT_NUMBER			55//���������Ƿ�Ҫ����ʾ�����м���ֵ
#define CC_INVALID_INDEX					255//�Ƿ�����
#define CC_INVALID_ID						255//�Ƿ�ID
#define CC_TIMER_OUT						1000//ͨ����ʱTIMER����
#define CC_DIAL_TIMER_OUT					10000//����TIMER����
#define CC_DISCONNECT_TIMER_OUT				3000//DISCONNECT TIMER����
#define CC_VIBRATE_TIMER_OUT                3000    //��
#define CC_ADJUSTVOLUME_TIMER_OUT           3000    //��������
#define CC_HOUR_TO_SECOND					3600//Сʱ����Ϊ��
#define CC_MINUTE_TO_SECOND					60//���ӻ���Ϊ��
#define CC_DISPLAY_TIME_LEN					9//��ʾͨ��ʱ���ִ��ĳ���
#define CC_INVALID_DTMF_VALUE				0//�Ƿ�DTMFֵ
#define CC_EMERGENCY_CALL_NUM				1//�������к������
#define CC_UNKNOWN_NUM						"Unknown Number"//δ֪�����ַ���
#define CC_UNKNOWN_NUM_LEN					14//δ֪�����ַ�������
#define CC_MISSED_CALL_LEN					12//" missed call"�ĳ���
#define CC_MISSED_CALL_NUM_LEN				4//missed call������������ַ����ȣ���:1000��������Ϊ4

#define CC_MO_CALL_TYPE                     0
#define CC_MT_CALL_TYPE                     1

extern uint8 CurrCallId;
/********************************************************************
*******************************�ṹ�嶨��****************************
********************************************************************/
typedef struct
{
    BOOLEAN				is_dtmf;//if there is dtmf
    uint8				dtmf_len;//dtmf string len
	uint8				dtmf_str[CC_MAX_TELE_NUM_LEN]; //��������ֻ�����
}CC_DTMF_STATE_T;//DTMF��״̬

typedef struct
{
	BOOLEAN			exist_incoming_call;//�Ƿ����INCOMING��CALL
	BOOLEAN			exist_outgoing_call;//�Ƿ����׼��������CALL
	BOOLEAN			exist_mpty;//�Ƿ����MPTY
	uint8			exist_active_call_num;//����ACTIVE��CALL������
	uint8			exist_hold_call_num;//����HOLD��CALL������
}CC_EXIST_CALL_T; //��ǰ���ڵ�����CALL��״̬

typedef enum
{
	OTHER_OPERATE,//��������
	RELEASE_ALL_ACTIVE_AND_ACCEPT,//�ͷ�����ACTIVE��CALL��������
	HOLD_ALL_ACTIVE_AND_ACCEPT,//HOLD����ACTIVE��CALL��������
    BUILD_MPTY_AND_ACCEPT
}CC_OPERATE_WAITING_CALL_E; //�û�������Ĳ���

typedef struct
{
	BOOLEAN				is_want_mo_call;//�Ƿ�����Ҫ������CALL
	MN_CALLED_NUMBER_T 	address;//�������Ϣ
	uint8   			name[CC_MAX_TELE_NUM_LEN + 1];//��ͨCALL��Ӧ������
    BOOLEAN             is_name_ucs2;
    uint8               name_len;
	//uint8		number[CC_MAX_TELE_NUM_LEN + 1];//��Ҫ�����ĺ���
}CC_WANT_MO_CALL_T; //��Ҫ������CALL����Ϣ������ͨ���в��º���

typedef struct
{
    BOOLEAN speaker;            //If the speaker is open
    BOOLEAN microphone;            //If the microphone is open
    BOOLEAN  midi_state;		//saved the state of midi
}CC_AUDIO_STATE_T;

typedef struct
{
	BOOLEAN   is_remote_mpty;//���Է�����һ��MPTY
	BOOLEAN   is_remote_split;//���Է�SPLIT
	BOOLEAN   is_remote_hold;//���Է�HOLD
}CC_OPERATED_BY_REMOTE_T; //���Է��Ĳ���

typedef enum
{
	CC_INIT_STATE, //��ʼ״̬
	CC_CALLING_STATE,//����״̬
	CC_CALL_STATE,//ͨ��״̬
	CC_INCOMING_CALL_STATE, //����״̬
	CC_HOLD_STATE,//����״̬
    CC_WAITING_STATE   //@Louis wei 2003-12-18
} CC_CALL_STATE_E; //ÿͨCALL��״̬

typedef enum
{
    CC_RELEASE_ALL_NORMAL,      //�ر�call�Ĵ��ڣ������ر��������ڣ���ʾ�ͷ���ʾ��ͨ��ʱ��
    CC_RELEASE_ALL_PROMPT,      //�˻�idle����ʾ�ͷ���ʾ��ͨ��ʱ��
    CC_RELEASE_ALL_NOPROMPT    //�ر�call�Ĵ��ڣ������ر��������ڣ�����ʾ�ͷ���ʾ��ͨ��ʱ��
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
    uint32				start_time;//��ʼͨ��ʱ��
    uint32				stop_time;//����ͨ��ʱ��
}CC_CALL_TIME_T;//ͨ��ʱ��

//char **CC_CALL_RESTRICTION; // ���������б�

typedef enum
{
    CC_RESULT_SUCCESS,//�������óɹ�
	CC_RESULT_ERROR//�������ô���
}CC_RESULT_E;//CC�����ķ��ؽ��

typedef struct
{	
	uint8   						call_id;//��ͨCALL��ID
//wenming feng prd2749 begin
//    BOOLEAN                         is_mtfw_reject; //��ͨCALL�Ƿ�����mtfw�ж�
    BOOLEAN 				        is_Tch_Assigned;// Tch assigned or not when receive MT call 
	BOOLEAN 				        is_MT_Connected;// MMI send connect or not 
//wenming feng prd2749 end 
	BOOLEAN   						is_in_mpty;//��ͨCALL�Ƿ���һ��MPTY��
    uint8                           call_type;
	uint8   						name[CC_MAX_TELE_NUM_LEN + 1];//��ͨCALL��Ӧ������
	BOOLEAN					        is_name_ucs2;
	uint8						    name_len;
    uint8                           group_id;   //��ͨCALL��Ӧ��group id����pb�����ã�Ĭ��Ϊ0
	CC_OPERATED_BY_REMOTE_T   		operated_by_remote;//���Է��Ĳ���
	CC_CALL_STATE_E   				call_state;//��ͨCALL��״̬
    CC_OPERATED_BY_USER_E           operated_by_user;
    BOOLEAN                         is_release_all;     //�Ƿ��ǹҶ����е绰
    CC_RELEASE_ALL_TYPE_E          release_all_type; //�Ҷ����е绰��ԭ��
	MN_CALLED_NUMBER_T   			address;//�������Ϣ
	CC_CALL_TIME_T					call_time;//ͨ����ֹʱ��
    uint32                          start_call_time_count;  //��ʼ�Ķ�ʱ��ʱ��
}CC_CALL_INFO_T; //ÿͨCALL����Ϣ

typedef struct
{
	uint8							      count_time_id;
	uint8							  	  dial_time_id;
	uint8   								call_number;//��ǰCALL������,1-7
	uint8   								current_call;//��ǰ��ʾ��CALL,0-6
    //@���ڼ�¼incoming Call��call_info�е�index
    uint8                                   incoming_call_index;//@Louis.wei 2003-12-18
    BOOLEAN                                 need_dtmf;
	uint32									missed_call_num;//δ�����������
	uint32									call_time_count;//������¼��ʱ��ѭ���Ĵ���
	CC_DTMF_STATE_T							dtmf_state;//DTMF��״̬
	CC_EXIST_CALL_T   						exist_call;//��ǰ���ڵ�����CALL��״̬
	CC_OPERATE_WAITING_CALL_E				operate_waiting_call;//�û�������Ĳ���
	CC_WANT_MO_CALL_T						want_mo_call;//��Ҫ������CALL����Ϣ������ͨ���в��º���
	CC_AUDIO_STATE_T						audio_state;//�����ŵ���״̬
	CC_CALL_INFO_T   						call_info[CC_MAX_CALL_NUM];//ÿͨCALL����Ϣ
}CC_CALL_CONTEXT_T;//��ǰCCģ����������



/********************************************************************
*******************************��������******************************
********************************************************************/

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
								void *param_ptr
								);

/****************************************************************
  ��������	CC_CallSetupInd
  ��  ��  ���յ������¼��Ĵ���ĺ���
  ���������DPARAM param
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void CC_CallSetupInd(DPARAM param);


/****************************************************************
  ��������	CC_CallAlertingInd
  ��  ��  ��CCģ��ĳ�ʼ������
  �����������
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void CC_Init_Global(void);


/****************************************************************
  ��������	CC_CallAlertingInd
  ��  ��  ���յ�������ʾ�¼��Ĵ�����
  ���������DPARAM param 
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void CC_CallAlertingInd(DPARAM param);


/****************************************************************
  ��������	CC_ConnectCall
  ��  ��  �����Ⲧ��绰�Ĵ�����
  ���������DPARAM param
  �����������
  ��д��  ��������
  �޸ļ�¼������ 2007/8/20
****************************************************************/
CC_RESULT_E CC_ConnectCall(char* src_tele_num);


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
						);


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
							 );


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
						 );





//�ز���ϯ�绰
void SG_Call_Back_To();

#endif //_SG_CALL_H_