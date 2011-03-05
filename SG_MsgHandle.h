/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_MsgHandle.h
  �汾   ��1.50
  ������ �����ɷ�
  ����ʱ�䣺2007-7-23
  ����������SG2000  ���ķ��ͺʹ���  
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_MSGHANDLE_H_
#define _SG_MSGHANDLE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "msg.h"


// External Variables 


/********************************************************************
*******************************��������******************************
********************************************************************/

//�������óɹ���Ӧ����(������������)
MsgShort MsgReportSetOk(MsgUChar msgCode, MsgUChar **out, MsgInt *outlen);

//��ϴ������ݱ���
MsgShort MsgMakeMsg(MsgChar key, MsgUChar* data, MsgInt len, MsgUChar ans, MsgUChar **out, MsgInt *outlen);

//ASCII��ת��ΪBCD��(�ڴ�ռ����ⲿ����)
MsgShort MsgAscii2Bcd(char *ascii, char *bcd);

//�������ݱ���
MsgShort MsgCreatMsg(MsgUChar msgType, MsgUChar msgCode, MsgUChar *data, MsgInt dataLen, MsgUChar **out, MsgInt *outLen);

//��ʱ�㱨
MsgShort MsgWatchByTime(gps_data *gps, MsgUChar **out, MsgInt *outlen);

//���λ㱨
MsgShort MsgWatchByAmount(gps_data *gps, MsgUChar **out, MsgInt *outlen);
// External Prototypes

//MsgShort ElectronDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

//��������APN��ַ
MsgShort MsgSetMoveCen(MsgUChar **out, MsgInt *outlen);

//���ñ�������
MsgShort MsgSetPoliceNo(MsgUChar **out, MsgInt *outlen);

//���ú�������
MsgShort MsgSetCallLimit(MsgUChar **out, MsgInt *outlen);

//�Ͽ�GPRS����	
MsgShort MsgSetDisconnectGprs(MsgUChar **out, MsgInt *outlen);

//����GPRS��ʼ����
MsgShort MsgSetInitGprs(MsgUChar **out, MsgInt *outlen);
//�ز���ϯ�绰
MsgShort MsgCallBackToNo(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// �����û���������	
MsgShort MsgSetUserPsw(MsgUChar **out, MsgInt *outlen);

// ����:��������TCP��ַ
MsgShort MsgSetTcp(MsgUChar **out, MsgInt *outlen);

//���ó���
MsgShort MsgSetOverSpeed(MsgUChar **out, MsgInt *outlen);

//��������	
MsgShort MsgSetRegion(MsgUChar **out, MsgInt *outlen);

//��λ
MsgShort MsgSetReset(MsgUChar **out, MsgInt *outlen);

//����ͨѶ��ʽ
MsgShort MsgSetCommunicationType(MsgUChar **out, MsgInt *outlen);

//�����г�·��
MsgShort MsgSetLine(MsgUChar **out, MsgInt *outlen);

//�����г�·�ߴ����ٱ���
MsgShort MsgSetLineSpeed(MsgUChar **out, MsgInt *outlen);

//���ô����ַ
MsgShort MsgSetProxy(MsgUChar **out, MsgInt *outlen);

//���ô���ʹ��
MsgShort MsgSetProxyEnable(MsgUChar **out, MsgInt *outlen);

//����ͨѶ��ʱ��ʻ
MsgShort MsgSetDrvTout(MsgUChar **out, MsgInt *outlen);

//����ͨѶ��ʱͣ��
MsgShort MsgSetStopTout(MsgUChar **out, MsgInt *outlen);


//����ͨѶͨ����ʱ
MsgShort MsgSetCallTout(MsgUChar **out, MsgInt *outlen);

//���ö�ʱ���
MsgShort MsgIntervalWtachByTime(gps_data *gps,int bCompress, MsgUChar **out, MsgInt *outlen);

//ֹͣ��ѯ���
MsgShort MsgWatchIntervalStop(MsgUChar type, MsgUChar **out, MsgInt *outlen);

//���ȡ��
MsgShort MsgWatchCancle(MsgUChar **out, MsgInt *outlen);

//����(���ķ���)	
MsgShort MsgWatchHanleDown(MsgUChar **out, MsgInt *outlen);

//���򱨾�ȡ��
MsgShort MsgSafetyRegionCancle(MsgUChar **out, MsgInt *outlen);

//·�߱���ȡ��
MsgShort MsgSafetyLineCancle(MsgUChar **out, MsgInt *outlen);

//���ٱ���ȡ��
MsgShort MsgSafetyOverspeedCancle(MsgUChar **out, MsgInt *outlen);

//���賬������
MsgShort MsgSafetyChgPasswd(MsgUChar **out, MsgInt *outlen);

//��ϻ������Ӧ��
MsgShort MsgBlackBoxAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

//��ϻ��ֹͣӦ��
MsgShort MsgBlackBoxStopAck(MsgUChar **out, MsgInt *outlen);

//����Ӧ��
MsgShort MsgListenAck(MsgUChar **out, MsgInt *outlen);

//�Զ��屨��
MsgShort MsgSelfDefineAlarmAck(MsgUChar **out, MsgInt *outlen);

MsgShort MsgSafetyAlarm(gps_data *gps, MsgUChar **out, MsgInt *outlen);
// ���ó�̨������
MsgShort MsgSetOwnNo(MsgUChar **out, MsgInt *outlen);

//  ͸���豸����
MsgShort WatchDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// ���ż������Ӧ��
MsgShort MsgSmsInterval(MsgUChar **out, MsgInt *outlen,unsigned char ret);

// ����ͷ����Ӧ��
MsgShort MsgCameraRestAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// ����ͷ��������ȷ��
MsgShort MsgCameraAck(MsgUChar * data, MsgInt len, MsgUChar **out, MsgInt *outlen,int packNo,int packCnt);

// ����ͷֹͣӦ��
MsgShort MsgCameraStartAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// ���ñ�����Ӧ��
MsgShort MsgSetAlarm(MsgUChar **out, MsgInt *outlen);

// �����ƶ����ĺ���Ӧ��
MsgShort MsgSetMoveCen(MsgUChar **out, MsgInt *outlen);


// �����豸����
MsgShort ElectronDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// �ϴ�������Ϣ
MsgShort MsgEditSmsUpload(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

// ������������
MsgShort MsgSetHelpNo(MsgUChar **out, MsgInt *outlen);

// ����ҽ����������
MsgShort MsgSetMedicalNo(MsgUChar **out, MsgInt *outlen);

// ����ά����������
MsgShort MsgSetServiceNo(MsgUChar **out, MsgInt *outlen);


MsgShort MsgWatchHanleUp(MsgUChar type, MsgUChar **out, MsgInt *outlen);
MsgShort MsgAlarmGps(gps_data *gps, MsgUChar msgType, MsgUChar msgCode, MsgUChar **out, MsgInt *outlen);
void  ParseGPS(gps_data *gps, char *data, int * pos);

MsgShort MsgBusDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen);

#ifdef __cplusplus
}
#endif

#endif //_SG_MSGHANDLE_H_
