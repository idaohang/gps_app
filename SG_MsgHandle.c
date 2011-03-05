/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2007-2008 ��Ȩ����
  �ļ��� ��SG_MsgHandle.c
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2005-7-23
  ����������SG2000  ���ķ��ͺʹ���
  �޸ļ�¼��

***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os_api.h"

#include "msg.h"
#include "SG_GPS.h"
#include "SG_Send.h"
#include "SG_Set.h"
#include "SG_CRC16.h"
#include "SG_MsgHandle.h"
#include "SG_Receive.h"
#include "SG_Camera.h"

extern unsigned char*Camera_Buf;
extern int Camera_BufCnt ;


/*****************************************************************
����˵��:  ParseGPS

�������:   gps   	ָ����Ҫ������ GPS �ṹ��
         	data  	ָ���װ�������
        	pos  	ָ��� data ��ʼ�洢��λ��
���:   	�� data+*pos ��ʼ�洢������������
   			pos ָ���Ƶ��������Ľ�β
�޸ļ�¼:
			���4���ֽ������ͳ�ƣ��º�����2006-5-29
*****************************************************************/
void  ParseGPS(gps_data *gps, char *data, int * pos)
{
	int p = *pos;
	MsgChar tt[5] = "";
	MsgShort nAltitude;
	unsigned long alarmState;
	SCI_DATE_T	currDate={0};
	SCI_TIME_T	currTime={0xff};

	//״̬������
	alarmState = htonl(g_state_info.alarmState);
	SCI_MEMCPY(&data[p],&alarmState,4);
	p += 4;
	//���ڡ�ʱ��
	tt[0] = gps->sDate[4];
	tt[1] = gps->sDate[5];
	data[p] = (char)atoi(tt);
	if(data[p] > 99 || data[p] < 7){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currDate.year == 0)
				TM_GetSysDate(&currDate);

			data[p] = currDate.year -2000;
		}
	}

	p += 1;
	tt[0] = gps->sDate[2];
	tt[1] = gps->sDate[3];
	data[p] = (char)atoi(tt);
	if(data[p] > 12 || data[p] < 1){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currDate.year == 0)
				TM_GetSysDate(&currDate);

			data[p] = currDate.mon;
		}
	}

	p += 1;
	tt[0] = gps->sDate[0];
	tt[1] = gps->sDate[1];
	data[p] = (char)atoi(tt);
	if(data[p] > 31 || data[p] < 1){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currDate.year == 0)
				TM_GetSysDate(&currDate);

			data[p] = currDate.mday;
		}
	}

	p += 1;
	tt[0] = gps->sTime[0];
	tt[1] = gps->sTime[1];
	data[p] = (char)atoi(tt);
	if(data[p] > 23/* || data[p] < 0*/){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currTime.hour == 0xff)
				TM_GetSysTime(&currTime);

			data[p] = currTime.hour;
		}
	}

	p += 1;
	tt[0] = gps->sTime[2];
	tt[1] = gps->sTime[3];
	data[p] = (char)atoi(tt);
	if(data[p] > 59/* || data[p] < 0*/){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currTime.hour == 0xff)
				TM_GetSysTime(&currTime);

			data[p] = currTime.min;
		}
	}

	p += 1;
	tt[0] = gps->sTime[4];
	tt[1] = gps->sTime[5];
	data[p] = (char)atoi(tt);
	if(data[p] > 59/* || data[p] < 0*/){
		if(alarmState & GPS_CAR_STATU_POS){
			if(currTime.hour == 0xff)
				TM_GetSysTime(&currTime);

			data[p] = currTime.sec;
		}
	}

	p += 1;
	//����
	SCI_MEMCPY(tt, gps->sLongitude, 3);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLongitude[3], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLongitude[6], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLongitude[8], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	//γ��
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, gps->sLatitude, 2);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLatitude[2], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLatitude[5], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	SCI_MEMSET(tt, 0, 5);
	SCI_MEMCPY(tt, &gps->sLatitude[7], 2);
	data[p] = (char)atoi(tt);
	p += 1;
	//�ٶ�
	data[p] = (char)atoi(gps->sSpeed);
	p += 1;
	//����
	data[p] = (char)((atoi(gps->sAngle)+1)/2);
	p += 1;
	//���ٶ�
	data[p] = gps->nAcc;
	p += 1;
	//��ʻ����
	//big-endian

	if(((gps_data *)(g_state_info.pGpsCurrent))->status)
	{
		data[p] = (MsgUChar)(*((MsgUChar*)(&gps->nDistance) + 1));
		data[p+1] = (MsgUChar)(*((MsgUChar*)(&gps->nDistance)));
	}
	else
	{
		data[p] = 0;
		data[p+1] = 0;
	}
	p += 2;
	//�߶�
	//big-endian
	nAltitude = (MsgShort)atoi(gps->sAltitude);
	if (nAltitude > 30000 || nAltitude < -10000)
		nAltitude = 0;
	data[p] = (MsgUChar)(*((MsgUChar*)(&nAltitude) + 1));
	data[p+1] = (MsgUChar)(*((MsgUChar*)(&nAltitude)));
	p += 2;
	//�ǿ�
	data[p] = gps->status;
	p += 1;
	//����
	data[p] = gps->nNum;
	p += 1;
	//�����ͳ��
	data[p] = (char)((g_state_info.nGetTotalDistance >> 24)&0xff);
	data[p+1] = (char)((g_state_info.nGetTotalDistance >> 16)&0xff);
	data[p+2] = (char)((g_state_info.nGetTotalDistance >> 8)&0xff);
	data[p+3] = (char)g_state_info.nGetTotalDistance & 0xff;
	p +=4;

	*pos = p;
}


