/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Receive.h
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-7-23
  ����������SG2000 ���ձ��ĺʹ���  
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_RECEIVE_H_
#define _SG_RECEIVE_H_

#ifdef __cplusplus
extern "C" {
#endif


#if (1)
 // add by guobl for Big-Endian & Little-Endian conversion 2007/8/30 
// Big-Endian TO Little-Endian(long type)
#define htonl(l) ((((l) & 0xff000000) >> 24) | \
           			(((l) & 0x00ff0000) >>  8) | \
              		(((l) & 0x0000ff00) <<  8) | \
              		(((l) & 0x000000ff) << 24))

// Little-Endian TO Big-Endian(long type)
#define ntohl(l) htonl(l)

// Big-Endian TO Little-Endian(short type)
#define htons(s) ((((s) >> 8) & 0xff) | \
             		(((s) << 8) & 0xff00))

// Little-Endian TO Big-Endian(short type)
#define ntohs(s) htons(s)
#endif /* (0) */


#define  MAX_MUSTER_LIST_NUM 5
typedef struct _MUSTER_LIST_ 
{
	char msg[500];
	int len;
	struct _MUSTER_LIST_ *next;
	
}SG_MUSTER_LIST;

typedef struct _INFO_OIL_
{
	int  nPersernt;
	int  nconsult;
	struct  _INFO_OIL_ *next;
}SG_INFO_OIL;



int SG_Rcv_Handle_Msg(unsigned char *msg, int len);

#endif //_SG_RECEIVE_H_


