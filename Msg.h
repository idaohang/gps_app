/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2008 ��Ȩ����
  �ļ��� ��Msg.h
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2005-9-26
  ����������SG2000 ���ݱ���Э�鶨��          
  �޸ļ�¼��
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

#define MSG_NET_SMS 	0x00	//����ͨ�ŷ�ʽ
#define MSG_NET_GPRS 	0x01	//IPͨ�ŷ�ʽ
#define MSG_NET_UNION	0x02	//���ͨ�ŷ�ʽ

#define MSG_SET_OK		0x00	//�ɹ�
#define MSG_SET_FAIL	0x01	//ʧ��

//����ͷ
#define MSG_HEAD 		"&&" 
#define SMS_HEAD        "55"
#define ALLSMS_HEAD     "55*"

#define MSG_HEAD_LEN	 	2 		//����ͷ����
#define MSG_OWN_NO_LEN 	6 		//�����ų���
#define MSG_ANS_LEN 		1 		//��Ӧ���볤��
#define MSG_CRC_LEN 		2		//У����볤��
#define MSG_INIT_LEN 		1024*10	//Ԥ�����ڴ泤��
#define MSG_NO_LEN 			4		//�������кų���
#define MSG_SIGN_LEN 		2		//�����ʶ����
#define MSG_GPS_LEN 		31		//GPS���ݰ�����
//#define MSG_TAG 			"\015"	

//�������ͷ���
#define MSG_TYPE_SET 			0x01 //�������úͲ�����ѯ	0x01
#define MSG_TYPE_WATCH 		0x02 //��λ���	0x02
#define MSG_TYPE_MUSTER 		0x03 //����	0x03
#define MSG_TYPE_SAFETY 		0x04 //����	0x04
#define MSG_TYPE_FARE 			0x05 //�Ƽ������ݲɼ�	0x05
#define MSG_TYPE_INFO 			0x06 //��Ϣ�㲥	0x06
#define MSG_TYPE_NAVIGATION 	0x07 //������λ�ò�ѯ	0x07
#define MSG_TYPE_KEY		 	0x08 //��Կ�������	0x08
#define MSG_TYPE_CAMERA            0x09 //����ͷ
#define MSG_TYPE_EXT_DEV		0x0A//��������
//#define MSG_TYPE_VOIVE                0x0B //����оƬ



//������Ӧ����
#define MSG_ANS_OK 				0x20	//�ɹ�
#define MSG_ANS_OK_NO_ENCRYPT 	0x30	//�ɹ�����ѹ���޼���
#define MSG_ANS_ZLIB_DES_ERR 	0x91	//��ѹ����ʧ��
#define MSG_CRC_ERR 			0x92	//CRCУ�鲻ͨ��
#define MSG_DB_ERR 				0x93	//���ݿ�æ���޷������ݿ��������ݿ�ʧ��
#define MSG_TERMINAL_ERR 		0x94	//�ն���֤ʧ�ܣ����ն˻�δ�Ǽǹ���Ҳ�������ݿ���û�д��ն˺ŵļ�¼
#define MSG_OTHER_ERR 			0x95	//����δ֪����
#define MSG_CONNECT_ERR		0x96	//������������
#define MSG_USE_DEFAULT_KEY	0xFF	//ʹ�ù̶���Կ