/****************************************************************
  ������  ��MsgReportGps
  ��  ��  ��GPS��Ϣ�㱨(��ʱ�����ࡢ���Ρ�����)
  ���������
  				gps: gps��Ϣ�ṹ
  				bCompress: �Ƿ�ѹ������0��ѹ��1ѹ��
  				msgType:��������
  				msgCode:�������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
  ȥ��ѹ������ʱѹ����ŷ��͵����ƣ�2005-11-23
  ����λ��2�ȣ����ɷ壬2005-12-14
  ���Ӽ��ٶȺ���ʻ������ֵ�����ɷ壬2005-12-14
  (char)atoi(gps->sAngle/2��Ϊ(char)(atoi(gps->sAngle)/2)�����ɷ壬2005-12-17
  ���ε�ֵ����ʱ��0�����ɷ壬2005-12-28
****************************************************************/
static MsgShort MsgReportGps(gps_data *gps, MsgChar bCompress, MsgUChar msgType, MsgUChar msgCode, MsgUChar **out, MsgInt *outlen)
{
	MsgUChar* data = NULL;
	MsgInt len, p = 0;//, i;
	int temp;

	if (gps == NULL || out == NULL || outlen == NULL)
	{
		SCI_TRACE_LOW("<<<<<<MsgReportGps: PARA ERR, TYPE: 0x%02x, CODE: 0x%02x", msgType, msgCode);
		return MSG_FALSE;
	}

	len = MSG_GPS_LEN;

	data = SCI_ALLOC( len);
	if (data == NULL)
	{
		SCI_TRACE_LOW("<<<<<<MsgReportGps: CALLOC: %d", len);
		return MSG_FALSE;
	}

	SCI_MEMSET(data,0,len);

	//��������:GPS���ݰ�
	ParseGPS(gps,(char*)data,(int*)&p);

//	����GPS����
//�������ݱ�ǣ�4��+n*�������������ʹ��루1��+�������ݳ��ȣ�1��+����������4��
//���У��������ݱ�־Ϊ4���ֽڵ�ʮ�����ƣ�ֵ�̶�ΪFFFFFFFF
//�ٶ�С������	3	�ٶȵ�С�����֣�4������λ0.0001 ����/ʱ

//start
	data[p++]=0xff;
	data[p++]=0xff;
	data[p++]=0xff;
	data[p++]=0xff;

	if(g_set_info.bNewUart4Alarm & UT_LOCK)// �п�������ʱ����ʾ�˸�������
	{
	data[p++]=0x01;  //������������
	data[p++]=0x08;  //�������ݳ���

	//��Ҫ�ϴ��ĵ�ǰ����
	data[p] = (char)((g_state_info.upOilQuantity>> 24)&0xff);
	data[p+1] = (char)((g_state_info.upOilQuantity >> 16)&0xff);
	data[p+2] = (char)((g_state_info.upOilQuantity >> 8)&0xff);
	data[p+3] = (char)g_state_info.upOilQuantity & 0xff;
	p+=4;

	//��Ҫ�ϴ��ĵ�ǰADֵ
	data[p] = (char)((g_state_info.upAD>> 24)&0xff);
	data[p+1] = (char)((g_state_info.upAD >> 16)&0xff);
	data[p+2] = (char)((g_state_info.upAD >> 8)&0xff);
	data[p+3] = (char)g_state_info.upAD & 0xff;
	p+=4;
	}

	data[p++]=0x03;  //������������
	data[p++]=0x04;  //�������ݳ���

	temp = (atof(gps->sSpeed) - atoi(gps->sSpeed))*10000;

	SCI_MEMCPY(&data[p],&temp,4);
	p += 4;

	if (MsgCreatMsg(msgType, msgCode, data, p, out, outlen) == MSG_FALSE)
	{
		if (data)
			SCI_FREE(data);
		return MSG_FALSE;
	}
	else
	{
		if (data)
			SCI_FREE(data);
		return MSG_TRUE;
	}
}

/****************************************************************
  ������  ��MsgSetApn
  ��  ��  ����������APN��ַ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetApn(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_APN, out, outlen);
}

/****************************************************************
  ������  ��MsgSetApn
  ��  ��  �������ƶ����ĺ���
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/

MsgShort MsgSetMoveCen(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_MOVE_CEN, NULL, 0, out, outlen);
}


/****************************************************************
  ������  ��MsgCameraRestAck
  ��  ��  ������ͷ����Ӧ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2006-3-23
****************************************************************/

MsgShort MsgCameraRestAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_CAMERA, MSG_CAMERA_REST_ACK, msg, len, out, outlen);
}


/****************************************************************
  ������  ��MsgCameraStartAck
  ��  ��  ������ͷֹͣӦ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2006-3-23
****************************************************************/
MsgShort MsgCameraStartAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_CAMERA, MSG_CAMERA_START_ACK, msg, len, out, outlen);
}


/****************************************************************
  ������  ��MsgSetHelpNo
  ��  ��  ��������������
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetHelpNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_HELP_NO, NULL, 0, out, outlen);
}


/****************************************************************
  ������  ��MsgSetMedicalNo
  ��  ��  ������ҽ�ƾȻ��绰����
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetMedicalNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_MEDICAL_NO, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgSetServiceNo
  ��  ��  ������ά�޵绰����
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetServiceNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_SERVICE_NO, NULL, 0, out, outlen);
}




/****************************************************************
  ������  ��MsgSetTempRange
  ��  ��  �������¶�����ֵӦ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  �޸ļ�¼��������2006-9-18   ����
****************************************************************/
MsgShort MsgEditSmsUpload(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_MUSTER,MSG_SMS_UPLOAD,msg,len,out,outlen);
}




