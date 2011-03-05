/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Crc16.h
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-8-23
  ����������SG2000 ����CRCУ�鴦��ͷ�ļ�          
  �޸ļ�¼��
***************************************************************/

#ifndef _SG_CRC16_H_
#define _SG_CRC16_H_

#ifdef __cplusplus
extern "C" {
#endif

void Msg_Init_CRC32_Table(void); 
unsigned int Msg_Get_CRC(unsigned char *text, unsigned int len);
unsigned short Msg_Get_CRC16(const unsigned char * puchMsg, unsigned long unDataLen);
unsigned short CRC_16( unsigned char * aData, unsigned long aSize );
void BuildTable16( unsigned short aPoly );
#ifdef __cplusplus
}
#endif

#endif //_MSG_CRC_H_