//�������úͲ�ѯ������������0x01��
//��������
#define MSG_SET_POLICE_N0 					0x01	//���ñ�������
#define MSG_SET_CALL_LIMIT 					0x02 	//���ú�������
#define MSG_SET_ALARM 						0x03 	//���ñ�����
#define MSG_SET_HELP_NO 					0x04 	//������������
#define MSG_SET_SAMPLING_INTERVAL 		0x05 	//���ú�ϻ�Ӳ������
#define MSG_SET_CONFIG_QUERY 				0x06 	//�����ն����ò�����ѯ
#define MSG_SET_DISCONNECT_GPRS 			0x07 	//�Ͽ�GPRS����
#define MSG_SET_INIT_GPRS 					0x08 	//����GPRS��ʼ����
#define MSG_SET_APN 						0x09 	//��������APN��ַ
#define MSG_SET_USER_PSW 					0x0A 	//�����û���������
#define MSG_SET_TCP 						0x0B 	//��������TCP��ַ
#define MSG_SET_UDP 						0x0C 	//��������UDP��ַ
#define MSG_SET_VCC 						0x0D 	//����VCC����
#define MSG_SET_OVERSPEED 					0x0E 	//���ó���
#define MSG_SET_REGION 					0x0F 	//��������
#define MSG_SET_RESET 						0x10 	//��λ
#define MSG_SET_FARE_UP_TIME 				0x11 	//���üƼ��������ϴ�ʱ��
#define MSG_SET_HAND_INTERVAL 			0x12 	//�������ּ��ʱ��
#define MSG_SET_FARE_UP_TYPE 				0x13 	//���üƼ��������ϴ���ʽ
#define MSG_SET_EMPTY_UP_INTERVAL 		0x14 	//���ÿ��س��ϱ����
#define MSG_SET_CENTER_NO 					0x15	//���õ��Ȼ㱨���ĺ�
#define MSG_SET_PHONE_NO					0x16	//���õ绰�����ʼ����
#define MSG_SET_LISTEN_NO 					0x17	//�������ļ���¼���绰����
#define MSG_SET_OWN_NO 					0x18	//���ó�̨������
#define MSG_SET_MEDICAL_NO 				0x19 	//����ҽ�ƾȻ��绰����
#define MSG_SET_SERVICE_NO 				0x1A 	//����ά�޵绰����
#define MSG_SET_COMMUNICATION_TYPE 		0x1B 	//����ͨѶ��ʽ
#define MSG_SET_LINE 						0x1C 	//�����г�·��
#define MSG_SET_CHECK_ITSELF				0x1D 	//�Լ�
#define MSG_SET_PROXY 						0x1E 	//���ô����ַ�Ͷ˿�
#define MSG_SET_PROXY_ENABLE				0x1F 	//���ô���ʹ��
#define MSG_SET_UPDATE_APP				0x24 	//�����ն����
#define MSG_SET_STOP_TIMEOUT				0x21 	//���ó�ʱͣ������Ӧ��
#define MSG_SET_DRIVE_TIMEOUT				0x22 	//���ó�ʱ��ʻ����Ӧ��
#define MSG_SET_CALL_TIMEOUT				0x23 	//����ͨ��ʱ������Ӧ��
#define MSG_SET_BLIND_ENABLE				0x24 	//ͨѶä����������
#define MSG_SET_UPDATE_INIT				0x25	//ֱ����������
#define MSG_SET_UPDATE_DATA				0x26	//ֱ���������ķ���
#define MSG_SET_VER_ASK					0x27	//�汾��Ϣ��ѯ
#define MSG_SET_SIGN_REPORT				0x29	//����ǩ���㱨
#define MSG_SET_MOVE_CEN					0x28 // �����ƶ����ĺ���
#define MSG_SET_SELF_DEFINE_ALARM 			0x2C 	//�����Զ��屨��
#define MSG_SET_SMS_INTERVAL					0x2b		//���ö��ļ��
#define MSG_SET_TOTAL_DISTANSE                           0x2D             //�޸������
#define MSG_SET_LINE_SPEED						0x2e//�����г�·�ߴ����ٱ���
#define MSG_SET_TEMP_RANGE					0x2f //�����¶����·�Χ
#define MSG_GET_CURR_TEMP						0x3a //��ȡ��ǰ�¶�ֵ
#define MSG_SET_INFO_MUSTER					0x3b //������Ȥ�㼯��
#define MSG_DEL_INFO_MUSTER					0x3c//ɾ����Ȥ��
#define MSG_SET_REGION_NEW					0x3d //�������򱨾�
#define MSG_SET_ALARM_AD                    0x3e//�������ű�����ֵ
#define MSG_SET_OIL_CONSULT                 0x3f//�����������ο���
#define MSG_SET_ONEGPS_NUM                 0x43//����һ����������
#define MSG_SET_SAFETY                     0X44//���ð�ȫ����
#define MSG_SET_PND                        0X46//���ñ��ص�������
#define MSG_SET_CHECK_DRIVE				   0x48//�����ѯ
#define MSG_CALL_BACK_TO			        0x4f//�ز���ϯ�绰



