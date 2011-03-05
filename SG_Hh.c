/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2007-2008 ��Ȩ����
  �ļ��� ��SG_Hh.c
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2005-7-23
  ����������SG2000  ���ķ��ͺʹ���  
  �޸ļ�¼��

***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SG_GPS.h"
#include "SG_Send.h"
#include "SG_Set.h"
#include "os_api.h"
#include "tb_hal.h"
#include "SG_Hh.h"
#include "SG_Call.h"
#include "Msg.h"
#include "sio.h"
#include "ref_engineering.h"
#include "SG_DynamicMenu.h"
#include "SG_MsgHandle.h"
#include "SG_Camera.h"
#include "SG_Receive.h"
#include "SG_Timer.h"
#include "SG_Menu.h"
#include "SG_Sms.h"
#include "SG_Set.h"
#include "SG_Net.h"

// �궨��
#define TM_MAX_MONTHS_SUPPORTED   	12

// ��������
LOCAL const uint16 day_of_month[2][TM_MAX_MONTHS_SUPPORTED] =
{
    {31,  28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31  },
    {31,  29,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31  }
};



PDUSTRUCT g_sendpdustruct; // ���� pdu �ṹ
PDUSTRUCT g_revpdustruct;  // ���� pdu �ṹ
BOOLEAN g_framesendok; //֡���ͳɹ���־1:�ɹ�0:ʧ��
unsigned char g_sendframe1s; //֡1���ط���ʱ
uint8 g_sendframecnt;
SENDFRAME g_sendframe;
uint8 setmenu_status = 0;
sg_hh_list *gp_hh_list = NULL; //�ֱ����Ͷ���
int gn_hh_list = 0; //�ֱ����нڵ���Ŀ
uint8 MAX_HH_LIST_NUM = 30; //���洢��Ŀ
sg_hh_list *hhItem = NULL;// �����е�һ���ڵ�
extern uint8 tryCount;


void SG_Do_PowerUpHd(){
	int hhcnt=0;
#ifdef _DUART
	int gpio_pw = PWR_HH;
#else
	int gpio_pw = PWR_SB;
#endif
	int time = 10000;
	
	// �ն˸�λʱ�Ƚ��ֱ���Դ����100ms��������
	if(g_set_info.bNewUart4Alarm & UT_LED2)
	{
		if(g_set_info.LedResetFlag == 1)
		{
			SG_Set_Cal_CheckSum();
			g_set_info.LedResetFlag = 0;
			SG_Set_Save();
			time = 1000000;
		}
		else
		{
			return;
		}
	}
	
	HAL_SetGPIODirection(gpio_pw,1);				// ����PWR_SB �������
	HAL_SetGPIOVal(gpio_pw,0);					// ����PWR_SB ���ŵ�
	hhcnt = 0;
	while(hhcnt++ <= time);
	HAL_SetGPIOVal(gpio_pw,1);					// ����PWR_SB ���ŵ�
}

// �ɴ�ӡ�ַ���ת��Ϊ�ֽ�����
// �磺"C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ������ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ�����ݳ���
unsigned char gsmString2Bytes(const unsigned char* pSrc, unsigned char* pDst, unsigned int nSrcLength)
{
	unsigned	int i;
	for( i=0; i<nSrcLength; i+=2)
	{
		// �����4λ
		if(*pSrc>='0' && *pSrc<='9')
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}
		
		pSrc++;
		
		// �����4λ
		if(*pSrc>='0' && *pSrc<='9')
		{
			*pDst |= *pSrc - '0';
		}
		else
		{
			*pDst |= *pSrc - 'A' + 10;
		}
		
		pSrc++;
		pDst++;
	}
	
	// ����Ŀ�����ݳ���
	return nSrcLength / 2;
}



// �ֽ�����ת��Ϊ�ɴ�ӡ�ַ���
// �磺{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01" 
// pSrc: Դ����ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ���ݳ���
// ����: Ŀ���ַ�������

char hextab[]="0123456789ABCDEF";	// 0x0-0xf���ַ����ұ�

unsigned int gsmBytes2String(const unsigned char* pSrc, unsigned char* pDst, unsigned char nSrcLength)
{
	unsigned char i;
	
	
	for( i=0; i<nSrcLength; i++)
	{
		*pDst++ = hextab[*pSrc >> 4];		// �����4λ
		*pDst++ = hextab[*pSrc & 0x0f];	// �����4λ
		pSrc++;
	}
	
	// ����ַ����Ӹ�������
	*pDst = '\0';
	
	// ����Ŀ���ַ�������
	return nSrcLength * 2;
}




// ����˳����ַ���ת��Ϊ�����ߵ����ַ�����������Ϊ��������'F'�ճ�ż��
// �磺"8613851872468" --> "683158812764F8"
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ���ַ�������
unsigned char gsmInvertNumbers(const unsigned char* pSrc, unsigned char* pDst, unsigned char nSrcLength)
{
	unsigned char nDstLength=0;		// Ŀ���ַ�������
	unsigned char i;
	char ch;			// ���ڱ���һ���ַ�
	
	// ���ƴ�����
	nDstLength = nSrcLength;
	
	// �����ߵ�
	for( i=0; i<nSrcLength;i+=2)
	{
		ch = *pSrc++;		// �����ȳ��ֵ��ַ�
		*pDst++ = *pSrc++;	// ���ƺ���ֵ��ַ�
		*pDst++ = ch;		// �����ȳ��ֵ��ַ�
	}
	
	// Դ��������������
	if(nSrcLength & 1)
	{
		*(pDst-2) = 'F';	// ��'F'
		nDstLength++;		// Ŀ�괮���ȼ�1
	}
	
	// ����ַ����Ӹ�������
	*pDst = '\0';
	
	// ����Ŀ���ַ�������
	return nDstLength;
}


// �����ߵ����ַ���ת��Ϊ����˳����ַ���
// �磺"683158812764F8" --> "8613851872468"
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ���ַ�������
unsigned char gsmSerializeNumbers(const unsigned char* pSrc, unsigned char* pDst, unsigned char nSrcLength)
{
	unsigned char nDstLength=0;		// Ŀ���ַ�������
	unsigned char i;
	unsigned char ch;			//  ���ڱ���һ���ַ�
	
	// ���ƴ�����
	nDstLength = nSrcLength;
	
	// �����ߵ�
	for( i=0; i<nSrcLength;i+=2)
	{
		ch = *pSrc++;		// �����ȳ��ֵ��ַ�
		*pDst++ = *pSrc++;	// ���ƺ���ֵ��ַ�
		*pDst++ = ch;		// �����ȳ��ֵ��ַ�
	}
	
	// �����ַ���'F'��
	if(*(pDst-1) == 'F')
	{
		pDst--;
		nDstLength--;		// Ŀ���ַ������ȼ�1
	}
	
	// ����ַ����Ӹ�������
	*pDst = '\0';
	
	// ����Ŀ���ַ�������
	return nDstLength;
}

unsigned char  ascmap(unsigned char *pDst,unsigned char tempchar)
{
	static int ext=0;
	unsigned char n=1;
	if (ext==0)
	{
		if ((tempchar>=' ' && tempchar<='#') || (tempchar>='%' && tempchar<='?') || (tempchar>='A' && tempchar<='Z')|| (tempchar>='a' && tempchar<='z'))
		{
			*pDst=tempchar;
 	
		}
		else if (tempchar==0x00)
		{
			*pDst='@';

		}
		else if (tempchar==0x02)
		{
			*pDst='$';
				
		}
		else if (tempchar==0x03)
		{
			*pDst=0xa3;
			pDst++;	
			*pDst=0xa4;
		
			n=2;
		}
		else if (tempchar==0x1b)
		{
			ext=1;
			n=0	;
		}
		else if (tempchar==0x0a)
		{
			*pDst=tempchar;
		}
		else if (tempchar==0x0d)
		{
			n=0	;
		}
		else
		{
			*pDst='?';
		}
	}
	else
	{
		switch(tempchar)
		{
		case 0x14:
			*pDst='^';
		
			break;
		case 0x28:
			*pDst='{';
		
			break;
		case 0x29:
			*pDst='}';
	
			break;
		case 0x2f:
			*pDst='\\';
		
			break;
		case 0x3c:
			*pDst='[';
		
			break;
		case 0x3d:
			*pDst='~';
		
			break;
		case 0x3e:
			*pDst=']';
		
			break;
		case 0x40:
			*pDst='|';
	
			break;
		default:
			*pDst='?';
		  break;
		}
		ext=0;
	}
	return n;
}



// 7bit����
// pSrc: Դ���봮ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ���봮����
// ����: Ŀ���ַ�������
unsigned char gsmDecode7bit(const unsigned char* pSrc, unsigned char* pDst, unsigned char nSrcLength)
{
	unsigned char nSrc;		// Դ�ַ����ļ���ֵ
	unsigned char nDst;		// Ŀ����봮�ļ���ֵ
	unsigned char nByte;		// ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6
	unsigned char nLeft;	// ��һ�ֽڲ��������

		unsigned char tempchar;
	// ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0;
	
	// �����ֽ���źͲ������ݳ�ʼ��
	nByte = 0;
	nLeft = 0;
	
	// ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�
	// ѭ���ô�����̣�ֱ��Դ���ݱ�������
	// ������鲻��7�ֽڣ�Ҳ����ȷ����
	while(nSrc<nSrcLength)
	{
		// ��Դ�ֽ��ұ߲��������������ӣ�ȥ�����λ���õ�һ��Ŀ������ֽ�
	tempchar= ((*pSrc << nByte) | nLeft) & 0x7f;
		// �޸�Ŀ�괮��ָ��ͼ���ֵ
	
	tempchar=ascmap(pDst, tempchar);
	nDst+=tempchar;	
	pDst+=tempchar;	
		// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
		nLeft = *pSrc >> (7-nByte);

		// �޸��ֽڼ���ֵ
		nByte++;
		
		// ����һ������һ���ֽ�
		if(nByte == 7)
		{
			// ����õ�һ��Ŀ������ֽ�
			tempchar = nLeft;
			tempchar=ascmap(pDst, tempchar);
			nDst+=tempchar;	
			pDst+=tempchar;	
			// �����ֽ���źͲ������ݳ�ʼ��
			nByte = 0;
			nLeft = 0;
		}
		
		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}
	
	// ����ַ����Ӹ�������
	*pDst = '\0';
	
	// ����Ŀ�괮����
	return nDst;
}

// 8bit����
// pSrc: Դ���봮ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ���봮����
// ����: Ŀ���ַ�������
unsigned char gsmDecode8bit(const unsigned char* pSrc, unsigned char* pDst, unsigned char nSrcLength)
{
	// �򵥸���
	memcpy(pDst, pSrc, nSrcLength);
	
	// ����ַ����Ӹ�������
	*pDst = '\0';
	
	return nSrcLength;
}




// UCS2����
// pSrc: Դ���봮ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ���봮����
// ����: Ŀ���ַ�������
unsigned char gsmDecodeUcs2(char * pSrc, unsigned char * pDst, unsigned char nSrcLength)
{
	unsigned char nDstLength=0;		// Ŀ���ַ�������
	unsigned char i;
	unsigned char ch;			//  ���ڱ���һ���ַ�
	
	// ���ƴ�����
	nDstLength = nSrcLength;
	
	// �����ߵ�
	for( i=0; i<nSrcLength;i+=2)
	{
		ch = *pSrc++;		// �����ȳ��ֵ��ַ�
		*pDst++ = *pSrc++;	// ���ƺ���ֵ��ַ�
		*pDst++ = ch;		// �����ȳ��ֵ��ַ�
	}
	
	
	// ����Ŀ���ַ�������
	return nDstLength;
}


