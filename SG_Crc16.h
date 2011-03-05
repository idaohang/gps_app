/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_Crc16.h
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-8-23
  内容描述：SG2000 报文CRC校验处理头文件          
  修改记录：
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

