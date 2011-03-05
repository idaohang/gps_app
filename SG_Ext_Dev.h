/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2008 ��Ȩ����
  �ļ��� ��SG_Hh.h
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-7-23
  ����������SG2000  ���ķ��ͺʹ���  
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_Ext_Dev_H_
#define _SG_Ext_Dev_H_


#ifdef __cplusplus
extern "C" {
#endif
//#include "sci_types.h"

// �궨��



//ZB��������صĺ궨��
#define ZB_GPS_START    	0x7E  // ֡ͷ 
#define ZB_GPS_END       	0x7F  // ֡β 
#define ZB_GPS_SPE		    0x7D  // 

#define CMD_ZB_ACK  	    0x00  // ��̨����״̬��Ϣ����ʾ��
#define CMD_GPS_SEND        0xF0  //GPS����
#define CMD_CENTER_UP       0x11  //���ĵ���
#define CMD_SAFETY_SET      0x1E  //PND��ȫ��������
#define CMD_CALL_CENTER     0x10  //�������ĵ绰����������
#define CMD_CALL_OUT   		0x30  //����绰����������
#define CMD_CALL_RING   	0x31  //��������(�����ն�)
#define CMD_CALL_REV    	0x32  //�绰����(������Ļ=)
#define CMD_CALL_OFF   	    0x33  //�绰�һ�(������)
#define CMD_CALL_SPEAK  	0x34  //֪ͨ��������ͨ(�����ն�)
#define CMD_CALL_END    	0x35  //֪ͨ�������һ�(�����ն�)
#define CMD_CALL_DTMF    	0x36  //����ֻ�
#define CMD_CALL_VOL   	    0x37  //��������(������)

#define CMD_LETTER_UP  	    0x41  // �����·�������Ϣ
#define CMD_SMS_OUT  		0x42   // ���ֻ����Ͷ���Ϣ(������)
#define CMD_SMS_UP       	0x43  // ��̨�յ��ֻ�����
#define CMD_SMS_TNC       	0x44  // TNC��ʽ����

#define CMD_STATE_UP 	    0xF3  // ��̨����״̬��Ϣ����ʾ��


// ͨ�������������
#define CMD_ZB_DIALNUM     	0X30  //  ����绰��������->�նˣ�
#define CMD_ZB_INCOMING    	0X31  //  �������壨�ն�->��������
#define CMD_ZB_ANSCALL    	0X32  //  �绰����(������Ļ->�ն�) 
#define CMD_ZB_CALLOFF    	0X33  //  �绰�һ�(������->�ն�)
#define CMD_ZB_MOANS     	0X34  //  ֪ͨ��������ͨ(�����ն�->������) 
#define CMD_ZB_MOOFF     	0X35  //  ֪ͨ�������һ�(�����ն�)
#define CMD_ZB_SENDDTMF     0X36  //  DTMF����
#define CMD_ZB_SENDVOL     	0X37  //  ��������(������)
#define CMD_ZB_SENDMICVOL   0X38  //  ��˷��������(������)
#define CMD_ZB_SENDMICVOL   0X38  //  ��˷��������(������)

// ��Ϣ���� 

#define CMD_ZB_HEART 		0x31  // ��̨����ʾ����ʱ����
#define CMD_MSG_UP 		0x32  // ��̨����������Ϣ����ʾ��
#define CMD_DST_UP 		0x11  // ��̨���͵���Ŀ�ĵؾ�γ����Ϣ����ʾ��
//#define CMD_STATE_UP 	0x34  // ��̨����״̬��Ϣ����ʾ��





#define CMD_RCV_ACK 	0x7A  // ��ʾ�����͹̶��������̨



// YH��������غ궨��
//����	��Ϣ����	
#define CMD_YH_SINBYTE		0x01//01H	���ֽڲ�������	 		˫��
#define CMD_YH_DIALNUM		0x02//02H	��������	        		�ֳ��ն���̨����
#define CMD_YH_INCOMING		0x03//03H	���Ժ��루������ʾ��		��̨���ֳ��ն˷���
#define CMD_YH_STATE		0x05//05H	��־��Ϣ				��̨���ֳ��ն˷���
#define CMD_YH_RCVCENTMSG	0x06//06H	�������ĵĶ���			��̨���ֳ��ն˷���
#define CMD_YH_SENDDTMF		0xA2//A2HDTMF					�ֳ��ն���̨����
#define CMD_YH_RCVSMS		0xA6//A6H	һ�����				��̨���ֳ��ն˷���
#define CMD_YH_SENDSMS		0xA4//A4H	���Ͷ���Ϣ				�ֳ��ն���̨����
#define CMD_YH_HEART       	0x0D//������					��̨���ֳ��ն˷��� 3����һ��

//���ֽ�ָ�01H
// 	�ֳ��ն���̨��
#define SINBYTEUP_HEART		0x01//01H	�ն˹�������
#define SINBYTEUP_ANSCALL	0x02//02H	�ն�ժ�����������磩
#define SINBYTEUP_CALLOFF	0x03//03H	�ն˹һ����Ҷ����磩
#define SINBYTEUP_MEDREQ	0x04//04H	ҽ�Ʒ�������
#define SINBYTEUP_FALREQ	0x05//05H	���Ϸ�������
#define SINBYTEUP_INFOREQ	0x06//06H	��Ϣ��������
#define SINBYTEUP_HANDFREE	0x07//07H	�����������л�ͨ��
#define SINBYTEUP_VOLUP		0x09//09H	��������
#define SINBYTEUP_VOLDN		0x0A//0AH	��������
#define SINBYTEUP_CONNECT	0x0B//0BH	�ն�������
// 	��̨���ֳ��նˣ�
#define SINBYTEDN_INCOMING	0x03//03H	������ʾ
#define SINBYTEDN_RING		0x11//11H	����
#define SINBYTEDN_MOOFF		0x12//12H	��̨�ڹһ�״̬��ȥ�磬�Է������һ���
#define SINBYTEDN_MOANS		0x13//13H	��̨��ժ��״̬��ȥ�磬�Է�������
#define SINBYTEDN_VIBRATE	0x14//14H	������������
#define SINBYTEDN_DISABLE	0x19//19H	��ֹ����
#define SINBYTEDN_CALLONLY	0x1A//1AH	��������
#define SINBYTEDN_SENDOK	0x17//17H	���ŷ��ͳɹ�
#define SINBYTEDN_SENDERR	0x18//18H	���ŷ���ʧ��

// ����ͨ�ŵ�֡ͷ��֡β
#define YH_GPS_START        0x0F // �µ�������֡ͷ
#define YH_GPS_END       	0xF0 // �µ�������֡β
#define YH_GPS_END2       	0xFF // �µ�������֡β��β
// ��������

void SG_Ext_Dev_Main(void);
unsigned char * FindChar(unsigned char *buf,int len,unsigned char c);
void UnicodeBE2LE(unsigned char *str,int len);
void Pro_Time_Handle(void);

#ifdef __cplusplus
}
#endif

#endif //_SG_HH_H_