/*****************************************************************************
*
*   Function name : gsmDecodePdu
*
*   Returns :       �û���Ϣ������
*
*   Parameters :    pSrc ԴPDU��ָ�� ;prevpdu pdu������Ϣ�ṹ
*
*   Purpose :       PDU����
*
*****************************************************************************/

unsigned char gsmDecodePdu(const unsigned char* pSrcptr,PDUSTRUCT * prevpdu)
{
	unsigned char nDstLength=0;			// Ŀ��PDU������
	unsigned char tmp;		// �ڲ��õ���ʱ�ֽڱ���
	unsigned char buf[256];	// �ڲ��õĻ�����
	unsigned char *pSrc = (unsigned char*)pSrcptr;
	
	// SMSC��ַ��Ϣ��
	gsmString2Bytes(pSrc, &tmp, 2);	// ȡ����
	SCI_TRACE_LOW("smsc length %d",tmp);
	pSrc += 2;			

	
	pSrc += 2;			
	gsmString2Bytes(pSrc, &tmp, 2);	// Ŀ���ֻ��ų���
	if(tmp & 1) tmp += 1;	// ������ż��
	SCI_TRACE_LOW("dest NO length %d",tmp);
	
	pSrc += 4;			
	gsmSerializeNumbers(pSrc, prevpdu->pTPA, tmp);	// ת��Ŀ�ĺ��뵽Ŀ��PDU��
	
	SCI_TRACE_LOW("smsc no %s ",prevpdu->pTPA);
	pSrc += tmp;		// ָ�����
	
	
	// TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��

	gsmString2Bytes(pSrc, (unsigned char*)&tmp, 2);	// ȡЭ���ʶ(TP-PID)
	SCI_TRACE_LOW("id type %d (0x00)",tmp);

	pSrc += 2;		// ָ�����
	
	gsmString2Bytes(pSrc, (unsigned char*)&prevpdu->TP_DCS, 2);	// ȡ���뷽ʽ(TP-DCS)
	SCI_TRACE_LOW("coding type %d ",prevpdu->TP_DCS);

	pSrc += 2;		// ָ�����

	gsmString2Bytes(pSrc, (unsigned char*)&tmp, 2);	// 0xff
	SCI_TRACE_LOW("0xff %02x ",tmp);

	pSrc += 2;		// ָ�����

	
	gsmString2Bytes(pSrc, &tmp, 2);	// �û���Ϣ����(TP-UDL)
	SCI_TRACE_LOW("user info length %d ",tmp);
	
	pSrc += 2;		// ָ�����
	
	if(prevpdu->TP_DCS == GSM_7BIT)	
	{
		// 7-bit����
		//���ٴ���fuck
		#if 1
		nDstLength = gsmString2Bytes(pSrc, buf, tmp & 7 ? (unsigned int)tmp * 7 / 4 + 2 : (unsigned int)tmp * 7 / 4);	// ��ʽת��
		gsmDecode7bit(buf, prevpdu->pTP_UD, nDstLength);	// ת����TP-DU
		nDstLength = tmp;
		#endif 
	}
	else if(prevpdu->TP_DCS == GSM_UCS2)
	{
		// UCS2����
		nDstLength = gsmString2Bytes(pSrc, prevpdu->pTP_UD, tmp * 2);			// ��ʽת��
	}
	else
	{
		// 8-bit����
		nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);			// ��ʽת��
		nDstLength = gsmDecode8bit(buf, prevpdu->pTP_UD, nDstLength);	// ת����TP-DU
	}

#if (0)
	{
		int i;
		for(i=0;i<nDstLength;i++)
			SCI_TRACE_LOW("==%02x",prevpdu->pTP_UD[i]);
	}
#endif /* (0) */
	// ����Ŀ���ַ�������
	return nDstLength;
}




unsigned char SumEncodeUcs2(const unsigned char* pSrc,  unsigned char nSrcLength)
{
	unsigned char nDstLength=0;		// UNICODE���ַ���Ŀ
	unsigned char i;
	unsigned char  highindex;

	for(i=0;i<nSrcLength;)
	{
		highindex=pSrc[i] ;
		if (highindex&0x80)//�����ַ� 
		{
			
			i+=2;			
			nDstLength++;
			
		}
		else //if (highindex>20) //�ж�ΪӢ���ַ�
		{
			i++;
			nDstLength++;	
		}
	}

	return nDstLength * 2;
}




/*****************************************************************************
*
*   Function name : gsmEncodePdu
*
*   Returns :       Ŀ��PDU������
*
*   Parameters :    pDst ������pdu�� sendpdu pdu������Ϣ�ṹ
*
*   Purpose :       PDU����
*
*****************************************************************************/

unsigned int gsmEncodePdu( unsigned char* pDst,PDUSTRUCT * sendpdu)
{
	unsigned char nLength=0;			// �ڲ��õĴ�����
	unsigned int nDstLength=0;			// Ŀ��PDU������
	unsigned char buf[256];	// �ڲ��õĻ�����
	int temp;
	
// SMSC��ַ��Ϣ��  00
	
	// SMSC��ַ��Ϣ����
	nLength = strlen((char *)sendpdu->pSCA);
	buf[0] = (unsigned char)((nLength & 1) == 0 ? nLength : nLength + 1)/ 2 + 1 ;
	SCI_TRACE_LOW("<<<<<< smsc length %d",buf[0]);
	// ת��1���ֽڵ�Ŀ��PDU��
	nDstLength = gsmBytes2String(buf, pDst, 1);

	// ������SMSC��ַ��ʽ
	buf[0] = 0; 	
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 1);	

	// ת��SMSC���뵽Ŀ��PDU��
	nDstLength += gsmInvertNumbers(sendpdu->pSCA, &pDst[nDstLength], nLength);	


	// �յ�1���ֽ�
	buf[0] = 0;
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 1);	
	
	
	// OA��ַ��Ϣ����
	nLength = strlen((char *)sendpdu->pTPA);
	buf[0] = (unsigned char)((nLength & 1) == 0 ? nLength : nLength + 1) ;
	SCI_TRACE_LOW("<<<<<< oa length %d",buf[0]);
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 1);	

	// ������SMSC��ַ��ʽ
	buf[0] = 0; 	
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 1);	


	// ת��OA���뵽Ŀ��PDU��
	nDstLength += gsmInvertNumbers(sendpdu->pTPA, &pDst[nDstLength], nLength);	
	
	// pid type ����
	buf[0] = 0;
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 1);	
	

	// pdu type
	buf[0] = sendpdu->TP_DCS;
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 1);	
	
	// ����ʱ����ַ���(TP_SCTS)
	nDstLength += gsmInvertNumbers(sendpdu->pTP_SCTS, &pDst[nDstLength], 14);

	nLength = sendpdu->pDU_Len;
	buf[0] = nLength;
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 1);
		
	SCI_TRACE_LOW("<<<<<< pdu length %d", sendpdu->pDU_Len);
	memcpy(&buf[0],sendpdu->pTP_UD,sendpdu->pDU_Len);
	nLength = sendpdu->pDU_Len;
	


	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], nLength);	


	SCI_TRACE_LOW("<<<<<<encode data %s",pDst);
	
	// ����Ŀ���ַ�������
	return nDstLength;
}



// ʱ���ɸ�������ʱ���Ϊ������ʱ��   // ���� by ������ for �ֱ�ʱ����ʾ2008/6/13 
void time_trans(SCI_DATE_T *currDate,SCI_TIME_T *currTime)
{
    uint32  year_type = (TM_IsLeapYear(currDate->year)) == SCI_TRUE ? 1 : 0;

	SCI_TRACE_LOW("<<<<<<before year %d mon %d day %d hour %d min %d sec %d",
		currDate->year,currDate->mon,currDate->mday,currTime->hour,currTime->min,currTime->sec
		);
	currTime->hour += 8;
	if(currTime->hour >= 24 ) // ����һ��
	{
		currTime->hour -= 24;
		currDate->mday += 1;
		if(currDate->mday > day_of_month[year_type][currDate->mon - 1])
		{
			currDate->mday = 1;
			currDate->mon += 1;
			if(currDate->mon > 12)
			{
				currDate->mon=1;
				currDate->year += 1;
			}
			
		}
	}

	SCI_TRACE_LOW("<<<<<<after year %d mon %d day %d hour %d min %d sec %d",
		currDate->year,currDate->mon,currDate->mday,currTime->hour,currTime->min,currTime->sec
		);
} 


/****************************************************************
  ��������CheckSum
  ��  ��  �����У��
  ���������pdatasҪУ�������ָ���׵�ַ��len���ݳ���
  ���������1���ֽڵ�У����
  ��д��  ���º���
  �޸ļ�¼��������2006-7-7
****************************************************************/
int  CheckSum( unsigned char*pdatas,int len) 
{
	uint8 checksum=0;	
	uint32 i;

	for (i=0;i<len;i++)
	{
		checksum ^=*pdatas;
		pdatas++;
	}
	
	return checksum;
}

/****************************************************************
  ��������CheckAdd
  ��  ��  ���ۼ�У��
  ���������pdatasҪУ�������ָ���׵�ַ��len���ݳ���
  ���������1���ֽڵ�У����
  ��д��  ���º���
  �޸ļ�¼��������2006-7-7
****************************************************************/
uint8  CheckAdd( unsigned char*pdatas,int len) 
{
	uint8 checksum=0;	
	uint32 i;

	for (i=0;i<len;i++)
	{
		checksum +=*pdatas;
		pdatas++;
	}
	
	return checksum;
}
	

/****************************************************************
	��������SG_HH_Free_Item
	��	��	���ͷŽڵ�
	���������pitem:�ڵ�ָ���ָ��
	�����������
	��д��	�����ɷ�
	�޸ļ�¼��������2005-9-20
****************************************************************/
void SG_HH_Free_Item(sg_hh_list ** pitem)
{
  
  //  SCI_TRACE_LOW( "SG_Send_Free_Item: %p %d %s",*pitem,(*pitem)->len,(*pitem)->msg); 			  
	if (pitem == NULL)
	{
		SCI_TRACE_LOW( "SG_Send_Free_Item: PARA\r\n");			  
		return;
	}   
	  
	if ((*pitem) && (*pitem)->buf)
	{
		SCI_FREE((*pitem)->buf);
		(*pitem)->buf = NULL;
	}
	  
	if (*pitem)
	{
		SCI_FREE(*pitem);
		*pitem = NULL;
	} 	  
  
}
  