/****************************************************************
  ������  ElectronDevReport
  ��  ��  �������豸����
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
 �޸ļ�¼:���� 2006-8-8   ����
****************************************************************/
MsgShort ElectronDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_EXT_DEV, MSG_EXT_DEV_REPORT, msg, len, out, outlen);

}




/****************************************************************
  ������  ��MsgCameraAck
  ��  ��  ������ͷ��������ȷ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2006-3-14
  	ȥ��ͼ������ǰ��4���ֽ�ͼ�񳤶�,�º���,2006-6-6
****************************************************************/
MsgShort MsgCameraAck(MsgUChar * data, MsgInt len, MsgUChar **out, MsgInt *outlen,int packNo,int packCnt)
{
	int  pos = 0;
	unsigned short pic_Tx_Len = 0;
	static unsigned char *para =NULL;
	int nCameraID = 1;
	gps_data* gps = (gps_data*)g_state_info.pGpsFull;

#if (0)
	{
		strcpy(gps->sLongitude,"119.2953");
		strcpy(gps->sLatitude,"26.0827");
		strcpy(gps->sSpeed,"100");
		gps->nAcc = 15;
		strcpy(gps->sEastWest,"E");
		strcpy(gps->sSouthNorth,"N");
		gps->status = 1;
		gps->nNum = 9;
		strcpy(gps->sDate,"240408");
		strcpy(gps->sTime,"112233");
	}
#endif /* (0) */


	//������ľ����ͷ
	if(g_set_info.bNewUart4Alarm&UT_QQCAMERA)
	{
		if(packCnt>1)
		{
			para = data;
		}
		else
		{
			para = data +2;
		}
	}
	else if(g_set_info.bNewUart4Alarm&UT_XGCAMERA)
	{
		para = data;
	}


	//data�ṹ:ͼ������
	//para�ṹ:Ӧ�����ͣ�1��+ ����ͷID��1�� +GPS����(2)+GPS����(31)
	//+ ͼ�����ݴ�С��2��+ͼ������


    // ����15k��С��ͼƬ���÷ְ�Э��
    //para�ṹ:Ӧ�����ͣ�1��+����ͷID��1��+GPS���Ĵ�С��2��+GPS���ģ�31��
    //+ͼ���Ƭ��ţ�1��+ͼ���Ƭ������1��+ͼ�����ݴ�С��2��+ͼ�����ݣ�N��


	//Ӧ������
	para[pos] = 0X00;
	pos ++;

	// ����ͷID
	para[pos] = nCameraID;  // Ҫ�����cameraid���ж�
	pos ++;

	//gps���ĳ���
	para[pos] = ((MSG_GPS_LEN>>8)&0xff);
	pos ++;
	para[pos] = (MSG_GPS_LEN&0xff);
	pos ++;

	//����gps����
	ParseGPS(g_state_info.pGpsFull, (char*)para, &pos);

	if(packCnt>1)
	{
		para[pos] = packNo;
		pos ++;
		para[pos] = packCnt;
		pos ++;

	}

	//����ͼ�񳤶�
	pic_Tx_Len = len;
	memcpy(&para[pos],(char *)&pic_Tx_Len,2);

	pos +=2;

	//����ͼ��

	pos += len;
#if (0)
	{
		int i;
		for(i=pos - 10;i<pos;i++)
		{
			SCI_TRACE_LOW("== %02x",para[i]);
		}
	}
#endif /* (0) */


	memcpy(g_state_info.sMsgNo, g_set_info.sCameraMsgNo, SG_MSG_NO_LEN);

	if(packCnt > 1)
	{
		return MsgCreatMsg(MSG_TYPE_CAMERA, MSG_CAMERA_SET_PACKAGE_ACK, para, pos, out, outlen);
	}
	else
	{
	return MsgCreatMsg(MSG_TYPE_CAMERA, MSG_CAMERA_SET_ACK, para, pos, out, outlen);
	}


	return 1;
}




/****************************************************************
  ������  ��MsgSetPoliceNo
  ��  ��  �����ñ�������
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetPoliceNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_POLICE_N0, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgSetCallLimit
  ��  ��  �����ú�������
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetCallLimit(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_CALL_LIMIT, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgSetDisconnectGprs
  ��  ��  ���Ͽ�GPRS����
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetDisconnectGprs(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_DISCONNECT_GPRS, NULL, 0, out, outlen);
}


/****************************************************************
  ������  ��MsgSetUserPsw
  ��  ��  �������û���������
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetUserPsw(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_USER_PSW, out, outlen);
}


/****************************************************************
  ������  ��MsgSetTcp
  ��  ��  ����������TCP��ַ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetTcp(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_TCP, out, outlen);
}


/****************************************************************
  ������  ��MsgSetOverSpeed
  ��  ��  �����ó���
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetOverSpeed(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_OVERSPEED, out, outlen);
}

/****************************************************************
  ������  ��MsgSetRegion
  ��  ��  ����������
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetRegion(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_REGION, out, outlen);
}


/****************************************************************
  ������  ��MsgSetOneGPSNum
  ��  ��  ������һ������ͨ��Ӧ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetOneGPSNum(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_ONEGPS_NUM, out, outlen);
}

/****************************************************************
  ������  ��MsgSetReset
  ��  ��  ����λ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetReset(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_RESET, NULL, 0, out, outlen);
}


/****************************************************************
  ������  ��MsgSetCommunicationType
  ��  ��  ������ͨѶ��ʽ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetCommunicationType(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_COMMUNICATION_TYPE, out, outlen);
}

/****************************************************************
  ������  ��MsgSetADAlarm
  ��  ��  ������������ֵӦ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetADAlarm(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_ALARM_AD, out, outlen);
}

/****************************************************************
  ������  ��MsgSetPND
  ��  ��  �����ñ��ص���Ӧ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����׽�
  �޸ļ�¼��������2009-3-6
****************************************************************/
MsgShort MsgSetPND(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_PND, out, outlen);
}

