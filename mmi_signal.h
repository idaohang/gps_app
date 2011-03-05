/******************************************************************************
 ** File Name:      MMI_SIGNAL.h                                              *
 ** Author:         Richard.Yang                                              *
 ** DATE:           10/11/2004                                                *
 ** Copyright:      2004 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the basic Application Interface (API)   *
 **                 to the high-performance RTOS.                             *
 **                 All service prototypes for user and some data structure   *
 **                 definitions are defined in this file.                     *
 **                 Basic data type definitions is contained in sci_types.h   *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 11/14/2002     Xueliang.Wang    Create.                                   *
 ** 09/12/2003     Zhemin.Lin       Modify according to CR:MS00004678         *
 ******************************************************************************/
#ifndef _MMI_SIGNAL_H
#define _MMI_SIGNAL_H

#include "sci_types.h"
#include "os_api.h"

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
    extern   "C"
    {
#endif

/**---------------------------------------------------------------------------*
 ** SIGNAL MACRO: 
 **         MMI_GET_SIGCODE
 **         MMI_GET_SIGSENDER
 **         MMI_CREATE_SIGNAL
 **         MMI_SEND_SIGNAL
 **         MMI_RECEIVE_SIGNAL
 **         MMI_PEEK_SIGNAL
 **         MMI_FREE_SIGNAL
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    Get signal code from the pointer to signal.
//	Global resource dependence:
//  Author: Xueliang.Wang
//  Parameter:      _SIG_PTR        Pointer to the signal
//  Return:         Signal code of the signal.
//	Note:
/*****************************************************************************/
/******************************************************************************
uint16  MMI_GET_SIGCODE(
    xSignalHeader   _SIG_PTR
    )
******************************************************************************/
#define MMI_GET_SIGCODE(_SIG_PTR) \
        (((xSignalHeader)(_SIG_PTR))->SignalCode)

/*****************************************************************************/
//  Description:    Get sender from the pointer to signal.
//	Global resource dependence:
//  Author: Xueliang.Wang
//  Parameter:      _SIG_PTR        Pointer to the signal
//  Return:         Sender of the signal.
//	Note:
/*****************************************************************************/
/******************************************************************************
BLOCK_ID    SCI_GET_SIGSENDER(
    xSignalHeader   _SIG_PTR
    )
******************************************************************************/
#define MMI_GET_SIGSENDER(_SIG_PTR) \
        (((xSignalHeader)(_SIG_PTR))->Sender)


/*****************************************************************************/
//  Description:    Create a signal.
//	Global resource dependence:
//  Author: Xueliang.Wang
//  Parameter:      _SIG_PTR        Pointer to the signal which will be created
//                  _SIG_CODE       Signal code.
//                  _SIG_SIZE       Number of bytes will be alloc for the signal
//                  _SENDER         Sender of this signal.
//  Return:         None.
//	Note:
/*****************************************************************************/
/******************************************************************************
void    SCI_CREATE_SIGNAL(
    xSignalHeader   _SIG_PTR, 
    uint16      _SIG_CODE, 
    uint16      _SIG_SIZE, 
    BLOCK_ID    _SENDER
    )
******************************************************************************/
#define MMI_CREATE_SIGNAL(_SIG_PTR, _SIG_CODE, _SIG_SIZE, _SENDER) \
		(xSignalHeader)(_SIG_PTR) = (xSignalHeader)SCI_ALLOC(_SIG_SIZE);\
		SCI_ASSERT((_SIG_PTR) != SCI_NULL); \
		SCI_MEMSET((void*)(_SIG_PTR), 0 , _SIG_SIZE); \
		((xSignalHeader)(_SIG_PTR))->SignalSize = _SIG_SIZE; \
		((xSignalHeader)(_SIG_PTR))->SignalCode = _SIG_CODE; \
		((xSignalHeader)(_SIG_PTR))->Sender     = _SENDER;

/*****************************************************************************/
//  Description:    Send a signal.
//	Global resource dependence:
//  Author: Xueliang.Wang
//  Parameter:      _SIG_PTR        Pointer to the signal which will be sent
//                  _RECEIVER       ID of thread whihc receives this signal.
//  Return:         None.
//	Note:
/*****************************************************************************/
/******************************************************************************
void SCI_SEND_SIGNAL(
    xSignalHeader   _SIG_PTR,     // Signal pointer to be sent
    BLOCK_ID        _RECEIVER     // Dest thread ID
    )
******************************************************************************/
#define MMI_SEND_SIGNAL(_SIG_PTR, _RECEIVER) \
        SCI_SendSignal((xSignalHeader)(_SIG_PTR), _RECEIVER);

/*****************************************************************************/
//  Description:    Receive a signal.
//	Global resource dependence:
//  Author: Xueliang.Wang
//  Parameter:      _SIG_PTR        Pointer to a signal to save received signal.
//                  _SELF_ID        ID of thread which receives this signal.
//  Return:         None.
//	Note:
/*****************************************************************************/
/******************************************************************************
SCI_RECEIVE_SIGNAL(
    xSignalHeader   _SIG_PTR,
    BLOCK_ID        _SELF_ID
    )
******************************************************************************/
#define MMI_RECEIVE_SIGNAL(_SIG_PTR, _SELF_ID) \
		_SIG_PTR = SCI_GetSignal(_SELF_ID); 

/*****************************************************************************/
//  Description:    Gets a signal from a thread immediatelly, if no signal, 
//                  function returns immediatelly.
//	Global resource dependence: 
//  Author: Richard.Yang
//  Parameter:      _SIG_PTR        Pointer to a signal to save received signal.
//                  _SELF_ID        ID of thread which receives this signal.
//  Return:         None.
//	Note:
/*****************************************************************************/
/******************************************************************************
PUBLIC xSignalHeader SCI_PEEK_SIGNAL(   // Return received signal.
    xSignalHeader   _SIG_PTR,
    BLOCK_ID        _SELF_ID
    );
******************************************************************************/
#define MMI_PEEK_SIGNAL(_SIG_PTR, _SELF_ID) \
        _SIG_PTR = SCI_PeekSignal(_SELF_ID)
    

/*****************************************************************************/
//  Description:    Free a signal.
//	Global resource dependence:
//  Author: Xueliang.Wang
//  Parameter:      _SIG_PTR        Pointer to the signal which will be freed.
//  Return:         None   
//	Note:
/*****************************************************************************/
/******************************************************************************
void    SCI_FREE_SIGNAL(
    xSignalHeader   _SIG_PTR
    );
******************************************************************************/
#define MMI_FREE_SIGNAL(_SIG_PTR) \
        SCI_FREE(_SIG_PTR)

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
    }
#endif

#endif  // End of _MMI_SIGNAL_H