/****************************************************************
   ��������SG_HH_Put_Item
   ��  ��  ����һ���±��ķ��͵����ڴ�������
   ���������msg, len: �±���
				 list: ����ͷָ���ָ��
				 num: ���нڵ���Ŀ��ָ��
   �����������
   ����ֵ��0:ʧ��1:�ɹ�
   ��д��  �����ɷ�
   �޸ļ�¼��������2005-9-20
	 ���ϻ��ѹ��ܣ����ɷ壬2005-11-16
****************************************************************/
 int SG_HH_Put_Item(int type, int com, char *msg, int len, sg_hh_list **list, int *num)
 {
	 sg_hh_list *item = NULL, *tail = NULL, *first = NULL;

 
	 if (msg == NULL || len <= 0 || list == NULL || num == NULL)
	 {
		 SCI_TRACE_LOW( "SG_HH_Put_Item: PARA\r\n");	 
		 return 0;
	 }	 
 
	 //�����ڵ�
	 item = SCI_ALLOC(sizeof(sg_hh_list));
	 if (item == NULL)
	 {
		 SCI_TRACE_LOW( "SG_HH_Put_Item: CALLOC: %d\r\n", sizeof(sg_hh_list));	 
		 return 0;
	 }	 
 
	 SCI_MEMSET(item,0, sizeof(sg_hh_list));
	 
	 item->buf = msg;
	 item->len = len;
	 item->com = com;
 //  SCI_TRACE_LOW( "SG_Send_Put_Item: %p %d %s",item,item->len,item->msg);  
 
	 //�������������Ŀ
	 //β������һ����ͷɾ��һ��
	 if (*num < MAX_HH_LIST_NUM)
	 {
		 *num += 1;
	 }
	 else
	 {
		 first = *list;

		 *list = first->next;
		 tail = first;

		 if(tail == NULL){
			 tail = *list;
			 *list = (*list)->next;
		 }	 
		 
		 SG_HH_Free_Item(&tail);
		 tail = NULL;
	 }
 

	if (type == 1)		//��ӽڵ㵽����ĩ
	{
		tail = *list;
		
		while (tail && tail->next)
		{
			tail = tail->next;
		}
		
		if (tail)
			tail->next = item;
		else
			*list = item;
	}
	else if(type == 0)	//Ӧ����ӵ�����ͷ��
	{
		item->next = *list;
		*list = item;
	}

	 
	 return 1;
 }


/****************************************************************
  ��������SG_Send_Put_New_Item
  ��  ��  ����һ���±��ķ��͵��´�������
  ����������±���
  �����������
  ����ֵ��0:ʧ��1:�ɹ�
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
****************************************************************/
int SG_HH_Put_New_Item(int type, int com, unsigned char *buf, int len)
{

	char *msg = NULL;

	msg = SCI_ALLOC(len);
	if (msg == NULL)
	{
		SCI_TRACE_LOW("<<<<<<SG_HH_Put_New_Item CALLOC: %d", len);
		return 0;
	}

	SCI_MEMCPY(msg,buf,len);

	if (SG_HH_Put_Item(type, com, msg, len, &gp_hh_list, &gn_hh_list) == 1)
	{		
		SCI_TRACE_LOW( "========SG_HH_Put_New_Item: MSG LEN: %d, NEW LIST NO: %d", len, gn_hh_list);
		//SG_CreateSignal(SG_SEND_HH,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
		return 1;
	}
	else
	{		
		if(msg)
			SCI_FREE(msg);
		SCI_TRACE_LOW( "SG_HH_Put_New_Item: MSG LEN: %d, NEW LIST NO: %d", len, gn_hh_list);
		return 0;
	}
}

/****************************************************************
  ��������SG_Send_Get_Item
  ��  ��  ���Ӵ������л�ȡһ�������͵Ľڵ�
  ���������list: ����ͷָ���ָ��
  				num: ���нڵ���Ŀ��ָ��
  ��������������͵Ľڵ�
  ��д��  �����ɷ�
  �޸ļ�¼��������2005-9-20
 	 �����޷��͹���� �������ɷ壬2005-11-8
****************************************************************/
sg_hh_list *SG_HH_Get_Item(sg_hh_list **list, int *num)
{
	sg_hh_list *item = NULL;


	//��ͷ����ȡ
	if (list && (*list))
	{
		item = *list;
		*list = (*list)->next;
		*num -= 1;
	}

	return item;
}


/****************************************************************
  ��������	SG_sndmsgby_hh
  ��  ��  ��ͨ�����ڷ�ʽ���ͱ���
  �����������
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void SG_sndmsgby_hh()
{

	if(g_framesendok == 0)
	{
		g_sendframe1s = 2;//�ȴ�
		return;
	}
	
	if (hhItem == NULL)
		hhItem = SG_HH_Get_Item(&gp_hh_list, &gn_hh_list);
			
	if (hhItem != NULL)
	{
		SIO_WritePPPFrame(hhItem->com,(uint8*)hhItem->buf,hhItem->len);
		g_sendframe1s = 2;//1���ʱ
		g_framesendok = 0; //��־λ��0
	}
	
}


/****************************************************************
  ��������	SG_HH_SEND_ITEM
  ��  ��  ��ͨ�����ڷ�ʽ���ͱ���
  �����������
  �����������
  ��д��  ���º���
  �޸ļ�¼������ 2007/8/15
****************************************************************/
void SG_HH_SEND_ITEM(int SignalCode)
{
	xSignalHeaderRec      *signal_ptr = NULL;
	
	if(Log_Enable1 != HH_ENABLE && Log_Enable2 != HH_ENABLE) // �ֱ�������ʱֱ�ӷ���
	{
		return;
	}

	switch(SignalCode)
	{
		case SG_SEND_HH:  //  �нڵ㱻���������
			SG_sndmsgby_hh();
			break;
		case SG_ACK_HH:	// ���յ��ֱ�Ӧ��֮��
			//�ͷŽڵ�
			SG_HH_Free_Item(&hhItem);
			SG_sndmsgby_hh();
			break;
		case SG_SEND_HH_TO: // ���ͳ�ʱ
			//����3�Σ��ж������Ѿ��Ͽ�
			if(++g_sendframecnt > 2)
			{
				SCI_TRACE_LOW(" ====SG_SEND_HH_TO======== ");
				g_framesendok = 1 ;
				g_sendframecnt = 0; 
				SG_HH_Free_Item(&hhItem);
#if (1)
				// free�������������еĶ���
				hhItem = hhItem = SG_HH_Get_Item(&gp_hh_list, &gn_hh_list);
				while(hhItem != NULL)
				{
					SG_HH_Free_Item(&hhItem);
					hhItem = NULL;
					hhItem = SG_HH_Get_Item(&gp_hh_list, &gn_hh_list);
				}
				checkhandset = 0;				
#endif /* (0) */
			}
			else
			{
				SCI_TRACE_LOW("=========== g_sendframecnt is: %d!",g_sendframecnt);
				SG_sndmsgby_hh(); 
			}
			break;
		default:
			SCI_TRACE_LOW("<<<<<<SG_HH_SEND_ITEM other signal !!! %x",SignalCode);
			return;
			break;
	}
}



/*****************************************************************************
*
*   Function name : SendFrame
*
*   Returns :      none
*
*   Parameters :    sendframe
*
*   Purpose :       ��װͨѶ֡�����ŵ����ͻ�����
*
*****************************************************************************/
void SendFrame(SENDFRAME * sendframe)
{

	xSignalHeaderRec    *signal_ptr = NULL;
	unsigned int i,j = 0;
	unsigned char chk;
	unsigned char uctemp;
	unsigned char buf[500] = {0};
	

	//��������
	buf[j++] = (sendframe->type);
	chk=sendframe->type;
		
	//���ͳ���		
	uctemp=sendframe->len>>8;
	buf[j++] = (uctemp);
	chk^=uctemp	 ;
		
	uctemp=sendframe->len&0x00ff;
	buf[j++] = (uctemp);
	chk^=uctemp	;
		
		
	//��������
	for (i=0;i<sendframe->len;i++)
	{
		uctemp=sendframe->dat[i] ;
		buf[j++] = (uctemp);
		chk^=uctemp	;	
	}
		
	//��������У��
	buf[j++] = (chk); 
	
	//�������ԣ��������ڷ���������
	if((g_state_info.TestFlag == 1)&&(sendframe->type < 0x21))
	{
		return;
	}
	
		
	#ifdef _DUART
	if(SG_HH_Put_New_Item(1, COM2, buf, j) == 1)	
	#else
	if(SG_HH_Put_New_Item(1, COM_DEBUG, buf, j) == 1)
	#endif	
	{
		SG_CreateSignal(SG_SEND_HH,0, &signal_ptr); 
	}
			
}

/*****************************************************************************
*
*   Function name : SendReturnFrame
*
*   Returns :      none
*
*   Parameters :    suc 1 �ɹ� 2 ʧ�� 
*
*   Purpose :       ����Ӧ��֡ 
*
*****************************************************************************/


void SendReturnFrame(unsigned char suc)
{
	xSignalHeaderRec    *signal_ptr = NULL;
	unsigned char buf[8] = {0};
	int j = 0;
	
	buf[j++] = REV_OK;
	buf[j++] = 0x00;
	buf[j++] = 0x01;
	buf[j++] = suc;
	buf[j++] = (REV_OK^0x00^0x01^suc);
	
	// head flag and end flag not needed
	#ifdef _DUART
	SIO_WritePPPFrame(COM2,buf,j);	
	#else
	SIO_WritePPPFrame(COM_DEBUG,buf,j);
	#endif

	//if(SG_HH_Put_New_Item(1, COM_DEBUG, buf, j) == 1)
		//SG_CreateSignal(SG_SEND_HH,0, &signal_ptr); 
#if (0)
	{
		int i;
		HAL_DumpPutChar(COM_DATA,0x7e);
		for(i=0;i<j;i++)
			HAL_DumpPutChar(COM_DATA,buf[i]);
		HAL_DumpPutChar(COM_DATA,0x7e);
	}
#endif /* (0) */
	
}



/*****************************************************************************
*
*	Function name : revreturnframe
*
*	Returns :		None
*
*	Parameters :	pdat	-->	ָ���Ӧ���ݵ�ָ��
*				
*
*	Purpose :		����Ӧ֡
*					
*
******************************************************************************/
void SG_RevReturnFrame(unsigned char * pdat)
{
	xSignalHeaderRec    *signal_ptr = NULL;
	
	if ((pdat[0]<<8)|(pdat[1])==1)
	{
		if (pdat[2]==0x01)
		{
			//���ͳɹ�����
			g_framesendok = 1;
			g_sendframecnt = 0;
			SG_CreateSignal(SG_ACK_HH,0, &signal_ptr); 
		}
		else if (pdat[2]==0x02)
		{
			//����ʧ�ܴ���
			g_framesendok = 0;
		}
		else
			SendReturnFrame(2);	
	}
	else
		//��Ӧ���մ���֡
		SendReturnFrame(2);
}



void CheckSendFrame(void)
{
	xSignalHeaderRec    *signal_ptr = NULL;
	//3�η��Ͳ��ɹ������ٷ���
//	if(g_sendframecnt >= 2)
//	{
//		g_framesendok = 1 ;
//		g_sendframecnt = 0;
//	}
//	else
	if(g_sendframe1s==0) //���Ͳ��ɹ������·���
	{
				
		g_framesendok=1;
		SG_CreateSignal(SG_SEND_HH_TO,0, &signal_ptr); 
	}
	
}



/****************************************************************
  ��������SG_Send_Ver
  ��  ��  �����ڻ��������ݽ�������
  �����������
  �����������
  ��д��  ���º���
  �޸ļ�¼��������2007-09-30
****************************************************************/
void SG_Send_Ver(unsigned char * pdat)
{
	//���ݳ���(���ֽ���ǰ)
	if (*(pdat+1)==1)
	{
		//������ȷ��Ӧ֡
		SendReturnFrame(1);
	}
	else
	{
		//���ʹ����Ӧ֡
		SendReturnFrame(2);
	}
}