/****************************************************************
  ������  ��MsgSetConsult
  ��  ��  �����ü��ο���
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetConsult(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_OIL_CONSULT, out, outlen);
}

/****************************************************************
  ������  ��MsgSetLine
  ��  ��  �������г�·��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetLine(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_LINE, out, outlen);
}

/****************************************************************
  ������  ��MsgSetLine
  ��  ��  �������г�·�ߴ����ٱ���
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetLineSpeed(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_LINE_SPEED, out, outlen);
}

/****************************************************************
  ������  ��MsgSetProxy
  ��  ��  �����ô����ַ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetProxy(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_PROXY, out, outlen);
}

/****************************************************************
  ������  ��MsgSetProxyEnable
  ��  ��  �����ô���ʹ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetProxyEnable(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_PROXY_ENABLE, out, outlen);
}


/****************************************************************
  ������  ��MsgSetBlindEnable
  ��  ��  ������ͨѶ��ʱͣ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetStopTout(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_STOP_TIMEOUT, out, outlen);
}


/****************************************************************
  ������  ��MsgSetDrvTout
  ��  ��  ������ͨѶ��ʱ��ʻ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetDrvTout(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_DRIVE_TIMEOUT, out, outlen);
}

/****************************************************************
  ������  ��MsgSetCallTout
  ��  ��  ������ͨѶͨ����ʱ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetCallTout(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_CALL_TIMEOUT, out, outlen);
}

/****************************************************************
������  ��MsgListenAck
��  ��  ������Ӧ��
������� ��
������� ��
			out:�������
			outlen:�������
			MSG_TRUE :�ɹ�
			MSG_FALSE:ʧ��
��д��  ��л�Ƶ�
�޸ļ�¼ ��������2010-05-05
****************************************************************/
MsgShort MsgSetListenOk(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_LISTEN_NO, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgIntervalWtachByTime
  ��  ��  �����ö�ʱ���
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgIntervalWtachByTime(gps_data *gps,int bCompress, MsgUChar **out, MsgInt *outlen)
{
	return MsgReportGps(gps, bCompress, MSG_TYPE_WATCH, MSG_WATCH_ANS_INTERVALCHECK, out, outlen);
}


/****************************************************************
  ������  ��MsgWatchIntervalStop
  ��  ��  ��ֹͣ��ѯ���
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgWatchIntervalStop(MsgUChar type, MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_ANS_INTERVALSTOP, &type, 1, out, outlen);
}

/****************************************************************
  ������  ��MsgWatchCancle
  ��  ��  �����ȡ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgWatchCancle(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_ANS_CANCLE, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgWatchHanleDown
  ��  ��  ������(���ķ���)
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgWatchHanleDown(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_ANS_HAND_DOWN, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgSafetyRegionCancle
  ��  ��  �����򱨾�ȡ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSafetyRegionCancle(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_REGION_CANCLE, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgSafetyLineCancle
  ��  ��  ��·�߱���ȡ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSafetyLineCancle(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_LINE_CANCLE, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgSafetyOverspeedCancle
  ��  ��  �����ٱ���ȡ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSafetyOverspeedCancle(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_OVERSPEED_CANCLE, NULL, 0, out, outlen);
}

/****************************************************************
������  ��MsgListenAck
��  ��  ������Ӧ��
���������
���������
			out:�������
			outlen:�������
			MSG_TRUE :�ɹ�
			MSG_FALSE:ʧ��
��д��  �����ɷ�
�޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgListenAck(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_LISTEN, NULL, 0, out, outlen);
}


/****************************************************************
  ������  ��MsgSafetyOverspeedCancle
  ��  ��  �����賬������
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSafetyChgPasswd(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_CHGPASSWD, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgSetHelpNo
  ��  ��  ���ز���ϯ����	
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgCallBackToNo(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_CALL_BACK_TO, msg,len, out, outlen);
}

/****************************************************************
  ������  ��MsgBlackBoxAck
  ��  ��  ����ϻ������Ӧ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2006-5-28
****************************************************************/
MsgShort MsgBlackBoxAck(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_BLACK_BOX_ACK, msg, len, out, outlen);
}

/****************************************************************
  ������  ��MsgBlackBoxStopAck
  ��  ��  ����ϻ��ֹͣӦ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2006-5-28
****************************************************************/
MsgShort MsgBlackBoxStopAck(MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_BLACK_BOX_STOP_ACK, NULL, 0, out, outlen);
}

/****************************************************************
  ������  MsgSelfDefineAlarmAck
  ��  ��  ���Զ��屨��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2006-9-8
****************************************************************/
MsgShort MsgSelfDefineAlarmAck(MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_SELF_DEFINE_ALARM, NULL,0, out, outlen);

}

