#ifndef _MMI_DESCONTROL_H_
#define _MMI_DESCONTROL_H_

#include "sim_dphone_stk_verify.h"
#include "mn_type.h"
#include "Nv_item_id.h"

#define MN_NV_USER_BASE                 500
#define MAX_NV_USER_ITEM_NUM            1500
#define MN_SMS_IN_NV_MAX_NUM        200      // the max sms num in NV

#define NV_ASSERT_NUM      10
#define NV_ASSERT_BASE     (NV_ITEM_ID_E)(NV_ASSERT_INFO0)
#define NV_ASSERT_END      (NV_ASSERT_BASE + NV_ASSERT_NUM)

/*
typedef enum
{
	PUK2_PROCESS_RETURN = 0,
	NOTMATCH_DESCARD,
	NORMAL_RETURN,
	MAX_RETURN_NUM
}DPHONE_STK_PROCESS_ERROR_E;
*/
#define   DIALBCD_0                 0x0
#define   DIALBCD_1                 0x1
#define   DIALBCD_2                 0x2
#define   DIALBCD_3                 0x3
#define   DIALBCD_4                 0x4
#define   DIALBCD_5                 0x5
#define   DIALBCD_6                 0x6
#define   DIALBCD_7                 0x7
#define   DIALBCD_8                 0x8
#define   DIALBCD_9                 0x9
#define   DIALBCD_STAR              0xA
#define   DIALBCD_HASH              0xB
#define   DIALBCD_PAUSE             0xC
#define   DIALBCD_WILD              0xD
#define   DIALBCD_EXPANSION         0xE
#define   DIALBCD_FILLER            0xF

// 定义各个存储在NV中的资料的最大记录数
#define MMINV_MAX_REGION_RECORDS       	150
#define MMINV_MAX_LINE_RECORDS    		150
#define MMINV_MAX_BLACKBOX_RECORDS    	100
/*
 * BCD format - LSB first (1234 = 0x21 0x43), MSB first (1234 = 0x12, 0x34)
 * or unpacked (1 digit per byte).
 */
#define PACKED_LSB_FIRST  0  // LSB first (1234 = 0x21, 0x43)
#define PACKED_MSB_FIRST  1  // MSB first (1234 = 0x12, 0x34)
#define UNPACKED          2  // unpacked  (1234 = 0x01, 0x02, 0x03, 0x04)

typedef enum

{
    MMI_SMS_TYPE = MN_NV_USER_BASE,   // NV_USER_BASE = 500
    MMINV_3DES_KM  ,    //the km of 3des
	MMINV_IMEI_NUM  ,//the des card imei num
	MMINV_DES_CARD_TYPE ,    //the des sim card type
	MMINV_DES_NORMALCARD_ALLOWED_FLAG  = 596,    //the des sim card type
	MMINV_GSET_INFO,  // 存放配置信息	
	MMINV_DIST_INFO, // 存放里程信息
	MMINV_REGION_INFO_FIRST_ENTRY,
	MMINV_REGION_INFO_LAST_ENTRY = MMINV_REGION_INFO_FIRST_ENTRY + MMINV_MAX_REGION_RECORDS - 1,
 	MMINV_LINE_INFO_FIRST_ENTRY,
	MMINV_LINE_INFO_LAST_ENTRY = MMINV_LINE_INFO_FIRST_ENTRY + MMINV_MAX_LINE_RECORDS - 1,
 	MMINV_BLACK_BOX_FIRST_ENTRY,
	MMINV_BLACK_BOX_LAST_ENTRY = MMINV_BLACK_BOX_FIRST_ENTRY + MMINV_MAX_BLACKBOX_RECORDS - 1,
	MMINV_PIC0_INFO = 1050, // 存放pic 0
	MMINV_PIC1_INFO, // 存放pic 1
	MMINV_PIC2_INFO, // 存放pic 2
	MMINV_PIC3_INFO, // 存放pic 3
	MMINV_PIC4_INFO, // 存放pic 4

   MMI_NV_MAX_NUM = MN_NV_USER_BASE + MAX_NV_USER_ITEM_NUM
}MMI_NV_ITEM_E;
/********************************************************************
//    DESCRIPTION
//    set the flag= TURE when IMSI IS FULL 0xff, othersize FALSE 
//    Global resource dependence :
//    Author:Alex Li
//    Note:
***********************************************************************/
void MMI_SetIMSIIsFullFF(BOOLEAN flag);

/********************************************************************
//    DESCRIPTION
//    return TRUE if IMSI IS FULL 0xff
//    Global resource dependence :
//    Author:Alex Li
//    Note:
***********************************************************************/
BOOLEAN MMI_GetIMSIIsFullFF(void);


/********************************************************************
//    DESCRIPTION
//    set the flag= FALSE when The card is encrypted card 
//    Global resource dependence :
//    Author:Alex Li
//    Note:
***********************************************************************/
void MMI_SetNormalCardFlag(BOOLEAN flag);

