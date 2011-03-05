#define APP_TASKS_C
/*******************************************************************/
/*  Copyright (C) 2001 SPREADTRUM COMMUNICATION¡¡CO.               */
/*  All rights reserved.                                           */
/*                                                                 */
/*  This document contains proprietary information belonging to    */
/*  SPREADTRUM. Passing on and copying of this                     */
/*  document, use and communication of its contents is not         */
/*  permitted without prior written authorisation.                 */
/*******************************************************************/
/* Revision information:                                           */
/* File      [ mmilcd.c                       ]                    */
/* Version   [ 0.1  Draft  04/03/2003   jakle zhu  ]               */
/* Description  This file provide lcd APIs for LCD display         */
/*                                                                 */
/*******************************************************************/

#include "sci_types.h"
#include "os_api.h"
#include "user_tasks_id.h"
#include "tasks_id.h"

// Create static thread macro.
#define CREATE_APP_STATIC_THREAD(NAME, THREAD_ENTRY, PRIO) \
        SCI_CreateStaticThread( NAME, "T_"#NAME, THREAD_ENTRY, 0, 0, \
            NAME##_THREAD_ADDR, NAME##_STACK_SIZE, PRIO, SCI_PREEMPT, \
            "Q_"#NAME, NAME##_QUEUE_ADDR, NAME##_QUEUE_NUM, SCI_AUTO_START);

extern void APP_Task(uint32 argc, void * argv);

// stack and queue define
#define   P_APP_STACK_SIZE              8000
uint32    P_APP_THREAD_ADDR[P_APP_STACK_SIZE >> 2];
#define   P_APP_QUEUE_NUM               80
uint32    P_APP_QUEUE_ADDR[P_APP_QUEUE_NUM * SCI_QUEUE_ITEM_SIZE];


uint32 SCI_InitAppTasks(void)
{
    // Create the APP tasks

    // MMI task create
    CREATE_APP_STATIC_THREAD(P_APP,           APP_Task,                SCI_PRIORITY_LOWEST);

    // Add the other taks create function here
   
    return 0;
}