//��������
#define MSG_SET_ANS_POLICE_N0 			0x81	//����ȱʡ�����ֻ���Ӧ��
#define MSG_SET_ANS_EMPTY_UP_INTERVAL 	0x82	//���ÿ��س��ϱ����
#define MSG_SET_ANS_CALL_LIMIT 			0x83	//���ú�������Ӧ��
#define MSG_SET_ANS_ALARM 				0x84	//���ñ�����Ӧ��
#define MSG_SET_ANS_HELP_NO 				0x85	//������������Ӧ��
#define MSG_SET_ANS_SAMPLING_INTERVAL 	0x86	//���ú�ϻ�Ӳ������Ӧ��
#define MSG_SET_ANS_INIT_GPRS 				0x87	//����GPRS��ʼ����Ӧ��
#define MSG_SET_ANS_APN 					0x88	//��������APN��ַӦ��
#define MSG_SET_ANS_USER_PSW 				0x89	//�����û���������Ӧ��
#define MSG_SET_ANS_TCP 					0x8A	//��������TCP��ַӦ��
#define MSG_SET_ANS_UDP 					0x8B	//��������UDP��ַӦ��
#define MSG_SET_ANS_VCC 					0x8C	//����VCC����Ӧ��
#define MSG_SET_ANS_OVERSPEED 			0x8D	//���ó���Ӧ��
#define MSG_SET_ANS_REGION 				0x8E	//��������Ӧ��
#define MSG_SET_ANS_RESET 					0x8F	//��λӦ��
#define MSG_SET_ANS_FARE_UP_TIME 			0x90	//���üƼ��������ϴ�ʱ��Ӧ��
#define MSG_SET_ANS_CONFIG_QUERY 		0x91	//�����ն����ò�����ѯӦ��
#define MSG_SET_ANS_CHECK 					0x92	//�Լ�
#define MSG_SET_ANS_HAND_INTERVAL 		0x93	//�������ּ��ʱ��Ӧ��
#define MSG_SET_ANS_FARE_UP_TYPE 			0x94	//���üƼ��������ϴ���ʽӦ��
#define MSG_SET_ANS_CENTER_NO 			0x95	//���õ��Ȼ㱨���ĺ�Ӧ��
#define MSG_SET_ANS_PHONE_NO				0x96	//���õ绰�����ʼ����Ӧ��
#define MSG_SET_ANS_LISTEN_NO 			0x97	//�������ļ���¼���绰����
#define MSG_SET_ANS_OWN_NO 				0x98	//���ó�̨������Ӧ��
#define MSG_SET_ANS_MEDICAL_NO 			0x99	//����ҽ�ƾȻ��绰����Ӧ��
#define MSG_SET_ANS_SERVICE_NO 			0x9A	//����ά�޵绰����Ӧ��
#define MSG_SET_ANS_LINE 					0x9B	//�����г�·��Ӧ��
#define MSG_SET_ANS_COMMUNICATION_TYPE 	0x9C	//����ͨѶ��ʽӦ��
#define MSG_SET_ANS_DISCONNECT_GPRS 		0x9D 	//�Ͽ�GPRS����Ӧ��
#define MSG_SET_ANS_PROXY 					0x9E 	//���ô����ַ�Ͷ˿�
#define MSG_SET_ANS_PROXY_ENABLE			0x9F 	//���ô���ʹ��
#define MSG_SET_ANS_UPDATE_APP			0xAF 	//�����ն����
#define MSG_SET_ANS_BLIND_ENABLE			0xA1 	//ͨѶä����������
#define MSG_SET_ANS_STOP_TIMEOUT			0xB1 	//���ó�ʱͣ������Ӧ��
#define MSG_SET_ANS_DRIVE_TIMEOUT		0xB2 	//���ó�ʱ��ʻ����Ӧ��
#define MSG_SET_ANS_CALL_TIMEOUT			0xB3 	//����ͨ��ʱ������Ӧ��
#define MSG_SET_ANS_VER_ACK				0xAD	//��ѯ�汾��Ϣ��Ӧ��
#define MSG_SET_ANS_UPDATE_DATA				0xAE	//ֱ�����������������
#define MSG_SET_ANS_UPDATE_END				0xAF	//�����ն��������㱨
#define MSG_SET_ANS_SIGN_REPORT			0xB5	//ǩ���㱨Ӧ��
#define MSG_SET_ANS_MOVE_CEN				0xb4 //�����ƶ����ĺ���
#define MSG_SET_ANS_SELF_DEFINE_ALARM	0xBC	//�Զ��屨��
#define MSG_SET_ANS_SMS_INTERVAL			0xab		// ���ż�� 
#define MSG_SET_ANS_TOTAL_DISTANSE            0xBD         //�޸������Ӧ��
#define MSG_SET_ANS_LINE_SPEED			0xbe     //��������·����������
#define MSG_SET_ANS_TEMP_RANGE			0xbf 	//Ӧ�������¶����·�Χ
#define MSG_GET_ANS_CURR_TEMP				0xca	//Ӧ��ǰ�¶�
#define MSG_SET_ANS_INFO_MUSTER				0xcb //������Ȥ�㼯��Ӧ��
#define MSG_DEL_ANS_INFO_MUSTER			0xcc //ɾ����Ȥ��Ӧ��
#define MSG_SET_ANS_REGION_NEW			0xcd //Ӧ���µ����򱨾�
#define MSG_SET_ANS_ALARM_AD                      0xce //Ӧ�𱨾���ֵ����
#define MSG_SET_ANS_OIL_CONSULT                 0xcf //Ӧ������ο�������
#define MSG_SET_ANS_ONEGPS_NUM                 0xd4 //Ӧ��һ������ͨ������
#define MSG_SET_ANS_SAFETY                0xd5 //Ӧ��ȫ��������
#define MSG_SET_ANS_PND                   0xd6 //Ӧ�𱾵ص���
#define MSG_SET_ANS_DRIVE 				  0xda	//�����ѯӦ��
#define MSG_SET_ANS_CALL_BACK_TO          0xdf	//�ز���ϯ

