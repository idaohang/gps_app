/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Timer.h
  �汾   ��1.50
  ������ ���º���
  ����ʱ�䣺2007-7-23
  ���������� 
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_TIMER_H_
#define _SG_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
*******************************��������****************************
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
