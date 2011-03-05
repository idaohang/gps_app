/******************************************************************************
 ** File Name:      keymap.c                                                  *
 ** Author:         Xia.Yang                                                  *
 ** DATE:           04/13/2004                                                *
 ** Copyright:      2004 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 04/13/2004     Xia.Yang         Create.                                   *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "tb_dal.h"
const uint16 keymap[] = {
    SCI_VK_CALL,                     	// 00 BOARD_FOR_KEY_SEND
    SCI_VK_UP,                     		// 01 BOARD_FOR_KEY_UP 
    SCI_VK_STAR,                     	// 02 BOARD_FOR_KEY_STAR
    SCI_VK_7,                     		// 03 BOARD_FOR_KEY_7
    SCI_VK_4,                     		// 04 BOARD_FOR_KEY_4 
    SCI_VK_1,                     		// 05 BOARD_FOR_KEY_1 
    SCI_VK_INVALID_KEY,                 // 06 undefined
    SCI_VK_INVALID_KEY,                 // 07 undefined   
    SCI_VK_LEFT,                        // 08 BOARD_FOR_KEY_LEFT
    SCI_GPIO_SIG1,             			// 09 BOARD_FOR_KEY_HANDFREE
    SCI_VK_0,                           // 10 BOARD_FOR_KEY_0
    SCI_VK_8,                           // 11 BOARD_FOR_KEY_8 , 
    SCI_VK_5,                           // 12 BOARD_FOR_KEY_5 
    SCI_VK_2,                           // 13 BOARD_FOR_KEY_2
    SCI_VK_INVALID_KEY,                 // 14 undefined
    SCI_VK_INVALID_KEY,                 // 15 undefined
    SCI_VK_RIGHT,                       // 16 BOARD_FOR_KEY_RIGHT
    SCI_VK_DOWN,                        // 17 BOARD_FOR_KEY_DOWN
    SCI_VK_POUND,                       // 18 BOARD_FOR_KEY_POUND
    SCI_VK_9,                           // 19 BOARD_FOR_KEY_9
    SCI_VK_6,                           // 20 BOARD_FOR_KEY_6
    SCI_VK_3,                 			// 21 BOARD_FOR_KEY_3
    SCI_VK_INVALID_KEY,                 // 22 undefined
    SCI_VK_INVALID_KEY,                 // 23 undefined
    SCI_VK_CANCEL,                      // 24 BOARD_FOR_KEY_CANCEL
    SCI_VK_POWER,                       // 25 BOARD_FOR_KEY_REDIAL
    SCI_VK_MENU_SELECT,                 // 26 BOARD_FOR_KEY_LEFTSOFT
    SCI_VK_MENU_CANCEL,                 // 27 BOARD_FOR_KEY_RIGHTSOFT
    SCI_VK_SIDE_DOWN,                   // 28 BOARD_FOR_KEY_SMS
    SCI_VK_SIDE_UP,                     // 29 BOARD_FOR_KEY_CALLLOG
    SCI_VK_INVALID_KEY,                 // 30 undefined
    SCI_VK_INVALID_KEY,                 // 31 undefined
    SCI_VK_INVALID_KEY,                 // 32 undefined
    SCI_VK_INVALID_KEY,                 // 33 undefined
    SCI_VK_INVALID_KEY,                 // 34 undefined
    SCI_VK_INVALID_KEY,                 // 35 undefined
    SCI_VK_INVALID_KEY,                 // 36 undefined
    SCI_VK_INVALID_KEY,                 // 37 undefined
};

