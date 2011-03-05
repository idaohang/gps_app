/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2008 版权所有
  文件名 ：SG_Sms.h
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-7-23
  内容描述：
  修改记录：
***************************************************************/
#ifndef _SG_SMS_H_
#define _SG_SMS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sci_types.h"
#include "mn_type.h"


/********************************************************************
*******************************宏定义********************************
********************************************************************/

// translate the struct from MMI_PARTY_NUM_T to MN_CALL_NUM_T
#define    MMIPARTYNUM_2_MNCALLEDNUM( _MMI_PARTY_NUM, _MN_CALL_NUM_PTR ) \
            {\
            _MN_CALL_NUM_PTR->number_type = _MMI_PARTY_NUM.number_type ;\
			SCI_TRACE_LOW("MMIPARTYNUM_2_MNCALLEDNUM: _MMI_PARTY_NUM.num_len = %d", _MMI_PARTY_NUM.num_len);\
            _MN_CALL_NUM_PTR->num_len = MIN( _MMI_PARTY_NUM.num_len , MN_MAX_ADDR_BCD_LEN );\
            SCI_MEMCPY((void*)_MN_CALL_NUM_PTR->party_num,\
            (void*)_MMI_PARTY_NUM.bcd_num ,\
            _MN_CALL_NUM_PTR->num_len );\
			SCI_TRACE_LOW("MMIPARTYNUM_2_MNCALLEDNUM: SCI_MEMCPY length = %d", _MN_CALL_NUM_PTR->num_len);\
            }



/********************************************************************
*******************************结构体定义****************************
********************************************************************/
// 已解码的接收短信存放的结构体
typedef struct _APP_SMS_USER_DATA_T
{
	MN_SMS_USER_DATA_HEAD_T  user_data_head_t;     // the head sturcture
	MN_SMS_USER_VALID_DATA_T user_valid_data_t;    // the used valid data sructure
} APP_SMS_USER_DATA_T; 

#define SMS_BUF_SIZE 1024

typedef struct _SG_SMS_T
{
	uint8  sms_ide[3];
	uint32 sms_len;
	uint8  sms_buf[SMS_BUF_SIZE];
}SG_SMS_T;


/********************************************************************
*******************************函数声明****************************
********************************************************************/

// 设置短信存储优先为ME
void SG_Sms_Init(void);

// 短信相关业务的callback函数
void MnSms_EventCallback( 
								uint32 task_id, //task ID
								uint32 event_id, //event ID
								void *param_ptr
								);

//从ascII码转换为短信的7bit默认编码方式
unsigned char MMI_Ascii2default(
    uint8             *ascii_ptr,
    uint8             *default_ptr,
    uint16            str_len);

//向外发送短信的处理函数
ERR_MNSMS_CODE_E SendMsgReqToMN(uint8 *dest_number_ptr,uint8 *sms_context_ptr,uint sms_context_len,MN_SMS_ALPHABET_TYPE_E alphabet_type);



#endif //_SG_SMS_H_