/****************************************************************
  ������  ��MsgSetOwnNo
  ��  ��  �����ó�̨������
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetOwnNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_OWN_NO, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgSetInitGprs
  ��  ��  ������GPRS��ʼ����
  ���������out:�������
  			outlen:�������
  ���������MSG_TRUE :�ɹ�
  			MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetInitGprs(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_INIT_GPRS, out, outlen);
}


/****************************************************************
  ������  ��MsgReportSetOk
  ��  ��  ���������óɹ���Ӧ����(������������)
  ���������
  				code:�������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgReportSetOk(MsgUChar msgCode, MsgUChar **out, MsgInt *outlen)
{
	MsgUChar data[1];

	data[0] = MSG_SET_OK;

	return MsgCreatMsg(MSG_TYPE_SET, msgCode, data, 1, out, outlen);
}


/****************************************************************
  ������  ��MsgCreatMsg
  ��  ��  ���������ݱ���
  ���������
  				msgType:��������
  				msgCode:�������
  				data:������
  				dataLen:����������
  ���������
  				out:�������
  				outLen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
  	δ������Կʱֻ�ܴ��������ù̶���Կ�ı���
  	��������ʧ�ܣ�2005-11-9�����ɷ�
  	���ӵ��ٱ������к����֣������㱨����Ϣ�㲥����
  	����������Կ�������к�Ϊ0�����ɷ壬2005-12-15
  	BUG:��ػ㱨ʹ�ü�����кţ����ɷ壬2005-12-16
  	����������Ӧ��ֻ���ڵ�һ������ʱʹ�ù̶���Կ�����ɷ壬2005-12-18
****************************************************************/
MsgShort MsgCreatMsg(MsgUChar msgType, MsgUChar msgCode, MsgUChar *data, MsgInt dataLen, MsgUChar **out, MsgInt *outLen)
{
	MsgUChar* msg = NULL;
	MsgInt len, p = 0;
	MsgUChar ans=0x30;
	MsgChar key;


	SCI_TRACE_LOW( "******MsgCreatMsg");

	if (out == NULL || outLen == NULL || dataLen < 0)
	{
		SCI_TRACE_LOW( "MsgCreatMsg: PARA ERR, TYPE: 0x%02x, CODE: 0x%02x, DATALEN: %d\r\n", msgType, msgCode, dataLen);
		return MSG_FALSE;
	}

	if (data == NULL && dataLen > 0)
	{
		SCI_TRACE_LOW( "MsgCreatMsg: DATA ERR, TYPE: 0x%02x, CODE: 0x%02x, DATALEN: %d\r\n", msgType, msgCode, dataLen);
		return MSG_FALSE;
	}

	len = MSG_NO_LEN + MSG_SIGN_LEN + dataLen;
	msg = SCI_ALLOC( len);
	if (msg == NULL)
	{
		SCI_TRACE_LOW( "MsgCreatMsg: CALLOC: %d \r\n", len);
		return MSG_FALSE;
	}

	SCI_MEMSET(msg,0,len);

	if (msgType == MSG_TYPE_MUSTER)
	{
		SCI_MEMCPY(msg, g_set_info.sMusterMsgNo, SG_MSG_NO_LEN);
		p += SG_MSG_NO_LEN;
	}
	else if (	(msgType == MSG_TYPE_SAFETY && msgCode == MSG_SAFETY_ANS_POLICE)
		|| (msgType == MSG_TYPE_INFO && (msgCode == MSG_INFO_ASK_MENU || msgCode == MSG_INFO_ASK || msgCode == MSG_INFO_CANCLE))
		|| (msgType == MSG_TYPE_NAVIGATION && (msgCode == MSG_NAVIGATION_QUERY_CURRENT_LOCATION || msgCode == MSG_NAVIGATION_ASK_INFO_MENU || msgCode == MSG_NAVIGATION_QUERY_INFO_ROUND))
		|| (msgType == MSG_TYPE_KEY && msgCode == MSG_KEY_ASK)
		|| (msgType == MSG_TYPE_SAFETY && msgCode == MSG_SAFETY_ANS_ALARM))
	{
		SCI_MEMSET(msg, 0, SG_MSG_NO_LEN);
		p += SG_MSG_NO_LEN;
	}
	else if(msgType==MSG_TYPE_EXT_DEV){
		memcpy(msg, g_set_info.sExtMsgNo, SG_MSG_NO_LEN);
		p += SG_MSG_NO_LEN;
	}
	else
	{
		SCI_MEMCPY(msg, g_state_info.sMsgNo, SG_MSG_NO_LEN);
		p += SG_MSG_NO_LEN;
	}

	//�����ʶ
	msg[p] = msgType;
	p += 1;
	msg[p] = msgCode;
	p += 1;

	//������
	if (data != NULL && dataLen > 0)
	{
		SCI_MEMCPY(&msg[p], data, dataLen);
		p += dataLen;
	}

	if (MsgMakeMsg(key, msg, len, ans, out, outLen) == MSG_FALSE)
	{
		if (msg)
			SCI_FREE(msg);
		SCI_TRACE_LOW( "MsgCreatMsg: TYPE: 0x%02x, CODE: 0x%02x, DATALEN: %d\r\n", msgType, msgCode, dataLen);
		return MSG_FALSE;
	}
	else
	{
		if (msg)
			SCI_FREE(msg);

		SCI_TRACE_LOW( "MsgCreatMsg: TYPE: 0x%02x, CODE: 0x%02x, DATALEN: %d\r\n", msgType, msgCode, dataLen);
		return MSG_TRUE;
	}
}