/****************************************************************
  ��������SG_Send_Call_Handle
  ��  ��  �����ڴ����ֱ��·��Ĳ���绰�����
  �����������
  �����������
  ��д��  ��������
  �޸ļ�¼��������2007-10-22
****************************************************************/
void SG_Send_Call_Handle(unsigned char * msg)
{
	xSignalHeaderRec      *signal_ptr = NULL;
	unsigned short len =0;
	char telnum[15] = "";

	SCI_MEMCPY((unsigned char*)&len,msg,2);
	SCI_TRACE_LOW("<<<<<<The lenth of tel num is %d!!",len);
	
	SCI_MEMCPY(telnum,msg+2,len);

	if(g_set_info.bCallOutDisable==1)
	{
		if( 0 != strcmp(g_state_info.sAllowCall,""))
		{
				
			//	�жϵ�ǰincoming�Ƿ��������
			if (NULL == strstr(g_state_info.sAllowCall,telnum))
			{
				SCI_TRACE_LOW("<<<<<<The Number is been reject:%s",telnum);// ���벻������
				//SG_CreateSignal(SG_SEND_CALLOFF,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
				PhoneAllow = 1;
				//return;
			}
		}
		else
		{
			SCI_TRACE_LOW("THE sAllowCall LIST is NULL");
			SCI_TRACE_LOW("<<<<<<All The Number is been reject:%s",telnum);// ���벻������
			//SG_CreateSignal(SG_SEND_CALLOFF,0, &signal_ptr); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
			PhoneAllow = 1;
			//return;
		}
	}
	

	if(CC_RESULT_SUCCESS != CC_ConnectCall(telnum))
	{
		SCI_TRACE_LOW("<<<<<<SG_Send_Call_Handle:connect No Failure!!");
	}
	else
	{
		SCI_TRACE_LOW("<<<<<<SG_Send_Call_Handle:connect No Successful!!");
	}

}



/****************************************************************
  ��������SG_Send_Call_Handle
  ��  ��  �����ڴ����ֱ��·��Ĳ���绰�����
  �����������
  �����������
  ��д��  ��������
  �޸ļ�¼��������2007-10-22
****************************************************************/
void SG_Set_HandFree_Handle(void)
{

	if(AUD_GetDevMode() == AUD_DEV_MODE_HANDFREE)
	{
		AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
		AUD_SetDevMode(AUD_DEV_MODE_HANDHOLD);
	
	}
	else // ������
	{
		AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
		AUD_SetDevMode(AUD_DEV_MODE_HANDFREE);
	}

}



/****************************************************************
  ��������SG_Send_Call_Handle
  ��  ��  �����ڴ����ֱ��·��Ĳ���绰�����
  �����������
  �����������
  ��д��  ��������
  �޸ļ�¼��������2007-10-22
****************************************************************/
void SG_Send_SMS_Handle(unsigned char * msg)
{
	unsigned short msg_len =0;
	int pos = 0;
	int len;
	MN_SMS_ALPHABET_TYPE_E alphabet_type;
	
	SCI_MEMCPY(&msg_len,&msg[pos],2);
	SCI_TRACE_LOW("<<<<<msg_len = %02x",msg_len);
	pos +=2;

	pos += 3;
	len = gsmDecodePdu(&msg[pos],&g_revpdustruct);
	SCI_TRACE_LOW("<<<<<<pdu LEN = %d",len);
	switch(g_revpdustruct.TP_DCS) // ת���ֱ������ı��뷽ʽ�����صı��뷽ʽ����
	{
		case GSM_7BIT:
			alphabet_type = MN_SMS_DEFAULT_ALPHABET;
			break;
		case GSM_8BIT:
			alphabet_type = MN_SMS_8_BIT_ALPHBET;
			break;
		case GSM_UCS2:
			alphabet_type = MN_SMS_UCS2_ALPHABET;
			break;
		default:
			break;
	}
	//���ű���
	{
		g_sms_save.flag = 1;
		g_sms_save.alphabet_type = alphabet_type;
		g_sms_save.len = len;
		memset(g_sms_save.msg, 0, 200);
		memcpy((char *)g_sms_save.msg, (char *)g_revpdustruct.pTP_UD,len);
		memset(g_sms_save.telenum, 0, 20);
		strcpy((char *)g_sms_save.telenum, (char *)g_revpdustruct.pTPA);
		SCI_TRACE_LOW("======g_sms_save.flag: %d",g_sms_save.flag);
	}
	
 	SendMsgReqToMN(g_revpdustruct.pTPA,g_revpdustruct.pTP_UD,len,alphabet_type);
	
}


void Get_Sms_Rev_Time(void)
{
	SCI_DATE_T  currDate;
	SCI_TIME_T  currTime;

	TM_GetSysDate(&currDate);
	TM_GetSysTime(&currTime);
	time_trans(&currDate,&currTime);
	sprintf((char *)&g_sendpdustruct.pTP_SCTS[0],"%02d",currDate.year-2000);
	sprintf((char *)&g_sendpdustruct.pTP_SCTS[2],"%02d",currDate.mon);
	sprintf((char *)&g_sendpdustruct.pTP_SCTS[4],"%02d",currDate.mday);
	sprintf((char *)&g_sendpdustruct.pTP_SCTS[6],"%02d",currTime.hour);
	sprintf((char *)&g_sendpdustruct.pTP_SCTS[8],"%02d",currTime.min);

}

/****************************************************************
  ��������SG_Send_Call_Handle
  ��  ��  �����ڴ����ֱ��·��Ĳ���绰�����
  �����������
  �����������
  ��д��  ��������
  �޸ļ�¼��������2007-10-22
****************************************************************/
void SG_Rev_SMS_Handle(unsigned char * valid_data,short data_len,	
	MN_SMS_ALPHABET_TYPE_E  alphabet_type,char * destnumber)
{
	char pdubuf[400] = "";
	int pdulen = 0;
	int pos = 0;
	int len = 0;
	char telnum[20] = "";


	SCI_MEMSET(&g_sendpdustruct,0,sizeof(g_sendpdustruct));
	strcpy((char *)g_sendpdustruct.pTPA,destnumber);
	SCI_TRACE_LOW("<<<<<<dest %s datalen %d",
		destnumber,data_len);
	if((*g_sendpdustruct.pTPA == '+')&&(*(g_sendpdustruct.pTPA+1) == '8'))
	{
		memcpy(telnum,g_sendpdustruct.pTPA+3,strlen((char *)g_sendpdustruct.pTPA)-3);
	}
	else if((*g_sendpdustruct.pTPA == '8')&&(*(g_sendpdustruct.pTPA+1) == '6'))
	{
		memcpy(telnum,g_sendpdustruct.pTPA+2,strlen((char *)g_sendpdustruct.pTPA)-2);	
	}
	memset(g_sendpdustruct.pTPA,0,sizeof(g_sendpdustruct));
	strcpy((char *)g_sendpdustruct.pTPA,telnum);

	strcpy((char *)g_sendpdustruct.pSCA,g_set_info.sOwnNo);

	SCI_TRACE_LOW("<<<<<<PTPA %s PSAC %s",g_sendpdustruct.pTPA,g_sendpdustruct.pSCA);
	memcpy(g_sendpdustruct.pTP_UD,valid_data+1,data_len-1); // ��һ���ֽ��ǳ���
	g_sendpdustruct.pDU_Len = data_len - 1;
	Get_Sms_Rev_Time();
	SCI_TRACE_LOW("<<<<<<alphabet_type %d",alphabet_type);
	switch(alphabet_type)
	{
		case MN_SMS_DEFAULT_ALPHABET:
			g_sendpdustruct.TP_DCS = GSM_7BIT;
			break;
		case MN_SMS_8_BIT_ALPHBET:
			g_sendpdustruct.TP_DCS = GSM_8BIT;
			break;
		case MN_SMS_UCS2_ALPHABET:
			g_sendpdustruct.TP_DCS = GSM_UCS2;
			break;
		default:
			break;
	}
	
	len=gsmEncodePdu( (unsigned char *)&pdubuf[0],&g_sendpdustruct);

		SG_Uart_Send_Msg((unsigned char *)pdubuf,len,REV_SMS);
}


/****************************************************************
  ��������SG_Send_Rev_Handle
  ��  ��  �����ڴ����ֱ��·��Ľ������Ǿܽӵ绰����ĺ���
  �����������
  �����������
  ��д��  ��������
  �޸ļ�¼��������2007-10-22
****************************************************************/
void SG_Send_Rev_Handle(unsigned char * msg)
{
	uint8 rev = *(msg + 2);

	if(rev == 0)// �����绰
	{
		ConnectPhoneTimer = 0;
		AUD_StopRing();
		AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����
		AUD_EnableVoiceCodec( 1 );
#ifndef _TRACK
	GPIO_SetValue(HF_MUTE,SCI_FALSE);
#endif
		MNCALL_ConnectCall(CurrCallId,PNULL);
	}
	else if(rev == 1)// �Ҷϵ绰
	{
		// ֱ�ӹҶ�
		if(MN_RETURN_SUCCESS != MNCALL_DisconnectCall(CurrCallId, PNULL))
		{
			SCI_TRACE_LOW("<<<<<<ccapp.c:CC_DisconnectCall() MNCALL_DisconnectCall return failure");
		}
	}

}



/****************************************************************
  ��������SG_Send_DTMF
  ��  ��  �����ڴ����ֱ��·��Ľ������Ǿܽӵ绰����ĺ���
  �����������
  �����������
  ��д��  ��������
  �޸ļ�¼��������2007-10-22
****************************************************************/
void SG_Send_DTMF(unsigned char * msg)
{
	uint8 dtmf;
	int ret;

	if((g_set_info.bNewUart4Alarm & UT_GPS)&&(g_set_info.GpsDevType == 1))
	{
		dtmf = *msg;
	}
	else
	{
		dtmf = *(msg + 2);
	}

	if(PhoneFlag == 2) // ��ͨ�绰�Ժ�
	{
	  	if((ret = MNCALL_StartDTMF(dtmf, CurrCallId)) != MN_RETURN_SUCCESS)
        {
           SCI_TRACE_LOW("<<<<<<Send DTMF err!!! %d",ret);
        }
        //stop send dtmf
        if((ret == MNCALL_StopDTMF(CurrCallId)) != MN_RETURN_SUCCESS)
        {
           SCI_TRACE_LOW("<<<<<<Stop Send DTMF err!!! %d",ret);
        }

	}

}




void SG_Uart_Send_Msg(unsigned char*msg,short len,uint8 type)
{

#ifdef _TRACK
	return;
#endif 

	if((Log_Enable1 != HH_ENABLE && Log_Enable2 != HH_ENABLE)  || (checkhandset == 0))	
	{
		return ;
	}
	
	SCI_MEMSET(&g_sendframe,0,sizeof(g_sendframe));
	g_sendframe.type = type;
	g_sendframe.len = len;
	memcpy(g_sendframe.dat,msg,len);

	SCI_TRACE_LOW("<<<<<<SG_Uart_Send_Msg: TYPE %02x LEN %d",type,len);

	
	SendFrame(&g_sendframe);
	
}
/****************************************************************
  ��������SG_Notify_Hh
  ��  ��  ���Զ��������ý��֪ͨ�ֱ�
  ���������
  ���������
  ��д��  ������֦
  �޸ļ�¼��������2006-2-12
****************************************************************/
void SG_Notify_Hh(void)
{
	unsigned char buf[2];

	if(g_set_info.bAutoAnswerDisable)
		buf[0] = 0;
	else
		buf[0] = 1;

	

	SG_Uart_Send_Msg(buf, 1, RTN_AUTO_REV);	
	
}


void SG_Send_Net_Err(char err)
{	


	unsigned char buf[4] = "";
	
	memset(buf,0,sizeof(buf));
	switch(err)
	{
		case ERR_NO_SIM_CARD:   //��sim��
			buf[2] = 0x01;
			break;

		case ERR_NO_NETWORK:  //������
			buf[2] = 0x03;		
			break;
			
		case ERR_PIN_CODE:  //PIN���д�
			buf[2] = 0x04;		
			break;
			
		default:
			break;
	}
		SG_Uart_Send_Msg(buf,3,RTN_ERR);
}



