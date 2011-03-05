/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_Timer.h
  版本   ：1.50
  创建者 ：陈海华
  创建时间：2007-7-23
  内容描述： 
  修改记录：
***************************************************************/
#ifndef _SG_TIMER_H_
#define _SG_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
*******************************函数声明****************************
********************************************************************/
void MnTime_EventCallback(
                                uint32 task_id, //task ID
                                uint32 event_id, //event ID
                                void *param_ptr
                                );
void RevTmr_isr_handler(uint32 state);
void SG_LED_Status(void);

void SG_CreateSignal (uint16       signalType,
                       int16      signalSize,
                       xSignalHeaderRec **signal);

void SG_CreateSignal_Para (uint16       signalType,
                       int16      signalSize,
                       xSignalHeaderRec **signal,
                       char *para);

#endif //_SG_TIMER_H_
