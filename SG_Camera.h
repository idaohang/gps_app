/****************************************************************
  ��Ȩ   �������������ͨѶ���޹�˾ 2002-2005 ��Ȩ����
  �ļ��� ��SG_Camera.h
  �汾   ��1.00
  ������ ���º���
  ����ʱ�䣺2006-3-17
  ����������SG2000 Camera��Ϣ���շ���     
  �޸ļ�¼��
***************************************************************/
#ifndef _SG_Camera_H_
#define _SG_Camera_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CAMERA_OFFSET 37

#define SG_PICTURE_SAVE_MAX_FILE 5		//ͼƬ�洢�ļ�

#define PIC_NOT_SAVE 0 // ͼƬ�����д洢
#define PIC_SAVE 1 // ͼƬ���д洢

#define CAMERA_TIME_WATCH_REQ 0
#define CAMERA_CONDITION_WATCH_REQ 1
#define CAMERA_ONE_REQ 2

#define CAMERA_ONE_PIC_LEN 10000
#define CAMERA_ONE_PACK_LEN 5120



typedef struct _CAMERA_CMD_
{
	unsigned long cmd;//��ǰ������ 
	unsigned long para;//
	unsigned char saveflag;
	struct _CAMERA_CMD_ *next;
}CAMERA_CMD;



typedef  enum  SG_CAMERA_STATE
{
	CAMERA_START = 0, // ��ʼ״̬
	CAMERA_SYNC =1,//����ͬ���ȴ�Ӧ��   ���Ͳ�������ȴ�Ӧ��(������ľ)
	CAMERA_WAITSYNC=2,//�ȴ��Է�����ͬ��
	CAMERA_WAITINIT=3,//�ȴ���ʼ��״̬
	CAMERA_INIT=4,//��ʼ��   ������������ȴ�Ӧ��(������ľ)
	CAMERA_SET_MSG_LEN=5,//�������ݰ���С
	CAMERA_WAIT_GET_PIC=6,//�ȴ���ȡͼ��
	CAMERA_GET_PIC=7,//��ȡͼ��
	CAMERA_GET_XINDEX_PIC=8,//������ϣ���ȡ��n������
	CAMERA_WAIT=9, // �ȴ�״̬
	CAMERA_IDLE=10, // ����״̬
	CAMERA_IDLE_SYNC=11,//idle״̬�·���ͬ���ȴ�Ӧ��
	CAMERA_IDLE_WAITSYNC=12, // idle״̬�µȴ�ͬ��״̬
	CAMERA_RESET=13, // ��λ״̬
	CAMERA_WAIT_START,
	CAMERA_WAIT_SET
	
}sg_camera_state;

typedef struct SG_CAMERA_
{
	sg_camera_state	currst;//
	CAMERA_CMD	*head;
}SG_CAMERA;



void camera_construct_cmd(uint8 id,uint8 p1,uint8 p2,uint8 p3,uint8 p4);

void sg_camera_request(int quality,int cmd,int save_flag);
void SG_Camera_Task_Delete(int task);
void camera_reset(void);
BOOLEAN SG_Camera_Task_Check(int task);
int SG_Camera_Interval_Check(void);
void Camera_Condition_Watch(void);
int Camera_Get_Pic_IamgeQuality(void);
void camera_run(void);
void camera_cmd_delete(void);
int camera_task_count(void);
void camera_timout_check(void);
void Camera_Wakeup_Handle(void);
void QQZM_Camera_Run(void);


#ifdef __cplusplus
}
#endif

#endif //_SG_Camera_H_
