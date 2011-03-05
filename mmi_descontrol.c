#include "sci_types.h"
//#include "sim_macro_switch.h"
//#include "sim_dphone_stk_verify.h"
//#include "mmi_nv.h"
//#include  "mmi_common.h"
#include "mn_api.h"

#include "mmi_descontrol.h"


//added by alex li on 2005 10 18,  the var is used to control entry STK getinput signal
static uint8 s_des_get_input_count = 0;
static BOOLEAN s_imsi_ff_flag = FALSE;
//end by alex li

static uint8 s_normalcard_flag = TRUE;


//added by alex li on 2005 10 18
const STATIC uint8 default_puk2_text[DPHONE_IND_PUK2_TEXT_LEN] = 
{0x8b,0xf7,0x8f,0x93,0x51,0x65,0x0,0x50,0x0,0x55,0x0,0x4b,0x0,0x32,0x5b,0xc6,0x78,0x1,0xff,0x1a};
//end add


///added by alex li on 2005 10 18
/********************************************************************
//    DESCRIPTION
//    set the flag= TURE when IMSI IS FULL 0xff, othersize FALSE 
//    Global resource dependence :
//    Author:Alex Li
//    Note:
***********************************************************************/
void MMI_SetIMSIIsFullFF(BOOLEAN flag)
{
	SCI_TRACE_LOW("Set imsi is full 0xff flag is %d", flag);
	s_imsi_ff_flag = flag;
}


/********************************************************************
//    DESCRIPTION
//    return TRUE if IMSI IS FULL 0xff
//    Global resource dependence :
//    Author:Alex Li
//    Note:
***********************************************************************/
BOOLEAN MMI_GetIMSIIsFullFF()
{
	SCI_TRACE_LOW("Get imsi is full 0xff flag is %d", s_imsi_ff_flag);
	
	return s_imsi_ff_flag;
}


/********************************************************************
//    DESCRIPTION
//    set the flag= FALSE when The card is encrypted card  
//    Global resource dependence :
//    Author:Alex Li
//    Note:
***********************************************************************/
void MMI_SetNormalCardFlag(BOOLEAN flag)
{
	SCI_TRACE_LOW("Set Normalcard flag is %d", flag);
	s_normalcard_flag = flag;
}


/********************************************************************
//    DESCRIPTION
//    return TRUE if The card is normal
//    Global resource dependence :
//    Author:Alex Li
//    Note:
***********************************************************************/
BOOLEAN MMI_IsNormalCardFlag()
{
	SCI_TRACE_LOW("Get Normalcard flag is %d", s_normalcard_flag);
	
	return s_normalcard_flag;
}

/********************************************************************
//    DESCRIPTION
//     get the get input command count
//    Global resource dependence :
//    Author:Alex Li
//    Note:
***********************************************************************/
uint32 MMI_GetGetInputCount(void)
{
    return s_des_get_input_count;
}


/********************************************************************
//    DESCRIPTION
//     get the get input command count
//    Global resource dependence :
//    Author:Alex.Li
//    Note:
***********************************************************************/
void MMI_IncreaseGetInputCount(void)
{
    s_des_get_input_count++;
}

