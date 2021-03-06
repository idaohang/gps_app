/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_Crc16.c
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2004-6-29
  内容描述：SG2000 报文CRC校验处理          
  修改记录：
***************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SG_Crc16.h"

static unsigned long crc32_table[0xff];
const unsigned short cnCRC_CCITT = 0x1021;
unsigned long Table_CRC[256]; // CRC 表

/****************************************************************
  函数名  Msg_Reflect
  功  能  ：初始化的时候给Init_CRC32_Table() 来调用的
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2004-6-29
****************************************************************/
unsigned long  Msg_Reflect(unsigned long ref, unsigned char ch) 
{
    unsigned long value=0; 
	unsigned int i;
	// Swap bit 0 for bit 7 
	// bit 1 for bit 6, etc. 
	for(i = 1; i < (ch + 1); i++) 
	{ 
		if(ref & 1) 
			value |= 1 << (ch - i); 
		ref >>= 1; 
	} 
	return value; 
} 

/****************************************************************
  函数名  Msg_Init_CRC32_Table
  功  能  ：初始化一个CRC32表,你只需要调用一次
  输入参数：
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2004-6-29
****************************************************************/
void Msg_Init_CRC32_Table() 
{// Call this function only once to initialize the CRC table. 
	
	// This is the official polynomial used by CRC-32 
	// in PKZip, WinZip and Ethernet. 
	unsigned long ulPolynomial = 0x04c11db7; 
	unsigned long i;
	unsigned long j;
	// 256 values representing ASCII character codes. 
	for(i = 0; i <= 0xFF; i++) 
	{ 
		crc32_table[i]=Msg_Reflect(i, 8) << 24; 
		for (j = 0; j < 8; j++) 
			crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0); 
		crc32_table[i] = Msg_Reflect(crc32_table[i], 32); 
	} 
} 

/****************************************************************
  函数名  Msg_Get_CRC
  功  能  ：取CRC32信息
  输入参数：
			text:输入字符串
  输出参数：
  编写者  ：李松峰
  修改记录：创建，2004-6-29
****************************************************************/
unsigned int Msg_Get_CRC(unsigned char *text, unsigned int  len) 
{ // Pass a text string to this function and it will return the CRC.

	// Once the lookup table has been filled in by the two functions above, 
	// this function creates all CRCs using only the lookup table. 
	// Note that CString is an MFC class. 
	// If you don't have MFC, use the function below instead. 
	
	// Be sure to use unsigned variables, 
	// because negative values introduce high bits 
	// where zero bits are required. 
	
	// Start out with all bits set high. 
	unsigned long   ulCRC=0xffffffff; 
	unsigned char* buffer; 
	
	// Save the text in the buffer. 
	buffer = text; 
	// Perform the algorithm on each character 
	// in the string, using the lookup table values. 
	while(len--) 
		ulCRC = (ulCRC >> 8) ^ crc32_table[(ulCRC & 0xFF) ^ *buffer++]; 
	// Exclusive OR the result with the beginning value. 
	return ulCRC ^ 0xffffffff; 
} 


static unsigned char auchCRCHi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
		0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
		0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
		0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
		0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
		0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
		0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
		0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
		0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
		0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
		0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
		0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 		
		0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 		
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 		
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40		
};

static unsigned char auchCRCLo[] = {	
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 		
		0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 		
		0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 		
		0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 		
		0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 		
		0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 		
		0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 		
		0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 		
		0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 		
		0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 		
		0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 		
		0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 		
		0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 		
		0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 		
		0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 		
		0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 		
		0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 		
		0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 		
		0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 		
		0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 		
		0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 		
		0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 		
		0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 		
		0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 		
		0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 		
		0x43, 0x83, 0x41, 0x81, 0x80, 0x40		
};

/****************************************************************
  函数名  Msg_Get_CRC16
  功  能  ：取CRC16信息
  输入参数：puchMsg:输入字符串
  				unDataLen:输入长度
  输出参数：CRC16校验值（16bit）
  编写者  ：李松峰
  修改记录：创建，2004-6-29
****************************************************************/
unsigned short Msg_Get_CRC16(const unsigned char * puchMsg, unsigned long unDataLen)
{
	unsigned char uchCRCHi = 0xFF ; 	
	unsigned char uchCRCLo = 0xFF ; 	
	unsigned uIndex ;
	
	while (unDataLen--) 
	{		
		uIndex = uchCRCHi ^ *puchMsg++ ;		
		uchCRCHi = (unsigned char)(uchCRCLo ^ auchCRCHi[uIndex]);		
		uchCRCLo = (unsigned char)(auchCRCLo[uIndex]);		
	}
	
	return (unsigned short)(uchCRCHi << 8 | uchCRCLo) ;
}


void BuildTable16( unsigned short aPoly )
{
    unsigned short i, j;
    unsigned short nData;
    unsigned short nAccum;

    for ( i = 0; i < 256; i++ )
    {

        nData = ( unsigned short )( i << 8 );
        nAccum = 0;

        for ( j = 0; j < 8; j++ )
        {
            if (( nData ^ nAccum ) & 0x8000 )  nAccum = ( nAccum << 1 ) ^ aPoly;
            else
                nAccum <<= 1;
            nData <<= 1;
        }
        Table_CRC[i] = ( unsigned long )nAccum;

    }
}

// 计算 16 位 CRC 值，CRC-16 或 CRC-CCITT

unsigned short CRC_16( unsigned char * aData, unsigned long aSize )
{
    unsigned long i;
    unsigned short nAccum = 0;

// BuildTable16( cnCRC_16 );  // CRC_16
 BuildTable16( cnCRC_CCITT ); // CRC_CCITT



    for ( i = 0; i < aSize; i++ ){

  nAccum = ( nAccum << 8 ) ^ ( unsigned short )Table_CRC[( nAccum >> 8 ) ^ *aData++];

 }

    return nAccum;
}