// ���ֱ����ͻ���״̬ 0x00 ���ж�ʧ 0x01 ���� 0x02 ͨ��
void SG_Send_CallStatus(int status)
{
	unsigned char buf[2] = "";

	buf[0] = status;
	if(g_set_info.bNewUart4Alarm & UT_GPS && g_set_info.GpsDevType == 1)
	{
		if(status == CAll_OFF)
			ZBGpsDev_SendCalloff();
	}

	SG_Uart_Send_Msg(buf,1,REV_TELSTA);

}

void SG_HH_CorrectTime(SCI_DATE_T currDate,SCI_TIME_T  currTime)
{
	char buf[10] = "";
	xSignalHeaderRec      *signal_ptr = NULL;
	
	if(currDate.year < 2005)
		return ;
	if(currDate.mon > 12)
		return ;
	if(currDate.mday > 31)
		return ;
	if(currTime.hour > 23)
		return ;
	if(currTime.min > 59)
		return ;
	if(currTime.sec > 59)
		return ;

	
	buf[0] = currDate.year - 2000;
	buf[1] = currDate.mon;
	buf[2] = currDate.mday;
	buf[3] = currTime.hour;
	buf[4] = currTime.min;
	buf[5] = currTime.sec;
	
	SG_CreateSignal_Para(SG_SEND_TIME,6, &signal_ptr,buf); // �����źŵ���ѭ��Ҫ�����Ͷ����еı���
	

}

void SG_LED_CorrectTime(gps_data *gps)
{

	char ledmsg[64]={0};
	unsigned char buf[10] = "";	
	unsigned char crctmp[64]={0};
	unsigned short CRC;	
	
	uint32  yearType; 
	uint16 year = 0;
	uint8 mon = 0;
	uint8 day = 0;
	uint8 hour = 0;
	
	gps_data *NowTime = (gps_data *)gps;


	if((NowTime->sTime[4] != '0')||(NowTime->sTime[5] != '0')||(NowTime->sTime[3] != '1'))
		return;

	SCI_MEMSET(ledmsg,0,sizeof(ledmsg));

	if(g_set_info.bNewUart4Alarm & UT_HBLED)
	{
		SCI_DATE_T  currDate;
		SCI_TIME_T  currTime;

		TM_GetSysDate(&currDate);
		TM_GetSysTime(&currTime);
		time_trans(&currDate,&currTime);
	
		ledmsg[0]=0x7e;
		ledmsg[1]=0x43;
		ledmsg[2]=0x00;
		ledmsg[3]=0x10;
		strcpy(&ledmsg[6],"TIME");
		ledmsg[10]= currDate.year - 2000;
		ledmsg[11]=	currDate.mon;
		ledmsg[12]= currDate.mday;
		ledmsg[13]=	currDate.wday+1;
		ledmsg[14]=	currTime.hour;
		ledmsg[15]=	currTime.min;
		ledmsg[16]= currTime.sec;
		memcpy(crctmp,&ledmsg[1],16);
		
		CRC = CRC_16(crctmp, 16);
		ledmsg[17] = *((MsgUChar*)(&CRC));
		ledmsg[18] = *((MsgUChar*)(&CRC)+1);

		ledmsg[19]=0x7e;
		SIO_WriteFrame(COM_DEBUG, ledmsg, 20);

		return;
	}

	//ʱ��ת��
	year = 20*100+(NowTime->sDate[4]-'0')*10+(NowTime->sDate[5]-'0');
	yearType = (TM_IsLeapYear(year)) == SCI_TRUE ? 1 : 0;
	mon = (NowTime->sDate[2]-'0')*10+(NowTime->sDate[3]-'0');
	day = (NowTime->sDate[0]-'0')*10+(NowTime->sDate[1]-'0');
	hour = (NowTime->sTime[0]-'0')*10 + (NowTime->sTime[1]-'0') + 8;
	if(hour >= 24)
	{
		hour -= 24;
		day += 1;
		if(day > day_of_month[yearType][mon - 1])
		{
			day = 1;
			mon += 1;
			if(mon > 12)
			{
				mon = 1;
				year += 1;
			}
		}
	}

	ledmsg[0]=0x0a;

	strcpy(&ledmsg[1],"$$00T12345");
	sprintf(&ledmsg[11],"%04d",year);
	sprintf(&ledmsg[15],"%02d",mon);
	sprintf(&ledmsg[17],"%02d",day);
	sprintf(&ledmsg[19],"%02d",hour);
	memcpy(ledmsg+21, NowTime->sTime+2, 2);
	memcpy(ledmsg+23, NowTime->sTime+4, 2);



	strcpy(&ledmsg[25],"&&");
	ledmsg[27]=0x0d;
	if(g_set_info.bNewUart4Alarm & UT_LED)
		SIO_WriteFrame(COM_DEBUG,(unsigned char*)ledmsg,28);
#ifdef _DUART  
	else if(g_set_info.bNewUart4Alarm & UT_LED2)
		SIO_WriteFrame(COM2,(unsigned char*)ledmsg,28);
#endif
}


void SG_CorrectTime_Handle(void)
{
	SCI_DATE_T  currDate;
	SCI_TIME_T  currTime;
	xSignalHeaderRec      *signal_ptr = NULL;
	char sig[2];

	if(g_state_info.pGpsCurrent == NULL)
		return;
	
	if(((gps_data*)g_state_info.pGpsCurrent)->status != 1)
		return;

	if(!CheckTimeOut(HhTimeCorrectTimer))
		return;
	// ÿʮ����Уʱһ��	
	TM_GetSysDate(&currDate);
	TM_GetSysTime(&currTime);
	time_trans(&currDate,&currTime);

	HhTimeCorrectTimer = GetTimeOut(120);
#if (0)
	if(g_set_info.bNewUart4Alarm & (UT_LED2|UT_LED))
		SG_LED_CorrectTime(currDate,currTime);
#endif /* (0) */
	SG_HH_CorrectTime(currDate,currTime); // �ֱ�Уʱ
	

}

void SG_LED_Hand()
{
	if(g_set_info.bNewUart4Alarm & (UT_LED2|UT_LED))
	{
		if((SecTimer == 45) || (SecTimer%1800 == 0))
		{
			unsigned char ledmsg[]={0x0A,0x24,0x26,0x30,0x30,0x30,0x38,0x30,0x30,0x48,0x31,0x32,0x33,0x34,0x35,0x46,0x44,0x39,0x30,0x0D};	
			
			if(g_set_info.bNewUart4Alarm & UT_LED)
				SIO_WriteFrame(COM_DEBUG, ledmsg, sizeof(ledmsg));
			else if(g_set_info.bNewUart4Alarm & UT_LED2)
				SIO_WriteFrame(COM2, ledmsg, sizeof(ledmsg));

			g_state_info.LedTimeCount = 1;
		}

		//��ʱ�ж�
		if(g_state_info.LedTimeCount > 0)
		{
			g_state_info.LedTimeCount++;

			if(g_state_info.LedTimeCount > 10)
			{
				g_state_info.LedOverTime++;

				//3�γ�ʱ�ж�Ϊ����
				if(g_state_info.LedOverTime >= 3)
				{
					g_state_info.LedOverTime = 0;
					g_state_info.LedState = 2;
					g_state_info.LedTimeCount = 0;
				}
				else
				{
					unsigned char ledmsg[]={0x0A,0x24,0x26,0x30,0x30,0x30,0x38,0x30,0x30,0x48,0x31,0x32,0x33,0x34,0x35,0x46,0x44,0x39,0x30,0x0D};	
			
					if(g_set_info.bNewUart4Alarm & UT_LED)
						SIO_WriteFrame(COM_DEBUG, ledmsg, sizeof(ledmsg));
					else if(g_set_info.bNewUart4Alarm & UT_LED2)
						SIO_WriteFrame(COM2, ledmsg, sizeof(ledmsg));

					g_state_info.LedTimeCount = 1;
				}
				
			}
		}
	}
}

void SG_Send_OptrRexlev(char type)
{
	unsigned char buf[8]="";
	static int opter,rxlevelnum;
	char net_status = 0;
	char sig[2] = "";
	
	net_status = SG_Net_Judge();

	switch(net_status)
	{
		case NO_SIM_CARD:
			sig[0] = ERR_NO_SIM_CARD;
			SG_Send_Net_Err(sig[0]);		
			SCI_TRACE_LOW("$$$$$$$$$$$$ IMSI is NULL !!!!");
			break;
		case NO_NET_WORK:
			sig[0] = ERR_NO_NETWORK;
			SG_Send_Net_Err(sig[0]);	
			SCI_TRACE_LOW("$$$$$$$$$$$$ No Network!!!!");
			break;
		default:				
			if(g_state_info.opter == 46000)
				memcpy((char*)buf,"46000",strlen("46000"));
			else if(g_state_info.opter == 46001)
				memcpy((char*)buf,"46001",strlen("46001"));

			buf[5] = g_state_info.rxLevelnum;
			SG_Uart_Send_Msg(buf,6,RTN_NAME_SINGAL);		
			break;
	}

}

void SG_Send_SendSMSok(int result)
{
	unsigned char buf[2]="";

	buf[0] = result;
	SG_Uart_Send_Msg(buf,1,SMS_RESULT);

}