/****************************************************************
  ������  ��MsgMakeMsg
  ��  ��  ����ϴ������ݱ���
  ���������
  				data: ��������(δѹ������)
  				len: �������ݳ���o
  				ans: ������Ӧ����
  				key:0:�̶���Կ1: ������Կ
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
  ��Ԥ����̶��ռ��Ϊʵ��ʹ�ÿռ䣬���ɷ壬2005-12-27
****************************************************************/
MsgShort MsgMakeMsg(MsgChar key, MsgUChar* data, MsgInt len, MsgUChar ans, MsgUChar **out, MsgInt *outlen)
{
	MsgInt p = 0;
	MsgUChar *msg = MSG_NULL;//, *msgdesed = MSG_NULL;
	MsgInt msglen = 0;//, msgdesedlen = 0;
	unsigned short CRC;
	MsgChar sASCIICenterNo[32 + 1] = "", sBCDCenterNo[MSG_OWN_NO_LEN] = "";

	SCI_TRACE_LOW( "********MsgMakeMsg: PARA %d\r\n",len);

	if (out == NULL || outlen == NULL)
	{
		SCI_TRACE_LOW( "MsgMakeMsg: PARA \r\n");
		return MSG_FALSE;
	}


	//Ԥ����������Ŀռ�
	msglen = MSG_HEAD_LEN + MSG_OWN_NO_LEN + 1 + len + MSG_CRC_LEN + 1;
	msg = SCI_ALLOC( msglen);
	if (msg == MSG_NULL)
	{
		SCI_TRACE_LOW( "MsgMakeMsg: CALLOC: %d \r\n", msglen);
		return MSG_FALSE;
	}

	SCI_MEMSET(msg,0,msglen);

	//����֡ͷ����ѹ�������ܣ�����2λ��
	SCI_MEMCPY(&msg[p], MSG_HEAD, MSG_HEAD_LEN);
	p += MSG_HEAD_LEN;

	//�ն˺Ų��֣���ѹ�������ܣ�15λASCII��ת������6λBCD�룩
	strcpy(sASCIICenterNo, g_set_info.sOwnNo);
	MsgAscii2Bcd(sASCIICenterNo, sBCDCenterNo);
	SCI_MEMCPY(&msg[p], sBCDCenterNo, MSG_OWN_NO_LEN);
	p += MSG_OWN_NO_LEN;

	//������Ӧ���루��ѹ�������ܣ�����1λ��
	SCI_MEMCPY(&msg[p], &ans, 1);
	p += 1;

	//������
	if (data != MSG_NULL && len > 0)
	{
		SCI_MEMCPY(&msg[p],data,len);
		p += len;
	}

	//CRCУ�飨��ѹ�������ܣ�
	CRC = Msg_Get_CRC16(msg, p);

	msg[p] = CRC>>8;
	msg[p+1] = CRC;
	p += MSG_CRC_LEN;

	//���
	*out = msg;
	*outlen = p;

	return MSG_TRUE;
}


/****************************************************************
  ������  ��MsgAscii2Bcd
  ��  ��  ��ASCII��ת��ΪBCD��(�ڴ�ռ����ⲿ����)
  ���������
  				ascii: ����ASCII���ַ���
  ���������
  				bcd:���BCD��
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-23
  	��������λ����ת���������ɷ壬2005-11-17
****************************************************************/
MsgShort MsgAscii2Bcd(char *ascii, char *bcd)
{
	MsgInt len, i = 0, j = 0;
	unsigned char c, c1, c2;

	if (ascii == NULL || bcd == NULL)
	{
		SCI_TRACE_LOW( "MsgAscii2Bcd: PARA \r\n");
		return MSG_FALSE;
	}

	len = strlen(ascii);

	for (i = 0; i < len; i++)
	{
		c1 = ascii[i++];

		if(i < len)
			c2 = ascii[i];
		else
			c2 = 0;

		c1 <<= 4;
		c2 &= 0x0f;
		c = (char)(c1 | c2);

		bcd[j++] = c;
	}

	return MSG_TRUE;
}

MsgShort MsgSafetyAlarm(gps_data *gps, MsgUChar **out, MsgInt *outlen)
{
	return MsgAlarmGps(gps, MSG_TYPE_SAFETY, MSG_SAFETY_ANS_ALARM, out, outlen);
}


/****************************************************************
  ������  ��MsgWatchByDistance
  ��  ��  ������㱨
  ���������
  				gps: gps��Ϣ�ṹ
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
    ȥ��ѹ�����Ͳ�����ȫ����Ϊ��ѹ����2005-11-23
****************************************************************/
MsgShort MsgWatchByDistance(gps_data *gps, MsgUChar **out, MsgInt *outlen)
{
	return MsgReportGps(gps, 1, MSG_TYPE_WATCH, MSG_WATCH_ANS_DISTANCE, out, outlen);
}


/****************************************************************
  ������  ��MsgWatchByTime
  ��  ��  ����ʱ�㱨
  ���������
  				gps: gps��Ϣ�ṹ
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
  ȥ��ѹ�����Ͳ�����ȫ����Ϊ��ѹ����2005-11-23
****************************************************************/
MsgShort MsgWatchByTime(gps_data *gps, MsgUChar **out, MsgInt *outlen)
{
	return MsgReportGps(gps, 1, MSG_TYPE_WATCH, MSG_WATCH_ANS_TIME, out, outlen);
}