void MMI_ProcessFirstSTKGetInput(SIMAT_GET_INPUT_T* param_in)
{
	extern BOOLEAN MMI_ProcessDesSimGetInput(SIMAT_GET_INPUT_T* com, SIMAT_DO_TEXT_STRING_T* temp_text_str);
	//SIMAT_DO_TEXT_STRING_T temp_text_str;

	//DPHONE_DES_SIMCARD_TYPE_E card_type;
	SIMAT_GET_INPUT_CNF_T param_out = {0};
		

	if(!MMI_ProcessDesSimGetInput(param_in, &(param_out.text_str)))
	{
		SCI_TRACE_LOW("SIMAT::SIMAT_ProcessDesSimGetInput(): not match the des card, display error.");
		
		//SIMAT_CloseShareBuf(param_in->buf_id);
		//not encrypted sim card
		MMI_Des_SetProcessDes(FALSE);
		
		//return;
	}
	else
	{
		SCI_TRACE_LOW("MMI_ProcessDesSimGetInput is ture");
		
		SCI_ASSERT(param_out.text_str.text_ptr != NULL);
	}
	
	
//	SIMAT_CloseShareBuf(param_in->buf_id);
	param_out.result = SIMAT_TR_SUCCESS;
	param_out.mp_info = SIMAT_MP_NO_SPECIFIC_CAUSE;
	
//	SIMAT_OpenShareBuf(&(param_out.buf_id)); //added by alex li 

	SIMAT_CnfGetInput(&param_out);

	//send response signal
	return;
	
	
}

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
				 )
{
    /* BCD format is defined in applayer.h                    */
    /* NOTE: LSB first 1234 = 0x21 0x43, bcdLen = 4           */
    /*                 123  = 0x21 0xf3, bcdLen = 3           */
    /*       MSB first 1234 = 0x12 0x34, bcdLen = 4           */
    /*                 123  = 0x12 0x3f, bcdLen = 3           */
    /*       unpacked  1234 = 0x01 0x02 0x03 0x04, bcdLen = 4 */
    switch(bcdFmt)
    {
        case PACKED_LSB_FIRST:
            MMI_BcdLfToStr(pBcd,bcdLen,str);
            break;
        case PACKED_MSB_FIRST:
            MMI_BcdMfToStr(pBcd,bcdLen,str);
            break;
        case UNPACKED        :
            MMI_BcdUpToStr(pBcd,bcdLen,str);
            break;
        default:
            // DevFail("wrong bcd format");
            break;
    }
}   /* End Of mmi_BcdToStr() */

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
				   )
{   /*BCD format - LSB first (1234 = 0x21, 0x43)*/
    uint8 i;
    uint8 bcdCode;
    uint8 ascCode;

    for(i = 0;i < bcdLen; i++)
    {
        bcdCode = (pBcd[i/2] >> (((i+1) & 1) ? 0 : 4)) & 0x0F;
        if(bcdCode == DIALBCD_FILLER)
        {
            break;
        }
        ascCode = (bcdCode == DIALBCD_STAR) ? '*':
                  (bcdCode == DIALBCD_HASH) ? '#':
                  (bcdCode == DIALBCD_PAUSE)? 'P':
                  (bcdCode == DIALBCD_WILD) ? 'w': (bcdCode + '0');


        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
           ascCode = 0;

        str[i] = ascCode;
    }

    str[i] = 0;
}  /*-- End of MMI_BcdLfToStr( ) --*/

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
				   )
{   /*BCD format - MSB first (1234 = 0x12 0x34)*/
    uint8 i;
    uint8 bcdCode;
    uint8 ascCode;

    for(i = 0;i < bcdLen; i++)
    {
        bcdCode = (pBcd[i/2] >> ((i & 1) ? 0 : 4)) & 0x0F;
        if(bcdCode == 0x0f) break;
        ascCode = (bcdCode == DIALBCD_STAR) ? '*':
                  (bcdCode == DIALBCD_HASH) ? '#':
                  (bcdCode == DIALBCD_PAUSE)? 'P':
                  (bcdCode == DIALBCD_WILD) ? 'w': (bcdCode + '0');

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
           ascCode = 0;

        str[i] = ascCode;
    }

    str[i] = 0;
}  /*-- End of MMI_BcdMfToStr( ) --*/

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
				   )
{   /*BCD format - unpacked (1 digit per byte)*/
    uint8 i;
    uint8 bcdCode;
    uint8 ascCode;

    for(i = 0;i < bcdLen; i++)
    {
        bcdCode = pBcd[i];
        ascCode = (bcdCode == DIALBCD_STAR) ? '*':
                  (bcdCode == DIALBCD_HASH) ? '#':
                  (bcdCode == DIALBCD_PAUSE)? 'P':
                  (bcdCode == DIALBCD_WILD) ? 'w': (bcdCode + '0');

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
           ascCode = 0;

        str[i] = ascCode;
    }

    str[i] = 0;
}  /*-- End of MMI_BcdUpToStr( ) --*/

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
				 )
{
    uint8 strLen;

    strLen = strlen(pStr);

    switch(bcdFmt)
    {
        case PACKED_LSB_FIRST:
            MMI_StrToBcdLf(pStr,strLen,pBcd);
            break;
        case PACKED_MSB_FIRST:
            MMI_StrToBcdMf(pStr,strLen,pBcd);
            break;
        case UNPACKED        :
            MMI_StrToBcdUp(pStr,strLen,pBcd);
            break;
        default:
            // DevFail("Wrong bcd format");
            break;
    }
}   /* End Of mmi_StrToBcd() */

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
				   )
{
    uint8 bcdCode;
    uint8 ascCode;
    uint8 i;

    /*memset(pBcd,0xFF,(strLen&1) ? (strLen+1)/2 : strLen/2+1);*/

    for(i=0;i < strLen;i++)
    {
        ascCode = pStr[i];
        /*bcdCode = 0xFF;*/

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
            break;
        /*
        bcdCode = ( ascCode == '*' ) ? DIALBCD_STAR:
                  ( ascCode == '#' ) ? DIALBCD_HASH:
                  ( ascCode == 'P'||ascCode == 'p') ? DIALBCD_PAUSE:
                  ( ascCode == 'W' ) ? DIALBCD_WILD:(ascCode - '0');
        */
        switch( ascCode )
        {
            case '*':
                bcdCode = DIALBCD_STAR;
                break;
            case '#':
                bcdCode = DIALBCD_HASH;
                break;
            case 'P':
            case 'p':
                bcdCode = DIALBCD_PAUSE;
                break;
            case 'W':
            case 'w':
                bcdCode = DIALBCD_WILD;
                break;
            default:
                bcdCode = ascCode - '0';
                break;
        }
        pBcd[i/2] = ((i & 1) ? pBcd[i/2] : 0) +
                    (bcdCode << ((i & 1) ? 4 : 0));
    }
    if(i&1)
    {
       pBcd[(i-1)/2] |= 0xf0;
    }

}   /* End Of MMI_StrToBcdLf() */

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
				   )
{
    uint8 bcdCode;
    uint8 ascCode;
    uint8 i;

  /*memset(pBcd,0xFF,(strLen&1) ? (strLen+1)/2 : strLen/2+1); */

    for(i=0;i < strLen;i++)
    {
        ascCode = pStr[i];
        /*bcdCode = 0xFF;*/

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
            break;
        /*
        bcdCode = ( ascCode == '*' ) ? DIALBCD_STAR:
                  ( ascCode == '#' ) ? DIALBCD_HASH:
                  ( ascCode == 'P'||ascCode == 'p') ? DIALBCD_PAUSE:
                  ( ascCode == 'W' ) ? DIALBCD_WILD:(ascCode - '0');
        */
        switch( ascCode )
        {
            case '*':
                bcdCode = DIALBCD_STAR;
                break;
            case '#':
                bcdCode = DIALBCD_HASH;
                break;
            case 'P':
            case 'p':
                bcdCode = DIALBCD_PAUSE;
                break;
            case 'W':
            case 'w':
                bcdCode = DIALBCD_WILD;
                break;
            default:
                bcdCode = ascCode - '0';
                break;
        }
        pBcd[i/2] = ((i & 1) ? pBcd[i/2] : 0) +
                    (bcdCode << ((i+1 & 1) ? 4 : 0));
    }
    if(i&1)
    {
       pBcd[(i-1)/2] |= 0x0f;
    }

}   /* End Of MMI_StrToBcdMf() */

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
				   )
{
    uint8 bcdCode;
    uint8 ascCode;
    uint8 i;

    /*memset(pBcd,0xFF,strLen+1);*/

    for(i=0;i < strLen;i++)
    {
        ascCode = pStr[i];

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
            break;
        /*
        bcdCode = ( ascCode == '*' ) ? DIALBCD_STAR:
                  ( ascCode == '#' ) ? DIALBCD_HASH:
                  ( ascCode == 'P'||ascCode == 'p') ? DIALBCD_PAUSE:
                  ( ascCode == '+' ) ? DIALBCD_WILD:(ascCode - '0');
        */

        switch( ascCode )
        {
            case '*':
                bcdCode = DIALBCD_STAR;
                break;
            case '#':
                bcdCode = DIALBCD_HASH;
                break;
            case 'P':
            case 'p':
                bcdCode = DIALBCD_PAUSE;
                break;
            case 'W':
            case 'w':
                bcdCode = DIALBCD_WILD;
                break;
            default:
                bcdCode = ascCode - '0';
                break;
        }
        pBcd[i] = bcdCode;
    }
}   /* End Of MMI_StrToBcdUp() */