void SG_Adjust_Vol(unsigned char *msg)
{
	unsigned char vol = *(msg +2);

	SG_Set_Cal_CheckSum();

	
	g_set_info.speaker_vol = vol +3;

	SG_Set_Save();
	if(AUD_GetDevMode() == AUD_DEV_MODE_HANDFREE)
	{
		AUD_SetVolume(AUD_DEV_MODE_HANDFREE, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����			
	}
	else if(AUD_GetDevMode() == AUD_DEV_MODE_HANDHOLD)
	{
		AUD_SetVolume(AUD_DEV_MODE_HANDHOLD, AUD_TYPE_VOICE, g_set_info.speaker_vol); // ��������Ϊ����ʱ����			
	}
}


void SG_Send_SpeakVol(void)
{
	unsigned char buf[2] = "";
	buf[0] = g_set_info.speaker_vol -3;	
	SG_Uart_Send_Msg(buf,1,RTN_VAL);
}


void SG_Send_Net_State(char type)
{
	unsigned char buf[2] = "";
	
	buf[0] = type;
	
	SG_Uart_Send_Msg(buf,1,RTN_NWRPT);
}

void SG_Send_Time_HH(xSignalHeaderRec   *receiveSignal)
{
	unsigned char buf[10] = "";

	SCI_MEMCPY(buf, (char*)receiveSignal + 16, 6);

	SG_Uart_Send_Msg(buf,6,RTN_TIME);

}

void SG_Msg_HH(xSignalHeaderRec   *receiveSignal)
{
	uint8 len;
	short msglen;
	char msg[300] = "";
	char tele_num[20] =""; 
	MN_SMS_ALPHABET_TYPE_E type = 0;
	uint8 tele_len;
	
	len = *((char*)receiveSignal + 16);
	type = *((char*)receiveSignal + 17);
	msglen = *((char*)receiveSignal + 18);
	memcpy(msg,(char*)receiveSignal + 19,msglen);
	tele_len = *((char*)receiveSignal + 19+msglen);
	memcpy(tele_num,(char*)receiveSignal + 19+msglen+1,tele_len);

	SCI_TRACE_LOW("<<<<<<SG_Msg_Handle: %d , %d",msglen, tele_len);
	SG_Rev_SMS_Handle((unsigned char *)msg, msglen, type, tele_num);
	
}

void SG_HH_Send_To_Handle(void)
{
	if(Log_Enable1 == HH_ENABLE || Log_Enable2 == HH_ENABLE)
	{
		if(g_framesendok == 0)
			CheckSendFrame();
		
		if(g_sendframe1s)
			g_sendframe1s--;
	}

}
/****************************************************************
  ��������SG_Menu_Send_Computer
  ��  ��  ���������ݵ�PC��
  �����������
  �����������
  ��д��  �����׽�
  �޸ļ�¼��������2008-11-25
****************************************************************/
int SG_Menu_Send_Computer (uint8 type, short len, char *msg)
{

	SCI_MEMSET(&g_sendframe,0,sizeof(g_sendframe));
	g_sendframe.type = type;
	g_sendframe.len = len;
	memcpy(g_sendframe.dat,msg,len);

	SendFrame(&g_sendframe);
}


#ifdef _TRACK
void SG_Xih_Off(void)
{
	char title[200] = {0};
	short len;

	strcpy(title, "\r\nϨ��,�ȴ���...\r\n");
	g_xih_ctrl.XIH_State = SG_MENU_CTRL;
	tryCount = 3;
	Pic_Send(0xa6); // Ϩ�𣬹ر���·
	
	len =strlen(title);
	SG_Menu_Send_Computer(SG_XIH_OFF, len, title);
				
}

void SG_Xih_On(void)
{
	char title[200] = {0};
	short len;

	strcpy(title, "\r\nϨ��ָ�,�ȴ���...\r\n");
	g_xih_ctrl.XIH_State = SG_MENU_CTRL;
	tryCount = 3;
	Pic_Send(0xc5); // Ϩ��ָ�������·
	
	len =strlen(title);
	SG_Menu_Send_Computer(SG_XIH_ON, len, title);
				
}
void SG_Show_Ver(void)
{
	char title[200] = {0};
	short len;

	strcpy(title, "\r\n����汾: ");
			
	strcat(title, g_set_info.sVersion);
				
	strcat(title, "\r\n");

	strcat(title,"Ӳ���ͺ�:G01B\r\n");

	//sprintf(title+strlen(title),"%x\r\n",g_state_info.hardware_ver);

	strcat(title,"�û�����:");

	if(g_state_info.user_type == COMMON_VER)
	{
		strcat(title,"��ͨ�汾\r\n");
	}
	else if(g_state_info.user_type == PRIVATE_CAR_VER)
	{
		strcat(title,"˽�ҳ��汾\r\n");	
	}
	
	len =strlen(title);
	SG_Menu_Send_Computer(SG_SHOW_VER, len, title);
				
}

void SG_Show_Net(void)
{
	char title[200] = {0};
	short len;

	strcpy(title, "\r\nGPRS���Ž׶�:");
	
	switch(g_gprs_t.state)
	{
		case GPRS_ACTIVE_PDP:
			strcat(title,"Active PDP������\n");
			break;
		case GPRS_DEACTIVE_PDP:
			strcat(title,"DeActive PDP������\n");
			break;	
		case GPRS_SOCKET_CONNECT:
			strcat(title,"����SOCKET\n");
			break;
		case GPRS_PORXY:
			strcat(title,"���Ӵ���\n");
			break;
		case GPRS_HANDUP:
			strcat(title,"����\n");
			break;
		case GPRS_RXHEAD:
		case GPRS_RXCONTENT:
			strcat(title,"������������\n");
			break;
		case GPRS_DISCONNECT:
			strcat(title,"���ӶϿ�\n");
			break;
		default:
			break;
			
	}
	
	// �ϴη����Ĵ���
	strcat(title,"\r\n�������:");
	
	switch(g_state_info.lasterr)
	{
		case SG_ERR_NONE:
			strcat(title,"δ��������");
			break;
		case SG_ERR_CREAT_SOCKET:
			strcat(title,"����Socket");
			break;
		case SG_ERR_PROXY_CONNET:
			strcat(title,"����Proxy");
			break;
		case SG_ERR_PROXY_NOTFOUND:
			strcat(title,"��������");
			break;
		case SG_ERR_PROXY_DENY:
			strcat(title,"Proxy �ܾ�");
			break;
		case SG_ERR_CONNET_CENTER:
			strcat(title,"���Ĵ���");
			break;
		case SG_ERR_RCV_HANDUP:
			strcat(title, "����Ӧ��");
			break;
		case SG_ERR_RCV_DATA:
			strcat(title,"��������");
			break;
		case SG_ERR_SEND_DATA:
			strcat(title, "��������");
			break;
		case SG_ERR_RCV_ANS:
			strcat(title, "����Ӧ��");
			break;
		case SG_ERR_TIME_OUT:
			strcat(title, "��ʱ����");
			switch(g_state_info.nNetTimeOut)
			{
				case GPRS_ACTIVE_PDP:
					strcat(title,"Active PDP������");
					break;
				case GPRS_DEACTIVE_PDP:
					strcat(title,"DeActive PDP������");
					break;	
				case GPRS_PORXY:
					strcat(title,"���Ӵ���");
					break;
				case GPRS_HANDUP:
					strcat(title,"����");
					break;
				case GPRS_RXHEAD:
				case GPRS_RXCONTENT:
					strcat(title,"���ձ�������");
					break;
				case GPRS_SOCKET_CONNECT:
					strcat(title,"����SOCKET");
					break;
			}
			strcat(title, "��ʱ!");
			break;
			
		case SG_ERR_CLOSE_SOCKET:
			strcat(title, "�ر�SOCKET");
			break;
	
		case SG_ERR_HOST_PARSE:
			strcat(title, "������ַ����");
			break;
		case SG_ERR_ACTIVE_PDP:
			strcat(title, "����PDP������");
			break;
		case SG_ERR_DEACTIVE_PDP:
			strcat(title, "ȥ��PDP������");
			break;
		case SG_ERR_SET_OPT:
			strcat(title, "��������");
			break;
		default:
			break;
	}

	strcat(title, "\r\n");

	// ��ǰ�ۼ���������
	sprintf(title+strlen(title), "�ۼ���������:%ld��",ReConCnt);
	strcat(title, "\r\n");
	sprintf(title+strlen(title), "����״̬:%d\r\n",g_state_info.plmn_status);
	
	len =strlen(title);
	SG_Menu_Send_Computer(SG_SHOW_NET, len, title);
				
}

void SG_Show_IO(void)
{
	char title[200] = {0};
	char buf[128] = {0};
	short len;

	//�ֶ�����
	if(g_state_info.alarmState & GPS_CAR_STATU_HIJACK)
	{
		sprintf(buf, "\r\n%s%s\r\n", "�ֶ�����:", "��");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "\r\n%s%s\r\n", "�ֶ�����:", "��");
		strcat(title, buf);
	}
	//�Զ���1
	if(!(g_set_info.alarmSet & GPS_CAR_STATU_IO1))// �Զ��屨���ߵ�ƽ����
	{
		strcat(title,"�Զ���1�����˿�:�ߵ�ƽ����\r\n");
	}
	else
	{
		strcat(title,"�Զ���1�����˿�:�͵�ƽ����\r\n");
	}
	
	if(g_state_info.alarmState & GPS_CAR_STATU_IO1)
	{
		strcat(title,"�Զ���1�����˿�:��\r\n");
	}
	else
	{
		strcat(title,"�Զ���1�����˿�:��\r\n");
	}

	//����
	if(g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF)
	{
		sprintf(buf, "%s%s\r\n", "���籨��:", "��");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "���籨��:", "��");
		strcat(title, buf);
	}

	//�Ƽ���
	if(g_state_info.alarmState & GPS_CAR_STATU_JJQ)
	{
		sprintf(buf, "%s%s\r\n", "�Ƽ����ź�:", "��");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "�Ƽ����ź�:", "��");
		strcat(title, buf);
	}

	//ACC
	if(g_state_info.alarmState & GPS_CAR_STATU_GAS_ALARM)
	{
		sprintf(buf, "%s%s\r\n", "ACC�ź�:", "��");
		strcat(title, buf);
	}
	else 
	{
		sprintf(buf, "%s%s\r\n", "ACC�ź�:", "��");
		strcat(title, buf);
	}
		
	//Ƿѹ
	if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE)
	{
		sprintf(buf, "%s%s\r\n", "��ƿ:", "Ƿѹ");
		strcat(title, buf);
	}
	else
	{
		sprintf(buf, "%s%s\r\n", "��ƿ:", "����");
		strcat(title, buf);
	}
	
	len =strlen(title);
	SG_Menu_Send_Computer(SG_SHOW_VER, len, title);
				
}

void SG_Show_State(void)
{
	char title[200] = {0};
	short len;
	char net_status = 0;

	if(((gps_data *)(g_state_info.pGpsCurrent))->status)// �Ѿ���λ
	{
		strcpy(title, "\r\nGPS �Ѷ�λ!\r\n");
	}
	else
	{
		strcpy(title, "\r\nGPS δ��λ!\r\n");
	}
	if(g_state_info.line == 1)
	{
		strcat(title,"û������\r\n");
	}

	sprintf(title+strlen(title), "��ǰ����: %d\r\n",((gps_data *)(g_state_info.pGpsCurrent))->nNum);

					
	if (g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF )
		strcat(title, "����ƽ����!\r\n");

	else if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE )
		strcat(title, "����ƽǷѹ!\r\n");
		
	strcat(title, "����״̬:  ");
	
	if(g_state_info.alarmState & ~(GPS_CAR_STATU_POS|GPS_CAR_STATU_ACC))
	{
		sprintf(title+strlen(title), "0x%08x\r\n",(int)g_state_info.alarmState);
	}
	else
		strcat(title, "����.\r\n");

	if(TRUE == SG_Set_Check_CheckSum()){
		strcat(title, "��������\r\n");
	}
	else{
		strcat(title, "�����쳣\r\n");
	}


	if (g_set_info.bRegisted == 0)
		strcat(title, "�ն�δע��!\r\n");
	
	sprintf(title+strlen(title), "�ۼ���������: %d�� ��������: %d��\r\n",g_set_info.nResetCnt,g_set_info.nReConCnt);

	sprintf(title+strlen(title), "�ϴ�����ԭ�� %d\r\n",Restart_Reason);
	sprintf(title+strlen(title), "��Ӫ�� %d\r\n�ź� %d\r\n",g_state_info.opter,g_state_info.rxLevel);
	strcat(title, "IMEI:");
	strcat(title, g_state_info.IMEI);
	strcat(title, "\r\n");
	//sprintf(title+strlen(title), "IMSI %s\r\n",g_state_info.imsi);
		
	net_status = SG_Net_Judge();

	switch(net_status)
	{
		case NO_SIM_CARD:
			strcat(title,"��SIM��\r\n");
			break;
		case NO_NET_WORK:
			strcat(title,"������\r\n");
			break;
		case GSM_ONLY:
			strcat(title,"ONLY GSM\r\n");	
			break;
		case GPRS_ONLY:
			strcat(title,"ONLY GPRS\r\n");	
			break;
		case NETWORK_OK:
			strcat(title,"��������\r\n");	
			break;
		default:
			break;
	}

	len =strlen(title);
	SG_Menu_Send_Computer(SG_SHOW_STATE, len, title);
				
}