//��λ���������������0x02��
//��������
#define MSG_WATCH_TIME 			0x01	//��ʱ���
#define MSG_WATCH_DISTANCE 		0x02	//������
#define MSG_WATCH_AMOUNT 			0x03	//���μ��
#define MSG_WATCH_CANCLE 			0x04	//���ȡ��
#define MSG_WATCH_HAND_DOWN		0x05	//����
#define MSG_WATCH_ANS_HAND_UP	0x06	//����Ӧ��
#define MSG_WATCH_INTERVAL_CHCK	0x07	//��ѯ���
#define MSG_WATCH_INTERVAL_STOP	0x08	//ֹͣ��ѯ���
#define MSG_WATCH_BLACK_BOX_ASK  0x0B		//�����ϻ�������ϴ�
#define MSG_WATCH_BLACK_BOX_STOP_ASK  0x0C		//ֹͣ��ϻ�������ϴ�

//��������
#define MSG_WATCH_ANS_TIME 		0x81	//��ʱ���Ӧ��
#define MSG_WATCH_ANS_DISTANCE 	0x82	//������Ӧ��
#define MSG_WATCH_ANS_AMOUNT 	0x83	//���μ��Ӧ��
#define MSG_WATCH_ANS_CANCLE 	0x84	//���ȡ��Ӧ��
#define MSG_WATCH_ANS_HAND_DOWN 0x85	//����Ӧ��
#define MSG_WATCH_HAND_UP 		0x86	//����
#define MSG_WATCH_ANS_INTERVALCHECK 0x87 //��ѯ���Ӧ��
#define MSG_WATCH_ANS_INTERVALSTOP 0x88 //ֹͣ��ѯ���Ӧ��
#define MSG_WATCH_SIGN_REPORT	0x89	//ǩ���㱨Ӧ��
#define MSG_WATCH_ANS_JJQ			0x8a	//���⳵���Ӧ��
#define MSG_WATCH_BLACK_BOX_ACK		0x8B 	//��ϻ������Ӧ��
#define MSG_WATCH_BLACK_BOX_STOP_ACK		0x8C 	//��ϻ������Ӧ��
#define MSG_WATCH_CMP_ANS_TIME		0x8d	//��ʱ���Ӧ��ѹ��
#define MSG_WATCH_CMP_ANS_DISTANCE	0x8e	//������Ӧ��ѹ��
#define  MSG_WATCH_CMP_ANS_AMOUNT	0x8f	//���μ��Ӧ��ѹ��	
//����������������0x03��
//��������
#define MSG_MUSTER_COMMON 			0x01	//��ͨ��Ϣ
#define MSG_MUSTER_TEL 				0x02	//������Ϣ
#define MSG_MUSTER_PARTICULAR 		0x03	//��ϸ������Ϣ
#define MSG_MUSTER_PREDEFINE 			0x04	//����Ԥ����Ϣ
#define MSG_MUSTER_CALL 				0x05	//����绰����
#define MSG_ONE_GPS                0x0b      //һ������ 
#define MSG_ADVERTISING_PLAY       0x0c      //�����
#define MSG_SMS_PLAY               0x0d      //��������
//��������
#define MSG_MUSTER_ANS_COMMON 		0x81	//��ͨ��ϢӦ��
#define MSG_MUSTER_ANS_TEL 			0x82	//������ϢӦ��
#define MSG_MUSTER_ANS_PARTICULAR  	0x83	//��ϸ������ϢӦ��
#define MSG_MUSTER_ANS_FIRST 			0x84	//������Ϣ����
#define MSG_MUSTER_ANS_PREDEFINE 	0x85	//����Ԥ����ϢӦ��
#define MSG_MUSTER_PREDEFINE_UP		0x86	//Ԥ����Ϣ�ϴ�
#define MSG_MUSTER_ANS_ACK			0x87	//����Ӧ��
#define MSG_SMS_UPLOAD			0x89	//sms�ϴ�
#define MSG_ONE_GPS_ACK             0x8b    //һ������Ӧ��
#define MSG_ADVERTISING_PLAY_ACK    0x8c      //�����Ӧ��
#define MSG_SMS_PLAY_ACK            0x8d      //��������Ӧ��

//����������������0x04��
//��������
#define MSG_SAFETY_OIL_LOCK				0x01	//��/���͵�·��/����
#define MSG_SAFETY_LISTEN 					0x02	//����
#define MSG_SAFETY_POLICE_CONFIRM 		0x03	//����ȷ��
#define MSG_SAFETY_CANCLE					0x04	//������λ��Ϣ����ȡ��
#define MSG_SAFETY_REGION_CANCLE			0x05	//����ȡ��
#define MSG_SAFETY_LINE_CANCLE			0x06	//·��ȡ��
#define MSG_SAFETY_OVERSPEED_CANCLE		0x07	//����ȡ��
#define MSG_SAFETY_CHGPASSWD				0x0a	//�޸ĳ����û�����
#define MSG_SAFETY_PROJECT_LOCK				0x10	//�ع�����
//#define MSG_SAFETY_DRIVE_TOUT_CANCLE		0x09	//���ټ�ʻȡ��


//��������
#define MSG_SAFETY_ANS_OIL 				0x81	//��/���͵�·��/����Ӧ��
#define MSG_SAFETY_ANS_POLICE 			0x82	//����
#define MSG_SAFETY_ANS_CANCLE 			0x83	//������λ��Ϣ����ȡ��Ӧ��
#define MSG_SAFETY_ANS_REGION_CANCLE	0x84	//����ȡ��Ӧ��
#define MSG_SAFETY_ANS_LINE_CANCLE		0x85	//·��ȡ��Ӧ��
#define MSG_SAFETY_ANS_OVERSPEED_CANCLE	0x86	//����ȡ��Ӧ��
#define MSG_SAFETY_ANS_CHGPASSWD		0x8a	//�޸ĳ����û�����Ӧ��
#define MSG_SAFETY_ANS_LISTEN		    0x8F	//����Ӧ��
#define MSG_SAFETY_ANS_ALARM					0x8b	//�°汾����code ֧������id
#define MSG_SAFETY_ANS_LOCK				0x90	//�ع�����Ӧ��
//#define MSG_SAFETY_ANS_STOP_TOUT_CANCLE		0x88	//��ʱͣ��ȡ��
//#define MSG_SAFETY_ANS_DRIVE_TOUT_CANCLE	0x89	//���ټ�ʻȡ��

//��Ϣ�㲥������������0x06��
//��������
#define MSG_INFO_MENU 			0x01	//�̶���Ϣ�㲥�˵�����
#define MSG_INFO_DATA 			0x02	//�̶���Ϣ�㲥
#define MSG_INFO_BROADCAST 	0x03	//�㲥��Ϣ
#define MSG_INFO_CANCLE_ANS 	0x04	//��Ϣ�㲥ȡ���ɹ�Ӧ��
//��������
#define MSG_INFO_ASK_MENU 	0x81	//����̶���Ϣ�㲥�˵�����
#define MSG_INFO_ASK 			0x82	//�̶���Ϣ�㲥
#define MSG_INFO_CANCLE	 	0x83	//��Ϣ�㲥ȡ��

//������λ�ò�ѯ������������0x07��
//��������
#define MSG_NAVIGATION_CURRENT_LOCATION 		0x01	//��ѯ��ǰλ�ù���
#define MSG_NAVIGATION_INFO_MENU 				0x02	//�ܱ���Ϣ����˵�����
#define MSG_NAVIGATION_INFO_ROUND 				0x03	//��ѯ�ܱ���Ϣ����
//��������
#define MSG_NAVIGATION_QUERY_CURRENT_LOCATION 	0x81	//��ѯ��ǰλ�ù���
#define MSG_NAVIGATION_ASK_INFO_MENU 			0x82	//�����ܱ���Ϣ����˵�����
#define MSG_NAVIGATION_QUERY_INFO_ROUND 		0x83	//��ѯ�ܱ���Ϣ����

//��Կ����������������0x08��
//��������
#define MSG_KEY_WORK	0x01	//������Կ
#define MSG_KEY_RESET	0x02	//���½���������Կ
//��������
#define MSG_KEY_ASK 	0x81	//��������Կ����
#define MSG_KEY_ANS_OK 		0x82	//������Կ�ɹ�������Ӧ


//����ͷ������������0x09��
//��������
#define MSG_CAMERA_SET_ASK	0x01	//����ͼ��֡
#define MSG_CAMERA_STOP_ASK	0x02	//ֹͣ��������´���ͼ��֡
#define MSG_CAMERA_REST_ASK	0x03	//����ͷ����
#define MSG_CAMERA_UP_SAVE  0x04	//�ϴ��洢��ͼƬ
//��������
#define MSG_CAMERA_SET_PACKAGE_ACK	0x85	//����ͼ��֡Ӧ��(�ְ������Ӧ��)
#define MSG_CAMERA_SET_ACK	0x84	//����ͼ��֡Ӧ��
#define MSG_CAMERA_START_ACK	0x82	//ֹͣ�����´��䵱ǰ֡Ӧ��
#define MSG_CAMERA_REST_ACK	0x83	//����ͷ����Ӧ��
#define MSG_CAMERA_UP_ANS  0x84	//�ϴ��洢��ͼƬӦ��


//�������(�������0x0A)
//��������
#define MSG_EXT_DEV_SET		0x01

//��������
#define MSG_EXT_DEV_REPORT	0x81		//
//����оƬ���(0x0B)
//��������
#define  MSG_VOICE_SET_COMBIN    0x01    //�ϳ�����
/*
#define  MSG_VOICE_PAU_COMBIN    0X02    //��ͣ�ϳ�
#define  MSG_VOICE_RET_COMBIN    0X03            //�ָ��ϳ�
#define  MSG_VOICE_STO_COMBIN    0x04    //ֹͣ�ϳ�
#define  MSG_VOICE_SLE_COMBIN     0X05     //��������
*/
#endif //_MSG_H_