/****************************************************************
  ������  ��MsgWatchByAmount
  ��  ��  �����λ㱨
  ���������
  				gps: gps��Ϣ�ṹ
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
    ȥ��ѹ�����Ͳ�����ȫ����Ϊ��ѹ����2005-11-23
****************************************************************/
MsgShort MsgWatchByAmount(gps_data *gps, MsgUChar **out, MsgInt *outlen)
{
	return MsgReportGps(gps, 1, MSG_TYPE_WATCH, MSG_WATCH_ANS_AMOUNT, out, outlen);
}


/****************************************************************
  ������  ��MsgWatchHanleUp
  ��  ��  ������(�ն˷���)
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgWatchHanleUp(MsgUChar type, MsgUChar **out, MsgInt *outlen)
{
	MsgUChar  pdu[4] = {0};

	SCI_TRACE_LOW("*******MsgWatchHanleUp");
	pdu[0] = type;
	pdu[2] = 2;
	pdu[3] = 98;
	return MsgCreatMsg(MSG_TYPE_WATCH, MSG_WATCH_HAND_UP, pdu, 4, out, outlen);
}


/****************************************************************
  ������  ��MsgSetListenNo
  ��  ��  ���������ļ���¼���绰����
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetListenNo(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_LISTEN_NO, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgSetTotalDist
  ��  ��  �������޸������Ӧ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  �޸ļ�¼��������2006-9-18   ����
****************************************************************/
MsgShort MsgSetTotalDist(MsgUChar **out, MsgInt *outlen)
{
	static unsigned char para[100]={0};
	int pos=0;

	ParseGPS(g_state_info.pGpsFull, (char*)para, &pos);
	SCI_TRACE_LOW( "MsgSetTotalDist:MsgLen:%d\r\n",pos);
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_TOTAL_DISTANSE,para,pos,out,outlen);
}


/****************************************************************
  ������  ��MsgSetCheckItself
  ��  ��  ���Լ�
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetCheckItself(MsgUChar *data, MsgInt len, MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_CHECK, data, len, out, outlen);
}

/****************************************************************
  ������  ��MsgSetCheckDrive
  ��  ��  ������
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetCheckDrive(MsgUChar *data, MsgInt len, MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_DRIVE, data, len, out, outlen);
}

/****************************************************************
  ������  ��MsgVerAck
  ��  ��  ���汾��Ϣ��ѯӦ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2006-3-21
****************************************************************/
MsgShort MsgVerAck(MsgUChar * data, MsgInt len, MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_VER_ACK, data, len, out, outlen);
}

/****************************************************************
  ������  ��MsgSetRegionNew
  ��  ��  ����������
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetRegionNew(MsgUChar **out, MsgInt *outlen)
{
	return MsgReportSetOk(MSG_SET_ANS_REGION_NEW, out, outlen);
}


MsgShort MsgSmsInterval(MsgUChar **out, MsgInt *outlen,unsigned char ret)
{
	unsigned char c = (unsigned char)ret;

	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_SMS_INTERVAL, &c, 1, out, outlen);
}


/****************************************************************
  ������  ��MsgDelInterestPiont
  ��  ��  ��ɾ����Ȥ��Ӧ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  �޸ļ�¼��������2006-9-18   ����
****************************************************************/
MsgShort MsgDelInterestPiont(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_SET, MSG_DEL_ANS_INFO_MUSTER, msg, len, out, outlen);

}
/****************************************************************
  ������  ��MsgSetInfoMuster
  ��  ��  ��������Ȥ��Ӧ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  �޸ļ�¼��������2006-9-18   ����
****************************************************************/
MsgShort MsgSetInterestPiont(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_INFO_MUSTER, msg, len, out, outlen);

}


/****************************************************************
  ������  ��MsgSetAlarm
  ��  ��  �����ñ�����
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetAlarm(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_ALARM, NULL, 0, out, outlen);
}

/****************************************************************
  ������  WatchDevReport
  ��  ��  ��͸���豸����
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
 �޸ļ�¼:���� 2006-8-8   ����
****************************************************************/
MsgShort WatchDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_EXT_DEV, MSG_EXT_DEV_REPORT, msg, len, out, outlen);
}