void SG_Show_Device(void)
{
	char title[500] = {0};
	short len;
		
	if (g_set_info.bRegisted == 0)
		strcpy(title, "\r\nδע��!\r\n");
	else
		strcpy(title, "\r\n��ע��!\r\n");
	
	if (g_state_info.alarmState & GPS_CAR_STATU_POWER_OFF )
		strcat(title, "����ƽ����!\r\n");

	else if(g_state_info.alarmState & GPS_CAR_STATU_LESS_PRESSURE )
		strcat(title, "����ƽǷѹ!\r\n");
	
	#if (__cwt_)
	if (temp_s_udpconn == NULL)
		strcat(title, "GPRS��!\r\n");
	else
		strcat(title, "GPRSͨ!\r");
	#else
	if (g_state_info.socket <= 0)
		strcat(title, "GPRS��!\r\n");
	else
		strcat(title, "GPRSͨ!\r\n");
	#endif		
	strcat(title, "������:");
	strcat(title, g_set_info.sOwnNo);
	strcat(title, "\r\n");
	#if(__cwt_)
	strcat(title, "COMMAND ADDR:\r");
	strcat(title, g_set_info.sCommAddr);
	strcat(title, "\r\n");
	
	strcat(title, "��Ȩ��:");
	strcat(title, g_set_info.sOemCode);
	strcat(title, "\r\n");
	#endif
	strcat(title, "���ĺ�:");
	strcat(title, g_set_info.sCenterNo);
	strcat(title, "\r\n");

	strcat(title, "ͨѶ��ʽ:");
	if (g_set_info.nNetType == MSG_NET_SMS)
		strcat(title, "����");
	else if (g_set_info.nNetType == MSG_NET_GPRS)
		strcat(title, "����");
	else if (g_set_info.nNetType == MSG_NET_UNION)
		strcat(title, "���");
	strcat(title, "\r\n");
	
	strcat(title, "����IP:");
	strcat(title, g_set_info.sCenterIp);
	strcat(title, " ");
	sprintf(title+strlen(title), "%d", g_set_info.nCenterPort);
	strcat(title, "\r\n");
	
	strcat(title, "����IP:\r");
	strcat(title, g_set_info.sProxyIp);
	strcat(title, " ");
	sprintf(title+strlen(title), "%d", g_set_info.nProxyPort);
	strcat(title, "\r\n");

	if (g_set_info.bProxyEnable==1)
		strcat(title, "(ʹ�ô���)\r\n");
	else
		strcat(title, "(��ʹ�ô���)\r\n");

	strcat(title, "APN:");
	strcat(title, g_set_info.sAPN);
	strcat(title, "\r\n");	
	strcat(title, "USER:");	
	strcat(title, g_set_info.sGprsUser);
	strcat(title, "\r\n");	
	strcat(title, "PSW:");
	strcat(title, g_set_info.sGprsPsw);	
	strcat(title, "\r\n");

	strcat(title, "��ط�ʽ:");
	if (g_set_info.nWatchType == MSG_WATCH_TIME)
		strcat(title, "��ʱ");
	else if (g_set_info.nWatchType == MSG_WATCH_AMOUNT)
		strcat(title, "����");
	else if (g_set_info.nWatchType == MSG_WATCH_DISTANCE)
		strcat(title, "����");
	else
		strcat(title, "ȡ��");
	strcat(title, "\r\n");	
	
	if (g_set_info.bCompress == 0)
		strcat(title, "(��ѹ��");
	else
		strcat(title, "(��ѹ��");	

	if (g_set_info.nInflexionRedeem == 0)
		strcat(title, ",�޹յ㲹��");
	else
		strcat(title, ",�йյ㲹��");		
	
	if (g_set_info.bStopReport== 0)
		strcat(title, ",ͣ�����㱨)\r");
	else
		strcat(title, ")\r\n");		

	sprintf(title+strlen(title), "��ؼ��:%d\r\n", g_set_info.nWatchInterval);
	if(g_set_info.nWatchType == MSG_WATCH_AMOUNT)
	{

		sprintf(title+strlen(title), "��ش���:%ld\r\n", g_set_info.nWatchTime);
		if(g_set_info.nWatchTime == 0)
		{
			sprintf(title+strlen(title), "�Ѽ�ش���:%ld\r\n", g_watch_info.nWatchedTime);
		}
		else
		{
			sprintf(title+strlen(title), "�Ѽ�ش���:%ld\r\n", g_watch_info.nWatchedTime);
		}
	}
	else
 	{

		sprintf(title+strlen(title), "���ʱ��:%ld\r\n", g_set_info.nWatchTime);
		if(g_set_info.nWatchTime == 0)
		{
			sprintf(title+strlen(title), "�Ѽ��ʱ��:%ld\r\n", g_watch_info.nWatchedTime);
		}
		else
		{
			sprintf(title+strlen(title), "�Ѽ��ʱ��:%ld\r\n", g_watch_info.nWatchedTime);
		}
	}


	if(g_set_info.bRegionEnable)
	{
		sprintf(title+strlen(title),"���򱨾���\r\n");
	}
	
	if(g_set_info.bLineEnable)
	{
		sprintf(title+strlen(title), "·�߱�����\r\n");
	}	
 


	sprintf(title+strlen(title),"��չ����:  0x%08x \r\n ",g_set_info.bNewUart4Alarm);

	len =strlen(title);
	SG_Menu_Send_Computer(SG_SHOW_DEVICE, len, title);
	
}
/****************************************************************
  ��������SG_Show_Device
  ��  ��  ����������
  ���������data   
  �����������
  ��д��  �����׽�
  �޸ļ�¼��������2008-11-25
****************************************************************/
void SG_Set_Device(unsigned char * data)
{
	char *temp_ptr = NULL;
	char *temp_ptr0 = NULL;
	unsigned char len = 0;


	//*ͨѶ��ʽ*����ʹ��*��������*���ĺ���*IP*�˿�*
	
	SG_Set_Cal_CheckSum();
	temp_ptr = strchr((char *)data, '*');
	if(temp_ptr == NULL)
	{
		SCI_TRACE_LOW( "=======data == NULL=====");
		goto err;
	}	
	
	//ͨѶ��ʽ
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;
	temp_ptr = strchr(temp_ptr, '*');
	if(temp_ptr == NULL)
	{
		SCI_TRACE_LOW( "=======nNetType1=====");
		goto err;
	}	
	g_set_info.nNetType = temp_ptr0[0]-'0';
	
	if((g_set_info.nNetType != MSG_NET_UNION)&&(g_set_info.nNetType != MSG_NET_GPRS)&&(g_set_info.nNetType != MSG_NET_SMS))
	{
		SCI_TRACE_LOW( "=======nNetType2=====");
		goto err;
	}

	//����ʹ��
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;
	temp_ptr = strchr(temp_ptr, '*');
	if(temp_ptr == NULL)
	{
		SCI_TRACE_LOW( "=======g_set_info.nProxyPort1=====");
		goto err;
	}	

	if((temp_ptr-temp_ptr0) == 1)
	{
		g_set_info.bProxyEnable = temp_ptr0[0]-'0';
		
		memset(g_set_info.sAPN,0,sizeof(g_set_info.sOwnNo));
		memset(g_set_info.sGprsUser,0,sizeof(g_set_info.sOwnNo));
		memset(g_set_info.sGprsPsw,0,sizeof(g_set_info.sOwnNo));

		
		if(g_set_info.bProxyEnable)
		{
			g_set_info.nProxyPort = 80;
			memset(g_set_info.sProxyIp,0,sizeof(g_set_info.sOwnNo));
			strcpy(g_set_info.sProxyIp,"10.0.0.172");	
			
			strcpy(g_set_info.sAPN, "cmwap");
			strcpy(g_set_info.sGprsUser, "cmwap");
			strcpy(g_set_info.sGprsPsw, "cmwap");
		}
		else
		{
			strcpy(g_set_info.sAPN, "cmnet");
			strcpy(g_set_info.sGprsUser, "cmnet");
			strcpy(g_set_info.sGprsPsw, "cmnet"); 
		}
	}
	else
	{
		SCI_TRACE_LOW( "=======g_set_info.bProxyEnable=====");
		goto err;
	}
	
	//������
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;
	temp_ptr = strchr(temp_ptr, '*');
	if(temp_ptr == NULL)
	{
		SCI_TRACE_LOW( "=======sOwnNo1=====");
		goto err;
	}	

	if (temp_ptr-temp_ptr0-1 > SG_PHONE_LEN)
	{
		SCI_TRACE_LOW( "=======sOwnNo2====");
		goto err;
	}	
	memset(g_set_info.sOwnNo,0,sizeof(g_set_info.sOwnNo));
	strncpy(g_set_info.sOwnNo, temp_ptr0, temp_ptr-temp_ptr0);

	if(g_set_info.sOwnNo[0] == '0' && strlen(g_set_info.sOwnNo) == 1)
	{
		memset(g_set_info.sOwnNo,0,sizeof(g_set_info.sOwnNo));
		g_set_info.bRegisted = 0;
	}
	else
	{
		g_set_info.bRegisted = 1;		//�����ն��Ѿ��Ǽ�
	}

	//���ĺ�
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;
	temp_ptr = strchr(temp_ptr, '*');
	if(temp_ptr == NULL)
	{
		SCI_TRACE_LOW( "=======sCenterNo1=====");
		goto err;
	}	

	if (temp_ptr-temp_ptr0-1 > SG_PHONE_LEN)
	{
		SCI_TRACE_LOW( "=======sCenterNo2====");
		goto err;
	}	
	memset(g_set_info.sCenterNo,0,sizeof(g_set_info.sCenterNo));
	strncpy(g_set_info.sCenterNo, temp_ptr0, temp_ptr-temp_ptr0);


	//���ĵ�ַ
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;
	temp_ptr = strchr(temp_ptr, '*');
	if(temp_ptr == NULL)
	{
		SCI_TRACE_LOW( "=======g_set_info.sCenterIp1=====");
		goto err;
	}	
	
	if ((temp_ptr-temp_ptr0) <= 16)
	{
		char sIp[3+1] = "";
		int nIp;
		char s[SG_IP_LEN] ="" ;

		memset(g_set_info.sCenterIp,0,sizeof(g_set_info.sCenterIp));
		strncpy(g_set_info.sCenterIp, temp_ptr0, temp_ptr-temp_ptr0);

		
	}	
	else
	{
		SCI_TRACE_LOW( "=======g_set_info.sCenterIp2====");
		goto err;
	}

	//���Ķ˿�
	temp_ptr += 1;
	temp_ptr0 = temp_ptr;
	temp_ptr = strchr(temp_ptr, '*');
	if(temp_ptr == NULL)
	{
		SCI_TRACE_LOW( "=======g_set_info.nCenterPort1=====");
		goto err;
	}	
	
 	if((temp_ptr-temp_ptr0) <=SG_PORT_LEN)
 	{
		char s1[SG_PORT_LEN+1] ="" ;
		strncpy(s1, temp_ptr0, temp_ptr-temp_ptr0);
		g_set_info.nCenterPort = atoi(s1);
		SCI_TRACE_LOW( "=======g_set_info.nCenterPort:%d",g_set_info.nCenterPort);
	}
	else
	{
		SCI_TRACE_LOW( "=======g_set_info.nCenterPort2=====");
		goto err;
	}
	//ר��APN����
	if(temp_ptr[1] != '$')
	{
		//APN
		temp_ptr += 1;
		temp_ptr0 = temp_ptr;
		temp_ptr = strchr(temp_ptr, '*');
		if(temp_ptr == NULL)
		{
			goto err;
		}	
		if (temp_ptr-temp_ptr0 != 0)
		{
			memset(g_set_info.sAPN,0,sizeof(g_set_info.sAPN));
			strncpy((char*)g_set_info.sAPN, temp_ptr0, temp_ptr-temp_ptr0);
		}	
		else
		{
			goto err;
		}
		//�û���
		temp_ptr += 1;
		temp_ptr0 = temp_ptr;
		temp_ptr = strchr(temp_ptr, '*');
		if(temp_ptr == NULL)
		{
			goto err;
		}	
		if (temp_ptr-temp_ptr0 != 0)
		{
			memset(g_set_info.sGprsUser,0,sizeof(g_set_info.sGprsUser));
			strncpy((char*)g_set_info.sGprsUser, temp_ptr0, temp_ptr-temp_ptr0);
			if(g_set_info.sGprsUser[0] == '0' && strlen(g_set_info.sGprsUser) == 1)
			{
				memset(g_set_info.sGprsUser, 0, sizeof(g_set_info.sGprsUser));
			}
			
		}	
		else
		{
			
			goto err;
		}
		//����
		temp_ptr += 1;
		temp_ptr0 = temp_ptr;
		temp_ptr = strchr(temp_ptr, '*');
		if(temp_ptr == NULL)
		{
			goto err;
		}	
		if (temp_ptr-temp_ptr0 != 0)
		{
			memset(g_set_info.sGprsPsw,0,sizeof(g_set_info.sGprsPsw));
			strncpy((char*)g_set_info.sGprsPsw, temp_ptr0, temp_ptr-temp_ptr0);
			if(g_set_info.sGprsPsw[0] == '0' && strlen(g_set_info.sGprsPsw) == 1)
			{
				memset(g_set_info.sGprsPsw, 0, sizeof(g_set_info.sGprsPsw));
			}
		
		}	
		else
		{
			goto err;
		}
	}

	SG_Set_Save();
	SG_Net_Disconnect();

err:
	SG_Show_Device();
	
}
/****************************************************************
  ��������SG_PC_State
  ��  ��  ������״̬
  �����������
  �����������
  ��д��  �����׽�
  �޸ļ�¼��������2008-11-25
****************************************************************/
void SG_PC_State(void)
{

	gps_data* gps = (gps_data*) g_state_info.pGpsCurrent;
	
	char title[20] = "";
	short len = 0;

	title[len] = gps->nNum;
	len += 1;
	title[len] = g_state_info.rxLevel;
	len += 1;

	SG_Menu_Send_Computer(SG_PC_STATE, len, title);
				
}