/********************************************************************
//    DESCRIPTION
//    return TRUE if The card is normal
//    Global resource dependence :
//    Author:Alex Li
//    Note:
***********************************************************************/
BOOLEAN MMI_IsNormalCardFlag(void);

/********************************************************************
//    DESCRIPTION
//     get the get input command count
//    Global resource dependence :
//    Author:Alex Li
//    Note:
***********************************************************************/
uint32 MMI_GetGetInputCount(void);

/********************************************************************
//    DESCRIPTION
//     get the get input command count
//    Global resource dependence :
//    Author:Alex.Li
//    Note:
***********************************************************************/
void MMI_IncreaseGetInputCount(void);

/********************************************************************
//    DESCRIPTION
//    read the type of the des sim card from nv
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************/
DPHONE_DES_SIMCARD_TYPE_E MMI_GetDesSimCardType(void);

void MMI_ProcessFirstSTKGetInput(SIMAT_GET_INPUT_T* param_in);


/*-------------------------------------------------------------------------*/
/*                         TYPES/CONSTANTS                                 */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*                         FUNCTIONS                                       */
/*-------------------------------------------------------------------------*/
/*****************************************************************************/
// 	Description : get the length of the bcd code
//	Global resource dependence : 
//  Author: Great.Tian
//	Note:
/*****************************************************************************/
void MMI_GetBcdLen(
					   uint8 bcdFmt, // BCD码的类型，目前有三种: PACKED_LSB_FIRST, PACKED_MSB_FIRST, UNPACKED
					   uint8 *pBcd,  // BCD码的头指针
					   uint8 *pLen   // 返回BCD码的长度
					  );

/*****************************************************************************/
// 	Description : convert the bcd code to string
//	Global resource dependence : 
//  Author: Great.Tian
//	Note:
/*****************************************************************************/
 void MMI_BcdToStr(
					  uint8 bcdFmt, // BCD码的类型，目前有三种: PACKED_LSB_FIRST, PACKED_MSB_FIRST, UNPACKED
					  uint8 *pBcd,  // BCD码的头指针
					  uint8 bcdLen, // BCD码的长度
					  char* str     // 转换后String的头指针
					 );

/*****************************************************************************/
// 	Description : convert the bcd code(PACKED_LSB_FIRST) to the string
//	Global resource dependence : 
//  Author: Great.Tian
//	Note:
/*****************************************************************************/
 void MMI_BcdLfToStr(
						uint8 *pBcd,  // BCD码的头指针
						uint8 bcdLen, // BCD码的长度
						char* str     // 转换后String的头指针
					   );

/*****************************************************************************/
// 	Description : convert the bcd code(PACKED_MSB_FIRST) to the string
//	Global resource dependence : 
//  Author: Great.Tian
//	Note:
/*****************************************************************************/
 void MMI_BcdMfToStr(
						uint8 *pBcd,  // BCD码的头指针
						uint8 bcdLen, // BCD码的长度
						char* str     // 转换后String的头指针
					   );

/*****************************************************************************/
// 	Description : convert the bcd code(UNPACKED) to the string
//	Global resource dependence : 
//  Author: Great.Tian
//	Note:
/*****************************************************************************/
 void MMI_BcdUpToStr(
						uint8 *pBcd,  // BCD码的头指针
						uint8 bcdLen, // BCD码的长度
						char *str     // 转换后String的头指针
					   );

/*****************************************************************************/
// 	Description : convert the string to the bcd code
//	Global resource dependence : 
//  Author: Great.Tian
//	Note:
/*****************************************************************************/
 void MMI_StrToBcd(
					  uint8 bcdFmt, // BCD码的类型，目前有三种: PACKED_LSB_FIRST, PACKED_MSB_FIRST, UNPACKED
					  char *pStr,   // String的头指针
					  uint8 *pBcd   // 转换后BCD码的头指针
					 );

/*****************************************************************************/
// 	Description : convert the string to the bcd code(PACKED_LSB_FIRST)
//	Global resource dependence : 
//  Author: Great.Tian
//	Note:
/*****************************************************************************/
 void MMI_StrToBcdLf(
						char *pStr,   // String的头指针
						uint8 strLen, // String的长度
						uint8 *pBcd   // 转换后BCD码的头指针
					   );

/*****************************************************************************/
// 	Description : convert the string to the bcd code(PACKED_MSB_FIRST)
//	Global resource dependence : 
//  Author: Great.Tian
//	Note:
/*****************************************************************************/
 void MMI_StrToBcdMf(
						char *pStr,   // String的头指针
						uint8 strLen, // String的长度
						uint8 *pBcd   // 转换后BCD码的头指针
					   );

/*****************************************************************************/
// 	Description : convert the string to the bcd code(UNPACKED)
//	Global resource dependence : 
//  Author: Great.Tian
//	Note:
/*****************************************************************************/
 void MMI_StrToBcdUp(
						char *pStr,   // String的头指针
						uint8 strLen, // String的长度
						uint8 *pBcd   // 转换后BCD码的头指针
					   );





#endif