/****************************************************************
  ������  ��MsgReportGps
  ��  ��  ��GPS��Ϣ�㱨(��ʱ�����ࡢ���Ρ�����)
  ���������
  				gps: gps��Ϣ�ṹ
  				bCompress: �Ƿ�ѹ������0��ѹ��1ѹ��
  				msgType:��������
  				msgCode:�������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
  ȥ��ѹ������ʱѹ����ŷ��͵����ƣ�2005-11-23
  ����λ��2�ȣ����ɷ壬2005-12-14
  ���Ӽ��ٶȺ���ʻ������ֵ�����ɷ壬2005-12-14
  (char)atoi(gps->sAngle/2��Ϊ(char)(atoi(gps->sAngle)/2)�����ɷ壬2005-12-17
  ���ε�ֵ����ʱ��0�����ɷ壬2005-12-28
****************************************************************/
MsgShort MsgAlarmGps(gps_data *gps, MsgUChar msgType, MsgUChar msgCode, MsgUChar **out, MsgInt *outlen)
{
	MsgUChar* data = NULL;
	MsgInt len, p = 0;
	char alarmBuf[24]={0};
	unsigned short Len_alarmBuf = 0;
	unsigned short alarmlen = 0;
	unsigned long alarmstate = 0;

	if (gps == NULL || out == NULL || outlen == NULL)
	{
		SCI_TRACE_LOW( "MsgReportGps: PARA ERR, TYPE: 0x%02x, CODE: 0x%02x\r\n", msgType, msgCode);
		return MSG_FALSE;
	}

	if(msgType != MSG_TYPE_SAFETY || msgCode != MSG_SAFETY_ANS_ALARM){
		return MSG_FALSE;
	}

	len = MSG_GPS_LEN +2;

	if(g_state_info.alarmState & GPS_CAR_STATU_REGION_IN){
		alarmstate = GPS_CAR_STATU_REGION_IN;
		SCI_MEMCPY(&alarmBuf[alarmlen],(char*)&alarmstate,4);
		alarmBuf[4+alarmlen] = 0x01;
		alarmBuf[5+alarmlen] = g_state_info.loginID;
		alarmlen +=6;
	}
	if(g_state_info.alarmState & GPS_CAR_STATU_REGION_OUT){
		alarmstate = GPS_CAR_STATU_REGION_OUT;
		SCI_MEMCPY(&alarmBuf[alarmlen],(char*)&alarmstate,4);
		alarmBuf[4+alarmlen] = 0x01;
		alarmBuf[5+alarmlen] = g_state_info.logoutID;
		alarmlen +=6;
	}
	if(g_state_info.alarmState & GPS_CAR_STATU_PATH_ALRAM){
		alarmstate = GPS_CAR_STATU_PATH_ALRAM;
		SCI_MEMCPY(&alarmBuf[alarmlen],(char*)&alarmstate,4);
		alarmBuf[4+alarmlen] = 0x01;
		alarmBuf[5+alarmlen] = 0x00;
		alarmlen +=6;
	}

	len+= 2+alarmlen;

	data = SCI_ALLOC(len);
	if (data == NULL)
	{
		SCI_TRACE_LOW( "MsgReportGps: CALLOC: %d \r\n", len);
		return MSG_FALSE;
	}

	SCI_MEMSET(data,0,len);

	p =2;

	Len_alarmBuf = MSG_GPS_LEN;
	SCI_MEMCPY(data,(char*)&Len_alarmBuf,2);

	//��������:GPS���ݰ�
	ParseGPS(gps,(char*)data,&p);

	if(alarmstate)
	{
		Len_alarmBuf = alarmlen;
		SCI_MEMCPY(&data[p],(char*)&Len_alarmBuf,2);
		p+=2;
		SCI_MEMCPY(&data[p],alarmBuf,alarmlen);
		p+=alarmlen;
	}

	if (MsgCreatMsg(msgType, msgCode, data, len, out, outlen) == MSG_FALSE)
	{
		if (data)
			SCI_FREE(data);
		return MSG_FALSE;
	}
	else
	{
		if (data)
			SCI_FREE(data);
		return MSG_TRUE;
	}
}


/****************************************************************
  ������  ��MsgSafetyCancle
  ��  ��  ��������λ��Ϣ����ȡ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSafetyCancle(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_CANCLE, NULL, 0, out, outlen);
}

/****************************************************************
  ������  ��MsgSafetyOilLock
  ��  ��  ����/���͵�·��/����
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSafetyOilLock(MsgUChar id[2], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_OIL, id, 2, out, outlen);
}
#ifdef _DUART
/****************************************************************
  ������  ��MsgSafetyProLock
  ��  ��  ���ع�����Ӧ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSafetyProLock(MsgUChar id[2], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SAFETY, MSG_SAFETY_ANS_LOCK, id, 2, out, outlen);
}
#endif


/****************************************************************
  ������  ��MsgSetSafety
  ��  ��  ����ȫ����Ӧ��
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSetSafety(MsgUChar id[2], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_SET, MSG_SET_ANS_SAFETY, id, 2, out, outlen);
}


/****************************************************************
  ������  ��MsgMusterParticular
  ��  ��  ��������ϸ��Ϣ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgMusterParticular(MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_MUSTER, MSG_MUSTER_ANS_PARTICULAR, NULL, 0, out, outlen);
}


/****************************************************************
  ������  ��MsgOneGpsACK
  ��  ��  ��������ϸ��Ϣ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgOneGpsACK(MsgUChar id[1], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_MUSTER, MSG_ONE_GPS_ACK, id, 1, out, outlen);
}

/****************************************************************
  ������  ��MsgAdvertisingPlayACK
  ��  ��  ��������ϸ��Ϣ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgAdvertisingPlayACK(MsgUChar id[1], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_MUSTER, MSG_ADVERTISING_PLAY_ACK, id, 1, out, outlen);
}
/****************************************************************
  ������  ��MsgOneGPS
  ��  ��  ��������ϸ��Ϣ
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-10-21
****************************************************************/
MsgShort MsgSmsPlayACK(MsgUChar id[1], MsgUChar **out, MsgInt *outlen)
{
	return MsgCreatMsg(MSG_TYPE_MUSTER, MSG_SMS_PLAY_ACK, id, 1, out, outlen);
}

/****************************************************************
  ������  MsgBusDevReport
  ��  ��  ���ⲿ�豸����
  ���������
  ���������
  				out:�������
  				outlen:�������
  				MSG_TRUE :�ɹ�
  				MSG_FALSE:ʧ��
  ��д��  ���º���
  �޸ļ�¼��������2006-7-5
****************************************************************/
MsgShort MsgBusDevReport(MsgUChar *msg, MsgInt len,MsgUChar **out, MsgInt *outlen)
{

	return MsgCreatMsg(MSG_TYPE_EXT_DEV, MSG_EXT_DEV_REPORT, msg, len, out, outlen);

}