/****************************************************************
  ��������SG_PC_CallOff
  ��  ��  ������绰����
  �����������
  �����������
  ��д��  �����׽�
  �޸ļ�¼��������2008-11-25
****************************************************************/
void SG_PC_CallOff(uint8 tmep)
{
	char title[20] = "";
	short len = 0;

	title[len] = tmep;
	len += 1;

	SG_Menu_Send_Computer(SG_PC_CALL, len, title);

}


/****************************************************************
  ��������SG_PC_Call
  ��  ��  ������绰
  �����������
  �����������
  ��д��  �����׽�
  �޸ļ�¼��������2008-11-25
****************************************************************/
void SG_PC_Call(void)
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

				
}

/****************************************************************
  ��������SG_PC_Ver
  ��  ��  ���汾��ѯ
  �����������
  �����������
  ��д��  �����׽�
  �޸ļ�¼��������2008-11-25
****************************************************************/
void SG_PC_Ver(void)
{
	char title[10] = "";
	short len = 0;


	SCI_MEMCPY(title, g_set_info.sVersion+5, 4)
	len += 4;

	SG_Menu_Send_Computer(SG_PC_VER, len, title);
				
}

/****************************************************************
  ��������SG_PC_Io
  ��  ��  ��IO���
  �����������
  �����������
  ��д��  �����׽�
  �޸ļ�¼��������2008-11-25
****************************************************************/
void SG_PC_Io(void)
{
	char title[10] = "";
	short len = 0;


	//����
	if(HAL_GetGPIOVal(CPU_DD_IN) == 0)
	{
		title[len] = 0x01;
	}
	else
	{
		title[len] = 0x00;
	}
	len += 1;

	//Ƿѹ
	if(HAL_GetGPIOVal(VCAR_PFO) == 0)
	{
		title[len] = 0x01;
	}
	else
	{
		title[len] = 0x00;
	}
	len += 1;

	//ACC
	if(HAL_GetGPIOVal(CPU_ACC_IN) == 0)
	{
		title[len] = 0x01;
	}
	else
	{
		title[len] = 0x00;
	}
	len += 1;

	//�Ƽ���
	if(HAL_GetGPIOVal(CPU_JJQ_IN) == 0)
	{
		title[len] = 0x01;
	}
	else
	{
		title[len] = 0x00;
	}
	len += 1;

	//�ֶ�����
	if(HAL_GetGPIOVal(CPU_SDBJ_IN) == 0)
	{
		title[len] = 0x01;
	}
	else
	{
		title[len] = 0x00;
	}
	len += 1;

	//�Զ���
	if(HAL_GetGPIOVal(CPU_CMKG_IN) == 0)
	{
		title[len] = 0x01;
	}
	else
	{
		title[len] = 0x00;
	}
	len += 1;

	SG_Menu_Send_Computer(SG_PC_IO, len, title);
				
}
/****************************************************************
  ��������SG_PC_XHOff
  ��  ��  ��Ϩ��ָ�
  �����������
  �����������
  ��д��  �����׽�
  �޸ļ�¼��������2008-11-25
****************************************************************/
void SG_PC_XHOff(void)
{

	g_xih_ctrl.XIH_State = SG_MENU_CTRL;
	tryCount = 3;
	Pic_Send(0xc5); // Ϩ��ָ�������·
				
}

/****************************************************************
  ��������SG_PC_XHOn
  ��  ��  ��Ϩ��
  �����������
  �����������
  ��д��  �����׽�
  �޸ļ�¼��������2008-11-25
****************************************************************/
void SG_PC_XHOn(void)
{

	g_xih_ctrl.XIH_State = SG_MENU_CTRL;
	tryCount = 3;
	Pic_Send(0xa6); // Ϩ�𣬹ر���·
				
}

/****************************************************************
  ��������SG_PC_Flag
  ��  ��  ���������Ա�־
  �����������
  �����������
  ��д��  �����׽�
  �޸ļ�¼��������2008-11-25
****************************************************************/
void SG_PC_Flag(void)
{
	char title[20] = "";
	short len = 0;

	g_state_info.TestFlag = 1;
	title[len] = 0x01;
	len += 1;

	SG_Menu_Send_Computer(SG_PC_FLAG, len, title);
				
}
#endif /* (0) */

/****************************************************************
  ��������DoUart
  ��  ��  �����ڻ��������ݽ�������
  �����������
  �����������
  ��д��  ���º���
  �޸ļ�¼��������2007-09-30
****************************************************************/
void DoUart()
{
	xSignalHeaderRec      *signal_ptr = NULL;	
	
	#ifdef _DUART
	unsigned char *p = Uart_Parse_COM2;	
	#else
	unsigned char *p = Uart_Parse;	
	#endif
#if (0)
	{
		int i;
		SCI_TRACE_LOW("++++DoUart *P %d LEN %d",*p,UartParseCnt_2);
		for(i=0;i<UartParseCnt_2;i++)
		{
			SCI_TRACE_LOW("-%d",Uart_Parse_COM2[i]);
		}
	}
#endif /* (0) */


	switch(*p)
	{
	case REV_OK:
		SG_RevReturnFrame(p+1);
		break;

	case SEND_BROWSER: // ��̬�˵�
		SendReturnFrame(1);
		if(0 == setmenu_status) // ��ͨ״̬������Ϊ�����˵�
		{
			SG_Menu_Set_MenuList(0);
		}
		else
		{
			SG_Menu_Set_MenuList(1);
		}
		SG_DynMenu_HandleMsg(p+3);
		break;

	case ASK_NAME_SINGAL:
		SendReturnFrame(1);
		SG_Send_OptrRexlev(0);

		break;

	case ASK_VAL:
		SendReturnFrame(1);
		SG_Send_SpeakVol();
		//SG_CreateSignal(SG_SEND_SPEAKVOL,0, &signal_ptr);
//		SG_Send_SpeakVol();
		break;

	case SEND_CALL:  // ���Ⲧ��绰
		SendReturnFrame(1);
		SG_Send_Call_Handle(p+1);
		break;

	case SEND_REV:  // �������Ҷ�
		SendReturnFrame(1);
		SG_Send_Rev_Handle(p+1);
		break;

	case SEND_DTMF:
		SendReturnFrame(1);
		SG_Send_DTMF(p+1);
		break;

	case SEND_VAL: // ��������
		SendReturnFrame(1);
		SG_Adjust_Vol(p+1);
		break;
		
	case SEND_SMS:			// ���Ͷ���
		SendReturnFrame(1);
		SG_Send_SMS_Handle(p+1);
		break;

	case COPY_TEL_BOOK:
		SendReturnFrame(1);
		break;

	case QUERY_TEL_BOOK:
		SendReturnFrame(1);
		break;

	case GET_TEL_BOOK:
		SendReturnFrame(1);
		break;

	case SEND_INSCH:
		SG_Send_Ver(p+1);
		SCI_TRACE_LOW("==========SEND_INSCH===========");
		break;

	case SEND_INIT:
		SendReturnFrame(1);
		break;

	case SEND_RESET:
		SendReturnFrame(1);
		break;

	case SEND_MT:
		SendReturnFrame(1);
		SG_Set_HandFree_Handle();
		break;

	case SEND_GOOD:
		SendReturnFrame(1);
		break;

	case PASS_KEY_MIANTI:
		SendReturnFrame(1);
		break;

	case SG2000_HH_EXIT:
		SendReturnFrame(1);
		SG_Return_LOG();
		break;
	case SG2000_MB_TEST:
		SendReturnFrame(1);
		break;

	case SG2000_MB_CONFIG:
		SendReturnFrame(1);
		SG_Menu_Set_MenuList(1);
		//SG_DynMenu_HandleMsg(p+3);
		setmenu_status = 1;
		break;

	case MUSTER_OK:
		SendReturnFrame(1);
		break;

	case MUSTER_CANCEL:
		SendReturnFrame(1);
		break;

#ifdef _TRACK

	//���ò�ѯ
	case SG_SHOW_DEVICE:
		SG_Show_Device();
		break;
	//��������
	case SG_SET_DEVICE:
		SG_Set_Device(p+3);
		break;
	//�汾��ѯ
	case SG_SHOW_VER:
		SG_Show_Ver();
		break;
	//����״̬
	case SG_SHOW_STATE:
		SG_Show_State();
		break;
	//IO״̬
	case SG_SHOW_IO:
		SG_Show_IO();
		break;
	//Ϩ��
	case SG_XIH_OFF:
		SG_Xih_Off();
		break;
	//Ϩ��ָ�
	case SG_XIH_ON:
		SG_Xih_On();
		break;
	//����״̬
	case SG_SHOW_NET:
		SG_Show_Net();
		break;


	 //��������
	case SG_PC_FLAG:
		SG_PC_Flag();
		break;
	//Ϩ��
	case SG_PC_XHON:
		SG_PC_XHOn();
		break;
	//Ϩ��ָ�
	case SG_PC_XHOFF:
		SG_PC_XHOff();
		break;
	//IO���
	case SG_PC_IO:
		SG_PC_Io();
		break;
	//�汾��ѯ	
	case SG_PC_VER:
		SG_PC_Ver();
		break;
	//����绰
	case SG_PC_CALL:
		{
			uint8 len;

			if(g_state_info.SimFlag == 0)
			{
				SG_PC_CallOff(0);
				return;
			}


			len = p[2];
			strncpy(g_state_info.sListenOutNo, (char*)p+3, len);
			SG_PC_Call();
	
		}
		break;
	//����״̬
	case SG_PC_STATE:
		SG_PC_State();
		break;

#endif
	default:
		break;

	}
}

